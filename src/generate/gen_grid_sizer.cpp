/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGridSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_grid_sizer.h"

wxObject* GridSizerGenerator::CreateMockup(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxGridSizer(node->prop_as_int(prop_rows), node->prop_as_int(prop_cols), node->prop_as_int(prop_vgap),
                                 node->prop_as_int(prop_hgap));

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));

    return sizer;
}

std::optional<ttlib::cstr> GridSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxGridSizer(";
    auto rows = node->prop_as_int(prop_rows);
    auto cols = node->prop_as_int(prop_cols);
    auto vgap = node->prop_as_int(prop_vgap);
    auto hgap = node->prop_as_int(prop_hgap);

    if (rows != 0)
    {
        code << rows << ", ";
    }
    code << cols;

    if (vgap != 0 || hgap != 0)
    {
        code << ", " << vgap << ", " << hgap;
    }
    code << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool GridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}
