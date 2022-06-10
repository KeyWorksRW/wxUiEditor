//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiNotebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
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
    if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiGenericTabArt"))
        widget->SetArtProvider(new wxAuiGenericTabArt());
    else if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiSimpleTabArt"))
        widget->SetArtProvider(new wxAuiSimpleTabArt());

    if (node->prop_as_int(prop_tab_height) > 0)
        widget->SetTabCtrlHeight(node->prop_as_int(prop_tab_height));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &AuiNotebookGenerator::OnPageChanged, this);

    return widget;
}

void AuiNotebookGenerator::OnPageChanged(wxNotebookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxNotebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> AuiNotebookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxAuiNotebook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    BookCtorAddImagelist(code, node);

    if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiGenericTabArt"))
    {
        code << "\n\t" << node->get_node_name() << "->SetArtProvider(new wxAuiGenericTabArt());";
    }
    else if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiSimpleTabArt"))
    {
        code << "\n\t" << node->get_node_name() << "->SetArtProvider(new wxAuiSimpleTabArt());";
    }

    return code;
}

std::optional<ttlib::cstr> AuiNotebookGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    if (node->prop_as_int(prop_tab_height) > 0)
    {
        ttlib::cstr code;
        code << node->get_node_name() << "->SetTabCtrlHeight(" << node->prop_as_string(prop_tab_height) << ");";
        return code;
    }
    return {};
}

std::optional<ttlib::cstr> AuiNotebookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool AuiNotebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/aui/auibook.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_aui.cpp
// ../../../wxWidgets/src/xrc/xh_aui.cpp
// wxAuiNotebook handler is near the end of this file.

int AuiNotebookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxAuiNotebook");

    if (node->value(prop_art_provider) == "wxAuiSimpleTabArt")
        item.append_child("art-provider").text().set("simple");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void AuiNotebookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxAuiXmlHandler");
}
