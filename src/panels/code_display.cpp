/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/msgdlg.h>       // common header and base class for wxMessageDialog

#include "code_display.h"  // auto-generated: wxui/codedisplay_base.h and wxui/codedisplay_base.cpp

#include "base_panel.h"      // BasePanel -- Code generation panel
#include "code.h"            // Code -- Helper class for generating code
#include "font_prop.h"       // FontProp -- Font properties
#include "image_handler.h"   // ImageHandler class
#include "mainframe.h"       // MainFrame -- Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator -- Class used to create nodes
#include "node_event.h"      // NodeEvent and NodeEventInfo classes
#include "preferences.h"     // Prefs -- Set/Get wxUiEditor preferences
#include "propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events
#include "to_casts.h"        // to_int, to_size_t, and to_char classes
#include "utils.h"           // Miscellaneous utility functions

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

// For the actual Scintilla constants, see the following file.
// ../../wxWidgets/src/stc/lexilla/include/SciLexer.h

extern const char* g_u8_cpp_keywords;
extern const char* g_python_keywords;
extern const char* g_ruby_keywords;
extern const char* g_fortran_keywords;
extern const char* g_haskell_keywords;
extern const char* g_lua_keywords;
extern const char* g_perl_keywords;
extern const char* g_rust_keywords;

// XRC Keywords are defined in gen_xrc_utils.cpp so they can easily be updated as XRC
// generators support more XRC controls.
extern const char* g_xrc_keywords;

const int node_marker = 1;

// clang-format off

// These are base wxWidgets classes that may be in generated code, or in member variables
// a user adds.
static const char* lst_widgets_keywords[] = {

    "wxArrayInt",
    "wxAuiToolBarItem",
    "wxBitmap",
    "wxBitmapBundle",
    "wxBitmapButton",
    "wxColour",
    "wxDocument",
    "wxFileHistory",
    "wxFont",
    "wxGenericAnimationCtrl",
    "wxIcon",
    "wxImage",
    "wxListItem",
    "wxMemoryInputStream",
    "wxMenuBar",
    "wxObject",
    "wxPoint",
    "wxSize",
    "wxSizerFlags",
    "wxString",
    "wxToolBar",
    "wxToolBarToolBase",
    "wxVector",
    "wxWindow",
    "wxZlibInputStream",

};
// clang-format on

