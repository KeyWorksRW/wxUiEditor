/////////////////////////////////////////////////////////////////////////////
// Purpose:   Implementation for the UnusedGenerators dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "unused_gen_dlg.h"  // auto-generated: unused_gen_dlg_base.h and unused_gen_dlg_base.cpp

#include <wx/filedlg.h>

#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include <unordered_set>

#include "gen_enums.h"        // GenName, map_GenNames, rmap_GenNames
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

UnusedGenerators::UnusedGenerators() {}

UnusedGenerators::UnusedGenerators(wxWindow* parent)
{
    Create(parent);
}

void FindGenerators(Node* node,
                    std::unordered_set<std::string, str_view_hash, std::equal_to<>>& used)
{
    if (node->is_Gen(gen_Images) || node->is_Gen(gen_Data))
        return;
    if (!node->is_Gen(gen_folder) && !node->is_Gen(gen_sub_folder))
    {
        auto genNameIter = map_GenNames.find(node->get_GenName());
        if (genNameIter != map_GenNames.end() && !used.contains(genNameIter->second))
        {
            used.emplace(genNameIter->second);
        }
    }
    if (node->get_ChildCount())
    {
        for (auto& child: node->get_ChildNodePtrs())
        {
            FindGenerators(child.get(), used);
        }
    }
}

// clang-format off

const auto gen_ignore_list = {

    gen_VerticalBoxSizer,
    gen_gbsizeritem,
    gen_name_array_size,
    gen_oldbookpage,
    gen_sizer_dimension,
    gen_sizeritem,
    gen_splitteritem,

    // These are categories, not actual generators -- this should be kept in sync with the
    // categories in gen_enums.cpp

    gen_Bitmaps,
    gen_Boolean_Validator,
    gen_Choice_Validator,
    gen_Code,
    gen_Code_Generation,
    gen_Command_Bitmaps,
    gen_DlgWindowSettings,
    gen_Integer_Validator,
    gen_List_Validator,
    gen_String_Validator,
    gen_Text_Validator,
    gen_Window_Events,
    gen_XRC,
    gen_XrcSettings,
    gen_flexgridsizerbase,
    gen_folder_Code,
    gen_folder_XRC,
    gen_folder_wxPython,
    gen_folder_wxRuby,
    gen_sizer_child,
    gen_sizeritem_settings,
    gen_wxMdiWindow,
    gen_wxPython,
    gen_wxTopLevelWindow,
    gen_wxTreeCtrlBase,
    gen_wxWindow,

    gen_CPlusSettings,
    gen_DerivedCPlusSettings,
    gen_CPlusHeaderSettings,
    gen_PythonSettings,
    gen_RubySettings,

    gen_unknown,

};

// clang-format on

void UnusedGenerators::OnInit(wxInitDialogEvent& event)
{
    std::unordered_set<std::string, str_view_hash, std::equal_to<>> used;

    for (const auto& child: Project.get_ProjectNode()->get_ChildNodePtrs())
    {
        FindGenerators(child.get(), used);
    }

    bool skipping = true;
    for (auto& iter: rmap_GenNames)
    {
        bool ignored_gen = false;
        for (const auto& ignore: gen_ignore_list)
        {
            if (ignore == iter.second)
            {
                ignored_gen = true;
                break;
            }
        }
        if (ignored_gen)
        {
            continue;
        }

        if (skipping)
        {
            if (iter.second != gen_BookPage)
            {
                continue;
            }

            skipping = false;
        }

        if (!used.contains(iter.first))
        {
            m_listbox->Append(wxString::FromUTF8Unchecked(iter.first.data(), iter.first.size()));
        }
    }

    event.Skip();
}

void UnusedGenerators::OnSave(wxCommandEvent& /* event unused */)
{
    auto filename = wxSaveFileSelector("Save unused", "txt", wxEmptyString, this);
    if (filename.empty())
    {
        return;
    }

    wxue::StringVector file;
    for (unsigned int idx = 0; idx < m_listbox->GetCount(); ++idx)
    {
        file.emplace_back(m_listbox->GetString(idx).ToStdString());
    }

    if (auto result = file.WriteFile(filename.utf8_string()); !result)
    {
        wxMessageBox(wxString("Cannot create or write to the file ") << filename, "Save unused");
    }
}
