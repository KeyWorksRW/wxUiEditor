//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAnimationCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>          // wxAnimation and wxAnimationCtrl
#include <wx/generic/animate.h>  // wxGenericAnimationCtrl

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties

#include "gen_animation.h"

wxObject* AnimationGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto get_animation = [](Node* node)
    {
        if (auto prop = node->getPropPtr(prop_animation); prop)
            return prop->as_animation();
        else
            return wxAnimation();
    };

    auto animation = get_animation(node);

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
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().CheckLineLength();
    if (code.hasValue(prop_animation))
    {
        tt_view_vector parts(code.node()->as_string(prop_animation), ';');
        if (code.is_cpp())
        {
            tt_string name(parts[IndexImage].filename());
            name.remove_extension();
            name.LeftTrim();
            if (parts[IndexType].starts_with("Embed"))
            {
                auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                if (embed)
                {
                    name = "wxue_img::" + embed->imgs[0].array_name;
                }
            }

            code << "wxueAnimation(" << name << ", sizeof(" << name << "))";
        }
        else if (code.is_python())
        {
            tt_string name(parts[IndexImage]);
            name.make_absolute();
            if (!name.file_exists())
            {
                name = Project.ArtDirectory();
                name.append_filename(parts[IndexImage]);
                name.make_absolute();
            }
            auto form_path = MakePythonPath(code.node());
            name.make_relative(form_path);
            name.backslashestoforward();

            code.Str("wx.adv.Animation(").QuotedString(name) += ")";
        }
        else if (code.is_ruby())
        {
            bool found_embedded = false;
            if (parts.size() > IndexImage)
            {
                if (const EmbeddedImage* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]); embed)
                {
                    code.Str("get_animation(").Str("$").Str(embed->imgs[0].array_name) += ")";
                    found_embedded = true;
                }
            }
            if (!found_embedded)
            {
                code.Str("Wx::Animation.new");
            }
        }
        else
        {
            if (code.is_ruby())
                code.Str("Wx::Animation.new");
            else
                code.Add("wxNullAnimation");
        }
    }
    code.PosSizeFlags(false);
    if (code.hasValue(prop_inactive_bitmap))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetInactiveBitmap(");
        if (code.is_cpp())
        {
            tt_string bundle_code;
            GenerateBundleCode(code.node()->as_string(prop_inactive_bitmap), bundle_code);
            code.CheckLineLength(bundle_code.size());
            code += bundle_code;
        }
        else if (code.is_python())
        {
            bool is_list_created = PythonBitmapList(code, prop_inactive_bitmap);
            if (is_list_created)
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
            else
            {
                code.Bundle(prop_inactive_bitmap);
            }
        }
        else if (code.is_ruby())
        {
            code.Bundle(prop_inactive_bitmap);
        }
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
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    if (!node->as_bool(prop_use_generic))
        GenXrcObjectAttributes(node, item, "wxAnimationCtrl");
    else
        GenXrcObjectAttributes(node, item, "wxGenericAnimationCtrl");
    GenXrcStylePosSize(node, item);

    if (node->hasValue(prop_animation))
    {
        tt_string_vector parts(node->as_string(prop_animation), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        item.append_child("animation").text().set(parts[IndexImage]);
    }
    if (node->hasValue(prop_inactive_bitmap))
    {
        tt_string_vector parts(node->as_string(prop_inactive_bitmap), ';', tt::TRIM::both);
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
    if (node->hasValue(prop_inactive_bitmap))
    {
        handlers.emplace("wxBitmapXmlHandler");
    }
}

bool AnimationGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                     int /* language */)
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
