/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStyledText (scintilla) generate
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// See https://docs.wxwidgets.org/trunk/classwx_styled_text_ctrl.html for wxWidgets documentation
// See https://www.scintilla.org/ScintillaDoc.html for Scintilla documentation

#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid
#include <wx/sizer.h>              // provide wxSizer class for layout
#include <wx/stc/stc.h>            // A wxWidgets implementation of Scintilla.

#include "styled_text.h"

#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "utils.h"         // Utility functions that work with properties

// Call g_NodeCreator->GetConstantAsInt("wx_define") to get the #defined integer value -- see node_creator.h

// To get the constant, prefix the name with "wxSTC_LEX_"
std::map<std::string, int> g_stc_lexers = {

    { "A68K", 100 },
    { "ABAQUS", 84 },
    { "ADA", 20 },
    { "APDL", 61 },
    { "AS", 113 },
    { "ASM", 34 },
    { "ASN1", 63 },
    { "ASYMPTOTE", 85 },
    { "AU3", 60 },
    { "AVE", 19 },
    { "AVS", 104 },
    { "BAAN", 31 },
    { "BASH", 62 },
    { "BATCH", 12 },
    { "BIBTEX", 116 },
    { "BLITZBASIC", 66 },
    { "BULLANT", 27 },
    { "CAML", 65 },
    { "CLW", 45 },
    { "CLWNOCASE", 46 },
    { "CMAKE", 80 },
    { "COBOL", 92 },
    { "COFFEESCRIPT", 102 },
    { "CONF", 17 },
    { "CONTAINER", 0 },
    { "CPP", 3 },
    { "CPPNOCASE", 35 },
    { "CSOUND", 74 },
    { "CSS", 38 },
    { "D", 79 },
    { "DIFF", 16 },
    { "DMAP", 112 },
    { "DMIS", 114 },
    { "ECL", 105 },
    { "EDIFACT", 121 },
    { "EIFFEL", 23 },
    { "EIFFELKW", 24 },
    { "ERLANG", 53 },
    { "ERRORLIST", 10 },
    { "ESCRIPT", 41 },
    { "F77", 37 },
    { "FLAGSHIP", 73 },
    { "FORTH", 52 },
    { "FORTRAN", 36 },
    { "FREEBASIC", 75 },
    { "GAP", 81 },
    { "GUI4CLI", 58 },
    { "HASKELL", 68 },
    { "HTML", 4 },
    { "IHEX", 118 },
    { "INNOSETUP", 76 },
    { "JSON", 120 },
    { "KIX", 57 },
    { "KVIRC", 110 },
    { "LATEX", 14 },
    { "LISP", 21 },
    { "LITERATEHASKELL", 108 },
    { "LOT", 47 },
    { "LOUT", 40 },
    { "LUA", 15 },
    { "MAGIK", 87 },
    { "MAKEFILE", 11 },
    { "MARKDOWN", 98 },
    { "MATLAB", 32 },
    { "METAPOST", 50 },
    { "MMIXAL", 44 },
    { "MODULA", 101 },
    { "MSSQL", 55 },
    { "MYSQL", 89 },
    { "NIMROD", 96 },
    { "NNCRONTAB", 26 },
    { "NSIS", 43 },
    { "NULL", 1 },
    { "OCTAVE", 54 },
    { "OPAL", 77 },
    { "OSCRIPT", 106 },
    { "PASCAL", 18 },
    { "PERL", 6 },
    { "PHPSCRIPT", 69 },
    { "PLM", 82 },
    { "PO", 90 },
    { "POV", 39 },
    { "POWERBASIC", 51 },
    { "POWERPRO", 95 },
    { "POWERSHELL", 88 },
    { "PROGRESS", 83 },
    { "PROPERTIES", 9 },
    { "PS", 42 },
    { "PUREBASIC", 67 },
    { "PYTHON", 2 },
    { "R", 86 },
    { "REBOL", 71 },
    { "REGISTRY", 115 },
    { "RUBY", 22 },
    { "RUST", 111 },
    { "SCRIPTOL", 33 },
    { "SMALLTALK", 72 },
    { "SML", 97 },
    { "SORCUS", 94 },
    { "SPECMAN", 59 },
    { "SPICE", 78 },
    { "SQL", 7 },
    { "SREC", 117 },
    { "STTXT", 109 },
    { "TACL", 93 },
    { "TADS3", 70 },
    { "TAL", 91 },
    { "TCL", 25 },
    { "TCMD", 103 },
    { "TEHEX", 119 },
    { "TEX", 49 },
    { "TXT2TAGS", 99 },
    { "VB", 8 },
    { "VBSCRIPT", 28 },
    { "VERILOG", 56 },
    { "VHDL", 64 },
    { "VISUALPROLOG", 107 },
    { "XCODE", 13 },
    { "XML", 5 },
    { "YAML", 48 },

};

// clang-format off

inline constexpr const auto txt_styled_sample = R"===(
// Sample text so that you can view effects of various settings

inline wxImage wxueImage(const unsigned char* long_parameter_name,size_t another_long_parameter_size_data)
{
    wxMemoryInputStream strm(another_long_parameter_size_data,size_data);
    {
        wxImage image;
        image.LoadFile(strm);
        return image;
    }
};
)===";

// clang-format on

