/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRichTextCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/richtext/richtextctrl.h>  // A rich edit control

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_rich_text.h"

wxObject* RichTextCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRichTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                     DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size),
                                     GetStyleInt(node) | wxRE_MULTILINE);

    if (node->HasValue(prop_hint))
        widget->SetHint(node->prop_as_wxString(prop_hint));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> RichTextCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->prop_as_string(prop_value).size())
        code << GenerateQuotedString(node->prop_as_string(prop_value));
    else
        code << "wxEmptyString";

    if (node->prop_as_string(prop_window_name).empty())
        GeneratePosSizeFlags(node, code);
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, but we have to supply something before the window name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> RichTextCtrlGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue(prop_hint))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHint(" << GenerateQuotedString(node->prop_as_string(prop_hint)) << ");";
    }

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (code.size())
        return code;
    else

        return {};
}

std::optional<ttlib::cstr> RichTextCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int RichTextCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRichTextCtrl");

    ADD_ITEM_PROP(prop_value, "value")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void RichTextCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxRichTextCtrlXmlHandler -- you must explicitly add this handler");
}

bool RichTextCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/richtext/richtextctrl.h>", set_src, set_hdr);
    return true;
}
