/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code::Gen() functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <charconv>  // for std::to_chars()

#include "code.h"

#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

auto Code::GenFont(GenEnum::PropName prop_name, wxue::string_view font_function) -> Code&
{
    FontProperty fontprop(m_node->get_PropPtr(prop_name));
    if (fontprop.isDefGuiFont())
    {
        GenDefGuiFont(fontprop, font_function);
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
            GenFontInfoCode(fontprop);
        }
        Eol(eol_if_needed);
        ApplyFontToControl(font_function);
        CloseFontBrace();
    }

    return *this;
}

void Code::GenColourValue(wxue::string_view colour_str, GenEnum::PropName prop_name)
{
    if (colour_str.contains("wx"))
    {
        Class("wxSystemSettings").ClassMethod("GetColour(").Add(colour_str) += ")";
    }
    else if (colour_str.starts_with('#'))
    {
        if (is_perl() && prop_name == prop_background_colour &&
            m_node->is_Gen(gen_wxPropertySheetDialog))
        {
            Class("wxColour->new(").QuotedString(colour_str) += ")";
        }
        else
        {
            Object("wxColour").QuotedString(colour_str) += ')';
        }
    }
    else
    {
        // This handles older project versions, and hand-edited project files
        const auto colour = m_node->as_wxColour(prop_name);
        Object("wxColour").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
    }
}

void Code::GenSetColourFunction(wxue::string_view function_name, bool for_property_sheet)
{
    if (m_node->is_Form())
    {
        if (for_property_sheet)
        {
            FormFunction("GetBookCtrl()").Function(function_name);
        }
        else
        {
            FormFunction(function_name);
        }
    }
    else
    {
        NodeName().Function(function_name);
    }
}

void Code::GenFontColourSettings()
{
    auto* node = m_node;
    if (HasValue(prop_font))
    {
        GenFont();
    }

    if (const auto& fg_clr = node->as_string(prop_foreground_colour); fg_clr.size())
    {
        Eol(eol_if_needed);
        GenSetColourFunction("SetForegroundColour(", m_node->is_Gen(gen_wxPropertySheetDialog));
        GenColourValue(fg_clr, prop_foreground_colour);
        EndFunction();
    }

    if (const auto& bg_clr = node->as_string(prop_background_colour); bg_clr.size())
    {
        Eol(eol_if_needed);
        GenSetColourFunction("SetBackgroundColour(", false);
        GenColourValue(bg_clr, prop_background_colour);
        EndFunction();

        // For background color, set both the background of the dialog and the background of the
        // book control
        if (m_node->is_Gen(gen_wxPropertySheetDialog))
        {
            FormFunction("GetBookCtrl()").Function("SetBackgroundColour(");
            GenColourValue(bg_clr, prop_background_colour);
            EndFunction();
        }
    }
}

