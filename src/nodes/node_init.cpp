/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize NodeCreator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <format>
#include <functional>

#include <frozen/set.h>

#include "node_creator.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "bitmaps.h"         // Contains various images handling functions
#include "gen_enums.h"       // Enumerations for generators
#include "mainapp.h"         // App -- Main application class
#include "node.h"            // Node class
#include "node_types.h"      // NodeType -- Class for storing node types and allowable child count
#include "prop_decl.h"       // PropChildDeclaration and PropDeclaration classes
#include "wxue_data.h"       // Generated wxue_data strings and functions

#include "pugixml.hpp"

// clang-format off

using namespace wxue_data;

// These functions are declared in the wxue_data namespace, and retrieve the XML data as a string.
// The data is compressed, so each function will decompress the data before returning it. See
// wxue_data.h for the original xml file that was converted into a compressed string by wxUiEditor.
const auto functionArray = std::to_array<std::function<std::string()>>({
    get_bars,
    get_boxes,
    get_buttons,
    get_containers,
    get_data_ctrls,
    get_forms,
    get_mdi,
    get_pickers,
    get_project,
    get_sizers,
    get_text_ctrls,
    get_widgets,
});

// var_names for these generators will default to "none" for class access
// inline const GenName set_no_class_access[] = {
constexpr auto set_no_class_access = frozen::make_set<GenName>({

    gen_BookPage,
    gen_CloseButton,
    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,
    gen_TextSizer,
    gen_VerticalBoxSizer,
    gen_auitool,
    gen_auitool_label,
    gen_separator,
    gen_submenu,
    gen_tool,
    gen_tool_dropdown,
    gen_wxBoxSizer,
    gen_wxFlexGridSizer,
    gen_wxGridBagSizer,
    gen_wxGridSizer,
    gen_wxMenuItem,
    gen_wxPanel,
    gen_wxRibbonButtonBar,
    gen_wxRibbonGallery,
    gen_wxRibbonPage,
    gen_wxRibbonPanel,
    gen_wxRibbonToolBar,
    gen_wxStaticBitmap,
    gen_wxStaticBoxSizer,
    gen_wxStaticLine,
    gen_wxStdDialogButtonSizer,
    gen_wxWizardPageSimple,
    gen_wxWrapSizer,

});

// clang-format on

using namespace child_count;
using namespace GenEnum;

struct ParentChild
{
    GenType parent;
    GenType child;

    ptrdiff_t max_children;
};