CodeDisplay::CodeDisplay(wxWindow* parent, GenLang panel_type) : CodeDisplayBase(parent), m_panel_type(panel_type)
{
    wxColor clr_comments(0, 128, 0);
    wxColor clr_functions("#c76605");
    wxColor clr_numbers("#fa5c5c");
    wxColor clr_keywords("#0000FF");
    wxColor clr_strings("#008000");
    wxColor clr_types("#f59afd");
    wxColor clr_variables("#94e6fa");

    // These will adjust for both dark mode and high contrast mode if needed
    auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
    auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);

    if (UserPrefs.is_DarkMode() && UserPrefs.is_HighContrast())
    {
        clr_comments = "#1cc462";
        clr_functions = "#faa75a";
        clr_numbers = "#FF0000";
        clr_keywords = "#569CD6";
        clr_strings = "#f1db0c";
        clr_types = "#f59afd";
        clr_variables = "#94e6fa";
    }

    if (panel_type == GEN_LANG_CPLUSPLUS)
    {
        m_scintilla->SetLexer(wxSTC_LEX_CPP);
        // On Windows, this saves converting the UTF16 characters to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords;
        for (auto iter: lst_widgets_keywords)
        {
            if (widget_keywords.size())
                widget_keywords << ' ' << iter;
            else
                widget_keywords = iter;
        }

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->declName();
        }
        widget_keywords << " wxAuiToolBarItem wxToolBarToolBase";

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_C_WORD, true);

        // First set all possible foreground/background colours
        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }

        if (UserPrefs.is_DarkMode())
        {
            if (UserPrefs.is_HighContrast())
            {
                m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour("#569CD6"));
            }
            else
            {
                m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
            }
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
        }

        m_scintilla->StyleSetForeground(wxSTC_C_STRING,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_CppStringColour());
        m_scintilla->StyleSetForeground(wxSTC_C_STRINGEOL,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_CppStringColour());
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENT,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENTDOC,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINEDOC,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_C_WORD,
                                        UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_CppKeywordColour());
        m_scintilla->StyleSetForeground(wxSTC_C_WORD2,
                                        UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_CppColour());
        m_scintilla->StyleSetForeground(wxSTC_C_NUMBER,
                                        UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_CppNumberColour());
    }
    else if (panel_type == GEN_LANG_PERL)
    {
        m_scintilla->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        m_scintilla->SetMarginWidth(0, m_scintilla->TextWidth(wxSTC_STYLE_LINENUMBER, "_999"));

        m_scintilla->SetLexer(wxSTC_LEX_PERL);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        // m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_perl_keywords);

        tt_string wxPerl_keywords(g_perl_keywords);
        for (auto iter: lst_widgets_keywords)
        {
            if (wxPerl_keywords.size())
                wxPerl_keywords << ' ' << iter;
            else
                wxPerl_keywords = iter;
        }

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx"))
                continue;
            else if (iter->declName().is_sameas("wxContextMenuEvent") || iter->declName() == "wxTreeCtrlBase" ||
                     iter->declName().starts_with("wxRuby") || iter->declName().starts_with("wxPython"))
                continue;
            wxPerl_keywords << ' ' << iter->declName().subview(2);
        }

        // Unfortunately, PERL_LEXER only supports one set of keywords so we have to combine the
        // regular keywords with the wxWidgets keywords.

        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxPerl_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_PL_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }

        m_scintilla->StyleSetForeground(wxSTC_PL_COMMENTLINE,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_PerlCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_PL_NUMBER,
                                        UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_PerlNumberColour());
        m_scintilla->StyleSetForeground(wxSTC_PL_STRING,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_PerlStringColour());
        m_scintilla->StyleSetForeground(wxSTC_PL_WORD,
                                        UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_PerlColour());
        m_scintilla->StyleSetForeground(wxSTC_PL_PREPROCESSOR,
                                        UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_PerlColour());
    }
    else if (panel_type == GEN_LANG_PYTHON)
    {
        m_scintilla->SetLexer(wxSTC_LEX_PYTHON);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_python_keywords);

        tt_string wxPython_keywords;
        for (auto* iter: lst_widgets_keywords)
        {
            if (wxPython_keywords.size())
                wxPython_keywords << ' ' << (iter + 2);
            else
                wxPython_keywords = (iter + 2);
        }

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            // wxPython_keywords << " wx." << iter->declName().subview(2);
            wxPython_keywords << ' ' << iter->declName().subview(2);
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) wxPython_keywords.c_str());

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            double hue, saturation, luminance;
            wxColourToHSL(wxColour(0, 128, 0), hue, saturation, luminance);
            luminance = .80;
            auto light_green = HSLToWxColour(hue, saturation, luminance);

            m_scintilla->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour("#85e085"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour(0, 128, 0));
        }
        m_scintilla->StyleSetForeground(wxSTC_P_COMMENTLINE,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_PythonCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_P_NUMBER,
                                        UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_PythonNumberColour());
        m_scintilla->StyleSetForeground(wxSTC_P_STRING,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_PythonStringColour());
        m_scintilla->StyleSetForeground(wxSTC_P_WORD,
                                        UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_PythonColour());
        m_scintilla->StyleSetForeground(wxSTC_P_WORD2,
                                        UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_PythonKeywordColour());
    }

    else if (panel_type == GEN_LANG_RUBY)
    {
        m_scintilla->SetLexer(wxSTC_LEX_RUBY);

        // We don't set ruby keywords because we can't colorize them differently from the
        // wxWidgets keywords.

        tt_string wxRuby_keywords("ALL LEFT RIGHT TOP BOTTOM DEFAULT_POSITION DEFAULT_SIZE HORIZONTAL VERTICAL "
                                  "ID_ANY ID_OK ID_CANCEL ID_SAVE ID_YES ID_NO "
                                  "TAB_TRAVERSAL FILTER_DIGITS Wx");
        for (auto& iter: lst_widgets_keywords)
        {
            wxRuby_keywords << ' ' << (iter + 2);
        }

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx"))
                continue;
            else if (iter->declName().is_sameas("wxContextMenuEvent") || iter->declName() == "wxTreeCtrlBase" ||
                     iter->declName().starts_with("wxRuby") || iter->declName().starts_with("wxPython"))
                continue;
            wxRuby_keywords << ' ' << iter->declName().subview(2);
        }

        // Unfortunately, RUBY_LEXER only supports one set of keywords so we have to combine the
        // regular keywords with the wxWidgets keywords.

        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxRuby_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_RB_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }
        m_scintilla->StyleSetForeground(wxSTC_RB_WORD,
                                        UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_RubyColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_STRING,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_STRING_Q,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_STRING_QQ,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_STRING_QX,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_STRING_QR,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_STRING_QW,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_COMMENTLINE,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_RubyCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_NUMBER,
                                        UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_RubyNumberColour());
        m_scintilla->StyleSetForeground(wxSTC_RB_SYMBOL, clr_functions);
        m_scintilla->StyleSetForeground(wxSTC_RB_INSTANCE_VAR, clr_variables);
        m_scintilla->StyleSetForeground(wxSTC_RB_CLASS_VAR, clr_keywords);
    }
    else if (panel_type == GEN_LANG_RUST)
    {
        m_scintilla->SetLexer(wxSTC_LEX_RUST);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_rust_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords;
        for (auto iter: lst_widgets_keywords)
        {
            if (widget_keywords.size())
                widget_keywords << ' ' << iter;
            else
                widget_keywords = iter;
        }

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->declName().view_substr(2);
        }
        widget_keywords << " wxAuiToolBarItem wxToolBarToolBase";

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_RUST_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }

        m_scintilla->StyleSetForeground(wxSTC_RUST_COMMENTLINE,
                                        UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_RustCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_RUST_NUMBER,
                                        UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_RustNumberColour());
        m_scintilla->StyleSetForeground(wxSTC_RUST_STRING,
                                        UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RustStringColour());
        m_scintilla->StyleSetForeground(wxSTC_RUST_WORD,
                                        UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_RustColour());
        m_scintilla->StyleSetForeground(wxSTC_RUST_WORD2,
                                        UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_RustKeywordColour());
    }
    else if (panel_type == GEN_LANG_XRC)
    {
        m_scintilla->SetLexer(wxSTC_LEX_XML);
        // [Randalphwa - 03-04-2024] Default tab width for wxSTC_LEX_XML appears to be 8 at
        // least in wxWidgets 3.3, but we want 4 for XRC to improve readability.
        m_scintilla->SetTabWidth(4);

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

        m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_H_COMMENT,
                                            UserPrefs.is_HighContrast() ? clr_comments : wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, UserPrefs.is_HighContrast() ? clr_numbers : wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, UserPrefs.is_HighContrast() ? clr_types : wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING,
                                            UserPrefs.is_HighContrast() ? clr_strings : wxColour("#85e085"));
            m_scintilla->StyleSetForeground(
                wxSTC_H_ATTRIBUTE, UserPrefs.is_HighContrast() ? clr_variables : UserPrefs.get_XrcAttributeColour());
            m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING,
                                            UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_XrcDblStringColour());
            m_scintilla->StyleSetForeground(wxSTC_H_TAG,
                                            UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_XrcTagColour());
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, clr_comments);
            m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, clr_numbers);
            m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, clr_types);
            m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, clr_strings);
            m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, UserPrefs.get_XrcAttributeColour());
            m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, UserPrefs.get_XrcDblStringColour());
            m_scintilla->StyleSetForeground(wxSTC_H_TAG, UserPrefs.get_XrcTagColour());
        }
    }