auto Code::GenSizerFlags() -> Code&
{
    if (is_perl())
    {
        // Perl doesn't have a wxSizerFlags() function, so we have to use the old wxSizer::Add()
        // function.
        Add(m_node->as_string(prop_proportion)).Comma();

        wxue::string prop_combined_flags;
        auto lambda = [&](GenEnum::PropName prop_name) -> void
        {
            if (const auto& prop = m_node->as_string(prop_name); prop.size())
            {
                wxue::StringVector vector(prop, "|", wxue::TRIM::both);
                for (auto& iter: vector)
                {
                    if (prop_combined_flags.size())
                    {
                        prop_combined_flags << '|';
                    }
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

    if (const auto& prop = m_node->as_string(prop_proportion); prop != "0")
    {
        *this << '(' << prop << ')';
    }
    else if (!is_ruby())  // Don't use empty () for Ruby
    {
        *this << "()";
    }

    if (const auto& prop = m_node->as_string(prop_alignment); prop.size())
    {
        ProcessAlignmentFlags(prop);
    }

    if (const auto& prop = m_node->as_string(prop_flags); prop.size())
    {
        ProcessSizerFlags(prop);
    }

    if (const auto& prop = m_node->as_string(prop_borders); prop.size())
    {
        ProcessBorderFlags(prop, m_node->as_int(prop_border_size));
    }

    m_auto_break = save_auto_break;

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

void Code::ProcessAlignmentFlags(const wxue::string& prop)
{
    if (prop.contains("wxALIGN_CENTER_HORIZONTAL") &&
        (m_node->get_Parent()->is_Gen(gen_wxGridSizer) ||
         m_node->get_Parent()->is_Gen(gen_wxFlexGridSizer) ||
         m_node->get_Parent()->is_Gen(gen_wxGridBagSizer)))
    {
        SizerFlagsFunction("CenterHorizontal") += ')';
    }
    else if (prop.contains("wxALIGN_CENTER_VERTICAL") &&
             (m_node->get_Parent()->is_Gen(gen_wxGridSizer) ||
              m_node->get_Parent()->is_Gen(gen_wxFlexGridSizer) ||
              m_node->get_Parent()->is_Gen(gen_wxGridBagSizer)))
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

void Code::ProcessSizerFlags(const wxue::string& prop)
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

void Code::ProcessBorderFlags(const wxue::string& prop, int border_size)
{
    if (prop.contains("wxALL"))
    {
        if (border_size == 5)
        {
            SizerFlagsFunction("Border").Add("wxALL").Str(")");
        }
        else if (border_size == 10)
        {
            SizerFlagsFunction("DoubleBorder").Add("wxALL").Str(")");
        }
        else if (border_size == 15)
        {
            SizerFlagsFunction("TripleBorder").Add("wxALL").Str(")");
        }
        else
        {
            SizerFlagsFunction("Border").Add("wxALL").Comma();
            BorderSize() += ')';
        }
    }
    else
    {
        SizerFlagsFunction("Border");
        std::string border_flags;

        if (prop.contains("wxLEFT"))
        {
            if (!border_flags.empty())
            {
                border_flags += '|';
            }
            border_flags += m_language_wxPrefix;
            border_flags += "LEFT";
        }
        if (prop.contains("wxRIGHT"))
        {
            if (!border_flags.empty())
            {
                border_flags += '|';
            }
            border_flags += m_language_wxPrefix;
            border_flags += "RIGHT";
        }
        if (prop.contains("wxTOP"))
        {
            if (!border_flags.empty())
            {
                border_flags += '|';
            }
            border_flags += m_language_wxPrefix;
            border_flags += "TOP";
        }
        if (prop.contains("wxBOTTOM"))
        {
            if (!border_flags.empty())
            {
                border_flags += '|';
            }
            border_flags += m_language_wxPrefix;
            border_flags += "BOTTOM";
        }
        if (border_flags.empty())
        {
            border_flags = "0";
        }

        *this << border_flags << ", ";
        if (border_size == 5)
        {
            if (is_cpp())
            {
                *this += "wxSizerFlags::GetDefaultBorder())";
            }
            else if (is_ruby())
            {
                *this += "Wx::SizerFlags.get_default_border)";
            }
            else
            {
                *this << m_language_wxPrefix << "SizerFlags.GetDefaultBorder())";
            }
        }
        else
        {
            BorderSize() += ')';
        }
    }
}

void Code::GenWindowSettings()
{
    GenExtraStyle();
    GenDisabledState();
    GenHiddenState();
    GenMinMaxSize();
    GenWindowVariant();
    GenTooltipAndHelp();
    GenFontColourSettings();
}

void Code::GenExtraStyle()
{
    if (HasValue(prop_window_extra_style))
    {
        CallNodeOrFormFunction("SetExtraStyle(");
        Add(prop_window_extra_style).EndFunction();
    }
}

void Code::GenDisabledState()
{
    if (IsTrue(prop_disabled))
    {
        Eol(eol_if_empty);
        CallNodeOrFormFunction("Enable(");
        False().EndFunction();
    }
}

void Code::GenHiddenState()
{
    if (IsTrue(prop_hidden))
    {
        Eol(eol_if_empty);
        CallNodeOrFormFunction("Hide(");
        EndFunction();
        if (is_ruby())
        {
            // Ruby style guidelines are to eliminate empty parenthesis
            pop_back();
            pop_back();
        }
    }
}

void Code::GenMinMaxSize()
{
    const bool allow_minmax =
        !m_node->is_Form() || m_node->is_Gen(gen_PanelForm) || m_node->is_Gen(gen_wxToolBar);

    if (allow_minmax && m_node->as_wxSize(prop_minimum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        CallNodeOrFormFunction("SetMinSize(");
        WxSize(prop_minimum_size).EndFunction();
    }

    if (allow_minmax && m_node->as_wxSize(prop_maximum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        CallNodeOrFormFunction("SetMaxSize(");
        WxSize(prop_maximum_size).EndFunction();
    }
}

void Code::GenWindowVariant()
{
    if (!m_node->is_Form() && !m_node->is_PropValue(prop_variant, "normal"))
    {
        Eol(eol_if_empty).NodeName().Function("SetWindowVariant(");
        if (m_node->is_PropValue(prop_variant, "small"))
        {
            Add("wxWINDOW_VARIANT_SMALL");
        }
        else if (m_node->is_PropValue(prop_variant, "mini"))
        {
            Add("wxWINDOW_VARIANT_MINI");
        }
        else
        {
            Add("wxWINDOW_VARIANT_LARGE");
        }
        EndFunction();
    }
}

void Code::GenTooltipAndHelp()
{
    // wxAuiNotebook uses page tooltips for the tabs, so it should be ignored when generating
    // the page code.
    if (HasValue(prop_tooltip) && !m_node->get_Parent()->is_Gen(gen_wxAuiNotebook))
    {
        Eol(eol_if_empty);
        CallNodeOrFormFunction("SetToolTip(");
        QuotedString(prop_tooltip).EndFunction();
    }

    if (HasValue(prop_context_help))
    {
        Eol(eol_if_empty);
        CallNodeOrFormFunction("SetHelpText(");
        QuotedString(prop_context_help).EndFunction();
    }
}

void Code::CallNodeOrFormFunction(wxue::string_view function_name)
{
    if (m_node->is_Form())
    {
        FormFunction(function_name);
    }
    else
    {
        NodeName().Function(function_name);
    }
}

void Code::GenDefGuiFont(const FontProperty& fontprop, wxue::string_view font_function)
{
    const auto* const font_var_name = is_perl() ? "$font" : "font";

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

    // Apply font property modifications
    if (fontprop.GetSymbolSize() != wxFONTSIZE_MEDIUM)
    {
        ApplyFontProperty(font_var_name, "SetSymbolicSize(",
                          font_symbol_pairs.GetValue(fontprop.GetSymbolSize()));
    }
    if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
    {
        ApplyFontProperty(font_var_name, "SetStyle(",
                          font_style_pairs.GetValue(fontprop.GetStyle()));
    }
    if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
    {
        ApplyFontProperty(font_var_name, "SetWeight(",
                          font_weight_pairs.GetValue(fontprop.GetWeight()));
    }
    if (fontprop.IsUnderlined())
    {
        Eol().Str(font_var_name).VariableMethod("SetUnderlined(").True().EndFunction();
    }
    if (fontprop.IsStrikethrough())
    {
        Eol().Str(font_var_name).VariableMethod("SetStrikethrough(").True().EndFunction();
    }

    Eol();
    SetFontOnControl(font_var_name, font_function);
}

void Code::ApplyFontProperty(const wxue::string& font_var_name, wxue::string_view method,
                             wxue::string_view value)
{
    Eol().Str(font_var_name).VariableMethod(method).Add(value).EndFunction();
}

void Code::SetFontOnControl(const wxue::string& font_var_name, wxue::string_view font_function)
{
    if (m_node->is_Form())
    {
        if (m_node->is_Gen(gen_wxPropertySheetDialog))
        {
            FormFunction("GetBookCtrl()").Function("SetFont(").Str(font_var_name).EndFunction();
        }
        else
        {
            FormFunction("SetFont(").Str(font_var_name).EndFunction();
        }
    }
    else if (m_node->is_Gen(gen_wxStyledTextCtrl))
    {
        NodeName().Function("StyleSetFont(").Add("wxSTC_STYLE_DEFAULT");
        Comma().Str(font_var_name).EndFunction();
    }
    else
    {
        NodeName().Function(font_function).Str(font_var_name).EndFunction();
    }
    CloseFontBrace();
}

void Code::GenFontInfoCode(const FontProperty& fontprop)
{
    const bool more_than_pointsize =
        ((fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default") ||
         fontprop.GetFamily() != wxFONTFAMILY_DEFAULT ||
         fontprop.GetStyle() != wxFONTSTYLE_NORMAL || fontprop.GetWeight() != wxFONTWEIGHT_NORMAL ||
         fontprop.IsUnderlined() || fontprop.IsStrikethrough());

    const auto point_size = fontprop.GetFractionalPointSize();

    GenFontInfoInit(fontprop, point_size, more_than_pointsize);
    GenFontInfoProperties(fontprop);

    // Clean up trailing characters
    if (back() == '.')
    {
        pop_back();
    }
    if (is_cpp())
    {
        while (back() == '\t')
        {
            pop_back();
        }
        if (back() != '\n')
        {
            *this += ';';
        }
    }
}

void Code::GenFontInfoInit(const FontProperty& fontprop, double point_size,
                           bool more_than_pointsize)
{
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
        std::array<char, 10> float_str {};
        if (auto [ptr, ec] =
                std::to_chars(float_str.data(), float_str.data() + float_str.size(), point_size);
            ec == std::errc())
        {
            Str(std::string_view(float_str.data(), ptr - float_str.data())).EndFunction();
        }
    }
    else
    {
        if (point_size <= 0)
        {
            Class("wxSystemSettings")
                .ClassMethod("GetFont(")
                .Add("wxSYS_DEFAULT_GUI_FONT")
                .Str(")");
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
}

void Code::GenFontInfoProperties(const FontProperty& fontprop)
{
#if defined(_WIN32)
    // REVIEW: [Randalphwa - 04-18-2025] Currently, wxPerl does support wxFontInfo, but
    // leave this code in case it is added later.
    if (is_perl())
    {
        if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
        {
            Eol().Str("$font_info->").Str("FaceName = ");
            QuotedString(wxue::string() << fontprop.GetFaceName().utf8_string()) += ";";
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
    else
#endif
    {
        if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
        {
            VariableMethod("FaceName(")
                .QuotedString(wxue::string() << fontprop.GetFaceName().utf8_string()) += ")";
        }
        if (fontprop.GetFamily() != wxFONTFAMILY_DEFAULT)
        {
            VariableMethod("Family(").Add(font_family_pairs.GetValue(fontprop.GetFamily())) += ")";
        }
        if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
        {
            VariableMethod("Style(").Add(font_style_pairs.GetValue(fontprop.GetStyle())) += ")";
        }
        if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
        {
            VariableMethod("Weight(").Add(font_weight_pairs.GetValue(fontprop.GetWeight())) += ")";
        }
        if (fontprop.IsUnderlined())
        {
            VariableMethod("Underlined()");
        }
        if (fontprop.IsStrikethrough())
        {
            VariableMethod("Strikethrough()");
        }
    }
}

void Code::ApplyFontToControl(wxue::string_view font_function)
{
    const char* font_var = is_perl() ? "$font" : "font_info";

    if (m_node->is_Form())
    {
        if (m_node->is_Gen(gen_wxPropertySheetDialog))
        {
            FormFunction("GetBookCtrl()").Function(font_function);
            if (!is_perl())
            {
                Object("wxFont").Str(font_var).Str(")");
            }
            else
            {
                Str(font_var);
            }
            EndFunction();
        }
        else
        {
            FormFunction(font_function);
            if (!is_perl())
            {
                Object("wxFont").VarName(font_var, false).Str(")");
            }
            else
            {
                Str(font_var);
            }
            EndFunction();
        }
    }
    else
    {
        NodeName().Function(font_function);
        if (!is_perl())
        {
            Object("wxFont").VarName(font_var, false).Str(")");
        }
        else
        {
            // wxPerl doesn't support wxFontInfo, so use the font creation generated above
            Str(font_var);
        }
        EndFunction();
    }
}
