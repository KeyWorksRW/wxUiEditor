/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    This class is used to generate code for any language that wxUiEditor supports (currently C++ and Python).

*/

#pragma once

#include "gen_enums.h"  // Enumerations for generators

class Node;

class Code
{
public:
    ttlib::cstr m_code;
    Node* m_node;
    int m_language;

    Code(Node* node, int language = GEN_LANG_CPLUSPLUS);

    void clear()
    {
        m_code.clear();
        if (m_auto_break)
        {
            m_break_at = m_break_length;
            m_minium_length = 10;
        }
        else
        {
            m_break_at = 100000;  // initialize this high enough that no line will break unless SetBreakAt() is called
        }
    }
    auto size() { return m_code.size(); }

    bool is_cpp() const { return m_language == GEN_LANG_CPLUSPLUS; }
    bool is_python() const { return m_language == GEN_LANG_PYTHON; }
    bool is_local_var() const;

    Node* node() const { return m_node; }

    bool HasValue(GenEnum::PropName prop_name) const;

    // Equivalent to calling m_node->prop_as_bool(prop_name)
    bool IsTrue(GenEnum::PropName prop_name) const;

    // Equivalent to calling (node->as_strin(prop_name) == text)
    bool IsEqualTo(GenEnum::PropName prop_name, ttlib::sview text) const;

    // Equivalent to calling node->as_string(prop_name).contains(text)
    bool PropContains(GenEnum::PropName prop_name, ttlib::sview text) const;

    // Adds comma and optional trailing space: ", "
    Code& Comma(bool trailing_space = true)
    {
        m_code += ',';
        if (trailing_space)
            m_code += ' ';
        return *this;
    }

    // Pass true to only add EOL if there is already code in the string
    Code& Eol(bool check_size = false);

    // Adds as many '\t' characters as specified by nTabs. Note that tabs are converted to
    // spaces when the line is written.
    Code& Tab(int nTabs = 1);

    void EnableAutoLineBreak(bool auto_break = true) { m_auto_break = auto_break; }

    // Only call the following two functions if you called DisableAutoLineBreak() first.

    // If current line (+ next_str_size) > m_break_at, then break the line. This will remove
    // any trailing space, and begin the next line with a single tab for C++, or a double
    // table for Python.
    Code& CheckLineLength(size_t next_str_size = 0);

    // Call this function if you added text directly including a final newline.
    void UpdateBreakAt()
    {
        m_break_at = m_code.size() + m_break_length;
        m_minium_length = m_code.size() + 10;
    }

    // If the string starts with "wx", Python code will be converted to "wx." and then the
    // string without the "wx" prefix. Ptyhon code will also handle multiple wx flags
    // separated by |.
    //
    // If needed, the line will be broken *before* the string is added.
    Code& Add(ttlib::sview text);

    // Use Str() instead of Add() if you don't need any special wxPython processing.
    Code& Str(std::string_view str)
    {
        m_code += str;
        return *this;
    }

    // Adds -> or . to the string, then wxFunction or wx.Function
    Code& Function(ttlib::sview text);

    // C++ will add "::" and the function name. Python will add "." and the function name.
    Code& ClassMethod(ttlib::sview function_name);

    // For C++, this simply calls the function. For Python it prefixes "self." to the
    // function name.
    Code& FormFunction(ttlib::sview text);

    // Adds ");" or ")"
    Code& EndFunction();

    // Adds wxClass or wx.Class
    Code& Class(ttlib::sview text);

    // Adds " = new wxClass" or " = wx.Class'
    Code& Assign(ttlib::sview class_name);

    // Adds " = new wxClass(" or " = wx.Class('.
    // Adds wxGeneric prefix if use_generic is true.
    // Creates wxPanel if node is a book page.
    // Specify override_name to override node->DeclName()
    Code& CreateClass(bool use_generic = false, ttlib::sview override_name = tt_empty_cstr);

    // For Python code, a non-local, non-form name will be prefixed with "self."
    //
    // m_code += m_node->get_node_name();
    Code& NodeName();

    // For Python code, a non-local, non-form name will be prefixed with "self."
    //
    // m_code += m_node->GetParent()->get_node_name();
    Code& ParentName();

    // This is *NOT* the same as ParentName() -- this will handle wxStaticBox and
    // wxCollapsiblePane parents as well as non-sizer parents
    Code& GetParentName();

    // Handles regular or or'd styles for C++ or Python
    Code& as_string(GenEnum::PropName prop_name);

    Code& itoa(int val)
    {
        m_code << val;
        return *this;
    }

    // Handles prop_internationalize and strings containing at least one utf8 character.
    // Generates correct code for C++ or Python.
    Code& QuotedString(GenEnum::PropName prop_name);
    Code& QuotedString(ttlib::sview text);

    // Will either generate wxSize(...) or ConvertDialogToPixels(wxSize(...))
    Code& WxSize(GenEnum::PropName prop_name = GenEnum::PropName::prop_size);

    Code& EmptyString()
    {
        m_code += is_cpp() ? "wxEmptyString" : "\"\"";
        return *this;
    }

    // Will either generate wxPoint(...) or ConvertDialogToPixels(wxPoint(...))
    Code& Pos(GenEnum::PropName prop_name = GenEnum::PropName::prop_pos);

    // Check for pos, size, style, window_style, and window name, and generate code if needed
    // starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
    //
    // If the only style specified is def_style, then it will not be added.
    Code& PosSizeFlags(bool uses_def_validator = false, ttlib::sview def_style = tt_empty_cstr);

    // Call this when you need to force a specific style such as "wxCHK_3STATE"
    Code& PosSizeForceStyle(ttlib::sview force_style, bool uses_def_validator = true);

    // This will output "0" if there are no styles (style, window_style, tab_position etc.)
    //
    // If style is a friendly name, add the prefix parameter to prefix lookups.
    Code& Style(const char* prefix = nullptr, ttlib::sview force_style = tt_empty_cstr);

    // Generates code for prop_window_extra_style, prop_background_colour,
    // prop_foreground_colour, prop_disabled, prop_hidden, prop_maximum_size, prop_variant,
    // prop_tooltip, and prop_context_help
    void GenWindowSettings();

    void GenFontColourSettings();

    Code& GenSizerFlags();

    Code& operator<<(std::string_view str)
    {
        m_code += str;
        return *this;
    }

    void operator+=(std::string_view str) { m_code += str; }

    Code& operator<<(char ch)
    {
        m_code += ch;
        return *this;
    }

    Code& operator<<(int i)
    {
        m_code << i;
        return *this;
    }

protected:
    void InsertLineBreak(size_t cur_pos);

private:
    bool m_auto_break { true };
    size_t m_break_length { 80 };
    size_t m_break_at { 80 };       // this should be the same as m_break_length
    size_t m_minium_length { 10 };  // if the line is shorter than this, don't break it
};
