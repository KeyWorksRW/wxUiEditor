/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code code generation panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/stc/stc.h>      // A wxWidgets implementation of Scintilla.  This class is the
#include <wx/wupdlock.h>     // wxWindowUpdateLocker prevents window redrawing

#include "base_panel.h"

#include "bitmaps.h"          // Map of bitmaps accessed by name
#include "code_display.h"     // CodeDisplay -- CodeDisplay class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "gen_base.h"         // Generate Base class
#include "gen_xrc.h"          // BaseXrcGenerator -- Generate XRC file
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "project_handler.h"  // ProjectHandler class
#include "write_code.h"       // Write code to Scintilla or file

// These are used everywhere we use scintilla to edit C++ code
const char* g_u8_cpp_keywords = "alignas alignof and and_eq atomic_cancel atomic_commit atomic_noexcept auto"
                                " bitand bitor bool break case catch char char8_t char16_t char32_t"
                                " class compl concept const consteval constexpr constinit const_cast"
                                " continue co_await co_return co_yield __declspec"
                                " decltype default delete dllexport do double dynamic_cast else enum explicit"
                                " export extern false float for friend goto if inline int long"
                                " mutable namespace new noexcept not not_eq nullptr operator private or or_eq"
                                " private protected public reflexpr register reinterpret_cast requires"
                                " return short signed sizeof static static_assert static_cast"
                                " struct switch synchronized template this thread_local throw true try typedef typeid"
                                " typename union unsigned using virtual void volatile wchar_t"
                                " while xor xor_eq";

BasePanel::BasePanel(wxWindow* parent, MainFrame* frame, int panel_type) : wxPanel(parent)
{
    m_panel_type = panel_type;
    auto top_sizer = new wxBoxSizer(wxVERTICAL);

    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiGenericTabArt());

    if (m_panel_type == GEN_LANG_CPLUSPLUS)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);

        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "header", false, wxWithImages::NO_IMAGE);

        m_derived_src_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_derived_src_panel, "derived_src", false, wxWithImages::NO_IMAGE);

        m_derived_hdr_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_derived_hdr_panel, "derived_hdr", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_PYTHON)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);

        // A lot of code expects m_hPanel to exist. This will give us something to add additional information to, such as
        // which properties are not supported.

        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "inherit", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_RUBY)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);

        // A lot of code expects m_hPanel to exist. This will give us something to add additional information to, such as
        // which properties are not supported.

        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "inherit", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_XRC)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);

        // A lot of code expects m_hPanel to exist. This will give us something to add additional information to, such as
        // which properties are not supported.

        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
    else
    {
        FAIL_MSG("Unknown Panel type!")

        // Add default panel creation just to prevent crashing

        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);

        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "header", false, wxWithImages::NO_IMAGE);
    }

    top_sizer->Add(m_notebook, wxSizerFlags(1).Expand());

    SetSizerAndFit(top_sizer);

    Bind(wxEVT_FIND, &BasePanel::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &BasePanel::OnFind, this);

    Bind(EVT_EventHandlerChanged,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_GridBagAction,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_NodeCreated,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_NodeDeleted,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_NodePropChange,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_ParentChanged,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_PositionChanged,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_ProjectUpdated,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });
    Bind(EVT_MultiPropChange,
         [this](wxEvent&)
         {
             GenerateBaseClass();
         });

    Bind(EVT_NodeSelected, &BasePanel::OnNodeSelected, this);

    frame->AddCustomEventHandler(GetEventHandler());
}

BasePanel::~BasePanel() {}

void BasePanel::OnFind(wxFindDialogEvent& event)
{
    auto notebook = wxStaticCast(m_cppPanel->GetParent(), wxAuiNotebook);
    ASSERT(notebook);

    auto text = notebook->GetPageText(notebook->GetSelection());
    if (text == "source")
    {
        m_cppPanel->GetEventHandler()->ProcessEvent(event);
    }
    else if (text == "derived_src")
    {
        m_derived_src_panel->GetEventHandler()->ProcessEvent(event);
    }
    else if (text == "header" || text == "inherit" || text == "info")
    {
        m_hPanel->GetEventHandler()->ProcessEvent(event);
    }
    else if (text == "derived_hdr")
    {
        m_derived_hdr_panel->GetEventHandler()->ProcessEvent(event);
    }
}