#if GENERATE_NEW_LANG_CODE
    else if (panel_type == GEN_LANG_FORTRAN)
    {
        m_scintilla->SetLexer(wxSTC_LEX_FORTRAN);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_fortran_keywords);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }

        m_scintilla->StyleSetForeground(wxSTC_F_COMMENT, UserPrefs.get_FortranCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_F_NUMBER, UserPrefs.get_FortranNumberColour());
        m_scintilla->StyleSetForeground(wxSTC_F_STRING1, UserPrefs.get_FortranStringColour());
        m_scintilla->StyleSetForeground(wxSTC_F_WORD, UserPrefs.get_FortranColour());
        m_scintilla->StyleSetForeground(wxSTC_F_WORD2, UserPrefs.get_FortranKeywordColour());
    }
    else if (panel_type == GEN_LANG_HASKELL)
    {
        m_scintilla->SetLexer(wxSTC_LEX_HASKELL);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_haskell_keywords);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }

        m_scintilla->StyleSetForeground(wxSTC_HA_COMMENTLINE, UserPrefs.get_HaskellCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_HA_NUMBER, UserPrefs.get_HaskellNumberColour());
        m_scintilla->StyleSetForeground(wxSTC_HA_STRING, UserPrefs.get_HaskellStringColour());
        m_scintilla->StyleSetForeground(wxSTC_HA_KEYWORD, UserPrefs.get_HaskellColour());
    }
    else if (panel_type == GEN_LANG_LUA)
    {
        m_scintilla->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        m_scintilla->SetMarginWidth(0, m_scintilla->TextWidth(wxSTC_STYLE_LINENUMBER, "_999"));

        m_scintilla->SetLexer(wxSTC_LEX_LUA);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_lua_keywords);

        tt_string wxLua_keywords;
        for (auto iter: lst_widgets_keywords)
        {
            if (wxLua_keywords.size())
                wxLua_keywords << ' ' << (iter + 2);
            else
                wxLua_keywords = (iter + 2);
        }

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx"))
                continue;
            else if (iter->declName().is_sameas("wxContextMenuEvent") || iter->declName() == "wxTreeCtrlBase" ||
                     iter->declName().starts_with("wxRuby") || iter->declName().starts_with("wxPython"))
                continue;
            wxLua_keywords << ' ' << iter->declName().subview(2);
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) wxLua_keywords.c_str());

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }

        m_scintilla->StyleSetForeground(wxSTC_LUA_COMMENT, UserPrefs.get_LuaCommentColour());
        m_scintilla->StyleSetForeground(wxSTC_LUA_NUMBER, UserPrefs.get_LuaNumberColour());
        m_scintilla->StyleSetForeground(wxSTC_LUA_STRING, UserPrefs.get_LuaStringColour());
        m_scintilla->StyleSetForeground(wxSTC_LUA_WORD, UserPrefs.get_LuaColour());
        m_scintilla->StyleSetForeground(wxSTC_LUA_WORD2, UserPrefs.get_LuaKeywordColour());
    }