wxObject* StyledTextGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto scintilla =
        new wxStyledTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node), node->prop_as_wxString(prop_var_name));

    // By default, scintilla sets this margin width to 16. We want to shut off all margins unless the user specifically
    // requests one.
    scintilla->SetMarginWidth(1, 0);

    if (node->HasValue(prop_stc_lexer) && node->prop_as_string(prop_stc_lexer) != "NULL")
    {
        scintilla->SetLexer(g_stc_lexers.at(node->prop_as_string(prop_stc_lexer)));
    }

    //////////// Wrap category settings ////////////

    if (!node->prop_as_string(prop_stc_wrap_mode).is_sameas("no wrapping"))
    {
        scintilla->SetWrapMode(node->prop_as_mockup(prop_stc_wrap_mode, "stc_"));
    }
    if (node->HasValue(prop_stc_wrap_visual_flag))
    {
        scintilla->SetWrapVisualFlags(node->prop_as_mockup(prop_stc_wrap_visual_flag, "stc_"));
    }
    if (node->HasValue(prop_stc_wrap_visual_location))
    {
        scintilla->SetWrapVisualFlagsLocation(node->prop_as_mockup(prop_stc_wrap_visual_location, "stc_"));
    }
    if (!node->prop_as_string(prop_stc_wrap_indent_mode).is_sameas("fixed"))
    {
        scintilla->SetWrapIndentMode(node->prop_as_mockup(prop_stc_wrap_indent_mode, "stc_"));
    }
    if (node->HasValue(prop_stc_wrap_start_indent))
    {
        scintilla->SetWrapStartIndent(node->prop_as_int(prop_stc_wrap_start_indent));
    }

    //////////// Margin category settings ////////////

    if (!node->prop_as_bool(prop_stc_select_wrapped_line))
    {
        scintilla->SetMarginOptions(wxSTC_MARGINOPTION_SUBLINESELECT);
    }

    if (node->prop_as_string(prop_line_margin) != "none")
    {
        auto margin = node->prop_as_string(prop_line_margin).atoi();

        scintilla->SetMarginType(margin, wxSTC_MARGIN_NUMBER);

        int width = node->prop_as_string(prop_line_digits).atoi();
        wxString numbers("_");
        while (width > 0)
        {
            numbers << '9';
            --width;
        }
        width = scintilla->TextWidth(wxSTC_STYLE_LINENUMBER, numbers);
        scintilla->SetMarginWidth(margin, width);
    }

    if (node->prop_as_string(prop_fold_margin) != "none" && node->prop_as_int(prop_fold_width))
    {
        auto margin = node->prop_as_string(prop_fold_margin).atoi();

        scintilla->SetProperty("fold", "1");

        if (node->prop_as_string(prop_fold_marker_style) == "arrow" ||
            node->prop_as_string(prop_fold_marker_style) == "plus/minus")
        {
            int symbol_folder;
            int symbol_open;

            if (node->prop_as_string(prop_fold_marker_style) == "plus/minus")
            {
                symbol_folder = wxSTC_MARK_PLUS;
                symbol_open = wxSTC_MARK_MINUS;
            }
            else
            {
                symbol_folder = wxSTC_MARK_ARROW;
                symbol_open = wxSTC_MARK_ARROWDOWN;
            }

            scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDER, symbol_folder);
            scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, symbol_open);
            scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, symbol_open);
            scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, symbol_folder);

            scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_BACKGROUND);
            scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_BACKGROUND);
            scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_BACKGROUND);

            if (node->HasValue(prop_fold_marker_colour))
            {
                scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, node->prop_as_wxColour(prop_fold_marker_colour));
                scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, node->prop_as_wxColour(prop_fold_marker_colour));
                scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, node->prop_as_wxColour(prop_fold_marker_colour));
                scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, node->prop_as_wxColour(prop_fold_marker_colour));
            }
        }
        else if (node->prop_as_string(prop_fold_marker_style) == "circle tree" ||
                 node->prop_as_string(prop_fold_marker_style) == "box tree")
        {
            // Not sure if this is a bug or by design, but the symbols for circle tree and box tree are drawn with the
            // background color -- which is the exact opposite of the arrow and plus/minus symbols. Note that the joining
            // lines won't show up at all unless we set their background to the default style foreground color.

            auto clr_foreground = scintilla->StyleGetForeground(wxSTC_STYLE_DEFAULT);
            scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, clr_foreground);
            scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, clr_foreground);
            scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, clr_foreground);
            scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, clr_foreground);

            scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, clr_foreground);
            scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, clr_foreground);
            scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, clr_foreground);

            auto clr_background = node->HasValue(prop_fold_marker_colour) ?
                                      node->prop_as_wxColour(prop_fold_marker_colour) :
                                      scintilla->StyleGetBackground(wxSTC_STYLE_DEFAULT);
            scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, clr_background);
            scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, clr_background);
            scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, clr_background);
            scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, clr_background);

            if (node->prop_as_string(prop_fold_marker_style) == "circle tree")
            {
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_CIRCLEPLUS);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_CIRCLEMINUS);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_CIRCLEMINUSCONNECTED);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_CIRCLEPLUSCONNECTED);

                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNERCURVE);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNERCURVE);
            }
            else
            {
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXMINUS);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXPLUS);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUSCONNECTED);

                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);
                scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER);
            }
        }

        scintilla->SetMarginWidth(margin, node->prop_as_int(prop_fold_width));
        scintilla->SetMarginType(margin, wxSTC_MARGIN_SYMBOL);
        scintilla->SetMarginMask(margin, wxSTC_MASK_FOLDERS);
        scintilla->SetMarginSensitive(margin, true);
        if (node->HasValue(prop_automatic_folding))
        {
            scintilla->SetAutomaticFold(node->prop_as_mockup(prop_automatic_folding, "stc_"));
        }
        if (node->HasValue(prop_fold_flags))
        {
            scintilla->SetFoldFlags(node->prop_as_mockup(prop_fold_flags, "stc_"));
        }
    }

    if (node->prop_as_string(prop_symbol_margin) != "none")
    {
        auto margin = node->prop_as_string(prop_symbol_margin).atoi();

        scintilla->SetMarginWidth(margin, 16);
        scintilla->SetMarginType(margin, wxSTC_MARGIN_SYMBOL);
        scintilla->SetMarginMask(margin, ~wxSTC_MASK_FOLDERS);
        scintilla->SetMarginSensitive(margin, node->prop_as_bool(prop_symbol_mouse_sensitive));
    }
    if (node->prop_as_string(prop_separator_margin) != "none")
    {
        auto margin = node->prop_as_string(prop_separator_margin).atoi();

        scintilla->SetMarginWidth(margin, 1);
        scintilla->SetMarginType(margin, wxSTC_MARGIN_FORE);
    }

    if (node->prop_as_string(prop_custom_margin) != "none")
    {
        auto margin = node->prop_as_string(prop_custom_margin).atoi();
        scintilla->SetMarginWidth(margin, node->prop_as_int(prop_custom_width));

        scintilla->SetMarginType(margin, node->prop_as_mockup(prop_custom_type, "stc_"));

        if (node->prop_as_string(prop_custom_type) == "colour" && node->HasValue(prop_custom_colour))
        {
            scintilla->SetMarginBackground(margin, node->prop_as_wxColour(prop_custom_colour));
        }

        if (node->prop_as_string(prop_custom_type) == "symbol" || node->prop_as_string(prop_custom_type) == "number")
        {
            scintilla->SetMarginMask(margin, node->prop_as_bool(prop_custom_mask_folders) ? wxSTC_MASK_FOLDERS :
                                                                                            ~wxSTC_MASK_FOLDERS);
        }
        scintilla->SetMarginSensitive(margin, node->prop_as_bool(prop_custom_mouse_sensitive));
    }

    //////////// Selection category settings ////////////

    scintilla->SetMouseSelectionRectangularSwitch(node->prop_as_bool(prop_allow_mouse_rectangle));
    scintilla->SetMultipleSelection(node->prop_as_bool(prop_multiple_selections));
    scintilla->SetMultiPaste(node->prop_as_bool(prop_paste_multiple) ? wxSTC_MULTIPASTE_EACH : wxSTC_MULTIPASTE_ONCE);
    scintilla->SetAdditionalCaretsVisible(node->prop_as_bool(prop_additional_carets_visible));
    scintilla->SetAdditionalCaretsBlink(node->prop_as_bool(prop_additional_carets_blink));
    scintilla->SetAdditionalSelectionTyping(node->prop_as_bool(prop_multiple_selection_typing));

    //////////// Tabs and Indentation settings ////////////

    if (node->HasValue(prop_indentation_guides))
    {
        scintilla->SetIndentationGuides(node->prop_as_mockup(prop_indentation_guides, "stc_"));
    }
    scintilla->SetIndent(node->prop_as_int(prop_stc_indentation_size));
    scintilla->SetUseTabs((node->prop_as_int(prop_use_tabs)));
    scintilla->SetTabWidth(node->prop_as_int(prop_tab_width));
    scintilla->SetTabIndents((node->prop_as_int(prop_tab_indents)));
    scintilla->SetBackSpaceUnIndents((node->prop_as_int(prop_backspace_unindents)));
    scintilla->SetIndent(node->prop_as_int(prop_tab_width));

    //////////// General settings ////////////

    if (node->prop_as_int(prop_stc_left_margin_width) == 5)
    {
        scintilla->SetMarginLeft(wxSizerFlags::GetDefaultBorder());
    }
    else
    {
        scintilla->SetMarginLeft(node->prop_as_int(prop_stc_left_margin_width));
    }

    if (node->prop_as_int(prop_stc_right_margin_width) == 5)
    {
        scintilla->SetMarginRight(wxSizerFlags::GetDefaultBorder());
    }
    else
    {
        scintilla->SetMarginRight(node->prop_as_int(prop_stc_right_margin_width));
    }

    if (node->HasValue(prop_eol_mode))
    {
        scintilla->SetEOLMode(node->prop_as_mockup(prop_eol_mode, "stc_"));
    }

    scintilla->SetViewEOL(node->prop_as_bool(prop_view_eol));
    if (!node->isPropValue(prop_view_whitespace, "invisible"))
    {
        scintilla->SetViewWhiteSpace(node->prop_as_mockup(prop_view_whitespace, "stc_"));
    }
    if (node->prop_as_bool(prop_view_tab_strikeout))
    {
        scintilla->SetTabDrawMode(wxSTC_TD_STRIKEOUT);
    }

    if (node->HasValue(prop_font))
    {
        scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, node->prop_as_font(prop_font));
    }

    // Now that all settings have been called, add some sample text.

    tt_string sample(txt_styled_sample);
    if (node->prop_as_string(prop_stc_lexer) == "CPP")
    {
        scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
    }
    else if (node->prop_as_string(prop_stc_lexer) == "PHP")
    {
        scintilla->StyleSetForeground(wxSTC_HPHP_COMMENT, wxColour(0, 128, 0));
    }
    else if (node->prop_as_string(prop_stc_lexer) == "PYTHON" || node->prop_as_string(prop_stc_lexer) == "CMAKE")
    {
        sample.Replace("//", "#");
        scintilla->StyleSetForeground(wxSTC_P_COMMENTLINE, wxColour(0, 128, 0));
    }
    else if (node->prop_as_string(prop_stc_lexer) == "RUBY")
    {
        sample.Replace("//", "#");
        scintilla->StyleSetForeground(wxSTC_RB_COMMENTLINE, wxColour(0, 128, 0));
    }
    else if (node->prop_as_string(prop_stc_lexer) == "LUA")
    {
        sample.Replace("//", "--");
        scintilla->StyleSetForeground(wxSTC_LUA_COMMENTLINE, wxColour(0, 128, 0));
    }
    else if (node->prop_as_string(prop_stc_lexer) == "BATCH")
    {
        sample.Replace("//", "REM");
        scintilla->StyleSetForeground(wxSTC_BAT_COMMENT, wxColour(0, 128, 0));
    }
    else if (node->prop_as_string(prop_stc_lexer) == "HTML" || node->prop_as_string(prop_stc_lexer) == "XML")
    {
        sample.Replace("//", "<!--");
        sample << " -->";
        scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
    }

    scintilla->AddTextRaw(sample.c_str());

    scintilla->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return scintilla;
}

