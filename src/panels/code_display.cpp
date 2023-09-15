/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/msgdlg.h>       // common header and base class for wxMessageDialog

#include "code_display.h"  // auto-generated: wxui/codedisplay_base.h and wxui/codedisplay_base.cpp

#include "base_panel.h"      // BasePanel -- Code generation panel
#include "code.h"            // Code -- Helper class for generating code
#include "mainframe.h"       // MainFrame -- Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator -- Class used to create nodes
#include "node_event.h"      // NodeEvent and NodeEventInfo classes
#include "preferences.h"     // Prefs -- Set/Get wxUiEditor preferences
#include "propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

// For the actual Scintilla constants, see the following file.
// ../../wxWidgets/src/stc/lexilla/include/SciLexer.h

extern const char* g_u8_cpp_keywords;
extern const char* g_python_keywords;

extern const char* g_golang_keywords;
extern const char* g_lua_keywords;
extern const char* g_perl_keywords;
extern const char* g_rust_keywords;
extern const char* g_ruby_keywords;

// XRC Keywords are defined in gen_xrc_utils.cpp so they can easily be updated as XRC
// generators support more XRC controls.
extern const char* g_xrc_keywords;

const int node_marker = 1;

CodeDisplay::CodeDisplay(wxWindow* parent, int panel_type) : CodeDisplayBase(parent), m_panel_type(panel_type)
{
    if (panel_type == GEN_LANG_XRC)
    {
        m_scintilla->SetLexer(wxSTC_LEX_XML);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

        m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#FF00FF"));
            m_scintilla->StyleSetForeground(wxSTC_H_TAG, wxColour("#80ccff"));
            m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour("#85e085"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#FF00FF"));
            m_scintilla->StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, *wxRED);
            m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
            m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));
        }
    }
    else if (panel_type == GEN_LANG_PYTHON)
    {
        m_scintilla->SetLexer(wxSTC_LEX_PYTHON);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_python_keywords);

        tt_string wxPython_keywords("ToolBar MenuBar BitmapBundle Bitmap MemoryInputStream Window");

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
        m_scintilla->StyleSetForeground(wxSTC_P_WORD2, UserPrefs.get_PythonColour());

        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            double hue, saturation, luminance;
            wxColourToHSL(wxColour(0, 128, 0), hue, saturation, luminance);
            luminance = .80;
            auto light_green = HSLToWxColour(hue, saturation, luminance);

            m_scintilla->StyleSetForeground(wxSTC_P_WORD, wxColour("#80ccff"));
            // m_scintilla->StyleSetForeground(wxSTC_P_STRING, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_P_STRING, light_green);
            m_scintilla->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_P_COMMENTLINE, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_P_NUMBER, wxColour("#ff6666"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_P_WORD, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_P_STRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_P_COMMENTLINE, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_P_NUMBER, *wxRED);
        }
    }
    else if (panel_type == GEN_LANG_RUBY)
    {
        m_scintilla->SetLexer(wxSTC_LEX_RUBY);

        // We don't set ruby keywords because we can't colorize them differently from the
        // wxWidgets keywords.

        tt_string wxRuby_keywords("ToolBar MenuBar BitmapBundle Bitmap Window Wx");

        // clang-format on

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

        // Unfortunately, RUBY_LEXER only supports one set of keywords so we have to combine the regular keywords with
        // the wxWidgets keywords.

        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxRuby_keywords.c_str());
        m_scintilla->StyleSetForeground(wxSTC_RB_WORD, UserPrefs.get_RubyColour());

        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_RB_STRING, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_RB_COMMENTLINE, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_RB_NUMBER, wxColour("#ff6666"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_RB_STRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_RB_COMMENTLINE, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_RB_NUMBER, *wxRED);
        }
    }