PANEL_PAGE BasePanel::GetPanelPage() const
{
    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_cppPanel)
            return CPP_PANEL;
        else if (page == m_hPanel)
            return HDR_PANEL;
        else if (page == m_derived_src_panel)
            return DERIVED_SRC_PANEL;
        else if (page == m_derived_hdr_panel)
            return DERIVED_HDR_PANEL;
    }
    return CPP_PANEL;
}

void BasePanel::GenerateBaseClass()
{
    if (!IsShown())
        return;

    // If no form is selected, display the first child form of the project
    m_cur_form = wxGetFrame().GetSelectedForm();
    if (!m_cur_form)
    {
        auto* cur_selection = wxGetFrame().GetSelectedNode();
        if ((cur_selection->isGen(gen_folder) || cur_selection->isGen(gen_sub_folder)) && cur_selection->getChildCount() > 0)
        {
            m_cur_form = cur_selection->getChild(0);
        }
        else if (Project.ChildCount() > 0)
        {
            m_cur_form = Project.GetFirstFormChild();
        }
        else
        {
            m_cppPanel->Clear();
            m_hPanel->Clear();
            if (m_derived_src_panel)
                m_derived_src_panel->Clear();
            if (m_derived_hdr_panel)
                m_derived_hdr_panel->Clear();
            return;
        }
    }

    wxWindowUpdateLocker freeze(this);

    PANEL_PAGE panel_page = CPP_PANEL;
    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_hPanel || page == m_derived_hdr_panel)
        {
            panel_page = HDR_PANEL;
        }
    }

    BaseCodeGenerator codegen(m_panel_type);

    m_cppPanel->Clear();
    codegen.SetSrcWriteCode(m_cppPanel);

    m_hPanel->Clear();
    codegen.SetHdrWriteCode(m_hPanel);

    switch (m_panel_type)
    {
        case GEN_LANG_CPLUSPLUS:
            codegen.GenerateCppClass(m_cur_form, panel_page);

            m_derived_src_panel->Clear();
            codegen.SetSrcWriteCode(m_derived_src_panel);
            m_derived_hdr_panel->Clear();
            codegen.SetHdrWriteCode(m_derived_hdr_panel);

            codegen.GenerateDerivedClass(Project.ProjectNode(), m_cur_form, panel_page);
            break;

        case GEN_LANG_PYTHON:
            codegen.GeneratePythonClass(m_cur_form, panel_page);
            break;

        case GEN_LANG_RUBY:
            codegen.GenerateRubyClass(m_cur_form, panel_page);
            break;

        case GEN_LANG_XRC:
            codegen.GenerateXrcClass(m_cur_form, panel_page);
            break;

        default:
            FAIL_MSG("Unknown panel type!")
            break;
    }

    if (panel_page == CPP_PANEL)
    {
        m_cppPanel->CodeGenerationComplete();
        m_cppPanel->OnNodeSelected(wxGetFrame().GetSelectedNode());
        if (m_panel_type == GEN_LANG_CPLUSPLUS)
        {
            m_derived_src_panel->CodeGenerationComplete();
            m_derived_src_panel->OnNodeSelected(wxGetFrame().GetSelectedNode());
        }
    }
    else
    {
        m_hPanel->CodeGenerationComplete();
        if (m_panel_type == GEN_LANG_CPLUSPLUS)
        {
            m_derived_hdr_panel->CodeGenerationComplete();
        }
    }
}

void BasePanel::OnNodeSelected(CustomEvent& event)
{
    if (!IsShown())
        return;

    auto form = event.GetNode()->getForm();

    if (form != m_cur_form)
    {
        m_cur_form = form;
        GenerateBaseClass();
    }

    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_hPanel)
        {
            m_hPanel->OnNodeSelected(event.GetNode());
        }
        else
        {
            m_cppPanel->OnNodeSelected(event.GetNode());
        }
    }
}