// A child node can only be created if it is listed below as valid for the current parent.
constexpr auto lstParentChild = std::to_array<ParentChild>({

    // Books

    ParentChild { .parent = type_bookpage, .child = type_gbsizer, .max_children = one },
    ParentChild { .parent = type_bookpage, .child = type_sizer, .max_children = one },
    ParentChild { .parent = type_bookpage,
                  .child = type_bookpage,
                  .max_children = infinite },  // only valid when grandparent is a wxTreebook
    ParentChild { .parent = type_bookpage, .child = type_widget, .max_children = infinite },

    ParentChild { .parent = type_page, .child = type_auinotebook, .max_children = one },
    ParentChild { .parent = type_page, .child = type_choicebook, .max_children = one },
    ParentChild { .parent = type_page, .child = type_container, .max_children = one },
    ParentChild { .parent = type_page, .child = type_dataviewctrl, .max_children = one },
    ParentChild { .parent = type_page, .child = type_dataviewlistctrl, .max_children = one },
    ParentChild { .parent = type_page, .child = type_dataviewtreectrl, .max_children = one },
    ParentChild { .parent = type_page, .child = type_listbook, .max_children = one },
    ParentChild { .parent = type_page, .child = type_notebook, .max_children = one },
    ParentChild { .parent = type_page, .child = type_panel, .max_children = one },
    ParentChild { .parent = type_page, .child = type_propgrid, .max_children = one },
    ParentChild { .parent = type_page, .child = type_propgridman, .max_children = one },
    ParentChild { .parent = type_page, .child = type_ribbonbar, .max_children = one },
    ParentChild { .parent = type_page, .child = type_simplebook, .max_children = one },
    ParentChild { .parent = type_page, .child = type_splitter, .max_children = one },
    ParentChild { .parent = type_page, .child = type_treelistctrl, .max_children = one },
    ParentChild { .parent = type_page, .child = type_widget, .max_children = one },

    ParentChild { .parent = type_choicebook, .child = type_bookpage, .max_children = infinite },
    ParentChild { .parent = type_choicebook, .child = type_page, .max_children = infinite },
    ParentChild { .parent = type_choicebook,
                  .child = type_widget,
                  .max_children = infinite },  // The only book that allows adding a widget
    ParentChild { .parent = type_listbook, .child = type_bookpage, .max_children = infinite },
    ParentChild { .parent = type_listbook, .child = type_page, .max_children = infinite },
    ParentChild { .parent = type_notebook, .child = type_bookpage, .max_children = infinite },
    ParentChild { .parent = type_notebook, .child = type_page, .max_children = infinite },
    ParentChild { .parent = type_simplebook, .child = type_bookpage, .max_children = infinite },
    ParentChild { .parent = type_simplebook, .child = type_page, .max_children = infinite },

    // Menus

    ParentChild { .parent = type_menu, .child = type_menuitem, .max_children = infinite },
    ParentChild { .parent = type_menu, .child = type_submenu, .max_children = infinite },
    ParentChild { .parent = type_menubar, .child = type_menu, .max_children = infinite },

    ParentChild { .parent = type_submenu, .child = type_menuitem, .max_children = infinite },
    ParentChild { .parent = type_submenu, .child = type_submenu, .max_children = infinite },

    ParentChild { .parent = type_popup_menu, .child = type_menuitem, .max_children = infinite },
    ParentChild { .parent = type_popup_menu, .child = type_submenu, .max_children = infinite },

    ParentChild { .parent = type_ctx_menu, .child = type_menuitem, .max_children = infinite },
    ParentChild { .parent = type_ctx_menu, .child = type_submenu, .max_children = infinite },

    // Forms

    ParentChild { .parent = type_frame_form, .child = type_gbsizer, .max_children = one },
    ParentChild { .parent = type_frame_form, .child = type_sizer, .max_children = one },

    ParentChild { .parent = type_frame_form, .child = type_container, .max_children = infinite },
    ParentChild { .parent = type_frame_form, .child = type_panel, .max_children = infinite },
    ParentChild { .parent = type_frame_form, .child = type_splitter, .max_children = infinite },

    ParentChild { .parent = type_frame_form, .child = type_statusbar, .max_children = one },
    ParentChild { .parent = type_frame_form, .child = type_toolbar, .max_children = one },
    ParentChild { .parent = type_frame_form, .child = type_aui_toolbar, .max_children = one },
    ParentChild { .parent = type_frame_form, .child = type_menubar, .max_children = one },
    ParentChild { .parent = type_frame_form, .child = type_ctx_menu, .max_children = one },
    ParentChild { .parent = type_frame_form, .child = type_timer, .max_children = infinite },

    ParentChild { .parent = type_frame_form, .child = type_choicebook, .max_children = infinite },
    ParentChild { .parent = type_frame_form, .child = type_listbook, .max_children = infinite },
    ParentChild { .parent = type_frame_form, .child = type_simplebook, .max_children = infinite },
    ParentChild { .parent = type_frame_form, .child = type_notebook, .max_children = infinite },
    ParentChild { .parent = type_frame_form, .child = type_widget, .max_children = infinite },

    ParentChild { .parent = type_form, .child = type_ctx_menu, .max_children = one },
    ParentChild { .parent = type_form, .child = type_gbsizer, .max_children = one },
    ParentChild { .parent = type_form, .child = type_sizer, .max_children = one },
    ParentChild { .parent = type_form, .child = type_timer, .max_children = infinite },

    ParentChild { .parent = type_panel_form, .child = type_gbsizer, .max_children = one },
    ParentChild { .parent = type_panel_form, .child = type_sizer, .max_children = one },

    ParentChild { .parent = type_panel_form, .child = type_ctx_menu, .max_children = one },
    ParentChild { .parent = type_panel_form, .child = type_timer, .max_children = infinite },

    ParentChild { .parent = type_panel_form, .child = type_aui_toolbar, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_panel_form, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_splitter, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_toolbar, .max_children = infinite },

    ParentChild { .parent = type_panel_form, .child = type_auinotebook, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_choicebook, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_container, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_dataviewctrl, .max_children = infinite },
    ParentChild {
        .parent = type_panel_form, .child = type_dataviewlistctrl, .max_children = infinite },
    ParentChild {
        .parent = type_panel_form, .child = type_dataviewtreectrl, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_listbook, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_notebook, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_propgrid, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_propgridman, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_ribbonbar, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_simplebook, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_splitter, .max_children = infinite },
    ParentChild { .parent = type_panel_form, .child = type_treelistctrl, .max_children = infinite },

    ParentChild { .parent = type_panel_form, .child = type_widget, .max_children = infinite },

    ParentChild { .parent = type_propsheetform, .child = type_bookpage, .max_children = infinite },

    ParentChild { .parent = type_menubar_form, .child = type_menu, .max_children = infinite },
    ParentChild {
        .parent = type_ribbonbar_form, .child = type_ribbonpage, .max_children = infinite },
    ParentChild { .parent = type_toolbar_form, .child = type_tool, .max_children = infinite },
    ParentChild {
        .parent = type_toolbar_form, .child = type_tool_separator, .max_children = infinite },
    ParentChild { .parent = type_toolbar_form, .child = type_widget, .max_children = infinite },
    ParentChild {
        .parent = type_aui_toolbar_form, .child = type_aui_tool, .max_children = infinite },
    ParentChild {
        .parent = type_aui_toolbar_form, .child = type_tool_separator, .max_children = infinite },
    ParentChild { .parent = type_aui_toolbar_form, .child = type_widget, .max_children = infinite },

    ParentChild { .parent = type_data_list, .child = type_data_string, .max_children = infinite },
    ParentChild { .parent = type_data_list, .child = type_data_folder, .max_children = infinite },
    ParentChild { .parent = type_data_folder, .child = type_data_string, .max_children = infinite },
    ParentChild { .parent = type_images, .child = type_embed_image, .max_children = infinite },
    ParentChild { .parent = type_wizard, .child = type_wizardpagesimple, .max_children = infinite },

    ParentChild { .parent = type_project, .child = type_data_list, .max_children = one },
    ParentChild { .parent = type_project, .child = type_form, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_folder, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_frame_form, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_images, .max_children = one },
    ParentChild { .parent = type_project, .child = type_menubar_form, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_panel_form, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_popup_menu, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_ribbonbar_form, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_toolbar_form, .max_children = infinite },
    ParentChild {
        .parent = type_project, .child = type_aui_toolbar_form, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_wizard, .max_children = infinite },
    ParentChild { .parent = type_project, .child = type_propsheetform, .max_children = infinite },

    // Folders and sub-folders

    ParentChild { .parent = type_folder, .child = type_sub_folder, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_form, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_frame_form, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_menubar_form, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_panel_form, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_popup_menu, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_ribbonbar_form, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_toolbar_form, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_aui_toolbar_form, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_wizard, .max_children = infinite },
    ParentChild { .parent = type_folder, .child = type_DocViewApp, .max_children = one },
    ParentChild { .parent = type_folder, .child = type_propsheetform, .max_children = infinite },

    ParentChild { .parent = type_sub_folder, .child = type_form, .max_children = infinite },
    ParentChild { .parent = type_sub_folder, .child = type_sub_folder, .max_children = infinite },
    ParentChild { .parent = type_sub_folder, .child = type_frame_form, .max_children = infinite },
    ParentChild { .parent = type_sub_folder, .child = type_menubar_form, .max_children = infinite },
    ParentChild { .parent = type_sub_folder, .child = type_panel_form, .max_children = infinite },
    ParentChild { .parent = type_sub_folder, .child = type_popup_menu, .max_children = infinite },
    ParentChild {
        .parent = type_sub_folder, .child = type_ribbonbar_form, .max_children = infinite },
    ParentChild { .parent = type_sub_folder, .child = type_toolbar_form, .max_children = infinite },
    ParentChild {
        .parent = type_sub_folder, .child = type_aui_toolbar_form, .max_children = infinite },
    ParentChild { .parent = type_sub_folder, .child = type_wizard, .max_children = infinite },
    ParentChild {
        .parent = type_sub_folder, .child = type_propsheetform, .max_children = infinite },

    // MDI
    ParentChild { .parent = type_DocViewApp, .child = type_wx_document, .max_children = infinite },
    ParentChild { .parent = type_DocViewApp, .child = type_mdi_menubar, .max_children = one },
    ParentChild { .parent = type_wx_document, .child = type_doc_menubar, .max_children = one },
    ParentChild { .parent = type_wx_document, .child = type_wx_view, .max_children = infinite },

    // Containers

    ParentChild { .parent = type_container, .child = type_gbsizer, .max_children = one },
    ParentChild { .parent = type_container, .child = type_sizer, .max_children = one },

    ParentChild { .parent = type_panel, .child = type_gbsizer, .max_children = one },
    ParentChild { .parent = type_panel, .child = type_sizer, .max_children = one },

    ParentChild { .parent = type_panel, .child = type_ctx_menu, .max_children = one },
    ParentChild { .parent = type_panel, .child = type_timer, .max_children = infinite },

    ParentChild { .parent = type_panel, .child = type_aui_toolbar, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_panel, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_splitter, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_toolbar, .max_children = infinite },

    ParentChild { .parent = type_panel, .child = type_auinotebook, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_choicebook, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_container, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_dataviewctrl, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_dataviewlistctrl, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_dataviewtreectrl, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_listbook, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_notebook, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_propgrid, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_propgridman, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_ribbonbar, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_simplebook, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_splitter, .max_children = infinite },
    ParentChild { .parent = type_panel, .child = type_treelistctrl, .max_children = infinite },

    ParentChild { .parent = type_panel, .child = type_widget, .max_children = infinite },

    // DataView

    ParentChild {
        .parent = type_dataviewctrl, .child = type_dataviewcolumn, .max_children = infinite },
    ParentChild { .parent = type_dataviewlistctrl,
                  .child = type_dataviewlistcolumn,
                  .max_children = infinite },

    ParentChild { .parent = type_propgrid, .child = type_propgriditem, .max_children = infinite },
    ParentChild {
        .parent = type_propgrid, .child = type_propgrid_category, .max_children = infinite },
    ParentChild {
        .parent = type_propgrid_category, .child = type_propgriditem, .max_children = infinite },
    ParentChild {
        .parent = type_propgriditem, .child = type_propgridpage, .max_children = infinite },
    ParentChild {
        .parent = type_propgridman, .child = type_propgridpage, .max_children = infinite },
    ParentChild {
        .parent = type_propgridpage, .child = type_propgrid_category, .max_children = infinite },
    ParentChild {
        .parent = type_propgridpage, .child = type_propgriditem, .max_children = infinite },

    // Ribbon bar

    ParentChild { .parent = type_ribbonbar, .child = type_ribbonpage, .max_children = infinite },
    ParentChild {
        .parent = type_ribbonbuttonbar, .child = type_ribbonbutton, .max_children = infinite },
    ParentChild {
        .parent = type_ribbongallery, .child = type_ribbongalleryitem, .max_children = infinite },
    ParentChild { .parent = type_ribbonpage, .child = type_ribbonpanel, .max_children = infinite },
    ParentChild { .parent = type_ribbonpanel, .child = type_ribbonbuttonbar, .max_children = one },
    ParentChild { .parent = type_ribbonpanel, .child = type_ribbongallery, .max_children = one },
    ParentChild { .parent = type_ribbonpanel, .child = type_ribbontoolbar, .max_children = one },
    ParentChild { .parent = type_ribbonpanel, .child = type_sizer, .max_children = one },
    ParentChild { .parent = type_ribbonpanel, .child = type_gbsizer, .max_children = one },
    ParentChild {
        .parent = type_ribbontoolbar, .child = type_ribbontool, .max_children = infinite },
    ParentChild {
        .parent = type_ribbontoolbar, .child = type_ribbontool, .max_children = infinite },

    // Sizers

    ParentChild { .parent = type_sizer, .child = type_aui_toolbar, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_auinotebook, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_choicebook, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_container, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_dataviewctrl, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_dataviewlistctrl, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_dataviewtreectrl, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_gbsizer, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_listbook, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_notebook, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_panel, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_propgrid, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_propgridman, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_ribbonbar, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_simplebook, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_sizer, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_splitter, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_staticbox, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_toolbar, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_treelistctrl, .max_children = infinite },
    ParentChild { .parent = type_sizer, .child = type_widget, .max_children = infinite },

    // Toolbars

    ParentChild { .parent = type_aui_toolbar, .child = type_aui_tool, .max_children = infinite },
    ParentChild {
        .parent = type_aui_toolbar, .child = type_tool_separator, .max_children = infinite },
    ParentChild { .parent = type_aui_toolbar, .child = type_widget, .max_children = infinite },
    // type_tool_dropdown only works in wxToolBar -- wxAuiToolBar requires the caller to create the
    // menu on demand

    ParentChild { .parent = type_toolbar, .child = type_tool, .max_children = infinite },
    ParentChild { .parent = type_toolbar, .child = type_tool_dropdown, .max_children = infinite },
    ParentChild { .parent = type_toolbar, .child = type_tool_separator, .max_children = infinite },
    ParentChild { .parent = type_toolbar, .child = type_widget, .max_children = infinite },
    ParentChild { .parent = type_tool_dropdown, .child = type_menuitem, .max_children = infinite },

    // wxStaticBox

    ParentChild { .parent = type_staticbox, .child = type_widget, .max_children = infinite },
    ParentChild { .parent = type_staticbox, .child = type_sizer, .max_children = infinite },
    ParentChild { .parent = type_staticbox, .child = type_gbsizer, .max_children = infinite },

    // Misc

    ParentChild { .parent = type_splitter, .child = type_auinotebook, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_choicebook, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_container, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_dataviewctrl, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_dataviewlistctrl, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_dataviewtreectrl, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_listbook, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_notebook, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_panel, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_propgrid, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_propgridman, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_simplebook, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_splitter, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_treelistctrl, .max_children = two },
    ParentChild { .parent = type_splitter, .child = type_widget, .max_children = two },

    ParentChild {
        .parent = type_treelistctrl, .child = type_treelistctrlcolumn, .max_children = infinite },

    ParentChild { .parent = type_wx_document,
                  .child = type_mdi_menubar,
                  .max_children = one },  // default menu bar when no document is loaded
    ParentChild { .parent = type_wx_document,
                  .child = type_doc_menubar,
                  .max_children = one },  // menu bar when a document is loaded
    ParentChild { .parent = type_mdi_menubar, .child = type_menu, .max_children = infinite },
    ParentChild { .parent = type_doc_menubar, .child = type_menu, .max_children = infinite },

    ParentChild { .parent = type_wizardpagesimple, .child = type_gbsizer, .max_children = one },
    ParentChild { .parent = type_wizardpagesimple, .child = type_sizer, .max_children = one },
});

