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
        new wxColourPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxColour(prop_colour),
                               DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ColourPickerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();
    if (code.node()->as_string(prop_colour).size())
        ColourCode(code, prop_colour);
    else
    {
        if (code.is_cpp())
            code.Str("*wxBLACK");
        else
            code.Add("wxBLACK");
    }
    code.PosSizeFlags(true, "wxCLRP_DEFAULT_STYLE");

    return true;
}

bool ColourPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                        int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/clrpicker.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_clrpicker.cpp
// ../../../wxWidgets/src/xrc/xh_clrpicker.cpp

int ColourPickerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxColourPickerCtrl");

    item.append_child("value").text().set(node->as_wxColour(prop_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ColourPickerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxColourPickerCtrlXmlHandler");
}
