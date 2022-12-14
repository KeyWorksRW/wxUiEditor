/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStyledText (scintilla) generate
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
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

    ttlib::cstr sample(txt_styled_sample);
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

std::optional<ttlib::cstr> StyledTextGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> StyledTextGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // There are potentially a LOT of settings, so we put them all in a bracket pair to make them easier to identifiy

    // auto_indent = false;
    code << "{";

    if (node->HasValue(prop_stc_lexer) && node->prop_as_string(prop_stc_lexer) != "NULL")
    {
        ttlib::cstr name("wxSTC_LEX_");
        name << node->prop_as_string(prop_stc_lexer);
        code << "\n\t" << node->get_node_name() << "->SetLexer(" << name << ");";
    }

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_read_only))
    {
        code << "\n\t" << node->get_node_name() << "->SetReadOnly(true);";
    }

    if (node->HasValue(prop_eol_mode))
    {
        code << "\n\t" << node->get_node_name() << "->SetEOLMode(" << node->prop_as_constant(prop_eol_mode, "stc_") << ");";
    }

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_view_eol))
    {
        code << "\n\t" << node->get_node_name() << "->SetViewEOL(true);";
    }

    // Default is false, so only set if true
    if (!node->isPropValue(prop_view_whitespace, "invisible"))
    {
        code << "\n\t" << node->get_node_name() << "->SetViewWhiteSpace("
             << node->prop_as_constant(prop_view_whitespace, "stc_") << ");";
        if (node->prop_as_bool(prop_view_tab_strikeout))
        {
            code << "\n\t" << node->get_node_name() << "->SetTabDrawMode(wxSTC_TD_STRIKEOUT);";
        }
    }

    //////////// Wrap category settings ////////////

    if (!node->prop_as_string(prop_stc_wrap_mode).is_sameas("no wrapping"))
    {
        code << "\n\t" << node->get_node_name() << "->SetWrapMode(" << node->prop_as_constant(prop_stc_wrap_mode, "stc_")
             << ");";
    }
    if (node->HasValue(prop_stc_wrap_visual_flag))
    {
        if (auto result = node->prop_as_constant(prop_stc_wrap_visual_flag, "stc_"); result.size())
        {
            code << "\n\t" << node->get_node_name() << "->SetWrapVisualFlags(" << result << ");";
        }
    }
    if (node->HasValue(prop_stc_wrap_visual_location))
    {
        if (auto result = node->prop_as_constant(prop_stc_wrap_visual_location, "stc_"); result.size())
        {
            code << "\n\t" << node->get_node_name() << "->SetWrapVisualFlagsLocation(" << result << ");";
        }
    }
    if (!node->prop_as_string(prop_stc_wrap_indent_mode).is_sameas("fixed"))
    {
        code << "\n\t" << node->get_node_name() << "->SetWrapIndentMode("
             << node->prop_as_constant(prop_stc_wrap_indent_mode, "stc_") << ");";
    }
    if (node->HasValue(prop_stc_wrap_start_indent))
    {
        code << "\n\t" << node->get_node_name() << "->SetWrapStartIndent(" << node->prop_as_int(prop_stc_wrap_start_indent)
             << ");";
    }

    //////////// Selection category settings ////////////

    if (node->prop_as_bool(prop_multiple_selections))
    {
        code << "\n\t" << node->get_node_name() << "->SetMultipleSelection(wxSTC_MULTIPASTE_EACH);";
        if (node->prop_as_bool(prop_paste_multiple))
        {
            code << "\n\t" << node->get_node_name() << "->SetMultiPaste(wxSTC_MULTIPASTE_EACH);";
        }
        if (node->prop_as_bool(prop_paste_multiple))
        {
            code << "\n\t" << node->get_node_name() << "->SetMultiPaste(wxSTC_MULTIPASTE_EACH);";
        }
        code << "\n\t" << node->get_node_name() << "->SetAdditionalSelectionTyping("
             << (node->prop_as_bool(prop_additional_carets_blink) ? "true" : "false") << ");";

        if (!node->prop_as_bool(prop_additional_carets_visible))
        {
            code << "\n\t" << node->get_node_name() << "->SetAdditionalCaretsVisible(false);";
        }
        else
        {
            code << "\n\t" << node->get_node_name() << "->SetAdditionalCaretsBlink("
                 << (node->prop_as_bool(prop_additional_carets_blink) ? "true" : "false") << ");";
        }
    }

    if (node->prop_as_bool(prop_allow_mouse_rectangle))
    {
        code << "\n\t" << node->get_node_name() << "->SetMouseSelectionRectangularSwitch(true);";
    }

    //////////// Margin category settings ////////////

    // The default margin is 1, so if that's what it is set to, then don't output any code
    if (node->prop_as_int(prop_stc_left_margin_width) != 1)
    {
        if (node->prop_as_int(prop_stc_left_margin_width) == 5)
        {
            code << "\n\t// Sets text margin scaled appropriately for the current DPI on Windows,\n\t// 5 on wxGTK or "
                    "wxOSX";
        }
        code << "\n\t" << node->get_node_name() << "->SetMarginLeft(";
        if (node->prop_as_int(prop_stc_left_margin_width) == 5)
        {
            code << "wxSizerFlags::GetDefaultBorder());";
        }
        else
        {
            code << node->prop_as_int(prop_stc_left_margin_width) << ");";
        }
    }

    if (node->prop_as_int(prop_stc_right_margin_width) != 1)
    {
        if (node->prop_as_int(prop_stc_left_margin_width) != 5 && node->prop_as_int(prop_stc_right_margin_width) == 5)
        {
            code << "\n\t\t// Sets text margin scaled appropriately for the current DPI on Windows,\n\t\t// 5 on wxGTK or "
                    "wxOSX";
        }
        code << "\n\t" << node->get_node_name() << "->SetMarginRight(";
        if (node->prop_as_int(prop_stc_right_margin_width) == 5)
        {
            code << "wxSizerFlags::GetDefaultBorder());";
        }
        else
        {
            code << node->prop_as_int(prop_stc_right_margin_width) << ");";
        }
    }

    if (!node->prop_as_bool(prop_stc_select_wrapped_line))
    {
        code << "\n\t" << node->get_node_name() << "->SetMarginOptions(wxSTC_MARGINOPTION_SUBLINESELECT);";
    }

    // By default, scintilla sets margin one to a width to 16. We want to shut off all margins unless the user specifically
    // uses it.
    bool is_margin_1_set { false };

    if (node->prop_as_string(prop_fold_margin) == "1")
    {
        is_margin_1_set = true;
    }
    else if (node->prop_as_string(prop_line_margin) == "1")
    {
        is_margin_1_set = true;
    }
    else if (node->prop_as_string(prop_separator_margin) == "1")
    {
        is_margin_1_set = true;
    }
    else if (node->prop_as_string(prop_symbol_margin) == "1")
    {
        is_margin_1_set = true;
    }
    else if (node->prop_as_string(prop_custom_width) == "1")
    {
        is_margin_1_set = true;
    }

    if (!is_margin_1_set)
    {
        code << "\n\t" << node->get_node_name() << "->SetMarginWidth(1, 0);  // Remove default margin";
    }

    if (node->prop_as_string(prop_line_margin) != "none")
    {
        auto margin = node->prop_as_string(prop_line_margin).atoi();
        int width = node->prop_as_string(prop_line_digits).atoi();

        ttlib::cstr numbers("_");
        while (width > 0)
        {
            numbers << '9';
            --width;
        }

        code << "\n\t" << node->get_node_name() << "->SetMarginWidth(" << margin << ", ";
        code << node->get_node_name() << "->TextWidth(wxSTC_STYLE_LINENUMBER, " << '"' << numbers << "\"));";
        code << "\n\t" << node->get_node_name() << "->SetMarginType(" << margin << ", wxSTC_MARGIN_NUMBER);";
    }

    if (node->prop_as_string(prop_fold_margin) != "none" && node->prop_as_int(prop_fold_width))
    {
        auto margin = node->prop_as_string(prop_fold_margin).atoi();
        code << "\n\t" << node->get_node_name() << "->SetProperty(\"fold\", \"1\");";
        code << "\n\t" << node->get_node_name() << "->SetMarginWidth(" << margin << ", 16);";
        code << "\n\t" << node->get_node_name() << "->SetMarginType(" << margin << ", wxSTC_MARGIN_SYMBOL);";
        code << "\n\t" << node->get_node_name() << "->SetMarginMask(" << margin << ", wxSTC_MASK_FOLDERS);";
        code << "\n\t" << node->get_node_name() << "->SetMarginSensitive(" << margin << ", true);";
        if (node->HasValue(prop_automatic_folding))
        {
            code << "\n\t" << node->get_node_name() << "->SetAutomaticFold("
                 << node->prop_as_constant(prop_automatic_folding, "stc_") << ");";
        }
        if (node->HasValue(prop_fold_flags))
        {
            code << "\n\t" << node->get_node_name() << "->SetFoldFlags(" << node->prop_as_constant(prop_fold_flags, "stc_")
                 << ");";
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
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDER, " << symbol_folder
                     << ", wxNullColour, " << GenerateColourCode(node, prop_fold_marker_colour) << ";";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, " << symbol_open
                     << ", wxNullColour, " << GenerateColourCode(node, prop_fold_marker_colour) << ";";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, " << symbol_open
                     << ", wxNullColour, " << GenerateColourCode(node, prop_fold_marker_colour) << ";";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, " << symbol_folder
                     << ", wxNullColour, " << GenerateColourCode(node, prop_fold_marker_colour) << ";";
            }
            else
            {
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDER, " << symbol_folder << ");";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, " << symbol_open
                     << ");";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, " << symbol_open
                     << ");";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, " << symbol_folder
                     << ");";
            }
            code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_BACKGROUND);";
            code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_BACKGROUND);";
            code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_BACKGROUND);";
        }
        else  // circle tree or box tree
        {
            code << "\n\t\t{\n\t\t\t// The outline colour of the circle and box tree symbols is reversed by "
                    "default.\n\t\t\t// The code below ensures that the symbol is visible.";
            code << "\n\t\t\tauto clr_foreground = " << node->get_node_name()
                 << "->StyleGetForeground(wxSTC_STYLE_DEFAULT);";
            code << "\n\t\t\tauto clr_background = ";
            if (node->HasValue(prop_fold_marker_colour))
            {
                code << GenerateColourCode(node, prop_fold_marker_colour) << ";";
            }
            else
            {
                code << node->get_node_name() << "->StyleGetForeground(wxSTC_STYLE_DEFAULT);";
            }
            code << "\n\t\t\t" << node->get_node_name() << "->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, clr_foreground);";
            code << "\n\t\t\t" << node->get_node_name()
                 << "->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, clr_foreground);";
            code << "\n\t\t\t" << node->get_node_name()
                 << "->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, clr_foreground);";
            code << "\n\t\t\t" << node->get_node_name() << "->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, clr_foreground);";
            code << "\n\t\t\t" << node->get_node_name()
                 << "->MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, clr_foreground);";
            code << "\n\t\t\t" << node->get_node_name() << "->MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, clr_foreground);";
            code << "\n\t\t\t" << node->get_node_name()
                 << "->MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, clr_foreground);";
            code << "\n\t\t\t" << node->get_node_name() << "->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, clr_background);";
            code << "\n\t\t\t" << node->get_node_name()
                 << "->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, clr_background);";
            code << "\n\t\t\t" << node->get_node_name()
                 << "->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, clr_background);";
            code << "\n\t\t\t" << node->get_node_name() << "->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, clr_background);";
            code << "\n\t\t}";

            if (node->prop_as_string(prop_fold_marker_style) == "circle tree")
            {
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_CIRCLEPLUS);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_CIRCLEMINUS);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_CIRCLEMINUSCONNECTED);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_CIRCLEPLUSCONNECTED);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNERCURVE);";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNERCURVE);";
            }
            else
            {
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXMINUS);";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXPLUS);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUSCONNECTED);";
                code << "\n\t" << node->get_node_name()
                     << "->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER);";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);";
                code << "\n\t" << node->get_node_name() << "->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER);";
            }
        }
    }

    if (node->prop_as_string(prop_symbol_margin) != "none")
    {
        auto margin = node->prop_as_string(prop_symbol_margin).atoi();
        code << "\n\t" << node->get_node_name() << "->SetMarginWidth(" << margin << ", 16);";
        code << "\n\t" << node->get_node_name() << "->SetMarginType(" << margin << ", wxSTC_MARGIN_SYMBOL);";
        code << "\n\t" << node->get_node_name() << "->SetMarginMask(" << margin << ", ~wxSTC_MASK_FOLDERS);";
        code << "\n\t" << node->get_node_name() << "->SetMarginSensitive(" << margin << ", ";
        code << (node->prop_as_bool(prop_symbol_mouse_sensitive) ? "true" : "false") << ");";
    }

    if (node->prop_as_string(prop_separator_margin) != "none")
    {
        auto margin = node->prop_as_string(prop_separator_margin).atoi();
        code << "\n\t" << node->get_node_name() << "->SetMarginWidth(" << margin << ", "
             << node->prop_as_int(prop_separator_width) << ");";
        code << "\n\t" << node->get_node_name() << "->SetMarginType(" << margin << ", wxSTC_MARGIN_FORE);";
    }

    if (node->prop_as_string(prop_custom_margin) != "none" && node->prop_as_int(prop_custom_width))
    {
        auto margin = node->prop_as_string(prop_custom_margin).atoi();
        code << "\n\t" << node->get_node_name() << "->SetMarginWidth(" << margin << ", "
             << node->prop_as_int(prop_custom_width) << ");";

        code << "\n\t" << node->get_node_name() << "->SetMarginType(" << margin << ", "
             << node->prop_as_constant(prop_custom_type, "stc_") << ");";

        if (node->prop_as_string(prop_custom_type) == "colour" && node->HasValue(prop_custom_colour))
        {
            code << "\n\t" << node->get_node_name() << "->SetMarginBackground(" << margin << ", "
                 << GenerateColourCode(node, prop_custom_colour) << ");";
        }
        else
        {
            code << "\n\t" << node->get_node_name() << "->SetMarginMask(" << margin << ", "
                 << (node->prop_as_bool(prop_custom_mask_folders) ? "wxSTC_MASK_FOLDERS" : "~wxSTC_MASK_FOLDERS") << ");";
        }
        if (node->prop_as_bool(prop_custom_mouse_sensitive))
        {
            code << "\n\t" << node->get_node_name() << "->SetMarginSensitive(" << margin << ", true);";
        }
    }

    //////////// Tabs and Indentation settings ////////////

    if (node->HasValue(prop_indentation_guides) && !node->prop_as_string(prop_indentation_guides).is_sameas("no guides") &&
        // false was what was used in previous versions as well as in some imported values
        !node->prop_as_string(prop_indentation_guides).is_sameas("false"))
    {
        code << "\n\t" << node->get_node_name() << "->SetIndentationGuides("
             << node->prop_as_constant(prop_indentation_guides, "stc_") << ");";
    }
    if (node->prop_as_int(prop_stc_indentation_size) != 0)
    {
        code << "\n\t" << node->get_node_name() << "->SetIndent(" << node->prop_as_int(prop_stc_indentation_size) << ");";
    }

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_use_tabs))
    {
        code << "\n\t" << node->get_node_name() << "->SetUseTabs(false);";

        if (node->prop_as_int(prop_tab_width) != 8)
            code << "\n\t" << node->get_node_name() << "->SetTabWidth(" << node->prop_as_string(prop_tab_width) << ");";
    }

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_tab_indents))
        code << "\n\t" << node->get_node_name() << "->SetTabIndents(false);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_backspace_unindents))
        code << "\n\t" << node->get_node_name() << "->SetBackSpaceUnIndents(true);";
    code << "\n}";

    if (code.is_sameas("\t{\n\t}"))
    {
        code.clear();  // means there were no settings
    }

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    return code;
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
