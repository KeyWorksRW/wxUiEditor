/////////////////////////////////////////////////////////////////////////////
// Purpose:   Art Property Dialog for image property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <set>

#include <wx/artprov.h>   // wxArtProvider class
#include <wx/imaglist.h>  // wxImageList base header

#include "art_prop_dlg.h"  // auto-generated: ../ui/artpropdlg_base.h and ../ui/artpropdlg_base.cpp

extern std::set<std::string> set_art_ids;  // defined in pg_image.cpp

ArtBrowserDialog::ArtBrowserDialog(wxWindow* parent, const ImageProperties& img_props) : ArtPropertyDlgBase(parent)
{
    m_list->AppendColumn(wxEmptyString);

    m_choice_client->Append("wxART_TOOLBAR");
    m_choice_client->Append("wxART_MENU");
    m_choice_client->Append("wxART_BUTTON");
    m_choice_client->Append("wxART_FRAME_ICON");
    m_choice_client->Append("wxART_CMN_DIALOG");
    m_choice_client->Append("wxART_HELP_BROWSER");
    m_choice_client->Append("wxART_MESSAGE_BOX");
    m_choice_client->Append("wxART_OTHER");

    if (auto pos = img_props.image.find('|'); ttlib::is_found(pos))
    {
        m_client = img_props.image.subview(pos + 1);
    }
    else
    {
        m_client = "wxART_OTHER";
    }

    m_choice_client->SetStringSelection(m_client);
    ChangeClient();
}

void ArtBrowserDialog::ChangeClient()
{
    auto img_list = new wxImageList(16, 16);
    int index = 0;

    auto sel = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel < 0)
        sel = 0;

    m_list->DeleteAllItems();
    for (auto& iter: set_art_ids)
    {
        auto bmp = wxArtProvider::GetBitmap(iter, wxART_MAKE_CLIENT_ID_FROM_STR(m_client), wxSize(16, 16));
        if (bmp.IsOk())
        {
            m_list->InsertItem(index, iter, img_list->Add(bmp));
            m_list->SetItemPtrData(index++, reinterpret_cast<wxUIntPtr>(iter.c_str()));
        }
    }

    m_list->AssignImageList(img_list, wxIMAGE_LIST_SMALL);
    m_list->SetColumnWidth(0, wxLIST_AUTOSIZE);

    m_list->SetItemState(sel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void ArtBrowserDialog::OnChooseClient(wxCommandEvent& WXUNUSED(event))
{
    m_client = m_choice_client->GetStringSelection();
    ChangeClient();
}

void ArtBrowserDialog::OnSelectItem(wxListEvent& event)
{
    m_id = reinterpret_cast<const char*>(event.GetData());
    if (!m_id)
        return;
    auto bmp = wxArtProvider::GetBitmap(m_id, wxART_MAKE_CLIENT_ID_FROM_STR(m_client));
    ASSERT(bmp.IsOk());
    if (bmp.IsOk())
    {
        m_canvas->SetSize(bmp.GetWidth(), bmp.GetHeight());
        m_canvas->SetBitmap(bmp);
        m_text->SetLabel(ttlib::cstr().Format("Size: %d x %d", bmp.GetWidth(), bmp.GetHeight()));
    }
    Refresh();
}

wxString ArtBrowserDialog::GetResults()
{
    wxString results = m_id;
    results << '|' << m_client;
    return results;
}
