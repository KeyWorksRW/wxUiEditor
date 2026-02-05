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
    auto* parent = node->get_Parent();
    while (parent->is_Sizer())
    {
        parent = parent->get_Parent();
    }

    if (parent->is_Gen(gen_wxDialog))
    {
        code << " = CreateTextSizer(";
    }
    else
    {
        code << " = wxTextSizerWrapper(" << parent->get_NodeName() << ").CreateSizer(";
    }

    code.QuotedString(prop_text).Comma().as_string(prop_width).EndFunction();

    return true;
}

bool TextSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    auto* parent = node->get_Parent();
    while (parent->is_Sizer())
    {
        parent = parent->get_Parent();
    }

    if (parent->is_Gen(gen_wxDialog))
    {
        InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    }
    else
    {
        InsertGeneratorInclude(node, "#include <wx/textwrapper.h>", set_src, set_hdr);
    }

    return true;
}