// These are types used to convert wxFormBuilder projects
static constexpr auto fb_ImportTypes = std::to_array<std::string_view>({

    "sizeritem",
    "gbsizeritem",
    "splitteritem",

    "oldbookpage",

});

void NodeCreator::Initialize()
{
    // REVIEW: [Randalphwa - 11-15-2025]
    // Make certain these are cleared to avoid possible memory corruption.
    rmap_PropNames.clear();
    map_MacroProps.clear();
    rmap_GenNames.clear();

    for (const auto& iter: GenEnum::map_PropNames)
    {
        GenEnum::rmap_PropNames[iter.second] = iter.first;
    }

    for (const auto& iter: map_PropMacros)
    {
        map_MacroProps[iter.second] = iter.first;
    }

    for (const auto& iter: GenEnum::map_GenNames)
    {
        rmap_GenNames[iter.second] = iter.first;
    }

    for (const auto& iter: map_GenTypes)
    {
        m_a_node_types.at(static_cast<size_t>(iter.first)).Create(iter.first);
    }

    for (const auto& iter: lstParentChild)
    {
        get_NodeType(iter.parent)->AddChild(iter.child, iter.max_children);
        if (iter.parent == type_sizer)
        {
            get_NodeType(type_gbsizer)->AddChild(iter.child, iter.max_children);
        }
    }

    {
        pugi::xml_document interface_doc;
        m_pdoc_interface = &interface_doc;

        auto result = interface_doc.load_string(wxue_data::get_interfaces());
        if (!result)
        {
            FAIL_MSG("xml/interfaces.xml is corrupted!");
            throw std::runtime_error("Internal XML file is corrupted.");
        }

        // Now parse the completed m_pdoc_interface document
        ParseGeneratorFile("");

        /*
         [Randalphwa - 11-09-2025] I looked into turning this into threaded code, but it's not
         really practical:

        - XML decompression (in wxue_data functions) is already CPU-intensive and would saturate
         cores
        - Lock contention on m_a_declarations would serialize the critical sections anyway
         - Memory allocation for NodeDeclaration objects has its own internal synchronization
         overhead
        - Only 12 iterations (based on functionArray size) - insufficient parallelism to
         overcome threading overhead

        */

        for (const auto& iter: functionArray)
        {
            auto xml_data = iter();
            if (xml_data.size())
            {
                ParseGeneratorFile(xml_data.c_str());
            }
        }

        m_interfaces.clear();
        m_pdoc_interface = nullptr;
    }

    InitGenerators();

    for (const auto& iter: fb_ImportTypes)
    {
        m_setOldHostTypes.emplace(iter);
    }
}

