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
    return wrapper.CreateSizer(node->as_wxString(prop_text), node->as_int(prop_wrap));
}

bool TextSizerGenerator::ConstructionCode(Code& code)
{
    auto* node = code.node();
    code.AddAuto().NodeName();
    auto parent = node->getParent();
    while (parent->isSizer())
        parent = parent->getParent();

    if (parent->isGen(gen_wxDialog))
    {
        code << " = CreateTextSizer(";
    }
    else
    {
        code << " = wxTextSizerWrapper(" << parent->getNodeName() << ").CreateSizer(";
    }

    code.QuotedString(prop_text).Comma().Str(prop_width).EndFunction();

    return true;
}

bool TextSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    auto parent = node->getParent();
    while (parent->isSizer())
        parent = parent->getParent();

    if (parent->isGen(gen_wxDialog))
        InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/textwrapper.h>", set_src, set_hdr);

    return true;
}
