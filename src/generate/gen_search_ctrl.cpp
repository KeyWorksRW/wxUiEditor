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
    auto widget = new wxSearchCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_value),
                                   DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->hasValue(prop_hint))
        widget->SetHint(node->as_wxString(prop_hint));

    if (node->hasValue(prop_search_button))
    {
        widget->ShowSearchButton(node->as_bool(prop_search_button));
    }

    if (node->hasValue(prop_cancel_button))
    {
        widget->ShowCancelButton(node->as_bool(prop_cancel_button));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SearchCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_value);
    code.PosSizeFlags(true);

    return true;
}

bool SearchCtrlGenerator::SettingsCode(Code& code)
{
    if (code.hasValue(prop_hint))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
        {
            code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
        }
    }

    if (code.IsTrue(prop_search_button))
    {
        code.Eol(eol_if_empty).NodeName().Function("ShowSearchButton(").True().EndFunction();
    }

    if (code.IsTrue(prop_cancel_button))
    {
        code.Eol(eol_if_empty).NodeName().Function("ShowCancelButton(").True().EndFunction();
    }

    return true;
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
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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
        if (node->as_bool(prop_search_button))
            ADD_ITEM_COMMENT("XRC does not support ShowSearchButton()")
        if (node->as_bool(prop_cancel_button))
            ADD_ITEM_COMMENT("XRC does not support ShowCancelButton()")
        GenXrcComments(node, item);
    }

    return result;
}

void SearchCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSearchCtrlXmlHandler");
}
