/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSearchCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/srchctrl.h>  // wxSearchCtrlBase class

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_search_ctrl.h"

wxObject* SearchCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSearchCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                   DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_hint))
        widget->SetHint(node->prop_as_wxString(prop_hint));

    if (node->HasValue(prop_search_button))
    {
        widget->ShowSearchButton(node->prop_as_bool(prop_search_button));
    }

    if (node->HasValue(prop_cancel_button))
    {
        widget->ShowCancelButton(node->prop_as_bool(prop_cancel_button));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::sview> SearchCtrlGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.GetParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_value);
    code.PosSizeFlags(true);

    return code.m_code;
}

std::optional<ttlib::sview> SearchCtrlGenerator::CommonSettings(Code& code)
{
    if (code.HasValue(prop_hint))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
    }

    if (code.IsTrue(prop_search_button))
    {
        code.Eol(eol_if_empty).NodeName().Function("ShowSearchButton(").AddTrue().EndFunction();
    }

    if (code.IsTrue(prop_cancel_button))
    {
        code.Eol(eol_if_empty).NodeName().Function("ShowCancelButton(").AddTrue().EndFunction();
    }

    return code.m_code;
}

bool SearchCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/srchctrl.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_srchctrl.cpp
// ../../../wxWidgets/src/xrc/xh_srchctrl.cpp

int SearchCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSearchCtrl");

    ADD_ITEM_PROP(prop_value, "value")

    // Note that XRC calls SetDescriptiveText() instead of SetHint() which has a different apperance (SetDescription text is
    // not greyed out).
    ADD_ITEM_PROP(prop_hint, "hint")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->prop_as_bool(prop_search_button))
            ADD_ITEM_COMMENT("XRC does not support ShowSearchButton()")
        if (node->prop_as_bool(prop_cancel_button))
            ADD_ITEM_COMMENT("XRC does not support ShowCancelButton()")
        GenXrcComments(node, item);
    }

    return result;
}

void SearchCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSearchCtrlXmlHandler");
}
