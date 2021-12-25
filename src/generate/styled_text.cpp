/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStyledText (scintilla) generate
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// See https://docs.wxwidgets.org/trunk/classwx_styled_text_ctrl.html for wxWidgets documentation
// See https://www.scintilla.org/ScintillaDoc.html for Scintilla documentation

#include <wx/stc/stc.h>  // A wxWidgets implementation of Scintilla.

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "styled_text.h"

#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "utils.h"         // Utility functions that work with properties

// Call g_NodeCreator->GetConstantAsInt("wx_define") to get the #defined integer value -- see node_creator.h

std::map<std::string, const char*> g_stc_flags = {

    { "no wrapping", "wxSTC_WRAP_NONE" },
    { "word", "wxSTC_WRAP_WORD" },
    { "character", "wxSTC_WRAP_CHAR" },
    { "whitespace", "wxSTC_WRAP_WHITESPACE" },

    // Visual flags
    { "end", "wxSTC_WRAPVISUALFLAG_END" },
    { "start", "wxSTC_WRAPVISUALFLAG_START" },
    { "margin", "wxSTC_WRAPVISUALFLAG_MARGIN" },

    { "end_text", "wxSTC_WRAPVISUALFLAGLOC_END_BY_TEXT" },
    { "start_text", "wxSTC_WRAPVISUALFLAGLOC_START_BY_TEXT" },

    { "fixed", "wxSTC_WRAPINDENT_FIXED" },
    { "same", "wxSTC_WRAPINDENT_SAME" },
    { "indent", "wxSTC_WRAPINDENT_INDENT" },
    // { "deep_indent", "SC_WRAPINDENT_DEEPINDENT" }, // not supported in 3.1.15
};

wxObject* StyledTextGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto scintilla =
        new wxStyledTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node), node->prop_as_wxString(prop_var_name));

    if (node->HasValue(prop_hint))
        scintilla->SetHint(node->prop_as_wxString(prop_hint));

    // Wrap category settings

    if (!node->prop_as_string(prop_stc_wrap_mode).is_sameas("no wrapping"))
        scintilla->SetWrapMode(g_NodeCreator.GetConstantAsInt(g_stc_flags.at(node->prop_as_string(prop_stc_wrap_mode))));
    if (node->HasValue(prop_stc_wrap_visual_flag))
    {
        ttlib::multistr flags(node->prop_as_string(prop_stc_wrap_visual_flag), '|');
        int value = 0;
        for (auto& iter: flags)
        {
            value |= g_NodeCreator.GetConstantAsInt(g_stc_flags.at(iter));
        }
        if (value)
            scintilla->SetWrapVisualFlags(value);
    }
    if (node->HasValue(prop_stc_wrap_visual_location))
    {
        ttlib::multistr flags(node->prop_as_string(prop_stc_wrap_visual_location), '|');
        int value = 0;
        for (auto& iter: flags)
        {
            value |= g_NodeCreator.GetConstantAsInt(g_stc_flags.at(iter));
        }
        if (value)
            scintilla->SetWrapVisualFlagsLocation(value);
    }
    if (!node->prop_as_string(prop_stc_wrap_indent_mode).is_sameas("fixed"))
        scintilla->SetWrapIndentMode(
            g_NodeCreator.GetConstantAsInt(g_stc_flags.at(node->prop_as_string(prop_stc_wrap_indent_mode))));
    if (node->HasValue(prop_stc_wrap_start_indent))
        scintilla->SetWrapStartIndent(node->prop_as_int(prop_stc_wrap_start_indent));

    if (node->prop_as_int(prop_line_numbers) != 0)
    {
        scintilla->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        scintilla->SetMarginWidth(0, scintilla->TextWidth(wxSTC_STYLE_LINENUMBER, "_99999"));
    }
    else
    {
        scintilla->SetMarginWidth(0, 0);
    }

    if (node->prop_as_int(prop_folding) != 0)
    {
        scintilla->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
        scintilla->SetMarginMask(1, wxSTC_MASK_FOLDERS);
        scintilla->SetMarginWidth(1, 16);
        scintilla->SetMarginSensitive(1, true);

        scintilla->SetProperty("fold", "1");
        scintilla->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
    }
    else
    {
        scintilla->SetMarginWidth(1, 0);
    }
    scintilla->SetIndentationGuides(node->prop_as_int(prop_indentation_guides));
    scintilla->SetUseTabs((node->prop_as_int(prop_use_tabs)));
    scintilla->SetTabWidth(node->prop_as_int(prop_tab_width));
    scintilla->SetTabIndents((node->prop_as_int(prop_tab_indents)));
    scintilla->SetBackSpaceUnIndents((node->prop_as_int(prop_backspace_unindents)));
    scintilla->SetIndent(node->prop_as_int(prop_tab_width));
    scintilla->SetViewEOL((node->prop_as_int(prop_view_eol)));
    scintilla->SetViewWhiteSpace(node->prop_as_int(prop_view_whitespace));

    if (node->HasValue(prop_font))
    {
        scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, node->prop_as_font(prop_font));
    }

    scintilla->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return scintilla;
}