auto NodeCreator::DetermineGenType(pugi::xml_node& generator, bool is_interface) -> GenType
{
    if (is_interface)
    {
        return type_interface;
    }

    auto type_name = generator.attribute("type").as_view();
#if defined(_DEBUG)
    if (is_interface && type_name != "interface")
    {
        ASSERT_MSG(type_name == "interface",
                   "Don't put a non-interface class in an interace xml file!");
    }
#endif  // _DEBUG

    for (const auto& iter: map_GenTypes)
    {
        if (type_name == iter.second)
        {
            return iter.first;
        }
    }

#if defined(_DEBUG)
    ASSERT_MSG(false, std::format("Unrecognized class type -- {}", type_name));
#endif  // _DEBUG
    return gen_type_unknown;
}

void NodeCreator::SetupGeneratorImage(pugi::xml_node& generator, NodeDeclaration* declaration)
{
    auto image_name = generator.attribute("image").as_view();
    if (image_name.size())
    {
        if (auto bndl_function = GetSvgFunction(image_name); bndl_function)
        {
            declaration->SetBundleFunction(bndl_function);
        }
        else
        {
            auto image = GetInternalImage(image_name);
            if (image.GetWidth() != GenImageSize || image.GetHeight() != GenImageSize)
            {
                MSG_INFO(std::format("{} width: {}height: {}", image_name, image.GetWidth(),
                                     image.GetHeight()));
                declaration->SetImage(image.Scale(GenImageSize, GenImageSize));
            }
            else
            {
                declaration->SetImage(image);
            }
        }
    }
    else
    {
        if (auto bndl_function = GetSvgFunction("unknown"); bndl_function)
        {
            declaration->SetBundleFunction(bndl_function);
        }
        else
        {
            declaration->SetImage(GetInternalImage("unknown").Scale(GenImageSize, GenImageSize));
        }
    }
}

