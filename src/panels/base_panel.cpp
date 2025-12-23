/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code code generation panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/stc/stc.h>      // A wxWidgets implementation of Scintilla.  This class is the
#include <wx/wupdlock.h>     // wxWindowUpdateLocker prevents window redrawing

#include "base_panel.h"

#include "code_display.h"     // CodeDisplay -- CodeDisplay class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "gen_xrc.h"          // BaseXrcGenerator -- Generate XRC file
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "gen_cpp.h"      // CppCodeGenerator -- Generate C++ code
#include "gen_perl.h"     // PerlCodeGenerator class
#include "gen_python.h"   // PythonCodeGenerator -- Generate wxPython code
#include "gen_results.h"  // GenResults -- Code generation orchestrator
#include "gen_ruby.h"     // RubyCodeGenerator -- Generate wxRuby code
#include "gen_xrc.h"      // XrcGenerator -- Generate XRC code

// These are used everywhere we use scintilla to edit C++ code. It is also used to verify valid
// var_name values.
const char* g_u8_cpp_keywords =
    "alignas alignof and and_eq atomic_cancel atomic_commit atomic_noexcept auto"
    " bitand bitor bool break case catch char char8_t char16_t char32_t"
    " class compl concept const consteval constexpr constinit const_cast"
    " continue co_await co_return co_yield __declspec"
    " decltype default delete dllexport do double dynamic_cast else enum explicit"
    " export extern false float for friend goto if inline int interface long"
    " mutable namespace new noexcept not not_eq nullptr operator private or or_eq"
    " private protected public reflexpr register reinterpret_cast requires"
    " return short signed sizeof static static_assert static_cast"
    " struct switch synchronized template this thread_local throw true try typedef typeid"
    " typename union unsigned using virtual void volatile wchar_t"
    " while xor xor_eq";

const char* g_python_keywords =
    "False None True and as assert async break class continue def del elif else except finally "
    "for from global if import in is lambda "
    "nonlocal not or pass raise return self try while with yield";

const char* g_ruby_keywords =
    "ENCODING LINE FILE BEGIN END alias and begin break case class def defined do else"
    " elsif end ensure false for if in module next nil not or redo require rescue retry"
    " return self super then true undef unless until when while yield";

const char* g_perl_keywords =
    "do if else elsif unless while until for foreach last next pod cut redo continue "
    "qw sub return goto and or not xor "
    "unless use no package require my our local state ";

BasePanel::BasePanel(wxWindow* parent, MainFrame* frame, GenLang panel_type) : wxPanel(parent)
{
    m_panel_type = panel_type;
    auto top_sizer = new wxBoxSizer(wxVERTICAL);

    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiGenericTabArt());

    // Note that a lot of code assumes m_hdr_info_panel is valid. It is up to the language
    // generator to generate inherited classes, or just generate generation information about
    // the class.

    if (m_panel_type == GEN_LANG_CPLUSPLUS)
    {
        m_source_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_source_panel, "source", false, wxWithImages::NO_IMAGE);

        m_hdr_info_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hdr_info_panel, "header", false, wxWithImages::NO_IMAGE);

        m_derived_src_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_derived_src_panel, "derived_src", false, wxWithImages::NO_IMAGE);

        m_derived_hdr_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_derived_hdr_panel, "derived_hdr", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_PERL)
    {
        m_source_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_source_panel, "source", false, wxWithImages::NO_IMAGE);
        m_hdr_info_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hdr_info_panel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_PYTHON)
    {
        m_source_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_source_panel, "source", false, wxWithImages::NO_IMAGE);
        m_hdr_info_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hdr_info_panel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_RUBY)
    {
        m_source_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_source_panel, "source", false, wxWithImages::NO_IMAGE);
        m_hdr_info_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hdr_info_panel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_XRC)
    {
        m_source_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_source_panel, "source", false, wxWithImages::NO_IMAGE);
        m_hdr_info_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hdr_info_panel, "info", false, wxWithImages::NO_IMAGE);
    }
    else
    {
        FAIL_MSG("Unknown Panel type!")

        // Add default panel creation just to prevent crashing

        m_source_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_source_panel, "source", false, wxWithImages::NO_IMAGE);

        m_hdr_info_panel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hdr_info_panel, "header", false, wxWithImages::NO_IMAGE);
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

