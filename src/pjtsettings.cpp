/////////////////////////////////////////////////////////////////////////////
// Purpose:   Hold data for currently loaded project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <filesystem>
#include <fstream>
#include <regex>

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
    static std::regex words_regex("\\[.+;.+\\]");

    ttlib::cstr desc_copy(description);

    // Convert "[num; num]" to "num, num" so that we can break the property string into multiple parts

    std::cmatch match;
    if (std::regex_search(description.c_str(), match, words_regex))
    {
        ttlib::cstr fix(match[0]);
        fix.Replace(";", ",");
        fix.Replace("[", "");
        fix.Replace("]", "");
        desc_copy.Replace(ttlib::cview(match[0]), fix);
    }

    ttlib::multistr parts(desc_copy, BMP_PROP_SEPARATOR);
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
        if (parts[IndexArtClient].empty())
            parts[IndexArtClient] = "wxART_OTHER";
        image = wxArtProvider::GetBitmap(parts[IndexArtID], wxART_MAKE_CLIENT_ID_FROM_STR(parts[IndexArtClient]))
                    .ConvertToImage();
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

    // cache it so that we don't need to read it from disk again
    if (!parts[IndexType].contains("Embed") && result == m_images.end())
        m_images[path] = image;

    // Scale if needed
    if (want_scaled && parts.size() > IndexConvert && parts[IndexSize].size() && parts[IndexSize] != "-1, -1")
    {
        auto scale_size = ConvertToSize(parts[IndexSize]);
        if (scale_size.x != -1 || scale_size.y != -1)
        {
            auto original_size = image.GetSize();
            if (scale_size.x != -1)
                original_size.x = scale_size.x;
            if (scale_size.y != -1)
                original_size.y = scale_size.y;

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
    if (!path.file_exists())
        return false;

    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

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
                auto& embed = m_embedded_images.emplace_back();
                embed.array_name = path.filename();
                embed.array_name.Replace(".", "_", true);
                m_map_embedded[path.filename().c_str()] = m_embedded_images.size() - 1;

                // At this point, other threads can lookup and add an embedded image, they just can't access the data of this
                // image until we're done.

                std::unique_lock<std::mutex> retrieve_lock(m_mutex_embed_retrieve);
                add_lock.unlock();

                wxMemoryOutputStream save_stream;
                auto mime_type = handler->GetMimeType();
                auto type = handler->GetType();

                // If possible, convert the file to a PNG -- even if the original file is a PNG, since we might end up with
                // better compression.

                if (isConvertibleMime(mime_type))
                {
                    // Maximize compression
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
                    image.SaveFile(save_stream, wxBITMAP_TYPE_PNG);
                }
                else
                {
                    image.SaveFile(save_stream, type);
                }

                auto read_stream = save_stream.GetOutputStreamBuffer();

                embed.form = form;
                embed.array_size = read_stream->GetBufferSize();
                embed.array_data = std::make_unique<unsigned char[]>(embed.array_size);
                memcpy(embed.array_data.get(), read_stream->GetBufferStart(), embed.array_size);

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
        return &m_embedded_images[result->second];
    }
    else
    {
        return nullptr;
    }
}
