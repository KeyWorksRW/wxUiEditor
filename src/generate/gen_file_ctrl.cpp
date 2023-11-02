/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFileCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filectrl.h>  // Header for wxFileCtrlBase and other common functions used by

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties

#include "gen_file_ctrl.h"

using namespace code;

wxObject* FileCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxString wild;
    if (node->hasValue(prop_wildcard))
        wild = node->as_wxString(prop_wildcard);
    else
        wild = wxFileSelectorDefaultWildcardStr;

    auto widget = new wxFileCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_initial_folder),
                                 node->as_wxString(prop_initial_filename), wild, GetStyleInt(node),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size));

    if (!(node->as_int(prop_style) & wxFC_NOSHOWHIDDEN))
        widget->ShowHidden(node->as_bool(prop_show_hidden));

    if (node->as_int(prop_filter_index) > 0)
        widget->SetFilterIndex(node->as_int(prop_filter_index));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool FileCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    code.Comma().QuotedString(prop_initial_folder).Comma().QuotedString(prop_initial_filename);
    code.Comma();
    if (code.hasValue(prop_wildcard))
        code.QuotedString(prop_wildcard);
    else
        code.AddType("wxFileSelectorDefaultWildcardStr");

    // Unlike most controls, wxFileCtrl expects the style to be specified *before* the
    // position and size.

    auto what_params = code.WhatParamsNeeded("wxFC_DEFAULT_STYLE");
    if (what_params != nothing_needed)
    {
        code.Comma().CheckLineLength(prop_style).Style();
        if (what_params != style_needed)
        {
            code.Comma().CheckLineLength().Pos().Comma().CheckLineLength().WxSize();
            if (what_params & window_name_needed)
                code.Comma().QuotedString(prop_window_name);
        }
    }
    code.EndFunction();

    return true;
}

bool FileCtrlGenerator::SettingsCode(Code& code)
{
    if (code.is_ruby() && Project.getProjectNode()->as_string(prop_wxRuby_version) == "0.9.0")
    {
        return false;
    }

    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
        {
            code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
        }
    }

    if (code.IntValue(prop_filter_index) > 0)
    {
        code.Eol(eol_if_empty).NodeName().Function("SetFilterIndex(").as_string(prop_filter_index).EndFunction();
    }

    if (code.IsTrue(prop_show_hidden))
    {
        code.Eol(eol_if_empty).NodeName().Function("ShowHidden(").True().EndFunction();
    }

    return true;
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
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxFileCtrl");

    ADD_ITEM_PROP(prop_initial_folder, "defaultdirectory")
    ADD_ITEM_PROP(prop_initial_filename, "defaultfilename")
    ADD_ITEM_PROP(prop_wildcard, "wildcard")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_int(prop_filter_index) > 0)
            ADD_ITEM_COMMENT("XRC does not support calling SetFilterIndex()")
        if (node->as_bool(prop_show_hidden))
            ADD_ITEM_COMMENT("XRC does not support calling ShowHidden()")
        GenXrcComments(node, item);
    }

    return result;
}

void FileCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxFileCtrlXmlHandler");
}
