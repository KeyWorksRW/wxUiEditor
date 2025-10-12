/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code::Pos()/Style() functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "code.h"

#include "mainframe.h"       // MainFrame class
#include "tt_view_vector.h"  // tt_view_vector -- Read/Write line-oriented strings/files

Code& Code::Pos(GenEnum::PropName prop_name, int enable_dpi_scaling)
{
    auto cur_pos = size();
    auto point = m_node->as_wxPoint(prop_name);
    auto pos_scaling = is_ScalingEnabled(prop_pos, enable_dpi_scaling);

    if (m_node->as_string(prop_name).contains("d", tt::CASE::either))
    {
        FAIL_MSG("Pos() should not be used with a string that contains 'd'");
        point = wxGetMainFrame()->getWindow()->ConvertDialogToPixels(point);
    }

    if (is_ruby())
    {
        if (m_node->as_wxPoint(prop_name) == wxDefaultPosition)
        {
            CheckLineLength((sizeof("pos=Wx::DEFAULT_POSITION") - 1));
            *this += "Wx::DEFAULT_POSITION";
            return *this;
        }

        if (pos_scaling)
        {
            CheckLineLength(sizeof(", from_DIP(Wx::Point.new(999, 999))"));
            FormFunction("FromDIP(");
            Class("Wx::Point.new(").itoa(point.x).Comma().itoa(point.y) << ')';
            *this += ')';
        }
        else
        {
            CheckLineLength(sizeof("Wx::Point.new(999, 999)"));
            Class("Wx::Point.new(").itoa(point.x).Comma().itoa(point.y) << ')';
        }

        if (m_auto_break && this->size() > m_break_at)
        {
            InsertLineBreak(cur_pos);
        }

        return *this;
    }

    // The following code is for non-Ruby languages

    if (m_node->as_wxPoint(prop_name) == wxDefaultPosition)
    {
        CheckLineLength((sizeof("DefaultPosition") - 1) + m_language_wxPrefix.size());
        if (is_perl())
        {
            *this << "wxDefaultPosition";
        }
        else
        {
            *this << m_language_wxPrefix << "DefaultPosition";
        }
        return *this;
    }

    if (pos_scaling)
    {
        if (is_cpp())
        {
            CheckLineLength(sizeof("FromDIP(wxPoint(999, 999))"));
        }
        else if (is_python())
        {
            CheckLineLength(sizeof("self.FromDIP(wxPoint(999, 999))"));
        }
        FormFunction("FromDIP(");
        Class("wxPoint(").itoa(point.x).Comma().itoa(point.y) << ')';
        *this += ')';
    }
    else
    {
        CheckLineLength(sizeof("wxPoint(999, 999)"));
        Class("wxPoint(").itoa(point.x).Comma().itoa(point.y) << ')';
    }
    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

auto Code::PosSizeFlags(ScalingType enable_dpi_scaling, bool uses_def_validator,
                        tt_string_view def_style) -> Code&
{
    auto pos_scaling = is_ScalingEnabled(prop_pos, enable_dpi_scaling);
    auto size_scaling = is_ScalingEnabled(prop_size, enable_dpi_scaling);

    if (m_node->HasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be
        // generated.
        Comma();
        Pos(prop_pos, pos_scaling).Comma().WxSize(prop_size, size_scaling).Comma();
        Style();
        if (uses_def_validator)
        {
            Comma().Add("wxDefaultValidator");
        }
        Comma();
        if (is_ruby())
        {
            *this += "name=";
        }
        QuotedString(prop_window_name).EndFunction();
        return *this;
    }

    // This could be done as a single if statement, but it is easier to read this way.
    bool style_needed = false;
    if ((m_node->HasValue(prop_style) && m_node->as_string(prop_style) != def_style))
    {
        style_needed = true;
    }
    else if (m_node->HasValue(prop_window_style))
    {
        style_needed = true;
    }
    else if (m_node->HasValue(prop_orientation) &&
             !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
             !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
    {
        style_needed = true;
    }
    else if (m_node->HasValue(prop_tab_position) &&
             !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        style_needed = true;
    }
    else if (m_node->is_Gen(gen_wxRichTextCtrl) || m_node->is_Gen(gen_wxListView))
    {
        style_needed = true;
    }

    // Do we need a style and/or a default validator?
    if (style_needed)
    {
        Comma();
        Pos(prop_pos, pos_scaling).Comma().WxSize(prop_size, size_scaling).Comma().Style();
        if (def_style.size() && ends_with(def_style))
        {
            erase(size() - def_style.size());
            if (back() == '|')
            {
                pop_back();
            }
        }
    }
    else if (m_node->as_wxSize(prop_size) != wxDefaultSize)
    {
        Comma();
        Pos(prop_pos, pos_scaling).Comma().WxSize(prop_size, size_scaling);
    }
    else if (m_node->as_wxPoint(prop_pos) != wxDefaultPosition)
    {
        Comma();
        Pos(prop_pos, pos_scaling);
    }
    EndFunction();
    return *this;
}

Code& Code::PosSizeForceStyle(tt_string_view force_style, bool uses_def_validator)
{
    if (m_node->HasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be
        // generated.
        Comma();
        Pos().Comma().WxSize().Comma();
        Style(nullptr, force_style);
        if (uses_def_validator)
        {
            Comma().Add("wxDefaultValidator");
        }
        Comma();
        QuotedString(prop_window_name).EndFunction();
        return *this;
    }

    Comma();
    Pos().Comma().WxSize().Comma().Style(nullptr, force_style);

    EndFunction();
    return *this;
}

auto Code::Style(const char* prefix, tt_string_view force_style) -> Code&
{
    auto add_separator = [this](bool& style_set)
    {
        if (style_set)
        {
            *this += '|';
        }
    };

    bool style_set = false;

    if (force_style.size())
    {
        Add(force_style);
        style_set = true;
    }

    // Add tab position style
    if (m_node->HasValue(prop_tab_position) &&
        !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        add_separator(style_set);
        style_set = true;
        as_string(prop_tab_position);
    }

    // Add orientation style
    if (m_node->HasValue(prop_orientation) &&
        !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        add_separator(style_set);
        style_set = true;
        as_string(prop_orientation);
    }

    // Note that as_string() may break the line, so recalculate any time as_string() is called
    auto cur_pos = size();

    if (m_node->is_Gen(gen_wxRichTextCtrl))
    {
        add_separator(style_set);
        style_set = true;
        AddConstant("wxRE_MULTILINE");
    }

    // Process style property
    if (m_node->HasValue(prop_style))
    {
        add_separator(style_set);

        if (prefix)
        {
            if (is_cpp())
            {
                *this += m_node->as_constant(prop_style, prefix);
            }
            else
            {
                tt_view_vector multistr(m_node->as_constant(prop_style, prefix), "|",
                                        tt::TRIM::both);
                for (auto& iter: multistr)
                {
                    if (iter.empty())
                    {
                        continue;
                    }

                    add_separator(style_set);

                    if (iter.is_sameprefix("wx"))
                    {
                        if (std::string_view language_prefix = GetLanguagePrefix(iter, m_language);
                            language_prefix.size())
                        {
                            // Some languages will have a module added after their standard prefix.
                            CheckLineLength(language_prefix.size() + iter.size() - 2);
                            *this << language_prefix << iter.substr(2);
                        }
                        else
                        {
                            // If there was no sub-language module added (e.g., wx.aui. for
                            // Python), then use the default language prefix.
                            CheckLineLength(m_language_wxPrefix.size() + iter.size() - 2);
                            *this << m_language_wxPrefix << iter.substr(2);
                        }
                    }
                    else
                    {
                        *this += iter;
                    }
                    style_set = true;
                }
            }
        }
        else
        {
            AddConstant(m_node->as_string(prop_style));
            cur_pos = size();
        }
        style_set = true;
    }

    if (m_node->HasValue(prop_window_style))
    {
        add_separator(style_set);
        style_set = true;
        AddConstant(m_node->as_string(prop_window_style));
        cur_pos = size();
    }

    if (m_node->is_Gen(gen_wxListView))
    {
        add_separator(style_set);
        style_set = true;
        as_string(prop_mode);
        cur_pos = size();
    }

    if (!style_set)
    {
        *this += "0";
    }

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}
