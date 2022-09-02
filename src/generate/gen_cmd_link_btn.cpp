//////////////////////////////////////////////////////////////////////////
// Purpose:   wxCommandLinkButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/commandlinkbutton.h>  // wxCommandLinkButtonBase and wxGenericCommandLinkButton classes

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_cmd_link_btn.h"

wxObject* CommandLinkBtnGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxCommandLinkButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_main_label),
                                          node->prop_as_wxString(prop_note), DlgPoint(parent, node, prop_pos),
                                          DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->prop_as_bool(prop_default))
        widget->SetDefault();

    if (node->prop_as_bool(prop_auth_needed))
        widget->SetAuthNeeded();

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->prop_as_wxBitmapBundle(prop_bitmap));

        if (node->HasValue(prop_disabled_bmp))
            widget->SetBitmapDisabled(node->prop_as_wxBitmapBundle(prop_disabled_bmp));

        if (node->HasValue(prop_pressed_bmp))
            widget->SetBitmapPressed(node->prop_as_wxBitmapBundle(prop_pressed_bmp));

        if (node->HasValue(prop_focus_bmp))
            widget->SetBitmapFocus(node->prop_as_wxBitmapBundle(prop_focus_bmp));

        if (node->HasValue(prop_current))
            widget->SetBitmapCurrent(node->prop_as_wxBitmapBundle(prop_current));

#if 0
        // REVIEW: [KeyWorks - 05-30-2022] As of 3.1.7, these don't work property on Windows 10.

        if (node->HasValue(prop_position))
            widget->SetBitmapPosition(static_cast<wxDirection>(node->prop_as_int(prop_position)));

        if (node->HasValue(prop_margins))
            widget->SetBitmapMargins(node->prop_as_wxSize(prop_margins));
#endif
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CommandLinkBtnGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    code << GenerateQuotedString(node, prop_main_label) << ", " << GenerateQuotedString(node, prop_note);

    GeneratePosSizeFlags(node, code, true);

    return code;
}

std::optional<ttlib::cstr> CommandLinkBtnGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_default))
    {
        if (code.size())
            code << '\n';

        code << node->get_node_name() << "->SetDefault();";
    }

    if (node->prop_as_bool(prop_auth_needed))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetAuthNeeded();";
    }

    if (node->HasValue(prop_bitmap))
    {
        auto_indent = indent::auto_keep_whitespace;

        GenBtnBimapCode(node, code);
    }

    return code;
}

std::optional<ttlib::cstr> CommandLinkBtnGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool CommandLinkBtnGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/commandlinkbutton.h>", set_src, set_hdr);
    return true;
}

int CommandLinkBtnGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxCommandLinkButton");

    ADD_ITEM_PROP(prop_main_label, "label")
    ADD_ITEM_PROP(prop_note, "note")
    ADD_ITEM_BOOL(prop_default, "default")

    GenXrcBitmap(node, item, xrc_flags);
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void CommandLinkBtnGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxCommandLinkButtonXmlHandler");
}
