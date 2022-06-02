//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAnimationCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mainapp.h"        // App -- Main application class
#include "node.h"           // Node class
#include "pjtsettings.h"    // ProjectSettings -- Hold data for currently loaded project
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_animation.h"

wxObject* AnimationGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto animation = node->prop_as_wxAnimation(prop_animation);
    auto widget = new wxAnimationCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, animation, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    if (animation.IsOk())
        widget->Play();

    return widget;
}

std::optional<ttlib::cstr> AnimationGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->HasValue(prop_animation))
    {
        ttlib::multiview parts(node->prop_as_string(prop_animation), ';');
        ttlib::cstr name(parts[IndexImage].filename());
        name.remove_extension();
        name.LeftTrim();
        if (parts[IndexType].starts_with("Embed"))
        {
            auto embed = wxGetApp().GetProjectSettings()->GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                name = "wxue_img::" + embed->array_name;
            }
        }

        code << "wxueAnimation(" << name << ", sizeof(" << name << "))";
    }
    else
    {
        code << "wxNullAnimation";
    }

    GeneratePosSizeFlags(node, code, false, "wxAC_DEFAULT_STYLE");
    if (node->HasValue(prop_inactive_bitmap))
    {
        code << "\n\t" << node->get_node_name() << "->SetInactiveBitmap(";
        code << GenerateBitmapCode(node->prop_as_string(prop_inactive_bitmap)) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> AnimationGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    if (node->prop_as_bool(prop_play))
    {
        ttlib::cstr code;
        code << node->get_node_name() << "->Play();";
        return code;
    }
    else
    {
        return {};
    }
}

int AnimationGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxAnimationCtrl");
    GenXrcStylePosSize(node, item);

    if (node->HasValue(prop_animation))
    {
        ttlib::multistr parts(node->prop_as_string(prop_animation), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        item.append_child("animation").text().set(parts[IndexImage]);
    }
    if (node->HasValue(prop_inactive_bitmap))
    {
        ttlib::multistr parts(node->prop_as_string(prop_inactive_bitmap), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            ttlib::multistr art_parts(parts[IndexArtID], '|');
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

    if (add_comments)
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
    return true;
}
