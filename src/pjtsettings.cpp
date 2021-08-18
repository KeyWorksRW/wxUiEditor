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
                    // Maximize compression
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
                    embed->type = wxBITMAP_TYPE_PNG;
                }

                wxMemoryOutputStream save_stream;
                image.SaveFile(save_stream, embed->type);
                auto read_stream = save_stream.GetOutputStreamBuffer();

                embed->type = handler->GetType();
                embed->array_size = read_stream->GetBufferSize();
                embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                memcpy(embed->array_data.get(), read_stream->GetBufferStart(), embed->array_size);

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