auto NodeCreator::ParseGenerator(pugi::xml_node& generator, bool is_interface) -> NodeDeclaration*
{
    auto class_name = generator.attribute("class").as_str();
    if (class_name.starts_with("gen_"))
    {
        class_name.erase(0, sizeof("gen_") - 1);
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        if (!rmap_GenNames.contains(class_name))
        {
            MSG_WARNING(std::format("{}{}",
                                    is_interface ? "Unrecognized interface name -- " :
                                                   "Unrecognized class name -- ",
                                    class_name));
        }
    }

    // This code makes it possible to add `enable="internal"` to an XML class/interface to
    // prevent it from being used when not testing.
    if (auto enable = generator.attribute("enable"); enable.as_view() == "internal")
    {
        if (!wxGetApp().isTestingMenuEnabled())
        {
            return nullptr;  // Skip this class if we're not testing
        }
    }

    GenType type = DetermineGenType(generator, is_interface);
    if (type == gen_type_unknown)
    {
        return nullptr;
    }

    if (is_interface)
    {
        m_interfaces[class_name] = generator;
    }

    auto* declaration = new NodeDeclaration(class_name, get_NodeType(type));
    m_a_declarations.at(declaration->get_GenName()) = declaration;

    if (auto flags = generator.attribute("flags").as_view(); flags.size())
    {
        declaration->SetGeneratorFlags(flags);
    }

    SetupGeneratorImage(generator, declaration);
    ParseProperties(generator, declaration, declaration->GetCategory());
    declaration->ParseEvents(generator, declaration->GetCategory());

    return declaration;
}

