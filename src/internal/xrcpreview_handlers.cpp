/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handlers for XrcPreview Dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include <algorithm>
#include <format>
#include <iterator>
#include <locale>

#include <wx/filedlg.h>     // wxFileDialog base header
#include <wx/mstream.h>     // Memory stream classes
#include <wx/xml/xml.h>     // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>  // XML resources

#include "generated/xrcpreview.h"

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
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties
#include "wxue_namespace/wxue_string.h"       // wxue::string
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector
#include "xrccompare.h"                       // C++/XRC UI Comparison dialog

#include "pugixml.hpp"

#include "xrc_list_dlg.h"

const int node_marker = 1;

void MainFrame::OnXrcPreview(wxCommandEvent& /* event */)
{
    XrcPreview preview_dlg(this);
    preview_dlg.ShowModal();
}

#if !defined(SCI_SETKEYWORDS)
    #define SCI_SETKEYWORDS 4005
    #define SCI_GETTEXT_MSG 2182
#endif

extern const char* g_xrc_keywords;

void XrcPreview::OnInit(wxInitDialogEvent& event)
{
    SetStcColors(m_scintilla, GenLang::xrc, false, true);

    m_scintilla->StyleSetBold(wxSTC_H_TAG, true);

    const FontProperty font_prop(UserPrefs.get_CodeDisplayFont().ToStdView());
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font_prop.GetFont());

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

    event.Skip();

    wxCommandEvent dummy;
    OnGenerate(dummy);

#if defined(INTERNAL_TESTING)
    const wxue::string& import_file = wxGetFrame().getImportPanel()->GetImportFile();
    if (wxue::string(import_file.extension()).MakeLower() != ".xrc")
    {
        m_btnCompare->Disable();
    }
#endif
}

void XrcPreview::OnClear(wxCommandEvent& /* event unused */)
{
    m_scintilla->ClearAll();
}

void XrcPreview::OnGenerate(wxCommandEvent& /* event unused */)
{
    m_form_node = wxGetMainFrame()->getSelectedNode();
    if (!m_form_node)
    {
        wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
        return;
    }

    if (!m_form_node->is_Form())
    {
        XrcListDlg list_dlg(this);
        if (list_dlg.ShowModal() != wxID_OK)
        {
            return;
        }

        m_form_node = list_dlg.get_form();
        if (!m_form_node)
        {
            wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
            return;
        }
    }

    if (!m_form_node->is_Form())
    {
        m_form_node = m_form_node->get_Form();
    }

    Generate();
}

void XrcPreview::Generate(Node* form_node)
{
    if (!form_node)
    {
        form_node = m_form_node;
    }

    if (!form_node)
    {
        MSG_ERROR("Generate() called without a form node");
        return;
    }

    const std::string doc_str =
        GenerateXrcStr(form_node, form_node->is_Gen(gen_PanelForm) ? xrc::previewing : 0);

    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
    m_scintilla->SetEmptySelection(0);

    wxue::ViewVector m_view;
    m_view.ReadString(std::string_view(doc_str));

    std::string search;

    if (form_node->HasProp(prop_id) && form_node->as_string(prop_id) != "wxID_ANY")
    {
        search = form_node->as_string(prop_id);
    }
    else if (form_node->HasValue(prop_var_name))
    {
        search = form_node->as_string(prop_var_name);
    }
    else
    {
        search = form_node->as_string(prop_class_name);
    }

    if (search.empty())
    {
        return;
    }

    m_contents->SetLabelText("Contents: " + search);

    const wxue::ViewVector::iterator iter =
        std::ranges::find_if(m_view,
                             [&search](const wxue::string_view& line)
                             {
                                 return line.contains(search);
                             });
    const int line =
        (iter != m_view.end()) ? static_cast<int>(std::distance(m_view.begin(), iter)) : -1;

    if (!wxue::is_found(line))
    {
        return;
    }

    m_scintilla->MarkerDeleteAll(node_marker);
    m_scintilla->MarkerAdd(line, node_marker);

    // Unlike GetLineVisible(), this function does ensure that the line is visible.
    m_scintilla->ScrollToLine(line);
}

void XrcPreview::OnPreview(wxCommandEvent& /* event unused */)
{
    if (!m_form_node)
    {
        wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
        return;
    }

    std::string xrc_text = m_scintilla->GetText().utf8_string();
    PreviewXrc(xrc_text, m_form_node->get_GenName(), nullptr);
}

