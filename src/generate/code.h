/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    This class is used to generate code for any language that wxUiEditor supports.
    Currently this is C++, wxPython and wxRuby3.

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

    enum ScalingType
    {
        no_scaling = 0,
        allow_scaling = 1,
        conditional_scaling,
        force_scaling
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

    constexpr const bool no_dpi_scaling = false;
    constexpr const bool allow_dpi_scaling = true;

};  // namespace code

// Assume anyone including this header file needs access to the code namespace
using namespace code;

extern const view_map g_map_python_prefix;
extern const view_map g_map_ruby_prefix;

// Returns true if value exists in one of the use Wx qw(...) declarations.
// If true, then the constant is available without modification.
bool HasPerlMapConstant(std::string_view value);

class Code : public tt_string
{
public:
    Node* m_node;
    GenLang m_language;

    Code(Node* node, GenLang language = GEN_LANG_CPLUSPLUS);
    void Init(Node* node, GenLang language = GEN_LANG_CPLUSPLUS);

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
    bool is_perl() const { return m_language == GEN_LANG_PERL; }
    bool is_python() const { return m_language == GEN_LANG_PYTHON; }
    bool is_ruby() const { return m_language == GEN_LANG_RUBY; }
    bool is_rust() const { return m_language == GEN_LANG_RUST; }

    bool is_fortran() const { return m_language == GEN_LANG_FORTRAN; }
    bool is_haskell() const { return m_language == GEN_LANG_HASKELL; }
    bool is_lua() const { return m_language == GEN_LANG_LUA; }

    bool is_local_var() const;

    // Equivalent to calling m_node->as_int(prop_name)
    int IntValue(GenEnum::PropName prop_name) const;

    Node* node() const { return m_node; }
    GenLang get_language() const { return m_language; }

    bool hasValue(GenEnum::PropName prop_name) const;

    // Avoid the temptation to use tt_string_view instead of const char* -- the MSVC compiler will assume value is a bool if
    // you call  isPropValue(propm, "string")

    bool isPropValue(PropName name, const char* value) const noexcept { return m_node->isPropValue(name, value); }
    bool isPropValue(PropName name, bool value) const noexcept { return m_node->isPropValue(name, value); }
    bool isPropValue(PropName name, int value) const noexcept { return m_node->isPropValue(name, value); }

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

    bool IsGen(GenEnum::GenName getGenName) const { return m_node->isGen(getGenName); }

    // Checks for prop_pos, prop_size, prop_style, prop_window_style, and prop_window_name
    bool IsDefaultPosSizeFlags(tt_string_view def_style = tt_empty_cstr) const;

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

    // If C++ and node is a local variable, will add "auto* "
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

    // Same as Add() except that Perl won't use a Wx:: prefix, instead it assumes the
    // constant was defined in the "use Wx qw(...);" statement.
    Code& AddConstant(tt_string_view text);

    Code& Add(const Code& text) { return Add(text.GetView()); }

    // Equivalent to calling as_string(prop_name). Correctly modifies the string for Python.
    Code& Add(GenEnum::PropName prop_name) { return as_string(prop_name); }

    Code& AddIfLang(GenLang lang, tt_string_view text)
    {
        if (m_language == lang)
            Add(text);
        return *this;
    }

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

    // Use this for wxWidgets types such as wxDefaultPosition, wxNullBitmap, etc.
    // Python will replace the "wx" with "wx."
    // Ruby will replace the "wx" with "Wx::" and change the name up uppercase snake_case.
    Code& AddType(tt_string_view text);

    // Equibalent to Add(node->as_constant(prop_name, "...")
    Code& AddConstant(GenEnum::PropName prop_name, tt_string_view short_name);

    // If UserPrefs.is_AddComments() is true, then add the comment on it's own line.
    // Set force to true to always add the comment.
    // The comment will be prefixed with "// " for C++ and "# " for Python and Ruby.
    // The comment will be followed by a newline.
    Code& AddComment(std::string_view comment, bool force = false);

    // Adds "True" for Python, "1" for Perl, and "true" for all other languages
    Code& True();

    // Calls AddTrue() or AddFalse() depending on the boolean value of the property
    Code& TrueFalseIf(GenEnum::PropName prop_name);

    // Adds "False" for Python, "0" for Perl, and "false" for all other languages
    Code& False();

