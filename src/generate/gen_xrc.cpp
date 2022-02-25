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

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings
#include "ttstr.h"       // ttString, ttSaveCwd -- Enhanced version of wxString
#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "gen_base.h"

#include "mainapp.h"      // App -- Main application class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties
#include "write_code.h"   // Write code to Scintilla or file

inline constexpr const auto txt_XRC_HEADER = R"===(<?xml version="1.0"?>
<resource xmlns="http://www.wxwidgets.org/wxxrc" version="2.5.3.0">
)===";

inline constexpr const auto txt_XRC_FOOTER = R"===(</resource>
)===";

void BaseCodeGenerator::GenerateXrcClass(Node* /* project */, Node* form_node, PANEL_TYPE panel_type)
{
    m_form_node = form_node;

    // If the code files are being written to disk, then UpdateEmbedNodes() has already been called.
    if (panel_type != NOT_PANEL)
    {
        wxGetApp().GetProjectSettings()->UpdateEmbedNodes();
    }

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    m_source->writeLine(txt_XRC_HEADER);

    m_source->writeLine(txt_XRC_FOOTER);
}
