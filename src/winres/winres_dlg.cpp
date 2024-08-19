/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource DIALOG or DIALOGEX
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "winres_form.h"

#include "import_winres.h"  // WinResource -- Parse a Windows resource file
#include "mainapp.h"        // App -- App class
#include "node_creator.h"   // NodeCreator -- Class used to create nodes

resForm::resForm() {}

void resForm::ParseDialog(WinResource* pWinResource, tt_string_vector& txtfile, size_t& curTxtLine)
{
    m_pWinResource = pWinResource;
    tt_string_view line = txtfile[curTxtLine].subview();
    auto end = line.find_space();
    if (end == tt::npos)
        throw std::invalid_argument("Expected an ID then a DIALOG or DIALOGEX.");

    bool isDialog = true;

    for (auto& iter: txtfile)
    {
        line = iter.subview(iter.find_nonspace());
        if (line.starts_with("STYLE"))
        {
            if (line.contains("DS_CONTROL"))
                isDialog = false;  // This is a panel dialog, typically used by a wizard
            line = txtfile[curTxtLine].subview();
            break;
        }
    }

    m_form_type = isDialog ? form_dialog : form_panel;
    m_form_node = NodeCreation.newNode(isDialog ? gen_wxDialog : gen_PanelForm);

    if (wxGetApp().isTestingMenuEnabled())
    {
        tt_string fullpath;
        fullpath.assignCwd();
        fullpath.append_filename(txtfile.filename().filename());
#if defined(_WIN32)
        // VSCode File Open dialog can't handle forward slashes on Windows
        fullpath.forwardslashestoback();
#endif  // _WIN32
        m_form_node->set_value(prop_base_src_includes, tt_string() << "// " << fullpath);
    }

    tt_string value;  // General purpose string we can use throughout this function
    value = line.substr(0, end);
    m_form_node->set_value(prop_class_name, ConvertFormID(value));

    if (wxGetApp().isTestingMenuEnabled())
    {
        m_form_id = m_form_node->as_string(prop_class_name);
    }

    line.remove_prefix(end);
    line.moveto_digit();

    ParseDimensions(line, m_du_rect, m_pixel_rect);

    for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.starts_with("STYLE"))
        {
            AddStyle(txtfile, curTxtLine);
        }
        else if (line.starts_with("CAPTION"))
        {
            line.moveto_nextword();
            value.ExtractSubString(line);
            m_form_node->set_value(prop_title, m_pWinResource->ConvertCodePageString(value));
        }
        else if (line.starts_with("FONT"))
        {
            line.moveto_nextword();
            // TODO: [KeyWorks - 10-18-2020] This needs to be ignored for all "standard" fonts, but might be critical
            // for fonts used for non-English dialogs.
        }
        else if (line.starts_with("BEGIN") || line.starts_with("{"))
        {
            ++curTxtLine;
            ParseControls(txtfile, curTxtLine);
            break;
        }
    }

    // TODO: [KeyWorks - 10-18-2020] The last step will be to figure what to use for the base and derived filenames.
}

