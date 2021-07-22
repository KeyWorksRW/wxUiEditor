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

#include <wx/bitmap.h>  // wxBitmap class interface
#include <wx/icon.h>    // wxIcon base header

#include "bitmaps.h"  // Contains various images handling functions
#include "utils.h"    // Utility functions that work with properties

#include "art_headers/default_png.hxx"
#include "art_headers/unknown_png.hxx"

#include "art_headers/nav_coll_expand_png.hxx"
#include "art_headers/nav_collapse_png.hxx"
#include "art_headers/nav_expand_png.hxx"
#include "art_headers/nav_movedown_png.hxx"
#include "art_headers/nav_moveleft_png.hxx"
#include "art_headers/nav_moveright_png.hxx"
#include "art_headers/nav_moveup_png.hxx"

#include "art_headers/flex_grid_sizer_png.hxx"
#include "art_headers/grid_bag_sizer_png.hxx"
#include "art_headers/grid_sizer_png.hxx"
#include "art_headers/sizer_horizontal_png.hxx"
#include "art_headers/sizer_png.hxx"
#include "art_headers/slider_png.hxx"
#include "art_headers/spacer_png.hxx"
#include "art_headers/stddialogbuttonsizer_png.hxx"
#include "art_headers/text_sizer_png.hxx"
#include "art_headers/wrap_sizer_png.hxx"
#include "art_headers/wxStaticBoxSizer_png.hxx"
#include "art_headers/wxStaticCheckBoxSizer_png.hxx"
#include "art_headers/wxStaticRadioBtnSizer_png.hxx"

