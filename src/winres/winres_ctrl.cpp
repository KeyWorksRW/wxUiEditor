/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "winres_ctrl.h"

#include "import_winres.h"  // WinResource -- Parse a Windows resource file
#include "mainapp.h"        // App -- App class
#include "node_creator.h"   // NodeCreator -- Class used to create nodes
#include "utils.h"          // Utility functions that work with properties

resCtrl::resCtrl() {}

struct ClassGenPair
{
    const char* class_name;
    GenEnum::GenName getGenName;
};

// clang-format off

// lst_class_gen is used when CONTROL is specified, and the class parameter is used to
// deterime what generator to create.
static const ClassGenPair lst_class_gen[] = {

    { "\"ComboBoxEx32\"", gen_wxComboBox },
    { "\"ComboBox\"", gen_wxComboBox },
    { "\"Edit\"", gen_wxTextCtrl },
    { "\"Listbox\"", gen_wxListBox },
    { "\"RICHEDIT_CLASS\"", gen_wxTextCtrl },
    { "\"RichEdit20A\"", gen_wxTextCtrl },
    { "\"RichEdit20W\"", gen_wxTextCtrl },
    { "\"RichEdit\"", gen_wxTextCtrl },
    { "\"Scrollbar\"", gen_wxScrollBar },

    { "\"msctls_trackbar32\"", gen_wxSlider },
    { "\"msctls_updown32\"", gen_wxSpinCtrl },
    { "\"msctls_progress32\"", gen_wxGauge },

    { "\"SysAnimate32\"", gen_wxAnimationCtrl },
    { "\"SysLink\"", gen_wxHyperlinkCtrl },
    { "\"SysListView32\"", gen_wxListView },
    { "\"SysTreeView32\"", gen_wxTreeListCtrl },

    { "\"MfcButton\"", gen_wxButton },
    { "\"MfcEditBrowse\"", gen_wxFilePickerCtrl },
    { "\"MfcColorButton\"", gen_wxColourPickerCtrl },
    { "\"MfcFontComboBox\"", gen_wxFontPickerCtrl },
    { "\"MfcLink\"", gen_wxHyperlinkCtrl },
    { "\"MfcMaskedEdit\"", gen_wxTextCtrl },
    { "\"MfcPropertyGrid\"", gen_wxPropertyGridManager },
    { "\"MfcShellList\"", gen_wxGenericDirCtrl },
    { "\"MfcShellTree\"", gen_wxGenericDirCtrl },

    { "\"MfcVSListBox\"", gen_wxEditableListBox },

    // TODO: [KeyWorks - 11-22-2021] Supporting this would mean creating a custom control that has a wxMenu as a child
    // { "\"MfcMenuButton\"", ??? },

};

// lst_name_gen is used when there is a resource directive rather than a CONTROL directive.
static const ClassGenPair lst_name_gen[] = {

    { "COMBOBOX", gen_wxComboBox },
    { "AUTO3STATE", gen_Check3State },
    { "AUTOCHECKBOX", gen_wxCheckBox },
    { "CHECKBOX", gen_wxCheckBox },
    { "EDITTEXT", gen_wxTextCtrl },
    { "GROUPBOX", gen_wxStaticBoxSizer },
    { "LISTBOX", gen_wxListBox },
    { "PUSHBUTTON", gen_wxButton },
    { "STATE3", gen_Check3State },

};

// clang-format on

/*

    A CONTROL directive takes the form:

        CONTROL text, id, class, style, dimensions, extended style

    whereas a regular directive takes the form:

        directive [text], id, dimensions, style, extended style

    To use a single function to process either CONTROL directives or specific directives like CTEXT and LISTBOX, we have to
    get the text if specified or step over it if not. Once the id has been retrieved, we need to step over the class and
    style parameters *only* if it's a CONTROL.

*/

