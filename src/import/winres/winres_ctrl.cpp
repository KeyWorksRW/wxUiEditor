/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "winres_ctrl.h"

#include "../import_winres.h"  // WinResource -- Parse a Windows resource file
#include "node_creator.h"      // NodeCreator -- Class used to create nodes
#include "utils.h"             // Utility functions that work with properties

rcCtrl::rcCtrl() {}

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
    ASSERT_MSG(line.size(), "Could not locate control's dimensions");
    if (line.empty())
        return;

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
    ASSERT_MSG(line.size(), "Could not locate control's id");

    if (line.size())
    {
        ttlib::cstr id;
        if (line[0] == ',')
        {
            line = StepOverComma(line, id);
        }
        else
        {
            auto end = line.find_first_of(',');
            ASSERT_MSG(ttlib::is_found(end), "Expected a comma after the id");
            if (!ttlib::is_found(end))
                end = line.size();
            id = line.substr(0, end);
            line.remove_prefix(end < line.size() ? end + 1 : end);
        }

        if (id == "IDOK")
            m_node->prop_set_value(prop_id, "wxID_OK");
        else if (id == "IDCANCEL")
            m_node->prop_set_value(prop_id, "wxID_CANCEL");
        else if (id == "IDYES")
            m_node->prop_set_value(prop_id, "wxID_YES");
        else if (id == "IDNO")
            m_node->prop_set_value(prop_id, "wxID_NO");
        else if (id == "IDABORT")
            m_node->prop_set_value(prop_id, "wxID_ABORT ");
        else if (id == "IDCLOSE")
            m_node->prop_set_value(prop_id, "wxID_CLOSE");
        else if (id == "IDHELP")
            m_node->prop_set_value(prop_id, "wxID_HELP");
        else if (id == "IDC_STATIC")
            m_node->prop_set_value(prop_id, "wxID_ANY");
        else
            m_node->prop_set_value(prop_id, id);
    }
    return line;
}

ttlib::cview rcCtrl::GetLabel(ttlib::cview line)
{
    ASSERT_MSG(line.size(), "Could not locate control's id");

    if (line.size())
    {
        // This should be the label (can be empty but must be quoted).
        if (line[0] == '"')
        {
            ttlib::cstr label;
            line = StepOverQuote(line, label);

            m_node->prop_set_value(prop_label, ConvertEscapeSlashes(label));
        }
        else
        {
            throw std::invalid_argument("Expected a quoted label.");
        }
    }

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

    return line.subview(pos + 1);
}

void rcCtrl::AppendStyle(GenEnum::PropName prop_name, ttlib::cview style)
{
    ttlib::cstr updated_style = m_node->prop_as_string(prop_name);
    if (updated_style.size())
        updated_style << '|';
    updated_style << style;
    m_node->prop_set_value(prop_name, updated_style);
}

/*

    A CONTROL directive takes the form:

        CONTROL text, id, class, style, dimensions, extended style

    whereas a regular directive takes the form:

        directive [text], id, dimensions, style, extended style

    To use a single function to process either CONTROL directives or specific directives like CTEXT and LISTBOX, we have to
    get the text if specified or step over it if not. Once the id has been retrieved, we need to step over the class and
    style parameters *only* if it's a CONTROL.

*/

