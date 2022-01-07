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

    for (auto& iter: font_symbol_pairs.GetPairs())
    {
        m_comboSymbolSize->Append(iter.first);
    }
    m_comboSymbolSize->SetStringSelection(font_symbol_pairs.GetName(m_system_font.GetSymbolSize()));

    for (auto& iter: font_family_pairs.GetPairs())
    {
        m_comboFamily->Append(iter.first);
    }
    m_comboFamily->SetStringSelection(font_family_pairs.GetName(m_custom_font.GetFamily()));

    for (auto& iter: font_style_pairs.GetPairs())
    {
        m_comboSystemStyles->Append(iter.first);
        m_comboCustomStyles->Append(iter.first);
    }
    m_comboSystemStyles->SetStringSelection(font_style_pairs.GetName(m_system_font.GetStyle()));
    m_comboCustomStyles->SetStringSelection(font_style_pairs.GetName(m_custom_font.GetStyle()));

    for (auto& iter: font_weight_pairs.GetPairs())
    {
        m_comboSystemWeight->Append(iter.first);
        m_comboCustomWeight->Append(iter.first);
    }
    m_comboSystemWeight->SetStringSelection(font_weight_pairs.GetName(m_system_font.GetWeight()));
    m_comboCustomWeight->SetStringSelection(font_weight_pairs.GetName(m_custom_font.GetWeight()));

    auto names = m_font_enum.GetFacenames();
    m_comboFacenames->Append(names);

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

    m_staticCustomSample->SetFont(m_custom_font.GetFont());
    m_staticSystemSample->SetFont(m_system_font.GetFont());

    UpdateFontInfo();
}

void FontPropDlg::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    if (!m_custom_font.isDefGuiFont())
    {
        m_system_box->GetStaticBox()->Enable(false);
        m_radioSystem->SetValue(false);
        m_radioCustom->SetValue(true);
        m_staticSystemSample->Enable();  // Keep the sample font enabled
    }
    else
    {
        m_custom_box->GetStaticBox()->Enable(false);
        m_radioSystem->SetValue(true);
        m_radioCustom->SetValue(false);
        m_staticCustomSample->Enable();  // Keep the sample font enabled
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
}

void FontPropDlg::OnSymbolSize(wxCommandEvent& WXUNUSED(event))
{
    UpdateFontInfo();
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
        m_system_font.SymbolicSize(
            font_symbol_pairs.GetValue((const char*) m_comboSymbolSize->GetStringSelection().mb_str()));
        m_system_font.Style(font_style_pairs.GetValue((const char*) m_comboSystemStyles->GetStringSelection().mb_str()));
        m_system_font.Weight(font_weight_pairs.GetValue((const char*) m_comboSystemWeight->GetStringSelection().mb_str()));
        m_system_font.Underlined(m_checkSystemUnderlined->GetValue());
        m_system_font.Strikethrough(m_checkSystemStrikeThrough->GetValue());
    }
    else
    {
        m_custom_font.Family(font_family_pairs.GetValue((const char*) m_comboFamily->GetValue().mb_str()));
        m_custom_font.PointSize(m_spinCustomPointSize->GetValue());
        m_custom_font.Style(font_style_pairs.GetValue((const char*) m_comboCustomStyles->GetValue().mb_str()));
        m_custom_font.Weight(font_weight_pairs.GetValue((const char*) m_comboCustomWeight->GetStringSelection().mb_str()));
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
    Fit();
}

void FontPropDlg::OnOK(wxCommandEvent& event)
{
    if (m_radioSystem->GetValue())
    {
        m_system_font.setDefGuiFont(true);
        m_system_font.SymbolicSize(
            font_symbol_pairs.GetValue((const char*) m_comboSymbolSize->GetStringSelection().mb_str()));
        m_system_font.Style(font_style_pairs.GetValue((const char*) m_comboSystemStyles->GetStringSelection().mb_str()));
        m_system_font.Weight(font_weight_pairs.GetValue((const char*) m_comboSystemWeight->GetStringSelection().mb_str()));
        m_system_font.Underlined(m_checkSystemUnderlined->GetValue());
        m_system_font.Strikethrough(m_checkSystemStrikeThrough->GetValue());
        m_value = m_system_font.as_wxString();
    }
    else
    {
        m_custom_font.setDefGuiFont(false);
        m_custom_font.Family(font_family_pairs.GetValue((const char*) m_comboFamily->GetValue().mb_str()));
        m_custom_font.PointSize(m_spinCustomPointSize->GetValue());
        m_custom_font.Style(font_style_pairs.GetValue((const char*) m_comboCustomStyles->GetValue().mb_str()));
        m_custom_font.Weight(font_weight_pairs.GetValue((const char*) m_comboCustomWeight->GetStringSelection().mb_str()));
        m_custom_font.Underlined(m_checkCustomUnderlined->GetValue());
        m_custom_font.Strikethrough(m_checkCustomStrikeThrough->GetValue());
        auto facename = m_comboFacenames->GetStringSelection();
        if (facename == "default")
            m_custom_font.FaceName("");
        else
            m_custom_font.FaceName(facename);
        m_value = m_custom_font.as_wxString();
    }

    event.Skip();
}
