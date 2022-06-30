/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++/XRC UI Comparison dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/mstream.h>           // Memory stream classes
#include <wx/persist.h>           // common classes for persistence support
#include <wx/persist/toplevel.h>  // persistence support for wxTLW
#include <wx/stattext.h>          // wxStaticText base header
#include <wx/xml/xml.h>           // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>        // XML resources

// The following handlers must be explicitly added

#include <wx/xrc/xh_aui.h>             // XRC resource handler for wxAUI
#include <wx/xrc/xh_auitoolb.h>        // XML resource handler for wxAuiToolBar
#include <wx/xrc/xh_ribbon.h>          // XML resource handler for wxRibbon related classes
#include <wx/xrc/xh_richtext.h>        // XML resource handler for wxRichTextCtrl
#include <wx/xrc/xh_styledtextctrl.h>  // XML resource handler for wxStyledTextCtrl

#include "xrccompare.h"  // auto-generated: xrccompare_base.h and xrccompare_base.cpp

#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "project_class.h"  // Project class

// Defined in mockup_preview.cpp
void CreateMockupChildren(Node* node, wxWindow* parent, wxObject* parentNode, wxSizer* parent_sizer, wxWindow* form_window);

// Defined in gen_xrc.cpp
std::string GenerateXrcStr(Node* node_start, size_t xrc_flags);

extern const char* txt_dlg_name;

void MainFrame::OnCompareXrcDlg(wxCommandEvent& /* event */)
{
    if (!m_selected_node)
    {
        wxMessageBox("You need to select a form first.", "Compare");
        return;
    }

    auto form_node = m_selected_node.get();
    if (!form_node->IsForm())
    {
        if (form_node->isGen(gen_Project) && form_node->GetChildCount())
        {
            form_node = GetProject()->GetFirstFormChild();
        }
        else
        {
            form_node = form_node->get_form();
        }
    }

    if (!form_node->isGen(gen_wxDialog) && !form_node->isGen(gen_PanelForm))
    {
        wxMessageBox("You can only compare dialogs and panels", "Compare");
        return;
    }

    XrcCompare dlg_compare;
    if (!dlg_compare.DoCreate(this, form_node))
    {
        wxMessageBox("Unable to create the XrcCompare dialog box!", "Compare");
        return;
    }

    dlg_compare.ShowModal();
}

// If this constructor is used, the caller must call Create(parent)
XrcCompare::XrcCompare()
{
    auto xrc_resource = wxXmlResource::Get();
    xrc_resource->InitAllHandlers();
    xrc_resource->AddHandler(new wxRichTextCtrlXmlHandler);
    xrc_resource->AddHandler(new wxAuiXmlHandler);
    xrc_resource->AddHandler(new wxAuiToolBarXmlHandler);
    xrc_resource->AddHandler(new wxRibbonXmlHandler);
    xrc_resource->AddHandler(new wxStyledTextCtrlXmlHandler);
    m_res_name = "wxuiCompare";
}

XrcCompare::~XrcCompare()
{
    wxXmlResource::Get()->Unload(m_res_name);
}

bool XrcCompare::DoCreate(wxWindow* parent, Node* form_node)
{
    if (!wxDialog::Create(parent, wxID_ANY, "Compare C++/XRC Generated UI", wxDefaultPosition, wxDefaultSize,
                          wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER))
        return false;

    // Everything below up to the customization section is a direct copy of the XrcCompareBase::Create() function

    m_flex_grid_sizer = new wxFlexGridSizer(2, 0, 20);
    m_flex_grid_sizer->SetFlexibleDirection(wxVERTICAL);

    auto staticText = new wxStaticText(this, wxID_ANY, "C++ Generated");
    m_flex_grid_sizer->Add(staticText, wxSizerFlags().Border(wxALL));

    auto staticText_2 = new wxStaticText(this, wxID_ANY, "XRC Generated");
    m_flex_grid_sizer->Add(staticText_2, wxSizerFlags().Border(wxALL));

    // Customization section

    // TODO: [KeyWorks - 06-09-2022] Add the C++ and XRC top level sizers here

    auto xrc_resource = wxXmlResource::Get();

    switch (form_node->gen_name())
    {
        case gen_PanelForm:
            {
                CreateMockupChildren(form_node, this, nullptr, m_flex_grid_sizer, this);

                if (!InitXrc(form_node))
                    return false;

                if (auto object = xrc_resource->LoadObject(this, form_node->value(prop_class_name), "wxPanel"); object)
                {
                    m_flex_grid_sizer->Add(wxStaticCast(object, wxPanel));
                }
                else
                {
                    wxMessageBox("Could not load XRC wxPanel", "Compare");
                    return false;
                }
            }
            break;

        case gen_wxDialog:
            {
                if (form_node->GetChildCount() < 1)
                {
                    wxMessageBox("Only a dialog's children can be shown -- this dialog has none.", "Compare");
                    return false;
                }

                // The wxDialog generator will create a wxPanel as the mockup
                CreateMockupChildren(form_node, this, nullptr, m_flex_grid_sizer, this);

                // In theory, we should be able to start from the sizer and pass that to m_flex_grid_sizer. In practice, it
                // causes wxWidgets to crash. I'm not sure why, but setting both the C++ and XRC generators to use wxPanel
                // solves the problem.

                // GenerateXrcStr will return a wxPanel using the name txt_dlg_name ("_wxue_temp_dlg")
                if (!InitXrc(form_node))
                    return false;

                if (auto object = xrc_resource->LoadObject(this, txt_dlg_name, "wxPanel"); object)
                {
                    m_flex_grid_sizer->Add(wxStaticCast(object, wxPanel));
                }
                else
                {
                    wxMessageBox("Could not load top level sizer", "Compare");
                    return false;
                }
            }
            break;

        default:
            wxMessageBox("This form is not supported yet...", "Compare");
            return false;
    }

    SetSizerAndFit(m_flex_grid_sizer);
    Centre(wxBOTH);

    wxPersistentRegisterAndRestore(this, "XrcCompare");

    return true;
}

bool XrcCompare::InitXrc(Node* form_node)
{
    size_t xrc_flags = (form_node->isGen(gen_wxDialog) ? xrc::previewing : 0);
    auto doc_str = GenerateXrcStr(form_node, xrc_flags);
    wxMemoryInputStream stream(doc_str.c_str(), doc_str.size());
    wxScopedPtr<wxXmlDocument> xmlDoc(new wxXmlDocument(stream, "UTF-8"));
    if (!xmlDoc->IsOk())
    {
        wxMessageBox("Invalid XRC file generated -- it cannot be loaded.", "Compare");
        return false;
    }
    if (!wxXmlResource::Get()->LoadDocument(xmlDoc.release(), m_res_name))
    {
        wxMessageBox("wxWidgets could not parse the XRC data.", "Compare");
        return false;
    }

    return true;
}