void NodeCreator::ProcessGeneratorInheritance(pugi::xml_node& elem_obj)
{
    auto class_name = elem_obj.attribute("class").as_view();
    if (class_name.starts_with("gen_"))
    {
        class_name.remove_prefix(sizeof("gen_") - 1);
    }

    auto* class_info = get_NodeDeclaration(class_name);
    if (!class_info)
    {
        return;  // Corrupted or unsupported project file
    }

    auto elem_base = elem_obj.child("inherits");
    while (elem_base)
    {
        auto base_name = elem_base.attribute("class").as_view();
        if (base_name == "Language Settings")
        {
            class_info->AddBaseClass(get_NodeDeclaration("C++ Settings"));
            class_info->AddBaseClass(get_NodeDeclaration("C++ Header Settings"));
            class_info->AddBaseClass(get_NodeDeclaration("C++ Derived Class Settings"));
            class_info->AddBaseClass(get_NodeDeclaration("wxPython Settings"));
            class_info->AddBaseClass(get_NodeDeclaration("wxRuby Settings"));
            class_info->AddBaseClass(get_NodeDeclaration("wxPerl Settings"));

            elem_base = elem_base.next_sibling("inherits");
            continue;
        }

        auto* base_info = get_NodeDeclaration(base_name);
        if (!class_info || !base_info)
        {
            elem_base = elem_base.next_sibling("inherits");
            continue;
        }

        class_info->AddBaseClass(base_info);

        auto inheritedProperty = elem_base.child("property");
        while (inheritedProperty)
        {
            auto lookup_name = rmap_PropNames.find(inheritedProperty.attribute("name").as_view());
            if (lookup_name == rmap_PropNames.end())
            {
                MSG_ERROR(std::format("Unrecognized inherited property name -- {}",
                                      inheritedProperty.attribute("name").as_view()));
                inheritedProperty = inheritedProperty.next_sibling("property");
                continue;
            }
            class_info->SetOverRideDefValue(lookup_name->second,
                                            inheritedProperty.text().as_view());
            inheritedProperty = inheritedProperty.next_sibling("property");
        }

        inheritedProperty = elem_base.child("hide");
        while (inheritedProperty)
        {
            auto lookup_name = rmap_PropNames.find(inheritedProperty.attribute("name").as_view());
            if (lookup_name == rmap_PropNames.end())
            {
                MSG_ERROR(std::format("Unrecognized inherited property name -- {}",
                                      inheritedProperty.attribute("name").as_view()));
                inheritedProperty = inheritedProperty.next_sibling("hide");
                continue;
            }
            class_info->HideProperty(lookup_name->second);
            inheritedProperty = inheritedProperty.next_sibling("hide");
        }

        elem_base = elem_base.next_sibling("inherits");
    }
}

