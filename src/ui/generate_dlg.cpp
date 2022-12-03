/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for choosing and generating specific language file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>  // wxConfig base header

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"      // MainFrame -- Main application window
#include "project_class.h"  // Project class

#include "generate_dlg.h"  // auto-generated: generatedlg_base.h and generatedlg_base.cpp

#include "../wxui/dlg_gen_results.h"

enum
{
    GEN_BASE_CODE = 1 << 0,
    GEN_INHERITED_CODE = 1 << 1,
    GEN_PYTHON_CODE = 1 << 2,
    GEN_XRC_CODE = 1 << 3
};

// This generates the base class files. For the derived class files, see OnGenInhertedClass()
// in generate/gen_codefiles.cpp
void MainFrame::OnGenerateCode(wxCommandEvent&)
{
    GetProject()->UpdateEmbedNodes();
    GenerateDlg dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        long cur_setting = 0;
        if (dlg.is_gen_base())
            cur_setting |= GEN_BASE_CODE;
        if (dlg.is_gen_inherited())
            cur_setting |= GEN_INHERITED_CODE;
        if (dlg.is_gen_python())
            cur_setting |= GEN_PYTHON_CODE;
        if (dlg.is_gen_xrc())
            cur_setting |= GEN_XRC_CODE;

        auto config = wxConfig::Get();
        config->Write("GenCode", cur_setting);

        GenResults results;

        if (cur_setting & GEN_BASE_CODE)
        {
            GenerateCodeFiles(results);
        }
        if (cur_setting & GEN_INHERITED_CODE)
        {
            GenInhertedClass(results);
        }
        if (cur_setting & GEN_PYTHON_CODE)
        {
            GeneratePythonFiles(results);
        }
        if (cur_setting & GEN_XRC_CODE)
        {
            GenerateXrcFiles(results);
        }

        if ((results.updated_files.size() || results.msgs.size()))
        {
            GeneratedResultsDlg results_dlg;
            results_dlg.Create(this);
            for (auto& iter: results.updated_files)
            {
                iter.make_relative(GetProject()->getProjectPath());
                results_dlg.m_lb_files->Append(iter);
            }

            if (results.updated_files.size() == 1)
                results.msgs.emplace_back("1 file was updated");
            else
                results.msgs.emplace_back() << results.updated_files.size() << " files were updated";

            for (auto& iter: results.msgs)
            {
                results_dlg.m_lb_info->Append(iter);
            }

            results_dlg.ShowModal();
        }
        else if (results.file_count)
        {
            ttlib::cstr msg;
            msg << '\n' << "All " << results.file_count << " generated files are current";
            wxMessageBox(msg, "Code Generation", wxOK, this);
        }

        UpdateWakaTime();
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
    // TODO: [Randalphwa - 11-11-2022] It would be really helpful if we checked to see if any of these files should be
    // written, and if so, added a " (*modified)" to the end of the relevant radio button.

    auto config = wxConfig::Get();

    if (GetProject()->as_string(prop_code_preference) == "Python")
    {
        m_gen_python_code = true;
        m_gen_base_code = false;
        m_gen_xrc_code = false;
    }
    else if (GetProject()->as_string(prop_code_preference) == "XRC")
    {
        m_gen_python_code = false;
        m_gen_base_code = false;
        m_gen_xrc_code = true;
    }
    else
    {
        m_gen_python_code = false;
        m_gen_base_code = true;
        m_gen_xrc_code = false;
    }

    auto cur_setting = config->ReadLong("GenCode", 0);

    if (cur_setting & GEN_BASE_CODE)
        m_gen_base_code = true;
    if (cur_setting & GEN_INHERITED_CODE)
        m_gen_inherited_code = true;
    if (cur_setting & GEN_PYTHON_CODE)
        m_gen_python_code = true;
    if (cur_setting & GEN_XRC_CODE)
        m_gen_xrc_code = true;

    event.Skip();
}
