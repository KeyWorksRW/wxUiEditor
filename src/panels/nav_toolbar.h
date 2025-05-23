///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#pragma once

#include <wx/gdicmn.h>
#include <wx/toolbar.h>

namespace wxue_img
{
    // ../art_src/nav_coll_expand.svg
    extern const unsigned char nav_coll_expand_svg[291];
    // ../art_src/nav_collapse.svg
    extern const unsigned char nav_collapse_svg[214];
    // ../art_src/nav_expand.svg
    extern const unsigned char nav_expand_svg[270];
}

class NavToolbar : public wxToolBar
{
public:
    enum
    {
        id_NavCollExpand = START_NAVTOOL_IDS,
        id_NavCollapse,
        id_NavExpand,
        id_NavMoveDown,
        id_NavMoveLeft,
        id_NavMoveRight,
        id_NavMoveUp
    };

    NavToolbar(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size =
        wxDefaultSize, long style = wxTB_FLAT|wxTB_HORIZONTAL, const wxString &name = wxPanelNameStr);

protected:
};

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************
