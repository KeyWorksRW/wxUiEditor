/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "tttextfile.h"  // ttTextFile -- Similar to wxTextFile, but uses UTF8 strings

#include "import_winres.h"

#include "../ui/importwinresdlg.h"  // auto-generated: importwinres_base.h and importwinres_base.cpp
#include "mainapp.h"                // App -- App class
#include "mainframe.h"              // Main window frame
#include "node.h"                   // Node class
#include "node_creator.h"           // NodeCreator class
#include "uifuncs.h"                // Miscellaneous functions for displaying UI

WinResource::WinResource() {}

bool WinResource::Import(const ttString& filename, bool write_doc)
{
    ttlib::cstr file;
    file.utf(filename.wx_str());
    ttlib::textfile rc_file;
    if (!rc_file.ReadFile(file))
    {
        appMsgBox(ttlib::cstr() << "Unable to read the file " << file, "Import Windows Resource");
        return false;
    }

    std::vector<ttlib::cstr> dialogs;
    for (auto& iter: rc_file)
    {
        if (iter.contains(" DIALOG"))
        {
            auto pos_end = iter.find(' ');
            auto name = iter.substr(0, pos_end);
            if (ttlib::is_alpha(name[0]))
            {
                dialogs.emplace_back(name);
            }
        }
    }

    if (ImportRc(ttlib::cstr() << filename.wx_str(), dialogs))
    {
        if (write_doc)
            m_project->CreateDoc(m_docOut);
        return true;
    }

    return false;
}

bool WinResource::ImportRc(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& dialogs)
{
    m_RcFilename = rc_file;

    if (!m_file.ReadFile(m_RcFilename))
    {
        return false;
    }

    m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);
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
        MSG_ERROR(e.what());
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
            throw std::invalid_argument("Expected an ID then a DIALOG or DIALOGEX.");

        auto settings = line.subview(line.find_nonspace(end));

        if (!settings.is_sameprefix("DIALOG"))  // verify this is a dialog
            throw std::invalid_argument("Expected an ID then a DIALOG or DIALOGEX.");

        auto pos = ttlib::stepover_pos(settings);
        if (pos == tt::npos)
            throw std::invalid_argument("Expected dimensions following DIALOG or DIALOGEX.");

        auto& form = m_forms.emplace_back();
        form.ParseDialog(m_file, m_curline);
    }
    catch (const std::exception& e)
    {
        MSG_ERROR(e.what());
        appMsgBox(ttlib::cstr() << "Problem parsing " << m_RcFilename << " at around line " << m_curline + 1 << "\n\n"
                                << e.what(),
                  "RC Parser");
    }
}

void WinResource::InsertDialogs(std::vector<ttlib::cstr>& dialogs)
{
    for (auto& dlg_name: dialogs)
    {
        for (auto& dlg: m_forms)
        {
            if (dlg_name.is_sameas(dlg.GetFormName()))
            {
                FormToNode(dlg);
                break;
            }
        }
    }
}

void WinResource::FormToNode(rcForm& form)
{
    switch (form.GetFormType())
    {
        case rcForm::form_dialog:
            {
                auto node = g_NodeCreator.MakeCopy(form.GetFormNode());
                m_project->AddChild(node);
                node->SetParent(m_project);
            }
            return;
    }
}
