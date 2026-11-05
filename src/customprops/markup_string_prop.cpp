/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for labels that support wxWidgets markup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/colordlg.h>           // wxGetColourFromUser
#include <wx/colour.h>             // wxColour
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "markup_string_prop.h"

#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class
#include "node_prop.h"  // NodeProperty class

#include "wxue_namespace/wxue_string.h"  // wxue::string

MarkupStringProperty::MarkupStringProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

bool MarkupDialogAdapter::DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                                       wxPGProperty* /* property unused */)
{
    MarkStringDlg markup_dlg(wxGetFrame().getWindow(), m_prop);
    if (markup_dlg.ShowModal() == wxID_OK)
    {
        SetValue(markup_dlg.GetResults());
        return true;
    }

    return false;
}

MarkStringDlg::MarkStringDlg(wxWindow* parent, NodeProperty* prop) :
    MarkStringDlgBase(parent),
    m_prop(prop)
{
    SetTitle(wxue::string() << prop->get_DeclName() << " property editor");

    const wxString reference_text = R"(Tags:
  <b>bold</b>   <big>   <i>   <s>   <small>   <tt>   <u>

Attributes for <span fgcolor="#FF0000" ...>:
  fgcolor / foreground / color
  bgcolor / background
  face / font_family
  weight / font_weight
  style / font_style
  size

Sample:
  This is <i>italic</i> and this is <span fgcolor="#FF0000">red</span>

These characters must be escaped:
  &  ->  &amp;
  <  ->  &lt;
  >  ->  &gt;
  '  ->  &apos;
  "  ->  &quot;
)";

    m_reference->SetValue(reference_text);

    m_text->SetValue(prop->as_wxString());
    UpdatePreview();
    m_text->SetFocus();
}

void MarkStringDlg::InsertMarkup(const wxString& open_tag, const wxString& close_tag)
{
    long from {};
    long to {};
    m_text->GetSelection(&from, &to);

    if (from != to)
    {
        const wxString selected_text = m_text->GetStringSelection();
        m_text->Replace(from, to, open_tag + selected_text + close_tag);
        m_text->SetInsertionPoint(
            from + static_cast<long>((open_tag + selected_text + close_tag).length()));
        m_text->SetFocus();
        return;
    }

    // Nothing selected -- insert both tags and leave the caret between them.
    const long insert_at = m_text->GetInsertionPoint();
    m_text->Replace(insert_at, insert_at, open_tag + close_tag);
    m_text->SetInsertionPoint(insert_at + static_cast<long>(open_tag.length()));
    m_text->SetFocus();
}

void MarkStringDlg::UpdatePreview()
{
    const wxString value = m_text->GetValue();

    Node* node = m_prop->getNode();
    const bool markup_enabled = (node != nullptr && node->as_bool(prop_markup));

    if (value.empty())
    {
        m_preview->SetLabelText(wxEmptyString);
        m_status->SetLabel(wxEmptyString);
    }
    else if (!markup_enabled)
    {
        m_preview->SetLabelText(value);
        m_status->SetLabel(
            "The markup property is not enabled, so this label is shown as plain text.");
    }
    else if (!m_preview->SetLabelMarkup(value))
    {
        m_preview->SetLabelText(value);
        m_status->SetLabel("Markup is not well-formed; showing plain text.");
    }
    else
    {
        m_status->SetLabel(wxEmptyString);
    }

    // Wrap to the current pane width so the preview stays inside the right pane.
    const int pane_width = m_preview->GetParent()->GetClientSize().x - 16;
    if (pane_width > 0)
    {
        m_preview->Wrap(pane_width);
    }

    m_preview->GetParent()->Layout();
    Layout();
}

void MarkStringDlg::OnMarkupChanged([[maybe_unused]] wxCommandEvent& event)
{
    UpdatePreview();
}

void MarkStringDlg::OnInsertTag(wxCommandEvent& event)
{
    const wxObject* const event_object = event.GetEventObject();

    if (event_object == m_btn_bold)
    {
        InsertMarkup("<b>", "</b>");
    }
    else if (event_object == m_btn_italic)
    {
        InsertMarkup("<i>", "</i>");
    }
    else if (event_object == m_btn_underline)
    {
        InsertMarkup("<u>", "</u>");
    }
    else if (event_object == m_btn_strike)
    {
        InsertMarkup("<s>", "</s>");
    }
    else if (event_object == m_btn_monospace)
    {
        InsertMarkup("<tt>", "</tt>");
    }
    else if (event_object == m_btn_bigger)
    {
        InsertMarkup("<big>", "</big>");
    }
    else if (event_object == m_btn_smaller)
    {
        InsertMarkup("<small>", "</small>");
    }
}

void MarkStringDlg::OnInsertColour(wxCommandEvent& event)
{
    const wxString attribute = (event.GetEventObject() == m_btn_foreground) ? "fgcolor" : "bgcolor";

    const wxColour colour = wxGetColourFromUser(this, wxColour("red"));
    if (!colour.IsOk())
    {
        return;
    }

    const wxString open_tag = wxString("<span ") << attribute << "=\""
                                                 << colour.GetAsString(wxC2S_HTML_SYNTAX) << "\">";
    InsertMarkup(open_tag, "</span>");
}

void MarkStringDlg::OnOK(wxCommandEvent& event)
{
    m_result = m_text->GetValue();
    event.Skip();
}