#include "art_headers/auinotebook_png.hxx"
#include "art_headers/auitoolbar_png.hxx"
#include "art_headers/bitmap_button_png.hxx"
#include "art_headers/bitmap_toggle_button_png.hxx"
#include "art_headers/bmp_combo_box_png.hxx"
#include "art_headers/book_page_png.hxx"
#include "art_headers/calendar_png.hxx"
#include "art_headers/check3state_png.hxx"
#include "art_headers/colourPickerIcon_png.hxx"
#include "art_headers/CustomControl_png.hxx"
#include "art_headers/dataview_ctrl_png.hxx"
#include "art_headers/dataviewlist_column_png.hxx"
#include "art_headers/dataviewlist_ctrl_png.hxx"
#include "art_headers/dataviewtree_ctrl_png.hxx"
#include "art_headers/datepicker_png.hxx"
#include "art_headers/dirPicker_png.hxx"
#include "art_headers/doc_mdi_parent_frame_png.hxx"
#include "art_headers/doc_parent_frame_png.hxx"
#include "art_headers/document_png.hxx"
#include "art_headers/empty_png.hxx"
#include "art_headers/filePicker_png.hxx"
#include "art_headers/fontPicker_png.hxx"
#include "art_headers/gauge_png.hxx"
#include "art_headers/genericdir_ctrl_png.hxx"
#include "art_headers/grid_png.hxx"
#include "art_headers/htmlwin_png.hxx"
#include "art_headers/hyperlink_ctrl_png.hxx"
#include "art_headers/infobar_png.hxx"
#include "art_headers/menu_png.hxx"
#include "art_headers/menuitem_png.hxx"
#include "art_headers/moveColLeft_png.hxx"
#include "art_headers/moveColRight_png.hxx"
#include "art_headers/moveRowDown_png.hxx"
#include "art_headers/moveRowUp_png.hxx"
#include "art_headers/project_png.hxx"
#include "art_headers/propgriditem_png.hxx"
#include "art_headers/propgridpage_png.hxx"
#include "art_headers/radio_box_png.hxx"
#include "art_headers/ribbon_bar_png.hxx"
#include "art_headers/ribbon_button_dropdown_png.hxx"
#include "art_headers/ribbon_button_hybrid_png.hxx"
#include "art_headers/ribbon_button_png.hxx"
#include "art_headers/ribbon_button_toggle_png.hxx"
#include "art_headers/ribbon_buttonbar_png.hxx"
#include "art_headers/ribbon_gallery_item_png.hxx"
#include "art_headers/ribbon_gallery_png.hxx"
#include "art_headers/ribbon_page_png.hxx"
#include "art_headers/ribbon_panel_png.hxx"
#include "art_headers/richtextctrl_png.hxx"
#include "art_headers/scintilla_png.hxx"
#include "art_headers/scrollbar_png.hxx"
#include "art_headers/search_png.hxx"
#include "art_headers/separator_png.hxx"
#include "art_headers/spin_ctrl_double_png.hxx"
#include "art_headers/spin_ctrl_png.hxx"
#include "art_headers/spinbtn_png.hxx"
#include "art_headers/static_bitmap_png.hxx"
#include "art_headers/static_line_png.hxx"
#include "art_headers/statusbar_png.hxx"
#include "art_headers/submenu_png.hxx"
#include "art_headers/timepicker_png.hxx"
#include "art_headers/timer_png.hxx"
#include "art_headers/title_close_png.hxx"
#include "art_headers/toggle_button_png.hxx"
#include "art_headers/tool_png.hxx"
#include "art_headers/toolseparator_png.hxx"
#include "art_headers/tree_ctrl_png.hxx"
#include "art_headers/treelistctrl_png.hxx"
#include "art_headers/treelistctrlcolumn_png.hxx"
#include "art_headers/wxActivityIndicator_png.hxx"
#include "art_headers/wxAnimation_png.hxx"
#include "art_headers/wxBannerWindow_png.hxx"
#include "art_headers/wxButton_png.hxx"
#include "art_headers/wxCheckBox_png.hxx"
#include "art_headers/wxCheckListBox_png.hxx"
#include "art_headers/wxChoice_png.hxx"
#include "art_headers/wxChoicebook_png.hxx"
#include "art_headers/wxCollapsiblePane_png.hxx"
#include "art_headers/wxComboBox_png.hxx"
#include "art_headers/wxCommandLinkButton_png.hxx"
#include "art_headers/wxDialog_png.hxx"
#include "art_headers/wxFileCtrl_png.hxx"
#include "art_headers/wxFrame_png.hxx"
#include "art_headers/wxListBox_png.hxx"
#include "art_headers/wxListView_png.hxx"
#include "art_headers/wxListbook_png.hxx"
#include "art_headers/wxMenuBar_png.hxx"
#include "art_headers/wxNotebook_png.hxx"
#include "art_headers/wxPanel_png.hxx"
#include "art_headers/wxPopupTransientWindow_png.hxx"
#include "art_headers/wxPropertyGridManager_png.hxx"
#include "art_headers/wxPropertyGrid_png.hxx"
#include "art_headers/wxRadioButton_png.hxx"
#include "art_headers/wxRearrangeCtrl_png.hxx"
#include "art_headers/wxScrolledWindow_png.hxx"
#include "art_headers/wxSimpleHtmlListBox_png.hxx"
#include "art_headers/wxSimplebook_png.hxx"
#include "art_headers/wxSplitterWindow_png.hxx"
#include "art_headers/wxStaticText_png.hxx"
#include "art_headers/wxTextCtrl_png.hxx"
#include "art_headers/wxToolBar_png.hxx"
#include "art_headers/wxToolbook_png.hxx"
#include "art_headers/wxTreebook_png.hxx"
#include "art_headers/wxWizardPageSimple_png.hxx"
#include "art_headers/wxWizard_png.hxx"

#if defined(_DEBUG)
    #include "art_headers/debug_16_png.hxx"
    #include "art_headers/debug_32_png.hxx"
#else
    #include "art_headers/logo16_png.hxx"
    #include "art_headers/logo32_png.hxx"
#endif  // _DEBUG

struct ImageMap
{
    const char* name;
    const unsigned char* data;
    size_t size_data;
};

// Images that may be accessed more than once (typically for popup menus) should be at the front for fastest access. The rest
// are typically only loaded during application startup, and never accessed again.

