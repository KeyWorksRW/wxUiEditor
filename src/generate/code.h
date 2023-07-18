/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    This class is used to generate code for any language that wxUiEditor supports
    (currently C++, Python and Ruby).

*/

#pragma once

#include "gen_enums.h"  // Enumerations for generators

#include "node.h"  // Node class

namespace code
{
    enum
    {
        nothing_needed = 0,
        pos_needed = 1 << 0,
        size_needed = 1 << 1,
        style_needed = 1 << 2,
        window_name_needed = 1 << 3
    };

    enum
    {
        // Will add eol if empty.
        eol_if_empty = 1,
        // Will not add eol if empty, or there is already an eol at the end of
        // the current string.
        eol_if_needed,
        eol_always
    };

    constexpr const bool no_dlg_units = false;
    constexpr const bool allow_dlg_units = true;

};  // namespace code

// Assume anyone including this header file needs acces to the code namespace
using namespace code;

class Code : public tt_string
{
public:
    Node* m_node;
    int m_language;

    Code(Node* node, int language = GEN_LANG_CPLUSPLUS);
    void Init(Node* node, int language = GEN_LANG_CPLUSPLUS);

    tt_string& GetCode() { return *this; }
    tt_string_view GetView() const { return *this; }

    void clear()
    {
        tt_string::clear();
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

    bool is_cpp() const { return m_language == GEN_LANG_CPLUSPLUS; }
    bool is_python() const { return m_language == GEN_LANG_PYTHON; }
    bool is_ruby() const { return m_language == GEN_LANG_RUBY; }
    bool is_local_var() const;

    // Equivalent to calling m_node->as_int(prop_name)
    int IntValue(GenEnum::PropName prop_name) const;

    Node* node() const { return m_node; }
    int get_language() const { return m_language; }

    bool HasValue(GenEnum::PropName prop_name) const;

    // Avoid the temptation to use tt_string_view instead of const char* -- the MSVC compiler will assume value is a bool if
    // you call  is_value(propm, "string")

    bool is_value(PropName name, const char* value) const noexcept { return m_node->is_value(name, value); }
    bool is_value(PropName name, bool value) const noexcept { return m_node->is_value(name, value); }
    bool is_value(PropName name, int value) const noexcept { return m_node->is_value(name, value); }

    // Can return nullptr
    NodeProperty* prop(PropName name) const { return m_node->prop(name); }

    tt_string_view view(PropName name) const { return m_node->view(name); }

    // Returns m_node->as_bool(prop_name);
    bool IsTrue(GenEnum::PropName prop_name) const { return m_node->as_bool(prop_name); }

    // Equivalent to calling m_node->as_bool(prop_name)
    bool IsFalse(GenEnum::PropName prop_name) const { return !m_node->as_bool(prop_name); }

    // Equivalent to calling (node->as_string(prop_name) == text)
    bool IsEqualTo(GenEnum::PropName prop_name, tt_string_view text) const { return (m_node->as_string(prop_name) == text); }

    // Equivalent to calling (node->as_string(prop_name) != text)
    bool IsNotEqualTo(GenEnum::PropName prop_name, tt_string_view text) const
    {
        return (m_node->as_string(prop_name) != text);
    }

    // Equivalent to calling (node->as_int(prop_name) == val)
    bool IsEqualTo(GenEnum::PropName prop_name, int val) const { return (m_node->as_int(prop_name) == val); }

    // Equivalent to calling (node->as_int(prop_name) != val)
    bool IsNotEqualTo(GenEnum::PropName prop_name, int val) const { return (m_node->as_int(prop_name) != val); }

    bool IsGen(GenEnum::GenName gen_name) const { return m_node->isGen(gen_name); }

    // Equivalent to calling node->as_string(prop_name).contains(text)
    bool PropContains(GenEnum::PropName prop_name, tt_string_view text) const;

    // Returns flags indicating what parameters are needed for the current node.
    // Code::nothing_needed is returned if no parameters are needed.
    //
    // If prop_style == default style, then style_needed is not set.
    int WhatParamsNeeded(tt_string_view default_style = tt_empty_cstr) const;

    // Adds comma and optional trailing space: ", "
    Code& Comma(bool trailing_space = true)
    {
        *this += ',';
        if (trailing_space)
            *this += ' ';
        return *this;
    }

    // eol_if_empty, eol_if_needed or eol_always
    Code& Eol(int flag = code::eol_always);

    // Pass true to only add EOL if there is already code in the string
    Code& NewLine(bool check_size = false) { return Eol(check_size); }

    // Adds as many tab characters as specified by nTabs. Note that tabs are converted to
    // spaces when the line is written.
    Code& Tab(int nTabs = 1);

    // In C++, this adds "{\n" and indents all lines until CloseBrace() is called.
    //
    // Ignored by Python and Ruby.
    Code& OpenBrace();

    Code& CloseBrace();

    // If C++ and node is a local variable, will add "auto* " -- otherwise, it does nothing.
    Code& AddAuto();

    void EnableAutoLineBreak(bool auto_break = true) { m_auto_break = auto_break; }

    // Only call the following two functions if you called DisableAutoLineBreak() first.

    // If current line (+ next_str_size) > m_break_at, then break the line. This will remove
    // any trailing space, and begin the next line with a single tab for C++, or a double
    // table for Python.
    Code& CheckLineLength(size_t next_str_size = 0);

    // Equivalent to calling CheckLineLength(node->prop_as_string(prop_name).size())
    Code& CheckLineLength(GenEnum::PropName next_prop_name);

    // Call this function if you added text directly including a final newline.
    void UpdateBreakAt()
    {
        m_break_at = size() + m_break_length;
        m_minium_length = size() + 10;
    }

    // Equivalent to calling node->prop_as_string(prop_name).size()
    size_t PropSize(GenEnum::PropName prop_name) const;

    // If the string starts with "wx", Python code will be converted to "wx." and then the
    // string without the "wx" prefix. Ptyhon code will also handle multiple wx flags
    // separated by |.
    //
    // If needed, the line will be broken *before* the string is added.
    Code& Add(tt_string_view text);

    Code& Add(const Code& text) { return Add(text.GetView()); }

    // Equivalent to calling as_string(prop_name). Correctly modifies the string for Python.
    Code& Add(GenEnum::PropName prop_name) { return as_string(prop_name); }

    Code& AddIfCpp(tt_string_view text)
    {
        if (is_cpp())
            Add(text);
        return *this;
    }

    Code& AddIfPython(tt_string_view text)
    {
        if (is_python())
            Add(text);
        return *this;
    }

    Code& AddIfRuby(tt_string_view text)
    {
        if (is_ruby())
            Add(text);
        return *this;
    }

    // Equibalent to Add(node->as_constant(prop_name, "...")
    Code& AddConstant(GenEnum::PropName prop_name, tt_string_view short_name);

    // Adds "true" for C++ or "True" for Python
    Code& AddTrue() { return Str(is_cpp() ? "true" : "True"); }

    // Adds "true" for C++ or "True" for Python
    Code& True() { return AddTrue(); }

    // Calls AddTrue() or AddFalse() depending on the boolean value of the property
    Code& TrueFalseIf(GenEnum::PropName prop_name);

    // Adds "false" for C++ or "False" for Python
    Code& AddFalse() { return Str(is_cpp() ? "false" : "False"); }

    // Adds "false" for C++ or "False" for Python
    Code& False() { return AddFalse(); }

    // Use Str() instead of Add() if you are *absolutely* certain you will never need
    // wxPython or wxRuby (or any other language) processing.
    //
    // This will call CheckLineLength(str.size()) first.
    Code& Str(std::string_view str)
    {
        CheckLineLength(str.size());
        *this += str;
        return *this;
    }

    // Equivalent to calling as_string(prop_name)
    Code& Str(GenEnum::PropName prop_name) { return as_string(prop_name); }

    // Adds -> or . to the string, then wxFunction or wx.Function
    Code& Function(tt_string_view text);

    // C++ will add "::" and the function name. Python will add "." and the function name.
    Code& ClassMethod(tt_string_view function_name);

    // For C++, this simply calls the function. For Python it prefixes "self." to the
    // function name.
    Code& FormFunction(tt_string_view text);

    // Adds ");" or ")"
    Code& EndFunction();

    // Adds wxClass or wx.Class
    Code& Class(tt_string_view text);

    // Adds " = new wxClass;" or " = wx.Class"
    Code& Assign(tt_string_view class_name);

    // Adds " = new wxClass(" or " = wx.Class('.
    // Adds wxGeneric prefix if use_generic is true.
    // Creates wxPanel if node is a book page.
    // Specify override_name to override node->DeclName()
    Code& CreateClass(bool use_generic = false, tt_string_view override_name = tt_empty_cstr);

    // For Python code, a non-local, non-form name will be prefixed with "self."
    //
    // *this += node->get_node_name();
    Code& NodeName(Node* node = nullptr);

    // For Python code, a non-local, non-form name will be prefixed with "self."
    //
    // *this += m_node->GetParent()->get_node_name();
    Code& ParentName();

    // Find a valid parent for the current node and add it's name. This is *not* the same as
    // ParentName() -- this will handle wxStaticBox and wxCollapsiblePane parents as well as
    // non-sizer parents.
    Code& ValidParentName();

    // Handles regular or or'd styles for C++ or Python
    Code& as_string(GenEnum::PropName prop_name);

    Code& itoa(int val)
    {
        *this += std::to_string(val);
        return *this;
    }

    Code& itoa(int val1, int val2)
    {
        Str(std::to_string(val1)).Comma() += std::to_string(val2);
        return *this;
    }

    Code& itoa(size_t val)
    {
        *this += std::to_string(val);
        return *this;
    }

    Code& itoa(GenEnum::PropName prop_name1, GenEnum::PropName prop_name2)
    {
        Str(prop_name1).Comma().Str(prop_name2);
        return *this;
    }

    // Handles prop_internationalize and strings containing at least one utf8 character.
    // Generates correct code for C++ or Python.
    Code& QuotedString(GenEnum::PropName prop_name);
    Code& QuotedString(tt_string_view text);

    // Will either generate wxSize(...) or ConvertDialogToPixels(wxSize(...))
    Code& WxSize(GenEnum::PropName prop_name = GenEnum::PropName::prop_size, bool enable_dlg_units = allow_dlg_units);

    Code& EmptyString()
    {
        *this += is_cpp() ? "wxEmptyString" : "\"\"";
        return *this;
    }

    // Will prefix text with "// " for C++ or "# " for Python
    Code& AddComment(tt_string_view text);

    // Will either generate wxPoint(...) or ConvertDialogToPixels(wxPoint(...))
    Code& Pos(GenEnum::PropName prop_name = GenEnum::PropName::prop_pos, bool enable_dlg_units = allow_dlg_units);

    // Check for pos, size, style, window_style, and window name, and generate code if needed
    // starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
    //
    // If the only style specified is def_style, then it will not be added.
    Code& PosSizeFlags(bool uses_def_validator = false, tt_string_view def_style = tt_empty_cstr);

    // Call this when you need to force a specific style such as "wxCHK_3STATE"
    Code& PosSizeForceStyle(tt_string_view force_style, bool uses_def_validator = true);

    // This will output "0" if there are no styles (style, window_style, tab_position etc.)
    //
    // If style is a friendly name, add the prefix parameter to prefix lookups.
    Code& Style(const char* prefix = nullptr, tt_string_view force_style = tt_empty_cstr);

    // Generates code for prop_window_extra_style, prop_background_colour,
    // prop_foreground_colour, prop_disabled, prop_hidden, prop_maximum_size, prop_variant,
    // prop_tooltip, and prop_context_help
    void GenWindowSettings();

    void GenFontColourSettings();

    // Creates a string using either wxSystemSettings::GetColour(name) or wxColour(r, g, b).
    // Generates wxNullColour if the property is empty.
    Code& ColourCode(GenEnum::PropName prop_name);

    Code& GenSizerFlags();

    void Indent(int amount = 1) { m_indent += amount; }
    void Unindent(int amount = 1)
    {
        while (amount > 0 && m_indent > 0)
        {
            --m_indent;
            --amount;
        }
    }
    void ResetIndent() { m_indent = 0; }
    void ResetBraces() { m_within_braces = false; }

protected:
    void InsertLineBreak(size_t cur_pos);
    // Prefix with a period, lowercase for wxRuby, and add open parenthesis
    Code& SizerFlagsFunction(tt_string_view function_name);

private:
    // wx for C++, wx. for Python, Wx:: for Ruby
    tt_string m_lang_wxPrefix { "wx" };

    size_t m_break_length { 80 };
    size_t m_break_at { 80 };       // this should be the same as m_break_length
    size_t m_minium_length { 10 };  // if the line is shorter than this, don't break it

    int m_indent { 0 };
    int m_indent_size { 4 };  // amount of spaces to assume tab size is set to

    bool m_auto_break { true };
    bool m_within_braces { false };
};
