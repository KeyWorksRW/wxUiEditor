/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <tttextfile.h>  // ttTextFile -- Similar to wxTextFile, but uses UTF8 strings

#include "import_winres.h"

#include "mainapp.h"       // App -- App class
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

WinResource::WinResource() {}

bool WinResource::Import(const ttString& /* filename */, bool /* write_doc */)
{
    return false;
}

bool WinResource::ImportRc(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& dialogs)
{
    m_RcFilename = rc_file;

    if (!m_file.ReadFile(m_RcFilename))
    {
        return false;
    }

    m_codepage = 1252;

    try
    {
        for (m_curline = 0; m_curline < m_file.size(); ++m_curline)
        {
            auto curline = m_file[m_curline].subview();
            auto start = curline.find_nonspace();
            if (curline.empty() || curline[start] == '/')  // Ignore blank lines and comments.
                continue;
            if (curline[start] == '#')
            {
                auto directive = curline.subview(curline.find_nonspace(start + 1));

                if (directive.is_sameprefix("ifdef"))
                {
                    directive.moveto_nextword();
                    if (ttlib::is_sameprefix(directive, "APSTUDIO_INVOKED"))
                    {
                        // Step over any APSTUDIO_INVOKED section.
                        for (++m_curline; m_curline < m_file.size(); ++m_curline)
                        {
                            auto line = m_file[m_curline].subview();
                            start = line.find_nonspace();
                            if (line.empty() || line[start] == '/')  // Ignore blank lines and comments.
                                continue;
                            if (line[start] == '#')
                            {
                                if (auto tmp = line.subview(line.find_nonspace() + 1); tmp.is_sameprefix("endif"))
                                {
                                    break;
                                }
                            }
                        }
                        continue;
                    }
                }
                else if (directive.is_sameprefix("pragma"))
                {
                    if (curline.contains(" code_page("))
                    {
                        auto code = curline.find('(');
                        m_codepage = std::atoi(curline.subview(code + 1));
                    }
                }
            }
            else if (curline.contains(" DIALOG"))
            {
                auto pos_end = curline.find(' ');
                if (auto result = std::find(dialogs.begin(), dialogs.end(), curline.substr(0, pos_end));
                    result != dialogs.end())
                {
                    ParseDialog();
                }
            }
        }
    }

    catch (const std::exception& e)
    {
        appMsgBox(ttlib::cstr() << _tt("Problem parsing ") << m_RcFilename << _tt(" at around line ")
                                << ttlib::itoa(m_curline << 1) << "\n\n"
                                << e.what(),
                  "RC Parser");
        return false;
    }

    InsertDialogs(dialogs);

    return true;
}

void WinResource::ParseDialog()
{
    try
    {
        auto line = m_file[m_curline].subview();
        auto end = line.find_space();
        if (end == tt::npos)
            throw std::invalid_argument(_tt("Expected an ID then a DIALOG or DIALOGEX."));

        auto settings = line.subview(line.find_nonspace(end));

        if (!settings.is_sameprefix("DIALOG"))  // verify this is a dialog
            throw std::invalid_argument(_tt("Expected an ID then a DIALOG or DIALOGEX."));

        auto pos = ttlib::stepover_pos(settings);
        if (pos == tt::npos)
            throw std::invalid_argument(_tt("Expected dimensions following DIALOG or DIALOGEX."));

        auto& form = m_forms.emplace_back();
        form.ParseDialog(m_file, m_curline);
    }
    catch (const std::exception& e)
    {
        appMsgBox(ttlib::cstr() << _tt("Problem parsing ") + m_RcFilename + _tt(" at around line ") +
                                       ttlib::itoa(m_curline + 1) + "\n\n" + e.what(),
                  "RC Parser");
    }
}

void WinResource::InsertDialogs(std::vector<ttlib::cstr>& dialogs)
{
    m_project = wxGetApp().GetProject();

    for (auto& dlg_name: dialogs)
    {
        for (auto& dlg: m_forms)
        {
            if (dlg_name.is_sameas(dlg.m_Name))
            {
                FormToNode(dlg);
                break;
            }
        }
    }
}

void WinResource::FormToNode(rcForm& form)
{
    if (form.m_Styles.contains("wxDEFAULT_DIALOG_STYLE"))
    {
        auto dlg_node = g_NodeCreator.CreateNode(gen_wxDialog, m_project);
        m_project->AddChild(dlg_node);
        dlg_node->SetParent(m_project->GetSharedPtr());

        auto parent_sizer = g_NodeCreator.CreateNode(gen_wxBoxSizer, dlg_node.get());
        dlg_node->AddChild(parent_sizer);
        parent_sizer->SetParent(dlg_node);
        parent_sizer->get_prop_ptr(prop_orientation)->set_value("wxVERTICAL");

        if (form.m_Name.size())
        {
            dlg_node->get_prop_ptr(prop_var_name)->set_value(form.m_Name);
        }
        if (form.m_Title.size())
        {
            dlg_node->prop_set_value(prop_title, form.m_WinExStyles);
        }
        if (form.m_Center.size() && form.m_Center.is_sameas("wxBOTH"))
        {
            dlg_node->prop_set_value(prop_center, form.m_Center);
        }

        if (form.m_Styles.size())
        {
            dlg_node->prop_set_value(prop_style, form.m_Styles);
        }
        if (form.m_ExStyles.size())
        {
            dlg_node->prop_set_value(prop_extra_style, form.m_ExStyles);
        }
        if (form.m_WinStyles.size())
        {
            dlg_node->prop_set_value(prop_window_style, form.m_WinStyles);
        }
        if (form.m_WinExStyles.size())
        {
            dlg_node->prop_set_value(prop_window_extra_style, form.m_WinExStyles);
        }

        dlg_node->prop_set_value(prop_size, ttlib::cstr() << form.m_rc.right << ',' << form.m_rc.bottom);
    }
}
