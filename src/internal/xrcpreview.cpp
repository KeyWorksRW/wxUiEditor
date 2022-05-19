/////////////////////////////////////////////////////////////////////////////
// Purpose:   Preview XRC code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if !defined(INTERNAL_TESTING)
    #error "INTERNAL_TESTING must be defined if you include this moduel!"
#endif

#include <wx/mstream.h>     // Memory stream classes
#include <wx/xml/xml.h>     // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>  // XML resources

#include "mainframe.h"
#include "xrcpreview.h"  // auto-generated: xrcpreview_base.h and xrcpreview_base.cpp

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
    // TODO: Implement OnCreate
}

void XrcPreview::OnClipBoard(wxCommandEvent& WXUNUSED(event))
{
    // event.Skip();
}

void XrcPreview::OnXrcCopy(wxCommandEvent& WXUNUSED(event))
{
    // event.Skip();
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
    while (pos < xrc_text.size() && xrc_text[pos] != '"')
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
