/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <set>

#include <wx/mstream.h>  // Memory stream classes

#include "json.h"        // json parser
#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "import_wxcrafter.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "mainapp.h"         // App -- Main application class
#include "mainframe.h"       // Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "pjtsettings.h"     // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"           // Utility functions that work with properties

WxCrafter::WxCrafter() {}

bool WxCrafter::Import(const ttString& filename, bool /* write_doc */)
{
    std::ifstream istrm(filename.wx_str(), std::ifstream::binary);
    if (!istrm.is_open())
    {
        wxMessageBox(wxString() << "Cannot open " << filename, "Import wxCrafter project");
        return false;
    }

    Json::Value json_doc;
    istrm >> json_doc;

    if (!json_doc.isObject())
    {
        wxMessageBox(wxString() << filename << " is not a valid wxCrafter file", "Import wxCrafter project");
        return false;
    }

    m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);

    if (auto& metadata = json_doc["metadata"]; !metadata.isNull())
    {
        m_generate_ids = GetBoolValue(metadata, "m_useEnum", true);

        if (auto& include_files = metadata["m_includeFiles"]; !include_files.isNull())
        {
            if (include_files.isArray() && include_files.size() > 0)
            {
                // TODO: [KeyWorks - 12-16-2021]
            }
        }
    }

    if (auto& windows = json_doc["windows"]; !windows.isNull())
    {
        auto members = windows.GetMemberMap();
        if (auto result = members.find("m_type"); result != members.end())
        {
            auto& value = windows[result->first];
            if (!value.isNumeric())
            {
                wxMessageBox(
                    wxString()
                        << filename
                        << " is not a recognized wxCrafter file. It may be a newer version that wxUiEditor doesn't support.",
                    "Import wxCrafter project");
                return false;
            }
        }
    }

    return false;
}

bool WxCrafter::GetBoolValue(Json::Value& value, ttlib::cview name, bool def_return)
{
    if (auto& result = value[name.c_str()]; !result.isNull())
    {
        return result.asBool();
    }
    else
    {
        return def_return;
    }
}
