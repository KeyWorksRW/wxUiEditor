/////////////////////////////////////////////////////////////////////////////
// Purpose:   Hold data for currently loaded project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <filesystem>
#include <fstream>
#include <thread>

#include <wx/animate.h>   // wxAnimation and wxAnimationCtrl
#include <wx/artprov.h>   // wxArtProvider class
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/filesys.h>   // class for opening files - virtual file system
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes

#include "ttcview.h"     // cview -- string_view functionality on a zero-terminated char string.
#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "pjtsettings.h"  // ProjectSettings

#include "bitmaps.h"    // Map of bitmaps accessed by name
#include "mainapp.h"    // App -- App class
#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class
#include "utils.h"      // Utility functions that work with properties

// Convert a data array into a wxAnimation
inline wxAnimation GetAnimFromHdr(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream strm(data, size_data);
    wxAnimation animation;
    animation.Load(strm);
    return animation;
};

namespace wxue_img
{
    extern const unsigned char pulsing_unknown_gif[377];
}

ProjectSettings::ProjectSettings() {}

ProjectSettings::~ProjectSettings()
{
    // If the thread is running, this will tell it to stop
    m_is_terminating = true;

    if (m_collect_thread)
    {
        m_collect_thread->join();
        delete m_collect_thread;
    }
}

ttlib::cstr& ProjectSettings::SetProjectFile(const ttString& file)
{
    m_projectFile.clear();
    m_projectFile << file.wx_str();
    return m_projectFile;
}

ttlib::cstr& ProjectSettings::setProjectFile(const ttlib::cstr& file)
{
    m_projectFile = file;
    return m_projectFile;
}

ttlib::cstr& ProjectSettings::SetProjectPath(const ttString& file, bool remove_filename)
{
    m_projectPath.clear();
    m_projectPath << file.wx_str();
    if (remove_filename)
        m_projectPath.remove_filename();
#if defined(_WIN32)
    m_projectPath.backslashestoforward();
#endif  // _WIN32
    return m_projectPath;
}

ttlib::cstr& ProjectSettings::setProjectPath(const ttlib::cstr& file, bool remove_filename)
{
    m_projectPath = file;
    if (remove_filename)
        m_projectPath.remove_filename();
    return m_projectPath;
}

