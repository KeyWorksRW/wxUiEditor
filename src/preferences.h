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
    Prefs(Prefs&&) = delete;
    auto operator=(Prefs&&) -> Prefs& = delete;
    Prefs(Prefs const&) = delete;

    void operator=(Prefs const&) = delete;

    static Prefs& getInstance()
    {
        static Prefs instance;
        return instance;
    }

    enum class PREVIEW_TYPE : std::uint8_t
    {
        xrc = 0,
        both,
        cpp,
    };

    void ReadConfig();
    void WriteConfig();
    // Add borders around all new sizers
    auto is_SizersAllBorders() const -> bool { return m_sizers_all_borders; }

    // Add expand flag to all new sizers
    auto is_SizersExpand() const -> bool { return m_sizers_always_expand; }

    // Enable WakaTime support
    auto is_WakaTimeEnabled() const -> bool { return m_enable_wakatime; }

    auto is_DarkMode() const -> bool { return m_dark_mode; }
    auto is_DarkModePending() const -> bool { return m_dark_mode_pending; }
    void set_DarkMode(bool value) { m_dark_mode = value; }
    void set_DarkModePending(size_t value) { m_dark_mode_pending = value; }
    // Only true if both dark mode and high contrast are enabled
    auto is_HighContrast() const -> bool { return (m_dark_mode && m_high_contrast); }
    void set_HighContrast(bool value) { m_high_contrast = value; }

    auto is_FullPathTitle() const -> bool { return m_fullpath_title; }
    void set_FullPathTitle(bool value) { m_fullpath_title = value; }

    auto is_AddComments() const -> bool { return m_add_comments; }

    auto is_SvgImages() const -> bool { return m_svg_images; }
    void set_SvgImages(bool value) { m_svg_images = value; }

    auto is_LoadLastProject() const -> bool { return m_is_load_last_project; }
    void set_LoadLastProject(bool value) { m_is_load_last_project = value; }

    auto is_RightPropGrid() const -> bool { return m_is_right_propgrid; }
    void set_RightPropGrid(bool value) { m_is_right_propgrid = value; }

    auto is_CppSnakeCase() const -> bool { return m_is_cpp_snake_case; }
    void set_CppSnakeCase(bool value) { m_is_cpp_snake_case = value; }

    auto get_CppLineLength() const -> int { return m_cpp_line_length; }
    void set_CppLineLength(int length) { m_cpp_line_length = length; }

    auto get_PythonLineLength() const -> int { return m_python_line_length; }
    void set_PythonLineLength(int length) { m_python_line_length = length; }

    auto get_RubyLineLength() const -> int { return m_ruby_line_length; }
    void set_RubyLineLength(int length) { m_ruby_line_length = length; }

    auto get_PerlLineLength() const -> int { return m_perl_line_length; }
    void set_PerlLineLength(int length) { m_perl_line_length = length; }

    auto get_IconSize() const -> int { return m_icon_size; }
    void set_IconSize(int size) { m_icon_size = size; }

    // Use this string to construct a FontProperty() to get the values
    auto get_CodeDisplayFont() const -> const wxue::string& { return m_code_display_font; }

    // This should be the string returned from FontProperty::as_string()
    void set_CodeDisplayFont(const wxue::string& font) { m_code_display_font = font; }

    auto get_CppWidgetsVersion() const -> const wxue::string& { return m_cpp_widgets_version; }
    void set_CppWidgetsVersion(const wxue::string& version) { m_cpp_widgets_version = version; }
    auto get_PythonVersion() const -> const wxue::string& { return m_python_version; }
    void set_PythonVersion(const wxue::string& version) { m_python_version = version; }
    auto get_RubyVersion() const -> const wxue::string& { return m_ruby_version; }
    void set_RubyVersion(const wxue::string& version) { m_ruby_version = version; }

    auto get_CppColour() const -> const wxColour& { return m_colour_cpp; }  // wxWidgets keywords
    void set_CppColour(const wxColour& colour) { m_colour_cpp = colour; }
    auto get_CppKeywordColour() const -> const wxColour&
    {
        return m_colour_cpp_keyword;
    }  // C++ keywords
    void set_CppKeywordColour(const wxColour& colour) { m_colour_cpp_keyword = colour; }
    auto get_CppCommentColour() const -> const wxColour& { return m_colour_cpp_comment; }
    void set_CppCommentColour(const wxColour& colour) { m_colour_cpp_comment = colour; }
    auto get_CppNumberColour() const -> const wxColour& { return m_colour_cpp_number; }
    void set_CppNumberColour(const wxColour& colour) { m_colour_cpp_number = colour; }
    auto get_CppStringColour() const -> const wxColour& { return m_colour_cpp_string; }
    void set_CppStringColour(const wxColour& colour) { m_colour_cpp_string = colour; }

    auto get_PythonColour() const -> const wxColour& { return m_colour_python; }
    void set_PythonColour(const wxColour& colour) { m_colour_python = colour; }
    auto get_PythonKeywordColour() const -> const wxColour& { return m_colour_python_keyword; }
    void set_PythonKeywordColour(const wxColour& colour) { m_colour_python_keyword = colour; }
    auto get_PythonNumberColour() const -> const wxColour& { return m_colour_python_number; }
    void set_PythonNumberColour(const wxColour& colour) { m_colour_python_number = colour; }
    auto get_PythonStringColour() const -> const wxColour& { return m_colour_python_string; }
    void set_PythonStringColour(const wxColour& colour) { m_colour_python_string = colour; }
    auto get_PythonCommentColour() const -> const wxColour& { return m_colour_python_comment; }
    void set_PythonCommentColour(const wxColour& colour) { m_colour_python_comment = colour; }

    auto get_RubyColour() const -> const wxColour& { return m_colour_ruby; }
    void set_RubyColour(const wxColour& colour) { m_colour_ruby = colour; }
    auto get_RubyCommentColour() const -> const wxColour& { return m_colour_ruby_comment; }
    void set_RubyCommentColour(const wxColour& colour) { m_colour_ruby_comment = colour; }
    auto get_RubyNumberColour() const -> const wxColour& { return m_colour_ruby_number; }
    void set_RubyNumberColour(const wxColour& colour) { m_colour_ruby_number = colour; }
    auto get_RubyStringColour() const -> const wxColour& { return m_colour_ruby_string; }
    void set_RubyStringColour(const wxColour& colour) { m_colour_ruby_string = colour; }

    auto get_PerlColour() const -> const wxColour& { return m_colour_perl; }
    void set_PerlColour(const wxColour& colour) { m_colour_perl = colour; }
    auto get_PerlCommentColour() const -> const wxColour& { return m_colour_perl_comment; }
    void set_PerlCommentColour(const wxColour& colour) { m_colour_perl_comment = colour; }
    auto get_PerlKeywordColour() const -> const wxColour& { return m_colour_perl_keyword; }
    void set_PerlKeywordColour(const wxColour& colour) { m_colour_perl_keyword = colour; }
    auto get_PerlNumberColour() const -> const wxColour& { return m_colour_perl_number; }
    void set_PerlNumberColour(const wxColour& colour) { m_colour_perl_number = colour; }
    auto get_PerlStringColour() const -> const wxColour& { return m_colour_perl_string; }
    void set_PerlStringColour(const wxColour& colour) { m_colour_perl_string = colour; }

    auto get_XrcAttributeColour() const -> const wxColour& { return m_colour_xrc_attribute; }
    void set_XrcAttributeColour(const wxColour& colour) { m_colour_xrc_attribute = colour; }
    auto get_XrcDblStringColour() const -> const wxColour& { return m_colour_xrc_dblstring; }
    void set_XrcDblStringColour(const wxColour& colour) { m_colour_xrc_dblstring = colour; }
    auto get_XrcTagColour() const -> const wxColour& { return m_colour_xrc_tag; }
    void set_XrcTagColour(const wxColour& colour) { m_colour_xrc_tag = colour; }

    void set_SizersAllBorders(bool setting) { m_sizers_all_borders = setting; }
    void set_SizersExpand(bool setting) { m_sizers_always_expand = setting; }
    void set_VarPrefix(bool setting) { m_var_prefix = setting; }
    void set_WakaTimeEnabled(bool setting) { m_enable_wakatime = setting; }

    auto GetDebugFlags() const -> long { return m_flags; }
    void SetDebugFlags(long flags) { m_flags = flags; }

    auto GetProjectFlags() const -> long { return m_project_flags; }
    void SetProjectFlags(long flags) { m_project_flags = flags; }

    // The following are used by the OptionsDlg class to efficiently update the preferences

    auto RefVarPrefix() -> bool& { return m_var_prefix; }
    auto RefSizersAllBorders() -> bool& { return m_sizers_all_borders; }
    auto RefSizersExpand() -> bool& { return m_sizers_always_expand; }
    auto RefWakaTimeEnabled() -> bool& { return m_enable_wakatime; }

    auto GetPreviewType() const -> PREVIEW_TYPE { return m_preview_type; }
    void SetPreviewType(PREVIEW_TYPE type) { m_preview_type = type; }

    // The returned colour will depend on whether dark mode (and high contrast) is enabled or
    // not.
    auto GetColour(wxSystemColour index) -> wxColour;

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

    enum
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
    PREVIEW_TYPE m_preview_type { PREVIEW_TYPE::xrc };

    wxue::string m_cpp_widgets_version { "3.2" };
    wxue::string m_python_version { "4.2" };
    wxue::string m_ruby_version { "1.2" };
    wxue::string m_perl_version { "3.2" };

    wxue::string m_code_display_font;

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

    wxColour m_colour_perl { wxColour("#FF00FF") };
    wxColour m_colour_perl_comment { wxColour("#008000") };
    wxColour m_colour_perl_keyword { wxColour("#0000FF") };
    wxColour m_colour_perl_number { wxColour("#FF0000") };
    wxColour m_colour_perl_string { wxColour("#008000") };

    wxColour m_colour_xrc_attribute { wxColour("#FF00FF") };
    wxColour m_colour_xrc_dblstring { wxColour("#008000") };
    wxColour m_colour_xrc_tag { wxColour("#0000FF") };

    size_t m_dark_mode_pending { 0 };  // 0 = no change, 1 = dark_mode_on, 2 = dark_mode_off

    int m_cpp_line_length { 110 };
    int m_python_line_length { 90 };
    int m_ruby_line_length { 80 };
    int m_perl_line_length { 80 };

    int m_icon_size { 20 };

    bool m_sizers_all_borders { true };
    bool m_sizers_always_expand { true };
    bool m_var_prefix { true };  // true to use "m_" prefix for member variables

    bool m_dark_mode { false };
    bool m_high_contrast { false };

    bool m_fullpath_title {
        false
    };  // If true, the full path to the project is displayed in the title bar

    bool m_svg_images { false };  // If true, SVG is the default image type

    bool m_enable_wakatime { true };
    bool m_is_load_last_project { false };
    bool m_is_right_propgrid { false };
    bool m_is_cpp_snake_case { true };
    bool m_add_comments { false };
};

extern Prefs& UserPrefs;

// These are utility functions for converting colors

void wxColourToHSL(const wxColour& colour, double& hue, double& saturation, double& luminance);
auto HSLToWxColour(double hue, double saturation, double luminance) -> wxColour;
auto wxColourToDarkForeground(const wxColour& colour) -> wxColour;
auto wxColourToDarkBackground(const wxColour& colour) -> wxColour;