#endif  // GENERATE_NEW_LANG_CODE

    else  // Unknown language, default to xml
    {
        m_scintilla->SetLexer(wxSTC_LEX_XML);
        // [Randalphwa - 03-04-2024] Default tab width for wxSTC_LEX_XML appears to be 8 at
        // least in wxWidgets 3.3, but we want 4 for XRC to improve readability.
        m_scintilla->SetTabWidth(4);

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

        m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_H_COMMENT,
                                            UserPrefs.is_HighContrast() ? clr_comments : wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, UserPrefs.is_HighContrast() ? clr_numbers : wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, UserPrefs.is_HighContrast() ? clr_types : wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING,
                                            UserPrefs.is_HighContrast() ? clr_strings : wxColour("#85e085"));
            m_scintilla->StyleSetForeground(
                wxSTC_H_ATTRIBUTE, UserPrefs.is_HighContrast() ? clr_variables : UserPrefs.get_XrcAttributeColour());
            m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING,
                                            UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_XrcDblStringColour());
            m_scintilla->StyleSetForeground(wxSTC_H_TAG,
                                            UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_XrcTagColour());
        }

        // TODO: [KeyWorks - 01-02-2022] We do this because currently font selection uses a facename which is not
        // cross-platform. See issue #597.

        FontProperty font_prop(UserPrefs.get_CodeDisplayFont().ToStdView());
        m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font_prop.GetFont());

        // wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        // m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

        m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

        Bind(wxEVT_FIND, &CodeDisplay::OnFind, this);
        Bind(wxEVT_FIND_NEXT, &CodeDisplay::OnFind, this);
    }
}

