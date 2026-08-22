/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++/XRC UI Comparison dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-22-2026]

#include <wx/mstream.h>           // Memory stream classes
#include <wx/persist.h>           // common classes for persistence support
#include <wx/persist/toplevel.h>  // persistence support for wxTLW
#include <wx/stattext.h>          // wxStaticText base header
#include <wx/stc/stc.h>           // A wxWidgets implementation of Scintilla.
#include <wx/xml/xml.h>           // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>        // XML resources

// The following handlers must be explicitly added

#include <wx/xrc/xh_aui.h>             // XRC resource handler for wxAUI
#include <wx/xrc/xh_auitoolb.h>        // XML resource handler for wxAuiToolBar
#include <wx/xrc/xh_ribbon.h>          // XML resource handler for wxRibbon related classes
#include <wx/xrc/xh_richtext.h>        // XML resource handler for wxRichTextCtrl
#include <wx/xrc/xh_styledtextctrl.h>  // XML resource handler for wxStyledTextCtrl

#include "xrccompare.h"  // includes xrccompare_base.h

#include "../internal/import_panel.h"    // ImportPanel -- Panel to display original imported file
#include "mainframe.h"                   // MainFrame -- Main window frame
#include "node.h"                        // Node class
#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "../generate/writers/gen_xrc.h"  // GenerateXrcStr, txt_dlg_name

XrcCompare::XrcCompare() {}

// Defined in mockup_preview.cpp
void CreateMockupChildren(Node* node, wxWindow* parent, wxObject* parentNode, wxSizer* parent_sizer,
                          wxWindow* form_window);

XrcCompare::~XrcCompare()
{
    if (m_created)
    {
        // In import mode both the generated and the import documents are loaded under
        // separate names, so both must be unloaded to keep the global pool clean.
        wxXmlResource* xrc_resource = wxXmlResource::Get();
        std::ignore = xrc_resource->Unload(m_res_name);
        std::ignore = xrc_resource->Unload(m_import_res_name);
    }
}

