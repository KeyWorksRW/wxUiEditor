//////////////////////////////////////////////////////////////////////////
// Purpose:   wxToolbook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/toolbook.h>  // wxToolbook: wxToolBar and wxNotebook combination

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "pugixml.hpp"       // xml read/write/create/process
#include "utils.h"           // Utility functions that work with properties

#include "gen_toolbook.h"

wxObject* ToolbookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxToolbook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    wxBookCtrlBase::Images bundle_list;
    for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
    {
        if (node->GetChild(idx_child)->HasValue(prop_bitmap))
        {
            bundle_list.push_back(node->GetChild(idx_child)->prop_as_wxBitmapBundle(prop_bitmap));
        }
    }
    auto book = wxStaticCast(widget, wxBookCtrlBase);
    book->SetImages(bundle_list);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_TOOLBOOK_PAGE_CHANGED, &ToolbookGenerator::OnPageChanged, this);

    return widget;
}

void ToolbookGenerator::OnPageChanged(wxBookCtrlEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxToolbook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::sview> ToolbookGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(false);

    // TODO: [Randalphwa - 12-21-2022] Add Python support
    if (code.is_cpp())
    {
        BookCtorAddImagelist(code.m_code, code.m_node);
    }

    return code.m_code;
}

bool ToolbookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/toolbook.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_toolbk.cpp
// ../../../wxWidgets/src/xrc/xh_toolbk.cpp

int ToolbookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxToolbook");
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ToolbookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxToolbookXmlHandler");
}
