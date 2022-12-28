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

std::optional<ttlib::sview> RichTextCtrlGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.GetParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_value);
    code.PosSizeFlags(true);

    return code.m_code;
}

std::optional<ttlib::sview> RichTextCtrlGenerator::CommonSettings(Code& code)
{
    if (code.HasValue(prop_hint))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetFocus(").EndFunction();
    }

    return code.m_code;
}

int RichTextCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRichTextCtrl");

    ADD_ITEM_PROP(prop_value, "value")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
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
