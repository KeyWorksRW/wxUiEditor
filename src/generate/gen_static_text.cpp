/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticText generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stattext.h>  // wxStaticText base header

// Blank line added because wx/stattext.h must be included first
#include <wx/generic/stattextg.h>  // wxGenericStaticText header

#include "gen_common.h"       // Common component functions
#include "gen_lang_common.h"  // Common mulit-language functions
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties

#include "gen_static_text.h"

wxObject* StaticTextGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxStaticTextBase* widget;
    if (node->prop_as_bool(prop_markup) && node->prop_as_int(prop_wrap) <= 0)
    {
        widget =
            new wxGenericStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString,
                                    DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
    }
    else
    {
        widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, DlgPoint(parent, node, prop_pos),
                                  DlgSize(parent, node, prop_size), GetStyleInt(node));
    }

    if (node->prop_as_bool(prop_markup) && node->prop_as_int(prop_wrap) <= 0)
        widget->SetLabelMarkup(node->prop_as_wxString(prop_label));
    else
        widget->SetLabel(node->prop_as_wxString(prop_label));

    if (node->prop_as_int(prop_wrap) > 0)
        widget->Wrap(node->prop_as_int(prop_wrap));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool StaticTextGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    ASSERT(widget)
    // Getting a nullptr for widget should never happen, but sometimes does, and things blow up if we try to use it
    if (widget && (prop->isProp(prop_wrap) || prop->isProp(prop_label) || prop->isProp(prop_markup)))
    {
        // If the text was wrapped previously, then it already has \n characters inserted in it, so we need to restore
        // it to it's original state before wrapping again.

        auto ctrl = wxStaticCast(widget, wxStaticText);
        if (node->prop_as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->prop_as_wxString(prop_label));
        else
            ctrl->SetLabel(node->prop_as_wxString(prop_label));

        if (node->prop_as_int(prop_wrap) > 0)
            ctrl->Wrap(node->prop_as_int(prop_wrap));

        return true;
    }

    return false;
}

// C++ version
std::optional<ttlib::cstr> StaticTextGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name()
         << GenerateNewAssignment(node, (node->prop_as_bool(prop_markup) && node->prop_as_int(prop_wrap) <= 0));

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    // If the label is going to be set via SetLabelMarkup(), then there is no reason to initialize it here and then
    // replace it on the next line of code (which will be the call to SetLabelMarkUp())
    if (node->prop_as_bool(prop_markup))
    {
        code << "wxEmptyString";
    }
    else
    {
        auto& label = node->prop_as_string(prop_label);
        if (label.size())
        {
            code << GenerateQuotedString(label);
        }
        else
        {
            code << "wxEmptyString";
        }
    }

    GeneratePosSizeFlags(node, code);

    return code;
}

// Multi-language version
std::optional<ttlib::cstr> StaticTextGenerator::GenConstruction(Node* node, int language)
{
    if (language == GEN_LANG_CPLUSPLUS)
        return GenConstruction(node);

    ttlib::cstr code;
    code << node->get_node_name()
         << GenerateNewAssignment(language, node, (node->prop_as_bool(prop_markup) && node->prop_as_int(prop_wrap) <= 0));

    code << GetParentName(language, node) << ", " << GetWidgetName(language, node->prop_as_string(prop_id)) << ", ";

    // If the label is going to be set via SetLabelMarkup(), then there is no reason to initialize it here and then
    // replace it on the next line of code (which will be the call to SetLabelMarkUp())
    if (node->prop_as_bool(prop_markup))
    {
        code << GetWidgetName(language, "wxEmptyString");
    }
    else
    {
        auto& label = node->prop_as_string(prop_label);
        if (label.size())
        {
            code << GenerateQuotedString(language, label);
        }
        else
        {
            code << GetWidgetName(language, "wxEmptyString");
        }
    }

    GeneratePosSizeFlags(node, code);
    if (code.back() == ';')
        code.pop_back();

    return code;
}

std::optional<ttlib::cstr> StaticTextGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

// C++ version
std::optional<ttlib::cstr> StaticTextGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_markup) && node->prop_as_int(prop_wrap) <= 0)
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetLabelMarkup(" << GenerateQuotedString(node->prop_as_string(prop_label))
             << ");";
    }

    // Note that wrap MUST be called after the text is set, otherwise it will be ignored.
    if (node->prop_as_int(prop_wrap) > 0)
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->Wrap(" << node->prop_as_string(prop_wrap) << ");";
    }
    return code;
}

// Multi-language version
std::optional<ttlib::cstr> StaticTextGenerator::GenSettings(Node* node, size_t& auto_indent, int language)
{
    if (language == GEN_LANG_CPLUSPLUS)
        return GenSettings(node, auto_indent);

    ttlib::cstr code;

    if (node->prop_as_bool(prop_markup) && node->prop_as_int(prop_wrap) <= 0)
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << LangPtr(language) << "SetLabelMarkup("
             << GenerateQuotedString(node->prop_as_string(prop_label)) << ")";
    }

    // Note that wrap MUST be called after the text is set, otherwise it will be ignored.
    if (node->prop_as_int(prop_wrap) > 0)
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << LangPtr(language) << "Wrap(" << node->prop_as_string(prop_wrap) << ")";
    }

    return code;
}

int StaticTextGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStaticText");

#if 0
    // REVIEW: [KeyWorks - 05-28-2022] Once markup and generic version is supported in XRC, this can be enabled
    // with a version check.

    if (node->prop_as_bool(prop_markup) && node->prop_as_int(prop_wrap) <= 0)
    {
        item.append_child("use_generic platform=\"msw\"").text().set("1");
    }
    ADD_ITEM_BOOL(prop_markup, "markup")
#endif

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_PROP(prop_wrap, "wrap")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->prop_as_bool(prop_markup))
        {
            item.append_child(pugi::node_comment).set_value(" markup cannot be be set in the XRC file. ");
        }

        GenXrcComments(node, item);
    }

    return result;
}

void StaticTextGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStaticTextXmlHandler");
}

bool StaticTextGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/stattext.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);

    return true;
}
