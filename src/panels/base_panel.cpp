/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code code generation panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/stc/stc.h>      // A wxWidgets implementation of Scintilla.  This class is the

#include "base_panel.h"

#include "bitmaps.h"       // Map of bitmaps accessed by name
#include "code_display.h"  // CodeDisplay -- CodeDisplay class
#include "cstm_event.h"    // CustomEvent -- Custom Event class
#include "gen_base.h"      // Generate Base class
#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "write_code.h"    // Write code to Scintilla or file

// These are used everywhere we use scintilla to edit C++ code
const char* g_u8_cpp_keywords = "alignas alignof and and_eq atomic_cancel atomic_commit atomic_noexcept auto \
                              bitand bitor bool break case catch char char8_t char16_t char32_t \
                              class compl concept const consteval constexpr constinit const_cast \
                              continue co_await co_return co_yield __declspec \
	                          decltype default delete dllexport do double dynamic_cast else enum explicit \
	                          export extern false float for friend goto if inline int long \
	                          mutable namespace new noexcept not not_eq nullptr operator private or or_eq \
                              private protected public reflexpr register reinterpret_cast requires \
	                          return short signed sizeof static static_assert static_cast \
	                          struct switch synchronized template this thread_local throw true try typedef typeid \
	                          typename union unsigned using virtual void volatile wchar_t \
	                          while xor xor_eq";

BasePanel::BasePanel(wxWindow* parent, MainFrame* frame, bool GenerateDerivedCode) : wxPanel(parent)
{
    m_GenerateDerivedCode = GenerateDerivedCode;
    auto top_sizer = new wxBoxSizer(wxVERTICAL);

    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiGenericTabArt());

    m_cppPanel = new CodeDisplay(m_notebook);
    m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);

    m_hPanel = new CodeDisplay(m_notebook);
    m_notebook->AddPage(m_hPanel, "header", false, wxWithImages::NO_IMAGE);

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
    else if (text == "header")
    {
        m_hPanel->GetEventHandler()->ProcessEvent(event);
    }
}

void BasePanel::GenerateBaseClass()
{
    if (!IsShown())
        return;

    auto project = wxGetApp().GetProject();

    // If no form is selected, display the first child form of the project
    m_cur_form = wxGetFrame().GetSelectedForm();
    if (!m_cur_form)
    {
        if (project->GetChildCount() > 0)
        {
            m_cur_form = project->GetChild(0);
        }
        else
        {
            m_cppPanel->Clear();
            m_hPanel->Clear();
            return;
        }
    }

    BaseCodeGenerator codegen;

    Freeze();

    PANEL_TYPE panel_type = CPP_PANEL;
    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_hPanel)
        {
            panel_type = HDR_PANEL;
        }
    }

    m_cppPanel->Clear();
    codegen.SetSrcWriteCode(m_cppPanel);

    m_hPanel->Clear();
    codegen.SetHdrWriteCode(m_hPanel);

    if (m_GenerateDerivedCode)
        codegen.GenerateDerivedClass(project, m_cur_form, panel_type);
    else
        codegen.GenerateBaseClass(project, m_cur_form, panel_type);

    if (panel_type == CPP_PANEL)
    {
        m_cppPanel->CodeGenerationComplete();
        m_cppPanel->OnNodeSelected(wxGetFrame().GetSelectedNode());
    }
    else
    {
        m_hPanel->CodeGenerationComplete();
    }

    Thaw();
}

void BasePanel::OnNodeSelected(CustomEvent& event)
{
    if (!IsShown())
        return;

    auto form = event.GetNode();
    if (!form->IsForm())
        form = form->FindParentForm();

    if (form != m_cur_form)
    {
        m_cur_form = form;
        GenerateBaseClass();
    }

    if (!m_GenerateDerivedCode)
    {
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
}
