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

std::optional<ttlib::cstr> HyperlinkGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node, !node->prop_as_bool(prop_underlined));

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }
    code << ", ";
    auto& url = node->prop_as_string(prop_url);
    if (url.size())
    {
        code << GenerateQuotedString(url);
    }
    else
    {
        code << "wxEmptyString";
    }

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> HyperlinkGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;
    if (!node->prop_as_bool(prop_underlined) && !node->HasValue(prop_font))
    {
        code << node->get_node_name() << "->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));";
    }

    if (node->HasValue(prop_hover_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(" << GenerateColourCode(node, prop_hover_color) << ");";
    }
    else if (node->prop_as_bool(prop_sync_hover_colour))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(" << node->get_node_name() << "->GetNormalColour());";
    }

    if (node->HasValue(prop_normal_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetNormalColour(" << GenerateColourCode(node, prop_normal_color) << ");";
    }

    if (node->HasValue(prop_visited_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetVisitedColour(" << GenerateColourCode(node, prop_visited_color) << ");";
    }
    return code;
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
    handlers.emplace("wxStaticLineXmlHandler");
}

bool HyperlinkGenerator::IsGeneric(Node* node)
{
    return (!node->prop_as_bool(prop_underlined));
}

std::optional<ttlib::cstr> HyperlinkGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool HyperlinkGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */, std::set<std::string>& set_hdr)
{
    // If there's an event, then this has to be in the header file.
    set_hdr.insert("#include <wx/hyperlink.h>");
    return true;
}