bool StyledTextGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().Add(prop_id);
    code.PosSizeFlags(true);

    // If the last parameter is wxID_ANY, then remove it. This is the default value, so it's
    // not needed.
    code.Replace(", wxID_ANY)", ")");

    return true;
}

bool StyledTextGenerator::SettingsCode(Code& code)
{
    Node* node = code.node();

    // There are potentially a LOT of settings, so we put them all in a bracket pair to make them easier to identifiy. This
    // is only done for C++ as Python syntax checkers don't like
    code.OpenBrace();

    if (code.HasValue(prop_stc_lexer) && !code.is_value(prop_stc_lexer, "NULL"))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetLexer(").Add("wxSTC_LEX_").Str(prop_stc_lexer).EndFunction();
    }

    // Default is false, so only set if true
    if (code.IsTrue(prop_read_only))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetReadOnly(").AddTrue().EndFunction();
    }

    if (code.HasValue(prop_eol_mode))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetEOLMode(").AddConstant(prop_eol_mode, "stc_").EndFunction();
    }

    // Default is false, so only set if true
    if (code.IsTrue(prop_view_eol))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetViewEol(").AddTrue().EndFunction();
    }

    if (!code.is_value(prop_view_whitespace, "invisible"))
    {
        code.Eol(eol_if_needed)
            .NodeName()
            .Function("SetViewWhiteSpace(")
            .AddConstant(prop_view_whitespace, "stc_")
            .EndFunction();
        if (code.IsTrue(prop_view_tab_strikeout))
        {
            code.Eol().NodeName().Function("SetTabDrawMode(").Add("wxSTC_TD_STRIKEOUT").EndFunction();
        }
    }

    //////////// Wrap category settings ////////////

    if (!code.is_value(prop_stc_wrap_mode, "no wrapping"))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetWrapMode(").AddConstant(prop_stc_wrap_mode, "stc_").EndFunction();
    }

    if (code.HasValue(prop_stc_wrap_visual_flag))
    {
        if (auto result = code.node()->prop_as_constant(prop_stc_wrap_visual_flag, "stc_"); result.size())
        {
            code.Eol(eol_if_needed).NodeName().Function("SetWrapVisualFlags(").Add(result).EndFunction();
        }
    }

    if (code.HasValue(prop_stc_wrap_visual_location))
    {
        if (auto result = code.node()->prop_as_constant(prop_stc_wrap_visual_location, "stc_"); result.size())
        {
            code.Eol(eol_if_needed).NodeName().Function("SetWrapVisualFlagsLocation(").Add(result).EndFunction();
        }
    }

    if (!code.is_value(prop_stc_wrap_indent_mode, "fixed"))
    {
        code.Eol(eol_if_needed)
            .NodeName()
            .Function("SetWrapIndentMode(")
            .AddConstant(prop_stc_wrap_indent_mode, "stc_")
            .EndFunction();
    }

    if (code.HasValue(prop_stc_wrap_start_indent))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetWrapStartIndent(").Str(prop_stc_wrap_start_indent).EndFunction();
    }

    //////////// Selection category settings ////////////
    if (code.IsTrue(prop_multiple_selections))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetMultipleSelection(").Add("wxSTC_MULTIPASTE_EACH").EndFunction();
        if (code.IsTrue(prop_paste_multiple))
        {
            code.Eol().NodeName().Function("SetMultiPaste(").Add("wxSTC_MULTIPASTE_EACH").EndFunction();
        }
        code.Eol().NodeName().Function("SetAdditionalSelectionTyping(");
        code.TrueFalseIf(prop_additional_carets_blink).EndFunction();

        if (!code.IsTrue(prop_additional_carets_visible))
        {
            code.Eol().NodeName().Function("SetAdditionalCaretsVisible(").AddFalse().EndFunction();
        }
        else
        {
            code.Eol().NodeName().Function("SetAdditionalCaretsBlink(");
            code.TrueFalseIf(prop_additional_carets_blink).EndFunction();
        }
    }
    //////////// Margin category settings ////////////

    // The default margin is 1, so if that's what it is set to, then don't output any code
    if (!code.is_value(prop_stc_left_margin_width, 1))
    {
        if (code.is_value(prop_stc_left_margin_width, 5))
        {
            code.Eol(eol_if_needed).AddComment("Sets text margin scaled appropriately for the current DPI on Windows,");
            code.Eol().AddComment("5 on wxGTK or wxOSX");
            code.Eol()
                .NodeName()
                .Function("SetMarginLeft(")
                .Add("wxSizerFlags")
                .ClassMethod("GetDefaultBorder()")
                .EndFunction();
        }
        else
        {
            code.Eol(eol_if_needed).NodeName().Function("SetMarginLeft(").Str(prop_stc_left_margin_width).EndFunction();
        }
    }

    if (!code.is_value(prop_stc_right_margin_width, 1))
    {
        if (!code.is_value(prop_stc_left_margin_width, 5) && code.is_value(prop_stc_right_margin_width, 5))
        {
            code.Eol(eol_if_needed);
            code.AddComment("Sets text margin scaled appropriately for the current DPI on Windows");
            code.Eol().AddComment("5 on wxGTK or wxOSX");
        }
        code.Eol(eol_if_needed).NodeName().Function("SetMarginRight(");
        if (code.is_value(prop_stc_right_margin_width, 5))
        {
            code.Add("wxSizerFlags").ClassMethod("GetDefaultBorder()").EndFunction();
        }
        else
        {
            code.Str(prop_stc_right_margin_width).EndFunction();
        }
    }

    if (code.IsFalse(prop_stc_select_wrapped_line))
    {
        code.Eol(eol_if_needed)
            .NodeName()
            .Function("SetMarginOptions(")
            .Add("wxSTC_MARGINOPTION_SUBLINESELECT")
            .EndFunction();
    }

    // By default, scintilla sets margin one to a width to 16. We want to shut off all margins unless the user
    // specifically uses it.
    bool is_margin_1_set { false };

    // These values can be set to "none" so you have to do a string comparison

    {
        auto lambda = [&](PropName name)
        {
            if (code.is_value(name, "1"))
            {
                is_margin_1_set = true;
            }
        };
        lambda(prop_fold_margin);
        lambda(prop_line_margin);
        lambda(prop_separator_margin);
        lambda(prop_symbol_margin);
        lambda(prop_custom_width);
    }

    if (!is_margin_1_set)
    {
        code.Eol(eol_if_needed).NodeName().Function("SetMarginWidth(1, 0").EndFunction().AddComment("Remove default margin");
    }

    tt_string margin = node->as_string(prop_fold_margin);
    if (margin.is_sameas("none"))
        margin = "0";

    if (!code.is_value(prop_line_margin, "none"))
    {
        int width = node->as_string(prop_line_digits).atoi();

        tt_string numbers("_");
        while (width > 0)
        {
            numbers << '9';
            --width;
        }

        code.Eol(eol_if_needed).NodeName().Function("SetMarginWidth(").Str(margin).Comma();
        code.NodeName().Function("TextWidth(").Add("wxSTC_STYLE_LINENUMBER, ").QuotedString(numbers).Str(")").EndFunction();
        code.Eol().NodeName().Function("SetMarginType(").Str(margin).Comma().Add("wxSTC_MARGIN_NUMBER").EndFunction();
    }

    if (!code.is_value(prop_fold_margin, "none") && code.IntValue(prop_fold_width) > 0)
    {
        code.Eol(eol_if_needed).NodeName().Function("SetProperty(\"fold\", \"1\"").EndFunction();
        code.Eol().NodeName().Function("SetMarginWidth(").Str(margin).Comma().Str("16").EndFunction();
        code.Eol().NodeName().Function("SetMarginType(").Str(margin).Comma().Add("wxSTC_MARGIN_SYMBOL").EndFunction();
        code.Eol().NodeName().Function("SetMarginMask(").Str(margin).Comma().Add("wxSTC_MASK_FOLDERS").EndFunction();
        code.Eol().NodeName().Function("SetMarginSensitive(").Str(margin).Comma().AddTrue().EndFunction();

        if (node->HasValue(prop_automatic_folding))
        {
            code.Eol().NodeName().Function("SetAutomaticFold(").AddConstant(prop_automatic_folding, "stc_").EndFunction();
        }
        if (node->HasValue(prop_fold_flags))
        {
            code.Eol().NodeName().Function("SetFoldFlags(").AddConstant(prop_fold_flags, "stc_").EndFunction();
        }

        if (node->prop_as_string(prop_fold_marker_style) == "arrow" ||
            node->prop_as_string(prop_fold_marker_style) == "plus/minus")
        {
            std::string symbol_folder;
            std::string symbol_open;

            if (node->prop_as_string(prop_fold_marker_style) == "plus/minus")
            {
                symbol_folder = "wxSTC_MARK_PLUS";
                symbol_open = "wxSTC_MARK_MINUS";
            }
            else
            {
                symbol_folder = "wxSTC_MARK_ARROW";
                symbol_open = "wxSTC_MARK_ARROWDOWN";
            }

            if (node->HasValue(prop_fold_marker_colour))
            {
                auto lambda = [&](tt_string_view name, const std::string& symbol)
                {
                    code.Eol().NodeName().Function("MarkerDefine(").Add(name).Comma();
                    code.Add(symbol).Comma().Add("wxNullColour, ").ColourCode(prop_fold_marker_colour).EndFunction();
                };
                lambda("wxSTC_MARKNUM_FOLDER", symbol_folder);
                lambda("wxSTC_MARKNUM_FOLDEROPEN", symbol_open);
                lambda("wxSTC_MARKNUM_FOLDEROPENMID", symbol_open);
                lambda("wxSTC_MARKNUM_FOLDEREND", symbol_folder);
            }
            else
            {
                auto lambda = [&](tt_string_view name, const std::string& symbol)
                {
                    code.Eol().NodeName().Function("MarkerDefine(").Add(name).Comma();
                    code.Add(symbol).EndFunction();
                };
                lambda("wxSTC_MARKNUM_FOLDER", symbol_folder);
                lambda("wxSTC_MARKNUM_FOLDEROPEN", symbol_open);
                lambda("wxSTC_MARKNUM_FOLDEROPENMID", symbol_open);
                lambda("wxSTC_MARKNUM_FOLDEREND", symbol_folder);
            }
            {
                auto lambda = [&](tt_string_view name)
                {
                    code.Eol().NodeName().Function("MarkerDefine(").Add(name).Comma();
                    code.Add("wxSTC_MARK_BACKGROUND").EndFunction();
                };
                lambda("wxSTC_MARKNUM_FOLDERMIDTAIL");
                lambda("wxSTC_MARKNUM_FOLDERSUB");
                lambda("wxSTC_MARKNUM_FOLDERTAIL");
            }
        }
        else  // circle tree or box tree
        {
            code.OpenBrace();
            code.Eol().AddComment("The outline colour of the circle and box tree symbols is reversed by default.");
            code.Eol().AddComment("The code below ensures that the symbol is visible.");
            code.Eol().Str(code.is_cpp() ? "auto clr_foreground" : "_clr_foreground_") += " = ";
            code.NodeName().Function("StyleGetForeground(").Add("wxSTC_STYLE_DEFAULT").EndFunction();
            code.Eol().Str(code.is_cpp() ? "clr_background" : "_clr_background_");
            if (node->HasValue(prop_fold_marker_colour))
            {
                code.ColourCode(prop_fold_marker_colour).EndFunction();
            }
            else
            {
                code.NodeName().Function("StyleGetForeground(").Add("wxSTC_STYLE_DEFAULT").EndFunction();
            }
            {
                auto lambda = [&](tt_string_view name)
                {
                    code.Eol().NodeName().Function("MarkerSetBackground(").Add(name).Comma();
                    code.Str(code.is_cpp() ? "clr_foreground" : "_clr_foreground_");
                };
                lambda("wxSTC_MARKNUM_FOLDER");
                lambda("wxSTC_MARKNUM_FOLDEROPEN");
                lambda("wxSTC_MARKNUM_FOLDEROPENMID");
                lambda("wxSTC_MARKNUM_FOLDERMIDTAIL");

                code.Eol().NodeName().Function("MarkerSetForeground(").Add("wxSTC_MARKNUM_FOLDEROPEN").Comma();
                code.Str(code.is_cpp() ? "clr_background" : "_clr_background_");

                code.Eol().NodeName().Function("MarkerSetForeground(").Add("wxSTC_MARKNUM_FOLDEROPENMID").Comma();
                code.Str(code.is_cpp() ? "clr_background" : "_clr_background_");
            }

            if (node->prop_as_string(prop_fold_marker_style) == "circle tree")
            {
                auto lambda = [&](tt_string_view mark_number, tt_string_view mark_symbol)
                {
                    code.Eol().NodeName().Function("MarkerDefine(").Add(mark_number).Comma();
                    code.Add(mark_symbol).EndFunction();
                };
                lambda("wxSTC_MARKNUM_FOLDER", "wxSTC_MARK_CIRCLE");
                lambda("wxSTC_MARKNUM_FOLDEROPEN", "wxSTC_MARK_CIRCLEMINUS");
                lambda("wxSTC_MARKNUM_FOLDEROPENMID", "wxSTC_MARK_CIRCLEMINUSCONNECTED");
                lambda("wxSTC_MARKNUM_FOLDEREND", "wxSTC_MARK_CIRCLEPLUSCONNECTED");
                lambda("wxSTC_MARKNUM_FOLDERMIDTAIL", "wxSTC_MARK_CIRCLEPLUSCONNECTED");
                lambda("wxSTC_MARKNUM_FOLDERSUB", "wxSTC_MARK_VLINE");
                lambda("wxSTC_MARKNUM_FOLDERTAIL", "wxSTC_MARK_LCORNERCURVE");
            }
            else
            {
                auto lambda = [&](tt_string_view mark_number, tt_string_view mark_symbol)
                {
                    code.Eol().NodeName().Function("MarkerDefine(").Add(mark_number).Comma();
                    code.Add(mark_symbol).EndFunction();
                };
                lambda("wxSTC_MARKNUM_FOLDER", "wxSTC_MARK_BOXMINUS");
                lambda("wxSTC_MARKNUM_FOLDEROPEN", "wxSTC_MARK_BOXPLUS");
                lambda("wxSTC_MARKNUM_FOLDEROPENMID", "wxSTC_MARK_BOXMINUSCONNECTED");
                lambda("wxSTC_MARKNUM_FOLDEREND", "wxSTC_MARK_BOXPLUSCONNECTED");
                lambda("wxSTC_MARKNUM_FOLDERMIDTAIL", "wxSTC_MARK_TCORNER");
                lambda("wxSTC_MARKNUM_FOLDERSUB", "wxSTC_MARK_VLINE");
                lambda("wxSTC_MARKNUM_FOLDERTAIL", "wxSTC_MARK_LCORNER");
            }
        }
    }

    if (node->prop_as_string(prop_symbol_margin) != "none")
    {
        code.Eol().NodeName().Function("SetMarginWidth(").Str(margin).Comma().Str("16").EndFunction();
        code.Eol().NodeName().Function("SetMarginType(").Str(margin).Comma().Add("wxSTC_MARGIN_SYMBOL").EndFunction();
        code.Eol().NodeName().Function("SetMarginMask(").Str(margin).Comma();
        code.Str("~").Add("wxSTC_MASK_FOLDERS").EndFunction();
        code.Eol().NodeName().Function("SetMarginSensitive(").Str(margin).Comma();
        code.TrueFalseIf(prop_symbol_mouse_sensitive).EndFunction();
    }

    if (node->prop_as_string(prop_separator_margin) != "none")
    {
        margin = node->as_string(prop_separator_margin);
        if (margin.is_sameas("none"))
            margin = "0";

        code.Eol().NodeName().Function("SetMarginWidth(").Add(margin);
        code.Comma().Str(prop_separator_width).EndFunction();
        code.Eol().NodeName().Function("SetMarginType(").Str(margin).Comma().Add("wxSTC_MARGIN_FORE").EndFunction();
    }

    if (node->prop_as_string(prop_custom_margin) != "none" && node->prop_as_int(prop_custom_width))
    {
        margin = node->prop_as_string(prop_custom_margin);
        if (margin.is_sameas("none"))
            margin = "0";

        code.Eol().NodeName().Function("SetMarginWidth(").Str(margin).Comma().Str(prop_custom_width).EndFunction();
        code.Eol().NodeName().Function("SetMarginType(").Str(margin);
        code.Comma().AddConstant(prop_custom_type, "stc_").EndFunction();

        if (code.IsEqualTo(prop_custom_type, "colour") && code.HasValue(prop_custom_colour))
        {
            code.Eol().NodeName().Function("SetMarginBackground(").Str(margin).Comma();
            code.ColourCode(prop_custom_colour).EndFunction();
        }
        else
        {
            code.Eol().NodeName().Function("SetMarginMask").Str(margin).Comma();
            if (code.IsTrue(prop_custom_mask_folders))
                code.Add("wxSTC_MASK_FOLDERS");
            else
                code.Str("~").Add("wxSTC_MASK_FOLDERS");
            code.EndFunction();
        }
        if (code.IsTrue(prop_custom_mouse_sensitive))
        {
            code.Eol().NodeName().Function("SetMarginSensitive(").Str(margin).Comma().True().EndFunction();
        }
    }

    //////////// Tabs and Indentation settings ////////////

    if (node->HasValue(prop_indentation_guides) && !code.is_value(prop_indentation_guides, "no guides") &&
        // false was what was used in previous versions as well as in some imported values
        !code.is_value(prop_indentation_guides, "false"))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetIndentationGuides(");
        code.AddConstant(prop_indentation_guides, "stc_").EndFunction();
    }

    if (!code.is_value(prop_stc_indentation_size, 0))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetIndent(");
        code.AddConstant(prop_stc_indentation_size, "stc_").EndFunction();
    }

    // Default is true, so only set if false
    if (code.IsFalse(prop_use_tabs))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetUseTabs(").False().EndFunction();

        if (code.IntValue(prop_tab_width) != 8)
        {
            code.Eol().NodeName().Function("SetTabWidth(").Str(prop_tab_width).EndFunction();
        }
    }

    // Default is true, so only set if false
    if (code.IsFalse(prop_tab_indents))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetTabIndents(").False().EndFunction();
    }

    // Default is false, so only set if true
    if (code.IsTrue(prop_backspace_unindents))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetBackSpaceUnIndents(").True().EndFunction();
    }
    code.CloseBrace();

    if (code.IsTrue(prop_focus))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetFocus(").EndFunction();
    }

    // REVIEW: [Randalphwa - 12-28-2022] The caller closes the brace -- but it makes more sense
    // for the callee to close it.

    // code.CloseBrace();

    return true;
}

