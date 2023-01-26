//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAnimationCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>          // wxAnimation and wxAnimationCtrl
#include <wx/generic/animate.h>  // wxGenericAnimationCtrl

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "image_handler.h"  // ImageHandler class
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_animation.h"

wxObject* AnimationGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto animation = node->prop_as_wxAnimation(prop_animation);
    if (!node->as_bool(prop_use_generic))
    {
        auto widget =
            new wxAnimationCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, animation, DlgPoint(parent, node, prop_pos),
                                DlgSize(parent, node, prop_size), GetStyleInt(node));

        widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
        if (animation.IsOk())
            widget->Play();

        return widget;
    }
    else
    {
        auto widget =
            new wxGenericAnimationCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, animation, DlgPoint(parent, node, prop_pos),
                                       DlgSize(parent, node, prop_size), GetStyleInt(node));

        widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
        if (animation.IsOk())
            widget->Play();

        return widget;
    }
}

bool AnimationGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().CheckLineLength();
    // TODO: [Randalphwa - 12-08-2022] Enable Python support once image handling is worked out
    if (code.HasValue(prop_animation) && code.is_cpp())
    {
        tt_view_vector parts(code.node()->as_string(prop_animation), ';');
        tt_string name(parts[IndexImage].filename());
        name.remove_extension();
        name.LeftTrim();
        if (parts[IndexType].starts_with("Embed"))
        {
            auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                name = "wxue_img::" + embed->array_name;
            }
        }

        code << "wxueAnimation(" << name << ", sizeof(" << name << "))";
    }
    else
    {
        code.Add("wxNullAnimation");
    }
    code.PosSizeFlags(false);
    if (code.HasValue(prop_inactive_bitmap) && code.is_cpp())
    {
        code.Eol().NodeName().Function("SetInactiveBitmap(");
        code.m_code << GenerateBitmapCode(code.node()->as_string(prop_inactive_bitmap));
        code.EndFunction();
    }

    return true;
}

bool AnimationGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_play))
        code.NodeName().Function("Play(").EndFunction();
    return true;
}

int AnimationGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    if (!node->as_bool(prop_use_generic))
        GenXrcObjectAttributes(node, item, "wxAnimationCtrl");
    else
        GenXrcObjectAttributes(node, item, "wxGenericAnimationCtrl");
    GenXrcStylePosSize(node, item);

    if (node->HasValue(prop_animation))
    {
        tt_string_vector parts(node->prop_as_string(prop_animation), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        item.append_child("animation").text().set(parts[IndexImage]);
    }
    if (node->HasValue(prop_inactive_bitmap))
    {
        tt_string_vector parts(node->prop_as_string(prop_inactive_bitmap), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            tt_string_vector art_parts(parts[IndexArtID], '|');
            auto bmp = item.append_child("inactive-bitmap");
            bmp.append_attribute("stock_id").set_value(art_parts[0]);
            bmp.append_attribute("stock_client").set_value(art_parts[1]);
        }
        else
        {
            item.append_child("inactive-bitmap").text().set(parts[IndexImage]);
        }
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void AnimationGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxAnimationCtrlXmlHandler");
    if (node->HasValue(prop_inactive_bitmap))
    {
        handlers.emplace("wxBitmapXmlHandler");
    }
}

bool AnimationGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/animate.h>", set_src, set_hdr);
    if (node->as_bool(prop_use_generic))
        InsertGeneratorInclude(node, "#include <wx/generic/animate.h>", set_src, set_hdr);
    return true;
}

bool AnimationGenerator::GetPythonImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("import wx.adv");
    return true;
}
