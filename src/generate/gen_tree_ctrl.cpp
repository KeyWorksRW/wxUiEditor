/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTreeCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// clang-format off
#include <wx/treectrl.h>          // wxTreeCtrl base header
#include <wx/generic/treectlg.h>  // wxGenericTreeCtrl
// clang-format on

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_tree_ctrl.h"

wxObject* TreeCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxTreeCtrlBase* widget;
    if (node->as_string(prop_subclass).starts_with("wxGeneric"))
    {
        widget = new wxGenericTreeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY,
                                       DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                                       GetStyleInt(node));
    }
    else
    {
        widget = new wxTreeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                DlgSize(node, prop_size), GetStyleInt(node));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool TreeCtrlGenerator::ConstructionCode(Code& code)
{
    bool use_generic_version =
        code.is_cpp() && code.node()->as_string(prop_subclass).starts_with("wxGeneric");
    code.AddAuto()
        .NodeName()
        .CreateClass(use_generic_version)
        .ValidParentName()
        .Comma()
        .as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, true, "wxTR_DEFAULT_STYLE");

    return true;
}

bool TreeCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/treectrl.h>", set_src, set_hdr);
    if (node->as_string(prop_subclass).starts_with("wxGeneric"))
    {
        InsertGeneratorInclude(node, "#include <wx/generic/treectlg.h>", set_src, set_hdr);
    }
    return true;
}

// ../../wxSnapShot/src/xrc/xh_tree.cpp
// ../../../wxWidgets/src/xrc/xh_tree.cpp

int TreeCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created :
                                                 BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxTreeCtrl");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void TreeCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxTreeCtrlXmlHandler");
}
