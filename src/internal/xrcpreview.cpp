/////////////////////////////////////////////////////////////////////////////
// Purpose:   Test XRC
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if !defined(INTERNAL_TESTING)
    #error "INTERNAL_TESTING must be defined if you include this moduel!"
#endif

#include <wx/filedlg.h>     // wxFileDialog base header
#include <wx/mstream.h>     // Memory stream classes
#include <wx/xml/xml.h>     // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>  // XML resources

#include "tttextfile.h"  // ttlib::viewfile

#include "gen_xrc.h"     // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class
#include "xrcpreview.h"  // auto-generated: xrcpreview_base.h and xrcpreview_base.cpp

#include "pugixml.hpp"

const int node_marker = 1;

void MainFrame::OnXrcPreviewDlg(wxCommandEvent& /* event */)
{
    XrcPreview dlg(this);
    dlg.ShowModal();
}

// If this constructor is used, the caller must call Create(parent)
XrcPreview::XrcPreview() {}

XrcPreview::XrcPreview(wxWindow* parent) { Create(parent); }

void XrcPreview::OnCreate(wxCommandEvent& WXUNUSED(event))
{
    auto doc_str = GenerateXrcStr(nullptr, false, true);

    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
}

void XrcPreview::OnXrcCopy(wxCommandEvent& WXUNUSED(event))
{
    auto sel_node = wxGetFrame().GetSelectedNode();

    if (!sel_node)
    {
        wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
        return;
    }

    if (!sel_node->IsForm())
    {
        sel_node = sel_node->get_form();
    }

    auto doc_str = GenerateXrcStr(sel_node, false, true);

    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
    m_scintilla->SetEmptySelection(0);

    ttlib::viewfile m_view;
    m_view.ReadString(doc_str);

    ttlib::cstr search("name=\"");
    sel_node = wxGetFrame().GetSelectedNode();

    if (sel_node->HasProp(prop_id) && sel_node->prop_as_string(prop_id) != "wxID_ANY")
    {
        search << sel_node->prop_as_string(prop_id);
    }
    else if (sel_node->HasValue(prop_var_name))
    {
        search << sel_node->prop_as_string(prop_var_name);
    }
    else
    {
        search << sel_node->prop_as_string(prop_class_name);
    }

    int line = (to_int) m_view.FindLineContaining(search);

    if (!ttlib::is_found(line))
        return;

    m_scintilla->MarkerDeleteAll(node_marker);
    m_scintilla->MarkerAdd(line, node_marker);

    // Unlike GetLineVisible(), this function does ensure that the line is visible.
    m_scintilla->ScrollToLine(line);
}

void XrcPreview::OnPreview(wxCommandEvent& WXUNUSED(event))
{
    auto xrc_text = m_scintilla->GetText();
    wxString dlg_name;
    auto pos = xrc_text.Find("name=\"");
    if (!ttlib::is_found(pos))
    {
        wxMessageBox("Could not locate the dialog's name.", "XRC Dialog Preview");
        return;
    }
    pos += (sizeof("name=\"") - 1);
    while (pos < (to_int) xrc_text.size() && xrc_text[pos] != '"')
    {
        dlg_name << xrc_text[pos++];
    }

    wxMemoryInputStream stream(xrc_text, xrc_text.size());
    wxScopedPtr<wxXmlDocument> xmlDoc(new wxXmlDocument(stream));
    if (!xmlDoc->IsOk())
    {
        wxMessageBox("Invalid XRC -- wxXmlDocument can't parse it.", "XRC Dialog Preview");
        return;
    }

    auto xrc_resource = wxXmlResource::Get();
    xrc_resource->InitAllHandlers();

    wxString res_name("wxuiDlgPreview");

    if (!xrc_resource->LoadDocument(xmlDoc.release(), res_name))
    {
        wxMessageBox("wxWidgets could not parse the XRC data.", "XRC Dialog Preview");
        return;
    }

    wxDialog dlg;
    if (xrc_resource->LoadDialog(&dlg, this, dlg_name))
    {
        dlg.ShowModal();
    }
    else
    {
        wxMessageBox(wxString("Could not load ") << dlg_name << " resource.", "XRC Dialog Preview");
    }
    xrc_resource->Unload(res_name);
}

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
    #define SCI_GETTEXT_MSG 2182
#endif

extern const char* g_xrc_keywords;

void XrcPreview::OnInit(wxInitDialogEvent& event)
{
    // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
    m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

    m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
    m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#E91AFF"));
    m_scintilla->StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
    m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, *wxRED);
    m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
    m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));

    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

    event.Skip();
}

void XrcPreview::OnExport(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(this, "Export Project As XRC", wxGetApp().GetProjectPath(), "preview_test.xrc",
                        "XRC File (*.xrc)|*.xrc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        ttString filename = dialog.GetPath();

        std::string buf;
        buf.reserve(m_scintilla->GetTextLength() + 1);
        auto len = m_scintilla->GetTextLength() + 1;
        m_scintilla->SendMsg(SCI_GETTEXT_MSG, len, (wxIntPtr) buf.data());

        pugi::xml_document doc;
        doc.load_string(buf.c_str());

        if (!doc.save_file(filename.wx_str(), "\t"))
        {
            wxMessageBox(wxString("An unexpected error occurred exportin ") << filename, "Export XRC");
        }
    }
}

void XrcPreview::OnSearch(wxCommandEvent& event)
{
    m_scintilla->SetSelectionStart(m_scintilla->GetSelectionEnd());
    m_scintilla->SearchAnchor();
    auto srch_string = event.GetString();
    auto result = m_scintilla->SearchNext(0, event.GetString());

    if (result == wxSTC_INVALID_POSITION)
    {
        wxMessageBox(wxString() << event.GetString() << " not found.", "Not Found", wxICON_ERROR);
    }
    else
    {
        m_scintilla->EnsureCaretVisible();
    }
}
