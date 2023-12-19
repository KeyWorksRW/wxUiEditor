//////////////////////////////////////////////////////////////////////////
// Purpose:   wxContextHelpButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/cshelp.h>

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_ctx_help_btn.h"

wxObject* CtxHelpButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxContextHelpButton(wxStaticCast(parent, wxWindow), wxID_CONTEXT_HELP, DlgPoint(parent, node, prop_pos),
                                DlgSize(parent, node, prop_size), GetStyleInt(node));

    return widget;
}

bool CtxHelpButtonGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();

    // TODO: [Randalphwa - 12-18-2023] Usually you only need the parent.
    code.ValidParentName().Comma().as_string(prop_id);

    code.PosSizeFlags(true);

    return true;
}

bool CtxHelpButtonGenerator::SettingsCode(Code& code)
{
    if (code.hasValue(prop_bitmap))
    {
        if (code.hasValue(prop_position))
        {
            code.Eol(eol_if_needed).NodeName().Function("SetBitmapPosition(").as_string(prop_position).EndFunction();
        }

        if (code.hasValue(prop_margins))
        {
            auto size = code.node()->as_wxSize(prop_margins);
            code.Eol(eol_if_needed).NodeName().Function("SetBitmapMargins(");
            code.itoa(size.GetWidth()).Comma().itoa(size.GetHeight()).EndFunction();
        }

        if (code.is_cpp())
            GenBtnBimapCode(code.node(), code.GetCode());
        else
            PythonBtnBimapCode(code);
    }

    return true;
}

bool CtxHelpButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                         int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/cshelp.h>", set_src, set_hdr);
    return true;
}

int CtxHelpButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    ADD_ITEM_COMMENT(" XRC does not support wxContextHelpButton ")
    return BaseGenerator::xrc_not_supported;
}
