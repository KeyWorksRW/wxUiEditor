/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code code generation panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
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

// These are used everywhere we use scintilla to edit C++ code. It is also used to verify valid
// var_name values.
const char* g_u8_cpp_keywords = "alignas alignof and and_eq atomic_cancel atomic_commit atomic_noexcept auto"
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

const char* g_python_keywords = "False None True and as assert async break class continue def del elif else except finally "
                                "for from global if import in is lambda "
                                "nonlocal not or pass raise return self try while with yield";

const char* g_ruby_keywords = "ENCODING LINE FILE BEGIN END alias and begin break case class def defined do else"
                              " elsif end ensure false for if in module next nil not or redo require rescue retry"
                              " return self super then true undef unless until when while yield";

const char* g_perl_keywords = "do if else elsif unless while until for foreach last next pod cut redo continue "
                              "qw sub return goto and or not xor "
                              "unless use no package require my our local state ";

const char* g_lua_keywords =
    "and break do else elseif end false for function goto if in local nil not or repeat return then true until while";

const char* g_rust_keywords =
    "as break const continue crate do else enum extern false fn for if impl in let loop match mod move mut pub "
    "ref return self self mut static struct trait true type unsafe use where while";

const char* g_haskell_keywords =
    "case class data deriving do else if import in infix infixl infixr instance let module newtype of then type where";

const char* g_fortran_keywords = "program module subroutine function if else end do select case";

BasePanel::BasePanel(wxWindow* parent, MainFrame* frame, GenLang panel_type) : wxPanel(parent)
{
    m_panel_type = panel_type;
    auto top_sizer = new wxBoxSizer(wxVERTICAL);

    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiGenericTabArt());

    // Note that a lot of code assumes m_hPanel is valid. It is up to the language generator to generate inherited
    // classes, or just generate generation information about the class.

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
    else if (m_panel_type == GEN_LANG_PERL)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_PYTHON)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_RUBY)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_XRC)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_RUST)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }

#if GENERATE_NEW_LANG_CODE
    else if (m_panel_type == GEN_LANG_FORTRAN)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_HASKELL)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
    else if (m_panel_type == GEN_LANG_LUA)
    {
        m_cppPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_cppPanel, "source", false, wxWithImages::NO_IMAGE);
        m_hPanel = new CodeDisplay(m_notebook, panel_type);
        m_notebook->AddPage(m_hPanel, "info", false, wxWithImages::NO_IMAGE);
    }
#endif  // GENERATE_NEW_LANG_CODE

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

BasePanel::~BasePanel()
{
    wxGetMainFrame()->RemoveCustomEventHandler(GetEventHandler());
}

wxString BasePanel::GetSelectedText()
{
    auto notebook = wxStaticCast(m_cppPanel->GetParent(), wxAuiNotebook);
    auto text = notebook->GetPageText(notebook->GetSelection());
    if (text == "source")
    {
        return m_cppPanel->GetTextCtrl()->GetSelectedText();
    }
    else if (text == "derived_src")
    {
        return m_derived_src_panel->GetTextCtrl()->GetSelectedText();
    }
    else if (text == "header" || text == "inherit" || text == "info")
    {
        return m_hPanel->GetTextCtrl()->GetSelectedText();
    }
    else if (text == "derived_hdr")
    {
        return m_derived_hdr_panel->GetTextCtrl()->GetSelectedText();
    }

    return {};
}

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
    auto* top_notebook = wxGetFrame().getTopNotebook();
    auto top_page_name = top_notebook->GetPageText(top_notebook->GetSelection());
    auto* child_panel = static_cast<BasePanel*>(top_notebook->GetCurrentPage());
    if (auto* page = child_panel->m_notebook->GetCurrentPage(); page)
    {
        if (page == child_panel->m_cppPanel)
            return CPP_PANEL;
        else if (page == child_panel->m_hPanel)
            return HDR_PANEL;
        else if (page == child_panel->m_derived_src_panel)
            return DERIVED_SRC_PANEL;
        else if (page == child_panel->m_derived_hdr_panel)
            return DERIVED_HDR_PANEL;
    }
    return CPP_PANEL;
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
        if ((cur_selection->isGen(gen_folder) || cur_selection->isGen(gen_sub_folder)) && cur_selection->getChildCount() > 0)
        {
            m_cur_form = cur_selection->getChild(0);
        }
        else if (Project.getChildCount() > 0)
        {
            m_cur_form = Project.getFirstFormChild();
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

    BaseCodeGenerator codegen(m_panel_type, m_cur_form);

    m_cppPanel->Clear();
    codegen.SetSrcWriteCode(m_cppPanel);

    m_hPanel->Clear();
    codegen.SetHdrWriteCode(m_hPanel);

    switch (m_panel_type)
    {
        case GEN_LANG_CPLUSPLUS:
            codegen.GenerateCppClass(panel_page);

            m_derived_src_panel->Clear();
            codegen.SetSrcWriteCode(m_derived_src_panel);
            m_derived_hdr_panel->Clear();
            codegen.SetHdrWriteCode(m_derived_hdr_panel);

            codegen.GenerateDerivedClass(Project.getProjectNode(), m_cur_form, panel_page);
            break;

        case GEN_LANG_PERL:
            codegen.GeneratePerlClass(panel_page);
            break;

        case GEN_LANG_PYTHON:
            codegen.GeneratePythonClass(panel_page);
            break;

        case GEN_LANG_RUBY:
            codegen.GenerateRubyClass(panel_page);
            break;

        case GEN_LANG_RUST:
            codegen.GenerateRustClass(panel_page);
            break;

        case GEN_LANG_XRC:
            codegen.GenerateXrcClass(panel_page);
            break;

#if GENERATE_NEW_LANG_CODE
        case GEN_LANG_FORTRAN:
            codegen.GenerateFortranClass(panel_page);
            break;

        case GEN_LANG_HASKELL:
            codegen.GenerateHaskellClass(panel_page);
            break;

        case GEN_LANG_LUA:
            codegen.GenerateLuaClass(panel_page);
            break;
#endif  // GENERATE_NEW_LANG_CODE

        default:
            FAIL_MSG("Unknown panel type!")
            break;
    }

    if (panel_page == CPP_PANEL)
    {
        m_cppPanel->CodeGenerationComplete();
        m_cppPanel->OnNodeSelected(wxGetFrame().getSelectedNode());
        if (m_panel_type == GEN_LANG_CPLUSPLUS)
        {
            m_derived_src_panel->CodeGenerationComplete();
            m_derived_src_panel->OnNodeSelected(wxGetFrame().getSelectedNode());
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

    auto form = event.getNode()->getForm();

    if (form != m_cur_form)
    {
        m_cur_form = form;
        GenerateBaseClass();
    }

    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_hPanel)
        {
            m_hPanel->OnNodeSelected(event.getNode());
        }
        else
        {
            m_cppPanel->OnNodeSelected(event.getNode());
        }
    }
}

void BasePanel::SetColor(int style, const wxColour& color)
{
    m_cppPanel->SetColor(style, color);
    m_hPanel->SetColor(style, color);
}

void BasePanel::SetCodeFont(const wxFont& font)
{
    m_cppPanel->SetCodeFont(font);
    m_hPanel->SetCodeFont(font);
    if (m_panel_type == GEN_LANG_CPLUSPLUS)
    {
        m_derived_src_panel->SetCodeFont(font);
        m_derived_hdr_panel->SetCodeFont(font);
    }
}
