/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "winres_ctrl.h"

#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "utils.h"         // Utility functions that work with properties

rcCtrl::rcCtrl() {}

void rcCtrl::ParseCommonStyles(ttlib::cview line)
{
    if (line.contains("WS_DISABLED"))
        m_node->prop_set_value(prop_disabled, true);
    if (line.contains("NOT WS_VISIBLE"))
        m_node->prop_set_value(prop_disabled, true);

    if (line.contains("WS_HSCROLL"))
        AppendStyle(prop_window_style, "wxHSCROLL");
    if (line.contains("WS_VSCROLL"))
        AppendStyle(prop_window_style, "wxVSCROLL");
}

void rcCtrl::GetDimensions(ttlib::cview line)
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
}

ttlib::cview rcCtrl::StepOverQuote(ttlib::cview line, ttlib::cstr& str)
{
    auto pos = str.AssignSubString(line, '"', '"');
    if (pos == std::string::npos || line[pos] != '"')
        throw std::invalid_argument("Missing closing quote");

    return line.subview(pos + 1);
}

ttlib::cview rcCtrl::StepOverComma(ttlib::cview line, ttlib::cstr& str)
{
    auto pos = str.AssignSubString(line, ',', ',');
    if (pos == std::string::npos)
        return ttlib::emptystring;

    return line.subview(pos + 1);
}

void rcCtrl::ParseStaticCtrl(ttlib::cview line)
{
    // TODO: [KeyWorks - 05-30-2021] If SS_BITMAP is specified then this is a gen_wxStaticBitmap
    m_node = g_NodeCreator.NewNode(gen_wxStaticText);

    if (line.is_sameprefix("CTEXT"))
    {
        // We don't know if this will be in a horizontal or vertical sizer, so we just use wxALIGN_CENTER which works for
        // either.
        m_node->prop_set_value(prop_alignment, "wxALIGN_CENTER");
    }
    else if (line.is_sameprefix("RTEXT"))
        m_node->prop_set_value(prop_alignment, "wxALIGN_RIGHT");
    else
        m_node->prop_set_value(prop_alignment, "wxALIGN_LEFT");

    line.moveto_nextword();

    // This should be the label (can be empty but must be quoted).
    if (line[0] == '"')
    {
        ttlib::cstr label;
        line = StepOverQuote(line, label);

        m_node->prop_set_value(prop_label, ConvertEscapeSlashes(label));
    }
    else
    {
        throw std::invalid_argument("Expected static control to be followed with quoted label.");
    }

    ParseCommonStyles(line);

    if (line.contains("SS_SUNKEN"))
    {
        AppendStyle(prop_window_style, "wxSUNKEN_BORDER");
    }
    if (line.contains("SS_SIMPLE"))
    {
        AppendStyle(prop_window_style, "wxBORDER_SIMPLE");
    }

    if (line.contains("SS_BLACKFRAME") || line.contains("SS_BLACKRECT"))
    {
        AppendStyle(prop_background_colour, "wxSYS_COLOUR_WINDOWFRAME");
    }
    else if (line.contains("SS_GRAYFRAME") || line.contains("SS_GRAYRECT"))
    {
        AppendStyle(prop_background_colour, "wxSYS_COLOUR_DESKTOP");
    }
    if (line.contains("SS_WHITEFRAME") || line.contains("SS_WHITERECT"))
    {
        AppendStyle(prop_background_colour, "wxSYS_COLOUR_WINDOW");
    }

    if (line.contains("SS_BLACKRECT") || line.contains("SS_GRAYRECT") || line.contains("SS_WHITERECT"))
    {
        // These styles are rectagles with no border
        AppendStyle(prop_window_style, "wxBORDER_NONE");
    }

    if (line.contains("SS_ENDELLIPSIS"))
    {
        AppendStyle(prop_window_style, "wxST_ELLIPSIZE_END");
    }
    else if (line.contains("SS_PATHELLIPSIS"))
    {
        AppendStyle(prop_window_style, "wxST_ELLIPSIZE_MIDDLE");
    }
    else if (line.contains("SS_WORDELLIPSIS"))
    {
        AppendStyle(prop_window_style, "wxST_ELLIPSIZE_START");
    }

    // This should be the id (typically IDC_STATIC).
    if (line[0] == ',')
    {
        ttlib::cstr id;
        line = StepOverComma(line, id);
        if (!id.is_sameas("IDC_STATIC"))
            m_node->prop_set_value(prop_id, id);
    }
    else
    {
        throw std::invalid_argument("Expected static control label to be followed with a comma and an ID.");
    }

    // This should be the dimensions.
    if (ttlib::is_digit(line[0]) || line[0] == ',')
    {
        GetDimensions(line);
        if (line.contains("SS_BLACK") || line.contains("SS_GRAYF") || line.contains("SS_WHITE"))
        {
            // TODO: [KeyWorks - 05-30-2021] The dimensions are in dialog coordinates which isn't going to match wxWidgets
            // coordinates
            m_node->prop_set_value(prop_size, wxSize(m_rc.right, m_rc.bottom));
        }
    }
    else
    {
        throw std::invalid_argument("Expected static control ID to be followed with a comma and dimensions.");
    }

    if (line.contains("SS_EDITCONTROL"))
    {
        // TODO: [KeyWorks - 05-30-2021] The dimensions are in dialog coordinates which isn't going to match wxWidgets
        // coordinates
        m_node->prop_set_value(prop_wrap, m_minWidth);
    }
}

