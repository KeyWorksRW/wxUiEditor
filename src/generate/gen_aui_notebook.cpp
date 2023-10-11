//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiNotebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>     // wxBookCtrlBase: common base class for wxList/Tree/Notebook

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_aui_notebook.h"

wxObject* AuiNotebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxAuiNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                    DlgSize(parent, node, prop_size), GetStyleInt(node));
    if (node->as_string(prop_art_provider).is_sameas("wxAuiGenericTabArt"))
        widget->SetArtProvider(new wxAuiGenericTabArt());
    else if (node->as_string(prop_art_provider).is_sameas("wxAuiSimpleTabArt"))
        widget->SetArtProvider(new wxAuiSimpleTabArt());

    if (node->as_int(prop_tab_height) > 0)
        widget->SetTabCtrlHeight(node->as_int(prop_tab_height));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &AuiNotebookGenerator::OnPageChanged, this);

    return widget;
}

void AuiNotebookGenerator::OnPageChanged(wxNotebookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxNotebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        getMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool AuiNotebookGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(false);
    BookCtorAddImagelist(code);

    // REVIEW: [Randalphwa - 12-21-2022] We use this ourselves in base_panel.cpp and it is in wx/aui/tabart.h
    // without any specific comments, however it is not documented. Should we support it? Currently there
    // is no setting for it in aui_xml.xml.

    if (code.IsEqualTo(prop_art_provider, "wxAuiGenericTabArt"))
    {
        if (code.is_cpp())
        {
            code.Eol().NodeName().Function("SetArtProvider(");
            code.Str(code.is_cpp() ? "new wxAuiGenericTabArt()" : "wx.aui.wxAuiGenericTabArt()").EndFunction();
        }
        else
        {
            code.Eol() += "# wxPython does not support wxAuiGenericTabArt";
        }
    }
    else if (code.IsEqualTo(prop_art_provider, "wxAuiSimpleTabArt"))
    {
        code.Eol().NodeName().Function("SetArtProvider(");
        code.Str(code.is_cpp() ? "new wxAuiSimpleTabArt()" : "wx.aui.AuiSimpleTabArt()").EndFunction();
    }

    return true;
}

bool AuiNotebookGenerator::SettingsCode(Code& code)
{
    if (code.IntValue(prop_tab_height) > 0)
    {
        code.Eol().NodeName().Function("SetTabCtrlHeight(").as_string(prop_tab_height).EndFunction();
    }

    return true;
}

bool AuiNotebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/aui/auibook.h>", set_src, set_hdr);
    if (node->hasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_aui.cpp
// ../../../wxWidgets/src/xrc/xh_aui.cpp
// wxAuiNotebook handler is near the end of this file.

int AuiNotebookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxAuiNotebook");

    if (node->as_string(prop_art_provider) == "wxAuiSimpleTabArt")
        item.append_child("art-provider").text().set("simple");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void AuiNotebookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxAuiXmlHandler");
}

bool AuiNotebookGenerator::GetRubyImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("require 'wx/aui'");
    return true;
}

