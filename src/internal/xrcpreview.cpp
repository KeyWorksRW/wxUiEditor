///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#if defined(INTERNAL_TESTING)

#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/sizer.h>

#include "xrcpreview.h"

bool XrcPreview::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_4 = new wxBoxSizer(wxHORIZONTAL);

    m_staticText = new wxStaticText(this, wxID_ANY,
        "Preview only works with Dialogs. Be certain the dialog will be visible and has a close box in the title bar!");
    box_sizer_4->Add(m_staticText, wxSizerFlags().Border(wxALL));

    box_sizer->Add(box_sizer_4, wxSizerFlags().Border(wxALL));

    auto* box_sizer_2 = new wxBoxSizer(wxHORIZONTAL);

    m_btn_2 = new wxButton(this, wxID_ANY, "&Blank");
    m_btn_2->SetToolTip("Create XRC with a single empty object");
    box_sizer_2->Add(m_btn_2, wxSizerFlags().Border(wxALL));

    m_btn_3 = new wxButton(this, wxID_ANY, "&Generate");
    m_btn_3->SetToolTip("Generate XRC from current selected form.");
    box_sizer_2->Add(m_btn_3, wxSizerFlags().Border(wxALL));

    m_btn_4 = new wxButton(this, wxID_ANY, "&Export...");
    m_btn_4->SetToolTip("Generate XRC from current selected form.");
    box_sizer_2->Add(m_btn_4, wxSizerFlags().Border(wxALL));

    m_btn = new wxButton(this, wxID_ANY, "&Preview...");
    m_btn->SetToolTip("Load the XRC into a dialog and display it.");
    box_sizer_2->Add(m_btn, wxSizerFlags().Border(wxALL));

    m_searchCtrl = new wxSearchCtrl(this, wxID_ANY, wxEmptyString);
    m_searchCtrl->ShowSearchButton(true);
    box_sizer_2->Add(m_searchCtrl, wxSizerFlags().Border(wxALL));

    box_sizer->Add(box_sizer_2, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer_3 = new wxBoxSizer(wxHORIZONTAL);

    m_scintilla = new wxStyledTextCtrl(this);
    {
        m_scintilla->SetLexer(wxSTC_LEX_XML);
        m_scintilla->SetEOLMode(wxSTC_EOL_LF);
        // Sets text margin scaled appropriately for the current DPI on Windows,
        // 5 on wxGTK or wxOSX
        m_scintilla->SetMarginLeft(wxSizerFlags::GetDefaultBorder());
        m_scintilla->SetMarginRight(wxSizerFlags::GetDefaultBorder());
        m_scintilla->SetProperty("fold", "1");
        m_scintilla->SetMarginWidth(1, 16);
        m_scintilla->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
        m_scintilla->SetMarginMask(1, wxSTC_MASK_FOLDERS);
        m_scintilla->SetMarginSensitive(1, true);
        m_scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_ARROW);
        m_scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN);
        m_scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN);
        m_scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_ARROW);
        m_scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_BACKGROUND);
        m_scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_BACKGROUND);
        m_scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_BACKGROUND);
        m_scintilla->SetBackSpaceUnIndents(true);
    }
    box_sizer_3->Add(m_scintilla, wxSizerFlags(1).Expand().Border(wxALL));

    box_sizer->Add(box_sizer_3, wxSizerFlags(1).Expand().Border(wxALL));

    dlg_sizer->Add(box_sizer, wxSizerFlags(1).Expand().Border(wxALL));

    auto* stdBtn = CreateStdDialogButtonSizer(wxCLOSE|wxNO_DEFAULT);
    dlg_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    SetSizer(dlg_sizer);
    SetMinSize(ConvertDialogToPixels(wxSize(800, 600)));
    Fit();
    Centre(wxBOTH);

    wxPersistentRegisterAndRestore(this, "XrcPreview");

    // Event handlers
    m_btn_2->Bind(wxEVT_BUTTON, &XrcPreview::OnCreate, this);
    m_btn_3->Bind(wxEVT_BUTTON, &XrcPreview::OnXrcCopy, this);
    m_btn_4->Bind(wxEVT_BUTTON, &XrcPreview::OnExport, this);
    m_btn->Bind(wxEVT_BUTTON, &XrcPreview::OnPreview, this);
    Bind(wxEVT_INIT_DIALOG, &XrcPreview::OnInit, this);
    m_searchCtrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &XrcPreview::OnSearch, this);

    return true;
}

#endif  // defined(INTERNAL_TESTING)

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************

/////////////////// Non-generated Copyright/License Info ////////////////////
// Purpose:   Test XRC
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if defined(INTERNAL_TESTING)

    #include <wx/filedlg.h>     // wxFileDialog base header
    #include <wx/mstream.h>     // Memory stream classes
    #include <wx/xml/xml.h>     // wxXmlDocument - XML parser & data holder class
    #include <wx/xrc/xmlres.h>  // XML resources

    #include "gen_xrc.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
    #include "mainframe.h"        // MainFrame -- Main window frame
    #include "node.h"             // Node class
    #include "project_handler.h"  // ProjectHandler class

    #include "pugixml.hpp"

