/////////////////////////////////////////////////////////////////////////////
// Purpose:   Includes all xpm files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#if !defined(_WIN32) || defined(__clang__)
    // warning: trigraph ignored
    #pragma clang diagnostic ignored "-Wtrigraphs"
#endif

#include <unordered_map>

#include "bitmaps.h"  // Map of bitmaps accessed by name

// default.xpm -- 16x16, transparent with double question marks
// unkown.xpm  -- 32x32, transparent with double question marks

#include "xpm/nav_coll_expand.xpm"
#include "xpm/nav_collapse.xpm"
#include "xpm/nav_expand.xpm"
#include "xpm/nav_movedown.xpm"
#include "xpm/nav_moveleft.xpm"
#include "xpm/nav_moveright.xpm"
#include "xpm/nav_moveup.xpm"

#include "xpm/auinotebook.xpm"
#include "xpm/auitoolbar.xpm"
#include "xpm/bitmap_button.xpm"
#include "xpm/bitmap_toggle_button.xpm"
#include "xpm/bmp_combo_box.xpm"
#include "xpm/book_page.xpm"
#include "xpm/bottom.xpm"
#include "xpm/calendar.xpm"
#include "xpm/check3state.xpm"
#include "xpm/colourPickerIcon.xpm"
#include "xpm/dataview_ctrl.xpm"
#include "xpm/dataviewlist_column.xpm"
#include "xpm/dataviewlist_ctrl.xpm"
#include "xpm/dataviewtree_ctrl.xpm"
#include "xpm/datepicker.xpm"
#include "xpm/default.xpm"
#include "xpm/dirPicker.xpm"
#include "xpm/doc_mdi_parent_frame.xpm"
#include "xpm/doc_parent_frame.xpm"
#include "xpm/document.xpm"
#include "xpm/filePicker.xpm"
#include "xpm/flex_grid_sizer.xpm"
#include "xpm/fontPicker.xpm"
#include "xpm/wxFrame.xpm"
#include "xpm/gauge.xpm"
#include "xpm/generate.xpm"
#include "xpm/genericdir_ctrl.xpm"
#include "xpm/grid.xpm"
#include "xpm/grid_bag_sizer.xpm"
#include "xpm/grid_sizer.xpm"
#include "xpm/hidden.xpm"
#include "xpm/htmlwin.xpm"
#include "xpm/hyperlink_ctrl.xpm"
#include "xpm/infobar.xpm"
#include "xpm/left.xpm"
#include "xpm/wxButton.xpm"
#include "xpm/wxCheckBox.xpm"
#include "xpm/wxCheckListBox.xpm"
#include "xpm/wxChoice.xpm"
#include "xpm/wxChoicebook.xpm"
#include "xpm/wxCollapsiblePane.xpm"
#include "xpm/wxComboBox.xpm"
#include "xpm/wxDialog.xpm"
#include "xpm/wxListBox.xpm"
#include "xpm/wxListView.xpm"
#include "xpm/wxListbook.xpm"
#if defined(_DEBUG)
    #include "xpm/debug_16.xpm"
    #include "xpm/debug_32.xpm"
#else
    #include "xpm/logo16.xpm"
    #include "xpm/logo32.xpm"
