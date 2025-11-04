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

#include "node.h"              // Node class
#include "tt_string_vector.h"  // tt_string_vector -- Read/Write line-oriented strings/files

namespace code
{
    enum : std::uint8_t
    {
        nothing_needed = 0,
        pos_needed = 1 << 0,
        size_needed = 1 << 1,
        style_needed = 1 << 2,
        window_name_needed = 1 << 3
    };

    enum ScalingType : std::uint8_t
    {
        no_scaling = 0,
        allow_scaling = 1,
        conditional_scaling,
        force_scaling
    };

    enum : std::uint8_t
    {
        // Will add eol if empty.
        eol_if_empty = 1,
        // Will not add eol if empty, or there is already an eol at the end of
        // the current string.
        eol_if_needed,
        eol_always
    };

// REVIEW: [Randalphwa - 04-22-2025] clangd version 18.1.4 claims this is an unsed variable, but
// it is used in several files that #include code.h and require this.
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-const-variable"
#endif

    constexpr const bool no_dpi_scaling = false;
    constexpr const bool allow_dpi_scaling = true;

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
};  // namespace code

// Assume anyone including this header file needs access to the code namespace
using namespace code;

// Maps tt_string_view to std::string_view, allowing std::string_view comparisons
using view_map = std::map<std::string_view, std::string_view, std::less<>>;

extern const view_map g_map_python_prefix;
extern const view_map g_map_ruby_prefix;

// Returns true if value exists in one of the use Wx qw(...) declarations.
// If true, then the constant is available without modification.
[[nodiscard]] auto HasPerlMapConstant(std::string_view value) -> bool;

class Code : public tt_string
{
public:
    static constexpr int DEFAULT_BREAK_LENGTH = 80;
    static constexpr int MIN_BREAK_LENGTH = 10;

    Code(Node* node, GenLang language = GEN_LANG_CPLUSPLUS);
    void Init(Node* node, GenLang language = GEN_LANG_CPLUSPLUS);

    auto GetCode() -> tt_string& { return *this; }
    [[nodiscard]] auto GetView() const -> tt_string_view { return *this; }

    void clear()
    {
        tt_string::clear();
        if (m_auto_break)
        {
            m_break_at = m_break_length;
            m_minium_length = MIN_BREAK_LENGTH;
        }
        else
        {
            // initialize this high enough that no line will break unless
            // SetBreakAt() is called
            m_break_at = 100000;  // NOLINT (magic number) // cppcheck-suppress magicLiteral
        }
    }

    [[nodiscard]] auto is_cpp() const -> bool { return m_language == GEN_LANG_CPLUSPLUS; }
    [[nodiscard]] auto is_perl() const -> bool { return m_language == GEN_LANG_PERL; }
    [[nodiscard]] auto is_python() const -> bool { return m_language == GEN_LANG_PYTHON; }
    [[nodiscard]] auto is_ruby() const -> bool { return m_language == GEN_LANG_RUBY; }

    [[nodiscard]] auto is_local_var() const -> bool;

    // Equivalent to calling m_node->as_int(prop_name)
    [[nodiscard]] auto IntValue(GenEnum::PropName prop_name) const -> int;

    [[nodiscard]] auto node() const -> Node* { return m_node; }
    void set_node(Node* node) { m_node = node; }

    [[nodiscard]] auto get_language() const -> GenLang { return m_language; }

    [[nodiscard]] auto HasValue(GenEnum::PropName prop_name) const -> bool;

    // Avoid the temptation to use tt_string_view instead of const char* -- the MSVC compiler will
    // assume value is a bool if you call  is_PropValue(propm, "string")

    [[nodiscard]] auto is_PropValue(PropName name, const char* value) const noexcept -> bool
    {
        return m_node->is_PropValue(name, value);
    }
    [[nodiscard]] auto is_PropValue(PropName name, bool value) const noexcept -> bool
    {
        return m_node->is_PropValue(name, value);
    }
    [[nodiscard]] auto is_PropValue(PropName name, int value) const noexcept -> bool
    {
        return m_node->is_PropValue(name, value);
    }

    [[nodiscard]] auto view(PropName name) const -> tt_string_view { return m_node->view(name); }

    // Returns m_node->as_bool(prop_name);
    [[nodiscard]] auto IsTrue(GenEnum::PropName prop_name) const -> bool
    {
        return m_node->as_bool(prop_name);
    }

