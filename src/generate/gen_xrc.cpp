/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <future>
#include <thread>
#include <unordered_set>

#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "gen_base.h"

#include "gen_common.h"   // GeneratorLibrary -- Generator classes
#include "mainapp.h"      // App -- Main application class
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
