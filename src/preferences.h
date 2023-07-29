/////////////////////////////////////////////////////////////////////////////
// Purpose:   Set/Get wxUiEditor preferences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
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
    void set_DarkMode(bool value) { m_dark_mode = value; }

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

    // Use this string to construct a FontProperty() to get the values
    const tt_string& get_CodeDisplayFont() const { return m_code_display_font; }

    // This should be the string returned from FontProperty::as_string()
    void set_CodeDisplayFont(const tt_string& font) { m_code_display_font = font; }

    const tt_string& get_CppWidgetsVersion() const { return m_cpp_widgets_version; }
    void set_CppWidgetsVersion(const tt_string& version) { m_cpp_widgets_version = version; }

    const wxColour& get_CppColour() const { return m_colour_cpp; }
    void set_CppColour(const wxColour& colour) { m_colour_cpp = colour; }

    const wxColour& get_PythonColour() const { return m_colour_python; }
    void set_PythonColour(const wxColour& colour) { m_colour_python = colour; }

    const wxColour& get_RubyColour() const { return m_colour_ruby; }
    void set_RubyColour(const wxColour& colour) { m_colour_ruby = colour; }

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

    // clang-format on

private:
    // These store both Debug and INTERNAL flags
    long m_flags { 0 };

    long m_project_flags { 0 };
    PREVIEW_TYPE m_preview_type { PREVIEW_TYPE_XRC };

    tt_string m_cpp_widgets_version { "3.2" };
    tt_string m_code_display_font { "modern,10" };

    wxColour m_colour_cpp { wxColour("#FF00FF") };
    wxColour m_colour_python { wxColour("#FF00FF") };
    wxColour m_colour_ruby { wxColour("#FF00FF") };

    int m_cpp_line_length { 110 };
    int m_python_line_length { 90 };
    int m_ruby_line_length { 80 };

    bool m_sizers_all_borders { true };
    bool m_sizers_always_expand { true };
    bool m_var_prefix { true };  // true to use "m_" prefix for member variables

    bool m_enable_wakatime { true };
    bool m_dark_mode { false };
    bool m_is_load_last_project { false };
    bool m_is_right_propgrid { false };
    bool m_is_cpp_snake_case { true };
};


extern Prefs& UserPrefs;
