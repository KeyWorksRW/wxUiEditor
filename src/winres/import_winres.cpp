/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "tttextfile.h"  // ttTextFile -- Similar to wxTextFile, but uses UTF8 strings

#include "import_winres.h"

#include "../ui/importwinresdlg.h"  // auto-generated: importwinres_base.h and importwinres_base.cpp
#include "mainapp.h"                // App -- App class
#include "mainframe.h"              // Main window frame
#include "node.h"                   // Node class
#include "node_creator.h"           // NodeCreator class

WinResource::WinResource() {}

bool WinResource::Import(const ttString& filename, bool write_doc)
{
    std::vector<ttlib::cstr> forms;
    if (ImportRc(ttlib::cstr() << filename.wx_str(), forms))
    {
        if (write_doc)
            m_project->CreateDoc(m_docOut);
        return true;
    }

    return false;
}

// clang-format off
static constexpr const auto lst_ignored_includes = {

    "afxres.h",
    "windows.h",
    "winres.h",

};
// clang-format on

bool WinResource::ImportRc(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& forms)
{
    m_RcFilename = rc_file;

    if (!m_file.ReadFile(m_RcFilename))
    {
        return false;
    }

    // First step though the file to find all #includes. Local header files get stored to an array to add to forms.
    // #included resource files get added to the end of m_file.

    for (size_t idx = 0; idx < m_file.size(); ++idx)
    {
        if (m_file[idx].contains("#include"))
        {
            ttlib::cstr name;
            auto curline = m_file[idx].view_nonspace();
            name.ExtractSubString(curline, curline.stepover());
            if (name.size())
            {
                auto ext = name.extension();
                if (ext.is_sameas(".h"))
                {
                    bool ignore_file = false;
                    for (auto& iter: lst_ignored_includes)
                    {
                        if (name.is_sameas(iter))
                        {
                            ignore_file = true;
                            break;
                        }
                    }
                    if (!ignore_file)
                    {
                        m_include_lines.emplace(curline);
                    }
                }
                else if (ext.is_sameas(".dlg") || ext.contains(".rc"))
                {
                    ttlib::cstr path = m_RcFilename;
                    path.replace_filename(name);

                    ttlib::viewfile sub_file;
                    if (sub_file.ReadFile(path))
                    {
                        for (auto& iter: sub_file)
                        {
                            m_file.emplace_back(iter);
                        }
                    }
                }
            }
        }
    }

    m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);
    m_codepage = 1252;

    // Resource statements often continue onto the next line. Processing a statement is more straightforward if
    // everything needed is on a single line, so we combine those lines here. Note that this will make error messages
    // about parsing problems not be accurate in terms of the line number.

    for (size_t idx = 0; idx < m_file.size() - 1; ++idx)
    {
        m_file[idx].trim();
        while (m_file[idx].size() && (m_file[idx].back() == ',' || m_file[idx].back() == '|'))
        {
            m_file[idx] << m_file[idx + 1].view_nonspace();
            m_file[idx].trim();
            m_file.RemoveLine(idx + 1);
        }

        if (m_file[idx].size() > 3 && ttlib::is_found(m_file[idx].find("NOT", m_file[idx].size() - 4)))
        {
            m_file[idx] << ' ' << m_file[idx + 1].view_nonspace();
            m_file[idx].trim();
            m_file.RemoveLine(idx + 1);
        }

        if (m_file[idx].contains("ICON") || m_file[idx].contains("BITMAP"))
        {
            auto line = m_file[idx].view_nonspace();
            ttlib::cstr id;
            if (line.at(0) == '"')
                id.AssignSubString(line);
            else
                id = line.subview(0, line.find_space());
            line.moveto_nextword();
            ttlib::cstr type = line.subview(0, line.find_space());
            if (!type.is_sameas("ICON") && !type.is_sameas("BITMAP"))
                continue;  // type must be an exact match at this point.
            line.moveto_nextword();
            ttlib::cstr filename;
            filename.AssignSubString(line);
            if (type.is_sameas("ICON"))
                m_map_icons[id] = filename;
            else
                m_map_bitmaps[id] = filename;
        }
    }

    try
    {
        for (m_curline = 0; m_curline < m_file.size(); ++m_curline)
        {
            auto curline = m_file[m_curline].view_nonspace();
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
                    else
                    {
                        // This is a custom #ifdef and since we're not a compiler, we have no way of knowing whether the
                        // definition being checked is true or not. All we can do is assume the #ifdef is true and parse
                        // until either a #else of #endif.

                        m_file.RemoveLine(m_curline);
                        for (auto erase_position = m_curline; erase_position < m_file.size(); ++erase_position)
                        {
                            curline = m_file[erase_position].view_nonspace();
                            if (curline.is_sameprefix("#else"))
                            {
                                do
                                {
                                    m_file.RemoveLine(erase_position);
                                    curline = m_file[erase_position].view_nonspace();
                                    if (curline.is_sameprefix("#endif"))
                                    {
                                        break;
                                    }
                                } while (erase_position < m_file.size());
                            }
                            if (curline.is_sameprefix("#endif"))
                            {
                                m_file.RemoveLine(erase_position);

                                while (m_file[erase_position - 1].size() && (m_file[erase_position - 1].back() == ',' ||
                                                                             m_file[erase_position - 1].back() == '|'))
                                {
                                    m_file[erase_position - 1] << m_file[erase_position].view_nonspace();
                                    m_file[erase_position - 1].trim();
                                    m_file.RemoveLine(erase_position);
                                }

                                break;
                            }
                        }
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
                if (forms.size())
                {
                    auto pos_end = curline.find(' ');
                    if (auto result = std::find(forms.begin(), forms.end(), curline.substr(0, pos_end));
                        result == forms.end())
                    {
                        // dialog id wasn't in the list, so ignore it
                        continue;
                    }
                }
                ParseDialog();
            }
        }
    }

    catch (const std::exception& e)
    {
        MSG_ERROR(e.what());
        wxMessageBox((ttlib::cstr() << "Problem parsing " << m_RcFilename << " at around line "
                                    << ttlib::itoa(m_curline << 1) << "\n\n"
                                    << e.what())
                         .wx_str(),
                     "RC Parser");
        return false;
    }

    InsertDialogs(forms);

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
        form.ParseDialog(this, m_file, m_curline);
    }
    catch (const std::exception& e)
    {
        MSG_ERROR(e.what());
        wxMessageBox((ttlib::cstr() << "Problem parsing " << m_RcFilename << " at around line " << m_curline + 1 << "\n\n"
                                    << e.what())
                         .wx_str(),
                     "RC Parser");
    }
}

void WinResource::InsertDialogs(std::vector<ttlib::cstr>& dialogs)
{
    if (dialogs.size())
    {
        for (auto& dlg_name: dialogs)
        {
            for (auto& dlg: m_forms)
            {
                if (dlg.ConvertDialogId(dlg_name).is_sameas(dlg.GetFormName()))
                {
                    FormToNode(dlg);
                    break;
                }
            }
        }
    }
    else
    {
        for (auto& dlg: m_forms)
        {
            FormToNode(dlg);
        }
    }
}

void WinResource::FormToNode(resForm& form)
{
    form.AddSizersAndChildren();

    switch (form.GetFormType())
    {
        case resForm::form_dialog:
            {
                auto node = g_NodeCreator.MakeCopy(form.GetFormNode());
                m_project->Adopt(node);
            }
            return;
    }
}

std::optional<ttlib::cstr> WinResource::FindIcon(const std::string& id)
{
    if (auto result = m_map_icons.find(id); result != m_map_icons.end())
        return result->second;
    else
        return {};
}

std::optional<ttlib::cstr> WinResource::FindBitmap(const std::string& id)
{
    if (auto result = m_map_bitmaps.find(id); result != m_map_bitmaps.end())
        return result->second;
    else
        return {};
}