wxImage ProjectSettings::GetPropertyBitmap(const ttlib::cstr& description, bool want_scaled)
{
    ttlib::multistr parts(description, BMP_PROP_SEPARATOR);
    for (auto& iter: parts)
    {
        iter.BothTrim();
    }

    if (parts[IndexImage].empty())
    {
        return GetInternalImage("unknown");
    }

    wxImage image;

    auto path = parts[IndexImage];

    auto result = m_images.find(path);
    if (result != m_images.end())
    {
        image = result->second;
    }
    else if (parts[IndexType].contains("Art"))
    {
        if (auto pos = parts[IndexArtID].find('|'); ttlib::is_found(pos))
        {
            ttlib::cstr client = parts[IndexArtID].subview(pos + 1);
            parts[IndexArtID].erase(pos);
            image = wxArtProvider::GetBitmap(parts[IndexArtID], wxART_MAKE_CLIENT_ID_FROM_STR(client)).ConvertToImage();
        }
        else
        {
            image =
                wxArtProvider::GetBitmap(parts[IndexArtID], wxART_MAKE_CLIENT_ID_FROM_STR("wxART_OTHER")).ConvertToImage();
        }
    }
    else if (parts[IndexType].contains("Embed"))
    {
        if (!path.file_exists())
        {
            path = wxGetApp().GetProjectPtr()->prop_as_string(prop_original_art);
            path.append_filename(parts[IndexImage]);
        }
        auto embed = GetEmbeddedImage(path);
        if (!embed)
        {
            bool added = AddEmbeddedImage(path, wxGetFrame().GetSelectedForm());
            if (added)
            {
                embed = GetEmbeddedImage(path);
            }
        }

        if (embed)
        {
            wxMemoryInputStream stream(embed->array_data.get(), embed->array_size);
            image.LoadFile(stream);
        }
    }
    else
    {
        if (!path.file_exists())
        {
            path = wxGetApp().GetProjectPtr()->prop_as_string(prop_converted_art);
            path.append_filename(parts[IndexImage]);

            if (result = m_images.find(path); result != m_images.end())
            {
                image = result->second;
            }
        }

        if (!image.IsOk())
        {
            if (path.has_extension(".h_img"))
            {
                image = GetHeaderImage(path);
            }
            else
            {
                image.LoadFile(path);
            }
        }
    }

    if (!image.IsOk())
    {
        return GetInternalImage("unknown");
    }

    // If it's not embedded, then cache it so that we don't read it from disk again
    if (!parts[IndexType].contains("Embed") && result == m_images.end())
        m_images[path] = image;

    // Scale if needed
    if (want_scaled && parts.size() > IndexScale)
    {
        // If a dimension was specified, then it will have been split out, so we need to combine them
        if (parts.size() > IndexScale + 1)
        {
            parts[IndexScale] << ',' << parts[IndexScale + 1];
        }

        ttlib::multistr scale_parts(parts[IndexScale].c_str() + 1, ',');

        wxSize scale_size;
        scale_size.x = scale_parts[0].atoi();
        scale_size.y = scale_parts[1].atoi();

        if (scale_size.x != -1 || scale_size.y != -1)
        {
            auto original_size = image.GetSize();
            if (scale_size.x != -1)
                original_size.x = scale_size.x;
            if (scale_size.y != -1)
                original_size.y = scale_size.y;

            // Scaling a mask doesn't work well at high quality, so only use higher quality for images with no mask (alpha
            // channel is fine)
            auto newImage = image.Scale(original_size.x, original_size.y,
                                        image.HasMask() ? wxIMAGE_QUALITY_NORMAL : wxIMAGE_QUALITY_HIGH);
            if (newImage.IsOk())
            {
                return newImage;
            }
        }
    }

    return image;
}

wxAnimation ProjectSettings::GetPropertyAnimation(const ttlib::cstr& description)
{
    ttlib::multistr parts(description, BMP_PROP_SEPARATOR);
    for (auto& iter: parts)
    {
        iter.BothTrim();
    }

    wxAnimation image;

    if (parts[IndexImage].empty())
    {
        return GetAnimFromHdr(wxue_img::pulsing_unknown_gif, sizeof(wxue_img::pulsing_unknown_gif));
    }

    auto path = parts[IndexImage];
    if (!path.file_exists())
    {
        path = wxGetApp().GetProjectPtr()->prop_as_string(prop_original_art);
        path.append_filename(parts[IndexImage]);
    }

    if (parts[IndexType].contains("Embed"))
    {
        auto embed = GetEmbeddedImage(path);
        if (!embed)
        {
            bool added = AddEmbeddedImage(path, wxGetFrame().GetSelectedForm());
            if (added)
            {
                embed = GetEmbeddedImage(path);
            }
        }

        if (embed)
        {
            wxMemoryInputStream stream(embed->array_data.get(), embed->array_size);
            image.Load(stream);
        }
    }
    else
    {
        // This handles Header files
        GetAnimationImage(image, path);
    }

    if (!image.IsOk())
    {
        return GetAnimFromHdr(wxue_img::pulsing_unknown_gif, sizeof(wxue_img::pulsing_unknown_gif));
    }

    return image;
}

bool isConvertibleMime(const ttString& suffix);  // declared in embedimg.cpp