    // Use Str() instead of Add() if you are *absolutely* certain you will never need
    // wxPython or wxRuby (or any other language) processing.
    //
    // This will call CheckLineLength(str.size()) first.
    Code& Str(std::string_view str)
    {
        if (is_ruby() && size() && (back() == '$' || back() == '(' || ends_with(".new")))
        {
            *this += str;
        }
        else
        {
            CheckLineLength(str.size());
            *this += str;
        }
        return *this;
    }

    // If add_operator is true, adds -> or . to the string, then function (fixing wx prefix
    // if needed)
    //
    // Use add_operator = false to convert a wxFunction to snake_case for Ruby.
    Code& Function(tt_string_view text, bool add_operator = true);

    // C++ will add "::" and the function name. Python will add "." and the function name.
    // Ruby changes the function to snake_case.
    Code& ClassMethod(tt_string_view function_name);

    // Like ClassMethod(), but assumes a variable not a class. C++ and Python add "." and the
    // name.
    //
    // Ruby adds "." and changes the function to snake_case.
    Code& VariableMethod(tt_string_view function_name);

    // For C++, this simply calls the function. For Python it prefixes "self." to the
    // function name. Ruby changes the function to snake_case.
    Code& FormFunction(tt_string_view text);

    // Adds ");" or ")"
    Code& EndFunction();

    // Adds wxClass or wx.Class
    Code& Class(tt_string_view text);

    // Adds " = "
    // If class_name is specified, adds the " = new wxClass;" for C++ or normal
    // class assignment for other languages.
    Code& Assign(tt_string_view class_name = tt_empty_cstr);

    // Adds " = new wxClass(" or " = wx.Class('. Set assign to false to not add the '='
    // Adds wxGeneric prefix if use_generic is true.
    // Creates wxPanel if node is a book page.
    // Specify override_name to override node->declName()
    Code& CreateClass(bool use_generic = false, tt_string_view override_name = tt_empty_cstr, bool assign = true);

    // Adds the object's class name and a open parenthesis: class(
    //
    // For Ruby, the object will be followed by ".new": class.new(
    Code& Object(tt_string_view class_name);

    // For non-C++ languages, this will remove any "m_" prefix from the node name
    // (node->getNodeName()).
    //
    // For Python code, a non-local, non-form name will be prefixed with "self."
    // For Ruby code, a non-local, non-form name will be prefixed with "@"
    Code& NodeName(Node* node = nullptr);

    // For C++, adds the var_name unchanged. Otherwise, any "m_" is removed.
    // If class_access is true, then "self." is prefixed for Python, or "@" for Ruby.
    //
    // Use this when NodeName() is not appropriate, e.g., checkbox in wxStaticBoxSizer
    Code& VarName(tt_string_view var_name, bool class_access = true);

    // For Python code, a non-local, non-form name will be prefixed with "self."
    //
    // *this += m_node->getParent()->getNodeName();
    Code& ParentName();

    // Find a valid parent for the current node and add it's name. This is *not* the same as
    // ParentName() -- this will handle wxStaticBox and wxCollapsiblePane parents as well as
    // non-sizer parents.
    Code& ValidParentName();

