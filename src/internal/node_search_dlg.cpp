
/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node search dialog implementation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include "node_search_dlg.h"
#include <tuple>  // for std::ignore

#include "../panels/nav_panel.h"
#include "mainframe.h"
#include "node.h"
#include "project_handler.h"
#include "unused_gen_dlg.h"
#include "utils.h"

NodeSearchDlg::NodeSearchDlg() {}

NodeSearchDlg::NodeSearchDlg(wxWindow* parent)
{
    std::ignore = Create(parent);
}

Node* FindNodeByGenerator(Node* node, GenEnum::GenName get_GenName)
{
    if (node->is_Gen(get_GenName))
    {
        return node;
    }

    for (auto& child: node->get_ChildNodePtrs())
    {
        Node* result = FindNodeByGenerator(child.get(), get_GenName);
        if (result)
        {
            return result;
        }
    }

    return nullptr;
}

void MainFrame::OnFindWidget(wxCommandEvent& /* event unused */)
{
    NodeSearchDlg search_dlg(this);
    if (search_dlg.ShowModal() == wxID_OK && search_dlg.GetForm())
    {
        // Recursively search for the first Node* containing a property with a specific value.
        auto FindNodeByProp = [&](Node* node, GenEnum::PropName prop, const std::string& label_name,
                                  auto&& FindNodeByProp) -> Node*
        {
            if (node->HasValue(prop) && node->as_string(prop) == label_name)
            {
                return node;
            }

            for (auto& child: node->get_ChildNodePtrs())
            {
                Node* result = FindNodeByProp(child.get(), prop, label_name, FindNodeByProp);
                if (result)
                {
                    return result;
                }
            }

            return nullptr;
        };

        if (search_dlg.isSearchGenerators())
        {
            if (!rmap_GenNames.contains(search_dlg.GetNameChoice()))
            {
                wxMessageBox(wxString() << "Unknown generator " << search_dlg.GetNameChoice());
                return;
            }

            Node* node = FindNodeByGenerator(search_dlg.GetForm(),
                                             rmap_GenNames.at(search_dlg.GetNameChoice()));
            if (node)
            {
                SelectNode(node);
                m_nav_panel->SetFocus();
            }
            else
            {
                wxMessageBox(wxString() << "Unable to find " << search_dlg.GetName());
            }
        }
        else if (search_dlg.isSearchVarnames())
        {
            Node* node = FindNodeByProp(search_dlg.GetForm(), prop_var_name,
                                        search_dlg.GetNameChoice(), FindNodeByProp);
            if (node)
            {
                SelectNode(node);
                m_nav_panel->SetFocus();
            }
            else
            {
                wxMessageBox(wxString() << "Unable to find " << search_dlg.GetNameChoice());
            }
        }
        else if (search_dlg.isSearchLabels())
        {
            Node* node = FindNodeByProp(search_dlg.GetForm(), prop_label,
                                        search_dlg.GetNameChoice(), FindNodeByProp);
            if (node)
            {
                SelectNode(node);
                m_nav_panel->SetFocus();
            }
            else
            {
                wxMessageBox(wxString() << "Unable to find " << search_dlg.GetNameChoice());
            }
        }
        else if (search_dlg.isSearchIDs())
        {
            Node* node = FindNodeByProp(search_dlg.GetForm(), prop_id, search_dlg.GetNameChoice(),
                                        FindNodeByProp);
            if (node)
            {
                SelectNode(node);
                m_nav_panel->SetFocus();
            }
            else
            {
                wxMessageBox(wxString() << "Unable to find " << search_dlg.GetNameChoice());
            }
        }
        else
        {
            wxMessageBox("No selected node, or search criteria selected");
        }
    }
}

void NodeSearchDlg::FindGenerators(Node* node)
{
    if (node->is_Gen(gen_Images) || node->is_Gen(gen_Data))
    {
        return;
    }

    if (!node->is_Gen(gen_folder) && !node->is_Gen(gen_sub_folder))
    {
        // A GenName with no display name has nothing to search on, so skip the node.
        if (map_GenNames.contains(node->get_GenName()))
        {
            const std::string gen_display_name { map_GenNames.at(node->get_GenName()) };
            if (!m_map_found.contains(gen_display_name))
            {
                std::set<Node*> list;
                if (!node->is_Form())
                {
                    list.emplace(node->get_Form());
                }
                else if (const Node* parent = node->get_Parent())
                {
                    if (parent->is_Gen(gen_folder) || parent->is_Gen(gen_sub_folder))
                    {
                        list.emplace(node->get_Form());
                    }
                    else
                    {
                        list.emplace(Project.get_ProjectNode());
                    }
                }
                else
                {
                    list.emplace(Project.get_ProjectNode());
                }
                m_map_found[gen_display_name] = list;
            }
            else if (!node->is_Form())
            {
                std::set<Node*>& list = m_map_found.at(gen_display_name);
                list.emplace(node->get_Form());
            }
        }
    }

    if (node->get_ChildCount())
    {
        for (auto& child: node->get_ChildNodePtrs())
        {
            FindGenerators(child.get());
        }
    }
}

