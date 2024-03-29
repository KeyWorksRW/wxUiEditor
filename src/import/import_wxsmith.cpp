/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith or XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "import_wxsmith.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "dlg_msgs.h"        // wxMessageDialog dialogs
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "utils.h"           // Utility functions that work with properties

WxSmith::WxSmith() {}

bool WxSmith::Import(const tt_string& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!tt::is_sameas(root.name(), "wxsmith", tt::CASE::either) &&
        !tt::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        dlgInvalidProject(filename, "wxSmith or XRC", "Import project");
        return false;
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        m_project = NodeCreation.createNode(gen_Project, nullptr);
        for (auto& iter: root.children())
        {
            CreateXrcNode(iter, m_project.get());
        }

        if (!m_project->getChildCount())
        {
            wxMessageBox(wxString() << filename.make_wxString() << " does not contain any top level forms.", "Import");
            return false;
        }

        if (write_doc)
            m_project->createDoc(m_docOut);
    }

    catch (const std::exception& err)
    {
        MSG_ERROR(err.what());
        dlgImportError(err, filename, "Import Project");
        return false;
    }

    if (m_errors.size())
    {
        tt_string errMsg("Not everything in the project could be converted:\n\n");
        MSG_ERROR(tt_string() << "------  " << m_importProjectFile.filename() << "------");
        for (auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg << iter << '\n';
        }

        wxMessageDialog dlg(nullptr, errMsg, "Import Project", wxICON_WARNING | wxOK);
        dlg.ShowModal();
    }

    return true;
}
