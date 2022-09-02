/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextSizerWrapper generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/textwrapper.h>  // declaration of wxTextWrapper class

#include <wx/sizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_text_sizer.h"

wxObject* TextSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxTextSizerWrapper wrapper(wxStaticCast(parent, wxWindow));
    return wrapper.CreateSizer(node->prop_as_wxString(prop_text), node->prop_as_int(prop_wrap));
}

std::optional<ttlib::cstr> TextSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";

    code << node->get_node_name();
    auto parent = node->GetParent();
    while (parent->IsSizer())
        parent = parent->GetParent();

    if (parent->isGen(gen_wxDialog))
    {
        code << " = CreateTextSizer(";
    }
    else
    {
        code << " = wxTextSizerWrapper(" << parent->get_node_name() << ").CreateSizer(";
    }

    code << GenerateQuotedString(node->prop_as_string(prop_text)) << ", " << node->prop_as_string(prop_width) << ");";

    return code;
}

bool TextSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    auto parent = node->GetParent();
    while (parent->IsSizer())
        parent = parent->GetParent();

    if (parent->isGen(gen_wxDialog))
        InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/textwrapper.h>", set_src, set_hdr);

    return true;
}
