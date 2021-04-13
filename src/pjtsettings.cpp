/////////////////////////////////////////////////////////////////////////////
// Purpose:   Hold data for currently loaded project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <filesystem>
#include <fstream>
#include <regex>

#include <wx/artprov.h>   // wxArtProvider class
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/filesys.h>   // class for opening files - virtual file system

#include <ttcview.h>     // cview -- string_view functionality on a zero-terminated char string.
#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "pjtsettings.h"  // ProjectSettings

#include "bitmaps.h"  // Map of bitmaps accessed by name
#include "mainapp.h"  // App -- App class
#include "node.h"     // Node class
#include "utils.h"    // Utility functions that work with properties

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

// The purpose of this function is to initially create a wximage and then store it so that the next time we need it, we
// simply retrieve it from our map. Given that both the Mockup and Properties panel request the same image frequently,
// this avoids the situation where an image is read from a file multiple times. The map of images persists for as long
// as the project is loaded.

wxImage ProjectSettings::GetImage(const ttlib::cstr& filename)
{
    // If the filename was specified as part of a HDR source, then it may already have been stored.
    if (auto result = m_images.find(filename); result != m_images.end())
    {
        return result->second;
    }

    ttlib::cstr path;
    ttlib::cstr client;

    if (auto pos = filename.find(BMP_PROP_SEPARATOR); ttlib::is_found(pos))
    {
        auto end = path.AssignSubString(filename.subview(pos), BMP_PROP_SEPARATOR, BMP_PROP_SEPARATOR);
        path.trim(tt::TRIM::both);
        if (end + pos < filename.size())
        {
            client = filename.view_nonspace(pos + end + 1);
            client.trim();
            if (client.size())
            {
                path << BMP_PROP_SEPARATOR << client;
            }
        }
    }
    else
    {
        path = filename;
    }

    if (auto result = m_images.find(path); result != m_images.end())
    {
        return result->second;
    }

    if (path.empty())
    {
        return GetXPMImage("unknown");
    }

    if (filename.is_sameprefix("Art Provider"))
    {
        auto id = path;
        id.erase_from(BMP_PROP_SEPARATOR);
        auto bmp = wxArtProvider::GetBitmap(id, client);
        if (bmp.IsOk())
        {
            m_images[path] = bmp.ConvertToImage();
            return m_images[path];
        }
        else
        {
            m_images[path] = GetXPMImage("unknown");
            return m_images[path];
        }
    }

    wxFileSystem system;
    system.ChangePathTo(m_projectPath.wx_str(), true);

    auto fsfile = system.OpenFile(path.wx_str(), wxFS_READ | wxFS_SEEKABLE);
    if (!fsfile)
    {
        m_images[path] = GetXPMImage("unknown");
        return m_images[path];
    }

    wxImage img(*(fsfile->GetStream()));
    delete fsfile;

    if (!img.Ok())
    {
        // We do NOT want to keep trying to load this file because GetImage() will be called frequently in Mock Up,
        // properties, etc. To prevent rereading, we store it as a failed image.

        // TODO: [KeyWorks - 08-30-2020] We need a different image for this to indicate the file can't be found versus just
        // unknown;
        m_images[path] = GetXPMImage("unknown");
    }
    else
    {
        m_images[path] = img;
    }

    return m_images[path];
}

wxImage ProjectSettings::GetHdrImage(const ttlib::cstr& description)
{
    static std::regex words_regex("\\[.+;.+\\]");

    ttlib::cstr desc_copy(description);

    // Convert "[num; num]" to "num, num"

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

    if (parts[1].empty())
    {
        return GetXPMImage("unknown");
    }

    if (parts.size() < 3)
        parts.emplace_back("none");

    wxImage image;

    if (auto result = m_images.find(parts[1]); result != m_images.end())
    {
        image = result->second;
    }
    else
    {
        if (parts[1].has_extension(".h") || parts[1].has_extension(".hpp") || parts[1].has_extension(".hh") ||
            parts[1].has_extension(".hxx"))
        {
            image = GetHeaderImage(parts[1]);
        }
        else
        {
            image.LoadFile(parts[1]);
        }

        if (image.IsOk())
        {
            m_images[parts[1]] = image;
        }
    }

    // Scale if needed
    if (parts.size() > 2 && parts[2].size() && parts[2] != "-1, -1")
    {
        auto scale_size = ConvertToSize(parts[2]);
        if (scale_size.x != -1 || scale_size.y != -1)
        {
            auto original_size = image.GetSize();
            if (scale_size.x != -1)
                original_size.x = scale_size.x;
            if (scale_size.y != -1)
                original_size.y = scale_size.y;

            // We don't want to scale the original image since it might be used elsewhere, so we create a scaled copy.
            // We append the scaling information and store the image so that the image only has to be scaled once.

            std::string name(parts[1] + parts[2]);
            if (auto result = m_images.find(name); result != m_images.end())
            {
                return result->second;
            }

            auto newImage = image.Scale(original_size.x, original_size.y,
                                        image.HasMask() ? wxIMAGE_QUALITY_NORMAL : wxIMAGE_QUALITY_HIGH);
            m_images[name] = newImage;
            return newImage;
        }
    }

    return image;
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
        return GetXPMImage("unknown");
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
            if (path.has_extension(".h") || path.has_extension(".hpp") || path.has_extension(".hh") ||
                path.has_extension(".hxx"))
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
        return GetXPMImage("unknown");
    }

    // cache it so that we don't need to read it from disk again
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
