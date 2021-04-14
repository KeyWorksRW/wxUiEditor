/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "winres_ctrl.h"

rcCtrl::rcCtrl() {}

void rcCtrl::ParseCommonStyles(ttlib::cview line)
{
    m_isEnabled = !line.contains("WS_DISABLED");
    m_isHidden = line.contains("NOT WS_VISIBLE");

    if (line.contains("BORDER"))
    {
        ParseCommonStyles("wxBORDER_DEFAULT");
    }
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
    m_Class = "wxStaticText";

    if (line.is_sameprefix("CTEXT"))
        m_Styles = "wxALIGN_CENTER_HORIZONTAL";
    else if (line.is_sameprefix("RTEXT"))
        m_Styles = "wxALIGN_RIGHT";
    else
        m_Styles = "wxALIGN_LEFT";

    line.moveto_nextword();
    ParseCommonStyles(line);

    if (line.contains("SS_SUNKEN"))
    {
        AddWinStyle("wxSUNKEN_BORDER");
    }

    if (line.contains("SS_ENDELLIPSIS"))
    {
        AddWinStyle("wxST_ELLIPSIZE_END");
    }
    else if (line.contains("SS_PATHELLIPSIS"))
    {
        AddWinStyle("wxST_ELLIPSIZE_MIDDLE");
    }
    else if (line.contains("SS_WORDELLIPSIS"))
    {
        AddWinStyle("wxST_ELLIPSIZE_START");
    }

    // This should be the label (can be empty but must be quoted).
    if (line[0] == '"')
    {
        line = StepOverQuote(line, m_Label);
    }
    else
    {
        throw std::invalid_argument("Expected static control to be followed with quoted label.");
    }

    // This should be the id (typically IDC_STATIC).
    if (line[0] == ',')
    {
        line = StepOverComma(line, m_ID);
        if (m_ID.is_sameas("IDC_STATIC"))
            m_ID.clear();
    }
    else
    {
        throw std::invalid_argument("Expected static control label to be followed with a comma and an ID.");
    }

    // This should be the dimensions.
    if (ttlib::is_digit(line[0]) || line[0] == ',')
    {
        GetDimensions(line);
        m_minWidth = m_rc.right;
        m_minHeight = m_rc.bottom;
    }
    else
    {
        throw std::invalid_argument("Expected static control ID to be followed with a comma and dimensions.");
    }

    if (line.contains("SS_EDITCONTROL"))
    {
        m_Wrap = m_minWidth;
        m_isMultiLine = true;
    }
}
void rcCtrl::ParseEditCtrl(ttlib::cview line)
{
    m_Class = "wxTextCtrl";

    auto next = ttlib::stepover_pos(line);
    if (next == tt::npos)
    {
        throw std::invalid_argument("Expected edit control name to be followed by additional information.");
    }

    auto start = next;
    while (next < line.size() && line[next] != ',' && !ttlib::is_whitespace(line[next]))
        ++next;

    m_ID.assign(line.substr(start, next - start));
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
        AddStyle("wxTE_CENTER");
    }

    if (line.contains("ES_RIGHT"))
    {
        AddStyle("wxTE_RIGHT");
    }

    if (line.contains("ES_MULTILINE"))
    {
        AddStyle("wxTE_MULTILINE");
    }

    if (line.contains("ES_PASSWORD"))
    {
        AddStyle("wxTE_PASSWORD");
    }

    if (line.contains("ES_READONLY"))
    {
        AddStyle("wxTE_READONLY");
    }

    if (line.contains("ES_WANTRETURN"))
    {
        AddStyle("wxTE_PROCESS_ENTER");
    }

    if (line.contains("ES_NOHIDESEL"))
    {
        AddStyle("wxTE_NOHIDESEL");
    }

    /*

     REVIEW: [KeyWorks - 10-24-2019] As far as I can tell, version 3.1 and earlier of wxWidgets does not support
     the following Windows styles:

        ES_AUTOHSCROLL
        ES_AUTOVSCROLL
        ES_LOWERCASE
        ES_NUMBER
        ES_OEMCONVERT

    */

    ParseCommonStyles(line);
}

void rcCtrl::ParsePushButton(ttlib::cview line)
{
    m_Class = "wxButton";

    if (line.contains("DEFPUSHBUTTON"))
    {
        m_isDefault = true;
    }

    line.moveto_nextword();
    // This should be the label (can be empty but must be quoted).
    if (line[0] == '"')
    {
        line = StepOverQuote(line, m_Label);
    }
    else
    {
        throw std::invalid_argument("Expected static control to be followed with quoted label.");
    }

    // This should be the id
    if (line[0] == ',')
    {
        line = StepOverComma(line, m_ID);
        if (m_ID == "IDOK")
            m_ID = "wxID_OK";
        else if (m_ID == "IDCANCEL")
            m_ID = "wxID_CANCEL";
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
