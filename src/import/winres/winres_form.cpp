/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource form  (usually a dialog)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "winres_form.h"

#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

rcForm::rcForm() {}

void rcForm::ParseDialog(WinResource* pWinResource, ttlib::textfile& txtfile, size_t& curTxtLine)
{
    m_pWinResource = pWinResource;
    auto line = txtfile[curTxtLine].subview();
    auto end = line.find_space();
    if (end == tt::npos)
        throw std::invalid_argument(_tt("Expected an ID then a DIALOG or DIALOGEX."));

    bool isDialog = true;

    for (size_t idx = curTxtLine; idx < txtfile.size(); ++idx)
    {
        line = txtfile[curTxtLine].subview(txtfile[idx].find_nonspace());
        if (line.is_sameprefix("STYLE"))
        {
            ttlib::cstr style(txtfile[curTxtLine]);

            // A line ending with a , or | character means it is continued onto the next line.
            while (style.back() == ',' || style.back() == '|')
            {
                std::string_view tmp("");
                for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
                {
                    tmp = ttlib::find_nonspace(txtfile[curTxtLine]);
                    if (!tmp.empty() && tmp[0] != '/')  // ignore blank lines and comments
                        break;
                }
                style += tmp;
            }

            // Now that we've gathered up all the styles, check for DS_CONTROL -- if that's set, then we need to create a
            // PanelForm not a wxDialog.
            isDialog = style.contains("DS_CONTROL");
            break;
        }
    }

    m_form_type = isDialog ? form_dialog : form_panel;
    m_node = g_NodeCreator.NewNode(isDialog ? gen_wxDialog : gen_PanelForm);

    ttlib::cstr value;  // General purpose string we can use throughout this function
    value = line.substr(0, end);
    if (value[0] == '"')
    {
        value.erase(0, 1);
        if (value.back() == '"')
        {
            value.erase(value.size() - 1, 1);
        }
    }
    m_node->prop_set_value(prop_id, value);
#if defined(_DEBUG)
    m_form_id = value;
#endif  // _DEBUG

    // Note that we can't change the name here or we won't match with the list of names saved from the dialog that got
    // the resource file.
    m_node->prop_set_value(prop_class_name, line.substr(0, end));

    line.remove_prefix(end);
    line.moveto_digit();
    GetDimensions(line);

    for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.is_sameprefix("STYLE"))
        {
            AddStyle(txtfile, curTxtLine);
        }
        else if (line.is_sameprefix("CAPTION"))
        {
            line.moveto_nextword();
            value.ExtractSubString(line);
            m_node->prop_set_value(prop_title, value);
        }
        else if (line.is_sameprefix("FONT"))
        {
            line.moveto_nextword();
            // TODO: [KeyWorks - 10-18-2020] This needs to be ignored for all "standard" fonts, but might be critical
            // for fonts used for non-English dialogs.
        }
        else if (line.is_sameprefix("BEGIN"))
        {
            ++curTxtLine;
            ParseControls(txtfile, curTxtLine);
            break;
        }
    }

    // TODO: [KeyWorks - 10-18-2020] The last step will be to figure what to use for the base and derived filenames.
}

