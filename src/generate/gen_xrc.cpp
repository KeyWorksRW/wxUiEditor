/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <future>
#include <sstream>
#include <thread>
#include <unordered_set>

#include <wx/filename.h>    // wxFileName - encapsulates a file path
#include <wx/mstream.h>     // Memory stream classes
#include <wx/xml/xml.h>     // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>  // XML resources

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "gen_base.h"

#include "gen_common.h"   // GeneratorLibrary -- Generator classes
#include "mainapp.h"      // App -- Main application class
#include "mainframe.h"    // MainFrame -- Main window frame
#include "node.h"         // Node class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties
#include "write_code.h"   // Write code to Scintilla or file

#include "pugixml.hpp"

inline constexpr const auto txt_XRC_HEADER = R"===(<?xml version="1.0"?>
<resource xmlns="http://www.wxwidgets.org/wxxrc" version="2.5.3.0">
)===";

inline constexpr const auto txt_XRC_FOOTER = R"===(</resource>
)===";

static bool s_isXmlInitalized { false };

void MainFrame::OnPreviewXrc(wxCommandEvent& /* event */)
{
    if (!m_selected_node)
    {
        wxMessageBox("You need to select a dialog first.", "XRC Dialog Preview");
        return;
    }

    auto form_node = m_selected_node.get();
    if (!form_node->IsForm())
    {
        if (form_node->isGen(gen_Project) && form_node->GetChildCount())
        {
            form_node = form_node->GetChild(0);
        }
        else
        {
            form_node = form_node->get_form();
        }
    }

    if (!form_node->isGen(gen_wxDialog))
    {
        wxMessageBox("Only dialogs can be previewed.", "XRC Dialog Preview");
        return;
    }

    try
    {
        BaseCodeGenerator codegen;

        // We probably won't ever use h_cw, but BaseCodeGenerator expects it to exist, so this avoids adding a bunch of
        // conditional code to determine if it actually exists or not.
        auto h_cw = std::make_unique<FileCodeWriter>("XRC-info");
        codegen.SetHdrWriteCode(h_cw.get());

        auto xrc_cw = std::make_unique<FileCodeWriter>("XRC");
        codegen.SetSrcWriteCode(xrc_cw.get());
        codegen.GenerateXrcClass(form_node);

        wxMemoryInputStream stream(xrc_cw->GetString().c_str(), xrc_cw->GetString().size());
        wxScopedPtr<wxXmlDocument> xmlDoc(new wxXmlDocument(stream, "UTF-8"));
        if (!xmlDoc->IsOk())
        {
            wxMessageBox("Invalid XRC file generated -- it cannot be loaded.", "XRC Dialog Preview");
            return;
        }

        auto xrc_resource = wxXmlResource::Get();

        if (!s_isXmlInitalized)
        {
            xrc_resource->InitAllHandlers();
            s_isXmlInitalized = true;
        }

        wxString res_name("wxuiPreview");

        if (!xrc_resource->LoadDocument(xmlDoc.release(), res_name))
        {
            wxMessageBox("wxWidgets could not parse the XRC data.", "XRC Dialog Preview");
            return;
        }

        wxDialog dlg;
        if (xrc_resource->LoadDialog(&dlg, this, form_node->prop_as_string(prop_class_name)))
        {
            dlg.ShowModal();
        }
        else
        {
            wxMessageBox(ttlib::cstr("Could not load ") << form_node->prop_as_string(prop_class_name) << " resource.",
                         "XRC Dialog Preview");
        }
        xrc_resource->Unload(res_name);
    }
    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox("An internal error occurred generating XRC code", "XRC Dialog Preview");
    }
}

int GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto generator = node->GetNodeDeclaration()->GetGenerator();
    auto result = generator->GenXrcObject(node, object, add_comments);
    if (result == BaseGenerator::xrc_sizer_item_created)
    {
        auto actual_object = object.child("object");
        for (auto& child: node->GetChildNodePtrs())
        {
            auto child_object = actual_object.append_child("object");
            auto child_result = GenXrcObject(child.get(), child_object, add_comments);
            if (child_result == BaseGenerator::xrc_not_supported)
            {
                actual_object.remove_child(child_object);
                break;
            }
        }
        return result;
    }
    else if (result == BaseGenerator::xrc_updated)
    {
        for (auto& child: node->GetChildNodePtrs())
        {
            auto child_object = object.append_child("object");
            auto child_result = GenXrcObject(child.get(), child_object, add_comments);
            if (child_result == BaseGenerator::xrc_not_supported)
            {
                object.remove_child(child_object);
                break;
            }
        }
        return result;
    }
    else
    {
        return BaseGenerator::xrc_not_supported;
    }
}

void CollectHandlers(Node* node, std::set<std::string>& handlers)
{
    auto generator = node->GetNodeDeclaration()->GetGenerator();
    generator->RequiredHandlers(node, handlers);
    for (auto& child: node->GetChildNodePtrs())
    {
        generator = child->GetNodeDeclaration()->GetGenerator();
        generator->RequiredHandlers(child.get(), handlers);
        if (child->GetChildCount())
        {
            CollectHandlers(child.get(), handlers);
        }
    }
}

void BaseCodeGenerator::GenerateXrcClass(Node* form_node, PANEL_TYPE panel_type)
{
    m_project = wxGetApp().GetProject();
    m_form_node = form_node;

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    pugi::xml_document doc;
    auto root = doc.append_child("resource");
    root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
    root.append_attribute("version") = "2.5.3.0";

    auto object = root.append_child("object");
    auto form_result = GenXrcObject(form_node, object, m_panel_type == CPP_PANEL);
    if (form_result == BaseGenerator::xrc_not_supported)
    {
        root.remove_child(object);
    }

    if (m_panel_type != HDR_PANEL)
    {
        if (form_result != BaseGenerator::xrc_not_supported)
        {
            std::ostringstream xml_stream;
            doc.save(xml_stream, "\t");
            std::string str = xml_stream.str();
            m_source->doWrite(str);
        }
        else
        {
            m_source->writeLine("This form cannot be output to XRC.");
        }
    }
    else
    {
        if (form_result != BaseGenerator::xrc_not_supported)
        {
            m_header->writeLine(ttlib::cstr("Resource name is ") << form_node->prop_as_string(prop_class_name));
            m_header->writeLine();
            m_header->writeLine("Required handlers:");
            m_header->writeLine();
            m_header->Indent();

            std::set<std::string> handlers;
            CollectHandlers(form_node, handlers);
            for (auto& iter: handlers)
            {
                m_header->writeLine(iter);
            }
        }
        else
        {
            m_header->writeLine("This form cannot be output to XRC.");
        }
    }
}