#endif  // _DEBUG
#include "xpm/menu.xpm"
#include "xpm/wxMenuBar.xpm"
#include "xpm/menuitem.xpm"
#include "xpm/moveColLeft.xpm"
#include "xpm/moveColRight.xpm"
#include "xpm/moveRowDown.xpm"
#include "xpm/moveRowUp.xpm"
#include "xpm/project.xpm"
#include "xpm/propgriditem.xpm"
#include "xpm/propgridpage.xpm"
#include "xpm/radio_box.xpm"
#include "xpm/ribbon_bar.xpm"
#include "xpm/ribbon_button.xpm"
#include "xpm/ribbon_button_dropdown.xpm"
#include "xpm/ribbon_button_hybrid.xpm"
#include "xpm/ribbon_button_toggle.xpm"
#include "xpm/ribbon_buttonbar.xpm"
#include "xpm/ribbon_gallery.xpm"
#include "xpm/ribbon_gallery_item.xpm"
#include "xpm/ribbon_page.xpm"
#include "xpm/ribbon_panel.xpm"
#include "xpm/richtextctrl.xpm"
#include "xpm/scintilla.xpm"
#include "xpm/scrollbar.xpm"
#include "xpm/search.xpm"
#include "xpm/separator.xpm"
#include "xpm/sizer.xpm"
#include "xpm/sizer_horizontal.xpm"
#include "xpm/slider.xpm"
#include "xpm/spacer.xpm"
#include "xpm/spin_ctrl.xpm"
#include "xpm/spin_ctrl_double.xpm"
#include "xpm/spinbtn.xpm"
#include "xpm/static_bitmap.xpm"
#include "xpm/static_line.xpm"
#include "xpm/staticbox_sizer.xpm"
#include "xpm/statusbar.xpm"
#include "xpm/stddialogbuttonsizer.xpm"
#include "xpm/submenu.xpm"
#include "xpm/text_sizer.xpm"
#include "xpm/timepicker.xpm"
#include "xpm/timer.xpm"
#include "xpm/toggle_button.xpm"
#include "xpm/tool.xpm"
#include "xpm/wxToolBar.xpm"
#include "xpm/toolseparator.xpm"
#include "xpm/tree_ctrl.xpm"
#include "xpm/treelistctrl.xpm"
#include "xpm/treelistctrlcolumn.xpm"
#include "xpm/unknown.xpm"
#include "xpm/wxWizard.xpm"
#include "xpm/wxWizardPageSimple.xpm"
#include "xpm/wrap_sizer.xpm"
#include "xpm/wxNotebook.xpm"
#include "xpm/wxPanel.xpm"
#include "xpm/wxPropertyGrid.xpm"
#include "xpm/wxPropertyGridManager.xpm"
#include "xpm/wxRadioButton.xpm"
#include "xpm/wxScrolledWindow.xpm"
#include "xpm/wxSimplebook.xpm"
#include "xpm/wxSplitterWindow.xpm"
#include "xpm/wxStaticText.xpm"
#include "xpm/wxTextCtrl.xpm"

