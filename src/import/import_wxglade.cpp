/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a WxGlade file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "import_WxGlade.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "mainapp.h"         // App -- Main application class
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "pjtsettings.h"     // ProjectSettings -- Hold data for currently loaded project
#include "uifuncs.h"         // Miscellaneous functions for displaying UI
#include "utils.h"           // Utility functions that work with properties

WxGlade::WxGlade() {}

bool WxGlade::Import(const ttString& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!ttlib::is_sameas(root.name(), "application", tt::CASE::either) )
    {
        appMsgBox(filename.wx_str() + _ttc(" is not a wxGlade file"), _tt("Import"));
        return false;
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);
        for (auto& iter: root.children())
        {
            CreateGladeNode(iter, m_project.get());
        }

        if (!m_project->GetChildCount())
        {
            appMsgBox(filename.wx_str() + _ttc(" does not contain any top level forms."), _tt("Import"));
            return false;
        }

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    catch (const std::exception& DBG_PARAM(e))
    {
        MSG_ERROR(e.what());
        appMsgBox(ttlib::cstr("This project file is invalid and cannot be loaded: ") << filename.wx_str(), "Import Project");
        return false;
    }

    return true;
}

NodeSharedPtr WxGlade::CreateGladeNode(pugi::xml_node& /* xml_obj */, Node* /* parent */, Node* /* sizeritem */)
{
    return nullptr;
}