void resCtrl::ParseDirective(WinResource* pWinResource, tt_string_view line)
{
    if (wxGetApp().isTestingMenuEnabled())
    {
        // Create a copy of the original line without the extra spaces that can be used to send to our log window if there
        // are problems processing it.

        m_original_line.clear();
        auto temp_view = line.subview();

        // First copy the diretive name without the leading whitespace
        temp_view.moveto_nonspace();
        auto pos_space = temp_view.find_space();
        if (!tt::is_found(pos_space))
        {
            MSG_ERROR(tt_string() << "Invalid directive: " << line);
            return;
        }

        m_original_line.assign(temp_view, temp_view.find_space());

        // Now copy the rest of the line after skipping over all the alignment whitespace used after the directive
        temp_view.moveto_nextword();
        m_original_line << ' ' << temp_view;
    }

    m_pWinResource = pWinResource;
    bool is_control = line.starts_with("CONTROL");
    m_add_wrap_property = false;
    m_add_min_width_property = false;

    // CONTROL statement is always followed by a label, but some specific directives like
    // COMBOBOX do not. It's possible that the label is a #defined value, and therefore
    // non-quoted so we can't rely on the existance of a quote to know for sure that it is a
    // label or should be treated as an ID.
    bool label_required = true;

    if (is_control)
    {
        line.moveto_nextword();

        for (auto& iter: lst_class_gen)
        {
            if (line.contains(iter.class_name, tt::CASE::either))
            {
                m_node = NodeCreation.newNode(iter.getGenName);
                if (tt::is_sameprefix(iter.class_name, "\"Rich", tt::CASE::either))
                {
                    m_node->set_value(prop_style, "wxTE_RICH2");
                }

                break;
            }
        }

        if (m_node)
        {
            // do nothing, just the start of the following else clauses
        }
        // Start by looking for one of the predefined system classes -- see
        // https://docs.microsoft.com/en-us/windows/win32/controls/individual-control-info

        else if (line.contains("\"Button\"", tt::CASE::either))
        {
            if (line.contains("BS_3STATE") || line.contains("BS_AUTO3STATE"))
                m_node = NodeCreation.newNode(gen_Check3State);
            else if (line.contains("BS_CHECKBOX") || line.contains("BS_AUTOCHECKBOX"))
                m_node = NodeCreation.newNode(gen_wxCheckBox);
            else if (line.contains("BS_RADIOBUTTON") || line.contains("BS_AUTORADIOBUTTON"))
            {
                m_node = NodeCreation.newNode(gen_wxRadioButton);
                if (line.contains("WX_GROUP"))
                    AppendStyle(prop_style, "wxRB_GROUP");
            }
            else if (line.contains("BS_DEFPUSHBUTTON"))
            {
                m_node = NodeCreation.newNode(gen_wxButton);
                m_node->set_value(prop_default, true);
            }
            else if (line.contains("BS_COMMANDLINK") || line.contains("BS_DEFCOMMANDLINK"))
                m_node = NodeCreation.newNode(gen_wxCommandLinkButton);
            else if (line.contains("BS_PUSHLIKE"))
                m_node = NodeCreation.newNode(gen_wxToggleButton);
            else if (line.contains("BS_GROUPBOX"))
                m_node = NodeCreation.newNode(gen_wxStaticBoxSizer);
            else
            {
                // This covers BS_PUSHBUTTON and BS_OWNERDRAW or any unsupported style
                m_node = NodeCreation.newNode(gen_wxButton);
            }
        }
        else if (line.contains("\"Static\"", tt::CASE::either))
        {
            if (line.contains("SS_BITMAP") || line.contains("SS_ICON"))
                m_node = NodeCreation.newNode(gen_wxStaticBitmap);
            else
                m_node = NodeCreation.newNode(gen_wxStaticText);
        }
        else if (line.contains("\"SysDateTimePick32\"", tt::CASE::either))
        {
            // Visual Studio 16.09 formt:time simply displays "DTS_UPDOWN" to get the time picker.
            if (line.contains("DTS_UPDOWN") && !line.contains("DTS_SHORTDATECENTURYFORMAT") &&
                !line.contains("DTS_LONGDATEFORMAT"))
            {
                m_node = NodeCreation.newNode(gen_wxTimePickerCtrl);
            }
            else if (line.contains("DTS_TIMEFORMAT"))
            {
                m_node = NodeCreation.newNode(gen_wxTimePickerCtrl);
            }

            else
                m_node = NodeCreation.newNode(gen_wxDatePickerCtrl);
        }
        else if (line.contains("\"MfcButton\"", tt::CASE::either))
        {
            m_node = NodeCreation.newNode(gen_wxButton);
        }
        else if (line.contains("\"SysTabControl32\"", tt::CASE::either))
        {
            if (line.contains("TCS_BUTTONS"))
                m_node = NodeCreation.newNode(gen_wxToolbook);
            else
                m_node = NodeCreation.newNode(gen_wxNotebook);
        }

        else
        {
            if (wxGetApp().isTestingMenuEnabled())
            {
                tt_string msg("Unrecognized CONTROL: ");
                auto pos = line.find_space();
                msg << line.subview(0, pos);
                line.moveto_nextword();
                msg << ' ' << line;
                MSG_WARNING(msg);
            }
            return;
        }
    }
    else
    {
        ////////// This section handles non-CONTROL statements //////////

        for (auto& iter: lst_name_gen)
        {
            if (line.is_sameprefix(iter.class_name, tt::CASE::either))
            {
                m_node = NodeCreation.newNode(iter.getGenName);
                break;
            }
        }

        if (m_node)
        {
            if (m_node->isGen(gen_wxComboBox) || m_node->isGen(gen_wxTextCtrl) || m_node->isGen(gen_wxListBox))
            {
                label_required = false;
            }
        }

        else if (line.starts_with("AUTORADIOBUTTON"))
        {
            m_node = NodeCreation.newNode(gen_wxRadioButton);
            if (line.contains("WX_GROUP"))
                AppendStyle(prop_style, "wxRB_GROUP");
        }
        else if (line.starts_with("CTEXT"))
        {
            m_node = NodeCreation.newNode(gen_wxStaticText);
            // We don't know if this will be in a horizontal or vertical sizer, so we just use wxALIGN_CENTER which works for
            // either.
            m_node->set_value(prop_style, "wxALIGN_CENTER_HORIZONTAL");
        }
        else if (line.starts_with("DEFPUSHBUTTON"))
        {
            m_node = NodeCreation.newNode(gen_wxButton);
            m_node->set_value(prop_default, true);
        }
        else if (line.starts_with("LTEXT"))
        {
            m_node = NodeCreation.newNode(gen_wxStaticText);
            // m_node->set_value(prop_style, "wxALIGN_LEFT");
        }
        else if (line.starts_with("RTEXT"))
        {
            m_node = NodeCreation.newNode(gen_wxStaticText);
            m_node->set_value(prop_style, "wxALIGN_RIGHT");
        }
        else if (line.starts_with("RADIOBUTTON "))
        {
            m_node = NodeCreation.newNode(gen_wxRadioButton);
            if (line.contains("WX_GROUP"))
                AppendStyle(prop_style, "wxRB_GROUP");
        }
        else if (line.starts_with("SCROLLBAR"))
        {
            m_node = NodeCreation.newNode(gen_wxScrollBar);
            label_required = false;
            if (line.contains("SBS_VERT"))
                m_node->set_value(prop_style, "wxSB_VERTICAL");
        }
        else if (line.starts_with("ICON"))
        {
            ParseIconControl(line);
            return;
        }

        else
        {
            // TODO: [KeyWorks - 06-01-2021] We handle all controls that MS documented on 05/31/2018, which as of 6/01/2021
            // is still the current documentation. So, if we get here the control is unrecognizable.

            if (wxGetApp().isTestingMenuEnabled())
            {
                tt_string msg("Unrecognized resource directive: ");
                auto pos = line.find_space();
                msg << line.subview(0, pos);
                line.moveto_nextword();
                msg << ' ' << line;
                MSG_WARNING(msg);
            }
            return;
        }
        line.moveto_nextword();
    }

    if (line.empty())
    {
        MSG_ERROR(tt_string() << "Unparsable control :" << m_original_line);
        m_node.reset();
        return;
    }

    ASSERT_MSG(m_node, "Node not created!");
    if (!m_node)
        return;

    if (m_node->isGen(gen_wxStaticBitmap))
    {
        ParseImageControl(line);
        return;
    }

    if (label_required)
    {
        line = GetLabel(line);
    }
    line = GetID(line);

    if (is_control)
    {
        // This should be the class
        if (line.size() && line.at(0) == '"')
        {
            tt_string value;
            line = StepOverQuote(line, value);

            // This could be a system control like "SysTabControl32"
        }
        else
        {
            MSG_ERROR(tt_string() << "CONTROL missing class :" << m_original_line);
            // Without a class, style and dimensions are probably wrong, so just ignore the entire control.
            m_node.reset();
            return;
        }
    }
    ParseCommonStyles(line);

    bool is_style_processed = false;  // true means any non-common styles have been processed

    switch (m_node->getGenName())
    {
        case gen_wxSlider:
            ParseStyles(line);
            is_style_processed = true;
            break;

        case gen_wxAnimationCtrl:
            // There are no supported styles for an animation control
            is_style_processed = true;
            break;

        case gen_wxButton:
        case gen_Check3State:
        case gen_wxCheckBox:
        case gen_wxRadioButton:
            ParseButtonStyles(line);
            is_style_processed = true;
            break;

        case gen_wxComboBox:
        case gen_wxBitmapComboBox:
            ParseStyles(line);
            is_style_processed = true;
            break;

        case gen_wxListView:
            ParseListViewStyles(line);
            is_style_processed = true;
            break;

        case gen_wxSpinCtrl:
            ParseStyles(line);
            if (line.contains("UDS_AUTOBUDDY") && line.contains("UDS_SETBUDDYINT"))
                m_non_processed_style = "UDS_AUTOBUDDY";
            is_style_processed = true;
            break;

        case gen_wxDatePickerCtrl:
            ParseStyles(line);
            is_style_processed = true;
            break;

        case gen_wxTimePickerCtrl:
            // There are no supported styles for an Time style DateTime control
            is_style_processed = true;
            break;

        case gen_wxGauge:
            if (line.contains("PBS_SMOOTH"))
                AppendStyle(prop_style, "wxGA_SMOOTH");
            if (line.contains("PBS_VERTICAL"))
                m_node->set_value(prop_orientation, "wxCAL_SHOW_WEEK_NUMBERS");
            is_style_processed = true;
            break;

        case gen_wxCalendarCtrl:
            if (line.contains("MCS_WEEKNUMBERS"))
                AppendStyle(prop_style, "wxGA_SMOOTH");
            is_style_processed = true;
            break;

        case gen_wxTreeCtrl:
            ParseStyles(line);
            if (!line.contains("TVS_HASLINES"))
                AppendStyle(prop_style, "wxTR_NO_LINES");
            is_style_processed = true;
            break;

        case gen_wxNotebook:
        case gen_wxToolbook:
            ParseStyles(line);
            is_style_processed = true;
            break;

        default:
            break;
    }

    if (!is_style_processed)
    {
        AddSpecialStyles(line);
    }

    tt_string value;

    if (is_control)
    {
        // Step over the style
        line = StepOverComma(line, value);
    }

    if (line.empty())
    {
        MSG_ERROR(tt_string() << "Missing dimensions :" << m_original_line);
        return;
    }

    // This should be the dimensions.
    if (line.size() && (tt::is_digit(line.at(0)) || line.at(0) == ','))
    {
        if (!ParseDimensions(line, m_du_rect, m_pixel_rect))
        {
            MSG_ERROR(tt_string() << "Missing dimensions :" << m_original_line);
            return;
        }

        if (m_add_wrap_property || (m_node->isGen(gen_wxStaticText) && du_height() > 14))
        {
            m_node->set_value(prop_wrap, m_pixel_rect.GetWidth());
        }

        if (m_add_min_width_property || m_node->isGen(gen_wxTextCtrl) || m_node->isGen(gen_wxComboBox))
        {
            m_node->set_value(prop_minimum_size, tt_string() << m_du_rect.GetWidth() << ",-1d");
        }
    }
    else
    {
        MSG_ERROR(tt_string() << "Missing dimensions :" << m_original_line);
    }
}