void XrcPreview::OnVerify(wxCommandEvent& /* event unused */)
{
    pugi::xml_document xml_doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).

        // Verify that the XML in the Scintilla control ia valid by parsing it with PugiXML.
        const std::string xrc_text = m_scintilla->GetText().utf8_string();
        if (auto result = xml_doc.load_string(xrc_text); !result)
        {
            const std::string msg =
                std::format("Parsing error: {}\n Line: {}, Column: {}, Offset: {}\n",
                            result.description(), result.line, result.column, result.offset);
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();

            return;
        }
    }

    const pugi::xml_node root = xml_doc.first_child();
    if (!wxue::string_view(root.name()).is_sameas("resource", wxue::CASE::either))
    {
        wxMessageBox("Invalid XML -- no resource object", "XML Verification Test",
                     wxOK | wxICON_ERROR);
        return;
    }

    wxMessageBox("XML in Contents can be parsed.", "XRC Verification Test", wxOK | wxICON_NONE);
}

void XrcPreview::OnExport(wxCommandEvent& /* event unused */)
{
    const wxue::string path = Project.get_ProjectPath();
    wxFileDialog dialog(this, "Export Project As XRC", path.wx(), "preview_test.xrc",
                        "XRC File (*.xrc)|*.xrc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        const wxue::string filename = dialog.GetPath().utf8_string();

        const std::string xrc_text = m_scintilla->GetText().utf8_string();

        pugi::xml_document xml_doc;
        if (auto result = xml_doc.load_string(xrc_text); !result)
        {
            const std::string msg =
                std::format("Parsing error: {}\n Line: {}, Column: {}, Offset: {}\n",
                            result.description(), result.line, result.column, result.offset);
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();

            return;
        }

        if (!xml_doc.save_file(filename))
        {
            wxMessageBox(wxString("An unexpected error occurred exporting ") << filename.wx(),
                         "Export XRC");
        }
    }
}

void XrcPreview::OnDuplicate(wxCommandEvent& /* event unused */)
{
    pugi::xml_document xml_doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).
        const std::string xrc_text = m_scintilla->GetText().utf8_string();
        if (auto result = xml_doc.load_string(xrc_text); !result)
        {
            const std::string msg =
                std::format("Parsing error: {}\n Line: {}, Column: {}, Offset: {}\n",
                            result.description(), result.line, result.column, result.offset);
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();

            return;
        }
    }

    const pugi::xml_node root = xml_doc.first_child();
    if (!wxue::string_view(root.name()).is_sameas("resource", wxue::CASE::either))
    {
        wxMessageBox("Invalid XRC -- no resource object", "Import XRC Test");
        return;
    }

    WxSmith doc_import;

    pugi::xml_node first_child = root.first_child();
    if (!first_child ||
        !wxue::string_view(first_child.name()).is_sameas("object", wxue::CASE::either))
    {
        MSG_ERROR("Invalid XRC -- no exported object in the resource");
        return;
    }

    const NodeSharedPtr new_node = doc_import.CreateXrcNode(first_child, nullptr);
    if (new_node)
    {
        Project.FixupDuplicatedNode(new_node.get());
        wxue::string undo_str("duplicate ");
        undo_str << new_node->get_DeclName();
        wxGetMainFrame()->PushUndoAction(std::make_shared<InsertNodeAction>(
            new_node.get(), Project.get_ProjectNode(), undo_str));
        wxGetMainFrame()->FireCreatedEvent(new_node);
        wxGetMainFrame()->SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
    }
    else
    {
        MSG_ERROR("Failed to create node");
    }
}

void XrcPreview::OnCompare(wxCommandEvent& /* event unused */)
{
    if (!m_form_node)
    {
        wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
        return;
    }

    if (!m_form_node->is_Gen(gen_wxDialog) && !m_form_node->is_Gen(gen_PanelForm))
    {
        wxMessageBox("You can only compare dialogs and panels", "Compare");
        return;
    }

    const wxue::SaveCwd save_cwd(wxue::restore_cwd);
    wxSetWorkingDirectory(Project.ArtDirectory().wx());

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
    const int result = m_scintilla->SearchNext(0, event.GetString());

    if (result == wxSTC_INVALID_POSITION)
    {
        wxMessageBox(wxString() << event.GetString() << " not found.", "Not Found", wxICON_ERROR);
    }
    else
    {
        m_scintilla->EnsureCaretVisible();
    }
}
