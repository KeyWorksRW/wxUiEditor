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
    auto widget = new wxCommandLinkButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_main_label),
                                          node->as_wxString(prop_note), DlgPoint(parent, node, prop_pos),
                                          DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->as_bool(prop_default))
        widget->SetDefault();

    if (node->as_bool(prop_auth_needed))
        widget->SetAuthNeeded();

    if (node->hasValue(prop_bitmap))
    {
        widget->SetBitmap(node->as_wxBitmapBundle(prop_bitmap));

        if (node->hasValue(prop_disabled_bmp))
            widget->SetBitmapDisabled(node->as_wxBitmapBundle(prop_disabled_bmp));

        if (node->hasValue(prop_pressed_bmp))
            widget->SetBitmapPressed(node->as_wxBitmapBundle(prop_pressed_bmp));

        if (node->hasValue(prop_focus_bmp))
            widget->SetBitmapFocus(node->as_wxBitmapBundle(prop_focus_bmp));

        if (node->hasValue(prop_current))
            widget->SetBitmapCurrent(node->as_wxBitmapBundle(prop_current));

#if 0
        // REVIEW: [KeyWorks - 05-30-2022] As of 3.1.7, these don't work property on Windows 10.

        if (node->hasValue(prop_position))
            widget->SetBitmapPosition(static_cast<wxDirection>(node->as_int(prop_position)));

        if (node->hasValue(prop_margins))
            widget->SetBitmapMargins(node->as_wxSize(prop_margins));
#endif
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool CommandLinkBtnGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_main_label);
    code.Comma().QuotedString(prop_note).PosSizeFlags(true);

    return true;
}

bool CommandLinkBtnGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_default))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetDefault(").EndFunction();
    }

    if (code.IsTrue(prop_auth_needed))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetAuthNeeded(").EndFunction();
    }

    if (code.hasValue(prop_bitmap))
    {
        if (code.is_cpp())
            GenBtnBimapCode(code.node(), code);
        else
            PythonBtnBimapCode(code);
    }
    return true;
}

bool CommandLinkBtnGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/commandlinkbutton.h>", set_src, set_hdr);
    return true;
}

int CommandLinkBtnGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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
