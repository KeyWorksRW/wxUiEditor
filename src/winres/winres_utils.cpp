/////////////////////////////////////////////////////////////////////////////
// Purpose:   rcCtrl class utility functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "winres_ctrl.h"

#include "utils.h"  // Utility functions that work with properties

void rcCtrl::ParseCommonStyles(ttlib::cview line)
{
    if (line.contains("WS_DISABLED"))
        m_node->prop_set_value(prop_disabled, true);
    if (line.contains("NOT WS_VISIBLE"))
        m_node->prop_set_value(prop_hidden, true);

    if (line.contains("WS_HSCROLL"))
        AppendStyle(prop_window_style, "wxHSCROLL");
    if (line.contains("WS_VSCROLL"))
        AppendStyle(prop_window_style, "wxVSCROLL");
}

void rcCtrl::GetDimensions(ttlib::cview line)
{
    line.moveto_nonspace();

    if (line.empty())
    {
        MSG_ERROR(ttlib::cstr() << "Missing dimensions :" << m_original_line);
        return;
    }

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

    /*

        On Windows 10, dialogs are supposed to use Segoe UI, 9pt font. However, a lot of dialogs are going to be using
        "MS Shell Dlg" or "MS Shell Dlg2" using an 8pt size. Those coordinates will end up being wrong when displayed by
        wxWidgets because wxWidgets follows the Windows 10 guidelines which normally uses a 9pt font.

        The following code converts dialog coordinates into pixels assuming a 9pt font.

        For the most part, these values are simply used to determine which sizer to place the control in. However, it will
        change things like the wrapping width of a wxStaticText -- it will be larger if the dialog used an 8pt font, smaller
        if it used a 10pt font.

    */

    m_left = static_cast<int>((static_cast<int64_t>(m_rc.left) * 7 / 4));
    m_width = static_cast<int>((static_cast<int64_t>(m_rc.right) * 7 / 4));
    m_top = static_cast<int>((static_cast<int64_t>(m_rc.top) * 15 / 4));
    m_height = static_cast<int>((static_cast<int64_t>(m_rc.bottom) * 15 / 4));
}

ttlib::cview rcCtrl::GetID(ttlib::cview line)
{
    line.moveto_nonspace();

    if (line.empty())
    {
        MSG_ERROR(ttlib::cstr() << "Missing ID :" << m_original_line);
        return line;
    }

    ttlib::cstr id;
    if (line[0] == ',')
    {
        line = StepOverComma(line, id);
        id.LeftTrim();
        if (id.is_sameas("-1"))
        {
            id = "wxID_ANY";
        }
        else if (ttlib::is_digit(id[0]))
        {
            id.insert(0, "id_");
        }
    }
    else
    {
        auto end = line.find_first_of(',');
        if (!ttlib::is_found(end))
        {
            MSG_WARNING(ttlib::cstr() << "Missing comma after ID :" << m_original_line);
            end = line.size();
        }
        id = line.substr(0, end);
        line.remove_prefix(end < line.size() ? end + 1 : end);
    }

    if (id == "IDOK" || id == "1")
        m_node->prop_set_value(prop_id, "wxID_OK");
    else if (id == "IDCANCEL" || id == "2")
        m_node->prop_set_value(prop_id, "wxID_CANCEL");
    else if (id == "IDYES" || id == "6")
        m_node->prop_set_value(prop_id, "wxID_YES");
    else if (id == "IDNO" || id == "7")
        m_node->prop_set_value(prop_id, "wxID_NO");
    else if (id == "IDABORT" || id == "3")
        m_node->prop_set_value(prop_id, "wxID_ABORT ");
    else if (id == "IDCLOSE" || id == "8")
        m_node->prop_set_value(prop_id, "wxID_CLOSE");
    else if (id == "IDHELP" || id == "9" || id == "IDD_HELP")
        m_node->prop_set_value(prop_id, "wxID_HELP");
    else if (id == "IDC_STATIC")
        m_node->prop_set_value(prop_id, "wxID_ANY");
    else
        m_node->prop_set_value(prop_id, id);

    line.moveto_nonspace();
    return line;
}

ttlib::cview rcCtrl::GetLabel(ttlib::cview line)
{
    line.moveto_nonspace();

    if (line.empty())
    {
        MSG_ERROR(ttlib::cstr() << "Missing label :" << m_original_line);
        return line;
    }

    ttlib::cstr label;

    if (line[0] == '"')
    {
        line = StepOverQuote(line, label);
    }
    else
    {
        auto pos = line.find(',');
        if (ttlib::is_error(pos))
        {
            throw std::invalid_argument("Expected a quoted label.");
        }

        label.assign(line, pos);
        line.remove_prefix(pos);
    }

    m_node->prop_set_value(prop_label, ConvertEscapeSlashes(label));

    line.moveto_nonspace();
    return line;
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

    line.remove_prefix(pos + 1);
    line.moveto_nonspace();
    return line;
}

void rcCtrl::AppendStyle(GenEnum::PropName prop_name, ttlib::cview style)
{
    ttlib::cstr updated_style = m_node->prop_as_string(prop_name);
    if (updated_style.size())
        updated_style << '|';
    updated_style << style;
    m_node->prop_set_value(prop_name, updated_style);
}