    // Equivalent to calling m_node->as_bool(prop_name)
    [[nodiscard]] auto IsFalse(GenEnum::PropName prop_name) const -> bool
    {
        return !m_node->as_bool(prop_name);
    }

    // Equivalent to calling (node->as_string(prop_name) == text)
    [[nodiscard]] auto IsEqualTo(GenEnum::PropName prop_name, tt_string_view text) const -> bool
    {
        return (m_node->as_string(prop_name) == text);
    }

    // Equivalent to calling (node->as_string(prop_name) != text)
    [[nodiscard]] auto IsNotEqualTo(GenEnum::PropName prop_name, tt_string_view text) const -> bool
    {
        return (m_node->as_string(prop_name) != text);
    }

    // Equivalent to calling (node->as_int(prop_name) == val)
    [[nodiscard]] auto IsEqualTo(GenEnum::PropName prop_name, int val) const -> bool
    {
        return (m_node->as_int(prop_name) == val);
    }

    // Equivalent to calling (node->as_int(prop_name) != val)
    [[nodiscard]] auto IsNotEqualTo(GenEnum::PropName prop_name, int val) const -> bool
    {
        return (m_node->as_int(prop_name) != val);
    }

    [[nodiscard]] auto IsGen(GenEnum::GenName get_GenName) const -> bool
    {
        return m_node->is_Gen(get_GenName);
    }

    // Checks for prop_pos, prop_size, prop_style, prop_window_style, and prop_window_name
    [[nodiscard]] auto IsDefaultPosSizeFlags(tt_string_view def_style = tt_empty_cstr) const
        -> bool;

    // Equivalent to calling node->as_string(prop_name).contains(text)
    [[nodiscard]] auto PropContains(GenEnum::PropName prop_name, tt_string_view text) const -> bool;

    // Returns flags indicating what parameters are needed for the current node.
    // Code::nothing_needed is returned if no parameters are needed.
    //
    // If prop_style == default style, then style_needed is not set.
    [[nodiscard]] auto WhatParamsNeeded(tt_string_view default_style = tt_empty_cstr) const -> int;

    // Adds comma and optional trailing space: ", "
    auto Comma(bool trailing_space = true) -> Code&
    {
        *this += ',';
        if (trailing_space)
        {
            *this += ' ';
        }
        return *this;
    }

    // eol_if_empty, eol_if_needed or eol_always
    auto Eol(int flag = code::eol_always) -> Code&;

    // Pass true to only add EOL if there is already code in the string
    auto NewLine(bool check_size = false) -> Code& { return Eol(check_size); }

    // Adds as many tab characters as specified by nTabs. Note that tabs are converted to
    // spaces when the line is written.
    auto Tab(int nTabs = 1) -> Code&;

    // If C++ and node is a local variable, will add "auto* "
    auto AddAuto() -> Code&;

    void EnableAutoLineBreak(bool auto_break = true) { m_auto_break = auto_break; }

    // Only call the following two functions if you called DisableAutoLineBreak() first.

    // If current line (+ next_str_size) > m_break_at, then break the line. This will remove
    // any trailing space, and begin the next line with a single tab for C++, or a double
    // table for Python.
    auto CheckLineLength(size_t next_str_size = 0) -> Code&;

    // Equivalent to calling CheckLineLength(node->prop_as_string(prop_name).size())
    auto CheckLineLength(GenEnum::PropName next_prop_name) -> Code&;

    // Call this function if you added text directly including a final newline.
    void UpdateBreakAt()
    {
        m_break_at = size() + m_break_length;
        m_minium_length = size() + 10;  // NOLINT (magic number) // cppcheck-suppress magicLiteral
    }

    // Equivalent to calling node->prop_as_string(prop_name).size()
    [[nodiscard]] auto PropSize(GenEnum::PropName prop_name) const -> size_t;

    // If the string starts with "wx", Python code will be converted to "wx." and then the
    // string without the "wx" prefix. Ptyhon code will also handle multiple wx flags
    // separated by |.
    //
    // If needed, the line will be broken *before* the string is added.
    auto Add(tt_string_view text) -> Code&;

    // Same as Add() except that Perl won't use a Wx:: prefix, instead it assumes the
    // constant was defined in the "use Wx qw(...);" statement.
    auto AddConstant(tt_string_view text) -> Code&;

