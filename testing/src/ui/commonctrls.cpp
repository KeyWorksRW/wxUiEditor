/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common controls dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/artprov.h>

#include "commonctrls.h"  // auto-generated: commonctrls_base.h and commonctrls_base.cpp

#include "popupwin.h"  // PopupWin -- Popup Window

CommonCtrls::CommonCtrls(wxWindow* parent) : CommonCtrlsBase(parent)
{
    m_bmpComboBox->Append("Home", wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_MENU));
    m_bmpComboBox->Append("Print", wxArtProvider::GetBitmap(wxART_PRINT, wxART_MENU));
}

CommonCtrls::~CommonCtrls()
{
    delete m_popup_win;
}

void CommonCtrls::OnListChecked(wxCommandEvent& WXUNUSED(event))
{
    m_infoBar->ShowMessage("wxEVT_CHECKLISTBOX event");
    Fit();
}

void CommonCtrls::OnRadioBox(wxCommandEvent& WXUNUSED(event))
{
    m_infoBar->ShowMessage("wxEVT_RADIOBOX event");
    Fit();
}

void CommonCtrls::OnSlider(wxCommandEvent& WXUNUSED(event))
{
    m_infoBar->ShowMessage("wxEVT_SLIDER event");
    Fit();
}

void CommonCtrls::OnListBox(wxCommandEvent& WXUNUSED(event))
{
    m_infoBar->ShowMessage("wxEVT_LISTBOX event");
    Fit();
}

void CommonCtrls::OnPopupBtn(wxCommandEvent& event)
{
    if (m_popup_win)
        delete m_popup_win;
    m_popup_win = new PopupWin(this);

    auto btn = wxStaticCast(event.GetEventObject(), wxWindow);
    auto pos = btn->ClientToScreen(wxPoint(0, 0));
    m_popup_win->Position(pos, btn->GetSize());

    m_popup_win->Popup();
}