const int node_marker = 1;

void XrcPreview::OnCreate(wxCommandEvent& WXUNUSED(event))
{
    auto doc_str = GenerateXrcStr(nullptr, xrc::no_flags);

    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
}

void XrcPreview::OnXrcCopy(wxCommandEvent& WXUNUSED(event))
{
    auto evt_flags = wxGetFrame().GetSelectedNode();

    if (!evt_flags)
    {
        wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
        return;
    }

    if (!evt_flags->isForm())
    {
        evt_flags = evt_flags->getForm();
    }

    auto doc_str = GenerateXrcStr(evt_flags, evt_flags->isGen(gen_PanelForm) ? xrc::previewing : 0);

    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
    m_scintilla->SetEmptySelection(0);

    tt_view_vector m_view;
    m_view.ReadString(doc_str);

    tt_string search("name=\"");
    evt_flags = wxGetFrame().GetSelectedNode();

    if (evt_flags->hasProp(prop_id) && evt_flags->as_string(prop_id) != "wxID_ANY")
    {
        search << evt_flags->as_string(prop_id);
    }
    else if (evt_flags->hasValue(prop_var_name))
    {
        search << evt_flags->as_string(prop_var_name);
    }
    else
    {
        search << evt_flags->as_string(prop_class_name);
    }

    int line = (to_int) m_view.FindLineContaining(search);

    if (!tt::is_found(line))
        return;

    m_scintilla->MarkerDeleteAll(node_marker);
    m_scintilla->MarkerAdd(line, node_marker);

    // Unlike GetLineVisible(), this function does ensure that the line is visible.
    m_scintilla->ScrollToLine(line);
}

void XrcPreview::OnPreview(wxCommandEvent& WXUNUSED(event))
{
    auto xrc_text = m_scintilla->GetText();
    wxString dlg_name;
    auto pos = xrc_text.Find("name=\"");
    if (!tt::is_found(pos))
    {
        wxMessageBox("Could not locate the dialog's name.", "XRC Dialog Preview");
        return;
    }
    pos += (sizeof("name=\"") - 1);
    while (pos < (to_int) xrc_text.size() && xrc_text[pos] != '"')
    {
        dlg_name << xrc_text[pos++];
    }

    wxMemoryInputStream stream(xrc_text.data(), xrc_text.size());
    auto xmlDoc = std::make_unique<wxXmlDocument>(wxXmlDocument(stream));
    if (!xmlDoc->IsOk())
    {
        wxMessageBox("Invalid XRC -- wxXmlDocument can't parse it.", "XRC Dialog Preview");
        return;
    }

    auto xrc_resource = wxXmlResource::Get();
    xrc_resource->InitAllHandlers();

    wxString res_name("wxuiDlgPreview");

    if (!xrc_resource->LoadDocument(xmlDoc.release(), res_name))
    {
        wxMessageBox("wxWidgets could not parse the XRC data.", "XRC Dialog Preview");
        return;
    }

    wxDialog dlg;
    if (xrc_resource->LoadDialog(&dlg, this, dlg_name))
    {
        dlg.ShowModal();
    }
    else
    {
        wxMessageBox(wxString("Could not load ") << dlg_name << " resource.", "XRC Dialog Preview");
    }
    xrc_resource->Unload(res_name);
}

    #ifndef SCI_SETKEYWORDS
        #define SCI_SETKEYWORDS 4005
        #define SCI_GETTEXT_MSG 2182
    #endif

extern const char* g_xrc_keywords;

void XrcPreview::OnInit(wxInitDialogEvent& event)
{
    // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
    m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

    m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
    m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#E91AFF"));
    m_scintilla->StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
    m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, *wxRED);
    m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
    m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));

    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

    event.Skip();
}

void XrcPreview::OnExport(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(this, "Export Project As XRC", Project.getProjectPath(), "preview_test.xrc",
                        "XRC File (*.xrc)|*.xrc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        tt_string filename = dialog.GetPath().utf8_string();

        std::string buf;
        buf.reserve(m_scintilla->GetTextLength() + 1);
        auto len = m_scintilla->GetTextLength() + 1;
        m_scintilla->SendMsg(SCI_GETTEXT_MSG, len, (wxIntPtr) buf.data());

        pugi::xml_document doc;
        doc.load_string(buf.c_str());

        if (!doc.save_file(filename.c_str(), "\t"))
        {
            wxMessageBox(wxString("An unexpected error occurred exportin ") << filename.make_wxString(), "Export XRC");
        }
    }
}

void XrcPreview::OnSearch(wxCommandEvent& event)
{
    m_scintilla->SetSelectionStart(m_scintilla->GetSelectionEnd());
    m_scintilla->SearchAnchor();
    auto srch_string = event.GetString();
    auto result = m_scintilla->SearchNext(0, event.GetString());

    if (result == wxSTC_INVALID_POSITION)
    {
        wxMessageBox(wxString() << event.GetString() << " not found.", "Not Found", wxICON_ERROR);
    }
    else
    {
        m_scintilla->EnsureCaretVisible();
    }
}

#endif  // defined(INTERNAL_TESTING)