void NodeSearchDlg::FindVariables(Node* node)
{
    if (node->is_Gen(gen_Images) || node->is_Gen(gen_Data))
    {
        return;
    }

    if (node->HasProp(prop_var_name) && node->HasValue(prop_var_name) &&
        !node->is_Gen(gen_folder) && !node->is_Gen(gen_sub_folder))
    {
        if (!m_map_found.contains(node->as_string(prop_var_name)))
        {
            std::set<Node*> form_list;
            if (!node->is_Form())
            {
                form_list.emplace(node->get_Form());
            }
            else if (const Node* parent = node->get_Parent())
            {
                if (parent->is_Gen(gen_folder) || parent->is_Gen(gen_sub_folder))
                {
                    form_list.emplace(node->get_Form());
                }
                else
                {
                    form_list.emplace(Project.get_ProjectNode());
                }
            }
            else
            {
                form_list.emplace(Project.get_ProjectNode());
            }
            m_map_found[node->as_string(prop_var_name)] = form_list;
        }
        else if (!node->is_Form())
        {
            std::set<Node*>& form_list = m_map_found.at(node->as_string(prop_var_name));
            form_list.emplace(node->get_Form());
        }
    }

    if (node->get_ChildCount())
    {
        for (auto& child: node->get_ChildNodePtrs())
        {
            FindVariables(child.get());
        }
    }
}

void NodeSearchDlg::FindLabels(Node* node)
{
    if (node->is_Gen(gen_Images) || node->is_Gen(gen_Data))
    {
        return;
    }

    if (node->HasProp(prop_label) && node->HasValue(prop_label) && !node->is_Gen(gen_folder) &&
        !node->is_Gen(gen_sub_folder))
    {
        if (!m_map_found.contains(node->as_string(prop_label)))
        {
            std::set<Node*> form_list;
            if (!node->is_Form())
            {
                form_list.emplace(node->get_Form());
            }
            else if (const Node* parent = node->get_Parent())
            {
                if (parent->is_Gen(gen_folder) || parent->is_Gen(gen_sub_folder))
                {
                    form_list.emplace(node->get_Form());
                }
                else
                {
                    form_list.emplace(Project.get_ProjectNode());
                }
            }
            else
            {
                form_list.emplace(Project.get_ProjectNode());
            }
            m_map_found[node->as_string(prop_label)] = form_list;
        }
        else if (!node->is_Form())
        {
            std::set<Node*>& form_list = m_map_found.at(node->as_string(prop_label));
            form_list.emplace(node->get_Form());
        }
    }

    if (node->get_ChildCount())
    {
        for (auto& child: node->get_ChildNodePtrs())
        {
            FindLabels(child.get());
        }
    }
}

void NodeSearchDlg::OnGenerators(wxCommandEvent& /* event unused */)
{
    m_map_found.clear();
    m_listbox->Clear();
    m_listbox_forms->Clear();

    if (auto cur_sel = wxGetFrame().getSelectedNode(); cur_sel)
    {
        if (cur_sel->is_Gen(gen_Project) || cur_sel->is_Gen(gen_folder) ||
            cur_sel->is_Gen(gen_sub_folder))
        {
            for (auto& child: cur_sel->get_ChildNodePtrs())
            {
                FindGenerators(child.get());
            }
        }
        else
        {
            FindGenerators(wxGetFrame().getSelectedNode());
        }
        for (auto& iter: m_map_found)
        {
            m_listbox->Append(iter.first);
        }
    }
}

void NodeSearchDlg::OnVariables(wxCommandEvent& /* event unused */)
{
    m_map_found.clear();
    m_listbox->Clear();
    m_listbox_forms->Clear();

    if (auto cur_sel = wxGetFrame().getSelectedNode(); cur_sel)
    {
        if (cur_sel->is_Gen(gen_Project) || cur_sel->is_Gen(gen_folder) ||
            cur_sel->is_Gen(gen_sub_folder))
        {
            for (auto& child: cur_sel->get_ChildNodePtrs())
            {
                FindVariables(child.get());
            }
        }
        else
        {
            FindVariables(wxGetFrame().getSelectedNode());
        }
        for (auto& iter: m_map_found)
        {
            m_listbox->Append(iter.first);
        }
    }
}

void NodeSearchDlg::OnLabels(wxCommandEvent& /* event unused */)
{
    m_map_found.clear();
    m_listbox->Clear();
    m_listbox_forms->Clear();

    if (auto cur_sel = wxGetFrame().getSelectedNode(); cur_sel)
    {
        if (cur_sel->is_Gen(gen_Project) || cur_sel->is_Gen(gen_folder) ||
            cur_sel->is_Gen(gen_sub_folder))
        {
            for (auto& child: cur_sel->get_ChildNodePtrs())
            {
                FindLabels(child.get());
            }
        }
        else
        {
            FindLabels(wxGetFrame().getSelectedNode());
        }
        for (auto& iter: m_map_found)
        {
            m_listbox->Append(iter.first);
        }
    }
}