    auto Add(const Code& text) -> Code& { return Add(text.GetView()); }

    // Equivalent to calling as_string(prop_name). Correctly modifies the string for Python.
    auto Add(GenEnum::PropName prop_name) -> Code& { return as_string(prop_name); }

    auto AddIfLang(GenLang lang, tt_string_view text) -> Code&
    {
        if (m_language == lang)
        {
            Add(text);
        }
        return *this;
    }

    auto AddIfCpp(tt_string_view text) -> Code&
    {
        if (is_cpp())
        {
            Add(text);
        }
        return *this;
    }

    auto AddIfPerl(tt_string_view text) -> Code&
    {
        if (is_perl())
        {
            Add(text);
        }
        return *this;
    }

    auto AddIfPython(tt_string_view text) -> Code&
    {
        if (is_python())
        {
            Add(text);
        }
        return *this;
    }

    auto AddIfRuby(tt_string_view text) -> Code&
    {
        if (is_ruby())
        {
            Add(text);
        }
        return *this;
    }

    // Use this for wxWidgets types such as wxDefaultPosition, wxNullBitmap, etc.
    // Python will replace the "wx" with "wx."
    // Ruby will replace the "wx" with "Wx::" and change the name up uppercase snake_case.
    auto AddType(tt_string_view text) -> Code&;

    // Equibalent to Add(node->as_constant(prop_name, "...")
    auto AddConstant(GenEnum::PropName prop_name, tt_string_view short_name) -> Code&;

    // If Project.AddOptionalComments() is true, then add the comment on it's own line.
    // Set force to true to always add the comment.
    // The comment will be prefixed with "// " for C++ and "# " for Python and Ruby.
    // The comment will be followed by a newline.
    auto AddComment(std::string_view comment, bool force = false) -> Code&;

    // Adds "True" for Python, "1" for Perl, and "true" for all other languages
    auto True() -> Code&;

    // Calls AddTrue() or AddFalse() depending on the boolean value of the property
    auto TrueFalseIf(GenEnum::PropName prop_name) -> Code&;

    // Adds "False" for Python, "0" for Perl, and "false" for all other languages
    auto False() -> Code&;

    // Use Str() instead of Add() if you are *absolutely* certain you will never need
    // wxPython or wxRuby (or any other language) processing.
    //
    // This will call CheckLineLength(str.size()) first.
    auto Str(std::string_view str) -> Code&
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
    auto Function(tt_string_view text, bool add_operator = true) -> Code&;

    // C++ will add "::" and the function name. Python will add "." and the function name.
    // Ruby changes the function to snake_case.
    auto ClassMethod(tt_string_view function_name) -> Code&;

    // Like ClassMethod(), but assumes a variable not a class. C++ and Python add "." and the
    // name.
    //
    // Ruby adds "." and changes the function to snake_case.
    auto VariableMethod(tt_string_view function_name) -> Code&;

    // For C++, this simply calls the function. For Python it prefixes "self." to the
    // function name. Ruby changes the function to snake_case.
    auto FormFunction(tt_string_view text) -> Code&;

    // Adds ");" or ")"
    auto EndFunction() -> Code&;

    // Adds wxClass, Wx::Class, wx.Class, or any other language-required class name variant.
    auto Class(tt_string_view text) -> Code&;

    // Adds " = "
    // If class_name is specified, adds the " = new wxClass;" for C++ or normal
    // class assignment for other languages.
    auto Assign(tt_string_view class_name = tt_empty_cstr) -> Code&;

    // Adds " = new wxClass(" or " = wx.Class('. Set assign to false to not add the '='
    // Adds wxGeneric prefix if use_generic is true.
    // Creates wxPanel if node is a book page.
    // Specify override_name to override node->get_DeclName()
    auto CreateClass(bool use_generic = false, tt_string_view override_name = tt_empty_cstr,
                     bool assign = true) -> Code&;

    // Adds the object's class name and a open parenthesis: class(
    //
    // For Perl, the class name will be followed by "->new("
    // For Ruby, the class name will be followed by ".new("
    auto Object(tt_string_view class_name) -> Code&;

    // For non-C++ languages, this will remove any "m_" prefix from the node name
    // (node->get_NodeName()).
    //
    // For Python code, a non-local, non-form name will be prefixed with "self."
    // For Ruby code, a non-local, non-form name will be prefixed with "@"
    auto NodeName(Node* node = nullptr) -> Code&;