// The xml_data parameter is the char* pointer to the XML data. It will be empty when
// processing an interface document.
void NodeCreator::ParseGeneratorFile(const char* xml_data)
{
    pugi::xml_document doc;
    pugi::xml_node root;
    bool is_interface = (xml_data == nullptr || !*xml_data);

    if (!xml_data || !*xml_data)
    {
        root = m_pdoc_interface->child("GeneratorDefinitions");
    }
    else
    {
        auto result = doc.load_string(xml_data);
        if (!result)
        {
            FAIL_MSG("XML file is corrupted!");
            throw std::runtime_error("Internal XML file is corrupted.");
        }
        root = doc.child("GeneratorDefinitions");
    }

    if (!root)
    {
        FAIL_MSG("GeneratorDefinitions not found in XML file.");
        throw std::runtime_error("Internal XML file is corrupted.");
    }

    auto generator = root.child("gen");
    while (generator)
    {
        ParseGenerator(generator, is_interface);
        generator = generator.next_sibling("gen");
    }

    // Interface processing doesn't have xml_data
    if (xml_data && *xml_data)
    {
        auto elem_obj = root.child("gen");
        while (elem_obj)
        {
            ProcessGeneratorInheritance(elem_obj);
            elem_obj = elem_obj.next_sibling("gen");
        }
    }
}

void NodeCreator::AddPropertyOptions(pugi::xml_node& elem_prop, PropDeclaration* prop_info)
{
    auto& opts = prop_info->getOptions();
    auto elem_opt = elem_prop.child("option");
    while (elem_opt)
    {
        auto& opt = opts.emplace_back();
        opt.name = elem_opt.attribute("name").as_view();
        opt.help = elem_opt.attribute("help").as_view();

        elem_opt = elem_opt.next_sibling("option");
    }
}

void NodeCreator::AddVarNameRelatedProperties(NodeDeclaration* node_declaration,
                                              NodeCategory& category)
{
    // var_comment property
    category.addProperty(prop_var_comment);
    auto* prop_info = new PropDeclaration(
        prop_var_comment, type_string_edit_single, PropDeclaration::DefaultValue(tt_empty_cstr),
        PropDeclaration::HelpText(
            "Comment to add to the variable name in the generated header file "
            "if the class access is set to protected or public"));
    node_declaration->GetPropInfoMap()[std::string(map_PropNames.at(prop_var_comment))] = prop_info;

    // class_access property
    category.addProperty(prop_class_access);
    std::string access("protected:");

    // Most widgets will default to protected: as their class access. Those in the
    // set_no_class_access array should have "none" as the default class access.
    if (set_no_class_access.contains(node_declaration->get_GenName()))
    {
        access = "none";
    }

    prop_info = new PropDeclaration(
        prop_class_access, type_option, PropDeclaration::DefaultValue(access),
        PropDeclaration::HelpText(
            "Determines the type of access your inherited class has to this item."));
    node_declaration->GetPropInfoMap()[std::string(map_PropNames.at(prop_class_access))] =
        prop_info;

    auto& opts = prop_info->getOptions();

    if (!node_declaration->is_Gen(gen_wxTimer))
    {
        opts.emplace_back();
        opts[opts.size() - 1].name = "none";
        opts[opts.size() - 1].help = "The item can only be accessed within the class.";
    }

    opts.emplace_back();
    opts[opts.size() - 1].name = "protected:";
    opts[opts.size() - 1].help =
        "In C++, only derived classes can access this item.\nIn wxPython, item will have a "
        "self. prefix.\nIn wxPerl, item will have a $self-> prefix.";

    opts.emplace_back();
    opts[opts.size() - 1].name = "public:";
    opts[opts.size() - 1].help =
        "In C++, item is added as a public: class member.\nIn Python, item will have a "
        "self. prefix.\nIn wxPerl, item will have a $self-> prefix.";
}