void rcCtrl::ParseDirective(WinResource* pWinResource, ttlib::cview line)
{
    m_pWinResource = pWinResource;
    bool is_control = line.is_sameprefix("CONTROL");
    bool add_wrap_property = false;

    if (is_control)
    {
        line.moveto_nextword();
        if (line.contains("BS_3STATE") || line.contains("BS_AUTO3STATE"))
            m_node = g_NodeCreator.NewNode(gen_Check3State);
        else if (line.contains("BS_CHECKBOX") || line.contains("BS_AUTOCHECKBOX"))
            m_node = g_NodeCreator.NewNode(gen_wxCheckBox);
        else if (line.contains("BS_RADIOBUTTON") || line.contains("BS_AUTORADIOBUTTON"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxRadioButton);
            if (line.contains("WX_GROUP"))
                AppendStyle(prop_style, "wxRB_GROUP");
        }
        else if (line.contains("BS_PUSHBUTTON"))
            m_node = g_NodeCreator.NewNode(gen_wxButton);
        else if (line.contains("BS_DEFPUSHBUTTON"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxButton);
            m_node->prop_set_value(prop_default, true);
        }
        else if (line.contains("BS_COMMANDLINK") || line.contains("BS_DEFCOMMANDLINK"))
            m_node = g_NodeCreator.NewNode(gen_wxCommandLinkButton);
        else if (line.contains("BS_PUSHLIKE"))
            m_node = g_NodeCreator.NewNode(gen_wxToggleButton);
        else if (line.contains("BS_GROUPBOX"))
            m_node = g_NodeCreator.NewNode(gen_wxStaticBoxSizer);
        else if (line.contains("CBS_"))
            m_node = g_NodeCreator.NewNode(gen_wxComboBox);
        else if (line.contains("ES_"))
            m_node = g_NodeCreator.NewNode(gen_wxTextCtrl);
        else if (line.contains("SS_"))
            m_node = g_NodeCreator.NewNode(gen_wxStaticText);
        else if (line.contains("LBS_"))
            m_node = g_NodeCreator.NewNode(gen_wxListBox);
        else if (line.contains("SBS_"))
            m_node = g_NodeCreator.NewNode(gen_wxScrollBar);

        else
        {
            // TODO: [KeyWorks - 06-01-2021] If we get here, then we need to look at the class specifies to determine type of
            // control it is.

            return;
        }
    }
    else
    {
        if (line.is_sameprefix("AUTO3STATE"))
        {
            m_node = g_NodeCreator.NewNode(gen_Check3State);
        }
        else if (line.is_sameprefix("AUTOCHECKBOX"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxCheckBox);
        }
        else if (line.is_sameprefix("AUTORADIOBUTTON"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxRadioButton);
            if (line.contains("WX_GROUP"))
                AppendStyle(prop_style, "wxRB_GROUP");
        }
        else if (line.is_sameprefix("CHECKBOX "))
        {
            m_node = g_NodeCreator.NewNode(gen_wxCheckBox);
        }
        else if (line.is_sameprefix("COMBOBOX"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxComboBox);
        }
        else if (line.is_sameprefix("CTEXT"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxStaticText);
            // We don't know if this will be in a horizontal or vertical sizer, so we just use wxALIGN_CENTER which works for
            // either.
            m_node->prop_set_value(prop_alignment, "wxALIGN_CENTER");
        }
        else if (line.is_sameprefix("DEFPUSHBUTTON"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxButton);
            m_node->prop_set_value(prop_default, true);
        }
        else if (line.is_sameprefix("EDITTEXT"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxTextCtrl);
        }
        else if (line.is_sameprefix("GROUPBOX"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxStaticBoxSizer);
        }
        else if (line.is_sameprefix("LISTBOX"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxListBox);
        }
        else if (line.is_sameprefix("LTEXT"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxStaticText);
            m_node->prop_set_value(prop_alignment, "wxALIGN_LEFT");
        }
        else if (line.is_sameprefix("PUSHBUTTON"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxButton);
        }
        else if (line.is_sameprefix("RTEXT"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxStaticText);
            m_node->prop_set_value(prop_alignment, "wxALIGN_RIGHT");
        }
        else if (line.is_sameprefix("RADIOBUTTON "))
        {
            m_node = g_NodeCreator.NewNode(gen_wxRadioButton);
            if (line.contains("WX_GROUP"))
                AppendStyle(prop_style, "wxRB_GROUP");
        }
        else if (line.is_sameprefix("SCROLLBAR"))
        {
            m_node = g_NodeCreator.NewNode(gen_wxScrollBar);
            if (line.contains("SBS_VERT"))
                m_node->prop_set_value(prop_style, "wxSB_VERTICAL");
        }
        else if (line.is_sameprefix("STATE3"))
        {
            m_node = g_NodeCreator.NewNode(gen_Check3State);
        }
        else if (line.is_sameprefix("ICON"))
        {
            ParseIconControl(line);
            return;
        }

        else
        {
            // TODO: [KeyWorks - 06-01-2021] We handle all controls that MS documented on 05/31/2018, which as of 6/01/2021
            // is still the current documentation. So, if we get here the control is unrecognizable.

#if defined(_DEBUG)
            ttlib::cstr msg("Unrecognized control: ");
            auto pos = line.find_space();
            msg << line.subview(0, pos);
            line.moveto_nextword();
            msg << ' ' << line;
            MSG_WARNING(msg);
#endif  // _DEBUG
            return;
        }
        line.moveto_nextword();
    }

    ASSERT_MSG(line.size(), "Unparsable control line.");
    if (line.empty())
    {
        m_node.reset();
        return;
    }

    if (line[0] == '"')
        line = GetLabel(line);
    line = GetID(line);

    if (is_control)
    {
        ASSERT_MSG(line.size() && line[0] == '"', "CONTROL directive is missing class");

        // This should be the class
        if (line.size() && line[0] == '"')
        {
            ttlib::cstr value;
            line = StepOverQuote(line, value);

            // This could be a system control like "SysTabControl32"
        }
        else
        {
            // Without a class, style and dimensions are probably wrong, so just ignore the entire control.
            m_node.reset();
            return;
        }
    }
    ParseCommonStyles(line);

    //////////// Button styles ////////////

    if (line.contains("BS_RIGHT") || line.contains("BS_LEFTTEXT"))
    {
        if (m_node->isGen(gen_wxCheckBox) || m_node->isGen(gen_Check3State))
            AppendStyle(prop_style, "wxALIGN_RIGHT");
        else
            AppendStyle(prop_style, "wxBU_RIGHT");
    }
    else if (line.contains("BS_TOP"))
        AppendStyle(prop_style, "wxBU_TOP");
    else if (line.contains("BS_BOTTOM"))
        AppendStyle(prop_style, "wxBU_BOTTOM");

    //////////// Combobox styles ////////////

    if (line.contains("CBS_SIMPLE"))
        AppendStyle(prop_style, "wxCB_SIMPLE");
    else if (line.contains("CBS_DROPDOWN"))
        AppendStyle(prop_style, "wxCB_DROPDOWN");

    if (line.contains("CBS_SORT"))
        AppendStyle(prop_style, "wxCB_DROPDOWN");
    if (line.contains("ES_CENTER"))
    {
        AppendStyle(prop_style, "wxTE_CENTER");
    }

    //////////// Edit control styles ////////////

    if (line.contains("ES_CENTER"))
    {
        AppendStyle(prop_style, "wxTE_CENTER");
    }
    else if (line.contains("ES_RIGHT"))
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

    //////////// Static control styles ////////////

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

    if (line.contains("SS_EDITCONTROL"))
    {
        add_wrap_property = true;
    }

    //////////// List box styles ////////////

    if (line.contains("LBS_EXTENDEDSEL"))
    {
        m_node->prop_set_value(prop_type, "wxLB_MULTIPLE");
    }
    else if (line.contains("LBS_MULTIPLESEL"))
    {
        m_node->prop_set_value(prop_type, "wxLB_EXTENDED");
    }
    if (line.contains("LBS_SORT") || line.contains("LBS_STANDARD"))
    {
        AppendStyle(prop_style, "wxLB_SORT");
    }
    if (line.contains("LBS_DISABLENOSCROLL"))
    {
        AppendStyle(prop_style, "wxLB_ALWAYS_SB");
    }
    if (line.contains("LBS_WANTKEYBOARDINPUT"))
    {
        AppendStyle(prop_window_style, "wxWANTS_CHARS");
    }

    //////////// Scrollbar styles ////////////

    if (line.contains("SBS_VERT"))
        m_node->prop_set_value(prop_style, "wxSB_VERTICAL");

    ttlib::cstr value;

    if (is_control)
    {
        // Step over the style
        line = StepOverComma(line, value);
    }

    ASSERT_MSG(line.size() && (ttlib::is_digit(line[0]) || line[0] == ','), "Control is missing dimensions!");
    if (line.empty())
        return;

    // This should be the dimensions.
    if (line.size() && (ttlib::is_digit(line[0]) || line[0] == ','))
    {
        GetDimensions(line);

        if (add_wrap_property)
        {
            m_node->prop_set_value(prop_wrap, m_width);
        }
    }
}

