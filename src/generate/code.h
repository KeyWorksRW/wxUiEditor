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

    void clear() { m_code.clear(); }
    auto size() { return m_code.size(); }

    bool is_cpp() const { return m_language == GEN_LANG_CPLUSPLUS; }
    bool is_python() const { return m_language == GEN_LANG_PYTHON; }
    bool is_local_var() const;

    Code& Eol()
    {
        m_code << '\n';
        return *this;
    }

    // Adds as many '\t' as specified
    Code& Tab(int tabs = 1);

    // If string starts with "wx" and language is not C++, then this will add "wx." and then
    // the string without the "wx" prefix.
    Code& Add(ttlib::sview text);

    // Adds -> or . to the string, then wxFunction or wx.Function
    Code& Function(ttlib::sview text);

    // Adds " = new wxClass" or " = wx.Class'
    Code& Assign(ttlib::sview class_name);

    // Adds ");" or ")"
    Code& EndFunction();

    // m_code << m_node->get_node_name();
    Code& NodeName();

    // m_code << m_node->GetParent()->get_node_name();
    Code& ParentName();

    // Handles regular or or'd styles for C++ or Python
    Code& as_string(GenEnum::PropName prop_name);

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
};
