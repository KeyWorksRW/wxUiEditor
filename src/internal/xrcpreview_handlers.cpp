/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handlers for XrcPreview Dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "xrcpreview.h"

#include <format>

#include <wx/filedlg.h>     // wxFileDialog base header
#include <wx/mstream.h>     // Memory stream classes
#include <wx/xml/xml.h>     // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>  // XML resources

// The following handlers must be explicitly added

#include <wx/xrc/xh_aui.h>             // XRC resource handler for wxAUI
#include <wx/xrc/xh_auitoolb.h>        // XML resource handler for wxAuiToolBar
#include <wx/xrc/xh_ribbon.h>          // XML resource handler for wxRibbon related classes
#include <wx/xrc/xh_richtext.h>        // XML resource handler for wxRichTextCtrl
#include <wx/xrc/xh_styledtextctrl.h>  // XML resource handler for wxStyledTextCtrl

#include "../import/import_wxsmith.h"  // Import a wxSmith file
#include "gen_xrc.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "import_panel.h"     // ImportPanel -- Panel to display original imported file
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "preferences.h"      // Prefs -- Set/Get wxUiEditor preferences
#include "previews.h"         // Top level Preview functions
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_string_vector -- read/write line-oriented strings/files
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties
#include "xrccompare.h"       // C++/XRC UI Comparison dialog

#include "pugixml.hpp"

#include "xrc_list_dlg.h"

#include "xrc_list_dlg.h"

const int node_marker = 1;

void MainFrame::OnXrcPreview(wxCommandEvent& /* event */)
{
    XrcPreview dlg(this);
    dlg.ShowModal();
}

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
    #define SCI_GETTEXT_MSG 2182
#endif

extern const char* g_xrc_keywords;

void XrcPreview::OnInit(wxInitDialogEvent& event)
{
    SetStcColors(m_scintilla, GEN_LANG_XRC, false, true);

    m_scintilla->StyleSetBold(wxSTC_H_TAG, true);

    FontProperty font_prop(UserPrefs.get_CodeDisplayFont().ToStdView());
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font_prop.GetFont());

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

    event.Skip();

    wxCommandEvent dummy;
    OnGenerate(dummy);

    if (wxGetApp().isTestingMenuEnabled())
    {
        const auto& import_file = wxGetFrame().getImportPanel()->GetImportFile();
        if (tt_string(import_file.extension()).MakeLower() != ".xrc")
            m_btnCompare->Disable();
    }
}

void XrcPreview::OnClear(wxCommandEvent& WXUNUSED(event))
{
    m_scintilla->ClearAll();
}

void XrcPreview::OnGenerate(wxCommandEvent& WXUNUSED(event))
{
    m_form_node = wxGetMainFrame()->getSelectedNode();
    if (!m_form_node->isForm())
    {
        XrcListDlg dlg(this);
        if (dlg.ShowModal() != wxID_OK)
            return;

        m_form_node = dlg.get_form();
    }

    if (!m_form_node)
    {
        wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
        return;
    }

    if (!m_form_node->isForm())
    {
        m_form_node = m_form_node->getForm();
    }

    Generate();
}

void XrcPreview::Generate(Node* form_node)
{
    if (!form_node)
    {
        form_node = m_form_node;
        ASSERT_MSG(form_node, "Generate() called without a form_node and m_form_node is nullptr");
    }

    auto doc_str = GenerateXrcStr(form_node, form_node->isGen(gen_PanelForm) ? xrc::previewing : 0);

    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
    m_scintilla->SetEmptySelection(0);

    tt_view_vector m_view;
    m_view.ReadString(doc_str);

    std::string search("name=\"");

    if (form_node->hasProp(prop_id) && form_node->as_string(prop_id) != "wxID_ANY")
    {
        search = form_node->as_string(prop_id);
    }
    else if (form_node->hasValue(prop_var_name))
    {
        search = form_node->as_string(prop_var_name);
    }
    else
    {
        search = form_node->as_string(prop_class_name);
    }

    m_contents->SetLabelText("Contents: " + search);

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
    auto xrc_text = m_scintilla->GetText().utf8_string();
    PreviewXrc(xrc_text, m_form_node->getGenName(), nullptr);
}