std::optional<ttlib::cstr> StyledTextGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> StyledTextGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    // There are potentially a LOT of settings, so we put them all in a bracket pair to make them easier to identifiy

    auto_indent = false;
    code << "\t{";

    if (node->HasValue(prop_hint))
    {
        if (code.size())
            code << "\n\t\t";
        code << node->get_node_name() << "->SetHint(" << GenerateQuotedString(node->prop_as_string(prop_hint)) << ");";
    }

    if (node->prop_as_bool(prop_folding))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetProperty(\"fold\", \"1\");";

        // TOD: [KeyWorks - 12-10-2020] All of these need to be user-settable options
        code << "\n\t\t" << node->get_node_name() << "->SetMarginType(1, wxSTC_MARGIN_SYMBOL);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginMask(1, wxSTC_MASK_FOLDERS);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginWidth(1, 16);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginSensitive(1, true);";
        code << "\n\t\t" << node->get_node_name()
             << "->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);";
    }

    // Wrap category settings

    if (!node->prop_as_string(prop_stc_wrap_mode).is_sameas("no wrapping"))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetWrapMode("
             << g_stc_flags.at(node->prop_as_string(prop_stc_wrap_mode)) << ");";
    }
    if (node->HasValue(prop_stc_wrap_visual_flag))
    {
        ttlib::multistr flags(node->prop_as_string(prop_stc_wrap_visual_flag), '|');
        ttlib::cstr value;
        for (auto& iter: flags)
        {
            if (value.size())
                value << '|';
            value << g_stc_flags.at(iter);
        }
        if (value.size())
            code << "\n\t\t" << node->get_node_name() << "->SetWrapVisualFlags(" << value << ");";
    }
    if (node->HasValue(prop_stc_wrap_visual_location))
    {
        ttlib::multistr flags(node->prop_as_string(prop_stc_wrap_visual_location), '|');
        ttlib::cstr value;
        for (auto& iter: flags)
        {
            if (value.size())
                value << '|';
            value << g_stc_flags.at(iter);
        }
        if (value.size())
            code << "\n\t\t" << node->get_node_name() << "->SetWrapVisualFlagsLocation(" << value << ");";
    }
    if (!node->prop_as_string(prop_stc_wrap_indent_mode).is_sameas("fixed"))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetWrapIndentMode("
             << g_stc_flags.at(node->prop_as_string(prop_stc_wrap_indent_mode)) << ");";
    }
    if (node->HasValue(prop_stc_wrap_start_indent))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetWrapStartIndent(" << node->prop_as_int(prop_stc_wrap_start_indent)
             << ");";
    }

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_line_numbers))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetMarginWidth(0, 0);";
    }
    else
    {
        code << "\n\t\t" << node->get_node_name() << "->SetMarginType(0, wxSTC_MARGIN_NUMBER);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginWidth(0, " << node->get_node_name()
             << "->TextWidth(wxSTC_STYLE_LINENUMBER, \"_99999\"));";
    }

    // BUGBUG: [KeyWorks - 12-10-2020] Indentation is not a boolean -- there are 4 possible values
    if (!node->prop_as_bool(prop_indentation_guides))
        code << "\n\t\t" << node->get_node_name() << "->SetIndentationGuides(0);";

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_use_tabs))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetUseTabs(false);";

        if (node->prop_as_int(prop_tab_width) != 8)
            code << "\n\t\t" << node->get_node_name() << "->SetTabWidth(" << node->prop_as_string(prop_tab_width) << ");";
    }

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_tab_indents))
        code << "\n\t\t" << node->get_node_name() << "->SetTabIndents(false);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_backspace_unindents))
        code << "\n\t\t" << node->get_node_name() << "->SetBackSpaceUnIndents(true);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_view_eol))
        code << "\n\t\t" << node->get_node_name() << "->SetViewEOL(true);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_view_whitespace))
        code << "\n\t\t" << node->get_node_name() << "->SetViewWhiteSpace(true);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_read_only))
        code << "\n\t\t" << node->get_node_name() << "->SetReadOnly(true);";

    code << "\n\t}";

    if (code.is_sameas("\t{\n\t}"))
        code.clear();  // means there were no settings

    return code;
}

std::optional<ttlib::cstr> StyledTextGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StyledTextGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/stc/stc.h>", set_src, set_hdr);
    return true;
}