bool ProjectSettings::AddEmbeddedImage(ttlib::cstr path, Node* form)
{
    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

    if (!path.file_exists())
    {
        if (wxGetApp().GetProject()->HasValue(prop_original_art))
        {
            ttlib::cstr art_path = wxGetApp().GetProject()->prop_as_string(prop_original_art);
            art_path.append_filename(path);
            if (!art_path.file_exists())
                return false;
            path = std::move(art_path);
        }
        else
        {
            return false;
        }
    }

    if (m_map_embedded.find(path.filename().c_str()) != m_map_embedded.end())
        return false;

    wxFFileInputStream stream(path.wx_str());
    if (!stream.IsOk())
        return false;

    wxImageHandler* handler;
    auto& list = wxImage::GetHandlers();
    for (auto node = list.GetFirst(); node; node = node->GetNext())
    {
        handler = (wxImageHandler*) node->GetData();
        if (handler->CanRead(stream))
        {
            wxImage image;
            if (handler->LoadFile(&image, stream))
            {
                m_map_embedded[path.filename().c_str()] = std::make_unique<EmbededImage>();
                auto embed = m_map_embedded[path.filename().c_str()].get();
                embed->array_name = path.filename();
                embed->array_name.Replace(".", "_", true);
                embed->form = form;

                // At this point, other threads can lookup and add an embedded image, they just can't access the data of this
                // image until we're done. I.e., GetEmbeddedImage() won't return until retrieve_lock is released.

                std::unique_lock<std::mutex> retrieve_lock(m_mutex_embed_retrieve);
                add_lock.unlock();

                // If possible, convert the file to a PNG -- even if the original file is a PNG, since we might end up with
                // better compression.

                if (isConvertibleMime(handler->GetMimeType()))
                {
                    embed->type = wxBITMAP_TYPE_PNG;

                    wxMemoryOutputStream save_stream;

                    // Maximize compression
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
                    image.SaveFile(save_stream, handler->GetMimeType());

                    auto read_stream = save_stream.GetOutputStreamBuffer();
                    embed->array_size = read_stream->GetBufferSize();
                    embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                    memcpy(embed->array_data.get(), read_stream->GetBufferStart(), embed->array_size);
                }
                else
                {
                    embed->type = handler->GetType();

                    stream.SeekI(0);
                    embed->array_size = stream.GetSize();
                    embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                    stream.Read(embed->array_data.get(), embed->array_size);
                }

                return true;
            }
        }
    }

    return false;
}

const EmbededImage* ProjectSettings::GetEmbeddedImage(ttlib::cstr path)
{
    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

    if (auto result = m_map_embedded.find(path.filename().c_str()); result != m_map_embedded.end())
    {
        std::unique_lock<std::mutex> retrieve_lock(m_mutex_embed_retrieve);
        return result->second.get();
    }
    else
    {
        return nullptr;
    }
}

// To consistently generate the same code, an image declaration needs to be added to the first form that uses it. That means
// that all embeded images need to be initialized -- otherwise, the user could select a form that has not been initialized
// yet, and it will look like it's the first form to use the image, when in fact it's just because the previous form wasn't
// initialized.

// When a project is loaded, MainFrame start processing nodes and collecting embedded image data. However, that will stop as
// soon as it's determined that at least one of the forms needs to have code regenerated. So to be certain that every node
// gets parsed, we create a background thread that parses all nodes every time a project is loaded.

void ProjectSettings::ParseEmbeddedImages()
{
    if (m_collect_thread)
    {
        m_collect_thread->join();
        return;
    }

    m_collect_thread = new std::thread(&ProjectSettings::CollectEmbeddedImages, this);
}

void ProjectSettings::CollectEmbeddedImages()
{
    // We need the shared ptr rather than the raw pointer because we can't let the pointer become invalid while we're still
    // processing nodes.
    auto project = wxGetApp().GetProjectPtr();

    for (size_t pos = 0; pos < project->GetChildCount(); ++pos)
    {
        if (m_is_terminating)
            return;

        auto form = project->GetChildPtr(pos);

        for (auto& iter: form->GetChildNodePtrs())
        {
            if (m_is_terminating)
                return;
            CollectNodeImages(iter.get(), form.get());
        }
    }
}

