/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticText generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stattext.h>  // wxStaticText base header

// Blank line added because wx/stattext.h must be included first
#include <wx/generic/stattextg.h>  // wxGenericStaticText header

#include "code.h"             // Code -- Helper class for generating code
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

        auto ctrl = wxStaticCast(widget, wxStaticTextBase);
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

std::optional<ttlib::sview> StaticTextGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass((code.m_node->prop_as_bool(prop_markup) && code.m_node->prop_as_int(prop_wrap) <= 0));
    code.GetParentName().Comma().as_string(prop_id).Comma();
    if (code.m_node->prop_as_bool(prop_markup))
    {
        code.EmptyString();
    }

    else
    {
        auto& label = code.m_node->prop_as_string(prop_label);
        if (label.size())
        {
            code.QuotedString(prop_label);
        }
        else
        {
            code.EmptyString();
        }
    }

    code.PosSizeFlags();

    return code.m_code;
}

std::optional<ttlib::sview> StaticTextGenerator::CommonSettings(Code& code)
{
    if (code.m_node->prop_as_bool(prop_markup) && code.m_node->prop_as_int(prop_wrap) <= 0)
    {
        code.NodeName().Function("SetLabelMarkup(");
        code.QuotedString(prop_label);
        code.EndFunction();
    }

    // Note that wrap MUST be called after the text is set, otherwise it will be ignored.
    if (code.node()->prop_as_int(prop_wrap) > 0)
    {
        code.Eol(true).NodeName().Function("Wrap(").as_string(prop_wrap).EndFunction();
    }

    return code.m_code;
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
