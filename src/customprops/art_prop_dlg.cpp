///////////////////////////////////////////////////////////////////////////////
// Purpose:   Art Property Dialog for image property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-09-2026]

#include <set>

#include <wx/artprov.h>  // wxArtProvider class

#include "art_prop_dlg.h"

#include "mainframe.h"                   // MainFrame -- Main window frame
#include "node.h"                        // Node class
#include "wxue_namespace/wxue_string.h"  // wxue::is_found

extern std::set<std::string> set_art_ids;  // defined in pg_image.cpp

ArtPropertyDlg::ArtPropertyDlg(wxWindow* parent, const ImageProperties& img_props)
{
    Create(parent);

    m_list->AppendColumn(wxEmptyString);

    m_choice_client->Append("wxART_TOOLBAR");
    m_choice_client->Append("wxART_MENU");
    m_choice_client->Append("wxART_BUTTON");
    m_choice_client->Append("wxART_FRAME_ICON");
    m_choice_client->Append("wxART_CMN_DIALOG");
    m_choice_client->Append("wxART_HELP_BROWSER");
    m_choice_client->Append("wxART_MESSAGE_BOX");
    m_choice_client->Append("wxART_OTHER");

    if (auto pos = img_props.image.find('|'); wxue::is_found(pos))
    {
        m_client = img_props.image.subview(pos + 1).wx();
        m_org_id = img_props.image.subview(0, pos);
    }
    else
    {
        if (auto node = wxGetFrame().getSelectedNode(); node)
        {
            if (node->is_Type(type_tool) || node->is_Type(type_ribbontool) ||
                node->is_Type(type_aui_tool))
            {
                m_client = "wxART_TOOLBAR";
            }
            else if (node->is_Type(type_menuitem))
            {
                m_client = "wxART_MENU";
            }
            else if (node->is_Gen(gen_wxButton))
            {
                m_client = "wxART_BUTTON";
            }
            else
            {
                m_client = "wxART_OTHER";
            }
        }
        else
        {
            m_client = "wxART_OTHER";
        }
    }

    m_choice_client->SetStringSelection(m_client);
    ChangeClient();
}

void ArtPropertyDlg::OnInit(wxInitDialogEvent& /* event unused */)
{
    m_choice_client->SetStringSelection(m_client);
    for (long pos = 0; pos < m_list->GetItemCount(); pos++)
    {
        if (m_list->GetItemText(pos) == m_org_id)
        {
            m_list->SetItemState(pos, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            auto bmp = wxArtProvider::GetBitmap(m_org_id, wxART_MAKE_CLIENT_ID_FROM_STR(m_client));
            ASSERT(bmp.IsOk());
            if (bmp.IsOk())
            {
                m_canvas->SetSize(bmp.GetWidth(), bmp.GetHeight());
                m_canvas->SetBitmap(bmp);
                m_text->SetLabel(
                    wxString::Format("Size: %d x %d", bmp.GetWidth(), bmp.GetHeight()));
                Layout();
            }
            Refresh();

            break;
        }
    }
}

void ArtPropertyDlg::ChangeClient()
{
    // Save the current selection before all items are deleted. Restore the selection after the new
    // items have been added.
    auto sel = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel < 0)
        sel = 0;

    m_list->DeleteAllItems();

    int index = 0;
    wxVector<wxBitmapBundle> bitmap_bundles;
    for (auto& iter: set_art_ids)
    {
        if (auto bundle = wxArtProvider::GetBitmapBundle(
                iter, wxART_MAKE_CLIENT_ID_FROM_STR(m_client), wxSize(16, 16));
            bundle.IsOk())
        {
            bitmap_bundles.push_back(bundle);
            m_list->InsertItem(index, iter, (to_int) (bitmap_bundles.size() - 1));
            m_list->SetItemPtrData(index++, reinterpret_cast<wxUIntPtr>(iter.c_str()));
        }
    }
    m_list->SetSmallImages(bitmap_bundles);

    m_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
    m_list->SetItemState(sel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void ArtPropertyDlg::OnChooseClient(wxCommandEvent& /* event unused */)
{
    m_client = m_choice_client->GetStringSelection();
    ChangeClient();
}

void ArtPropertyDlg::OnSelectItem(wxListEvent& event)
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
        m_text->SetLabel(wxString().Format("Size: %d x %d", bmp.GetWidth(), bmp.GetHeight()));
        Layout();
    }
    Refresh();
}

wxString ArtPropertyDlg::GetResults()
{
    wxString results = m_id;
    results << '|' << m_client;
    return results;
}
