/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains function for initializing wxStyledTextCtrl colors
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>

#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "preferences.h"   // Prefs -- Set/Get wxUiEditor preferences
#include "utils.h"         // WXDLLIMPEXP_BASE -- Miscellaneous utilities

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

// clang-format off

// These are base wxWidgets classes that may be in generated code, or in member variables
// a user adds.
inline const char* lst_widgets_keywords[] = {

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
    "wxGenericCalendarCtrl",
    "wxGenericHyperlinkCtrl",
    "wxGenericStaticBitmap",
    "wxGenericStaticText",
    "wxGenericTreeCtrl",
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

void SetStcColors(wxStyledTextCtrl* stc, GenLang language, bool set_lexer, bool add_keywords)
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

    if (language == GEN_LANG_CPLUSPLUS)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_CPP);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF16 characters to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

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
            stc->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        }
        stc->StyleSetBold(wxSTC_C_WORD, true);

        // First set all possible foreground/background colours
        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }
        }

        if (UserPrefs.is_DarkMode())
        {
            if (UserPrefs.is_HighContrast())
            {
                stc->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour("#569CD6"));
            }
            else
            {
                stc->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
            }
        }
        else
        {
            stc->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
        }

        stc->StyleSetForeground(wxSTC_C_STRING, UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_CppStringColour());
        stc->StyleSetForeground(wxSTC_C_STRINGEOL,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_CppStringColour());
        stc->StyleSetForeground(wxSTC_C_COMMENT,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_COMMENTLINE,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_COMMENTDOC,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_COMMENTLINEDOC,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_WORD, UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_CppKeywordColour());
        stc->StyleSetForeground(wxSTC_C_WORD2, UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_CppColour());
        stc->StyleSetForeground(wxSTC_C_NUMBER, UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_CppNumberColour());
    }
    else if (language == GEN_LANG_PERL)
    {
        stc->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        stc->SetMarginWidth(0, stc->TextWidth(wxSTC_STYLE_LINENUMBER, "_999"));

        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_PERL);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            // stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_perl_keywords);

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

            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxPerl_keywords.c_str());
        }

        stc->StyleSetBold(wxSTC_PL_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }
        }

        stc->StyleSetForeground(wxSTC_PL_COMMENTLINE,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_PerlCommentColour());
        stc->StyleSetForeground(wxSTC_PL_NUMBER,
                                UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_PerlNumberColour());
        stc->StyleSetForeground(wxSTC_PL_STRING,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_PerlStringColour());
        stc->StyleSetForeground(wxSTC_PL_WORD, UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_PerlColour());
        stc->StyleSetForeground(wxSTC_PL_PREPROCESSOR,
                                UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_PerlColour());
    }
    else if (language == GEN_LANG_PYTHON)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_PYTHON);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_python_keywords);

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
            stc->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) wxPython_keywords.c_str());
        }

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }

            double hue, saturation, luminance;
            wxColourToHSL(wxColour(0, 128, 0), hue, saturation, luminance);
            luminance = .80;
            auto light_green = HSLToWxColour(hue, saturation, luminance);

            stc->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour("#85e085"));
        }
        else
        {
            stc->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour(0, 128, 0));
        }
        stc->StyleSetForeground(wxSTC_P_COMMENTLINE,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_PythonCommentColour());
        stc->StyleSetForeground(wxSTC_P_NUMBER,
                                UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_PythonNumberColour());
        stc->StyleSetForeground(wxSTC_P_STRING,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_PythonStringColour());
        stc->StyleSetForeground(wxSTC_P_WORD, UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_PythonColour());
        stc->StyleSetForeground(wxSTC_P_WORD2,
                                UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_PythonKeywordColour());
    }

    else if (language == GEN_LANG_RUBY)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_RUBY);
        }
        if (add_keywords)
        {
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
                         iter->declName().starts_with("wxRuby"))
                    continue;
                wxRuby_keywords << ' ' << iter->declName().subview(2);
            }

            // Unfortunately, RUBY_LEXER only supports one set of keywords so we have to combine the
            // regular keywords with the wxWidgets keywords.

            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxRuby_keywords.c_str());
        }

        stc->StyleSetBold(wxSTC_RB_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }
        }
        stc->StyleSetForeground(wxSTC_RB_WORD, UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_RubyColour());
        stc->StyleSetForeground(wxSTC_RB_STRING,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_Q,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QQ,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QX,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QR,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QW,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_COMMENTLINE,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_RubyCommentColour());
        stc->StyleSetForeground(wxSTC_RB_NUMBER,
                                UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_RubyNumberColour());
        stc->StyleSetForeground(wxSTC_RB_SYMBOL, clr_functions);
        // stc->StyleSetForeground(wxSTC_RB_INSTANCE_VAR, clr_variables);
        stc->StyleSetForeground(wxSTC_RB_INSTANCE_VAR, clr_keywords);
        stc->StyleSetForeground(wxSTC_RB_CLASS_VAR, clr_keywords);
    }
    else if (language == GEN_LANG_RUST)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_RUST);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_rust_keywords);

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
            stc->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        }

        stc->StyleSetBold(wxSTC_RUST_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }
        }

        stc->StyleSetForeground(wxSTC_RUST_COMMENTLINE,
                                UserPrefs.is_HighContrast() ? clr_comments : UserPrefs.get_RustCommentColour());
        stc->StyleSetForeground(wxSTC_RUST_NUMBER,
                                UserPrefs.is_HighContrast() ? clr_numbers : UserPrefs.get_RustNumberColour());
        stc->StyleSetForeground(wxSTC_RUST_STRING,
                                UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_RustStringColour());
        stc->StyleSetForeground(wxSTC_RUST_WORD, UserPrefs.is_HighContrast() ? clr_keywords : UserPrefs.get_RustColour());
        stc->StyleSetForeground(wxSTC_RUST_WORD2,
                                UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_RustKeywordColour());
    }
    else if (language == GEN_LANG_XRC)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_XML);
        }
        // [Randalphwa - 03-04-2024] Default tab width for wxSTC_LEX_XML appears to be 8 at
        // least in wxWidgets 3.3, but we want 4 for XRC to improve readability.
        stc->SetTabWidth(4);
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);
        }
        stc->StyleSetBold(wxSTC_H_TAG, true);
        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }

            stc->StyleSetForeground(wxSTC_H_COMMENT, UserPrefs.is_HighContrast() ? clr_comments : wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_NUMBER, UserPrefs.is_HighContrast() ? clr_numbers : wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_ENTITY, UserPrefs.is_HighContrast() ? clr_types : wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_SINGLESTRING, UserPrefs.is_HighContrast() ? clr_strings : wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_ATTRIBUTE,
                                    UserPrefs.is_HighContrast() ? clr_variables : UserPrefs.get_XrcAttributeColour());
            stc->StyleSetForeground(wxSTC_H_DOUBLESTRING,
                                    UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_XrcDblStringColour());
            stc->StyleSetForeground(wxSTC_H_TAG, UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_XrcTagColour());
        }
        else
        {
            stc->StyleSetForeground(wxSTC_H_COMMENT, clr_comments);
            stc->StyleSetForeground(wxSTC_H_NUMBER, clr_numbers);
            stc->StyleSetForeground(wxSTC_H_ENTITY, clr_types);
            stc->StyleSetForeground(wxSTC_H_SINGLESTRING, clr_strings);
            stc->StyleSetForeground(wxSTC_H_ATTRIBUTE, UserPrefs.get_XrcAttributeColour());
            stc->StyleSetForeground(wxSTC_H_DOUBLESTRING, UserPrefs.get_XrcDblStringColour());
            stc->StyleSetForeground(wxSTC_H_TAG, UserPrefs.get_XrcTagColour());
        }
    }