    // For C++, adds the var_name unchanged. Otherwise, any "m_" is removed.
    // If class_access is true, then "self." is prefixed for Python, or "@" for Ruby.
    //
    // Use this when NodeName() is not appropriate, e.g., checkbox in wxStaticBoxSizer
    auto VarName(tt_string_view var_name, bool class_access = true) -> Code&;

    // For Python code, a non-local, non-form name will be prefixed with "self."
    //
    // *this += m_node->get_Parent()->get_NodeName();
    auto ParentName() -> Code&;

    // Find a valid parent for the current node and add it's name. This is *not* the same as
    // ParentName() -- this will handle wxStaticBox and wxCollapsiblePane parents as well as
    // non-sizer parents.
    auto ValidParentName() -> Code&;

    // Adds "this" for C++, "$self" for Perl, and "self" for Python and Ruby
    auto FormParent() -> Code&;

    // Handles regular or or'd properties.
    //
    // If the property value begins with wx and the language is not C++, this will change the
    // prefix to match the language's prefix (e.g., wx. for wxPython).
    auto as_string(GenEnum::PropName prop_name) -> Code&;

    auto itoa(int val) -> Code&
    {
        *this += std::to_string(val);
        return *this;
    }

    auto itoa(int val1, int val2) -> Code&
    {
        Str(std::to_string(val1)).Comma() += std::to_string(val2);
        return *this;
    }

    auto itoa(size_t val) -> Code&
    {
        *this += std::to_string(val);
        return *this;
    }

    auto itoa(GenEnum::PropName prop_name1, GenEnum::PropName prop_name2) -> Code&
    {
        as_string(prop_name1).Comma().as_string(prop_name2);
        return *this;
    }

    // Places string in quotes (single for Ruby, double for other languages). If
    // prop_internationalize is set, quoted string is placed in function call to
    // wxGetTranslation().
    //
    // Empty strings generate wxEmptyString for C++, '' for Ruby and "" for other languages.
    auto QuotedString(GenEnum::PropName prop_name) -> Code&;

    // Calls color.GetAsString(wxC2S_HTML_SYNTAX).ToStdString() and places the result in
    // quotes.
    auto QuotedString(const wxColour& clr) -> Code&
    {
        QuotedString(clr.GetAsString(wxC2S_HTML_SYNTAX).ToStdString());
        return *this;
    }

    // Places string in quotes (single for Ruby, double for other languages). If
    // prop_internationalize is set, quoted string is placed in function call to
    // wxGetTranslation().
    //
    // Empty strings generate wxEmptyString for C++, '' for Ruby and "" for other languages.
    auto QuotedString(tt_string_view text) -> Code&;

    // If scale_border_size is true, will add the language-specific code for
    // "FromDIP(wxSize(prop_border_size,-1)).x". Otherwise, it will just add
    // prop_border_size
    auto BorderSize(GenEnum::PropName prop_name = prop_border_size) -> Code&;

    auto EmptyString() -> Code&
    {
        *this += is_cpp() ? "wxEmptyString" : "\"\"";
        return *this;
    }

    // Will either generate wxSize(...) or FromDIP(wxSize(...))
    auto WxSize(GenEnum::PropName prop_name = GenEnum::PropName::prop_size,
                int enable_dpi_scaling = conditional_scaling) -> Code&;

    // Will either generate wxSize(...) or FromDIP(wxSize(...))
    auto WxSize(wxSize size, int enable_dpi_scaling = conditional_scaling) -> Code&;

    // Will either generate wxPoint(...) or FromDIP(wxPoint(...))
    // Uses prop_pos to determine scaling if conditional_scaling is set
    auto WxPoint(wxPoint position, int enable_dpi_scaling = conditional_scaling) -> Code&;

    // Will either generate wxPoint(...) or FromDIP(wxPoint(...))
    auto Pos(GenEnum::PropName prop_name = GenEnum::PropName::prop_pos,
             int enable_dpi_scaling = conditional_scaling) -> Code&;

    // Check for pos, size, style, window_style, and window name, and generate code if needed
    // starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
    //
    // If the only style specified is def_style, then it will not be added.
    auto PosSizeFlags(ScalingType enable_dpi_scaling = conditional_scaling,
                      bool uses_def_validator = false, tt_string_view def_style = tt_empty_cstr)
        -> Code&;

