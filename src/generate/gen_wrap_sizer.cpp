/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWrapSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>
#include <wx/wrapsizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_wrap_sizer.h"

wxObject* WrapSizerGenerator::CreateMockup(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxWrapSizer(node->prop_as_int(prop_orientation), node->prop_as_int(prop_flags));
    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    return sizer;
}

std::optional<ttlib::cstr> WrapSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxWrapSizer(" << node->prop_as_string(prop_orientation);
    auto wrap_flags = node->prop_as_string(prop_wrap_flags);
    if (wrap_flags.empty())
        wrap_flags = "0";
    code << ", " << wrap_flags << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool WrapSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/wrapsizer.h>", set_src, set_hdr);
    return true;
}