void CodeDisplay::OnFind(wxFindDialogEvent& event)
{
    auto wxflags = event.GetFlags();
    int sciflags = 0;

    if (wxflags & wxFR_WHOLEWORD)
    {
        sciflags |= wxSTC_FIND_WHOLEWORD;
    }
    if (wxflags & wxFR_MATCHCASE)
    {
        sciflags |= wxSTC_FIND_MATCHCASE;
    }

    int result;
    if (wxflags & wxFR_DOWN)
    {
        m_scintilla->SetSelectionStart(m_scintilla->GetSelectionEnd());
        m_scintilla->SearchAnchor();
        result = m_scintilla->SearchNext(sciflags, event.GetFindString());
    }
    else
    {
        m_scintilla->SetSelectionEnd(m_scintilla->GetSelectionStart());
        m_scintilla->SearchAnchor();
        result = m_scintilla->SearchPrev(sciflags, event.GetFindString());
    }

    if (result == wxSTC_INVALID_POSITION)
    {
        wxMessageBox(wxString() << event.GetFindString() << " not found.", "Not Found", wxICON_ERROR,
                     wxStaticCast(event.GetClientData(), wxWindow));
    }
    else
    {
        m_scintilla->EnsureCaretVisible();
    }
}

void CodeDisplay::Clear()
{
    m_view.clear();
    m_view.GetBuffer().clear();

    m_scintilla->SetReadOnly(false);
    m_scintilla->ClearAll();
}

void CodeDisplay::doWrite(tt_string_view code)
{
    m_view.GetBuffer() << code;
}

void CodeDisplay::CodeGenerationComplete()
{
    m_scintilla->AddTextRaw(m_view.GetBuffer().data(), (to_int) m_view.GetBuffer().size());
    m_scintilla->SetReadOnly(true);

    // Find doesn't work correctly unless there's a selection to start the search from.
    m_scintilla->SetEmptySelection(0);

    m_view.ParseBuffer();
}

void CodeDisplay::OnNodeSelected(Node* node)
{
    if (node->isGen(gen_embedded_image))
    {
        OnEmbedImageSelected(node);
        return;
    }
    else if (node->isGen(gen_ribbonTool) || node->isGen(gen_ribbonButton))
    {
        OnRibbonToolSelected(node);
        return;
    }

    if (!node->hasProp(prop_var_name) && m_panel_type != GEN_LANG_XRC && !node->isGen(gen_ribbonTool) &&
        !node->isGen(gen_ribbonButton))
    {
        return;  // probably a form, spacer, or image
    }

    auto is_event = wxGetFrame().getPropPanel()->IsEventPageShowing();
    PANEL_PAGE page = wxGetFrame().GetCppPanel()->GetPanelPage();

    if (m_panel_type != GEN_LANG_CPLUSPLUS && page != CPP_PANEL)
        return;  // Nothing to search for in secondary pages of non-C++ languages

    int line = -1;

    tt_string name(" ");

    GenLang language = GEN_LANG_CPLUSPLUS;
    if (m_panel_type == GEN_LANG_PYTHON)
        language = GEN_LANG_PYTHON;
    else if (m_panel_type == GEN_LANG_RUBY)
        language = GEN_LANG_RUBY;

    Code code(node, language);

    if (page == CPP_PANEL)
    {
        code.NodeName();
        name << code.GetCode();
    }

    if (is_event)
    {
        if (page == CPP_PANEL)
        {
            name << "->Bind";
            line = (to_int) m_view.FindLineContaining(name);
            if (!tt::is_found(line))
            {
                name.Replace("->Bind", " = ");
                line = (to_int) m_view.FindLineContaining(name);
            }
        }
        else
        {
            auto map_events = node->getMapEvents();
            for (auto& iter: map_events)
            {
                auto value = iter.second.get_value();
                if (value.empty())
                    continue;

                line = (to_int) m_view.FindLineContaining(value);
                if (tt::is_found(line))
                    break;
            }
        }
    }
    else if (m_panel_type == GEN_LANG_XRC)
    {
        tt_string search("name=\"");
        if (node->hasProp(prop_id) && node->as_string(prop_id) != "wxID_ANY")
        {
            search << node->getPropId();
        }
        else if (node->hasValue(prop_var_name))
        {
            search << node->as_string(prop_var_name);
        }
        else
        {
            search << node->as_string(prop_class_name);
        }
        line = (to_int) m_view.FindLineContaining(search);
    }
    else
    {
        if (node->isGen(gen_tool) || node->isGen(gen_auitool) || node->isGen(gen_ribbonTool) ||
            node->isGen(gen_ribbonButton))
        {
            if (node->hasValue(prop_bitmap))
            {
                tt_view_vector parts(node->as_string(prop_bitmap), BMP_PROP_SEPARATOR, tt::TRIM::both);
                if (parts.size() && parts[IndexImage].size())
                {
                    if (auto result = FileNameToVarName(parts[IndexImage]); result)
                    {
                        code.clear();
                        code.Function(node->isGen(gen_ribbonButton) ? "AddButton" : "AddTool");
                        line = (to_int) m_view.FindLineContaining(code.GetCode());
                        if (tt::is_found(line))
                        {
                            line = (to_int) m_view.FindLineContaining(*result, line);
                        }
                    }
                }
            }

            if (!tt::is_found(line) && node->hasValue(prop_label))
            {
                code.clear();
                code.Function("AddTool");
                line = (to_int) m_view.FindLineContaining(code.GetCode());
                if (tt::is_found(line))
                {
                    line = (to_int) m_view.FindLineContaining(node->as_string(prop_label), line);
                }
            }
        }

        if (!tt::is_found(line))
        {
            if (page == CPP_PANEL)
            {
                name << " = ";
                line = (to_int) m_view.FindLineContaining(name);
            }
            else
            {
                name << node->as_string(prop_var_name) << ";";
                line = (to_int) m_view.FindLineContaining(name);
            }
        }
    }

    if (!tt::is_found(line))
        return;

    m_scintilla->MarkerDeleteAll(node_marker);
    m_scintilla->MarkerAdd(line, node_marker);

#if 0
    // REVIEW: [KeyWorks - 01-20-2022] This would be great if it worked, but GetLineVisible() is returning true even if
    // the line is not visible.
    if (!m_scintilla->GetLineVisible(line))
    {
        m_scintilla->ScrollToLine(line);
    }
#endif

    // Unlike GetLineVisible(), this function does ensure that the line is visible.
    m_scintilla->ScrollToLine(line);
}