    // Call this when you need to force a specific style such as "wxCHK_3STATE"
    auto PosSizeForceStyle(tt_string_view force_style, bool uses_def_validator = true) -> Code&;

    // This will output "0" if there are no styles (style, window_style, tab_position etc.)
    //
    // If style is a friendly name, add the prefix parameter to prefix lookups.
    auto Style(const char* prefix = nullptr, tt_string_view force_style = tt_empty_cstr) -> Code&;

    auto GenFont(GenEnum::PropName prop_name = prop_font, tt_string_view font_function = "SetFont(")
        -> Code&;

    // Generates code for prop_window_extra_style, prop_background_colour,
    // prop_foreground_colour, prop_disabled, prop_hidden, prop_maximum_size, prop_variant,
    // prop_tooltip, and prop_context_help
    void GenWindowSettings();

    void GenFontColourSettings();

    // Calls the language-specific function to generate code for the specified bitmap
    // property
    auto Bundle(GenEnum::PropName prop) -> Code&;

    void BundlePerl(const tt_string_vector& parts);
    void BundlePython(const tt_string_vector& parts);
    void BundleRuby(const tt_string_vector& parts);

    void AddPythonImageName(const EmbeddedImage* embed);
    void AddPerlImageName(const EmbeddedImage* embed);

    void AddPythonSingleBitmapBundle(const tt_string_vector& parts, const ImageBundle* bundle,
                                     const tt_string& name);
    void AddPythonTwoBitmapBundle(const tt_string_vector& parts, const ImageBundle* bundle,
                                  const tt_string& name, const tt_string& path);
    void AddPythonMultiBitmapBundle(const tt_string_vector& parts, const ImageBundle* bundle);

    // Creates a string using either wxSystemSettings::GetColour(name) or wxColour(r, g, b).
    // Generates wxNullColour if the property is empty.
    auto ColourCode(GenEnum::PropName prop_name) -> Code&;

    auto GenSizerFlags() -> Code&;
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
    // In C++ and Perl, "{" will be added before calling Indent().
    auto OpenBrace(bool all_languages = false) -> Code&;

    // In C++, this adds "\\n}" and removes indentation set by OpenBrace().
    //
    // if (all_languages == true) other languages add '\n\ and call Unindent()
    // Set close_ruby to false if there will be an else statement next.
    auto CloseBrace(bool all_languages = false, bool close_ruby = true) -> Code&;

    void ResetBraces() { m_within_braces = false; }

    // In C++ adds "if (".
    // In Python and Ruby, adds "if ".
    auto BeginConditional() -> Code&;

    // For C++ and Ruby, adds " && ".
    // For Python, adds " and ".
    auto AddConditionalAnd() -> Code&;

    // For C++ and Ruby, adds " || ".
    // For Python, adds " or ".
    auto AddConditionalOr() -> Code&;

    // In C++ conditional statements are terminated with ')'.
    // In Python conditional statements are terminated with ':'
    // Ruby doesn't need anything to end a conditional statement.
    auto EndConditional() -> Code&;

    // Returns false if enable_dpi_scaling is set to no_dpi_scaling, or property contains a
    // 'n', or language is C++ and wxWidgets 3.1 is being used, or enable_dpi_scaling is set
    // to conditional_scaling and the node is a form.
    [[nodiscard]] auto is_ScalingEnabled(GenEnum::PropName prop_name,
                                         int enable_dpi_scaling = code::allow_scaling) const
        -> bool;

    // For Ruby, this will place any member variables declared as public in a attr_accessor
    // list.
    void AddPublicRubyMembers();

    // This will expand a lamda function according to the current language.
    // Note that it takes a copy of the lambda string since it needs to modify it.
    auto ExpandEventLambda(tt_string lambda) -> Code&;

    [[nodiscard]] auto get_Indentation() const { return m_indent; }

protected:
    void InsertLineBreak(size_t cur_pos);
    // Prefix with a period, lowercase for wxRuby, and add open parenthesis
    auto SizerFlagsFunction(tt_string_view function_name) -> Code&;

    static auto GetLanguagePrefix(std::string_view candidate, GenLang language) -> std::string_view;

    void OpenFontBrace();
    void CloseFontBrace();

