/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPropertyGrid generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with general properties
#include "utils_prop_grid.h"  // PropertyGrid utilities

#include "gen_prop_grid.h"

wxObject* PropertyGridGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPropertyGrid(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                     DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->hasValue(prop_extra_style))
    {
        widget->SetExtraStyle(node->as_int(prop_extra_style));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void PropertyGridGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node, bool /* is_preview */)
{
    AfterCreationAddItems(wxStaticCast(wxobject, wxPropertyGrid), node);
}

bool PropertyGridGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(false, "wxPG_DEFAULT_STYLE");

    if (code.hasValue(prop_extra_style))
        code.Eol().NodeName().Function("SetExtraStyle(").Add(prop_extra_style).EndFunction();

    return true;
}

bool PropertyGridGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                        int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/propgrid/propgrid.h>", set_src, set_hdr);

    if (CheckAdvancePropertyInclude(node))
    {
        InsertGeneratorInclude(node, "#include <wx/propgrid/advprops.h>", set_src, set_hdr);
    }

    return true;
}

bool PropertyGridGenerator::GetRubyImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("require 'wx/pg'");
    return true;
}
