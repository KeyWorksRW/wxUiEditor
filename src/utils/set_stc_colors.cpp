/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains function for initializing wxStyledTextCtrl colors
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>

#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "preferences.h"   // Prefs -- Set/Get wxUiEditor preferences
#include "utils.h"         // WXDLLIMPEXP_BASE -- Miscellaneous utilities

#include "wxue_namespace/wxue_string.h"  // wxue::string

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

// For the actual Scintilla constants, see the following file.
// ../../wxWidgets/src/stc/lexilla/include/SciLexer.h

extern const char* g_u8_cpp_keywords;
extern const char* g_python_keywords;
extern const char* g_ruby_keywords;
extern const char* g_fortran_keywords;
extern const char* g_go_keywords;
extern const char* g_julia_keywords;
extern const char* g_luajit_keywords;
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
    "wxAuiMDIParentFrame",
    "wxAuiToolBarItem",
    "wxBitmap",
    "wxBitmapBundle",
    "wxBitmapButton",
    "wxClassInfo",
    "wxColour",
    "wxDocChildFrameAny",
    "wxDocManager",
    "wxDocParentFrameAny",
    "wxDocTemplate",
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
    "wxWindowID",
    "wxZlibInputStream",

};
// clang-format on

// Colors used by the SetStcColors function and its helpers
struct StcColorTheme
{
    wxColor clr_comments;
    wxColor clr_functions;
    wxColor clr_numbers;
    wxColor clr_keywords;
    wxColor clr_strings;
    wxColor clr_types;
    wxColor clr_variables;
    wxColor foreground;
    wxColor background;
};

namespace
{

