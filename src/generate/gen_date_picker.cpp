/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDatePickerCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/datectrl.h>  // implements wxDatePickerCtrl

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_date_picker.h"

wxObject* DatePickerCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxDatePickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->as_string(prop_style).contains("wxDP_ALLOWNONE"))
        widget->SetNullText(node->as_wxString(prop_null_text));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool DatePickerCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().Add("wxDefaultDateTime");
    code.PosSizeFlags(true, "wxDP_DEFAULT|wxDP_SHOWCENTURY");

    return true;
}

bool DatePickerCtrlGenerator::SettingsCode(Code& code)
{
    if (code.PropContains(prop_style, "wxDP_ALLOWNONE"))
    {
        code.NodeName().Function("SetNullText").QuotedString(prop_null_text).EndFunction();
    }

    return true;
}

bool DatePickerCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/datectrl.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/dateevt.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_datectrl.cpp
// ../../../wxWidgets/src/xrc/xh_datectrl.cpp

int DatePickerCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxDatePickerCtrl");

    if (node->as_string(prop_style).contains("wxDP_ALLOWNONE"))
        item.append_child("null-text").text().set(node->as_string(prop_null_text));

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void DatePickerCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxDateCtrlXmlHandler");
}
