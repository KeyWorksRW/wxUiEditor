/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTimePickerCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/timectrl.h>  // Declaration of wxTimePickerCtrl class.

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_time_picker.h"

wxObject* TimePickerCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxTimePickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::sview> TimePickerCtrlGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().Add("wxDefaultDateTime");
    code.PosSizeFlags(true, "wxTP_DEFAULT");

    return code.m_code;
}

bool TimePickerCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/timectrl.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/dateevt.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_timectrl.cpp
// ../../../wxWidgets/src/xrc/xh_timectrl.cpp

int TimePickerCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxTimePickerCtrl");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void TimePickerCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxTimeCtrlXmlHandler");
}