void rcForm::AddStyle(ttlib::textfile& txtfile, size_t& curTxtLine)
{
    ttlib::cstr style(txtfile[curTxtLine]);

    // A line ending with a , or | character means it is continued onto the next line.

    while (style.back() == ',' || style.back() == '|')
    {
        std::string_view tmp("");
        for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
        {
            tmp = ttlib::find_nonspace(txtfile[curTxtLine]);
            if (!tmp.empty() && tmp[0] != '/')  // ignore blank lines and comments
                break;
        }
        style += tmp;
    }

    if (style.contains("DS_CENTER"))
        m_node->prop_set_value(prop_center, "wxBOTH");
    if (style.contains("WS_EX_CONTEXTHELP"))
        m_node->prop_set_value(prop_extra_style, "wxDIALOG_EX_CONTEXTHELP");

    ttlib::cstr original_styles(ttlib::stepover(style));

    if (original_styles.contains("DS_MODALFRAME"))
    {
        m_node->prop_set_value(prop_style, "wxDEFAULT_DIALOG_STYLE");
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

void rcForm::ParseControls(ttlib::textfile& txtfile, size_t& curTxtLine)
{
    for (; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        auto line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.empty() || line[0] == '/')  // ignore blank lines and comments
            continue;

        if (line.is_sameprefix("END"))
            break;

        auto& control = m_ctrls.emplace_back();
        control.ParseDirective(m_pWinResource, line);
        // If the control could not be converted into a node, then remove it from our list
        if (!control.GetNode())
        {
            m_ctrls.pop_back();
        }
        else if (control.GetNode()->isGen(gen_wxSpinCtrl) && control.GetPostProcessStyle().contains("UDS_AUTOBUDDY"))
        {
            auto cur_pos = m_ctrls.size() - 1;
            if (cur_pos > 0 && m_ctrls[cur_pos - 1].GetNode()->isGen(gen_wxTextCtrl))
            {
                control.GetNode()->prop_set_value(prop_id, m_ctrls[cur_pos - 1].GetNode()->prop_as_string(prop_id));
                m_ctrls.erase(m_ctrls.begin() + (cur_pos - 1));
            }
        }
    }
}

void rcForm::GetDimensions(ttlib::cview line)
{
    if (line[0] == ',')
        line.moveto_digit();

    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.left = ttlib::atoi(line);

    auto pos = line.find_first_of(',');
    if (pos == std::string::npos)
        throw std::invalid_argument("Expected comma-separated dimensions");

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.top = ttlib::atoi(line);

    pos = line.find_first_of(',');
    if (pos == std::string::npos)
        throw std::invalid_argument("Expected comma-separated dimensions");

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.right = ttlib::atoi(line);

    pos = line.find_first_of(',');
    if (pos == std::string::npos)
        throw std::invalid_argument("Expected comma-separated dimensions");

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.bottom = ttlib::atoi(line);

    // The resource file uses dialog coordinates which we need to convert into pixel dimensions. We assume that wxWidgets
    // will be using the default Windows 10 font (Segoe UI, 9pt) so we convert to match (note that this is what
    // rcCtrl::GetDimensions() does).

    m_rc.left = (m_rc.left * 7) / 4;
    m_rc.right = (m_rc.right * 7) / 4;
    m_rc.top = (m_rc.top * 15) / 8;
    m_rc.bottom = (m_rc.bottom * 15) / 8;
}

void rcForm::AppendStyle(GenEnum::PropName prop_name, ttlib::cview style)
{
    ttlib::cstr updated_style = m_node->prop_as_string(prop_name);
    if (updated_style.size())
        updated_style << '|';
    updated_style << style;
    m_node->prop_set_value(prop_name, updated_style);
}

void rcForm::AddSizersAndChildren()
{
    // First sort all children horizontally
    std::sort(std::begin(m_ctrls), std::end(m_ctrls), [](rcCtrl a, rcCtrl b) { return a.GetLeft() < b.GetLeft(); });

    // Now sort vertically
    std::sort(std::begin(m_ctrls), std::end(m_ctrls), [](rcCtrl a, rcCtrl b) { return a.GetTop() < b.GetTop(); });

    auto parent = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, m_node.get());
    m_node->Adopt(parent);

    NodeSharedPtr sizer;

    for (size_t idx_child = 0; idx_child < m_ctrls.size(); ++idx_child)
    {
        const auto& child = m_ctrls[idx_child];
        if (child.GetNode()->isGen(gen_wxStaticBoxSizer))
        {
            AddStaticBoxChildren(idx_child);
            parent->Adopt(child.GetNodePtr());
            continue;
        }

        if (idx_child + 1 >= m_ctrls.size())
        {
            // If last control is a button, we may need to center or right-align it.
            if (child.GetNode()->isGen(gen_wxButton))
            {
                int dlg_margin = (GetWidth() / 2) - child.GetWidth();
                if (child.GetLeft() > dlg_margin)
                {
                    if (child.GetRight() < (GetWidth() - dlg_margin))
                        child.GetNode()->prop_set_value(prop_alignment, "wxALIGN_CENTER_HORIZONTAL");
                    else
                        child.GetNode()->prop_set_value(prop_alignment, "wxALIGN_RIGHT");
                }
            }

            // orphaned child, add to form's top level sizer
            parent->Adopt(child.GetNodePtr());
            return;
        }

        if (m_ctrls[idx_child + 1].GetTop() == child.GetTop())
        {
            // If there is more than one child with the same top position, then create a horizontal box sizer
            // and add all children with the same top position.
            sizer = g_NodeCreator.CreateNode(gen_wxBoxSizer, parent.get());
            parent->Adopt(sizer);
            sizer->prop_set_value(prop_orientation, "wxHORIZONTAL");
            while (idx_child < m_ctrls.size() && m_ctrls[idx_child].GetTop() == child.GetTop())
            {
                // Note that we add the child we are comparing to first.
                sizer->Adopt(m_ctrls[idx_child].GetNodePtr());
                ++idx_child;
            }
        }
        else
        {
            sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, parent.get());
            parent->Adopt(sizer);
            sizer->Adopt(child.GetNodePtr());

            if (idx_child + 2 < m_ctrls.size())
            {
                // If the next two sizers have the same top, then they need to be placed in a horizontal sizer.
                if (m_ctrls[idx_child + 1].GetTop() == m_ctrls[idx_child + 2].GetTop())
                    continue;
            }
            ++idx_child;

            while (idx_child < m_ctrls.size() && m_ctrls[idx_child].GetTop() != m_ctrls[idx_child - 1].GetTop())
            {
                // Note that we add the child we are comparing to first.
                sizer->Adopt(m_ctrls[idx_child].GetNodePtr());
                if (idx_child + 2 < m_ctrls.size())
                {
                    // If the next two sizers have the same top, then they need to be placed in a horizontal sizer.
                    if (m_ctrls[idx_child + 1].GetTop() == m_ctrls[idx_child + 2].GetTop())
                        break;
                }
                ++idx_child;
            }
        }
    }
}

void rcForm::AddStaticBoxChildren(size_t& idx_child)
{
    const auto& static_box = m_ctrls[idx_child];
    for (size_t idx_group_child = idx_child + 1; idx_group_child < m_ctrls.size(); ++idx_group_child)
    {
        const auto& child_ctrl = m_ctrls[idx_group_child];
        if (child_ctrl.GetRight() > static_box.GetRight() || child_ctrl.GetTop() > static_box.GetBottom())
            break;
        static_box.GetNode()->Adopt(child_ctrl.GetNodePtr());

        // Update to that caller won't process this child.
        ++idx_child;
    }

    // TODO: [KeyWorks - 05-31-2021] Depending on the number and position of the children, we may need to change the
    // orientation as well as spanning more than one column or row.
}