bool StyledTextGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/stc/stc.h>", set_src, set_hdr);
    return true;
}

static const char* lst_margins[] = {

    "custom_width", "custom_type", "custom_colour", "custom_mask_folders", "custom_mouse_sensitive",

};

void StyledTextGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop)
{
    BaseGenerator::ChangeEnableState(prop_grid, changed_prop);
    auto changed_node = changed_prop->GetNode();

    if (changed_prop->isProp(prop_stc_wrap_mode))
    {
        bool is_wrapped = (changed_prop->as_string() != "no wrapping");

        if (auto pg_wrap_setting = prop_grid->GetProperty("wrap_visual_flag"); pg_wrap_setting)
        {
            pg_wrap_setting->Enable(is_wrapped);
        }
        if (auto pg_wrap_setting = prop_grid->GetProperty("wrap_indent_mode"); pg_wrap_setting)
        {
            pg_wrap_setting->Enable(is_wrapped);
        }
        if (auto pg_wrap_setting = prop_grid->GetProperty("wrap_visual_location"); pg_wrap_setting)
        {
            pg_wrap_setting->Enable(is_wrapped);
        }
        if (auto pg_wrap_setting = prop_grid->GetProperty("wrap_start_indent"); pg_wrap_setting)
        {
            if (is_wrapped)
            {
                pg_wrap_setting->Enable(changed_node->prop_as_string(prop_stc_wrap_indent_mode) == "fixed");
            }
            else
            {
                pg_wrap_setting->Enable(false);
            }
        }
    }
    else if (changed_prop->isProp(prop_stc_wrap_indent_mode))
    {
        bool is_wrapped = (changed_node->prop_as_string(prop_stc_wrap_mode) != "no wrapping");
        if (auto pg_wrap_setting = prop_grid->GetProperty("wrap_start_indent"); pg_wrap_setting)
        {
            if (is_wrapped)
            {
                pg_wrap_setting->Enable(changed_prop->as_string() == "fixed");
            }
            else
            {
                pg_wrap_setting->Enable(false);
            }
        }
    }
    else if (changed_prop->isProp(prop_multiple_selections))
    {
        bool is_multiple = changed_prop->as_bool();
        if (auto pg_property = prop_grid->GetProperty("multiple_selection_typing"); pg_property)
        {
            pg_property->Enable(is_multiple);
        }
        if (auto pg_property = prop_grid->GetProperty("additional_carets_visible"); pg_property)
        {
            pg_property->Enable(is_multiple);
        }
        if (auto pg_property = prop_grid->GetProperty("additional_carets_blink"); pg_property)
        {
            pg_property->Enable(is_multiple);
        }
        if (auto pg_property = prop_grid->GetProperty("paste_multiple"); pg_property)
        {
            pg_property->Enable(is_multiple);
        }
    }
    else if (changed_prop->isProp(prop_additional_carets_visible))
    {
        bool is_multiple = changed_node->prop_as_bool(prop_multiple_selections);
        if (is_multiple)
        {
            if (auto pg_property = prop_grid->GetProperty("additional_carets_blink"); pg_property)
            {
                pg_property->Enable(changed_prop->as_bool());
            }
        }
    }
    else if (changed_prop->isProp(prop_fold_margin))
    {
        if (auto pg_property = prop_grid->GetProperty("automatic_folding"); pg_property)
        {
            pg_property->Enable(changed_prop->as_string() != "none");
        }
        if (auto pg_property = prop_grid->GetProperty("fold_width"); pg_property)
        {
            pg_property->Enable(changed_prop->as_string() != "none");
        }
        if (auto pg_property = prop_grid->GetProperty("fold_flags"); pg_property)
        {
            pg_property->Enable(changed_prop->as_string() != "none");
        }
    }
    else if (changed_prop->isProp(prop_line_margin))
    {
        if (auto pg_margin_setting = prop_grid->GetProperty("line_digits"); pg_margin_setting)
        {
            pg_margin_setting->Enable((changed_prop->as_string() != "none"));
        }
    }
    else if (changed_prop->isProp(prop_symbol_margin))
    {
        if (auto pg_margin_setting = prop_grid->GetProperty("symbol_mouse_sensitive"); pg_margin_setting)
        {
            pg_margin_setting->Enable((changed_prop->as_string() != "none"));
        }
    }
    else if (changed_prop->isProp(prop_separator_margin))
    {
        if (auto pg_margin_setting = prop_grid->GetProperty("separator_width"); pg_margin_setting)
        {
            pg_margin_setting->Enable((changed_prop->as_string() != "none"));
        }
    }
    else if (changed_prop->isProp(prop_custom_margin))
    {
        bool is_enabled = (changed_prop->as_string() != "none");
        for (auto& iter: lst_margins)
        {
            if (auto pg_margin_setting = prop_grid->GetProperty(iter); pg_margin_setting)
            {
                pg_margin_setting->Enable(is_enabled);
            }
        }

        // Hack alert! To prevent duplicating the code below, we change the changed_prop pointer.
        if (is_enabled)
        {
            changed_prop = changed_node->get_prop_ptr(prop_custom_type);
        }
    }

    if (changed_prop->isProp(prop_custom_type))
    {
        bool is_enabled = (changed_node->prop_as_string(prop_custom_margin) != "none");
        if (auto pg_margin_setting = prop_grid->GetProperty("custom_colour"); pg_margin_setting)
        {
            if (changed_prop->as_string() != "colour")
            {
                pg_margin_setting->Enable(false);
            }
            else
            {
                pg_margin_setting->Enable(is_enabled);
            }
        }
        if (auto pg_margin_setting = prop_grid->GetProperty("custom_mask_folders"); pg_margin_setting)
        {
            if (changed_prop->as_string() != "symbol" && changed_prop->as_string() != "number")
            {
                pg_margin_setting->Enable(false);
            }
            else
            {
                pg_margin_setting->Enable(is_enabled);
            }
        }
    }
}

// ../../wxSnapShot/src/xrc/xh_styledtextctrl.cpp
// ../../../wxWidgets/src/xrc/xh_styledtextctrl.cpp

int StyledTextGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStyledTextCtrl");

    if (node->prop_as_string(prop_stc_wrap_mode) != "no wrapping")
    {
        item.append_child("wrapmode").text().set(node->prop_as_constant(prop_stc_wrap_mode, "stc_"));
    }

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        ADD_ITEM_COMMENT(" The only property supported by XRC is wrap_mode. ")
        GenXrcComments(node, item);
    }

    return result;
}

void StyledTextGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStyledTextCtrlXmlHandler");
}
