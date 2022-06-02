/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxColourPickerCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/clrpicker.h>  // wxColourPickerCtrl base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_clr_picker.h"

wxObject* ColourPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxColourPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxColour(prop_colour),
                               DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ColourPickerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->prop_as_string(prop_colour).size())
        code << GenerateColourCode(node, prop_colour);
    else
        code << "*wxBLACK";
    GeneratePosSizeFlags(node, code, true, "wxCLRP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> ColourPickerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ColourPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/clrpicker.h>", set_src, set_hdr);
    return true;
}
