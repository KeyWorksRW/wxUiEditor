/////////////////////////////////////////////////////////////////////////////
// Purpose:   Hold data for currently loaded project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
#include "ttsview.h"     // sview -- std::string_view with additional methods

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

inline ttlib::cstr ConvertToLookup(const ttlib::cstr& description)
{
    ttlib::multistr parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
    ASSERT(parts.size() > 1)

    ttlib::cstr lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();
    return lookup_str;
}

namespace wxue_img
{
    extern const unsigned char pulsing_unknown_gif[377];
}

ProjectSettings::ProjectSettings() {}

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

wxImage ProjectSettings::GetPropertyBitmap(const ttlib::cstr& description, bool check_image)
{
    ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty())
    {
        return GetInternalImage("unknown");
    }

    wxImage image;

    ttlib::cstr path = parts[IndexImage];

    auto result = m_images.find(path);
    if (result != m_images.end())
    {
        image = result->second;
    }
    else if (parts[IndexType].contains("Art"))
    {
        if (parts[IndexArtID].contains("|"))
        {
            ttlib::multistr id_client(parts[IndexArtID], '|');
            image = (wxArtProvider::GetBitmapBundle(id_client[0], wxART_MAKE_CLIENT_ID_FROM_STR(id_client[1]))
                         .GetBitmapFor(wxGetFrame().GetWindow()))
                        .ConvertToImage();
        }
        else
        {
            image = (wxArtProvider::GetBitmapBundle(parts[IndexArtID].wx_str(), wxART_MAKE_CLIENT_ID_FROM_STR("wxART_OTHER"))
                         .GetBitmapFor(wxGetFrame().GetWindow()))
                        .ConvertToImage();
        }
    }
    else if (parts[IndexType].contains("Embed"))
    {
        if (!path.file_exists())
        {
            path = wxGetApp().GetProjectPtr()->prop_as_string(prop_art_directory);
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
            path = wxGetApp().GetProjectPtr()->prop_as_string(prop_art_directory);
            path.append_filename(parts[IndexImage]);

            if (result = m_images.find(path); result != m_images.end())
            {
                image = result->second;
            }
        }

        if (!image.IsOk())
        {
            if (path.has_extension(".h_img") || path.has_extension(".h"))
            {
                image = GetHeaderImage(path);
            }
            else
            {
                // Note that this will load an XPM file
                image.LoadFile(path);
            }
        }
    }

    if (!image.IsOk())
    {
        return (check_image ? GetInternalImage("unknown") : image);
    }

    // If it's not embedded, then cache it so that we don't read it from disk again
    if (!parts[IndexType].contains("Embed") && result == m_images.end())
        m_images[path] = image;

    return image;
}

void ProjectSettings::UpdateBundle(const ttlib::cstr& description, Node* node)
{
    ttlib::multiview parts(description, ';', tt::TRIM::both);
    if (parts.size() < 2)
        return;

    ttlib::cstr lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    auto result = m_bundles.find(lookup_str);
    if (result == m_bundles.end())
    {
        ProcessBundleProperty(description, node);
        result = m_bundles.find(lookup_str);
    }

    if (result != m_bundles.end() && result->second.lst_filenames.size())
    {
        auto form = node->GetForm();
        for (auto& iter: result->second.lst_filenames)
        {
            if (auto embed = GetEmbeddedImage(iter); embed)
            {
                if (embed->form != form)
                {
                    // This will happen when a bundle bitmap is added to the Images generator. The initial bitmap will be
                    // correctly changed to use the new form, but we also need to process  all the sub images as well

                    if (form->isGen(gen_Images))
                    {
                        embed->form = form;
                    }
                }
                if (parts[IndexType].is_sameprefix("SVG") && parts.size() > 2)
                {
                    wxSize new_size;
                    GetSizeInfo(new_size, parts[IndexSize]);
                    if (embed->size_x != new_size.x || embed->size_y != new_size.y)
                    {
                        embed->size_x = new_size.x;
                        embed->size_y = new_size.y;
                        result->second.bundle = LoadSVG(embed);
                    }
                }
            }
        }
    }
}

wxBitmapBundle ProjectSettings::GetPropertyBitmapBundle(const ttlib::cstr& description, Node* node)
{
    ttlib::multiview parts(description, ';', tt::TRIM::both);
    if (parts.size() < 2)
    {
        return GetInternalImage("unknown");
    }

    ttlib::cstr lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    if (auto result = m_bundles.find(ConvertToLookup(description)); result != m_bundles.end())
    {
        return result->second.bundle;
    }

    if (auto result = ProcessBundleProperty(description, node); result)
    {
        return result->bundle;
    }

    return GetInternalImage("unknown");
}

const ImageBundle* ProjectSettings::GetPropertyImageBundle(const ttlib::cstr& description, Node* node)
{
    ttlib::multiview parts(description, ';', tt::TRIM::both);
    if (parts.size() < 2)
    {
        return nullptr;
    }

    ttlib::cstr lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    if (auto result = m_bundles.find(lookup_str); result != m_bundles.end())
    {
        return &result->second;
    }
    else if (node)
    {
        return ProcessBundleProperty(description, node);
    }
    else
    {
        return nullptr;
    }
}