void AuiNotebookGenerator::AddPropsAndEvents(NodeDeclaration* declaration)
{
    DeclAddVarNameProps(declaration, "m_notebook");
    DeclAddProp(declaration, prop_display_images, type_bool,
                "If true, an image will be displayed on the tab in addition to any text. The image to use for each tab is "
                "specifed in the individual pages.",
                "0");
    DeclAddProp(declaration, prop_persist, type_string,
                "If a name is specified, wxPersistenceManager will be used to save/restore the currently selected page.");

    auto prop_info = DeclAddProp(declaration, prop_art_provider, type_option, "Determines how the tabs are displayed.",
                                 "wxAuiDefaultTabArt");
    {
        DeclAddOption(prop_info, "wxAuiDefaultTabArt",
                      "Use bitmap art and a colour scheme that is adapted to the major platforms' look.");
        DeclAddOption(prop_info, "wxAuiSimpleTabArt",
                      "Use a simple art and colour scheme with a slanted left side for the tabs.");
    }
    prop_info =
        DeclAddProp(declaration, prop_style, type_option, {},
                    /* the default values are equivalent to wxAUI_NB_DEFAULT_STYLE */
                    "wxAUI_NB_TOP|wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_CLOSE_ON_ACTIVE_TAB"
                    "ACTIVE_TAB|wxAUI_NB_MIDDLE_CLICK_CLOSE");
    {
        DeclAddOption(prop_info, "wxAUI_NB_CLOSE_BUTTON", "Adds a close button on the tab bar.");
        DeclAddOption(prop_info, "wxAUI_NB_CLOSE_ON_ACTIVE_TAB", "Adds a close button on the active tab.");
        DeclAddOption(prop_info, "wxAUI_NB_CLOSE_ON_ALL_TABS", "Adds a close button on all tabs.");
        DeclAddOption(prop_info, "wxAUI_NB_MIDDLE_CLICK_CLOSE", "With this style, middle click on a tab closes the tab.");
        DeclAddOption(prop_info, "wxAUI_NB_SCROLL_BUTTONS", "Display left and right scroll buttons.");
        DeclAddOption(prop_info, "wxAUI_NB_TAB_EXTERNAL_MOVE", "Allows a tab to be moved to another tab control.");
        DeclAddOption(prop_info, "wxAUI_NB_TAB_FIXED_WIDTH", "Display all tabs with the same width.");
        DeclAddOption(prop_info, "wxAUI_NB_TAB_MOVE", "Allows a tab to be moved horizontally by dragging.");
        DeclAddOption(prop_info, "wxAUI_NB_TAB_SPLIT", "Allows the tab control to be split by dragging a tab.");
        DeclAddOption(prop_info, "wxAUI_NB_BOTTOM",
                      "Place tabs under instead of above the book pages. Cannot be combined with wxAUI_NB_TOP.");
        DeclAddOption(prop_info, "wxAUI_NB_TOP", "Place tabs on the top side. Cannot be combined with wxAUI_NB_BOTTOM.");
        DeclAddOption(prop_info, "wxAUI_NB_WINDOWLIST_BUTTON",
                      "With this style, a drop-down list of tabs is available. This menu can be displayed by calling "
                      "ShowWindowMenu().");
    }

    // Add events
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_PAGE_CLOSE", "wxAuiNotebookEvent", "A page is about to be closed.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_PAGE_CLOSED", "wxAuiNotebookEvent", "A page has been closed.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_PAGE_CHANGED", "wxAuiNotebookEvent", "The page selection was changed.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_PAGE_CHANGING", "wxAuiNotebookEvent",
                 "The page selection is about to be changed. This event can be vetoed.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_BUTTON", "wxAuiNotebookEvent", "The window list button has been pressed.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_BEGIN_DRAG", "wxAuiNotebookEvent", "Dragging is about to begin.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_END_DRAG", "wxAuiNotebookEvent", "Dragging has ended.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_DRAG_MOTION", "wxAuiNotebookEvent",
                 "Emitted during a drag and drop operation.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_ALLOW_DND", "wxAuiNotebookEvent",
                 "Whether to allow a tab to be dropped. This event must be specially allowed.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_DRAG_DONE", "wxAuiNotebookEvent", "The tab has been dragged.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN", "wxAuiNotebookEvent",
                 "The middle mouse button is pressed on a tab.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP", "wxAuiNotebookEvent",
                 "The middle mouse button is released on a tab.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN", "wxAuiNotebookEvent",
                 "The right mouse button is pressed on a tab.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_TAB_RIGHT_UP", "wxAuiNotebookEvent",
                 "The right mouse button is released on a tab.");
    DeclAddEvent(declaration, "wxEVT_AUINOTEBOOK_BG_DCLICK", "wxAuiNotebookEvent",
                 "Double clicked on the tabs background area.");
}
