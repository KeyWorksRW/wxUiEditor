/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing Font Property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "font_prop_dlg.h"  // auto-generated: ../ui/fontpropdlg_base.h and ../ui/fontpropdlg_base.cpp

#include "../nodes/node.h"        // Node class
#include "../nodes/node_event.h"  // NodeEventInfo -- NodeEvent and NodeEventInfo classes
#include "../utils/font_prop.h"   // FontProperty class

FontPropDlg::FontPropDlg(wxWindow* parent, NodeProperty* prop) : FontPropDlgBase(parent)
{
    m_value = prop->as_wxString();
    if (m_value.size())
    {
        m_custom_font.Convert(m_value.utf8_string());
        if (m_custom_font.isDefGuiFont())
        {
            m_system_font.Convert(m_value.utf8_string());
        }
    }

    auto names = m_font_enum.GetFacenames();
    m_comboFacenames->Append(names);

    if (m_custom_font.GetFamily() != wxFONTFAMILY_DEFAULT)
    {
        switch (m_custom_font.GetFamily())
        {
            case wxFONTFAMILY_DEFAULT:
                m_comboFamily->SetStringSelection("default");
                break;

            case wxFONTFAMILY_DECORATIVE:
                m_comboFamily->SetStringSelection("decorative");
                break;

            case wxFONTFAMILY_ROMAN:
                m_comboFamily->SetStringSelection("roman");
                break;

            case wxFONTFAMILY_SCRIPT:
                m_comboFamily->SetStringSelection("script");
                break;

            case wxFONTFAMILY_SWISS:
                m_comboFamily->SetStringSelection("swiss");
                break;

            case wxFONTFAMILY_MODERN:
                m_comboFamily->SetStringSelection("modern");
                break;

            case wxFONTFAMILY_TELETYPE:
                m_comboFamily->SetStringSelection("teletype");
                break;
        }
    }

    if (m_custom_font.GetStyle() != wxFONTSTYLE_NORMAL)
    {
        switch (m_custom_font.GetStyle())
        {
            case wxFONTSTYLE_ITALIC:
                m_comboCustomStyles->SetStringSelection("italic");
                break;

            case wxFONTSTYLE_SLANT:
                m_comboCustomStyles->SetStringSelection("slant");
                break;
        }
    }

    if (m_system_font.GetStyle() != wxFONTSTYLE_NORMAL)
    {
        switch (m_system_font.GetStyle())
        {
            case wxFONTSTYLE_ITALIC:
                m_comboSystemStyles->SetStringSelection("italic");
                break;

            case wxFONTSTYLE_SLANT:
                m_comboSystemStyles->SetStringSelection("slant");
                break;
        }
    }

    if (m_custom_font.GetWeight() != wxFONTWEIGHT_NORMAL)
    {
        switch (m_custom_font.GetWeight())
        {
            case wxFONTWEIGHT_THIN:
                m_comboCustomWeight->SetStringSelection("thin");
                break;

            case wxFONTWEIGHT_EXTRALIGHT:
                m_comboCustomWeight->SetStringSelection("extra light");
                break;

            case wxFONTWEIGHT_LIGHT:
                m_comboCustomWeight->SetStringSelection("light");
                break;

            case wxFONTWEIGHT_MEDIUM:
                m_comboCustomWeight->SetStringSelection("medium");
                break;

            case wxFONTWEIGHT_SEMIBOLD:
                m_comboCustomWeight->SetStringSelection("semi-bold");
                break;

            case wxFONTWEIGHT_BOLD:
                m_comboCustomWeight->SetStringSelection("bold");
                break;

            case wxFONTWEIGHT_EXTRABOLD:
                m_comboCustomWeight->SetStringSelection("extra bold");
                break;

            case wxFONTWEIGHT_HEAVY:
                m_comboCustomWeight->SetStringSelection("heavy");
                break;

            case wxFONTWEIGHT_EXTRAHEAVY:
                m_comboCustomWeight->SetStringSelection("extra heavy");
                break;
        }
    }

    if (m_system_font.GetWeight() != wxFONTWEIGHT_NORMAL)
    {
        switch (m_system_font.GetWeight())
        {
            case wxFONTWEIGHT_THIN:
                m_comboSystemWeight->SetStringSelection("thin");
                break;

            case wxFONTWEIGHT_EXTRALIGHT:
                m_comboSystemWeight->SetStringSelection("extra light");
                break;

            case wxFONTWEIGHT_LIGHT:
                m_comboSystemWeight->SetStringSelection("light");
                break;

            case wxFONTWEIGHT_MEDIUM:
                m_comboSystemWeight->SetStringSelection("medium");
                break;

            case wxFONTWEIGHT_SEMIBOLD:
                m_comboSystemWeight->SetStringSelection("semi-bold");
                break;

            case wxFONTWEIGHT_BOLD:
                m_comboSystemWeight->SetStringSelection("bold");
                break;

            case wxFONTWEIGHT_EXTRABOLD:
                m_comboSystemWeight->SetStringSelection("extra bold");
                break;

            case wxFONTWEIGHT_HEAVY:
                m_comboSystemWeight->SetStringSelection("heavy");
                break;

            case wxFONTWEIGHT_EXTRAHEAVY:
                m_comboSystemWeight->SetStringSelection("extra heavy");
                break;
        }
    }

    if (m_custom_font.IsUnderlined())
    {
        m_checkCustomUnderlined->SetValue(true);
    }

    if (m_system_font.IsUnderlined())
    {
        m_checkSystemUnderlined->SetValue(true);
    }

    if (m_custom_font.IsStrikethrough())
    {
        m_checkCustomStrikeThrough->SetValue(true);
    }

    if (m_system_font.IsStrikethrough())
    {
        m_checkSystemStrikeThrough->SetValue(true);
    }

    if (!m_custom_font.isDefGuiFont() && m_custom_font.GetFaceName().size())
    {
        m_comboFacenames->SetStringSelection(m_custom_font.GetFaceName());
    }

    m_spinCustomPointSize->SetValue(m_custom_font.GetFractionalPointSize());
    // m_spinSystemPointSize->SetValue(m_system_font.GetFractionalPointSize());

    m_staticCustomSample->SetFont(m_custom_font.GetFont());
    m_staticSystemSample->SetFont(m_system_font.GetFont());
}

