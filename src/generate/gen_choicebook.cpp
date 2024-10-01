//////////////////////////////////////////////////////////////////////////
// Purpose:   wxChoicebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/choicebk.h>  // wxChoicebook: wxChoice and wxNotebook combination

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_choicebook.h"

wxObject* ChoicebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxChoicebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                   DlgSize(node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_CHOICEBOOK_PAGE_CHANGED, &ChoicebookGenerator::OnPageChanged, this);

    return widget;
}
void ChoicebookGenerator::OnPageChanged(wxBookCtrlEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxChoicebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        getMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool ChoicebookGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(code::allow_scaling, false, "wxCHB_DEFAULT");

    return true;
}

bool ChoicebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                      GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/choicebk.h>", set_src, set_hdr);
    if (node->hasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }
    return true;
}

// ../../wxSnapShot/src/xrc/xh_choicbk.cpp
// ../../../wxWidgets/src/xrc/xh_choicbk.cpp

int ChoicebookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxChoicebook");

    tt_string styles(node->as_string(prop_style));
    if (node->as_string(prop_tab_position) != "wxCHB_DEFAULT")
    {
        if (styles.size())
            styles << '|';
        styles << node->as_string(prop_tab_position);
    }

    GenXrcPreStylePosSize(node, item, styles);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ChoicebookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxChoicebookXmlHandler");
}
