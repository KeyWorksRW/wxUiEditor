/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFileCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filectrl.h>  // Header for wxFileCtrlBase and other common functions used by

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_file_ctrl.h"

wxObject* FileCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxString wild;
    if (node->HasValue(prop_wildcard))
        wild = node->prop_as_wxString(prop_wildcard);
    else
        wild = wxFileSelectorDefaultWildcardStr;

    auto widget = new wxFileCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_initial_folder),
                                 node->prop_as_wxString(prop_initial_filename), wild, GetStyleInt(node),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size));

    if (!(node->prop_as_int(prop_style) & wxFC_NOSHOWHIDDEN))
        widget->ShowHidden(node->prop_as_bool(prop_show_hidden));

    if (node->prop_as_int(prop_filter_index) > 0)
        widget->SetFilterIndex(node->prop_as_int(prop_filter_index));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::sview> FileCtrlGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.GetParentName().Comma().as_string(prop_id);
    code.Comma().QuotedString(prop_initial_folder).Comma().QuotedString(prop_initial_filename);
    code.Comma();
    if (code.HasValue(prop_wildcard))
        code.QuotedString(prop_wildcard);
    else
        code.Add("wxFileSelectorDefaultWildcardStr");

    // Unlike most controls, wxFileCtrl expects the style to be specified *before* the
    // position and size.

    auto what_params = code.WhatParamsNeeded("wxFC_DEFAULT_STYLE");
    if (what_params != Code::nothing_needed)
    {
        code.Comma().CheckLineLength(prop_style).Style();
        if (what_params != Code::style_needed)
        {
            code.Comma().CheckLineLength().Pos().Comma().CheckLineLength().WxSize();
            if (what_params & Code::window_name_needed)
                code.Comma().QuotedString(prop_window_name);
        }
    }
    code.EndFunction();

    return code.m_code;
}

std::optional<ttlib::sview> FileCtrlGenerator::CommonSettings(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        code.Eol(true).NodeName().Function("SetFocus(").EndFunction();
    }

    if (code.IntValue(prop_filter_index) > 0)
    {
        code.Eol(true).NodeName().Function("SetFilterIndex(").Str(prop_filter_index).EndFunction();
    }

    if (code.IsTrue(prop_show_hidden))
    {
        code.Eol(true).NodeName().Function("ShowHidden(").AddTrue().EndFunction();
    }

    return code.m_code;
}

bool FileCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/filectrl.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_filectrl.cpp
// ../../../wxWidgets/src/xrc/xh_filectrl.cpp

int FileCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxFileCtrl");

    ADD_ITEM_PROP(prop_initial_folder, "defaultdirectory")
    ADD_ITEM_PROP(prop_initial_filename, "defaultfilename")
    ADD_ITEM_PROP(prop_wildcard, "wildcard")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->prop_as_int(prop_filter_index) > 0)
            ADD_ITEM_COMMENT("XRC does not support calling SetFilterIndex()")
        if (node->prop_as_bool(prop_show_hidden))
            ADD_ITEM_COMMENT("XRC does not support calling ShowHidden()")
        GenXrcComments(node, item);
    }

    return result;
}

void FileCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxFileCtrlXmlHandler");
}