void FontPropDlg::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    if (!m_custom_font.isDefGuiFont())
    {
        m_system_box->GetStaticBox()->Enable(false);
        m_radioSystem->SetValue(false);
        m_radioCustom->SetValue(true);
        m_staticSystemSample->Enable();  // Keep the sample fonts enabled
    }
    else
    {
        m_custom_box->GetStaticBox()->Enable(false);
        m_radioSystem->SetValue(true);
        m_radioCustom->SetValue(false);
        m_staticCustomSample->Enable();
    }
}

void FontPropDlg::OnFamily(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
}

void FontPropDlg::OnStyle(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
}

void FontPropDlg::OnWeight(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
}

void FontPropDlg::OnFacename(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
}

void FontPropDlg::OnUnderlined(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
}

void FontPropDlg::OnStrikeThrough(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
}

void FontPropDlg::OnCustomRadio(wxCommandEvent& WXUNUSED(event))
{
    if (m_radioCustom->GetValue())
    {
        m_radioSystem->SetValue(false);
        m_custom_box->GetStaticBox()->Enable(true);
        m_system_box->GetStaticBox()->Enable(false);
        m_staticSystemSample->Enable();  // Keep the sample fonts enabled
    }
}

void FontPropDlg::OnSystemRadio(wxCommandEvent& WXUNUSED(event))
{
    if (m_radioSystem->GetValue())
    {
        m_radioCustom->SetValue(false);
        m_custom_box->GetStaticBox()->Enable(false);
        m_system_box->GetStaticBox()->Enable(true);
        m_staticCustomSample->Enable();
    }
}

void FontPropDlg::OnPointSize(wxSpinDoubleEvent& WXUNUSED(event))
{
    UpdateFontInfo();
    Fit();
}

void FontPropDlg::OnSymbolSize(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
    Fit();
}

void FontPropDlg::OnEditPointSize(wxCommandEvent& event)
{
    if (auto digit = std::atof(event.GetString()); digit >= 4.0 && digit <= 72.0)
    {
        auto control = wxDynamicCast(event.GetEventObject(), wxSpinCtrlDouble);
        control->SetValue(digit);
        UpdateFontInfo();
    }
}

void FontPropDlg::UpdateFontInfo()
{
    if (m_radioSystem->GetValue())
    {
        m_system_font.SymbolicSize(font_symbol_pairs.at((const char*) m_comboSymbolSize->GetStringSelection().mb_str()));
        m_system_font.Style(font_style_pairs.at((const char*) m_comboSystemStyles->GetStringSelection().mb_str()));
        m_system_font.Weight(font_weight_pairs.at((const char*) m_comboSystemWeight->GetStringSelection().mb_str()));
        m_system_font.Underlined(m_checkSystemUnderlined->GetValue());
        m_system_font.Strikethrough(m_checkSystemStrikeThrough->GetValue());
    }
    else
    {
        m_custom_font.Family(font_family_pairs.at((const char*) m_comboFamily->GetValue().mb_str()));
        m_custom_font.PointSize(m_spinCustomPointSize->GetValue());
        m_custom_font.Style(font_style_pairs.at((const char*) m_comboCustomStyles->GetValue().mb_str()));
        m_custom_font.Weight(font_weight_pairs.at((const char*) m_comboCustomWeight->GetStringSelection().mb_str()));
        m_custom_font.Underlined(m_checkCustomUnderlined->GetValue());
        m_custom_font.Strikethrough(m_checkCustomStrikeThrough->GetValue());
        auto facename = m_comboFacenames->GetStringSelection();
        if (facename == "default")
            m_custom_font.FaceName("");
        else
            m_custom_font.FaceName(facename);
    }

    m_staticCustomSample->SetFont(m_custom_font.GetFont());

    wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    font.SetSymbolicSize(m_system_font.GetSymbolSize());
    font.SetStyle(m_system_font.GetStyle());
    font.SetWeight(m_system_font.GetWeight());
    font.SetUnderlined(m_system_font.IsUnderlined());
    font.SetStrikethrough(m_system_font.IsStrikethrough());

    m_staticSystemSample->SetFont(font);
}
