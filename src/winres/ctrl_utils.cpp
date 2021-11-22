/////////////////////////////////////////////////////////////////////////////
// Purpose:   resCtrl class utility functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "winres_ctrl.h"

#include "import_winres.h"  // WinResource -- Parse a Windows resource file
#include "utils.h"          // Utility functions that work with properties

void resCtrl::ParseCommonStyles(ttlib::cview line)
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

bool resCtrl::ParseDimensions(ttlib::cview line, wxRect& duRect, wxRect& pixelRect)
{
    duRect = { 0, 0, 0, 0 };
    pixelRect = { 0, 0, 0, 0 };
    line.moveto_nonspace();

    if (line.empty())
        return false;

    if (line.at(0) == ',')
        line.moveto_digit();

    if (line.empty() || !ttlib::is_digit(line.at(0)))
        return false;
    duRect.SetLeft(ttlib::atoi(line));

    auto pos = line.find_first_of(',');
    if (!ttlib::is_found(pos))
        return false;

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line.at(0)))
        return false;
    duRect.SetTop(ttlib::atoi(line));

    pos = line.find_first_of(',');
    if (!ttlib::is_found(pos))
        return false;

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line.at(0)))
        return false;
    duRect.SetWidth(ttlib::atoi(line));

    pos = line.find_first_of(',');
    if (!ttlib::is_found(pos))
        return false;

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line.at(0)))
        return false;
    duRect.SetHeight(ttlib::atoi(line));

    if (m_node->isGen(gen_wxComboBox) && !m_node->prop_as_string(prop_style).contains("wxCB_SIMPLE"))
    {
        // For a drop-down style, the resource file sets the height of the drop-down portion, but for figuring out layout of
        // the control, we need the non-drop height. We can't actually get that, so we take a reasonable guess.

        duRect.SetHeight(12);
    }

    if (m_node->isGen(gen_wxListBox))
    {
        m_node->prop_set_value(prop_minimum_size, ttlib::cstr() << duRect.GetWidth() << ',' << duRect.GetHeight() << 'd');
    }

    /*

        On Windows 10, dialogs are supposed to use Segoe UI, 9pt font. However, a lot of dialogs are going to be using
        "MS Shell Dlg" or "MS Shell Dlg2" using an 8pt size. Those coordinates will end up being wrong when displayed by
        wxWidgets because wxWidgets follows the Windows 10 guidelines which normally uses a 9pt font.

        The following code converts dialog coordinates into pixels assuming a 9pt font.

        For the most part, these values are simply used to determine which sizer to place the control in. However, it will
        change things like the wrapping width of a wxStaticText -- our wxWidgets version will be larger than the original if
        the dialog used an 8pt font, smaller if it used a 10pt font.

    */

    pixelRect.SetLeft(static_cast<int>((static_cast<int64_t>(duRect.GetLeft()) * 7 / 4)));
    pixelRect.SetWidth(static_cast<int>((static_cast<int64_t>(duRect.GetWidth()) * 7 / 4)));
    pixelRect.SetTop(static_cast<int>((static_cast<int64_t>(duRect.GetTop()) * 15 / 4)));
    pixelRect.SetHeight(static_cast<int>((static_cast<int64_t>(duRect.GetHeight()) * 15 / 4)));

    return true;
}

ttlib::cview resCtrl::GetID(ttlib::cview line)
{
    line.moveto_nonspace();

    if (line.empty())
    {
        MSG_ERROR(ttlib::cstr() << "Missing ID :" << m_original_line);
        return line;
    }

    ttlib::cstr id;
    if (line.at(0) == ',')
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

    if (id == "IDOK" || id == "1" || id == "IDC_OK")
        m_node->prop_set_value(prop_id, "wxID_OK");
    else if (id == "IDCANCEL" || id == "2" || id == "IDC_CANCEL")
        m_node->prop_set_value(prop_id, "wxID_CANCEL");
    else if (id == "IDYES" || id == "6" || id == "IDC_YES")
        m_node->prop_set_value(prop_id, "wxID_YES");
    else if (id == "IDNO" || id == "7" || id == "IDC_NO")
        m_node->prop_set_value(prop_id, "wxID_NO");
    else if (id == "IDABORT" || id == "3")
        m_node->prop_set_value(prop_id, "wxID_ABORT ");
    else if (id == "IDCLOSE" || id == "8" || id == "IDC_CLOSE")
        m_node->prop_set_value(prop_id, "wxID_CLOSE");
    else if (id == "IDHELP" || id == "9" || id == "IDD_HELP" || id == "IDC_HELP")
        m_node->prop_set_value(prop_id, "wxID_HELP");
    else if (id == "IDC_APPLY")
        m_node->prop_set_value(prop_id, "wxID_APPLY");
    else
        m_node->prop_set_value(prop_id, "wxID_ANY");

    if (m_node->prop_as_string(prop_id) != "wxID_ANY" || !id.is_sameprefix("IDC_STATIC"))
    {
        m_node->prop_set_value(prop_var_comment, id);
    }

    line.moveto_nonspace();
    return line;
}