    // Helper methods to reduce complexity in GenFont
    void GenDefGuiFont(const FontProperty& fontprop, tt_string_view font_function);
    void GenFontInfoCode(const FontProperty& fontprop);
    void GenFontInfoInit(const FontProperty& fontprop, double point_size, bool more_than_pointsize);
    void GenFontInfoProperties(const FontProperty& fontprop);
    void ApplyFontToControl(tt_string_view font_function);
    void ApplyFontProperty(const tt_string& font_var_name, tt_string_view method,
                           tt_string_view value);
    void SetFontOnControl(const tt_string& font_var_name, tt_string_view font_function);

    // Helper functions for GenFontColourSettings()
    void GenColourValue(tt_string_view colour_str, GenEnum::PropName prop_name);
    void GenSetColourFunction(tt_string_view function_name, bool for_property_sheet);

    // Helper functions for GenSizerFlags()
    void ProcessAlignmentFlags(const tt_string& prop);
    void ProcessSizerFlags(const tt_string& prop);
    void ProcessBorderFlags(const tt_string& prop, int border_size);

    // Helper methods for GenWindowSettings() - called internally to reduce complexity
    void GenExtraStyle();
    void GenDisabledState();
    void GenHiddenState();
    void GenMinMaxSize();
    void GenWindowVariant();
    void GenTooltipAndHelp();
    void CallNodeOrFormFunction(tt_string_view function_name);

    // Helper methods for CreateClass() - called internally to reduce complexity
    auto HandleCppSubclass() -> bool;
    [[nodiscard]] auto DetermineClassName(bool use_generic, tt_string_view override_name) const
        -> std::string;
    void AddClassNameForLanguage(const std::string& class_name);
    void AddSubclassParams();

    // Helper methods for WxSize() - called internally to reduce complexity
    auto WxSize_Ruby(wxSize size, size_t cur_pos, bool size_scaling) -> Code&;
    auto WxSize_Perl(wxSize size, size_t cur_pos, bool size_scaling) -> Code&;
    auto WxSize_Other(wxSize size, size_t cur_pos, bool size_scaling) -> Code&;

private:
    // Helper methods for initialization (static as they don't need instance data)
    [[nodiscard]] static auto GetLineBreakLength(GenLang language) -> size_t;
    [[nodiscard]] static auto GetLanguagePrefixForInit(GenLang language) -> std::string_view;
    [[nodiscard]] static auto GetIndentSize(GenLang language) -> int;
    [[nodiscard]] static auto GetLineOffset(GenLang language) -> size_t;

    // Helper methods for Function()
    void AddFunctionNoOperatorWithWx(tt_string_view text);
    void AddFunctionWithOperatorRuby(tt_string_view text);
    void AddFunctionWithOperatorPython(tt_string_view text);

    // Helper methods for ValidParentName()
    void AddFormParentName();

    // Helper methods for QuotedString()
    void ProcessEscapedChar(char chr, bool& has_escape);
    [[nodiscard]] static auto HasUtf8Char(tt_string_view text) -> bool;
    void AddQuoteClosing(bool has_escape, size_t begin_quote, bool has_utf_char);

    // Helper methods for WxSize()
    void AddScaledSizeRuby(wxSize size);
    void AddUnscaledSizeRuby(wxSize size);
    void AddScaledSizePerl(wxSize size);
    void AddUnscaledSizePerl(wxSize size);

    // Helper methods for Add()
    [[nodiscard]] auto AddRubyConstant(tt_string_view text) -> bool;
    auto AddCombinedValues(tt_string_view text) -> Code&;
    auto AddWxPrefixedConstant(tt_string_view text) -> Code&;

    Node* m_node;
    GenLang m_language;

    // This is changed on a per-language basis in Code::Init()
    tt_string m_language_wxPrefix { "wx" };

    size_t m_break_length { DEFAULT_BREAK_LENGTH };
    size_t m_break_at { DEFAULT_BREAK_LENGTH };  // this should be the same as m_break_length
    size_t m_minium_length {
        MIN_BREAK_LENGTH  // if the line is shorter than this, don't break it
    };

    int m_indent { 0 };
    int m_indent_size { 4 };  // amount of spaces to assume tab size is set to,  default: 4

    bool m_auto_break { true };
    bool m_within_braces { false };
    bool m_within_font_braces { false };
};