void NodeSearchDlg::OnIDs(wxCommandEvent& /* event unused */)
{
    m_map_found.clear();
    m_listbox->Clear();
    m_listbox_forms->Clear();

    auto FindIDs = [&](Node* node, auto&& FindIDs) -> void
    {
        if (node->is_Gen(gen_Images) || node->is_Gen(gen_Data))
        {
            return;
        }

        if (!node->is_NonWidget() && node->HasProp(prop_id) && node->HasValue(prop_id) &&
            !node->as_string(prop_id).starts_with("wxID_ANY"))
        {
            if (!m_map_found.contains(node->as_string(prop_id)))
            {
                std::set<Node*> form_list;
                if (!node->is_Form())
                {
                    form_list.emplace(node->get_Form());
                }
                else if (const Node* parent = node->get_Parent())
                {
                    if (parent->is_Gen(gen_folder) || parent->is_Gen(gen_sub_folder))
                    {
                        form_list.emplace(node->get_Form());
                    }
                    else
                    {
                        form_list.emplace(Project.get_ProjectNode());
                    }
                }
                else
                {
                    form_list.emplace(Project.get_ProjectNode());
                }
                m_map_found[node->as_string(prop_id)] = form_list;
            }
            else if (!node->is_Form())
            {
                std::set<Node*>& form_list = m_map_found.at(node->as_string(prop_id));
                form_list.emplace(node->get_Form());
            }
        }

        if (node->get_ChildCount())
        {
            for (auto& child: node->get_ChildNodePtrs())
            {
                FindIDs(child.get(), FindIDs);
            }
        }
    };

    if (auto cur_sel = wxGetFrame().getSelectedNode(); cur_sel)
    {
        if (cur_sel->is_Gen(gen_Project) || cur_sel->is_Gen(gen_folder) ||
            cur_sel->is_Gen(gen_sub_folder))
        {
            for (auto& child: cur_sel->get_ChildNodePtrs())
            {
                FindIDs(child.get(), FindIDs);
            }
        }
        else
        {
            FindIDs(wxGetFrame().getSelectedNode(), FindIDs);
        }
        for (auto& iter: m_map_found)
        {
            m_listbox->Append(iter.first);
        }
    }
}

void NodeSearchDlg::OnInit(wxInitDialogEvent& event)
{
    wxCommandEvent dummy;
    OnGenerators(dummy);

    m_text_search->SetFocus();

    event.Skip();
}

void NodeSearchDlg::OnOK(wxCommandEvent& event)
{
    if (m_listbox->GetCount() > 0)
    {
        m_name = m_listbox->GetStringSelection().utf8_string();
        if (m_listbox_forms->GetCount() > 0)
        {
            if (std::string class_name = m_listbox_forms->GetStringSelection().utf8_string();
                !class_name.empty())
            {
                m_form = FindNodeByClassName(Project.get_ProjectNode(), class_name);
            }
            else
            {
                // Top-level form: the forms list has no class name, so search the whole project
                // rather than silently searching only the currently selected subtree.
                m_form = Project.get_ProjectNode();
            }
        }
        else if (Node* cur_sel = wxGetFrame().getSelectedNode(); cur_sel)
        {
            if (cur_sel->is_Form())
            {
                m_form = cur_sel;
            }
            else
            {
                m_form = cur_sel->get_Form();
            }
        }
    }

    event.Skip();
}

void NodeSearchDlg::OnSelectLocated(wxCommandEvent& /* event unused */)
{
    const std::string name = m_listbox->GetStringSelection().utf8_string();
    if (m_map_found.contains(name))
    {
        const std::set<Node*>& list = m_map_found.at(name);
        m_listbox_forms->Clear();
        for (auto& iter: list)
        {
            m_listbox_forms->Append(iter->as_string(prop_class_name).wx());
        }
        if (m_listbox_forms->GetCount() > 0)
        {
            m_listbox_forms->SetSelection(0);
        }
    }
}

void NodeSearchDlg::OnUnused(wxCommandEvent& /* event unused */)
{
    UnusedGenerators unused_dlg(this);
    unused_dlg.ShowModal();
}

void NodeSearchDlg::OnSearchText(wxCommandEvent& /* event unused */)
{
    const std::string search_text = m_text_search->GetValue().ToStdString();
    for (auto& iter: m_map_found)
    {
        if (iter.first.starts_with(search_text))
        {
            const int form_index = m_listbox->FindString(iter.first);
            if (form_index == wxNOT_FOUND)
            {
                continue;
            }

            m_listbox->SetSelection(form_index);
            wxCommandEvent dummy_event;
            OnSelectLocated(dummy_event);
            break;
        }
    }
}