ttlib::cview resCtrl::GetLabel(ttlib::cview line)
{
    line.moveto_nonspace();

    if (line.empty())
    {
        MSG_ERROR(ttlib::cstr() << "Missing label :" << m_original_line);
        return line;
    }

    ttlib::cstr label;

    if (line.at(0) == '"')
    {
        line = StepOverQuote(line, label);
    }
    else
    {
        auto pos = line.find(',');
        if (!ttlib::is_found(pos))
        {
            throw std::invalid_argument("Expected a quoted label.");
        }

        label.assign(line, pos);
        line.remove_prefix(pos);
    }

    label = std::move(m_pWinResource->ConvertCodePageString(label));

    if (m_node->isGen(gen_wxHyperlinkCtrl))
    {
        auto begin_anchor = label.locate("<a", 0, tt::CASE::either);
        if (!ttlib::is_found(begin_anchor))
        {
            // Without an anchor, there is no URL
            m_node->prop_set_value(prop_label, ConvertEscapeSlashes(label));
        }
        else
        {
            ttlib::sview view_url = label.view_nonspace(begin_anchor);
            if (view_url.is_sameprefix("<a>", tt::CASE::either))
            {
                view_url.remove_prefix(3);
                view_url.erase_from("</a", tt::CASE::either);
                m_node->prop_set_value(prop_url, view_url);
                m_node->prop_set_value(prop_label, label.substr(0, begin_anchor));
            }
            else if (view_url.is_sameprefix("<a href=\"", tt::CASE::either))
            {
                view_url.remove_prefix(9);
                view_url.erase_from("\">", tt::CASE::either);
                m_node->prop_set_value(prop_url, view_url);
                ttlib::cstr actual_label;
                view_url = label.view_nonspace(label.find("\">"));
                view_url.remove_prefix(2);
                view_url.erase_from("</a", tt::CASE::either);
                actual_label << label.substr(0, begin_anchor) << view_url;
                m_node->prop_set_value(prop_label, actual_label);
            }
            // Also valid just as the above <a href= -- only difference is how many prefix chars to remove
            else if (view_url.is_sameprefix("<a ref=\"", tt::CASE::either))
            {
                view_url.remove_prefix(8);
                view_url.erase_from("\">", tt::CASE::either);
                m_node->prop_set_value(prop_url, view_url);
                ttlib::cstr actual_label;
                view_url = label.view_nonspace(label.find("\">"));
                view_url.remove_prefix(2);
                view_url.erase_from("</a", tt::CASE::either);
                actual_label << label.substr(0, begin_anchor) << view_url;
                m_node->prop_set_value(prop_label, actual_label);
            }
        }
    }

    else
    {
        m_node->prop_set_value(prop_label, ConvertEscapeSlashes(label));
    }

    line.moveto_nonspace();
    return line;
}

ttlib::cview resCtrl::StepOverQuote(ttlib::cview line, ttlib::cstr& str)
{
    ASSERT(line.at(0) == '"');

    // We can't use str.AssignSubString() because in a resource file, quotes are escaped simply by doubling them.

    size_t idx;
    for (idx = 1; idx < line.size(); ++idx)
    {
        if (line[idx] == '"')
        {
            if (idx + 1 >= line.size() || line[idx + 1] != '"')
            {
                return line.subview(idx + 1);
            }
            else
            {
                // Doubled quote is an escape, so add the quote char and step over it
                str += line[idx];
                ++idx;
            }
        }
        else
        {
            str += line[idx];
        }
    }
    return line.subview(idx);
}

ttlib::cview resCtrl::StepOverComma(ttlib::cview line, ttlib::cstr& str)
{
    auto pos = str.AssignSubString(line, ',', ',');
    if (!ttlib::is_found(pos))
        return ttlib::emptystring;

    line.remove_prefix(pos + 1);
    line.moveto_nonspace();
    return line;
}

void resCtrl::AppendStyle(GenEnum::PropName prop_name, ttlib::cview style)
{
    ttlib::cstr updated_style = m_node->prop_as_string(prop_name);
    if (updated_style.size())
        updated_style << '|';
    updated_style << style;
    m_node->prop_set_value(prop_name, updated_style);
}
