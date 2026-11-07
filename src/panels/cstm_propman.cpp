/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxPropertyGridManager class with an HTML description box
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-07-2026]

// wxPropertyGridManager creates two static text controls for the description box: a caption
// (m_pTxtHelpCaption) and the content area (m_pTxtHelpContent). Both are protected members of
// wxPropertyGridManager, and the content is filled with plain text by
// wxPropertyGridManager::SetDescribedProperty(). Plain text cannot render markdown, so we keep the
// built-in caption and overlay our own wxHtmlWindow on top of the content area, then drive the
// content ourselves from the property's help string.
//
// The base class handler for wxEVT_PG_SELECTED must keep running (it lays out the description box
// and positions the caption), so we bind our handler and call event.Skip(). Because the base class
// handler runs *after* ours and refills the placeholder content with plain text, every sync ends
// with the placeholder label cleared and the HTML window shown on top of it.

#include <wx/html/htmlwin.h>  // wxHtmlWindow
#include <wx/settings.h>      // wxSystemSettings
#include <wx/stattext.h>      // wxStaticText

#include "cstm_propman.h"  // CustomPropertyManager

#include "data/cppmark/include/cppmark_html.h"  // cmark_markdown_to_html

void CustomPropertyManager::InitializeHtmlDescription()
{
    if (m_html_content)
    {
        return;
    }

    // Sized and shown by SyncDescriptionWindow() once the base class has positioned the description
    // box content area.
    m_html_content = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                      wxHW_SCROLLBAR_AUTO | wxBORDER_NONE);
    m_html_content->Hide();

    Bind(wxEVT_PG_SELECTED, &CustomPropertyManager::OnPropertyGridSelect, this, GetId());

    // The splitter that sizes the description box is driven by the base class, so let it finish
    // that work first and then snap the HTML window to the new content area.
    Bind(wxEVT_SIZE, &CustomPropertyManager::OnDescriptionBoxSize, this);
    Bind(wxEVT_MOTION, &CustomPropertyManager::OnDescriptionBoxMotion, this);
    Bind(wxEVT_LEFT_UP, &CustomPropertyManager::OnDescriptionBoxLeftUp, this);
}

void CustomPropertyManager::ShowDescription(const wxString& caption, const wxString& markdown_text)
{
    if (!m_html_content || !m_pTxtHelpCaption)
    {
        return;
    }

    // XML declaration files store line breaks as a literal backslash + n. Replace them with real
    // newlines so markdown treats them as line breaks. Harmless if the text already has real
    // newlines.
    wxString text = markdown_text;
    text.Replace("\\n", "\n", true);

    // Let the base class size/position the caption and the content area, then blank the placeholder
    // so it doesn't show plain text under the HTML window.
    wxPropertyGridManager::SetDescription(caption, wxEmptyString);
    if (m_pTxtHelpContent)
    {
        m_pTxtHelpContent->SetLabel(wxEmptyString);
    }

    if (text.empty())
    {
        m_has_description = false;
        m_html_content->Hide();
        SyncDescriptionWindow();
        return;
    }

    m_has_description = true;

    const std::string body =
        cmark_markdown_to_html(text.utf8_string(), CMARK_OPT_UNSAFE | CMARK_OPT_HARDBREAKS);

    // Inherit the system colours so the text remains readable in dark mode.
    const wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    wxString html;
    html << "<html><body bgcolor=\"" << background.GetAsString(wxC2S_HTML_SYNTAX) << "\" text=\""
         << foreground.GetAsString(wxC2S_HTML_SYNTAX) << "\">";
    html << wxString::FromUTF8(body);
    html << "</body></html>";

    m_html_content->SetPage(html);
    SyncDescriptionWindow();
}

void CustomPropertyManager::ClearDescription()
{
    ShowDescription(wxEmptyString, wxEmptyString);
}

void CustomPropertyManager::SyncDescriptionWindow()
{
    if (!m_html_content || !m_pTxtHelpContent)
    {
        return;
    }

    // Nothing to show, so never reveal a stale page left over from a previous property.
    if (!m_has_description)
    {
        m_html_content->Hide();
        return;
    }

    // The base class hides the content area when the description box is too small to show anything
    // useful -- in that case the HTML window must stay hidden too.
    if (!m_pTxtHelpContent->IsShown() || m_pTxtHelpContent->GetRect().GetHeight() <= 2)
    {
        m_html_content->Hide();
        return;
    }

    if (!m_pTxtHelpContent->GetLabel().empty())
    {
        m_pTxtHelpContent->SetLabel(wxEmptyString);
    }

    const wxRect content_rect = m_pTxtHelpContent->GetRect();
    if (m_html_content->GetRect() != content_rect)
    {
        m_html_content->SetSize(content_rect);
    }

    m_html_content->Show();
}

void CustomPropertyManager::OnPropertyGridSelect(wxPropertyGridEvent& event)
{
    wxPGProperty* prop = event.GetProperty();
    if (prop && !prop->IsCategory())
    {
        ShowDescription(prop->GetLabel(), prop->GetHelpString());
    }
    else
    {
        ClearDescription();
    }

    // The base class handler must still run -- it lays out the description box. It also refills the
    // placeholder content, so sync again once it has finished.
    event.Skip();
    CallAfter(&CustomPropertyManager::SyncDescriptionWindow);
}

void CustomPropertyManager::OnDescriptionBoxSize(wxSizeEvent& event)
{
    event.Skip();
    CallAfter(&CustomPropertyManager::SyncDescriptionWindow);
}

void CustomPropertyManager::OnDescriptionBoxMotion(wxMouseEvent& event)
{
    event.Skip();
    CallAfter(&CustomPropertyManager::SyncDescriptionWindow);
}

void CustomPropertyManager::OnDescriptionBoxLeftUp(wxMouseEvent& event)
{
    event.Skip();
    CallAfter(&CustomPropertyManager::SyncDescriptionWindow);
}
