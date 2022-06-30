/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCalendarCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/calctrl.h>  // date-picker control

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_calendar_ctrl.h"

//////////////////////////////////////////  CalendarCtrlGenerator  //////////////////////////////////////////

wxObject* CalendarCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxCalendarCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime,
                                     DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CalendarCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxDefaultDateTime";
    GeneratePosSizeFlags(node, code, false, "wxCAL_SHOW_HOLIDAYS");

    code.Replace(", wxDefaultDateTime);", ");");

    return code;
}

std::optional<ttlib::cstr> CalendarCtrlGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (code.size())
        return code;
    else

        return {};
}
std::optional<ttlib::cstr> CalendarCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool CalendarCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/calctrl.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_cald.cpp
// ../../../wxWidgets/src/xrc/xh_cald.cpp

int CalendarCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxCalendarCtrl");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void CalendarCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxCalendarCtrlXmlHandler");
}