BasePanel::~BasePanel()
{
    wxGetMainFrame()->RemoveCustomEventHandler(GetEventHandler());
}

wxString BasePanel::GetSelectedText()
{
    auto notebook = wxStaticCast(m_source_panel->GetParent(), wxAuiNotebook);
    auto text = notebook->GetPageText(notebook->GetSelection());
    if (text == "source")
    {
        return m_source_panel->GetTextCtrl()->GetSelectedText();
    }
    else if (text == "derived_src")
    {
        return m_derived_src_panel->GetTextCtrl()->GetSelectedText();
    }
    else if (text == "header" || text == "inherit" || text == "info")
    {
        return m_hdr_info_panel->GetTextCtrl()->GetSelectedText();
    }
    else if (text == "derived_hdr")
    {
        return m_derived_hdr_panel->GetTextCtrl()->GetSelectedText();
    }

    return {};
}

void BasePanel::OnFind(wxFindDialogEvent& event)
{
    auto notebook = wxStaticCast(m_source_panel->GetParent(), wxAuiNotebook);
    ASSERT(notebook);

    auto text = notebook->GetPageText(notebook->GetSelection());
    if (text == "source")
    {
        m_source_panel->GetEventHandler()->ProcessEvent(event);
    }
    else if (text == "derived_src")
    {
        m_derived_src_panel->GetEventHandler()->ProcessEvent(event);
    }
    else if (text == "header" || text == "inherit" || text == "info")
    {
        m_hdr_info_panel->GetEventHandler()->ProcessEvent(event);
    }
    else if (text == "derived_hdr")
    {
        m_derived_hdr_panel->GetEventHandler()->ProcessEvent(event);
    }
}

PANEL_PAGE BasePanel::GetPanelPage() const
{
    auto* top_notebook = wxGetFrame().getTopNotebook();
    auto* child_panel = static_cast<BasePanel*>(top_notebook->GetCurrentPage());
    if (auto* page = child_panel->m_notebook->GetCurrentPage(); page)
    {
        if (page == child_panel->m_source_panel)
            return PANEL_PAGE::SOURCE_PANEL;
        else if (page == child_panel->m_hdr_info_panel)
            return PANEL_PAGE::HDR_INFO_PANEL;
        else if (page == child_panel->m_derived_src_panel)
            return PANEL_PAGE::DERIVED_SRC_PANEL;
        else if (page == child_panel->m_derived_hdr_panel)
            return PANEL_PAGE::DERIVED_HDR_PANEL;
    }
    return PANEL_PAGE::SOURCE_PANEL;
}