void CodeDisplay::OnRibbonToolSelected(Node* node)
{
    tt_string search;
    if (auto parent = node->getParent(); parent)
    {
        if (parent->isGen(gen_wxRibbonButtonBar))
        {
            search << '"' << node->as_string(prop_label) << '"';
        }
        else if (parent->isGen(gen_wxRibbonToolBar))
        {
            search << parent->as_string(prop_var_name) << "->AddTool(" << node->as_string(prop_id) << ",";
            if (m_panel_type == GEN_LANG_PYTHON)
                search.Replace("->", ".");
            else if (m_panel_type == GEN_LANG_RUBY)
                search.Replace("->AddTool(", ".add_tool($");
        }
    }

    if (search.size())
    {
        if (auto line = (to_int) m_view.FindLineContaining(search); line >= 0)
        {
            m_scintilla->MarkerDeleteAll(node_marker);
            m_scintilla->MarkerAdd(line, node_marker);
            m_scintilla->ScrollToLine(line);
        }
        return;
    }
}

void CodeDisplay::OnEmbedImageSelected(Node* node)
{
    if (node->hasValue(prop_bitmap))
    {
        auto func_name = ProjectImages.GetBundleFuncName(node->as_string(prop_bitmap));
        if (func_name.size())
        {
            if (func_name.starts_with("wxue_img::"))
                func_name.erase(0, sizeof("wxue_img::") - 1);
            if (auto pos = func_name.find("("); pos != tt::npos)
                func_name.erase(pos, tt::npos);

            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }

            // For icons, there is no bundle, just an image_ function
            func_name.Replace("bundle_", "image_");
            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }

            // If all else fails, try just the name. This will also handle Python and Ruby panels
            func_name.Replace("image_", "");
            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }
        }
    }
}

void CodeDisplay::SetColor(int style, const wxColour& color)
{
    m_scintilla->StyleSetForeground(style, color);
}

void CodeDisplay::SetCodeFont(const wxFont& font)
{
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
}
