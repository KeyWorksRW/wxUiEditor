/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <future>
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

inline constexpr const auto txt_XRC_HEADER = R"===(<?xml version="1.0"?>
<resource xmlns="http://www.wxwidgets.org/wxxrc" version="2.5.3.0">
)===";

inline constexpr const auto txt_XRC_FOOTER = R"===(</resource>
)===";

#if defined(XRC_ENABLED)

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

void GenXrcNode(Node* node, BaseCodeGenerator* code_gen)
{
    auto generator = node->GetNodeDeclaration()->GetGenerator();
    bool object_created = generator->GenXRC(node, code_gen);

    auto m_source = code_gen->GetSrcWriter();

    for (auto& child: node->GetChildNodePtrs())
    {
        m_source->Indent();
        GenXrcNode(child.get(), code_gen);
        m_source->Unindent();
    }
    if (object_created)
    {
        m_source->writeLine("</object>");
    }
}

void GenXrcInfo(Node* node, BaseCodeGenerator* code_gen)
{
    auto generator = node->GetNodeDeclaration()->GetGenerator();
    generator->GenXRCInfo(node, code_gen);

    for (auto& child: node->GetChildNodePtrs())
    {
        GenXrcInfo(child.get(), code_gen);
    }
}

#endif

void BaseCodeGenerator::GenerateXrcClass(Node* form_node, PANEL_TYPE panel_type)
{
#if defined(XRC_ENABLED)

    m_project = wxGetApp().GetProject();
    m_form_node = form_node;

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    m_source->writeLine(txt_XRC_HEADER);

    ttlib::cstr file;
    if (auto& base_file = form_node->prop_as_string(prop_base_file); base_file.size())
    {
        ttSaveCwd cwd;
        ttlib::ChangeDir(wxGetApp().getProjectPath());
        file = base_file;
        file.make_relative(wxGetApp().getProjectPath());
        file.backslashestoforward();
        file.remove_extension();

        m_baseFullPath = base_file;
        m_baseFullPath.make_absolute();
        m_baseFullPath.remove_filename();
    }

    if (m_panel_type != HDR_PANEL)
    {
        m_source->writeLine();
        GenXrcNode(form_node, this);
        m_source->writeLine();
        m_source->writeLine(txt_XRC_FOOTER);
    }
    else
    {
        GenXrcInfo(form_node, this);
    }

#endif  // defined(XRC_ENABLED)
}