static const ImageMap png_headers[] = {

    { "unknown", unknown_png, sizeof(unknown_png) },
    { "default", default_png, sizeof(default_png) },

    { "nav_movedown", nav_movedown_png, sizeof(nav_movedown_png) },
    { "nav_moveleft", nav_moveleft_png, sizeof(nav_moveleft_png) },
    { "nav_moveright", nav_moveright_png, sizeof(nav_moveright_png) },
    { "nav_moveup", nav_moveup_png, sizeof(nav_moveup_png) },

    { "nav_coll_expand", nav_coll_expand_png, sizeof(nav_coll_expand_png) },
    { "nav_collapse", nav_collapse_png, sizeof(nav_collapse_png) },
    { "nav_expand", nav_expand_png, sizeof(nav_expand_png) },

    { "flex_grid_sizer", flex_grid_sizer_png, sizeof(flex_grid_sizer_png) },
    { "grid_bag_sizer", grid_bag_sizer_png, sizeof(grid_bag_sizer_png) },
    { "grid_sizer", grid_sizer_png, sizeof(grid_sizer_png) },
    { "sizer", sizer_png, sizeof(sizer_png) },
    { "sizer_horizontal", sizer_horizontal_png, sizeof(sizer_horizontal_png) },
    { "slider", slider_png, sizeof(slider_png) },
    { "spacer", spacer_png, sizeof(spacer_png) },
    { "stddialogbuttonsizer", stddialogbuttonsizer_png, sizeof(stddialogbuttonsizer_png) },
    { "text_sizer", text_sizer_png, sizeof(text_sizer_png) },
    { "wrap_sizer", wrap_sizer_png, sizeof(wrap_sizer_png) },
    { "wxStaticBoxSizer", wxStaticBoxSizer_png, sizeof(wxStaticBoxSizer_png) },
    { "wxStaticCheckBoxSizer", wxStaticCheckBoxSizer_png, sizeof(wxStaticCheckBoxSizer_png) },
    { "wxStaticRadioBtnSizer", wxStaticRadioBtnSizer_png, sizeof(wxStaticRadioBtnSizer_png) },

    { "auinotebook", auinotebook_png, sizeof(auinotebook_png) },
    { "auitoolbar", auitoolbar_png, sizeof(auitoolbar_png) },
    { "bitmap_button", bitmap_button_png, sizeof(bitmap_button_png) },
    { "bitmap_toggle_button", bitmap_toggle_button_png, sizeof(bitmap_toggle_button_png) },
    { "bmp_combo_box", bmp_combo_box_png, sizeof(bmp_combo_box_png) },
    { "book_page", book_page_png, sizeof(book_page_png) },
    { "calendar", calendar_png, sizeof(calendar_png) },
    { "check3state", check3state_png, sizeof(check3state_png) },
    { "colourPickerIcon", colourPickerIcon_png, sizeof(colourPickerIcon_png) },
    { "CustomControl", CustomControl_png, sizeof(CustomControl_png) },
    { "dataview_ctrl", dataview_ctrl_png, sizeof(dataview_ctrl_png) },
    { "dataviewlist_column", dataviewlist_column_png, sizeof(dataviewlist_column_png) },
    { "dataviewlist_ctrl", dataviewlist_ctrl_png, sizeof(dataviewlist_ctrl_png) },
    { "dataviewtree_ctrl", dataviewtree_ctrl_png, sizeof(dataviewtree_ctrl_png) },
    { "datepicker", datepicker_png, sizeof(datepicker_png) },
    { "default", default_png, sizeof(default_png) },
    { "dirPicker", dirPicker_png, sizeof(dirPicker_png) },
    { "doc_mdi_parent_frame", doc_mdi_parent_frame_png, sizeof(doc_mdi_parent_frame_png) },
    { "doc_parent_frame", doc_parent_frame_png, sizeof(doc_parent_frame_png) },
    { "document", document_png, sizeof(document_png) },
    { "filePicker", filePicker_png, sizeof(filePicker_png) },
    { "fontPicker", fontPicker_png, sizeof(fontPicker_png) },
    { "gauge", gauge_png, sizeof(gauge_png) },
    { "genericdir_ctrl", genericdir_ctrl_png, sizeof(genericdir_ctrl_png) },
    { "grid", grid_png, sizeof(grid_png) },
    { "htmlwin", htmlwin_png, sizeof(htmlwin_png) },
    { "hyperlink_ctrl", hyperlink_ctrl_png, sizeof(hyperlink_ctrl_png) },
    { "infobar", infobar_png, sizeof(infobar_png) },
    { "menu", menu_png, sizeof(menu_png) },
    { "menuitem", menuitem_png, sizeof(menuitem_png) },
    { "moveColLeft", moveColLeft_png, sizeof(moveColLeft_png) },
    { "moveColRight", moveColRight_png, sizeof(moveColRight_png) },
    { "moveRowDown", moveRowDown_png, sizeof(moveRowDown_png) },
    { "moveRowUp", moveRowUp_png, sizeof(moveRowUp_png) },
    { "project", project_png, sizeof(project_png) },
    { "propgriditem", propgriditem_png, sizeof(propgriditem_png) },
    { "propgridpage", propgridpage_png, sizeof(propgridpage_png) },
    { "radio_box", radio_box_png, sizeof(radio_box_png) },
    { "ribbon_bar", ribbon_bar_png, sizeof(ribbon_bar_png) },
    { "ribbon_button", ribbon_button_png, sizeof(ribbon_button_png) },
    { "ribbon_button_dropdown", ribbon_button_dropdown_png, sizeof(ribbon_button_dropdown_png) },
    { "ribbon_button_hybrid", ribbon_button_hybrid_png, sizeof(ribbon_button_hybrid_png) },
    { "ribbon_button_toggle", ribbon_button_toggle_png, sizeof(ribbon_button_toggle_png) },
    { "ribbon_buttonbar", ribbon_buttonbar_png, sizeof(ribbon_buttonbar_png) },
    { "ribbon_gallery", ribbon_gallery_png, sizeof(ribbon_gallery_png) },
    { "ribbon_gallery_item", ribbon_gallery_item_png, sizeof(ribbon_gallery_item_png) },
    { "ribbon_page", ribbon_page_png, sizeof(ribbon_page_png) },
    { "ribbon_panel", ribbon_panel_png, sizeof(ribbon_panel_png) },
    { "richtextctrl", richtextctrl_png, sizeof(richtextctrl_png) },
    { "scintilla", scintilla_png, sizeof(scintilla_png) },
    { "scrollbar", scrollbar_png, sizeof(scrollbar_png) },
    { "search", search_png, sizeof(search_png) },
    { "separator", separator_png, sizeof(separator_png) },
    { "spin_ctrl", spin_ctrl_png, sizeof(spin_ctrl_png) },
    { "spin_ctrl_double", spin_ctrl_double_png, sizeof(spin_ctrl_double_png) },
    { "spinbtn", spinbtn_png, sizeof(spinbtn_png) },
    { "static_bitmap", static_bitmap_png, sizeof(static_bitmap_png) },
    { "static_line", static_line_png, sizeof(static_line_png) },
    { "statusbar", statusbar_png, sizeof(statusbar_png) },
    { "submenu", submenu_png, sizeof(submenu_png) },
    { "timepicker", timepicker_png, sizeof(timepicker_png) },
    { "timer", timer_png, sizeof(timer_png) },
    { "toggle_button", toggle_button_png, sizeof(toggle_button_png) },
    { "tool", tool_png, sizeof(tool_png) },
    { "toolseparator", toolseparator_png, sizeof(toolseparator_png) },
    { "tree_ctrl", tree_ctrl_png, sizeof(tree_ctrl_png) },
    { "treelistctrl", treelistctrl_png, sizeof(treelistctrl_png) },
    { "treelistctrlcolumn", treelistctrlcolumn_png, sizeof(treelistctrlcolumn_png) },
    { "wxActivityIndicator", wxActivityIndicator_png, sizeof(wxActivityIndicator_png) },
    { "wxAnimation", wxAnimation_png, sizeof(wxAnimation_png) },
    { "wxBannerWindow", wxBannerWindow_png, sizeof(wxBannerWindow_png) },
    { "wxButton", wxButton_png, sizeof(wxButton_png) },
    { "wxCheckBox", wxCheckBox_png, sizeof(wxCheckBox_png) },
    { "wxCheckListBox", wxCheckListBox_png, sizeof(wxCheckListBox_png) },
    { "wxChoice", wxChoice_png, sizeof(wxChoice_png) },
    { "wxChoicebook", wxChoicebook_png, sizeof(wxChoicebook_png) },
    { "wxCollapsiblePane", wxCollapsiblePane_png, sizeof(wxCollapsiblePane_png) },
    { "wxComboBox", wxComboBox_png, sizeof(wxComboBox_png) },
    { "wxCommandLinkButton", wxCommandLinkButton_png, sizeof(wxCommandLinkButton_png) },
    { "wxDialog", wxDialog_png, sizeof(wxDialog_png) },
    { "wxFileCtrl", wxFileCtrl_png, sizeof(wxFileCtrl_png) },
    { "wxFrame", wxFrame_png, sizeof(wxFrame_png) },
    { "wxListBox", wxListBox_png, sizeof(wxListBox_png) },
    { "wxListView", wxListView_png, sizeof(wxListView_png) },
    { "wxListbook", wxListbook_png, sizeof(wxListbook_png) },
    { "wxMenuBar", wxMenuBar_png, sizeof(wxMenuBar_png) },
    { "wxNotebook", wxNotebook_png, sizeof(wxNotebook_png) },
    { "wxPanel", wxPanel_png, sizeof(wxPanel_png) },
    { "wxPopupTransientWindow", wxPopupTransientWindow_png, sizeof(wxPopupTransientWindow_png) },
    { "wxPropertyGrid", wxPropertyGrid_png, sizeof(wxPropertyGrid_png) },
    { "wxPropertyGridManager", wxPropertyGridManager_png, sizeof(wxPropertyGridManager_png) },
    { "wxRadioButton", wxRadioButton_png, sizeof(wxRadioButton_png) },
    { "wxRearrangeCtrl", wxRearrangeCtrl_png, sizeof(wxRearrangeCtrl_png) },
    { "wxScrolledWindow", wxScrolledWindow_png, sizeof(wxScrolledWindow_png) },
    { "wxSimpleHtmlListBox", wxSimpleHtmlListBox_png, sizeof(wxSimpleHtmlListBox_png) },
    { "wxSimplebook", wxSimplebook_png, sizeof(wxSimplebook_png) },
    { "wxSplitterWindow", wxSplitterWindow_png, sizeof(wxSplitterWindow_png) },
    { "wxStaticText", wxStaticText_png, sizeof(wxStaticText_png) },
    { "wxTextCtrl", wxTextCtrl_png, sizeof(wxTextCtrl_png) },
    { "wxToolBar", wxToolBar_png, sizeof(wxToolBar_png) },
    { "wxToolbook", wxToolbook_png, sizeof(wxToolbook_png) },
    { "wxTreebook", wxTreebook_png, sizeof(wxTreebook_png) },
    { "wxWizard", wxWizard_png, sizeof(wxWizard_png) },
    { "wxWizardPageSimple", wxWizardPageSimple_png, sizeof(wxWizardPageSimple_png) },

// A different Icon is used for debug builds so that it is easier to identify that a debug build is being run.

#if defined(_DEBUG)
    { "logo16", debug_16_png, sizeof(debug_16_png) },
    { "logo32", debug_32_png, sizeof(debug_32_png) }
#else
    { "logo16", logo16_png, sizeof(logo16_png) },
    { "logo32", logo32_png, sizeof(logo32_png) }
#endif

};

wxImage GetInternalImage(ttlib::cview name)
{
    for (auto& iter: png_headers)
    {
        if (name.is_sameas(iter.name))
        {
            return LoadHeaderImage(iter.data, iter.size_data);
        }
    }

    // If all else fails, return a default 16x16 image
    return LoadHeaderImage(default_png, sizeof(default_png));
}

wxIcon GetIconImage(ttlib::cview name)
{
    for (auto& iter: png_headers)
    {
        if (name.is_sameas(iter.name))
        {
            auto image = LoadHeaderImage(iter.data, iter.size_data);
            image.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD);
            wxIcon icon;
            icon.CopyFromBitmap(image);
            return icon;
        }
    }

    auto image = GetInternalImage(name);
    wxIcon icon;
    icon.CopyFromBitmap(image);
    return icon;
}