    void SetDarkModeBase(wxStyledTextCtrl* stc, const StcColorTheme& theme)
    {
        for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; ++idx)
        {
            stc->StyleSetForeground(idx, theme.foreground);
            stc->StyleSetBackground(idx, theme.background);
        }
    }

    void SetCppColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                      const StcColorTheme& theme)
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

            wxue::string widget_keywords;
            for (const auto* iter: lst_widgets_keywords)
            {
                if (widget_keywords.size())
                {
                    widget_keywords << ' ' << iter;
                }
                else
                {
                    widget_keywords = iter;
                }
            }

            for (auto* iter: NodeCreation.get_NodeDeclarationArray())
            {
                if (!iter)
                {
                    // This will happen if there is an enumerated value but no generator for it
                    continue;
                }

                if (!iter->get_DeclName().starts_with("wx") ||
                    iter->get_DeclName() == "wxContextMenuEvent")
                {
                    continue;
                }
                widget_keywords << ' ' << iter->get_DeclName();
            }
            widget_keywords << " wxAuiToolBarItem wxToolBarToolBase";

            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        }
        stc->StyleSetBold(wxSTC_C_WORD, true);

        // First set all possible foreground/background colours
        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
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

        stc->StyleSetForeground(wxSTC_C_STRING, UserPrefs.is_HighContrast() ?
                                                    theme.clr_strings :
                                                    UserPrefs.get_CppStringColour());
        stc->StyleSetForeground(wxSTC_C_STRINGEOL, UserPrefs.is_HighContrast() ?
                                                       theme.clr_strings :
                                                       UserPrefs.get_CppStringColour());
        stc->StyleSetForeground(wxSTC_C_COMMENT, UserPrefs.is_HighContrast() ?
                                                     theme.clr_comments :
                                                     UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_COMMENTLINE, UserPrefs.is_HighContrast() ?
                                                         theme.clr_comments :
                                                         UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_COMMENTDOC, UserPrefs.is_HighContrast() ?
                                                        theme.clr_comments :
                                                        UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, UserPrefs.is_HighContrast() ?
                                                            theme.clr_comments :
                                                            UserPrefs.get_CppCommentColour());
        stc->StyleSetForeground(wxSTC_C_WORD, UserPrefs.is_HighContrast() ?
                                                  theme.clr_keywords :
                                                  UserPrefs.get_CppKeywordColour());
        stc->StyleSetForeground(wxSTC_C_WORD2, UserPrefs.is_HighContrast() ?
                                                   theme.clr_functions :
                                                   UserPrefs.get_CppColour());
        stc->StyleSetForeground(wxSTC_C_NUMBER, UserPrefs.is_HighContrast() ?
                                                    theme.clr_numbers :
                                                    UserPrefs.get_CppNumberColour());
    }

    void SetPythonColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                         const StcColorTheme& theme)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_PYTHON);
        }
        if (add_keywords)
        {
            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_python_keywords);

            wxue::string wxPython_keywords;
            for (const auto* iter: lst_widgets_keywords)
            {
                if (wxPython_keywords.size())
                {
                    wxPython_keywords << ' ' << (iter + 2);
                }
                else
                {
                    wxPython_keywords = (iter + 2);
                }
            }

            for (auto* iter: NodeCreation.get_NodeDeclarationArray())
            {
                if (!iter)
                {
                    // This will happen if there is an enumerated value but no generator for it
                    continue;
                }

                if (!iter->get_DeclName().starts_with("wx") ||
                    iter->get_DeclName() == "wxContextMenuEvent")
                {
                    continue;
                }
                // wxPython_keywords << " wx." << iter->get_DeclName().subview(2);
                wxPython_keywords << ' ' << iter->get_DeclName().substr(2);
            }

            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            stc->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) wxPython_keywords.c_str());
        }

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);

            double hue, saturation, luminance;
            wxColourToHSL(wxColour(0, 128, 0), hue, saturation, luminance);
            luminance = .80;
            stc->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour("#85e085"));
        }
        else
        {
            stc->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour(0, 128, 0));
        }
        stc->StyleSetForeground(wxSTC_P_COMMENTLINE, UserPrefs.is_HighContrast() ?
                                                         theme.clr_comments :
                                                         UserPrefs.get_PythonCommentColour());
        stc->StyleSetForeground(wxSTC_P_NUMBER, UserPrefs.is_HighContrast() ?
                                                    theme.clr_numbers :
                                                    UserPrefs.get_PythonNumberColour());
        stc->StyleSetForeground(wxSTC_P_STRING, UserPrefs.is_HighContrast() ?
                                                    theme.clr_strings :
                                                    UserPrefs.get_PythonStringColour());
        stc->StyleSetForeground(wxSTC_P_WORD, UserPrefs.is_HighContrast() ?
                                                  theme.clr_keywords :
                                                  UserPrefs.get_PythonColour());
        stc->StyleSetForeground(wxSTC_P_WORD2, UserPrefs.is_HighContrast() ?
                                                   theme.clr_functions :
                                                   UserPrefs.get_PythonKeywordColour());
    }

    void SetRubyColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                       const StcColorTheme& theme)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_RUBY);
        }
        if (add_keywords)
        {
            // We don't set ruby keywords because we can't colorize them differently from the
            // wxWidgets keywords.

            wxue::string wxRuby_keywords(
                "ALL LEFT RIGHT TOP BOTTOM DEFAULT_POSITION DEFAULT_SIZE HORIZONTAL VERTICAL "
                "ID_ANY ID_OK ID_CANCEL ID_SAVE ID_YES ID_NO "
                "TAB_TRAVERSAL FILTER_DIGITS Wx");
            for (const auto* iter: lst_widgets_keywords)
            {
                wxRuby_keywords << ' ' << (iter + 2);
            }

            for (auto* iter: NodeCreation.get_NodeDeclarationArray())
            {
                if (!iter)
                {
                    // This will happen if there is an enumerated value but no generator for it
                    continue;
                }

                if (!iter->get_DeclName().starts_with("wx"))
                {
                    continue;
                }
                if (iter->get_DeclName() == "wxContextMenuEvent" ||
                    iter->get_DeclName() == "wxTreeCtrlBase" ||
                    iter->get_DeclName().starts_with("wxRuby"))
                {
                    continue;
                }
                wxRuby_keywords << ' ' << iter->get_DeclName().substr(2);
            }

            // Unfortunately, RUBY_LEXER only supports one set of keywords so we have to combine the
            // regular keywords with the wxWidgets keywords.

            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxRuby_keywords.c_str());
        }

        stc->StyleSetBold(wxSTC_RB_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
        }
        stc->StyleSetForeground(wxSTC_RB_WORD, UserPrefs.is_HighContrast() ?
                                                   theme.clr_keywords :
                                                   UserPrefs.get_RubyColour());
        stc->StyleSetForeground(wxSTC_RB_STRING, UserPrefs.is_HighContrast() ?
                                                     theme.clr_strings :
                                                     UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_Q, UserPrefs.is_HighContrast() ?
                                                       theme.clr_strings :
                                                       UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QQ, UserPrefs.is_HighContrast() ?
                                                        theme.clr_strings :
                                                        UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QX, UserPrefs.is_HighContrast() ?
                                                        theme.clr_strings :
                                                        UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QR, UserPrefs.is_HighContrast() ?
                                                        theme.clr_strings :
                                                        UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_STRING_QW, UserPrefs.is_HighContrast() ?
                                                        theme.clr_strings :
                                                        UserPrefs.get_RubyStringColour());
        stc->StyleSetForeground(wxSTC_RB_COMMENTLINE, UserPrefs.is_HighContrast() ?
                                                          theme.clr_comments :
                                                          UserPrefs.get_RubyCommentColour());
        stc->StyleSetForeground(wxSTC_RB_NUMBER, UserPrefs.is_HighContrast() ?
                                                     theme.clr_numbers :
                                                     UserPrefs.get_RubyNumberColour());
        stc->StyleSetForeground(wxSTC_RB_SYMBOL, theme.clr_functions);
        // stc->StyleSetForeground(wxSTC_RB_INSTANCE_VAR, theme.clr_variables);
        stc->StyleSetForeground(wxSTC_RB_INSTANCE_VAR, theme.clr_keywords);
        stc->StyleSetForeground(wxSTC_RB_CLASS_VAR, theme.clr_keywords);
    }

    void SetFortranColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                          const StcColorTheme& theme)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_FORTRAN);
        }
        if (add_keywords)
        {
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_fortran_keywords);
        }
        stc->StyleSetBold(wxSTC_F_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
        }
        stc->StyleSetForeground(wxSTC_F_COMMENT, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_F_NUMBER, theme.clr_numbers);
        stc->StyleSetForeground(wxSTC_F_STRING1, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_F_STRING2, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_F_STRINGEOL, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_F_WORD, theme.clr_keywords);
        stc->StyleSetForeground(wxSTC_F_WORD2, theme.clr_functions);
        stc->StyleSetForeground(wxSTC_F_PREPROCESSOR, theme.clr_types);
    }

    void SetGoColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                     const StcColorTheme& theme)
    {
        // Go has no dedicated Scintilla lexer; use the C++ lexer since Go has C-like syntax.
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_CPP);
        }
        if (add_keywords)
        {
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_go_keywords);
        }
        stc->StyleSetBold(wxSTC_C_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
        }
        stc->StyleSetForeground(wxSTC_C_COMMENT, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_C_COMMENTLINE, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_C_COMMENTDOC, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_C_NUMBER, theme.clr_numbers);
        stc->StyleSetForeground(wxSTC_C_STRING, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_C_STRINGEOL, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_C_WORD, theme.clr_keywords);
        stc->StyleSetForeground(wxSTC_C_WORD2, theme.clr_functions);
    }

    void SetJuliaColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                        const StcColorTheme& theme)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_JULIA);
        }
        if (add_keywords)
        {
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_julia_keywords);
        }
        stc->StyleSetBold(wxSTC_JULIA_KEYWORD1, true);

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
        }
        stc->StyleSetForeground(wxSTC_JULIA_COMMENT, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_JULIA_DOCSTRING, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_JULIA_NUMBER, theme.clr_numbers);
        stc->StyleSetForeground(wxSTC_JULIA_STRING, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_JULIA_STRINGINTERP, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_JULIA_STRINGLITERAL, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_JULIA_CHAR, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_JULIA_KEYWORD1, theme.clr_keywords);
        stc->StyleSetForeground(wxSTC_JULIA_KEYWORD2, theme.clr_functions);
        stc->StyleSetForeground(wxSTC_JULIA_MACRO, theme.clr_types);
        stc->StyleSetForeground(wxSTC_JULIA_SYMBOL, theme.clr_functions);
        stc->StyleSetForeground(wxSTC_JULIA_TYPEANNOT, theme.clr_types);
    }

    void SetLuaJitColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                         const StcColorTheme& theme)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_LUA);
        }
        if (add_keywords)
        {
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_luajit_keywords);
        }
        stc->StyleSetBold(wxSTC_LUA_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
        }
        stc->StyleSetForeground(wxSTC_LUA_COMMENT, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_LUA_COMMENTLINE, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_LUA_COMMENTDOC, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_LUA_NUMBER, theme.clr_numbers);
        stc->StyleSetForeground(wxSTC_LUA_STRING, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_LUA_CHARACTER, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_LUA_LITERALSTRING, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_LUA_STRINGEOL, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_LUA_WORD, theme.clr_keywords);
        stc->StyleSetForeground(wxSTC_LUA_WORD2, theme.clr_functions);
    }

    void SetPerlColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                       const StcColorTheme& theme)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_PERL);
        }
        if (add_keywords)
        {
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_perl_keywords);
        }
        stc->StyleSetBold(wxSTC_PL_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
        }
        stc->StyleSetForeground(wxSTC_PL_COMMENTLINE, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_PL_POD, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_PL_NUMBER, theme.clr_numbers);
        stc->StyleSetForeground(wxSTC_PL_WORD, theme.clr_keywords);
        stc->StyleSetForeground(wxSTC_PL_STRING, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_PL_CHARACTER, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_PL_STRING_Q, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_PL_STRING_QQ, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_PL_STRING_QX, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_PL_STRING_QR, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_PL_STRING_QW, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_PL_SCALAR, theme.clr_variables);
        stc->StyleSetForeground(wxSTC_PL_ARRAY, theme.clr_variables);
        stc->StyleSetForeground(wxSTC_PL_HASH, theme.clr_variables);
        stc->StyleSetForeground(wxSTC_PL_OPERATOR, theme.clr_functions);
        stc->StyleSetForeground(wxSTC_PL_REGEX, theme.clr_types);
    }

    void SetRustColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                       const StcColorTheme& theme)
    {
        if (set_lexer)
        {
            stc->SetLexer(wxSTC_LEX_RUST);
        }
        if (add_keywords)
        {
            stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_rust_keywords);
        }
        stc->StyleSetBold(wxSTC_RUST_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            SetDarkModeBase(stc, theme);
        }
        stc->StyleSetForeground(wxSTC_RUST_COMMENTBLOCK, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_RUST_COMMENTLINE, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_RUST_COMMENTBLOCKDOC, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_RUST_COMMENTLINEDOC, theme.clr_comments);
        stc->StyleSetForeground(wxSTC_RUST_NUMBER, theme.clr_numbers);
        stc->StyleSetForeground(wxSTC_RUST_STRING, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_RUST_STRINGR, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_RUST_CHARACTER, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_RUST_BYTESTRING, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_RUST_BYTESTRINGR, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_RUST_BYTECHARACTER, theme.clr_strings);
        stc->StyleSetForeground(wxSTC_RUST_WORD, theme.clr_keywords);
        stc->StyleSetForeground(wxSTC_RUST_WORD2, theme.clr_functions);
        stc->StyleSetForeground(wxSTC_RUST_MACRO, theme.clr_types);
        stc->StyleSetForeground(wxSTC_RUST_LIFETIME, theme.clr_types);
    }

    void SetXrcColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                      const StcColorTheme& theme)
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
            SetDarkModeBase(stc, theme);

            stc->StyleSetForeground(wxSTC_H_COMMENT, UserPrefs.is_HighContrast() ?
                                                         theme.clr_comments :
                                                         wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_NUMBER, UserPrefs.is_HighContrast() ?
                                                        theme.clr_numbers :
                                                        wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_ENTITY, UserPrefs.is_HighContrast() ?
                                                        theme.clr_types :
                                                        wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_SINGLESTRING, UserPrefs.is_HighContrast() ?
                                                              theme.clr_strings :
                                                              wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_ATTRIBUTE, UserPrefs.is_HighContrast() ?
                                                           theme.clr_variables :
                                                           UserPrefs.get_XrcAttributeColour());
            stc->StyleSetForeground(wxSTC_H_DOUBLESTRING, UserPrefs.is_HighContrast() ?
                                                              theme.clr_strings :
                                                              UserPrefs.get_XrcDblStringColour());
            stc->StyleSetForeground(wxSTC_H_TAG, UserPrefs.is_HighContrast() ?
                                                     theme.clr_functions :
                                                     UserPrefs.get_XrcTagColour());
        }
        else
        {
            stc->StyleSetForeground(wxSTC_H_COMMENT, theme.clr_comments);
            stc->StyleSetForeground(wxSTC_H_NUMBER, theme.clr_numbers);
            stc->StyleSetForeground(wxSTC_H_ENTITY, theme.clr_types);
            stc->StyleSetForeground(wxSTC_H_SINGLESTRING, theme.clr_strings);
            stc->StyleSetForeground(wxSTC_H_ATTRIBUTE, UserPrefs.get_XrcAttributeColour());
            stc->StyleSetForeground(wxSTC_H_DOUBLESTRING, UserPrefs.get_XrcDblStringColour());
            stc->StyleSetForeground(wxSTC_H_TAG, UserPrefs.get_XrcTagColour());
        }
    }

    void SetXmlDefaultColors(wxStyledTextCtrl* stc, bool set_lexer, bool add_keywords,
                             const StcColorTheme& theme)
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
            SetDarkModeBase(stc, theme);

            stc->StyleSetForeground(wxSTC_H_COMMENT, UserPrefs.is_HighContrast() ?
                                                         theme.clr_comments :
                                                         wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_NUMBER, UserPrefs.is_HighContrast() ?
                                                        theme.clr_numbers :
                                                        wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_ENTITY, UserPrefs.is_HighContrast() ?
                                                        theme.clr_types :
                                                        wxColour("#ff6666"));
            stc->StyleSetForeground(wxSTC_H_SINGLESTRING, UserPrefs.is_HighContrast() ?
                                                              theme.clr_strings :
                                                              wxColour("#85e085"));
            stc->StyleSetForeground(wxSTC_H_ATTRIBUTE, UserPrefs.is_HighContrast() ?
                                                           theme.clr_variables :
                                                           UserPrefs.get_XrcAttributeColour());
            stc->StyleSetForeground(wxSTC_H_DOUBLESTRING, UserPrefs.is_HighContrast() ?
                                                              theme.clr_strings :
                                                              UserPrefs.get_XrcDblStringColour());
            stc->StyleSetForeground(wxSTC_H_TAG, UserPrefs.is_HighContrast() ?
                                                     theme.clr_functions :
                                                     UserPrefs.get_XrcTagColour());
        }
    }

}  // namespace

