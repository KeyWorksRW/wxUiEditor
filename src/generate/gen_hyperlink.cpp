//////////////////////////////////////////////////////////////////////////
// Purpose:   wxHyperlinkCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/hyperlink.h>  // Hyperlink control

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_hyperlink.h"

wxObject* HyperlinkGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxHyperlinkCtrlBase* widget;
    if (node->prop_as_bool(prop_underlined))
    {
        widget = new wxHyperlinkCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                     node->prop_as_wxString(prop_url), DlgPoint(parent, node, prop_pos),
                                     DlgSize(parent, node, prop_size), GetStyleInt(node));
    }
    else
    {
        widget = new wxGenericHyperlinkCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                            node->prop_as_wxString(prop_url), DlgPoint(parent, node, prop_pos),
                                            DlgSize(parent, node, prop_size), GetStyleInt(node));

        if (!node->HasValue(prop_font))
        {
            widget->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        }
    }

    if (node->HasValue(prop_hover_color))
    {
        widget->SetHoverColour(node->prop_as_wxColour(prop_hover_color));
    }
    else if (node->prop_as_bool(prop_sync_hover_colour))
    {
        widget->SetHoverColour(widget->GetNormalColour());
    }

    if (node->HasValue(prop_normal_color))
    {
        widget->SetNormalColour(node->prop_as_wxColour(prop_normal_color));
    }
    if (node->HasValue(prop_visited_color))
    {
        widget->SetVisitedColour(node->prop_as_wxColour(prop_visited_color));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool HyperlinkGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    if (code.is_cpp() && !code.IsTrue(prop_underlined))
        code.Replace("wxHyperlinkCtrl", "wxGenericHyperlinkCtrl");

    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
    code.Comma().QuotedString(prop_url);
    code.PosSizeFlags(false, "wxHL_DEFAULT_STYLE");

    return true;
}

bool HyperlinkGenerator::SettingsCode(Code& code)
{
    if (!code.IsTrue(prop_underlined) && !code.HasValue(prop_font))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetFont(").Add("wxSystemSettings").ClassMethod("GetFont(");
        code.Add("wxSYS_DEFAULT_GUI_FONT)").EndFunction();
    }

    if (code.HasValue(prop_hover_color))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetHoverColour(");
        ColourCode(code, prop_hover_color);
        code.EndFunction();
    }

    if (code.HasValue(prop_normal_color))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetNormalColour(");
        ColourCode(code, prop_normal_color);
        code.EndFunction();
    }

    if (code.HasValue(prop_visited_color))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetVisitedColour(");
        ColourCode(code, prop_visited_color);
        code.EndFunction();
    }

    return true;
}

int HyperlinkGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxHyperlinkCtrl");

#if defined(WIDGETS_FORK)
    if (!node->prop_as_bool(prop_underlined))
    {
        item.append_child("use_generic").text().set(1);
        if (!node->HasValue(prop_font))
        {
            auto font_object = item.append_child("font");
            font_object.append_child("sysfont").text().set("wxSYS_DEFAULT_GUI_FONT");
            FontProperty font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
            font_object.append_child("underlined").text().set("0");
        }
    }
    if (node->HasValue(prop_normal_color))
    {
        item.append_child("normal").text().set(
            node->prop_as_wxColour(prop_normal_color).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }
    if (node->HasValue(prop_hover_color))
    {
        item.append_child("hover").text().set(
            node->prop_as_wxColour(prop_hover_color).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }
    if (node->HasValue(prop_visited_color))
    {
        item.append_child("visited").text().set(
            node->prop_as_wxColour(prop_visited_color).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }
#endif

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_PROP(prop_url, "url")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
#if !defined(WIDGETS_FORK)
        if (node->HasValue(prop_hover_color))
        {
            item.append_child(pugi::node_comment).set_value(" hover color cannot be be set in the XRC file. ");
        }
        if (node->HasValue(prop_normal_color))
        {
            item.append_child(pugi::node_comment).set_value(" normal color cannot be be set in the XRC file. ");
        }
        if (node->HasValue(prop_visited_color))
        {
            item.append_child(pugi::node_comment).set_value(" visited color cannot be be set in the XRC file. ");
        }
#endif
        if (!node->prop_as_bool(prop_underlined))
        {
            item.append_child(pugi::node_comment).set_value(" removing underline cannot be be set in the XRC file. ");
        }
        if (node->prop_as_bool(prop_sync_hover_colour))
        {
            item.append_child(pugi::node_comment).set_value(" sync hover color cannot be be set in the XRC file. ");
        }
        GenXrcComments(node, item);
    }

    return result;
}

void HyperlinkGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxHyperlinkCtrlXmlHandler");
}

bool HyperlinkGenerator::IsGeneric(Node* node)
{
    return (!node->prop_as_bool(prop_underlined));
}

bool HyperlinkGenerator::GetIncludes(Node* node, std::set<std::string>& /* set_src */, std::set<std::string>& set_hdr)
{
    // Unfortunately wx/generic/hyperlink.h doesn't include the required wx/hyperlink.h file.
    // That means the order of inclusion is critical, hence the hack below to change the
    // alphabetical order of the two headers. See https://github.com/wxWidgets/wxWidgets/issues/23060

    if (!node->as_bool(prop_underlined))
        set_hdr.insert("#include <wx/hyperlink.h>\n#include <wx/generic/hyperlink.h>");
    else
        set_hdr.insert("#include <wx/hyperlink.h>");

    return true;
}