#if GENERATE_NEW_LANG_CODE
    else if (language == GEN_LANG_FORTRAN)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_FORTRAN);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_fortran_keywords);
        }
        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }
        }

        stc->StyleSetForeground(wxSTC_F_COMMENT, UserPrefs.get_FortranCommentColour());
        stc->StyleSetForeground(wxSTC_F_NUMBER, UserPrefs.get_FortranNumberColour());
        stc->StyleSetForeground(wxSTC_F_STRING1, UserPrefs.get_FortranStringColour());
        stc->StyleSetForeground(wxSTC_F_WORD, UserPrefs.get_FortranColour());
        stc->StyleSetForeground(wxSTC_F_WORD2, UserPrefs.get_FortranKeywordColour());
    }
    else if (language == GEN_LANG_HASKELL)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_HASKELL);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_haskell_keywords);
        }

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }
        }

        stc->StyleSetForeground(wxSTC_HA_COMMENTLINE, UserPrefs.get_HaskellCommentColour());
        stc->StyleSetForeground(wxSTC_HA_NUMBER, UserPrefs.get_HaskellNumberColour());
        stc->StyleSetForeground(wxSTC_HA_STRING, UserPrefs.get_HaskellStringColour());
        stc->StyleSetForeground(wxSTC_HA_KEYWORD, UserPrefs.get_HaskellColour());
    }
    else if (language == GEN_LANG_LUA)
    {
        stc->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        stc->SetMarginWidth(0, stc->TextWidth(wxSTC_STYLE_LINENUMBER, "_999"));

        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_LUA);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_lua_keywords);
        }
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
        stc->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) wxLua_keywords.c_str());

        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }
        }

        stc->StyleSetForeground(wxSTC_LUA_COMMENT, UserPrefs.get_LuaCommentColour());
        stc->StyleSetForeground(wxSTC_LUA_NUMBER, UserPrefs.get_LuaNumberColour());
        stc->StyleSetForeground(wxSTC_LUA_STRING, UserPrefs.get_LuaStringColour());
        stc->StyleSetForeground(wxSTC_LUA_WORD, UserPrefs.get_LuaColour());
        stc->StyleSetForeground(wxSTC_LUA_WORD2, UserPrefs.get_LuaKeywordColour());
    }
#endif  // GENERATE_NEW_LANG_CODE

    else  // Unknown language, default to xml
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_XML);
        }
        // [Randalphwa - 03-04-2024] Default tab width for wxSTC_LEX_XML appears to be 8 at
        // least in wxWidgets 3.3, but we want 4 for XRC to improve readability.
        stc->SetTabWidth(4);
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);
        }
        stc->StyleSetBold(wxSTC_H_TAG, true);
        if (UserPrefs.is_DarkMode())
        {
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                stc->StyleSetForeground(idx, fg);
                stc->StyleSetBackground(idx, bg);
            }

            stc->StyleSetForeground(wxSTC_H_COMMENT, UserPrefs.is_HighContrast() ? clr_comments : wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_NUMBER, UserPrefs.is_HighContrast() ? clr_numbers : wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_ENTITY, UserPrefs.is_HighContrast() ? clr_types : wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_SINGLESTRING, UserPrefs.is_HighContrast() ? clr_strings : wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_ATTRIBUTE,
                                    UserPrefs.is_HighContrast() ? clr_variables : UserPrefs.get_XrcAttributeColour());
            stc->StyleSetForeground(wxSTC_H_DOUBLESTRING,
                                    UserPrefs.is_HighContrast() ? clr_strings : UserPrefs.get_XrcDblStringColour());
            stc->StyleSetForeground(wxSTC_H_TAG, UserPrefs.is_HighContrast() ? clr_functions : UserPrefs.get_XrcTagColour());
        }
    }
}
