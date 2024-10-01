//////////////////////////////////////////////////////////////////////////
// Purpose:   wxTreebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/treebook.h>  // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "pugixml.hpp"       // xml read/write/create/process
#include "utils.h"           // Utility functions that work with properties

#include "gen_treebook.h"

wxObject* TreebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTreebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                 DlgSize(node, prop_size), GetStyleInt(node));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_TREEBOOK_PAGE_CHANGED, &TreebookGenerator::OnPageChanged, this);

    return widget;
}

void TreebookGenerator::OnPageChanged(wxBookCtrlEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxTreebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        getMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool TreebookGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags();
    BookCtorAddImagelist(code);

    return true;
}

bool TreebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                    GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/treebook.h>", set_src, set_hdr);
    if (node->hasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/treebook.h>");
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_treebk.cpp
// ../../../wxWidgets/src/xrc/xh_treebk.cpp

int TreebookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxTreebook");

    tt_string styles;  // Ignore wxNB_NOPAGETHEM which is not supported by XRC

    if (node->as_string(prop_tab_position) != "wxBK_DEFAULT")
    {
        styles << node->as_string(prop_tab_position);
    }

    GenXrcPreStylePosSize(node, item, styles);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_bool(prop_persist))
            item.append_child(pugi::node_comment).set_value(" persist is not supported in XRC. ");

        GenXrcComments(node, item);
    }

    return result;
}

void TreebookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxTreebookXmlHandler");
}
