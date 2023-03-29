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

#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class

// Defined in mockup_preview.cpp
void CreateMockupChildren(Node* node, wxWindow* parent, wxObject* parentNode, wxSizer* parent_sizer, wxWindow* form_window);

// Defined in gen_xrc.cpp
std::string GenerateXrcStr(Node* node_start, size_t xrc_flags);

extern const char* txt_dlg_name;

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
    if (!XrcCompareBase::Create(parent, wxID_ANY, "Compare C++/XRC Generated UI", wxDefaultPosition, wxDefaultSize,
                                wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER))
        return false;

        // Everything below up to the customization section is a direct copy of the XrcCompareBase::Create() function
#if 0
    m_grid_bag_sizer = new wxGridBagSizer();

    auto* staticText = new wxStaticText(this, wxID_ANY, "C++ Generated");
    m_grid_bag_sizer->Add(staticText, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALL, 5);

    m_static_line =
        new wxStaticLine(this, wxID_ANY, wxDefaultPosition, ConvertDialogToPixels(wxSize(-1, 100)), wxLI_VERTICAL);
    m_grid_bag_sizer->Add(m_static_line, wxGBPosition(0, 1), wxGBSpan(2, 1), wxALL, 5);

    auto* staticText_2 = new wxStaticText(this, wxID_ANY, "XRC Generated");
    m_grid_bag_sizer->Add(staticText_2, wxGBPosition(0, 2), wxGBSpan(1, 1), wxALL, 5);
#endif
    // Customization section

    // TODO: [KeyWorks - 06-09-2022] Add the C++ and XRC top level sizers here

    auto xrc_resource = wxXmlResource::Get();

    switch (form_node->gen_name())
    {
        case gen_PanelForm:
            {
                CreateMockupChildren(form_node, this, nullptr, m_grid_bag_sizer, this);

                if (!InitXrc(form_node))
                    return false;

                if (auto object = xrc_resource->LoadObject(this, form_node->value(prop_class_name), "wxPanel"); object)
                {
                    m_grid_bag_sizer->Add(wxStaticCast(object, wxPanel), wxGBPosition(1, 2), wxGBSpan(1, 1), wxALL, 5);
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
                CreateMockupChildren(form_node, this, nullptr, m_grid_bag_sizer, this);

                // In theory, we should be able to start from the sizer and pass that to m_grid_bag_sizer. In practice, it
                // causes wxWidgets to crash. I'm not sure why, but setting both the C++ and XRC generators to use wxPanel
                // solves the problem.

                // GenerateXrcStr will return a wxPanel using the name txt_dlg_name ("_wxue_temp_dlg")
                if (!InitXrc(form_node))
                    return false;

                if (auto object = xrc_resource->LoadObject(this, txt_dlg_name, "wxPanel"); object)
                {
                    m_grid_bag_sizer->Add(wxStaticCast(object, wxPanel), wxGBPosition(1, 2), wxGBSpan(1, 1), wxALL, 5);
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

    SetSizerAndFit(m_grid_bag_sizer);
    // Centre(wxBOTH);
    // Fit();

    wxPersistentRegisterAndRestore(this, "XrcCompare");

    return true;
}

bool XrcCompare::InitXrc(Node* form_node)
{
    size_t xrc_flags = (form_node->isGen(gen_wxDialog) ? xrc::previewing : 0);
    auto doc_str = GenerateXrcStr(form_node, xrc_flags);
    wxMemoryInputStream stream(doc_str.c_str(), doc_str.size());
    auto xmlDoc = std::make_unique<wxXmlDocument>(stream, "UTF-8");
    // wxScopedPtr<wxXmlDocument> xmlDoc(new wxXmlDocument(stream, "UTF-8"));
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
