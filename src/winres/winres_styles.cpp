/////////////////////////////////////////////////////////////////////////////
// Purpose:   resCtrl style processing functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "winres_ctrl.h"

struct StylePair
{
    const char* win_style;
    const char* wx_style;
};

// clang-format off

// lst_styles maps a window style to a wxWidgets style that is appended to prop_style.
static const StylePair lst_styles[] = {

    { .win_style = "CBS_DROPDOWN", .wx_style = "wxCB_DROPDOWN" },
    { "CBS_DROPDOWNLIST", "wxCB_READONLY" },
    { "CBS_SIMPLE", "wxCB_SIMPLE" },
    { "CBS_SORT", "wxCB_SORT" },

    { "DTS_SHORTDATECENTURYFORMAT", "wxDP_SHOWCENTURY" },
    { "DTS_UPDOWN", "wxDP_SPIN" },

    { "TCS_BOTTOM", "wxNB_BOTTOM" },  // yes, tcs_bottom and tcs_right both put the tabs at the bottom
    { "TCS_FIXEDWIDTH", "wxNB_FIXEDWIDTH" },
    { "TCS_MULTILINE", "wxNB_MULTILINE" },
    { "TCS_RIGHT", "wxNB_BOTTOM" },
    { "TCS_VERTICAL", "wxNB_RIGHT" },  // this will conflict if wxNB_BOTTOM is set

    { "TVS_EDITLABELS", "wxTR_EDIT_LABELS" },
    { "TVS_FULLROWSELECT", "wxTR_FULL_ROW_HIGHLIGHT" },
    { "TVS_HASBUTTONS", "wxTR_HAS_BUTTONS" },
    { "TVS_LINESATROOT", "wxTR_LINES_AT_ROOT" },

    { "TBS_AUTOTICKS", "wxSL_AUTOTICKS" },
    { "TBS_BOTH", "wxSL_BOTH" },
    { "TBS_BOTTOM", "wxSL_BOTTOM" },
    { "TBS_ENABLESELRANGE", "wxSL_SELRANGE" },
    { "TBS_HORZ", "wxSL_HORIZONTAL" },
    { "TBS_LEFT", "wxSL_LEFT" },
    { "TBS_REVERSED", "wxSL_INVERSE" },
    { "TBS_RIGHT", "wxSL_RIGHT" },
    { "TBS_TOP", "wxSL_TOP" },
    { "TBS_VERT", "wxSL_VERTICAL" },

    { "UDS_ALIGNLEFT", "wxALIGN_LEFT" },
    { "UDS_ALIGNRIGHT", "wxALIGN_RIGHT" },
    { "UDS_ARROWKEYS", "wxSP_ARROW_KEYS" },
    { "UDS_WRAP", "wxSP_WRAP" },

};

// clang-format on

void resCtrl::ParseStyles(tt_string_view line)
{
    for (auto& iter: lst_styles)
    {
        if (line.contains(iter.win_style))
            AppendStyle(prop_style, iter.wx_style);
    }
}

void resCtrl::ParseButtonStyles(tt_string_view line)
{
    if (line.contains("BS_RIGHTBUTTON"))
    {
        switch (m_node->gen_name())
        {
            case gen_wxCheckBox:
            case gen_Check3State:
            case gen_wxRadioButton:
                AppendStyle(prop_style, "wxALIGN_RIGHT");
                break;

            case gen_wxButton:
            case gen_wxToggleButton:
                AppendStyle(prop_style, "wxBU_RIGHT");
                break;

            default:
                break;
        }
    }

    if (m_node->isGen(gen_wxButton) || m_node->isGen(gen_wxToggleButton))
    {
        if (line.contains("BS_TOP"))
            AppendStyle(prop_style, "wxBU_TOP");
        else if (line.contains("BS_BOTTOM"))
            AppendStyle(prop_style, "wxBU_BOTTOM");
        else if (line.contains("BS_LEFT"))
            AppendStyle(prop_style, "wxBU_LEFT");
        else if (line.contains("BS_RIGHT"))
            AppendStyle(prop_style, "wxBU_RIGHT");

        if (line.contains("BS_FLAT"))
            AppendStyle(prop_window_style, "wxBORDER_NONE");
    }
    else
    {
        if (line.contains("BS_RIGHT"))
            // Note that for this to work, the parent sizer must be vertically aligned with the wxEXPAND flag set.
            m_node->set_value(prop_alignment, "wxALIGN_RIGHT");

        // Bottom and top won't have any effect, and left is the default, so ignore the other styles.
    }

    if (line.contains("WS_EX_STATICEDGE"))
        AppendStyle(prop_window_style, "wxBORDER_STATIC");
}

void resCtrl::ParseListViewStyles(tt_string_view line)
{
    m_node->set_value(prop_style, "");

    if (line.contains("LVS_ICON"))
        m_node->set_value(prop_mode, "wxLC_ICON");
    else if (line.contains("LVS_SMALLICON"))
        m_node->set_value(prop_mode, "wxLC_SMALL_ICON");
    else if (line.contains("LVS_LIST"))
        m_node->set_value(prop_mode, "wxLC_LIST");
    else if (line.contains("LVS_REPORT"))
        m_node->set_value(prop_mode, "wxLC_REPORT");
    else
        m_node->set_value(prop_mode, "wxLC_REPORT");

    if (line.contains("LVS_ALIGNLEFT"))
        AppendStyle(prop_style, "wxLC_ALIGN_LEFT");
    else if (line.contains("LVS_ALIGNTOP"))
        AppendStyle(prop_style, "wxLC_ALIGN_TOP");

    if (line.contains("LVS_AUTOARRANGE"))
        AppendStyle(prop_style, "wxLC_AUTOARRANGE");
    if (line.contains("LVS_EDITLABELS"))
        AppendStyle(prop_style, "wxLC_EDIT_LABELS");
    if (line.contains("LVS_NOCOLUMNHEADER"))
        AppendStyle(prop_style, "wxLC_NO_HEADER");
    if (line.contains("LVS_SINGLESEL"))
        AppendStyle(prop_style, "wxLC_SINGLE_SEL");
    if (line.contains("LVS_SORTASCENDING"))
        AppendStyle(prop_style, "wxLC_SORT_ASCENDING");
    if (line.contains("LVS_SORTDESCENDING"))
        AppendStyle(prop_style, "wxLC_SORT_DESCENDING");
}

void resCtrl::AddSpecialStyles(tt_string_view line)
{
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
        AppendStyle(prop_window_style, "wxBORDER_SUNKEN");
    }
    if (line.contains("SS_SIMPLE"))
    {
        AppendStyle(prop_window_style, "wxBORDER_SIMPLE");
        m_add_min_width_property = true;
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
        m_add_wrap_property = true;
    }

    //////////// List box styles ////////////

    if (line.contains("LBS_EXTENDEDSEL"))
    {
        m_node->set_value(prop_type, "wxLB_MULTIPLE");
    }
    else if (line.contains("LBS_MULTIPLESEL"))
    {
        m_node->set_value(prop_type, "wxLB_EXTENDED");
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
        m_node->set_value(prop_style, "wxSB_VERTICAL");
}