    // Handles regular or or'd properties.
    //
    // If the property value begins with wx and the language is not C++, this will change the
    // prefix to match the language's prefix (e.g., wx. for wxPython).
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
        as_string(prop_name1).Comma().as_string(prop_name2);
        return *this;
    }

    // Places string in quotes (single for Ruby, double for other languages). If
    // prop_internationalize is set, quoted string is placed in function call to
    // wxGetTranslation().
    //
    // Empty strings generate wxEmptyString for C++, '' for Ruby and "" for other languages.
    Code& QuotedString(GenEnum::PropName prop_name);

    // Calls color.GetAsString(wxC2S_HTML_SYNTAX).ToStdString() and places the result in
    // quotes.
    Code& QuotedString(wxColour clr)
    {
        QuotedString(clr.GetAsString(wxC2S_HTML_SYNTAX).ToStdString());
        return *this;
    }

    // Places string in quotes (single for Ruby, double for other languages). If
    // prop_internationalize is set, quoted string is placed in function call to
    // wxGetTranslation().
    //
    // Empty strings generate wxEmptyString for C++, '' for Ruby and "" for other languages.
    Code& QuotedString(tt_string_view text);

    // If scale_border_size is true, will add the language-specific code for
    // "FromDIP(wxSize(prop_border_size,-1)).x". Otherwise, it will just add
    // prop_border_size
    Code& BorderSize(GenEnum::PropName prop_name = prop_border_size);

    Code& EmptyString()
    {
        *this += is_cpp() ? "wxEmptyString" : "\"\"";
        return *this;
    }

    // Will either generate wxSize(...) or FromDIP(wxSize(...))
    Code& WxSize(GenEnum::PropName prop_name = GenEnum::PropName::prop_size, int enable_dpi_scaling = conditional_scaling);

    // Will either generate wxSize(...) or FromDIP(wxSize(...))
    Code& WxSize(wxSize size, int enable_dpi_scaling = conditional_scaling);

    // Will either generate wxPoint(...) or FromDIP(wxPoint(...))
    Code& Pos(GenEnum::PropName prop_name = GenEnum::PropName::prop_pos, int enable_dpi_scaling = conditional_scaling);

    // Check for pos, size, style, window_style, and window name, and generate code if needed
    // starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
    //
    // If the only style specified is def_style, then it will not be added.
    Code& PosSizeFlags(ScalingType enable_dpi_scaling = conditional_scaling, bool uses_def_validator = false,
                       tt_string_view def_style = tt_empty_cstr);

    // Call this when you need to force a specific style such as "wxCHK_3STATE"
    Code& PosSizeForceStyle(tt_string_view force_style, bool uses_def_validator = true);

    // This will output "0" if there are no styles (style, window_style, tab_position etc.)
    //
    // If style is a friendly name, add the prefix parameter to prefix lookups.
    Code& Style(const char* prefix = nullptr, tt_string_view force_style = tt_empty_cstr);

    Code& GenFont(GenEnum::PropName prop_name = prop_font, tt_string_view font_function = "SetFont(");

    // Generates code for prop_window_extra_style, prop_background_colour,
    // prop_foreground_colour, prop_disabled, prop_hidden, prop_maximum_size, prop_variant,
    // prop_tooltip, and prop_context_help
    void GenWindowSettings();

    void GenFontColourSettings();

    // Calls the language-specific function to generate code for the specified bitmap
    // property
    Code& Bundle(GenEnum::PropName prop);

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

    // Call Indent() and Eol(eol_if_needed).
    // In C++, Perl, and Rust "{" will be added before calling Indent().
    Code& OpenBrace(bool all_languages = false);

    // In C++, this adds "\\n}" and removes indentation set by OpenBrace().
    //
    // if (all_languages == true) other languages add '\n\ and call Unindent()
    // Set close_ruby to false if there will be an else statement next.
    Code& CloseBrace(bool all_languages = false, bool close_ruby = true);

    void ResetBraces() { m_within_braces = false; }

    // In C++ adds "if (".
    // In Python and Ruby, adds "if ".
    Code& BeginConditional();

    // For C++ and Ruby, adds " && ".
    // For Python, adds " and ".
    Code& AddConditionalAnd();

    // For C++ and Ruby, adds " || ".
    // For Python, adds " or ".
    Code& AddConditionalOr();

    // In C++ conditional statements are terminated with ')'.
    // In Python conditional statements are terminated with ':'
    // Ruby doesn't need anything to end a conditional statement.
    Code& EndConditional();

    // Returns false if enable_dpi_scaling is set to no_dpi_scaling, or property contains a
    // 'n', or language is C++ and wxWidgets 3.1 is being used, or enable_dpi_scaling is set
    // to conditional_scaling and the node is a form.
    bool is_ScalingEnabled(GenEnum::PropName prop_name, int enable_dpi_scaling = code::allow_scaling) const;

protected:
    void InsertLineBreak(size_t cur_pos);
    // Prefix with a period, lowercase for wxRuby, and add open parenthesis
    Code& SizerFlagsFunction(tt_string_view function_name);

    void OpenFontBrace();
    void CloseFontBrace();

private:
    // This is changed on a per-language basis in Code::Init()
    tt_string m_language_wxPrefix { "wx" };

    size_t m_break_length { 80 };
    size_t m_break_at { 80 };       // this should be the same as m_break_length
    size_t m_minium_length { 10 };  // if the line is shorter than this, don't break it

    int m_indent { 0 };
    int m_indent_size { 4 };  // amount of spaces to assume tab size is set to,  default: 4

    bool m_auto_break { true };
    bool m_within_braces { false };
    bool m_within_font_braces { false };
};