void rcCtrl::ParseEditCtrl(ttlib::cview line)
{
    m_node = g_NodeCreator.NewNode(gen_wxTextCtrl);

    auto next = ttlib::stepover_pos(line);
    if (next == tt::npos)
    {
        throw std::invalid_argument("Expected edit control name to be followed by additional information.");
    }

    auto start = next;
    while (next < line.size() && line[next] != ',' && !ttlib::is_whitespace(line[next]))
        ++next;

    ttlib::cstr value = line.substr(start, next - start);
    m_node->prop_set_value(prop_id, value);
    line.remove_prefix(next);

    // This should be the dimensions.
    if (ttlib::is_digit(line[0]) || line[0] == ',')
    {
        GetDimensions(line);
        m_minWidth = m_rc.right;
        m_minHeight = m_rc.bottom;
    }
    else
    {
        throw std::invalid_argument("Expected edit control ID to be followed with a comma and dimensions.");
    }

    if (line.contains("ES_CENTER"))
    {
        AppendStyle(prop_style, "wxTE_CENTER");
    }

    if (line.contains("ES_RIGHT"))
    {
        AppendStyle(prop_style, "wxTE_RIGHT");
    }

    if (line.contains("ES_MULTILINE"))
    {
        AppendStyle(prop_style, "wxTE_MULTILINE");
    }

    if (line.contains("ES_PASSWORD"))
    {
        AppendStyle(prop_style, "wxTE_PASSWORD");
    }

    if (line.contains("ES_READONLY"))
    {
        AppendStyle(prop_style, "wxTE_READONLY");
    }

    if (line.contains("ES_WANTRETURN"))
    {
        AppendStyle(prop_style, "wxTE_PROCESS_ENTER");
    }

    if (line.contains("ES_NOHIDESEL"))
    {
        AppendStyle(prop_style, "wxTE_NOHIDESEL");
    }

    /*

     REVIEW: [KeyWorks - 10-24-2019] As far as I can tell, version 3.1 and earlier of wxWidgets does not support
     the following Windows styles:

        ES_AUTOHSCROLL
        ES_AUTOVSCROLL
        ES_LOWERCASE
        ES_NUMBER  // a validator filter could be used to sort of get this...
        ES_OEMCONVERT

    */

    ParseCommonStyles(line);
}

void rcCtrl::ParsePushButton(ttlib::cview line)
{
    m_node = g_NodeCreator.NewNode(gen_wxButton);

    if (line.contains("DEFPUSHBUTTON"))
    {
        m_node->prop_set_value(prop_default, true);
    }

    line.moveto_nextword();
    // This should be the label (can be empty but must be quoted).
    if (line[0] == '"')
    {
        ttlib::cstr label;
        line = StepOverQuote(line, label);
        m_node->prop_set_value(prop_label, label);
    }
    else
    {
        throw std::invalid_argument("Expected static control to be followed with quoted label.");
    }

    ParseCommonStyles(line);

    // This should be the id
    if (line[0] == ',')
    {
        ttlib::cstr id;
        line = StepOverComma(line, id);
        if (id == "IDOK")
            m_node->prop_set_value(prop_id, "wxID_OK");
        else if (id == "IDCANCEL")
            m_node->prop_set_value(prop_id, "wxID_CANCEL");
        else
            m_node->prop_set_value(prop_id, id);
    }
    else
    {
        throw std::invalid_argument("Expected static control label to be followed with a comma and an ID.");
    }

    // This should be the dimensions.
    if (ttlib::is_digit(line[0]) || line[0] == ',')
    {
        GetDimensions(line);
    }
    else
    {
        throw std::invalid_argument("Expected edit control ID to be followed with a comma and dimensions.");
    }
}

void rcCtrl::ParseGroupBox(ttlib::cview line)
{
    m_node = g_NodeCreator.NewNode(gen_wxStaticBoxSizer);

    line.moveto_nextword();
    // This should be the label (can be empty but must be quoted).
    if (line[0] == '"')
    {
        ttlib::cstr label;
        line = StepOverQuote(line, label);
        m_node->prop_set_value(prop_label, label);
    }

    // This should be the id (typically IDC_STATIC).
    if (line[0] == ',')
    {
        ttlib::cstr id;
        line = StepOverComma(line, id);
        if (!id.is_sameas("IDC_STATIC"))
            m_node->prop_set_value(prop_id, id);
    }
    else
    {
        throw std::invalid_argument("Expected GROUPBOX label to be followed with a comma and an ID.");
    }

    // This should be the dimensions.
    if (ttlib::is_digit(line[0]) || line[0] == ',')
    {
        GetDimensions(line);
    }
    else
    {
        throw std::invalid_argument("Expected GROUPBOX ID to be followed with a comma and dimensions.");
    }

    ParseCommonStyles(line);
}

void rcCtrl::AppendStyle(GenEnum::PropName prop_name, ttlib::cview style)
{
    ttlib::cstr updated_style = m_node->prop_as_string(prop_name);
    if (updated_style.size())
        updated_style << '|';
    updated_style << style;
    m_node->prop_set_value(prop_name, updated_style);
}
