/////////////////////////////////////////////////////////////////////////////
// Purpose:   Hold data for currently loaded project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <wx/bitmap.h>

class Node;

class ProjectSettings
{
public:
    ProjectSettings();

    ttlib::cstr& getProjectFile() { return m_projectFile; }
    ttString GetProjectFile() { return ttString() << m_projectFile.wx_str(); }

    ttlib::cstr& SetProjectFile(const ttString& file);
    ttlib::cstr& setProjectFile(const ttlib::cstr& file);
    ttlib::cstr& SetProjectPath(const ttString& path, bool remove_filename = true);
    ttlib::cstr& setProjectPath(const ttlib::cstr& path, bool remove_filename = true);

    ttlib::cstr& getProjectPath() { return m_projectPath; }
    ttString GetProjectPath() { return ttString() << m_projectPath.wx_str(); }

    wxImage GetPropertyBitmap(const ttlib::cstr& description, bool want_scaled = true);

    wxImage GetImage(const ttlib::cstr& file);

    // Get image from HDR source file property
    wxImage GetHdrImage(const ttlib::cstr& description);

private:
    ttlib::cstr m_projectFile;
    ttlib::cstr m_projectPath;

    std::map<std::string, wxImage> m_images;
};
