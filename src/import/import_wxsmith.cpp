/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith or XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <format>

#include "import_wxsmith.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "dlg_msgs.h"        // wxMessageDialog dialogs
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "utils.h"           // Utility functions that work with properties

WxSmith::WxSmith() {}

bool WxSmith::Import(const std::string& filename, bool write_doc)
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

    // Using a try block means that if at any point it becomes obvious the project file is invalid
    // and we cannot recover, then we can throw an error and give a standard response about an
    // invalid file.

    try
    {
        m_project = NodeCreation.CreateNode(gen_Project, nullptr).first;
        for (auto& iter: root.children())
        {
            CreateXrcNode(iter, m_project.get());
        }

        if (!m_project->get_ChildCount())
        {
            wxMessageBox(wxString() << wxString::FromUTF8(filename)
                                    << " does not contain any top level forms.",
                         "Import");
            return false;
        }

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    catch (const std::exception& err)
    {
        MSG_ERROR(err.what());
        dlgImportError(err, filename, "Import Project");
        return false;
    }

    if (m_errors.size())
    {
        std::string errMsg("Not everything in the project could be converted:\n\n");
        MSG_ERROR(std::format("------  {}------",
                              std::filesystem::path(m_importProjectFile).filename().string()));
        for (auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg += iter;
            errMsg += '\n';
        }

        wxMessageDialog dlg(nullptr, wxString::FromUTF8(errMsg), "Import Project",
                            wxICON_WARNING | wxOK);
        dlg.ShowModal();
    }

    return true;
}

bool WxSmith::HandleUnknownProperty(const pugi::xml_node& xml_obj, Node* node, Node* /* parent */)
{
    auto node_name = xml_obj.name();

    if (node_name == "id_arg" || node_name == "pos_arg" || node_name == "size_arg")
    {
        return true;
    }
    else if (node_name == "labelrowheight")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            node->set_value(prop_default_row_size, xml_obj.text().as_int());
            return true;
        }
    }
    else if (node_name == "labelcolwidth")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            // wxGrid doesn't have a default column width, so we'll ignore this property
            return true;
        }
    }
    else if (node_name == "defaultcolsize")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            node->set_value(prop_default_col_size, xml_obj.text().as_int());
            return true;
        }
    }
    else if (node_name == "collabels")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            tt_string choices;
            for (auto& iter: xml_obj.children())
            {
                if (iter.name() == "item")
                {
                    auto child = iter.child_as_cstr();
                    child.Replace("\"", "\\\"", true);
                    if (choices.size())
                        choices << " ";
                    choices << '\"' << child << '\"';
                }
            }

            if (choices.size())
                node->set_value(prop_col_label_values, choices);
            return true;
        }
    }
    else if (node_name == "col" && node->is_Gen(gen_gbsizeritem))
    {
        node->set_value(prop_column, xml_obj.text().as_int());
        return true;
    }
    else if (node_name == "val")
    {
        node->set_value(prop_validator_variable, xml_obj.text().as_sview());
        return true;
    }
    return false;
}