void resForm::AddStyle(tt_string_vector& txtfile, size_t& curTxtLine)
{
    tt_string style(txtfile[curTxtLine]);

    // A line ending with a , or | character means it is continued onto the next line.

    while (style.back() == ',' || style.back() == '|')
    {
        std::string_view tmp("");
        for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
        {
            tmp = tt::find_nonspace(txtfile[curTxtLine]);
            if (!tmp.empty() && tmp[0] != '/')  // ignore blank lines and comments
                break;
        }
        style += tmp;
    }

    if (style.contains("DS_CENTER"))
        m_form_node->set_value(prop_center, "wxBOTH");
    if (style.contains("WS_EX_CONTEXTHELP"))
        m_form_node->set_value(prop_extra_style, "wxDIALOG_EX_CONTEXTHELP");

    tt_string original_styles(tt::stepover(style));

    if (original_styles.contains("DS_MODALFRAME"))
    {
        m_form_node->set_value(prop_style, "wxDEFAULT_DIALOG_STYLE");
        // It's common for dialogs to duplicate the styles that DS_MODALFRAME add, so we remove them here to
        // avoid adding them later.
        original_styles.Replace("WS_CAPTION", "");
        original_styles.Replace("WS_SYSMENU", "");
        original_styles.Replace("WS_POPUP", "");
    }

    if (original_styles.contains("WS_CAPTION"))
    {
        AppendStyle(prop_style, "wxCAPTION");
    }

    if (original_styles.contains("WS_SYSMENU"))
    {
        AppendStyle(prop_style, "wxSYSTEM_MENU");
    }

    if (original_styles.contains("WS_MAXIMIZEBOX"))
    {
        AppendStyle(prop_style, "wxMAXIMIZE_BOX");
    }

    if (original_styles.contains("WS_MINIMIZEBOX"))
    {
        AppendStyle(prop_style, "wxMINIMIZE_BOX");
    }

    if (original_styles.find("WS_THICKFRAME") != tt::npos || original_styles.find("WS_SIZEBOX") != tt::npos)
    {
        // In spite of what the documentation states (as of 3.1.6) there is no wxTHICK_FRAME. The closest would be
        // wxBORDER_THEME.

        // wxDialog interface (forms.xml) doesn't support this
        // AppendStyle(prop_style, "wxBORDER_THEME");
    }

    if (original_styles.find("WS_CLIPCHILDREN") != tt::npos)
    {
        // wxDialog interface (forms.xml) doesn't support this
        // AppendStyle(prop_style, "wxCLIP_CHILDREN");
    }

    if (original_styles.find("WS_CLIPSIBLINGS") != tt::npos)
    {
        // This won't make sense for the dialog we create since we don't allow overlapping children.
    }

    if (original_styles.find("WS_POPUP") != tt::npos)
    {
        // There is a wxPOPUP_WINDOW, but does it work with dialogs?
    }

    // REVIEW: [KeyWorks - 08-24-2019] Note that we do not convert WS_HSCROLL or WS_VSCROLL.
    // The assumption is that this would be better handled by a sizer parent.

    if (original_styles.find("WS_EX_TOPMOST") != tt::npos)
    {
        AppendStyle(prop_style, "WS_EX_TOPMOST");
    }
}

void resForm::ParseControls(tt_string_vector& txtfile, size_t& curTxtLine)
{
    for (; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        auto line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.empty() || line.at(0) == '/')  // ignore blank lines and comments
            continue;

        if (line.starts_with("END") || line.starts_with("}"))
            break;

        auto& control = m_ctrls.emplace_back();
        control.ParseDirective(m_pWinResource, line);
        // If the control could not be converted into a node, then remove it from our list
        if (!control.getNode())
        {
            m_ctrls.pop_back();
        }
        else if (control.getNode()->isGen(gen_wxSpinCtrl) && control.GetPostProcessStyle().contains("UDS_AUTOBUDDY"))
        {
            // A spin control can specifify that the previous control should be considered a "buddy" that responds to changes
            // in the spin control. In wxWidgets, a spin control already includes an edit control, so we delete the previous
            // edit control and use it's id for the spin control.
            auto cur_pos = m_ctrls.size() - 1;
            if (cur_pos > 0 && m_ctrls[cur_pos - 1].getNode()->isGen(gen_wxTextCtrl))
            {
                control.getNode()->set_value(prop_id, m_ctrls[cur_pos - 1].getNode()->as_string(prop_id));
                m_ctrls.erase(m_ctrls.begin() + (cur_pos - 1));
            }
        }
    }
}

void resForm::AppendStyle(GenEnum::PropName prop_name, tt_string_view style)
{
    tt_string updated_style = m_form_node->as_string(prop_name);
    if (updated_style.size())
        updated_style << '|';
    updated_style << style;
    m_form_node->set_value(prop_name, updated_style);
}

tt_string resForm::ConvertFormID(tt_string_view id)
{
    id.moveto_nonspace();
    tt_string value;
    if (id.at(0) == '"')
    {
        value.AssignSubString(id);
    }
    else if (tt::is_digit(value[0]))
    {
        value << "id_" << id;
    }
    else
    {
        value = id;
    }

    value.RightTrim();

    if (value.starts_with("IDD_"))
        value.erase(0, sizeof("IDD_") - 1);

    if (value.size() > 1 && std::isupper(value[1]))
    {
        auto utf8locale = std::locale("en_US.utf8");
        for (size_t idx = 1; idx < value.size(); ++idx)
        {
            if (value[idx] == '_')
            {
                value.erase(idx, 1);
                value[idx] = std::toupper(value[idx], utf8locale);
            }
            else
            {
                value[idx] = std::tolower(value[idx], utf8locale);
            }
        }
    }
    return value;
}