bool XrcCompare::DoCreate(wxWindow* parent, Node* form_node, bool compare_import)
{
    m_compare_import = compare_import;

    const std::string title =
        m_compare_import ? "Compare Import/Generated XRC" : "Compare C++/XRC Generated UI";
    if (!Create(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER))
    {
        return false;
    }

    if (m_compare_import)
    {
        m_staticTextLeft->SetLabel("Imported XRC");
        m_staticTextRight->SetLabel("Generated XRC");
    }
    else
    {
        m_staticTextLeft->SetLabel("C++ Generated");
        m_staticTextRight->SetLabel("XRC Generated");
    }

    // Customization section

    // TODO: [KeyWorks - 06-09-2022] Add the C++ and XRC top level sizers here

    m_created = true;

    wxXmlResource* xrc_resource = wxXmlResource::Get();

    // InitAllHandlers() is not idempotent (it always AddHandler()s ~75 handlers), so guard it
    // the same way previews.cpp does to avoid unbounded handler growth per invocation.
    static bool g_xrc_handlers_initialized { false };
    if (!g_xrc_handlers_initialized)
    {
        g_xrc_handlers_initialized = true;
        xrc_resource->InitAllHandlers();
        xrc_resource->AddHandler(new wxRichTextCtrlXmlHandler);
        xrc_resource->AddHandler(new wxAuiXmlHandler);
        xrc_resource->AddHandler(new wxAuiToolBarXmlHandler);
        xrc_resource->AddHandler(new wxRibbonXmlHandler);
        xrc_resource->AddHandler(new wxStyledTextCtrlXmlHandler);
    }

    // Distinct names for the generated and import documents -- wxXmlResource::Unload() only
    // removes the FIRST matching record, so a shared name would leave one document registered.
    m_res_name = "wxuiCompare";
    m_import_res_name = m_res_name + "_import";

    switch (form_node->get_GenName())
    {
        case gen_PanelForm:
            {
                if (m_compare_import)
                {
                    if (!InitImport(form_node))
                    {
                        return false;
                    }
                }
                else
                {
                    CreateMockupChildren(form_node, this, nullptr, m_grid_bag_sizer, this);
                }

                if (!InitXrc(form_node))
                {
                    return false;
                }

                if (auto* object = xrc_resource->LoadObject(
                        this, form_node->as_string(prop_class_name), "wxPanel");
                    object)
                {
                    m_grid_bag_sizer->Add(wxStaticCast(object, wxPanel), wxGBPosition(1, 2),
                                          wxGBSpan(1, 1), wxALL, 5);
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
                if (form_node->get_ChildCount() < 1)
                {
                    wxMessageBox("Only a dialog's children can be shown -- this dialog has none.",
                                 "Compare");
                    return false;
                }

                if (m_compare_import)
                {
                    if (!InitImport(form_node))
                    {
                        return false;
                    }
                }
                else
                {
                    // The wxDialog generator will create a wxPanel as the mockup
                    CreateMockupChildren(form_node, this, nullptr, m_grid_bag_sizer, this);
                }

                // In theory, we should be able to start from the sizer and pass that to
                // m_grid_bag_sizer. In practice, it causes wxWidgets to crash. I'm not sure why,
                // but setting both the C++ and XRC generators to use wxPanel solves the problem.

                // GenerateXrcStr will return a wxPanel using the name txt_dlg_name
                // ("_wxue_temp_dlg")
                if (!InitXrc(form_node))
                {
                    return false;
                }

                if (m_compare_import)
                {
                    if (auto* object = xrc_resource->LoadObject(
                            this, wxue::string(form_node->as_string(prop_class_name)) << "_import",
                            "wxPanel");
                        object)
                    {
                        m_grid_bag_sizer->Add(wxStaticCast(object, wxPanel), wxGBPosition(1, 0),
                                              wxGBSpan(1, 1), wxALL, 5);
                    }
                    else
                    {
                        wxMessageBox("Could not load top level sizer", "Compare");
                        return false;
                    }
                }

                if (auto* object = xrc_resource->LoadObject(this, txt_dlg_name, "wxPanel"); object)
                {
                    m_grid_bag_sizer->Add(wxStaticCast(object, wxPanel), wxGBPosition(1, 2),
                                          wxGBSpan(1, 1), wxALL, 5);
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
    const size_t xrc_flags = (form_node->is_Gen(gen_wxDialog) ? xrc::previewing : 0);
    const std::string doc_str = GenerateXrcStr(form_node, xrc_flags);
    wxMemoryInputStream stream(doc_str.c_str(), doc_str.size());
    if (!stream.IsOk())
    {
        wxMessageBox("Unable to create the XRC memory stream -- it cannot be loaded.", "Compare");
        return false;
    }
    auto xmlDoc = std::make_unique<wxXmlDocument>(stream);
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

bool XrcCompare::InitImport(Node* form_node)
{
    ImportPanel* import_panel = wxGetFrame().getImportPanel();
    if (!import_panel || !import_panel->GetTextCtrl())
    {
        wxMessageBox("The import panel is not available -- cannot compare.", "Compare");
        return false;
    }
    wxue::string xrc_text = import_panel->GetTextCtrl()->GetText().utf8_string();
    if (form_node->get_GenName() == gen_wxDialog)
    {
        // Because we need to place this within sizer, we switch the class to a wxPanel. We assume
        // it will work in spite of having child attributes that only apply to wxDialog.
        if (xrc_text.Replace("\"wxDialog", "\"wxPanel") == 0)
        {
            wxMessageBox("The imported XRC does not contain a wxDialog class -- cannot compare.",
                         "Compare");
            return false;
        }
    }

    // We need to change the name since it will be identical to the generated name
    wxue::string org_name("\"");
    org_name << form_node->as_string(prop_class_name);
    const size_t name_replacements =
        xrc_text.Replace(org_name, wxue::string(org_name) << "_import");
    if (name_replacements == 0)
    {
        wxMessageBox("The imported XRC does not contain the expected class name -- cannot compare.",
                     "Compare");
        return false;
    }

    wxMemoryInputStream stream(xrc_text.c_str(), xrc_text.size());
    if (!stream.IsOk())
    {
        wxMessageBox("Unable to create the XRC memory stream -- it cannot be loaded.", "Compare");
        return false;
    }
    auto xmlDoc = std::make_unique<wxXmlDocument>(stream);
    if (!xmlDoc->IsOk())
    {
        wxMessageBox("Invalid XRC file generated -- it cannot be loaded.", "Compare");
        return false;
    }
    if (!wxXmlResource::Get()->LoadDocument(xmlDoc.release(), m_import_res_name))
    {
        wxMessageBox("wxWidgets could not parse the XRC data.", "Compare");
        return false;
    }

    return true;
}