#if defined(_DEBUG)
    // The following language panels are experimental and only appear in a Debug build

    else if (panel_type == GEN_LANG_GOLANG)
    {
        // Currently, there is no lexer for GoLang, so we use the C++ lexer instead.
        m_scintilla->SetLexer(wxSTC_LEX_CPP);

        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_golang_keywords);

        // On Windows, this saves converting the UTF16 characters to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords("ToolBar MenuBar BitmapBundle Bitmap Image MemoryInputStream Vector Window");

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->declName().subview(2);
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_C_WORD, true);
        m_scintilla->StyleSetForeground(wxSTC_C_WORD2, UserPrefs.get_CppColour());

        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_C_WORD, wxColour("#80ccff"));
            m_scintilla->StyleSetForeground(wxSTC_C_STRING, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_STRINGEOL, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_C_NUMBER, wxColour("#ff6666"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_C_STRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_C_STRINGEOL, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_C_NUMBER, *wxRED);
        }
    }

    else if (panel_type == GEN_LANG_LUA)
    {
        m_scintilla->SetLexer(wxSTC_LEX_LUA);

        // On Windows, this saves converting the UTF16 characters to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_lua_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords("ToolBar MenuBar BitmapBundle Bitmap Image MemoryInputStream Vector Window");

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->declName().subview(2);
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_LUA_WORD, true);
        m_scintilla->StyleSetForeground(wxSTC_LUA_WORD2, UserPrefs.get_CppColour());

        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_C_WORD, wxColour("#80ccff"));
            m_scintilla->StyleSetForeground(wxSTC_C_STRING, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_STRINGEOL, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_C_NUMBER, wxColour("#ff6666"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_LUA_WORD, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_LUA_STRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_LUA_STRINGEOL, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_LUA_PREPROCESSOR, wxColour(49, 106, 197));
            m_scintilla->StyleSetForeground(wxSTC_LUA_COMMENT, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_LUA_COMMENTLINE, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_LUA_COMMENTDOC, wxColour(0, 128, 0));
        }
        m_scintilla->StyleSetForeground(wxSTC_LUA_NUMBER, *wxRED);
    }

    else if (panel_type == GEN_LANG_PERL)
    {
        m_scintilla->SetLexer(wxSTC_LEX_PERL);

        // LEX_PERL doesn't support coloring WORD2, so we only color the wxWidgets keywords.
        // m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_perl_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords("ToolBar MenuBar BitmapBundle Bitmap Image MemoryInputStream Vector Window");

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->declName().subview(2);
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_PL_WORD, true);

        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_PL_WORD, wxColour("#80ccff"));
            m_scintilla->StyleSetForeground(wxSTC_PL_STRING, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_PL_PREPROCESSOR, wxColour(49, 106, 197));
            m_scintilla->StyleSetForeground(wxSTC_PL_COMMENTLINE, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_PL_NUMBER, wxColour("#ff6666"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_PL_WORD, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_PL_STRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_PL_PREPROCESSOR, wxColour(49, 106, 197));
            m_scintilla->StyleSetForeground(wxSTC_PL_COMMENTLINE, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_PL_NUMBER, *wxRED);
        }
    }

    else if (panel_type == GEN_LANG_RUST)
    {
        m_scintilla->SetLexer(wxSTC_LEX_RUST);

        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_rust_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords("ToolBar MenuBar BitmapBundle Bitmap Image MemoryInputStream Vector Window");

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->declName().subview(2);
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_RUST_WORD, true);
        m_scintilla->StyleSetForeground(wxSTC_RUST_WORD2, UserPrefs.get_CppColour());

        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }

            m_scintilla->StyleSetForeground(wxSTC_RUST_WORD, wxColour("#80ccff"));
            m_scintilla->StyleSetForeground(wxSTC_RUST_STRING, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_RUST_COMMENTLINE, wxColour("#85e085"));
            m_scintilla->StyleSetForeground(wxSTC_RUST_COMMENTLINEDOC, wxColour("#ff6666"));
            m_scintilla->StyleSetForeground(wxSTC_RUST_NUMBER, wxColour("#ff6666"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_RUST_WORD, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_RUST_STRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_RUST_COMMENTLINE, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_RUST_COMMENTLINEDOC, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_RUST_NUMBER, *wxRED);
        }
    }