static const std::unordered_map<std::string, const char**> map_xpm_strings = {
    { "nav_coll_expand", nav_coll_expand_xpm },
    { "nav_collapse", nav_collapse_xpm },
    { "nav_expand", nav_expand_xpm },
    { "nav_movedown", nav_movedown_xpm },
    { "nav_moveleft", nav_moveleft_xpm },
    { "nav_moveright", nav_moveright_xpm },
    { "nav_moveup", nav_moveup_xpm },

    { "auinotebook", auinotebook_xpm },
    { "auitoolbar", auitoolbar_xpm },
    { "bitmap_button", bitmap_button_xpm },
    { "bitmap_toggle_button", bitmap_toggle_button_xpm },
    { "bmp_combo_box", bmp_combo_box_xpm },
    { "book_page", book_page_xpm },
    { "bottom", bottom_xpm },
    { "wxButton", wxButton_xpm },
    { "calendar", calendar_xpm },
    { "wxCheckBox", wxCheckBox_xpm },
    { "check3state", check3state_xpm },
    { "wxCheckListBox", wxCheckListBox_xpm },
    { "wxChoice", wxChoice_xpm },
    { "wxChoicebook", wxChoicebook_xpm },
    { "wxCollapsiblePane", wxCollapsiblePane_xpm },
    { "colourPickerIcon", colourPickerIcon_xpm },
    { "wxComboBox", wxComboBox_xpm },
    { "dataview_ctrl", dataview_ctrl_xpm },
    { "dataviewlist_column", dataviewlist_column_xpm },
    { "dataviewlist_ctrl", dataviewlist_ctrl_xpm },
    { "dataviewtree_ctrl", dataviewtree_ctrl_xpm },
    { "datepicker", datepicker_xpm },
    { "default", default_xpm },
    { "wxDialog", wxDialog_xpm },
    { "dirPicker", dirPicker_xpm },
    { "doc_mdi_parent_frame", doc_mdi_parent_frame_xpm },
    { "doc_parent_frame", doc_parent_frame_xpm },
    { "document", document_xpm },
    { "filePicker", filePicker_xpm },
    { "flex_grid_sizer", flex_grid_sizer_xpm },
    { "fontPicker", fontPicker_xpm },
    { "wxFrame", wxFrame_xpm },
    { "gauge", gauge_xpm },
    { "generate", generate_xpm },
    { "genericdir_ctrl", genericdir_ctrl_xpm },
    { "grid", grid_xpm },
    { "grid_bag_sizer", grid_bag_sizer_xpm },
    { "grid_sizer", grid_sizer_xpm },
    { "hidden", hidden_xpm },
    { "htmlwin", htmlwin_xpm },
    { "hyperlink_ctrl", hyperlink_ctrl_xpm },
    { "infobar", infobar_xpm },
    { "left", left_xpm },
    { "wxListBox", wxListBox_xpm },
    { "wxListView", wxListView_xpm },
    { "wxListbook", wxListbook_xpm },
#if defined(_DEBUG)
    { "logo16", debug_16_xpm },
    { "logo32", debug_32_xpm },
#else
    { "logo16", logo16_xpm },
    { "logo32", logo32_xpm },
#endif  // _DEBUG
    { "menu", menu_xpm },
    { "wxMenuBar", wxMenuBar_xpm },
    { "menuitem", menuitem_xpm },
    { "moveColLeft", moveColLeft_xpm },
    { "moveColRight", moveColRight_xpm },
    { "moveRowDown", moveRowDown_xpm },
    { "moveRowUp", moveRowUp_xpm },
    { "wxNotebook", wxNotebook_xpm },
    { "wxPanel", wxPanel_xpm },
    { "project", project_xpm },
    { "wxPropertyGrid", wxPropertyGrid_xpm },
    { "propgriditem", propgriditem_xpm },
    { "wxPropertyGridManager", wxPropertyGridManager_xpm },
    { "propgridpage", propgridpage_xpm },
    { "radio_box", radio_box_xpm },
    { "wxRadioButton", wxRadioButton_xpm },
    { "ribbon_bar", ribbon_bar_xpm },
    { "ribbon_button", ribbon_button_xpm },
    { "ribbon_button_dropdown", ribbon_button_dropdown_xpm },
    { "ribbon_button_hybrid", ribbon_button_hybrid_xpm },
    { "ribbon_button_toggle", ribbon_button_toggle_xpm },
    { "ribbon_buttonbar", ribbon_buttonbar_xpm },
    { "ribbon_gallery", ribbon_gallery_xpm },
    { "ribbon_gallery_item", ribbon_gallery_item_xpm },
    { "ribbon_page", ribbon_page_xpm },
    { "ribbon_panel", ribbon_panel_xpm },
    { "richtextctrl", richtextctrl_xpm },
    { "scintilla", scintilla_xpm },
    { "scrollbar", scrollbar_xpm },
    { "wxScrolledWindow", wxScrolledWindow_xpm },
    { "search", search_xpm },
    { "separator", separator_xpm },
    { "wxSimplebook", wxSimplebook_xpm },
    { "sizer", sizer_xpm },
    { "sizer_horizontal", sizer_horizontal_xpm },
    { "slider", slider_xpm },
    { "spacer", spacer_xpm },
    { "spin_ctrl", spin_ctrl_xpm },
    { "spin_ctrl_double", spin_ctrl_double_xpm },
    { "spinbtn", spinbtn_xpm },
    { "wxSplitterWindow", wxSplitterWindow_xpm },
    { "static_bitmap", static_bitmap_xpm },
    { "static_line", static_line_xpm },
    { "wxStaticText", wxStaticText_xpm },
    { "staticbox_sizer", staticbox_sizer_xpm },
    { "statusbar", statusbar_xpm },
    { "stddialogbuttonsizer", stddialogbuttonsizer_xpm },
    { "submenu", submenu_xpm },
    { "wxTextCtrl", wxTextCtrl_xpm },
    { "text_sizer", text_sizer_xpm },
    { "timepicker", timepicker_xpm },
    { "timer", timer_xpm },
    { "toggle_button", toggle_button_xpm },
    { "tool", tool_xpm },
    { "wxToolBar", wxToolBar_xpm },
    { "toolseparator", toolseparator_xpm },
    { "tree_ctrl", tree_ctrl_xpm },
    { "treelistctrl", treelistctrl_xpm },
    { "treelistctrlcolumn", treelistctrlcolumn_xpm },
    { "unknown", unknown_xpm },
    { "wxWizard", wxWizard_xpm },
    { "wxWizardPageSimple", wxWizardPageSimple_xpm },
    { "wrap_sizer", wrap_sizer_xpm },

};

wxImage GetXPMImage(ttlib::cview name)
{
    if (auto result = map_xpm_strings.find(name.c_str()); result != map_xpm_strings.end())
        return wxImage(result->second);
    else
        return wxImage(map_xpm_strings.find("default")->second);
}