/* static */ void NodeCreator::ParseSingleProperty(pugi::xml_node& elem_prop,
                                                   NodeDeclaration* node_declaration,
                                                   NodeCategory& category)
{
    auto name = elem_prop.attribute("name").as_str();
    if (name.starts_with("prop_"))
    {
        name.erase(0, sizeof("prop_") - 1);
    }

    auto lookup_name = rmap_PropNames.find(name);
    if (lookup_name == rmap_PropNames.end())
    {
        MSG_ERROR(std::format("Unrecognized property name -- {}", name));
        return;
    }
    GenEnum::PropName prop_name = lookup_name->second;

    category.addProperty(prop_name);

    auto description = elem_prop.attribute("help").as_view();

    auto prop_type = elem_prop.attribute("type").as_view();
    if (prop_type.starts_with("type_"))
    {
        prop_type.remove_prefix(sizeof("type_") - 1);
    }

    GenEnum::PropType property_type { type_unknown };

    if (auto result = umap_PropTypes.find(prop_type); result != umap_PropTypes.end())
    {
        property_type = result->second;
    }

    if (property_type == type_unknown)
    {
        MSG_ERROR(std::format("Unrecognized property type -- {}", prop_type));
        return;
    }

    wxString def_value;
    if (auto lastChild = elem_prop.last_child(); lastChild && !lastChild.text().empty())
    {
        def_value = lastChild.text().get();
        if (def_value.find('\n') != wxString::npos)
        {
            def_value.Trim(false);
            def_value.Trim();
        }
    }

    auto* prop_info = new PropDeclaration(prop_name, property_type,
                                          PropDeclaration::DefaultValue(def_value.ToStdString()),
                                          PropDeclaration::HelpText(description));
    node_declaration->GetPropInfoMap()[name] = prop_info;

    if (elem_prop.attribute("hide").as_bool())
    {
        node_declaration->HideProperty(prop_name);
    }

    if (property_type == type_bitlist || property_type == type_option ||
        property_type == type_editoption)
    {
        AddPropertyOptions(elem_prop, prop_info);
    }

    // Any time there is a var_name property, it needs to be followed by a var_comment and
    // class_access property. Rather than add this to all the XML generator specifications, we
    // simply insert it here if it doesn't exist.
    if (tt::is_sameas(name, map_PropNames.at(prop_var_name)) &&
        !node_declaration->is_Gen(gen_data_string) && !node_declaration->is_Gen(gen_data_xml))
    {
        AddVarNameRelatedProperties(node_declaration, category);
    }
}

void NodeCreator::ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* node_declaration,
                                  NodeCategory& category)
{
    auto elem_category = elem_obj.child("category");
    while (elem_category)
    {
        auto name = elem_category.attribute("name").as_view();
        auto& new_cat = category.addCategory(name);

        if (auto base_name = elem_category.attribute("base_name").value(); base_name.size())
        {
            if (auto node = m_interfaces.find(base_name); node != m_interfaces.end())
            {
                ParseProperties(node->second, node_declaration, new_cat);
            }
        }
        else
        {
            ParseProperties(elem_category, node_declaration, new_cat);
        }

        elem_category = elem_category.next_sibling("category");
    }

    auto elem_prop = elem_obj.child("property");
    while (elem_prop)
    {
        ParseSingleProperty(elem_prop, node_declaration, category);
        elem_prop = elem_prop.next_sibling("property");
    }
}

void NodeDeclaration::ParseEvents(pugi::xml_node& elem_obj, NodeCategory& category)
{
    auto elem_category = elem_obj.child("category");
    while (elem_category)
    {
        // Only create the category if there is at least one event.
        if (elem_category.child("event"))
        {
            auto name = elem_category.attribute("name").as_view();
            auto& new_cat = category.addCategory(name);

            ParseEvents(elem_category, new_cat);
        }
        elem_category = elem_category.next_sibling("category");
    }

    auto nodeEvent = elem_obj.child("event");
    while (nodeEvent)
    {
        auto evt_name = nodeEvent.attribute("name").as_str();
        category.addEvent(evt_name);

        auto evt_class = nodeEvent.attribute("class").as_view("wxEvent");
        auto description = nodeEvent.attribute("help").as_view();

        m_events[evt_name] = new NodeEventInfo(evt_name, evt_class, description);

        nodeEvent = nodeEvent.next_sibling("event");
    }
}
