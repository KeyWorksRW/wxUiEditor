/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code::Gen() functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <charconv>  // for std::to_chars()

#include "code.h"

Code& Code::GenFont(GenEnum::PropName prop_name, tt_string_view font_function)
{
    FontProperty fontprop(m_node->getPropPtr(prop_name));
    if (fontprop.isDefGuiFont())
    {
        std::string font_var_name;
        if (is_perl())
        {
            font_var_name = "$font";
        }
        else
        {
            font_var_name = "font";
        }
        OpenFontBrace();
        if (is_cpp())
        {
            Add("wxFont font(");
        }
        else
        {
            AddIfPerl("my ").Str(font_var_name).CreateClass(false, "wxFont");
        }
        Class("wxSystemSettings").ClassMethod("GetFont(").Add("wxSYS_DEFAULT_GUI_FONT").Str(")");
        EndFunction();

        if (fontprop.GetSymbolSize() != wxFONTSIZE_MEDIUM)
            Eol()
                .Str(font_var_name)
                .VariableMethod("SetSymbolicSize(")
                .Add(font_symbol_pairs.GetValue(fontprop.GetSymbolSize()))
                .EndFunction();
        if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
            Eol()
                .Str(font_var_name)
                .VariableMethod("SetStyle(")
                .Add(font_style_pairs.GetValue(fontprop.GetStyle()))
                .EndFunction();
        if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
            Eol()
                .Str(font_var_name)
                .VariableMethod("SetWeight(")
                .Add(font_weight_pairs.GetValue(fontprop.GetWeight()))
                .EndFunction();
        if (fontprop.IsUnderlined())
            Eol().Str(font_var_name).VariableMethod("SetUnderlined(").True().EndFunction();
        if (fontprop.IsStrikethrough())
            Eol().Str(font_var_name).VariableMethod("SetStrikethrough(").True().EndFunction();
        Eol();

        if (m_node->isForm())
        {
            if (m_node->isGen(gen_wxPropertySheetDialog))
            {
                FormFunction("GetBookCtrl()").Function("SetFont(").Str(font_var_name).EndFunction();
            }
            else
            {
                FormFunction("SetFont(").Str(font_var_name).EndFunction();
            }
            CloseFontBrace();
        }
        else if (m_node->isGen(gen_wxStyledTextCtrl))
        {
            NodeName().Function("StyleSetFont(").Add("wxSTC_STYLE_DEFAULT");
            Comma().Str(font_var_name).EndFunction();
            CloseFontBrace();
        }
        else
        {
            NodeName().Function(font_function).Str(font_var_name).EndFunction();
            CloseFontBrace();
        }
    }
    else  // not isDefGuiFont()
    {
        if (is_perl())
        {
            OpenFontBrace();
            Str("my $font = ");
            Class("wxFont").Function("new(");
            itoa(fontprop.GetPointSize()).Comma();
            Str(font_family_pairs.GetValue(fontprop.GetFamily())).Comma();
            Str(font_style_pairs.GetValue(fontprop.GetStyle())).Comma();
            Str(font_weight_pairs.GetValue(fontprop.GetWeight())).Comma();
            Str(fontprop.IsUnderlined() ? "1" : "0").Comma();
            QuotedString(fontprop.GetFaceName().utf8_string()).Str(");");
        }
        else
        {
            bool more_than_pointsize =
                ((fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default") ||
                 fontprop.GetFamily() != wxFONTFAMILY_DEFAULT || fontprop.GetStyle() != wxFONTSTYLE_NORMAL ||
                 fontprop.GetWeight() != wxFONTWEIGHT_NORMAL || fontprop.IsUnderlined() || fontprop.IsStrikethrough());

            const auto point_size = fontprop.GetFractionalPointSize();
            if (is_cpp())
            {
                OpenFontBrace();
                Str("wxFontInfo font_info(");
            }
            else
            {
                Eol(eol_if_needed);
                if (is_perl())
                {
                    *this += "my $";
                }
                Add("font_info").CreateClass(false, "wxFontInfo");
            }

            if (point_size != static_cast<int>(point_size))  // is there a fractional value?
            {
                std::array<char, 10> float_str;
                if (auto [ptr, ec] = std::to_chars(float_str.data(), float_str.data() + float_str.size(), point_size);
                    ec == std::errc())
                {
                    Str(std::string_view(float_str.data(), ptr - float_str.data())).EndFunction();
                }
            }
            else
            {
                if (point_size <= 0)
                {
                    Class("wxSystemSettings").ClassMethod("GetFont(").Add("wxSYS_DEFAULT_GUI_FONT").Str(")");
                    VariableMethod("GetPointSize()").EndFunction();
                    if (!is_cpp() && more_than_pointsize)
                    {
                        Eol().Str("font_info");
                    }
                }
                else
                {
                    // GetPointSize() will round the result rather than truncating the decimal
                    itoa(fontprop.GetPointSize()).EndFunction();
                }
            }

            if (is_cpp())
            {
                Eol();
                if (more_than_pointsize)
                {
                    Str("font_info");
                }
            }

#if defined(_WIN32)
            // REVIEW: [Randalphwa - 04-18-2025] Currently, wxPerl does support wxFontInfo, but leave this code
            // in case it is added later.
            if (is_perl())
            {
                if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
                {
                    Eol().Str("$font_info->").Str("FaceName = ");
                    QuotedString(tt_string() << fontprop.GetFaceName().utf8_string()) += ";";
                }
                if (fontprop.GetFamily() != wxFONTFAMILY_DEFAULT)
                {
                    Eol().Str("$font_info->").Str("Family = ");
                    Add(font_family_pairs.GetValue(fontprop.GetFamily())) += ";";
                }
                if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
                {
                    Eol().Str("$font_info->").Str("Style = ");
                    Add(font_style_pairs.GetValue(fontprop.GetStyle())) += ";";
                }
            }
#endif
            else
            {
                if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
                    VariableMethod("FaceName(").QuotedString(tt_string() << fontprop.GetFaceName().utf8_string()) += ")";
                if (fontprop.GetFamily() != wxFONTFAMILY_DEFAULT)
                    VariableMethod("Family(").Add(font_family_pairs.GetValue(fontprop.GetFamily())) += ")";
                if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
                    VariableMethod("Style(").Add(font_style_pairs.GetValue(fontprop.GetStyle())) += ")";
                if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
                {
                    VariableMethod("Weight(").Add(font_weight_pairs.GetValue(fontprop.GetWeight())) += ")";
                }
                if (fontprop.IsUnderlined())
                    VariableMethod("Underlined()");
                if (fontprop.IsStrikethrough())
                    VariableMethod("Strikethrough()");
            }
            if (back() == '.')
            {
                pop_back();
            }
            if (is_cpp())
            {
                while (back() == '\t')
                    pop_back();
                if (back() != '\n')
                    *this += ';';
            }
        }
        Eol(eol_if_needed);

        if (m_node->isForm())
        {
            if (m_node->isGen(gen_wxPropertySheetDialog))
            {
                if (!is_perl())
                {
                    FormFunction("GetBookCtrl()")
                        .Function(font_function)
                        .Object("wxFont")
                        .Str("font_info")
                        .Str(")")
                        .EndFunction();
                }
                else
                {
                    FormFunction("GetBookCtrl()").Function(font_function).Str("$font").EndFunction();
                }
            }
            else
            {
                if (!is_perl())
                {
                    FormFunction(font_function).Object("wxFont").VarName("font_info", false).Str(")").EndFunction();
                }
                else
                {
                    FormFunction(font_function).Str("$font").EndFunction();
                }
            }
        }
        else
        {
            if (!is_perl())
            {
                NodeName().Function(font_function).Object("wxFont").VarName("font_info", false).Str(")").EndFunction();
            }
            else
            {
                // wxPerl doesn't support wxFontInfo, so use the font creation generated above
                NodeName().Function(font_function).Str("$font").EndFunction();
            }
        }
        CloseFontBrace();
    }

    return *this;
}

void Code::GenFontColourSettings()
{
    auto* node = m_node;
    if (hasValue(prop_font))
    {
        GenFont();
    }

    if (auto& fg_clr = node->as_string(prop_foreground_colour); fg_clr.size())
    {
        Eol(eol_if_needed);
        if (node->isForm())
        {
            if (m_node->isGen(gen_wxPropertySheetDialog))
            {
                FormFunction("GetBookCtrl()").Function("SetForegroundColour(");
            }
            else
            {
                FormFunction("SetForegroundColour(");
            }
        }
        else
        {
            NodeName().Function("SetForegroundColour(");
        }
        if (fg_clr.contains("wx"))
        {
            Class("wxSystemSettings").ClassMethod("GetColour(").Add(fg_clr) += ")";
        }
        else
        {
            if (fg_clr.starts_with('#'))
            {
                Object("wxColour").QuotedString(fg_clr) += ')';
            }
            else
            {
                // This handles older project versions, and hand-edited project files
                const auto colour = m_node->as_wxColour(prop_foreground_colour);
                Object("wxColour").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
            }
        }
        EndFunction();
    }

    if (auto& bg_clr = node->as_string(prop_background_colour); bg_clr.size())
    {
        Eol(eol_if_needed);
        if (node->isForm())
        {
            FormFunction("SetBackgroundColour(");
        }
        else
        {
            NodeName().Function("SetBackgroundColour(");
        }
        if (bg_clr.contains("wx"))
        {
            Class("wxSystemSettings").ClassMethod("GetColour(").Add(bg_clr) += ")";
        }
        else
        {
            if (bg_clr.starts_with('#'))
            {
#if GENERATE_NEW_LANG_CODE
                if (is_lua())
                {
                    // Lua 3.2 doesn't allow passing a string to SetBackgroundColour
                    Class("wxColour(").QuotedString(bg_clr) += ')';
                }
                else
#endif
                {
                    Object("wxColour").QuotedString(bg_clr) += ')';
                }
            }
            else
            {
                // This handles older project versions, and hand-edited project files
                const auto colour = m_node->as_wxColour(prop_background_colour);
#if GENERATE_NEW_LANG_CODE
                if (is_lua())
                {
                    // Lua 3.2 doesn't allow passing a string to SetBackgroundColour
                    Class("wxColour(").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
                }
                else
#endif
                {
                    Object("wxColour").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
                }
            }
        }

        EndFunction();

        // For background color, set both the background of the dialog and the background of the book control
        if (m_node->isGen(gen_wxPropertySheetDialog))
        {
            FormFunction("GetBookCtrl()").Function("SetBackgroundColour(");
            if (bg_clr.contains("wx"))
            {
                Class("wxSystemSettings").ClassMethod("GetColour(").Add(bg_clr) += ")";
            }
            else
            {
                if (bg_clr.starts_with('#'))
                {
                    if (is_perl())
                    {
                        Class("wxColour->new(").QuotedString(bg_clr) += ")";
                    }
                    else
                    {
                        Object("wxColour").QuotedString(bg_clr) += ')';
                    }
                }
                else
                {
                    // This handles older project versions, and hand-edited project files
                    const auto colour = m_node->as_wxColour(prop_background_colour);
                    Object("wxColour").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
                }
            }

            EndFunction();
        }
    }
}

Code& Code::GenSizerFlags()
{
    if (is_perl())
    {
        // Perl doesn't have a wxSizerFlags() function, so we have to use the old wxSizer::Add() function.
        Add(m_node->as_string(prop_proportion)).Comma();

        tt_string prop_combined_flags;
        auto lambda = [&](GenEnum::PropName prop_name) -> void
        {
            if (auto& prop = m_node->as_string(prop_name); prop.size())
            {
                tt_string_vector vector(prop, "|", tt::TRIM::both);
                for (auto& iter: vector)
                {
                    if (prop_combined_flags.size())
                        prop_combined_flags << '|';
                    prop_combined_flags << iter;
                }
            }
        };
        lambda(prop_alignment);
        lambda(prop_flags);
        lambda(prop_borders);

        if (prop_combined_flags.size())
        {
            Add(prop_combined_flags);
        }
        else
        {
            Add("0");
        }
        Comma().as_string(prop_border_size);

        return *this;
    }
    // wxSizerFlags functions are chained together, so we don't want to break them. Instead,
    // shut off auto_break and then restore it when we are done, after which we can check whether
    // or note the entire wxSizerFlags() statement needs to be broken.

    bool save_auto_break = m_auto_break;
    m_auto_break = false;
    auto cur_pos = size();

    Add("wxSizerFlags");
    if (is_ruby())
    {
        Add(".new");
    }

    if (auto& prop = m_node->as_string(prop_proportion); prop != "0")
    {
        *this << '(' << prop << ')';
    }
    else if (!is_ruby())  // Don't use empty () for Ruby
    {
        *this << "()";
    }

    if (auto& prop = m_node->as_string(prop_alignment); prop.size())
    {
        if (prop.contains("wxALIGN_CENTER_HORIZONTAL") &&
            (m_node->getParent()->isGen(gen_wxGridSizer) || m_node->getParent()->isGen(gen_wxFlexGridSizer) ||
             m_node->getParent()->isGen(gen_wxGridBagSizer)))
        {
            SizerFlagsFunction("CenterHorizontal") += ')';
        }
        else if (prop.contains("wxALIGN_CENTER_VERTICAL") &&
                 (m_node->getParent()->isGen(gen_wxGridSizer) || m_node->getParent()->isGen(gen_wxFlexGridSizer) ||
                  m_node->getParent()->isGen(gen_wxGridBagSizer)))

        {
            SizerFlagsFunction("CenterVertical") += ')';
        }
        else if (prop.contains("wxALIGN_CENTER"))
        {
            // Note that CenterHorizontal() and CenterVertical() generate an assert if you try to
            // use one that is invalid if the sizer parent's orientation doesn't support it.
            // Center() just works without the assertion check.
            SizerFlagsFunction("Center") += ')';
        }

        if (prop.contains("wxALIGN_LEFT"))
        {
            SizerFlagsFunction("Left") += ')';
        }
        else if (prop.contains("wxALIGN_RIGHT"))
        {
            SizerFlagsFunction("Right") += ')';
        }

        if (prop.contains("wxALIGN_TOP"))
        {
            SizerFlagsFunction("Top") += ')';
        }
        else if (prop.contains("wxALIGN_BOTTOM"))
        {
            SizerFlagsFunction("Bottom") += ')';
        }
        if (is_ruby())
        {
            // Ruby style guidelines are to eliminate empty parenthesis
            pop_back();
            pop_back();
        }
    }

    if (auto& prop = m_node->as_string(prop_flags); prop.size())
    {
        if (prop.contains("wxEXPAND"))
        {
            SizerFlagsFunction("Expand") += ')';
        }
        if (prop.contains("wxSHAPED"))
        {
            SizerFlagsFunction("Shaped") += ')';
        }
        if (prop.contains("wxFIXED_MINSIZE"))
        {
            SizerFlagsFunction("FixedMinSize") += ')';
        }
        if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
        {
            SizerFlagsFunction("ReserveSpaceEvenIfHidden") += ')';
        }
        if (is_ruby())
        {
            // Ruby style guidelines are to eliminate empty parenthesis
            pop_back();
            pop_back();
        }
    }

    if (auto& prop = m_node->as_string(prop_borders); prop.size())
    {
        auto border_size = m_node->as_int(prop_border_size);
        if (prop.contains("wxALL"))
        {
            if (border_size == 5)
                SizerFlagsFunction("Border").Add("wxALL").Str(")");
            else if (border_size == 10)
                SizerFlagsFunction("DoubleBorder").Add("wxALL").Str(")");
            else if (border_size == 15)
                SizerFlagsFunction("TripleBorder").Add("wxALL").Str(")");
            else
            {
                SizerFlagsFunction("Border").Add("wxALL").Comma();
                BorderSize() += ')';
            }
        }
        else
        {
            SizerFlagsFunction("Border");
            tt_string border_flags;

            if (prop.contains("wxLEFT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "LEFT";
            }
            if (prop.contains("wxRIGHT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "RIGHT";
            }
            if (prop.contains("wxTOP"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "TOP";
            }
            if (prop.contains("wxBOTTOM"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "BOTTOM";
            }
            if (border_flags.empty())
                border_flags = "0";

            *this << border_flags << ", ";
            if (border_size == 5)
            {
                if (is_cpp())
                    *this += "wxSizerFlags::GetDefaultBorder())";
                else if (is_ruby())
                    *this += "Wx::SizerFlags.get_default_border)";
                else
                    *this << m_language_wxPrefix << "SizerFlags.GetDefaultBorder())";
            }
            else
            {
                BorderSize() += ')';
            }
        }
    }

    m_auto_break = save_auto_break;

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

void Code::GenWindowSettings()
{
    if (hasValue(prop_window_extra_style))
    {
        if (m_node->isForm())
        {
            FormFunction("SetExtraStyle(");
        }
        else
        {
            NodeName().Function("SetExtraStyle(");
        }

        Add(prop_window_extra_style).EndFunction();
    }

    if (IsTrue(prop_disabled))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("Enable(").False().EndFunction();
        }
        else
        {
            FormFunction("Enable(").False().EndFunction();
        }
    }

    if (IsTrue(prop_hidden))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("Hide(").EndFunction();
        }
        else
        {
            FormFunction("Hide(").EndFunction();
        }
        if (is_ruby())
        {
            // Ruby style guidelines are to eliminate empty parenthesis
            pop_back();
            pop_back();
        }
    }

    bool allow_minmax { true };
    if (m_node->isForm() && !m_node->isGen(gen_PanelForm) && !m_node->isGen(gen_wxToolBar))
        allow_minmax = false;

    if (allow_minmax && m_node->as_wxSize(prop_minimum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("SetMinSize(");
        }
        else
        {
            FormFunction("SetMinSize(");
        }
        WxSize(prop_minimum_size).EndFunction();
    }

    if (allow_minmax && m_node->as_wxSize(prop_maximum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("SetMaxSize(");
        }
        else
        {
            FormFunction("SetMaxSize(");
        }
        WxSize(prop_maximum_size).EndFunction();
    }

    if (!m_node->isForm() && !m_node->isPropValue(prop_variant, "normal"))
    {
        Eol(eol_if_empty).NodeName().Function("SetWindowVariant(");
        if (m_node->isPropValue(prop_variant, "small"))
            Add("wxWINDOW_VARIANT_SMALL");
        else if (m_node->isPropValue(prop_variant, "mini"))
            Add("wxWINDOW_VARIANT_MINI");
        else
            Add("wxWINDOW_VARIANT_LARGE");

        EndFunction();
    }

    // wxAuiNotebook uses page tooltips for the tabs, so it should be ignored when generating
    // the page cade.
    if (hasValue(prop_tooltip) && !m_node->getParent()->isGen(gen_wxAuiNotebook))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("SetToolTip(");
        }
        else
        {
            FormFunction("SetToolTip(");
        }
        QuotedString(prop_tooltip).EndFunction();
    }

    if (hasValue(prop_context_help))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
            NodeName().Function("SetHelpText(");
        else
            FormFunction("SetHelpText(");
        QuotedString(prop_context_help).EndFunction();
    }

    GenFontColourSettings();
}
