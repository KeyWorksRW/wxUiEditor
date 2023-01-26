//////////////////////////////////////////////////////////////////////////
// Purpose:   wxNotebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bookctrl.h>  // wxBookCtrlBase: common base class for wxList/Tree/Notebook

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_notebook.h"

wxObject* NotebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &NotebookGenerator::OnPageChanged, this);

    return widget;
}

void NotebookGenerator::OnPageChanged(wxNotebookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxNotebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool NotebookGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(false);

    // TODO: [Randalphwa - 12-21-2022] Add Python support
    if (code.is_cpp())
    {
        BookCtorAddImagelist(code.GetCode(), code.m_node);
    }

    return true;
}

bool NotebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/notebook.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_notbk.cpp
// ../../../wxWidgets/src/xrc/xh_notbk.cpp

int NotebookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxNotebook");

    tt_string styles(node->prop_as_string(prop_style));
    if (node->prop_as_string(prop_tab_position) != "wxBK_DEFAULT")
    {
        if (styles.size())
            styles << '|';
        styles << node->prop_as_string(prop_tab_position);
    }

    GenXrcPreStylePosSize(node, item, styles);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void NotebookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxNotebookXmlHandler");
}