void ProjectSettings::CollectNodeImages(Node* node, Node* form)
{
    for (auto& iter: node->get_props_vector())
    {
        if (iter.type() == type_image || iter.type() == type_animation)
        {
            if (!iter.HasValue())
                continue;
            auto& value = iter.as_string();
            if (value.is_sameprefix("Embed"))
            {
                if (m_is_terminating)
                    return;
                ttlib::multistr parts(value, BMP_PROP_SEPARATOR);
                for (auto& iter_parts: parts)
                {
                    iter_parts.BothTrim();
                }

                if (parts[IndexImage].size())
                {
                    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);
                    if (auto result = m_map_embedded.find(parts[IndexImage].filename().c_str());
                        result == m_map_embedded.end())
                    {
                        if (m_is_terminating)
                            return;

                        add_lock.unlock();
                        AddEmbeddedImage(parts[IndexImage], form);
                        if (m_is_terminating)
                            return;
                    }
                }
            }
        }
    }

    auto count = node->GetChildCount();
    for (size_t i = 0; i < count; i++)
    {
        if (m_is_terminating)
            return;
        auto child = node->GetChildPtr(i);
        CollectNodeImages(child.get(), form);
    }
}

namespace wxue_img
{
    inline const unsigned char pulsing_unknown_gif[377] {
        71,  73,  70,  56,  57,  97,  15,  0,   20,  0,   196, 0,   0,   255, 255, 255, 253, 124, 134, 253, 118, 129, 253,
        115, 126, 252, 108, 120, 252, 105, 117, 255, 102, 102, 251, 100, 113, 250, 87,  101, 250, 84,  98,  249, 77,  91,
        249, 71,  86,  248, 67,  82,  248, 62,  77,  248, 58,  74,  247, 48,  65,  246, 41,  59,  246, 36,  54,  245, 33,
        50,  238, 29,  47,  230, 28,  45,  222, 27,  43,  214, 26,  42,  206, 25,  40,  198, 24,  39,  189, 23,  37,  172,
        21,  34,  159, 19,  31,  148, 18,  29,  140, 17,  27,  132, 16,  26,  125, 15,  24,  33,  255, 11,  78,  69,  84,
        83,  67,  65,  80,  69,  50,  46,  48,  3,   1,   0,   0,   0,   33,  249, 4,   9,   40,  0,   0,   0,   44,  0,
        0,   0,   0,   15,  0,   20,  0,   0,   5,   80,  32,  32,  142, 100, 105, 158, 104, 74,  6,   3,   65,  28,  10,
        250, 54,  208, 211, 56,  209, 121, 60,  90,  167, 85,  145, 201, 165, 164, 120, 100, 62,  34,  137, 228, 178, 41,
        93,  56,  31,  36,  64,  65,  129, 170, 8,   140, 141, 84,  150, 192, 108, 79,  9,   198, 229, 123, 154, 100, 58,
        100, 149, 186, 80,  88,  80,  212, 140, 71,  69,  163, 206, 120, 162, 234, 188, 126, 207, 7,   132, 0,   0,   33,
        249, 4,   9,   40,  0,   0,   0,   44,  0,   0,   0,   0,   15,  0,   20,  0,   0,   5,   126, 32,  32,  2,   193,
        64,  16,  7,   50,  174, 129, 112, 40,  76,  204, 44,  204, 138, 46,  15,  117, 93,  149, 244, 60,  145, 209, 129,
        33,  185, 112, 62,  31,  77,  47,  50,  185, 0,   16,  138, 71,  101, 243, 25,  73,  34,  146, 138, 6,   48,  169,
        96,  52,  213, 81,  116, 130, 233, 136, 142, 200, 209, 0,   1,   49,  134, 87,  34,  151, 195, 210, 121, 195, 229,
        116, 251, 10,  95,  135, 11,  15,  13,  20,  84,  126, 35,  14,  77,  96,  132, 35,  90,  28,  30,  122, 137, 143,
        35,  5,   5,   8,   11,  20,  144, 8,   9,   12,  16,  25,  144, 12,  15,  18,  22,  27,  144, 16,  94,  26,  30,
        144, 25,  141, 105, 144, 144, 33,  0,   59
    };

}
