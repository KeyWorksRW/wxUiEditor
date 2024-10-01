/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFontPickerCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fontpicker.h>

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_font_picker.h"

wxObject* FontPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxFontPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxFont(prop_initial_font),
                                       DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));

    if (node->hasValue(prop_max_point_size))
    {
        widget->SetMaxPointSize(node->as_int(prop_max_point_size));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool FontPickerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();
    if (code.hasValue(prop_initial_font))
    {
        auto fontprop = code.node()->as_font_prop(prop_initial_font);
        wxFont font = fontprop.GetFont();

        code.Object("wxFont");

        if (fontprop.GetPointSize() <= 0)
        {
            code.Add("wxNORMAL_FONT").Function("GetPointSize()");
        }
        else
            code.itoa(fontprop.GetPointSize());

        code.Comma().Add(ConvertFontFamilyToString(fontprop.GetFamily())).Comma().Add(font.GetStyleString().utf8_string());
        code.Comma().Add(font.GetWeightString().utf8_string()).Comma();

        if (fontprop.IsUnderlined())
            code.True();
        else
            code.False();
        code.Comma();
        if (fontprop.GetFaceName().size())
            code.QuotedString(fontprop.GetFaceName().utf8_string());
        else
            code.Add("wxEmptyString");

        code += ")";
    }
    else
    {
        if (code.is_ruby())
            code.Str("Wx::NULL_FONT");
        else
            code.Add("wxNullFont");
    }

    code.PosSizeFlags(code::allow_scaling, true);

    return true;
}

bool FontPickerGenerator::SettingsCode(Code& code)
{
    Node* node = code.node();
    if (node->as_string(prop_min_point_size) != "0")
    {
        code.NodeName().Function("SetMinPointSize(").as_string(prop_min_point_size).EndFunction();
    }

    if (node->as_string(prop_max_point_size) != "100")
    {
        code.Eol(eol_if_empty).NodeName().Function("SetMaxPointSize(").as_string(prop_max_point_size).EndFunction();
    }

    return true;
}

bool FontPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                      GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/fontpicker.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/font.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_fontpicker.cpp
// ../../../wxWidgets/src/xrc/xh_fontpicker.cpp

int FontPickerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxFontPickerCtrl");

    GenXrcFont(item, "value", node, prop_initial_font);

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_int(prop_min_point_size) != 0)
            ADD_ITEM_COMMENT("XRC does not support calling SetMinPointSize().")
        if (node->as_int(prop_max_point_size) != 100)
            ADD_ITEM_COMMENT("XRC does not support calling SetMaxPointSize().")

        GenXrcComments(node, item);
    }

    return result;
}

void FontPickerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxFontPickerCtrlXmlHandler");
}