auto SetStcColors(wxStyledTextCtrl* stc, GenLang language, bool set_lexer, bool add_keywords)
    -> void
{
    StcColorTheme theme;
    theme.clr_comments = wxColor(0, 128, 0);
    theme.clr_functions = wxColor("#c76605");
    theme.clr_numbers = wxColor("#fa5c5c");
    theme.clr_keywords = wxColor("#0000FF");
    theme.clr_strings = wxColor("#008000");
    theme.clr_types = wxColor("#f59afd");
    theme.clr_variables = wxColor("#94e6fa");

    // These will adjust for both dark mode and high contrast mode if needed
    theme.foreground = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
    theme.background = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);

    if (UserPrefs.is_DarkMode() && UserPrefs.is_HighContrast())
    {
        theme.clr_comments = "#1cc462";
        theme.clr_functions = "#faa75a";
        theme.clr_numbers = "#FF0000";
        theme.clr_keywords = "#569CD6";
        theme.clr_strings = "#f1db0c";
        theme.clr_types = "#f59afd";
        theme.clr_variables = "#94e6fa";
    }

    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            SetCppColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_PYTHON:
            SetPythonColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_RUBY:
            SetRubyColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_FORTRAN:
            SetFortranColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_GO:
            SetGoColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_JULIA:
            SetJuliaColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_LUAJIT:
            SetLuaJitColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_PERL:
            SetPerlColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_RUST:
            SetRustColors(stc, set_lexer, add_keywords, theme);
            break;
        case GEN_LANG_XRC:
            SetXrcColors(stc, set_lexer, add_keywords, theme);
            break;
        default:  // Unknown language, default to xml
            SetXmlDefaultColors(stc, set_lexer, add_keywords, theme);
            break;
    }
}
