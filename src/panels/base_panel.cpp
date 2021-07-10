/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code code generation panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

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
#include "write_code.h"    // Write code to Scintilla or file

BasePanel::BasePanel(wxWindow* parent, MainFrame* frame, bool GenerateDerivedCode) : wxPanel(parent)
{
    m_GenerateDerivedCode = GenerateDerivedCode;
    auto top_sizer = new wxBoxSizer(wxVERTICAL);

    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiGenericTabArt());

    m_cppPanel = new CodeDisplay(m_notebook, wxID_ANY);
    InitStyledTextCtrl(m_cppPanel->GetTextCtrl());
    m_notebook->AddPage(m_cppPanel, "source", false, 0);

    m_hPanel = new CodeDisplay(m_notebook, wxID_ANY);
    InitStyledTextCtrl(m_hPanel->GetTextCtrl());
    m_notebook->AddPage(m_hPanel, "header", false, 1);

    top_sizer->Add(m_notebook, wxSizerFlags(1).Expand());

    SetSizerAndFit(top_sizer);

    m_hdr_display = std::make_unique<PanelCodeWriter>(m_hPanel->GetTextCtrl());
    m_src_display = std::make_unique<PanelCodeWriter>(m_cppPanel->GetTextCtrl());

    Bind(wxEVT_FIND, &BasePanel::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &BasePanel::OnFind, this);

    Bind(EVT_EventHandlerChanged, [this](wxEvent&) { GenerateBaseClass(); });
    Bind(EVT_NodeCreated, [this](wxEvent&) { GenerateBaseClass(); });
    Bind(EVT_NodeDeleted, [this](wxEvent&) { GenerateBaseClass(); });
    Bind(EVT_NodePropChange, [this](wxEvent&) { GenerateBaseClass(); });
    Bind(EVT_ParentChanged, [this](wxEvent&) { GenerateBaseClass(); });
    Bind(EVT_ProjectUpdated, [this](wxEvent&) { GenerateBaseClass(); });

    Bind(EVT_NodeSelected, &BasePanel::OnNodeSelected, this);

    frame->AddCustomEventHandler(GetEventHandler());
}

BasePanel::~BasePanel() {}

void BasePanel::InitStyledTextCtrl(wxStyledTextCtrl* stc)
{
    stc->SetLexer(wxSTC_LEX_CPP);

    // These are just the keywords that we might generate -- there is no need for a complete list of C++ keywords
    stc->SetKeyWords(0, "auto bool char char8_t class const constexpr \
	                          decltype default delete do double else enum explicit \
	                          extern false float for friend if inline int long \
	                          mutable namespace new noexcept nullptr private protected public \
	                          return short signed sizeof static static_cast \
	                          struct template this true typedef typeid \
	                          typename union unsigned using virtual void volatile wchar_t \
	                          while");

    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    stc->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    stc->StyleSetBold(wxSTC_C_WORD, true);
    stc->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
    stc->StyleSetForeground(wxSTC_C_STRING, *wxRED);
    stc->StyleSetForeground(wxSTC_C_STRINGEOL, *wxRED);
    stc->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
    stc->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 128, 0));
    stc->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
    stc->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 128, 0));
    stc->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour(0, 128, 0));
    stc->StyleSetForeground(wxSTC_C_NUMBER, *wxBLUE);

    stc->SetTabWidth(4);
    stc->SetTabIndents(true);
    stc->SetReadOnly(true);
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
    else if (text == "header")
    {
        m_hPanel->GetEventHandler()->ProcessEvent(event);
    }
}

void BasePanel::FindItemName(const wxString& name)
{
    if (!IsShown())
        return;

    auto notebook = wxStaticCast(m_cppPanel->GetParent(), wxAuiNotebook);
    ASSERT(notebook);

    auto text = notebook->GetPageText(notebook->GetSelection());
    if (text == "source")
    {
        m_cppPanel->FindItemName(name);
    }
    else if (text == "header")
    {
        m_hPanel->FindItemName(name);
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
            m_cur_form = project->GetChild(0);
        else
            return;
    }

    BaseCodeGenerator codegen;

    codegen.SetHdrWriteCode(m_hdr_display.get());
    codegen.SetSrcWriteCode(m_src_display.get());

    Freeze();

    try
    {
        wxStyledTextCtrl* cppEditor = m_cppPanel->GetTextCtrl();
        wxStyledTextCtrl* hEditor = m_hPanel->GetTextCtrl();
        cppEditor->SetReadOnly(false);
        auto cppLine = cppEditor->GetFirstVisibleLine() + cppEditor->LinesOnScreen() - 1;
        auto cppXOffset = cppEditor->GetXOffset();

        hEditor->SetReadOnly(false);
        auto hLine = hEditor->GetFirstVisibleLine() + hEditor->LinesOnScreen() - 1;
        auto hXOffset = hEditor->GetXOffset();

        PANEL_TYPE panel_type = CPP_PANEL;
        if (auto page = m_notebook->GetCurrentPage(); page)
        {
            if (page == m_hPanel)
                panel_type = HDR_PANEL;
        }

        if (m_GenerateDerivedCode)
            codegen.GenerateDerivedClass(project, m_cur_form, panel_type);
        else
            codegen.GenerateBaseClass(project, m_cur_form, panel_type);

        cppEditor->SetReadOnly(true);
        cppEditor->GotoLine(cppLine);
        cppEditor->SetXOffset(cppXOffset);
        cppEditor->SetAnchor(0);
        cppEditor->SetCurrentPos(0);

        hEditor->SetReadOnly(true);
        hEditor->GotoLine(hLine);
        hEditor->SetXOffset(hXOffset);
        hEditor->SetAnchor(0);
        hEditor->SetCurrentPos(0);
    }
    catch (const std::exception& DBG_PARAM(e))
    {
        MSG_ERROR(e.what());
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
}
