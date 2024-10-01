//////////////////////////////////////////////////////////////////////////
// Purpose:   wxListbook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/listbook.h>  // wxChoicebook: wxChoice and wxNotebook combination

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "pugixml.hpp"       // xml read/write/create/process
#include "utils.h"           // Utility functions that work with properties

#include "gen_listbook.h"

wxObject* ListbookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    // Note the currently, wxListbook does not have a "style" property since the only thing that can be set is the
    // label (tab) position
    auto widget = new wxListbook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                 DlgSize(node, prop_size), GetStyleInt(node));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_LISTBOOK_PAGE_CHANGED, &ListbookGenerator::OnPageChanged, this);

    return widget;
}

void ListbookGenerator::OnPageChanged(wxListbookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxListbook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        getMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool ListbookGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(code::allow_scaling, false, "wxBK_DEFAULT");
    BookCtorAddImagelist(code);

    return true;
}

bool ListbookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                    GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/listbook.h>", set_src, set_hdr);
    if (node->hasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_listbk.cpp
// ../../../wxWidgets/src/xrc/xh_listbk.cpp

int ListbookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxListbook");

    tt_string styles(node->as_string(prop_style));
    if (node->as_string(prop_tab_position) != "wxBK_DEFAULT")
    {
        if (styles.size())
            styles << '|';
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

void ListbookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxListbookXmlHandler");
}
