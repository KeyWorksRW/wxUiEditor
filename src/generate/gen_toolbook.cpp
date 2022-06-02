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

std::optional<ttlib::cstr> ToolbookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxToolbook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    BookCtorAddImagelist(code, node);

    return code;
}

std::optional<ttlib::cstr> ToolbookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ToolbookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/toolbook.h>", set_src, set_hdr);

    return true;
}
