/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"  // Enumerations for generators

class Node;

class Code
{
public:
    ttlib::cstr m_code;
    Node* m_node;
    int m_language;

    Code(Node* node, int language = GEN_LANG_CPLUSPLUS) : m_node(node), m_language(language) {}

    void clear()
    {
        m_code.clear();
        m_break_at = 10000;
    }
    auto size() { return m_code.size(); }

    bool is_cpp() const { return m_language == GEN_LANG_CPLUSPLUS; }
    bool is_python() const { return m_language == GEN_LANG_PYTHON; }
    bool is_local_var() const;

    bool HasValue(GenEnum::PropName prop_name) const;

    // Equivalent to calling m_node->prop_as_bool(prop_name)
    bool IsTrue(GenEnum::PropName prop_name) const;

    // Equivalent to calling m_node->prop_as_string(prop_name).contains(text)
    bool PropContains(GenEnum::PropName prop_name, ttlib::sview text) const;

    Node* node() const { return m_node; }

    Code& itoa(int val)
    {
        m_code << val;
        return *this;
    }

    // Pass true to only add EOL if there is already code in the string
    Code& Eol(bool check_size = false)
    {
        if (check_size)
        {
            if (m_code.size())
                m_code << '\n';
        }
        else
        {
            m_code << '\n';
        }
        return *this;
    }

    // Adds comma and space: ", "
    Code& Comma()
    {
        m_code << ", ";
        return *this;
    }

    Code& EmptyString()
    {
        m_code += is_cpp() ? "wxEmptyString" : "\"\"";
        return *this;
    }

    // Adds as many '\t' as specified
    Code& Tab(int tabs = 1);

    // If string starts with "wx" and language is not C++, then this will add "wx." and then
    // the string without the "wx" prefix.
    Code& Add(ttlib::sview text);

    // Adds -> or . to the string, then wxFunction or wx.Function
    Code& Function(ttlib::sview text);

    // Adds wxClass or wx.Class
    Code& Class(ttlib::sview text);

    // Adds " = new wxClass" or " = wx.Class'
    Code& Assign(ttlib::sview class_name);

    // Adds " = new wxClass(" or " = wx.Class('.
    // Adds wxGeneric prefix if use_generic is true.
    // Creates wxPanel if node is a book page.
    Code& CreateClass(bool use_generic = false);

    // Adds ");" or ")"
    Code& EndFunction();

    // m_code << m_node->get_node_name();
    Code& NodeName();

    // m_code << m_node->GetParent()->get_node_name();
    Code& ParentName();

    // This is *NOT* the same as ParentName() -- this will handle wxStaticBox and
    // wxCollapsiblePane parents as well as non-sizer parents
    Code& GetParentName();

    // Handles regular or or'd styles for C++ or Python
    Code& as_string(GenEnum::PropName prop_name);

    // Handles prop_internationalize and strings containing at least one utf8 character.
    // Generates correct code for C++ or Python.
    Code& QuotedString(GenEnum::PropName prop_name);

    // Will either generate wxSize(...) or ConvertDialogToPixels(wxSize(...))
    Code& WxSize(GenEnum::PropName prop_name = GenEnum::PropName::prop_size);

    // Will either generate wxPoint(...) or ConvertDialogToPixels(wxPoint(...))
    Code& Pos(GenEnum::PropName prop_name = GenEnum::PropName::prop_pos);

    // This will output "0" if there are no styles (style, window_style, tab_position etc.)
    //
    // If style is a friendly name, add the prefix parameter to prefix lookups.
    Code& Style(const char* prefix = nullptr);

    // Check for pos, size, flags, window_flags, and window name, and generate code if needed
    // starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
    //
    // If the only style specified is def_style, then it will not be added.
    Code& PosSizeFlags(bool uses_def_validator = false, ttlib::sview def_style = tt_empty_cstr);

    Code& GenSizerFlags();

    Code& operator<<(std::string_view str)
    {
        m_code += str;
        return *this;
    }

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

    Code& CheckLineLength();
    void SetBreakAt(int break_at) { m_break_at = break_at; }

private:
    size_t m_break_at { 10000 };
};
