///////////////////////////////////////////////////////////////////////////////
// Purpose:   Live XRC preview page for the XRC panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
///////////////////////////////////////////////////////////////////////////////
// CR: [11-09-2026]

#include <string>
#include <string_view>

#include <wx/button.h>    // wxButton class
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/stattext.h>  // wxStaticText class

#include "xrc_preview_panel.h"

#include "gen_xrc.h"   // GenerateXrcStr
#include "node.h"      // Node class
#include "previews.h"  // LoadXrcPanel, PreviewXrc

XrcPreviewPanel::XrcPreviewPanel(wxWindow* parent) : wxPanel(parent)
{
    m_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_sizer);
}

void XrcPreviewPanel::ClearContent()
{
    m_sizer->Clear();
    DestroyChildren();
}

void XrcPreviewPanel::ShowForm(Node* form_node)
{
    ClearContent();

    if (!form_node)
    {
        ShowMessage("Select a form to preview.", nullptr);
        return;
    }

    const bool is_panel = form_node->is_Gen(gen_PanelForm);
    const bool is_dialog = form_node->is_Gen(gen_wxDialog);

    if (!is_panel && !is_dialog)
    {
        // get_DeclName() gives the generator name (e.g. "wxFrame", "MenuBar") which reads better
        // to the user than the internal form type.
        const std::string_view decl_name = form_node->get_DeclName();
        wxString form_name = form_node->as_wxString(prop_class_name);
        if (form_name.empty())
        {
            form_name = wxString::FromUTF8(decl_name.data(), decl_name.size());
        }
        const wxString form_type = wxString::FromUTF8(decl_name.data(), decl_name.size());
        const wxString message = form_name + " is a " + form_type +
                                 " and cannot be displayed in this panel. Use the button to open "
                                 "it in its own window.";
        ShowMessage(message, form_node);
        return;
    }

    if (is_dialog && form_node->get_ChildCount() == 0)
    {
        // Generating preview XRC for a childless dialog dereferences a null child, so there is
        // nothing we can display.
        ShowMessage("This dialog has no contents to display.", form_node);
        return;
    }

    const size_t flags = is_dialog ? xrc::previewing : 0;
    const std::string xrc_str = GenerateXrcStr(form_node, flags);

    const wxString panel_name =
        is_dialog ? wxString(txt_dlg_name) : form_node->as_wxString(prop_class_name);

    wxString err_msg;
    wxPanel* panel = LoadXrcPanel(xrc_str, this, panel_name, err_msg);
    if (!panel)
    {
        ShowMessage(wxString("Unable to display the preview: ") + err_msg, form_node);
        return;
    }

    m_sizer->Add(panel, wxSizerFlags(1).Expand());
    Layout();
}

void XrcPreviewPanel::ShowMessage(const wxString& message, Node* form_node)
{
    auto* text = new wxStaticText(this, wxID_ANY, message);
    if (const int width = GetClientSize().GetWidth(); width > 0)
    {
        text->Wrap(width);
    }
    m_sizer->Add(text, wxSizerFlags().Border());

    if (form_node)
    {
        auto* button = new wxButton(this, wxID_ANY, "Open in Window");
        button->Bind(wxEVT_BUTTON,
                     [form_node](wxCommandEvent&)
                     {
                         PreviewXrc(form_node);
                     });
        m_sizer->Add(button, wxSizerFlags().Border());
    }

    Layout();
}
