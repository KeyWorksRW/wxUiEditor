//////////////////////////////////////////////////////////////////////////
// Purpose:   wxSpinButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/spinbutt.h>  // wxSpinButtonBase class

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_spin_btn.h"

using namespace code;

wxObject* SpinButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinButton(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetRange(node->as_int(prop_min), node->as_int(prop_max));
    widget->SetValue(node->as_int(prop_initial));

    if (node->as_int(prop_inc) > 1)
        widget->SetIncrement(node->as_int(prop_inc));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SpinButtonGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(false, "wxSP_VERTICAL");

    // If the last parameter is wxID_ANY, then remove it. This is the default value, so it's
    // not needed.
    code.Replace(", wxID_ANY)", ")");

    return true;
}

bool SpinButtonGenerator::SettingsCode(Code& code)
{
    code.NodeName().Function("SetRange(").as_string(prop_min).Comma().as_string(prop_max).EndFunction();

    if (code.IsTrue(prop_initial))
    {
        code.Eol().NodeName().Function("SetValue(").as_string(prop_initial).EndFunction();
    }

    if (code.node()->as_int(prop_inc) > 1)
    {
        code.Eol().NodeName().Function("SetIncrement(").as_string(prop_inc).EndFunction();
    }

    return true;
}

bool SpinButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/spinbutt.h>", set_src, set_hdr);
    return true;
}

int SpinButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSpinButton");

    ADD_ITEM_PROP(prop_min, "min")
    ADD_ITEM_PROP(prop_max, "max")
    ADD_ITEM_PROP(prop_initial, "value")

    if (node->as_int(prop_inc) > 1)
        ADD_ITEM_PROP(prop_inc, "inc")

    if (node->HasValue(prop_style))
    {
        GenXrcStylePosSize(node, item);
    }
    else
    {
        // XRC is going to force the wxSP_ARROW_KEYS if we don't pass something. Since a spin control
        // can only be horizontal, we simply pass that flag.
        GenXrcPreStylePosSize(node, item, "wxSP_VERTICAL");
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void SpinButtonGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSpinButtonXmlHandler");
}
