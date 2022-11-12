/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for generating XRC file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>  // wxConfig base header

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"      // MainFrame -- Main application window
#include "project_class.h"  // Project class

#include "generate_dlg.h"  // auto-generated: generatedlg_base.h and generatedlg_base.cpp

enum
{
    GEN_BASE_CODE = 1,
    GEN_INHERITED_CODE,
    GEN_LUA_CODE,
    GEN_PHP_CODE,
    GEN_PYTHON_CODE,
    GEN_XRC_CODE
};

// This generates the base class files. For the derived class files, see OnGenInhertedClass()
// in generate/gen_codefiles.cpp
void MainFrame::OnGenerateCode(wxCommandEvent&)
{
    GetProject()->UpdateEmbedNodes();
    GenerateDlg dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        long cur_setting = GEN_BASE_CODE;
        if (dlg.is_gen_inherited())
            cur_setting = GEN_INHERITED_CODE;
        else if (dlg.is_gen_lua())
            cur_setting = GEN_LUA_CODE;
        else if (dlg.is_gen_php())
            cur_setting = GEN_PHP_CODE;
        else if (dlg.is_gen_python())
            cur_setting = GEN_PYTHON_CODE;
        else if (dlg.is_gen_xrc())
            cur_setting = GEN_XRC_CODE;

        auto config = wxConfig::Get();
        config->Write("GenCode", cur_setting);

        switch (cur_setting)
        {
            case GEN_BASE_CODE:
                GenerateCodeFiles(this);
                break;

            case GEN_INHERITED_CODE:
                GenInhertedClass();
                break;

            case GEN_XRC_CODE:
                ExportXRC();
                break;
        }
        UpdateWakaTime();

        // m_menuTools->Enable(id_GenerateCode, !m_isProject_generated);
        // m_toolbar->EnableTool(id_GenerateCode, !m_isProject_generated);
    }
}

// If this constructor is used, the caller must call Create(parent)
GenerateDlg::GenerateDlg() {}

GenerateDlg::GenerateDlg(wxWindow* parent)
{
    Create(parent);
}

void GenerateDlg::OnInit(wxInitDialogEvent& event)
{

// TODO: [Randalphwa - 11-11-2022] It would be really helpful if we checked to see if any of these files should be written,
// and if so, added a " (*modified)" to the end of the relevant radio button.

    auto config = wxConfig::Get();
    auto cur_setting = config->ReadLong("GenCode", GEN_BASE_CODE);

    switch (cur_setting)
    {
        case GEN_BASE_CODE:
            m_gen_base_code = true;
            break;

        case GEN_INHERITED_CODE:
            m_gen_inherited_code = true;
            break;

        case GEN_LUA_CODE:
            m_gen_lua_code = true;
            break;

        case GEN_PHP_CODE:
            m_gen_php_code = true;
            break;

        case GEN_PYTHON_CODE:
            m_gen_python_code = true;
            break;

        case GEN_XRC_CODE:
            m_gen_xrc_code = true;
            break;
    }

    FindWindow(GetAffirmativeId())->SetLabel("Generate");

    event.Skip();
}