void rcCtrl::ParseIconControl(ttlib::cview line)
{
    line.moveto_nextword();

    ttlib::cstr icon_name;

    // Unlike a normal text parameter, for the ICON directive it might or might not be in quotes.
    if (line[0] == '"')
    {
        line = StepOverQuote(line, icon_name);
    }
    else
    {
        auto pos_comma = line.find(',');
        ASSERT_MSG(ttlib::is_found(pos_comma), "Expected a comma after the ICON control text")
        if (!ttlib::is_found(pos_comma))
            return;
        icon_name = line.subview(0, pos_comma);
        line.remove_prefix(pos_comma);
    }

    auto result = m_pWinResource->FindIcon(icon_name);
    ASSERT_MSG(result, ttlib::cstr() << "Couldn't locate icon: " << icon_name);
    if (!result)
        return;

    m_node = g_NodeCreator.NewNode(gen_wxStaticBitmap);
    ttlib::cstr final_name = result.value();
    final_name.remove_extension();
    final_name << "_ico.h";
    ttlib::cstr prop;
    prop << "Header; " << final_name << "; " << result.value() << "; [-1; -1]";

    // Note that this sets up the filename to convert, but doesn't actually do the conversion -- that will require the code
    // to be generated.
    m_node->prop_set_value(prop_bitmap, prop);
    line = GetID(line);
    GetDimensions(line);
}
