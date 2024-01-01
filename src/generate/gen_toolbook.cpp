//////////////////////////////////////////////////////////////////////////
// Purpose:   wxToolbook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/toolbook.h>  // wxToolbook: wxToolBar and wxNotebook combination

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "pugixml.hpp"       // xml read/write/create/process
#include "ui_images.h"       // Generated images header
#include "utils.h"           // Utility functions that work with properties

#include "gen_toolbook.h"

wxObject* ToolbookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxToolbook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    wxBookCtrlBase::Images bundle_list;
    for (size_t idx_child = 0; idx_child < node->getChildCount(); ++idx_child)
    {
        if (node->getChild(idx_child)->hasValue(prop_bitmap))
        {
            auto bundle = node->getChild(idx_child)->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                bundle = wxue_img::bundle_unknown_svg(24, 24);
            }

            bundle_list.push_back(bundle);
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
        getMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool ToolbookGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(false);

    BookCtorAddImagelist(code);

    return true;
}

bool ToolbookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                    int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/toolbook.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_toolbk.cpp
// ../../../wxWidgets/src/xrc/xh_toolbk.cpp

int ToolbookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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