void XrcPreview::OnVerify(wxCommandEvent& WXUNUSED(event))
{
    pugi::xml_document doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).

        // Verify that the XML in the Scintilla control ia valid by parsing it with PugiXML.
        auto xrc_text = m_scintilla->GetText().utf8_string();
        if (auto result = doc.load_string(xrc_text); !result)
        {
            std::string msg = std::format(
                std::locale(""), "Parsing error: {}\n Line: {}, Column: {}, Offset: {:L}\n",
                result.description(), result.line, result.column, result.offset);
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();

            return;
        }
    }

    auto root = doc.first_child();
    if (!tt::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        wxMessageBox("Invalid XML -- no resource object", "XML Verification Test",
                     wxOK | wxICON_ERROR);
        return;
    }

    wxMessageBox("XML in Contents can be parsed.", "XRC Verification Test", wxOK | wxICON_NONE);
}

void XrcPreview::OnExport(wxCommandEvent& WXUNUSED(event))
{
    tt_string path = Project.getProjectPath();
    wxFileDialog dialog(this, "Export Project As XRC", path.make_wxString(), "preview_test.xrc",
                        "XRC File (*.xrc)|*.xrc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        tt_string filename = dialog.GetPath().utf8_string();

        auto xrc_text = m_scintilla->GetText().utf8_string();

        pugi::xml_document doc;
        if (auto result = doc.load_string(xrc_text); !result)
        {
            std::string msg = std::format(
                std::locale(""), "Parsing error: {}\n Line: {}, Column: {}, Offset: {:L}\n",
                result.description(), result.line, result.column, result.offset);
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();

            return;
        }

        if (!doc.save_file(filename))
        {
            wxMessageBox(wxString("An unexpected error occurred exporting ")
                             << filename.make_wxString(),
                         "Export XRC");
        }
    }
}

void XrcPreview::OnDuplicate(wxCommandEvent& WXUNUSED(event))
{
    pugi::xml_document doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).
        auto xrc_text = m_scintilla->GetText().utf8_string();
        if (auto result = doc.load_string(xrc_text); !result)
        {
            std::string msg = std::format(
                std::locale(""), "Parsing error: {}\n Line: {}, Column: {}, Offset: {:L}\n",
                result.description(), result.line, result.column, result.offset);
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();

            return;
        }
    }

    auto root = doc.first_child();
    if (!tt::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        wxMessageBox("Invalid XRC -- no resource object", "Import XRC Test");
        return;
    }

    WxSmith doc_import;

    auto first_child = root.first_child();
    auto new_node = doc_import.CreateXrcNode(first_child, nullptr);
    if (new_node)
    {
        Project.FixupDuplicatedNode(new_node.get());
        tt_string undo_str("duplicate ");
        undo_str << new_node->declName();
        wxGetMainFrame()->PushUndoAction(
            std::make_shared<InsertNodeAction>(new_node.get(), Project.getProjectNode(), undo_str));
        wxGetMainFrame()->FireCreatedEvent(new_node);
        wxGetMainFrame()->SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
    }
    else
    {
        MSG_ERROR("Failed to create node");
    }
}

void XrcPreview::OnCompare(wxCommandEvent& WXUNUSED(event))
{
    if (!m_form_node->isGen(gen_wxDialog) && !m_form_node->isGen(gen_PanelForm))
    {
        wxMessageBox("You can only compare dialogs and panels", "Compare");
        return;
    }

    tt_cwd cwd(true);
    wxSetWorkingDirectory(Project.ArtDirectory().make_wxString());

    XrcCompare dlg_compare;
    if (!dlg_compare.DoCreate(wxGetMainFrame(), m_form_node, true))
    {
        wxMessageBox("Unable to create the XrcCompare dialog box!", "Compare");
        return;
    }

    dlg_compare.ShowModal();
    return;
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
