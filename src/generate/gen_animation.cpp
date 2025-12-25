//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAnimationCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>          // wxAnimation and wxAnimationCtrl
#include <wx/generic/animate.h>  // wxGenericAnimationCtrl
#include <wx/scopedptr.h>        // wxScopedPtr: scoped smart pointer class

#include "code.h"                               // Code -- Helper class for generating code
#include "gen_common.h"                         // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"                      // Common XRC generating functions
#include "image_gen.h"                          // Functions for generating embedded images
#include "image_handler.h"                      // ImageHandler class
#include "node.h"                               // Node class
#include "project_handler.h"                    // ProjectHandler class
#include "pugixml.hpp"                          // xml read/write/create/process
#include "utils.h"                              // Utility functions that work with properties
#include "wxue_namespace/wxue_string.h"         // wxue::string, wxue::string_view
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector
#include "wxue_namespace/wxue_view_vector.h"    // wxue::ViewVector

#include "gen_animation.h"

wxObject* AnimationGenerator::CreateMockup(Node* node, wxObject* parent)
{
    if (tt::contains(node->as_string(prop_animation), ".ani", tt::CASE::either))
    {
        auto* widget = new wxGenericAnimationCtrl(wxStaticCast(parent, wxWindow), wxID_ANY,
                                                  wxNullAnimation, DlgPoint(node, prop_pos),
                                                  DlgSize(node, prop_size), GetStyleInt(node));
        auto animation = widget->CreateAnimation();
        if (auto prop = node->get_PropPtr(prop_animation); prop)
            prop->as_animation(&animation);

        widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
        if (animation.IsOk())
        {
            widget->SetAnimation(animation);
            widget->Play();
        }
        return widget;
    }
    else
    {
        auto* widget = new wxAnimationCtrl(wxStaticCast(parent, wxWindow), wxID_ANY,
                                           wxNullAnimation, DlgPoint(node, prop_pos),
                                           DlgSize(node, prop_size), GetStyleInt(node));
        auto animation = widget->CreateAnimation();
        if (auto prop = node->get_PropPtr(prop_animation); prop)
            prop->as_animation(&animation);

        widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
        if (animation.IsOk())
        {
            widget->SetAnimation(animation);
            widget->Play();
        }
        return widget;
    }
}

bool AnimationGenerator::ConstructionCode(Code& code)
{
    if (code.get_language() == GEN_LANG_RUBY)
    {
        // wxRuby3 1.0.0 doesn't support the generic version of wxAnimationCtrl
        code.AddAuto().NodeName().CreateClass();
        code.ValidParentName().Comma().as_string(prop_id).Comma().CheckLineLength();
        if (code.HasValue(prop_animation))
        {
            bool found_embedded = false;
            wxue::ViewVector parts(code.node()->as_string(prop_animation), ';');
            if (parts.size() > IndexImage)
            {
                if (const EmbeddedImage* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                    embed)
                {
                    code.Str("get_animation(").Str("$").Str(embed->base_image().array_name) += ")";
                    found_embedded = true;
                }
            }
            if (!found_embedded)
            {
                code.Str("Wx::Animation.new");
            }
            code.PosSizeFlags();
        }
    }
    else
    {
        // The generic version is required to display .ANI files on wxGTK.
        bool use_generic_version =
            (code.node()->as_string(prop_animation).contains(".ani", tt::CASE::either) ||
             code.node()->as_string(prop_subclass).starts_with("wxGeneric"));
        code.AddAuto().NodeName().CreateClass(use_generic_version);
        code.ValidParentName()
            .Comma()
            .as_string(prop_id)
            .Comma()
            .Add("wxNullAnimation")
            .CheckLineLength();
        code.PosSizeFlags();
    }

    if (code.HasValue(prop_inactive_bitmap))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetInactiveBitmap(");
        if (code.is_cpp())
        {
            wxue::string bundle_code;
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

    if (code.HasValue(prop_animation))
    {
        code.Eol().OpenBrace();
        if (code.is_cpp())
        {
            code += "auto ";
        }

        wxue::ViewVector parts(code.node()->as_string(prop_animation), ';');
        if (code.is_cpp())
        {
            code.Str("animate = ").NodeName().Function("CreateAnimation(").EndFunction();
            wxue::string name(parts[IndexImage].filename());
            name.remove_extension();
            name.LeftTrim();
            if (parts[IndexType].starts_with("Embed"))
            {
                auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                if (embed)
                {
                    name = "wxue_img::" + embed->base_image().array_name;
                }
            }

            code.Eol() << "wxueAnimation(" << name << ", sizeof(" << name << ")";
            code.Comma().Str("animate").EndFunction();
            code.Eol().NodeName().Function("SetAnimation(animate").EndFunction().CloseBrace();
        }
        else if (code.is_python())
        {
            code.Str("animate = ").NodeName().Function("CreateAnimation(").EndFunction();
            bool found_embedded = false;
            if (parts.size() > IndexImage)
            {
                if (const EmbeddedImage* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                    embed)
                {
                    code.Eol()
                        .Str("stream = io.BytesIO(")
                        .Str(embed->base_image().array_name)
                        .Str(".GetData())");
                    code.Eol().Str("animate.Load(stream)");
                    found_embedded = true;
                }
            }
            if (!found_embedded)
            {
                wxue::string name(parts[IndexImage]);
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

                code.Eol().Str("animate.LoadFile(").QuotedString(name) += ")";
            }
            code.Eol().NodeName().Function("SetAnimation(animate").EndFunction().CloseBrace();
        }
        // wxRuby3 code is handled at the top of this function
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
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    // wxGenericAnimationCtrl is required to display .ANI files on wxGTK. Since the other platforms
    // effecitvely use wxGenericAnimationCtrl any way (since there are no native implementations of
    // wxAnimationCtrl) this shouldn't make any difference for them.
    if (node->HasValue(prop_animation) &&
        node->as_string(prop_animation).contains(".gif", tt::CASE::either))
        GenXrcObjectAttributes(node, item, "wxAnimationCtrl");
    else
        GenXrcObjectAttributes(node, item, "wxGenericAnimationCtrl");

    GenXrcStylePosSize(node, item);

    if (node->HasValue(prop_animation))
    {
        wxue::StringVector parts(node->as_string(prop_animation), ';', wxue::TRIM::both);
        ASSERT(parts.size() > 1)
        item.append_child("animation").text().set(parts[IndexImage]);
    }
    if (node->HasValue(prop_inactive_bitmap))
    {
        wxue::StringVector parts(node->as_string(prop_inactive_bitmap), ';', wxue::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            wxue::StringVector art_parts(parts[IndexArtID], '|');
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

bool AnimationGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/animate.h>", set_src, set_hdr);
    if ((node->HasValue(prop_animation) &&
         !node->as_string(prop_animation).contains(".gif", tt::CASE::either)) ||
        node->as_string(prop_subclass).starts_with("wxGeneric"))
    {
        InsertGeneratorInclude(node, "#include <wx/generic/animate.h>", set_src, set_hdr);
    }

    return true;
}

bool AnimationGenerator::GetPythonImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("import wx.adv");
    set_imports.insert("import io");
    return true;
}