wxAnimation ProjectSettings::GetPropertyAnimation(const ttlib::cstr& description)
{
    ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    wxAnimation image;

    if (parts.size() <= IndexImage || parts[IndexImage].empty())
    {
        return GetAnimFromHdr(wxue_img::pulsing_unknown_gif, sizeof(wxue_img::pulsing_unknown_gif));
    }

    ttlib::cstr path = parts[IndexImage];
    if (!path.file_exists())
    {
        path = wxGetApp().GetProjectPtr()->prop_as_string(prop_art_directory);
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

bool ProjectSettings::AddEmbeddedImage(ttlib::cstr path, Node* form, bool is_animation)
{
    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

    if (!path.file_exists())
    {
        if (wxGetApp().GetProject()->HasValue(prop_art_directory))
        {
            ttlib::cstr art_path = wxGetApp().GetProject()->prop_as_string(prop_art_directory);
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

    auto final_result = AddNewEmbeddedImage(path, form, add_lock);
    if (is_animation || !final_result)
        return final_result;

    // Note that path may now contain the prop_art_directory prefix

    add_lock.lock();

    if (auto pos = path.find_last_of('.'); ttlib::is_found(pos))
    {
        if (path.contains("_16x16."))
        {
            path.Replace("_16x16.", "_24x24.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
            path.Replace("_24x24.", "_32x32.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
        }
        else if (path.contains("_24x24."))
        {
            path.Replace("_24x24.", "_36x36.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
            path.Replace("_36x36.", "_48x48.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
        }
        else
        {
            path.insert(pos, "_1_5x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
            path.Replace("_1_5x", "_2x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
        }
    }

    return final_result;
}

bool ProjectSettings::AddNewEmbeddedImage(ttlib::cstr path, Node* form, std::unique_lock<std::mutex>& add_lock)
{
    wxFFileInputStream stream(path.wx_str());
    if (!stream.IsOk())
    {
        add_lock.unlock();
        return false;
    }

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
                m_map_embedded[path.filename().c_str()] = std::make_unique<EmbeddedImage>();
                auto embed = m_map_embedded[path.filename().c_str()].get();
                InitializeArrayName(embed, path.filename());
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
                    image.SaveFile(save_stream, "image/png");

                    auto read_stream = save_stream.GetOutputStreamBuffer();
                    stream.SeekI(0);
                    if (read_stream->GetBufferSize() <= static_cast<size_t>(stream.GetLength()))
                    {
                        embed->array_size = read_stream->GetBufferSize();
                        embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                        memcpy(embed->array_data.get(), read_stream->GetBufferStart(), embed->array_size);
                    }
                    else
                    {
#if defined(_DEBUG)
                        auto org_size = static_cast<size_t>(stream.GetLength());
                        auto png_size = read_stream->GetBufferSize();
                        ttlib::cstr size_comparison;
                        size_comparison.Format("Original: %ku, new: %ku", org_size, png_size);
#endif  // _DEBUG

                        embed->type = handler->GetType();
                        embed->array_size = stream.GetSize();
                        embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                        stream.Read(embed->array_data.get(), embed->array_size);
                    }
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

    add_lock.unlock();
    return false;
}

void ProjectSettings::InitializeArrayName(EmbeddedImage* embed, ttlib::sview filename)
{
    embed->array_name = filename;
    for (size_t idx = 0; idx < embed->array_name.size(); ++idx)
    {
        if (ttlib::is_alnum(embed->array_name[idx]) || embed->array_name[idx] == '_')
        {
            continue;
        }
        embed->array_name[idx] = '_';
    }
}

EmbeddedImage* ProjectSettings::GetEmbeddedImage(ttlib::sview path)
{
    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

    if (auto result = m_map_embedded.find(path.filename()); result != m_map_embedded.end())
    {
        std::unique_lock<std::mutex> retrieve_lock(m_mutex_embed_retrieve);
        return result->second.get();
    }
    else
    {
        return nullptr;
    }
}

bool ProjectSettings::UpdateEmbedNodes()
{
    bool is_changed = false;
    auto project = wxGetApp().GetProject();

    for (size_t idx_form = 0; idx_form < project->GetChildCount(); ++idx_form)
    {
        if (CheckNode(project->GetChild(idx_form)))
            is_changed = true;
    }
    return is_changed;
}

// REVIEW: [KeyWorks - 04-07-2022] We should eliminate this call if possible -- ProjectSettings::CollectBundles() processed
// all nodes initially, and the only reason this would be needed is if adding or changing a bitmap property did not get set
// up correctly (highly unlikely).

bool ProjectSettings::CheckNode(Node* node)
{
    bool is_changed = false;

    Node* node_form = node->IsForm() ? node : node->FindParentForm();

    auto node_position = wxGetApp().GetProject()->GetChildPosition(node_form);

    for (auto& iter: node->get_props_vector())
    {
        if ((iter.type() == type_image || iter.type() == type_animation) && iter.HasValue())
        {
            ttlib::multiview parts(iter.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts[IndexType] != "Embed")
                continue;

            auto result = m_map_embedded.find(parts[IndexImage].filename());
            // If it hasn't been added yet, add it now
            if (result == m_map_embedded.end())
            {
                FAIL_MSG("We get here if a bitmap did not get added to m_map_embedded -- that shouldn't happen")
                AddEmbeddedImage(parts[IndexImage], node_form);
                continue;
            }

            auto embed = result->second.get();
            ASSERT(embed)

            if (node_form->isGen(gen_Images))
            {
                if (embed->form != node_form)
                {
                    FAIL_MSG("Can we fix this without walking through every form?")
                    embed->form = node_form;
                    is_changed = true;
                }
            }
            else
            {
                auto child_pos = wxGetApp().GetProject()->GetChildPosition(embed->form);
                if (child_pos > node_position)
                {
                    FAIL_MSG("If this is valid, we need to document why.")
                    embed->form = node_form;
                    is_changed = true;
                }
            }
        }
    }

    for (size_t idx_child = 0; idx_child < node->GetChildCount(); idx_child++)
    {
        if (CheckNode(node->GetChild(idx_child)))
            is_changed = true;
    }

    return is_changed;
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
