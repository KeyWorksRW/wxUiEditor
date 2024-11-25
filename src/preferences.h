/////////////////////////////////////////////////////////////////////////////
// Purpose:   Set/Get wxUiEditor preferences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Prefs
{
private:
    Prefs() {}

public:
    Prefs(Prefs const&) = delete;

    void operator=(Prefs const&) = delete;

    static Prefs& getInstance()
    {
        static Prefs instance;
        return instance;
    }

    enum PREVIEW_TYPE
    {
        PREVIEW_TYPE_XRC = 0,
        PREVIEW_TYPE_BOTH,
        PREVIEW_TYPE_CPP,
    };

    void ReadConfig();
    void WriteConfig();
    // Add borders around all new sizers
    bool is_SizersAllBorders() const { return m_sizers_all_borders; }

    // Add expand flag to all new sizers
    bool is_SizersExpand() const { return m_sizers_always_expand; }

    // Enable WakaTime support
    bool is_WakaTimeEnabled() const { return m_enable_wakatime; }

    bool is_DarkMode() const { return m_dark_mode; }
    bool is_DarkModePending() const { return m_dark_mode_pending; }
    void set_DarkMode(bool value) { m_dark_mode = value; }
    void set_DarkModePending(size_t value) { m_dark_mode_pending = value; }
    bool is_HighContrast() const { return m_high_constrast; }
    void set_HighContrast(bool value) { m_high_constrast = value; }

    bool is_FullPathTitle() const { return m_fullpath_title; }
    void set_FullPathTitle(bool value) { m_fullpath_title = value; }

    bool is_AddComments() const { return m_add_comments; }
    void set_AddComments(bool value) { m_add_comments = value; }

    bool is_SvgImages() const { return m_svg_images; }
    void set_SvgImages(bool value) { m_svg_images = value; }

    bool is_LoadLastProject() const { return m_is_load_last_project; }
    void set_LoadLastProject(bool value) { m_is_load_last_project = value; }

    bool is_RightPropGrid() const { return m_is_right_propgrid; }
    void set_RightPropGrid(bool value) { m_is_right_propgrid = value; }

    bool is_CppSnakeCase() const { return m_is_cpp_snake_case; }
    void set_CppSnakeCase(bool value) { m_is_cpp_snake_case = value; }

    int get_CppLineLength() const { return m_cpp_line_length; }
    void set_CppLineLength(int length) { m_cpp_line_length = length; }

    int get_PythonLineLength() const { return m_python_line_length; }
    void set_PythonLineLength(int length) { m_python_line_length = length; }

    int get_RubyLineLength() const { return m_ruby_line_length; }
    void set_RubyLineLength(int length) { m_ruby_line_length = length; }

    int get_FortranLineLength() const { return m_haskell_line_length; }
    void set_FortranLineLength(int length) { m_haskell_line_length = length; }

    int get_HaskellLineLength() const { return m_haskell_line_length; }
    void set_HaskellLineLength(int length) { m_haskell_line_length = length; }

    int get_LuaLineLength() const { return m_lua_line_length; }
    void set_LuaLineLength(int length) { m_lua_line_length = length; }

    int get_PerlLineLength() const { return m_perl_line_length; }
    void set_PerlLineLength(int length) { m_perl_line_length = length; }

    int get_RustLineLength() const { return m_rust_line_length; }
    void set_RustLineLength(int length) { m_rust_line_length = length; }

    int get_IconSize() const { return m_icon_size; }
    void set_IconSize(int size) { m_icon_size = size; }

    // Use this string to construct a FontProperty() to get the values
    const tt_string& get_CodeDisplayFont() const { return m_code_display_font; }

    // This should be the string returned from FontProperty::as_string()
    void set_CodeDisplayFont(const tt_string& font) { m_code_display_font = font; }

    const tt_string& get_CppWidgetsVersion() const { return m_cpp_widgets_version; }
    void set_CppWidgetsVersion(const tt_string& version) { m_cpp_widgets_version = version; }
    const tt_string& get_PythonVersion() const { return m_python_version; }
    void set_PythonVersion(const tt_string& version) { m_python_version = version; }
    const tt_string& get_RubyVersion() const { return m_ruby_version; }
    void set_RubyVersion(const tt_string& version) { m_ruby_version = version; }

    const wxColour& get_CppColour() const { return m_colour_cpp; }  // wxWidgets keywords
    void set_CppColour(const wxColour& colour) { m_colour_cpp = colour; }
    const wxColour& get_CppKeywordColour() const { return m_colour_cpp_keyword; }  // C++ keywords
    void set_CppKeywordColour(const wxColour& colour) { m_colour_cpp_keyword = colour; }
    const wxColour& get_CppCommentColour() const { return m_colour_cpp_comment; }
    void set_CppCommentColour(const wxColour& colour) { m_colour_cpp_comment = colour; }
    const wxColour& get_CppNumberColour() const { return m_colour_cpp_number; }
    void set_CppNumberColour(const wxColour& colour) { m_colour_cpp_number = colour; }
    const wxColour& get_CppStringColour() const { return m_colour_cpp_string; }
    void set_CppStringColour(const wxColour& colour) { m_colour_cpp_string = colour; }

    const wxColour& get_PythonColour() const { return m_colour_python; }
    void set_PythonColour(const wxColour& colour) { m_colour_python = colour; }
    const wxColour& get_PythonKeywordColour() const { return m_colour_python_keyword; }
    void set_PythonKeywordColour(const wxColour& colour) { m_colour_python_keyword = colour; }
    const wxColour& get_PythonNumberColour() const { return m_colour_python_number; }
    void set_PythonNumberColour(const wxColour& colour) { m_colour_python_number = colour; }
    const wxColour& get_PythonStringColour() const { return m_colour_python_string; }
    void set_PythonStringColour(const wxColour& colour) { m_colour_python_string = colour; }
    const wxColour& get_PythonCommentColour() const { return m_colour_python_comment; }
    void set_PythonCommentColour(const wxColour& colour) { m_colour_python_comment = colour; }

    const wxColour& get_RubyColour() const { return m_colour_ruby; }
    void set_RubyColour(const wxColour& colour) { m_colour_ruby = colour; }
    const wxColour& get_RubyCommentColour() const { return m_colour_ruby_comment; }
    void set_RubyCommentColour(const wxColour& colour) { m_colour_ruby_comment = colour; }
    const wxColour& get_RubyNumberColour() const { return m_colour_ruby_number; }
    void set_RubyNumberColour(const wxColour& colour) { m_colour_ruby_number = colour; }
    const wxColour& get_RubyStringColour() const { return m_colour_ruby_string; }
    void set_RubyStringColour(const wxColour& colour) { m_colour_ruby_string = colour; }

    const wxColour& get_FortranColour() const { return m_colour_fortran; }
    void set_FortranColour(const wxColour& colour) { m_colour_fortran = colour; }
    const wxColour& get_FortranCommentColour() const { return m_colour_fortran_comment; }
    void set_FortranCommentColour(const wxColour& colour) { m_colour_fortran_comment = colour; }
    const wxColour& get_FortranKeywordColour() const { return m_colour_fortran_keyword; }
    void set_FortranKeywordColour(const wxColour& colour) { m_colour_fortran_keyword = colour; }
    const wxColour& get_FortranNumberColour() const { return m_colour_fortran_number; }
    void set_FortranNumberColour(const wxColour& colour) { m_colour_fortran_number = colour; }
    const wxColour& get_FortranStringColour() const { return m_colour_fortran_string; }
    void set_FortranStringColour(const wxColour& colour) { m_colour_fortran_string = colour; }

    const wxColour& get_HaskellColour() const { return m_colour_haskell; }
    void set_HaskellColour(const wxColour& colour) { m_colour_haskell = colour; }
    const wxColour& get_HaskellCommentColour() const { return m_colour_haskell_comment; }
    void set_HaskellCommentColour(const wxColour& colour) { m_colour_haskell_comment = colour; }
    const wxColour& get_HaskellKeywordColour() const { return m_colour_haskell_keyword; }
    void set_HaskellKeywordColour(const wxColour& colour) { m_colour_haskell_keyword = colour; }
    const wxColour& get_HaskellNumberColour() const { return m_colour_haskell_number; }
    void set_HaskellNumberColour(const wxColour& colour) { m_colour_haskell_number = colour; }
    const wxColour& get_HaskellStringColour() const { return m_colour_haskell_string; }
    void set_HaskellStringColour(const wxColour& colour) { m_colour_haskell_string = colour; }

    const wxColour& get_LuaColour() const { return m_colour_lua; }
    void set_LuaColour(const wxColour& colour) { m_colour_lua = colour; }
    const wxColour& get_LuaCommentColour() const { return m_colour_lua_comment; }
    void set_LuaCommentColour(const wxColour& colour) { m_colour_lua_comment = colour; }
    const wxColour& get_LuaKeywordColour() const { return m_colour_lua_keyword; }
    void set_LuaKeywordColour(const wxColour& colour) { m_colour_lua_keyword = colour; }
    const wxColour& get_LuaNumberColour() const { return m_colour_lua_number; }
    void set_LuaNumberColour(const wxColour& colour) { m_colour_lua_number = colour; }
    const wxColour& get_LuaStringColour() const { return m_colour_lua_string; }
    void set_LuaStringColour(const wxColour& colour) { m_colour_lua_string = colour; }

    const wxColour& get_PerlColour() const { return m_colour_perl; }
    void set_PerlColour(const wxColour& colour) { m_colour_perl = colour; }
    const wxColour& get_PerlCommentColour() const { return m_colour_perl_comment; }
    void set_PerlCommentColour(const wxColour& colour) { m_colour_perl_comment = colour; }
    const wxColour& get_PerlKeywordColour() const { return m_colour_perl_keyword; }
    void set_PerlKeywordColour(const wxColour& colour) { m_colour_perl_keyword = colour; }
    const wxColour& get_PerlNumberColour() const { return m_colour_perl_number; }
    void set_PerlNumberColour(const wxColour& colour) { m_colour_perl_number = colour; }
    const wxColour& get_PerlStringColour() const { return m_colour_perl_string; }
    void set_PerlStringColour(const wxColour& colour) { m_colour_perl_string = colour; }

    const wxColour& get_RustColour() const { return m_colour_rust; }
    void set_RustColour(const wxColour& colour) { m_colour_rust = colour; }
    const wxColour& get_RustCommentColour() const { return m_colour_rust_comment; }
    void set_RustCommentColour(const wxColour& colour) { m_colour_rust_comment = colour; }
    const wxColour& get_RustKeywordColour() const { return m_colour_rust_keyword; }
    void set_RustKeywordColour(const wxColour& colour) { m_colour_rust_keyword = colour; }
    const wxColour& get_RustNumberColour() const { return m_colour_rust_number; }
    void set_RustNumberColour(const wxColour& colour) { m_colour_rust_number = colour; }
    const wxColour& get_RustStringColour() const { return m_colour_rust_string; }
    void set_RustStringColour(const wxColour& colour) { m_colour_rust_string = colour; }

    const wxColour& get_XrcAttributeColour() const { return m_colour_xrc_attribute; }
    void set_XrcAttributeColour(const wxColour& colour) { m_colour_xrc_attribute = colour; }
    const wxColour& get_XrcDblStringColour() const { return m_colour_xrc_dblstring; }
    void set_XrcDblStringColour(const wxColour& colour) { m_colour_xrc_dblstring = colour; }
    const wxColour& get_XrcTagColour() const { return m_colour_xrc_tag; }
    void set_XrcTagColour(const wxColour& colour) { m_colour_xrc_tag = colour; }

    void set_SizersAllBorders(bool setting) { m_sizers_all_borders = setting; }
    void set_SizersExpand(bool setting) { m_sizers_always_expand = setting; }
    void set_VarPrefix(bool setting) { m_var_prefix = setting; }
    void set_WakaTimeEnabled(bool setting) { m_enable_wakatime = setting; }

    long GetDebugFlags() const { return m_flags; }
    void SetDebugFlags(long flags) { m_flags = flags; }

    long GetProjectFlags() const { return m_project_flags; }
    void SetProjectFlags(long flags) { m_project_flags = flags; }

    // The following are used by the OptionsDlg class to efficiently update the preferences

    bool& RefVarPrefix() { return m_var_prefix; }
    bool& RefSizersAllBorders() { return m_sizers_all_borders; }
    bool& RefSizersExpand() { return m_sizers_always_expand; }
    bool& RefWakaTimeEnabled() { return m_enable_wakatime; }

    PREVIEW_TYPE GetPreviewType() const { return m_preview_type; }
    void SetPreviewType(PREVIEW_TYPE type) { m_preview_type = type; }

    // The returned colour will depend on whether dark mode (and high contrast) is enabled or
    // not.
    wxColour GetColour(wxSystemColour index);

    // clang-format off
    enum : long
    {
        PREFS_MSG_WINDOW    = 1 << 2,   // automatically create CMsgFrame window
        PREFS_MSG_INFO      = 1 << 3,   // filter AddInfoMsg
        PREFS_MSG_EVENT     = 1 << 4,   // filter AddEventMsg
        PREFS_MSG_WARNING   = 1 << 5,   // filter AddWarningMsg

        PREFS_CREATION_MSG  = 1 << 6,  // Calls MSG_INFO when nav, prop, or mockup contents recreated
    };

    enum : long
    {
        PREFS_PJT_ALWAYS_LOCAL = 1 << 0,
        PREFS_PJT_MEMBER_PREFIX = 1 << 1,
    };

    enum : size_t
    {
        PENDING_DARK_MODE_ENABLE = 1,
        PENDING_DARK_MODE_ON = 1 << 1,
        PENDING_DARK_MODE_OFF = 1 << 2,
    };

    // clang-format on

private:
    // These store both Debug and INTERNAL flags
    long m_flags { 0 };

    long m_project_flags { 0 };
    PREVIEW_TYPE m_preview_type { PREVIEW_TYPE_XRC };

    tt_string m_cpp_widgets_version { "3.2" };
    tt_string m_python_version { "4.2" };
    tt_string m_ruby_version { "1.2" };
    tt_string m_fortran_version { "1.0" };
    tt_string m_haskell_version { "1.0" };
    tt_string m_lua_version { "3.2.0" };
    tt_string m_perl_version { "3.2" };
    tt_string m_rust_version { "3.0" };

    tt_string m_code_display_font;

    wxColour m_colour_cpp { wxColour("#FF00FF") };
    wxColour m_colour_cpp_comment { wxColour("#008000") };
    wxColour m_colour_cpp_keyword { wxColour("#0000FF") };
    wxColour m_colour_cpp_number { wxColour("#FF0000") };
    wxColour m_colour_cpp_string { wxColour("#008000") };

    wxColour m_colour_python { wxColour("#FF00FF") };
    wxColour m_colour_python_comment { wxColour("#008000") };
    wxColour m_colour_python_keyword { wxColour("#0000FF") };
    wxColour m_colour_python_number { wxColour("#FF0000") };
    wxColour m_colour_python_string { wxColour("#008000") };

    wxColour m_colour_ruby { wxColour("#FF00FF") };
    wxColour m_colour_ruby_comment { wxColour("#008000") };
    wxColour m_colour_ruby_number { wxColour("#FF000000") };
    wxColour m_colour_ruby_string { wxColour("#008000") };

    wxColour m_colour_fortran { wxColour("#FF00FF") };
    wxColour m_colour_fortran_comment { wxColour("#008000") };
    wxColour m_colour_fortran_keyword { wxColour("#0000FF") };
    wxColour m_colour_fortran_number { wxColour("#FF0000") };
    wxColour m_colour_fortran_string { wxColour("#008000") };

    wxColour m_colour_haskell { wxColour("#FF00FF") };
    wxColour m_colour_haskell_comment { wxColour("#008000") };
    wxColour m_colour_haskell_keyword { wxColour("#0000FF") };
    wxColour m_colour_haskell_number { wxColour("#FF0000") };
    wxColour m_colour_haskell_string { wxColour("#008000") };

    wxColour m_colour_lua { wxColour("#FF00FF") };
    wxColour m_colour_lua_comment { wxColour("#008000") };
    wxColour m_colour_lua_keyword { wxColour("#0000FF") };
    wxColour m_colour_lua_number { wxColour("#FF0000") };
    wxColour m_colour_lua_string { wxColour("#008000") };

    wxColour m_colour_perl { wxColour("#FF00FF") };
    wxColour m_colour_perl_comment { wxColour("#008000") };
    wxColour m_colour_perl_keyword { wxColour("#0000FF") };
    wxColour m_colour_perl_number { wxColour("#FF0000") };
    wxColour m_colour_perl_string { wxColour("#008000") };

    wxColour m_colour_rust { wxColour("#FF00FF") };
    wxColour m_colour_rust_comment { wxColour("#008000") };
    wxColour m_colour_rust_keyword { wxColour("#0000FF") };
    wxColour m_colour_rust_number { wxColour("#FF0000") };
    wxColour m_colour_rust_string { wxColour("#008000") };

    wxColour m_colour_xrc_attribute { wxColour("#FF00FF") };
    wxColour m_colour_xrc_dblstring { wxColour("#008000") };
    wxColour m_colour_xrc_tag { wxColour("#0000FF") };

    size_t m_dark_mode_pending { 0 };  // 0 = no change, 1 = dark_mode_on, 2 = dark_mode_off

    int m_cpp_line_length { 110 };
    int m_python_line_length { 90 };
    int m_ruby_line_length { 80 };
    int m_fortran_line_length { 100 };
    int m_haskell_line_length { 80 };
    int m_lua_line_length { 100 };
    int m_perl_line_length { 80 };
    int m_rust_line_length { 100 };

    int m_icon_size { 20 };

    bool m_sizers_all_borders { true };
    bool m_sizers_always_expand { true };
    bool m_var_prefix { true };  // true to use "m_" prefix for member variables

    bool m_dark_mode { false };
    bool m_high_constrast { false };

    bool m_fullpath_title { false };  // If true, the full path to the project is displayed in the title bar

    bool m_svg_images { false };  // If true, SVG is the default image type

    bool m_enable_wakatime { true };
    bool m_is_load_last_project { false };
    bool m_is_right_propgrid { false };
    bool m_is_cpp_snake_case { true };
    bool m_add_comments { false };
};

extern Prefs& UserPrefs;

// These are utiltiy functions for converting colors

void wxColourToHSL(const wxColour& colour, double& hue, double& saturation, double& luminance);
wxColour HSLToWxColour(double hue, double saturation, double luminance);
wxColour wxColourToDarkForeground(const wxColour& colour);
wxColour wxColourToDarkBackground(const wxColour& colour);
