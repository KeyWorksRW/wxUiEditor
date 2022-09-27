/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith or XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "import_wxsmith.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "project_class.h"   // Project class
#include "utils.h"           // Utility functions that work with properties

WxSmith::WxSmith() {}

bool WxSmith::Import(const ttString& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!ttlib::is_sameas(root.name(), "wxsmith", tt::CASE::either) &&
        !ttlib::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        wxMessageBox(wxString() << filename << " is not a wxSmith or XRC file", "Import");
        return false;
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);
        for (auto& iter: root.children())
        {
            CreateXrcNode(iter, m_project.get());
        }

        if (!m_project->GetChildCount())
        {
            wxMessageBox(wxString() << filename << " does not contain any top level forms.", "Import");
            return false;
        }

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox(wxString("This project file is invalid and cannot be loaded: ") << filename, "Import Project");
        return false;
    }

    if (m_errors.size())
    {
        ttlib::cstr errMsg("Not everything in the wxSmith project could be converted:\n\n");
        MSG_ERROR(ttlib::cstr() << "------  " << m_importProjectFile.filename().wx_str() << "------");
        for (auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg << iter << '\n';
        }

        wxMessageBox(errMsg, "Import wxSmith project");
    }

    return true;
}