void BasePanel::GenerateBaseClass()
{
    if (!IsShown())
        return;

    // If no form is selected, display the first child form of the project
    m_cur_form = wxGetFrame().getSelectedForm();
    if (!m_cur_form)
    {
        auto* cur_selection = wxGetFrame().getSelectedNode();
        if ((cur_selection->is_Gen(gen_folder) || cur_selection->is_Gen(gen_sub_folder)) &&
            cur_selection->get_ChildCount() > 0)
        {
            m_cur_form = cur_selection->get_Child(0);
        }
        else if (Project.get_ChildCount() > 0)
        {
            m_cur_form = Project.get_FirstFormChild();
        }
        else
        {
            m_source_panel->Clear();
            m_hdr_info_panel->Clear();
            if (m_derived_src_panel)
                m_derived_src_panel->Clear();
            if (m_derived_hdr_panel)
                m_derived_hdr_panel->Clear();
            return;
        }
    }

    wxWindowUpdateLocker freeze(this);

    PANEL_PAGE panel_page = PANEL_PAGE::SOURCE_PANEL;
    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_hdr_info_panel)
        {
            panel_page = PANEL_PAGE::HDR_INFO_PANEL;
        }
        else if (page == m_derived_src_panel)
        {
            panel_page = PANEL_PAGE::DERIVED_SRC_PANEL;
        }
        else if (page == m_derived_hdr_panel)
        {
            panel_page = PANEL_PAGE::DERIVED_HDR_PANEL;
        }
    }

    // All languages except C++ derived panels use GenResults for unified code generation
    // C++ base class panels (SOURCE_PANEL, HDR_INFO_PANEL) also use GenResults
    if (m_panel_type != GEN_LANG_CPLUSPLUS || panel_page == PANEL_PAGE::SOURCE_PANEL ||
        panel_page == PANEL_PAGE::HDR_INFO_PANEL)
    {
        m_source_panel->Clear();
        m_hdr_info_panel->Clear();

        GenResults results;
        if (results.SetDisplayTarget(m_cur_form, m_panel_type, m_source_panel, m_hdr_info_panel,
                                     panel_page))
        {
            std::ignore = results.Generate();
        }

        if (panel_page == PANEL_PAGE::SOURCE_PANEL || panel_page == PANEL_PAGE::HDR_INFO_PANEL)
        {
            if (panel_page == PANEL_PAGE::SOURCE_PANEL)
            {
                m_source_panel->CodeGenerationComplete();
                m_source_panel->OnNodeSelected(wxGetFrame().getSelectedNode());
            }
            else
            {
                m_hdr_info_panel->CodeGenerationComplete();
            }
        }
        return;
    }

    // C++ derived class panels only - generate derived code without regenerating base class
    ASSERT(m_panel_type == GEN_LANG_CPLUSPLUS);
    ASSERT(panel_page == PANEL_PAGE::DERIVED_SRC_PANEL ||
           panel_page == PANEL_PAGE::DERIVED_HDR_PANEL);

    m_derived_src_panel->Clear();
    m_derived_hdr_panel->Clear();

    CppCodeGenerator code_generator(m_cur_form);
    code_generator.SetSrcWriteCode(m_derived_src_panel);
    code_generator.SetHdrWriteCode(m_derived_hdr_panel);
    code_generator.GenerateDerivedClass(Project.get_ProjectNode(), m_cur_form, panel_page);

    if (panel_page == PANEL_PAGE::DERIVED_SRC_PANEL)
    {
        m_derived_src_panel->CodeGenerationComplete();
        m_derived_src_panel->OnNodeSelected(wxGetFrame().getSelectedNode());
    }
    else
    {
        m_derived_hdr_panel->CodeGenerationComplete();
    }
}

void BasePanel::OnNodeSelected(CustomEvent& event)
{
    if (!IsShown())
        return;

    auto form = event.getNode()->get_Form();

    if (form != m_cur_form)
    {
        m_cur_form = form;
        GenerateBaseClass();
    }

    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_hdr_info_panel)
        {
            m_hdr_info_panel->OnNodeSelected(event.getNode());
        }
        else
        {
            m_source_panel->OnNodeSelected(event.getNode());
        }
    }
}

void BasePanel::SetColor(int style, const wxColour& color)
{
    m_source_panel->SetColor(style, color);
    m_hdr_info_panel->SetColor(style, color);
}

void BasePanel::SetCodeFont(const wxFont& font)
{
    m_source_panel->SetCodeFont(font);
    m_hdr_info_panel->SetCodeFont(font);
    if (m_panel_type == GEN_LANG_CPLUSPLUS)
    {
        m_derived_src_panel->SetCodeFont(font);
        m_derived_hdr_panel->SetCodeFont(font);
    }
}