#endif  // _DEBUG

    else  // C++
    {
        m_scintilla->SetLexer(wxSTC_LEX_CPP);
        // On Windows, this saves converting the UTF16 characters to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords(
            "wxToolBar wxMenuBar wxBitmapBundle wxBitmap wxImage wxMemoryInputStream wxVector wxWindow");

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

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_C_WORD, true);

        // First set all possible foreground/background colours
        if (UserPrefs.is_DarkMode())
        {
            auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
            auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
            for (int idx = 0; idx <= wxSTC_STYLE_LASTPREDEFINED; idx++)
            {
                m_scintilla->StyleSetForeground(idx, fg);
                m_scintilla->StyleSetBackground(idx, bg);
            }
        }

        auto clr_green = wxColour(0, 128, 0);
        if (UserPrefs.is_DarkMode())
        {
            if (UserPrefs.is_HighContrast())
            {
                clr_green = wxColour("#1cc462");
            }
            else
            {
                wxColour(0, 192, 0);
            }
        }
        m_scintilla->StyleSetForeground(wxSTC_C_STRING, clr_green);
        m_scintilla->StyleSetForeground(wxSTC_C_STRINGEOL, clr_green);
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENT, clr_green);
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE, clr_green);
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENTDOC, clr_green);
        m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, clr_green);

        if (UserPrefs.is_DarkMode())
        {
            m_scintilla->StyleSetForeground(wxSTC_C_WORD2, wxColourToDarkForeground(UserPrefs.get_CppColour()));
            m_scintilla->StyleSetForeground(wxSTC_C_WORD, wxColourToDarkForeground(*wxBLUE));
            if (UserPrefs.is_HighContrast())
            {
                m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour("#569CD6"));
            }
            else
            {
                m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
            }
            m_scintilla->StyleSetForeground(wxSTC_C_NUMBER, wxColour("#ff6666"));
        }
        else
        {
            m_scintilla->StyleSetForeground(wxSTC_C_WORD2, UserPrefs.get_CppColour());
            m_scintilla->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
            m_scintilla->StyleSetForeground(wxSTC_C_NUMBER, *wxRED);
        }
    }

    // TODO: [KeyWorks - 01-02-2022] We do this because currently font selection uses a facename which is not cross-platform.
    // See issue #597.
    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

    Bind(wxEVT_FIND, &CodeDisplay::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &CodeDisplay::OnFind, this);
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
    if (!node->hasProp(prop_var_name) && m_panel_type != GEN_LANG_XRC && !node->isGen(gen_ribbonTool) &&
        !node->isGen(gen_ribbonButton))
    {
        return;  // probably a form, spacer, or image
    }

    auto is_event = wxGetFrame().getPropPanel()->IsEventPageShowing();
    PANEL_PAGE page = wxGetFrame().getGeneratedPanel()->GetPanelPage();

    if (m_panel_type != GEN_LANG_CPLUSPLUS && page != CPP_PANEL)
        return;  // Nothing to search for in secondary pages of non-C++ languages

    int line = -1;

    tt_string name(" ");

    int language = GEN_LANG_CPLUSPLUS;
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
                    if (auto result = ProjectImages.FileNameToVarName(parts[IndexImage]); result)
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
                if (node->getForm()->as_bool(prop_generate_translation_unit))
                {
                    name << node->as_string(prop_var_name) << ";";
                    line = (to_int) m_view.FindLineContaining(name);
                }
                else
                {
                    name << node->as_string(prop_var_name) << " = ";
                    line = (to_int) m_view.FindLineContaining(name);
                }
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
