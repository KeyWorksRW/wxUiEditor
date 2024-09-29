/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "import_winres.h"

#include "mainapp.h"       // App -- App class
#include "mainframe.h"     // Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class

WinResource::WinResource() {}

bool WinResource::Import(const tt_string& filename, bool write_doc)
{
    std::vector<tt_string> forms;
    if (ImportRc(filename, forms))
    {
        if (write_doc)
            m_project->createDoc(m_docOut);
        return true;
    }

    return false;
}

// clang-format off
static const std::set<std::string_view> lst_ignored_includes = {

    "afxres.h",
    "windows.h",
    "winres.h",

};
// clang-format on

bool WinResource::ImportRc(const tt_string& rc_file, std::vector<tt_string>& forms, bool isNested)
{
    wxBusyCursor busy;

    if (!isNested)
    {
        m_RcFilename = rc_file;
    }

    tt_cwd save_cwd(true);

    tt_string_vector file;
    if (!file.ReadFile(rc_file))
    {
        return false;
    }

    tt_string cwd(rc_file);
    cwd.remove_filename();
    if (cwd.size())
    {
        tt::ChangeDir(cwd);
    }

    if (m_OutDirectory.empty() && !isNested)
    {
        m_OutDirectory.assignCwd();
    }

    // First step though the file to find all #includes. Local header files get stored to an array to add to forms.
    // #included resource files get added to the end of file.

    for (auto& iter: file)
    {
        if (iter.contains("#include"))
        {
            tt_string name;
            tt_string_view curline = iter.view_nonspace();
            name.ExtractSubString(curline, curline.stepover());
            if (name.size())
            {
                tt_string_view ext = name.extension();
                if (ext.is_sameas(".h"))
                {
                    if (!lst_ignored_includes.contains(name))
                    {
                        m_include_lines.emplace(curline);
                    }
                }
                else if (ext.is_sameas(".dlg") || ext.contains(".rc"))
                {
                    curline.moveto_nextword();
                    tt_string path;
                    path.ExtractSubString(curline);
                    if (!path.file_exists())
                    {
                        path.make_relative(rc_file);
                    }

                    ImportRc(path, forms, true);
                }
            }
        }
    }

    if (!isNested)
    {
        m_project = NodeCreation.createNode(gen_Project, nullptr).first;
        m_codepage = 1252;
    }

    // Resource statements often continue onto the next line. Processing a statement is more straightforward if
    // everything needed is on a single line, so we combine those lines here. Note that this will make error messages
    // about parsing problems not be accurate in terms of the line number.

    for (size_t idx = 0; idx < file.size() - 1; ++idx)
    {
        file[idx].trim();
        while (file[idx].size() && (file[idx].back() == ',' || file[idx].back() == '|'))
        {
            file[idx] << file[idx + 1].view_nonspace();
            file[idx].trim();
            file.RemoveLine(idx + 1);
        }

        if (file[idx].size() > 3 && tt::is_found(file[idx].find("NOT", file[idx].size() - 4)))
        {
            file[idx] << ' ' << file[idx + 1].view_nonspace();
            file[idx].trim();
            file.RemoveLine(idx + 1);
        }

        if (file[idx].contains("ICON") || file[idx].contains("BITMAP"))
        {
            tt_string_view line = file[idx].view_nonspace();
            tt_string id;
            if (line.at(0) == '"')
                id.AssignSubString(line);
            else
                id = line.subview(0, line.find_space());
            line.moveto_nextword();
            tt_string type = line.subview(0, line.find_space());
            if (!type.is_sameas("ICON") && !type.is_sameas("BITMAP"))
                continue;  // type must be an exact match at this point.

            while (line.moveto_nextword())
            {
                if (line.at(0) == '"')
                {
                    break;
                }
                // This could be another command, such as DISCARDABLE
            }

            tt_string filename;
            filename.AssignSubString(line);
            filename.make_relative(cwd);
            filename.make_absolute();
            filename.make_relative(m_OutDirectory);
            if (type.is_sameas("ICON"))
                m_map_icons[id] = filename;
            else
                m_map_bitmaps[id] = filename;
        }
    }

    try
    {
        // String tables need to be processed first because we need the id in case it's used as the help string for a menu.
        m_curline = file.FindLineContaining("STRINGTABLE");
        if (tt::is_found(m_curline))
        {
            // We have to restart at zero in order to pickup code page changes
            for (m_curline = 0; m_curline < file.size(); ++m_curline)
            {
                tt_string_view curline = file[m_curline].view_nonspace();
                if (curline.starts_with("STRINGTABLE"))
                {
                    ParseStringTable(file);
                }
                else if (curline.starts_with("#pragma code_page"))
                {
                    auto code = curline.find('(');
                    m_codepage = tt::atoi(curline.subview(code + 1));
                }
            }
        }

        for (m_curline = 0; m_curline < file.size(); ++m_curline)
        {
            tt_string_view curline = file[m_curline].view_nonspace();
            auto start = curline.find_nonspace();
            if (curline.empty() || curline[start] == '/')  // Ignore blank lines and comments.
                continue;
            if (curline[start] == '#')
            {
                auto directive = curline.subview(curline.find_nonspace(start + 1));

                if (directive.starts_with("ifdef"))
                {
                    directive.moveto_nextword();
                    if (directive.starts_with("APSTUDIO_INVOKED"))
                    {
                        // Step over any APSTUDIO_INVOKED section.
                        for (++m_curline; m_curline < file.size(); ++m_curline)
                        {
                            auto line = file[m_curline].subview();
                            start = line.find_nonspace();
                            if (line.empty() || line[start] == '/')  // Ignore blank lines and comments.
                                continue;
                            if (line[start] == '#')
                            {
                                if (auto tmp = line.subview(line.find_nonspace() + 1); tmp.starts_with("endif"))
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

                        file.RemoveLine(m_curline);
                        for (auto erase_position = m_curline; erase_position < file.size(); ++erase_position)
                        {
                            curline = file[erase_position].view_nonspace();
                            if (curline.starts_with("#else"))
                            {
                                do
                                {
                                    file.RemoveLine(erase_position);
                                    curline = file[erase_position].view_nonspace();
                                    if (curline.starts_with("#endif"))
                                    {
                                        break;
                                    }
                                } while (erase_position < file.size());
                            }
                            if (curline.starts_with("#endif"))
                            {
                                file.RemoveLine(erase_position);

                                while (file[erase_position - 1].size() &&
                                       (file[erase_position - 1].back() == ',' || file[erase_position - 1].back() == '|'))
                                {
                                    file[erase_position - 1] << file[erase_position].view_nonspace();
                                    file[erase_position - 1].trim();
                                    file.RemoveLine(erase_position);
                                }

                                break;
                            }
                        }
                    }
                }
                else if (directive.starts_with("pragma"))
                {
                    if (curline.contains(" code_page("))
                    {
                        auto code = curline.find('(');
                        m_codepage = tt::atoi(curline.subview(code + 1));
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
                ParseDialog(file);
            }
            else if (curline.contains(" MENU"))
            {
                auto view = curline.subview(curline.find(" MENU"));
                if (view.size() > 5)
                    continue;  // Means this isn't really a menu command

                if (forms.size())
                {
                    auto pos_end = curline.find(' ');
                    if (auto result = std::find(forms.begin(), forms.end(), curline.substr(0, pos_end));
                        result == forms.end())
                    {
                        // menu id wasn't in the list, so ignore it
                        continue;
                    }
                }
                ParseMenu(file);
            }
            else if (curline.starts_with("STRINGTABLE"))
            {
                ParseStringTable(file);
            }
        }
    }

    catch (const std::exception& e)
    {
        MSG_ERROR(e.what());
        wxMessageBox((tt_string() << "Problem parsing " << m_RcFilename << " at around line " << tt::itoa(m_curline << 1)
                                  << "\n\n"
                                  << e.what())
                         .make_wxString(),
                     "RC Parser");
        return false;
    }

    if (!isNested)
    {
        std::sort(m_forms.begin(), m_forms.end(),
                  [](resForm a, resForm b)
                  {
                      return (a.GetFormName().compare(b.GetFormName()) < 0);
                  });

        InsertDialogs(forms);
    }
    return true;
}

void WinResource::ParseDialog(tt_string_vector& file)
{
    try
    {
        auto line = file[m_curline].subview();
        auto end = line.find_space();
        if (end == tt::npos)
            throw std::invalid_argument("Expected an ID then a DIALOG or DIALOGEX.");

        auto settings = line.subview(line.find_nonspace(end));

        if (!settings.starts_with("DIALOG"))  // verify this is a dialog
            throw std::invalid_argument("Expected an ID then a DIALOG or DIALOGEX.");

        auto pos = tt::stepover_pos(settings);
        if (pos == tt::npos)
            throw std::invalid_argument("Expected dimensions following DIALOG or DIALOGEX.");

        auto& form = m_forms.emplace_back();
        form.ParseDialog(this, file, m_curline);
    }
    catch (const std::exception& e)
    {
        MSG_ERROR(e.what());
        wxMessageBox((tt_string() << "Problem parsing " << m_RcFilename << " at around line " << m_curline + 1 << "\n\n"
                                  << e.what())
                         .make_wxString(),
                     "RC Parser");
    }
}

void WinResource::ParseMenu(tt_string_vector& file)
{
    try
    {
        auto line = file[m_curline].subview();
        auto end = line.find_space();
        if (end == tt::npos)
            throw std::invalid_argument("Expected an ID then a MENU.");

        auto settings = line.subview(line.find_nonspace(end));

        if (!settings.starts_with("MENU"))  // verify this is a dialog
            throw std::invalid_argument("Expected an ID then a MENU.");

        auto& form = m_forms.emplace_back();
        form.ParseMenu(this, file, m_curline);
    }
    catch (const std::exception& e)
    {
        MSG_ERROR(e.what());
        wxMessageBox((tt_string() << "Problem parsing " << m_RcFilename << " at around line " << m_curline + 1 << "\n\n"
                                  << e.what())
                         .make_wxString(),
                     "RC Parser");
    }
}

void WinResource::ParseStringTable(tt_string_vector& file)
{
    for (++m_curline; m_curline < file.size(); ++m_curline)
    {
        auto line = file[m_curline].subview(file[m_curline].find_nonspace());
        if (line.empty() || line.at(0) == '/')  // ignore blank lines and comments
            continue;

        if (line.starts_with("END") || line.starts_with("}"))
        {
            break;
        }
        if (line.starts_with("BEGIN") || line.starts_with("{"))
        {
            continue;
        }

        auto pos = line.find_space();
        if (tt::is_found(pos))
        {
            tt_string id(line.substr(0, pos));
            id.trim(tt::TRIM::right);
            if (id.back() == ',')
                id.pop_back();

            pos = line.find_nonspace(pos);
            if (tt::is_found(pos))
            {
                auto text = ConvertCodePageString(line.view_substr(pos));
                // tt_string text(line.view_substr(pos));
                m_map_stringtable[id] = text;
            }
        }
    }
}

void WinResource::InsertDialogs(std::vector<tt_string>& dialogs)
{
    if (dialogs.size())
    {
        for (auto& dlg_name: dialogs)
        {
            for (auto& dlg: m_forms)
            {
                if (dlg.ConvertFormID(dlg_name).is_sameas(dlg.GetFormName()))
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
    if (form.GetFormType() == resForm::form_dialog || form.GetFormType() == resForm::form_panel)
        form.CreateDialogLayout();

    switch (form.GetFormType())
    {
        case resForm::form_dialog:
        case resForm::form_panel:
            {
                auto node = NodeCreation.makeCopy(form.GetFormNode());
                m_project->adoptChild(node);
            }
            return;

        case resForm::form_menu:
            {
                auto node = NodeCreation.makeCopy(form.GetFormNode());
                m_project->adoptChild(node);
            }
            return;
    }
}

std::optional<tt_string> WinResource::FindIcon(const std::string& id)
{
    if (auto result = m_map_icons.find(id); result != m_map_icons.end())
        return result->second;
    else
        return {};
}

std::optional<tt_string> WinResource::FindBitmap(const std::string& id)
{
    if (auto result = m_map_bitmaps.find(id); result != m_map_bitmaps.end())
        return result->second;
    else
        return {};
}

std::optional<tt_string> WinResource::FindStringID(const std::string& id)
{
    if (auto result = m_map_stringtable.find(id); result != m_map_stringtable.end())
        return result->second;
    else
        return {};
}

tt_string WinResource::ConvertCodePageString(std::string_view str)
{
    if (m_codepage == 65001)  // utf8 code page
        return tt_string(str);
#if defined(_WIN32)
    std::wstring result;
    auto out_size = (str.size() * sizeof(wchar_t)) + sizeof(wchar_t);
    result.reserve(out_size);
    auto count_chars = MultiByteToWideChar(m_codepage, 0, str.data(), (to_int) str.size(), result.data(), (to_int) out_size);
    return tt::utf16to8(std::wstring_view(result.c_str(), count_chars));
#else
    return tt_string(str);
#endif  // _WIN32
}
