/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handlers for XrcPreview Dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "xrcpreview.h"

#if __has_include(<format>)
    #include <format>
#endif

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
#include "gen_xrc.h"                   // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "internal/msg_logging.h"      // MsgLogging -- Message logging class
#include "mainframe.h"                 // MainFrame -- Main window frame
#include "node.h"                      // Node class
#include "preferences.h"               // Prefs -- Set/Get wxUiEditor preferences
#include "project_handler.h"           // ProjectHandler class
#include "tt_view_vector.h"            // tt_string_vector -- Class for reading and writing line-oriented strings/files
#include "undo_cmds.h"                 // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"                     // Utility functions that work with properties

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
        if (tt_string(import_file.extension()).MakeLower() != "xrc")
            m_btnCompare->Disable();
    }
}

void XrcPreview::OnClear(wxCommandEvent& WXUNUSED(event))
{
    m_scintilla->ClearAll();
}

void XrcPreview::OnGenerate(wxCommandEvent& WXUNUSED(event))
{
    XrcListDlg dlg(this);
    if (dlg.ShowModal() != wxID_OK)
        return;

    auto form = dlg.get_form();

    if (!form)
    {
        wxMessageBox("You need to select a form first.", "XRC Dialog Preview");
        return;
    }

    if (!form->isForm())
    {
        form = form->getForm();
    }

    auto doc_str = GenerateXrcStr(form, form->isGen(gen_PanelForm) ? xrc::previewing : 0);

    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
    m_scintilla->SetEmptySelection(0);

    tt_view_vector m_view;
    m_view.ReadString(doc_str);

    std::string search("name=\"");

    if (form->hasProp(prop_id) && form->as_string(prop_id) != "wxID_ANY")
    {
        search = form->as_string(prop_id);
    }
    else if (form->hasValue(prop_var_name))
    {
        search = form->as_string(prop_var_name);
    }
    else
    {
        search = form->as_string(prop_class_name);
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

extern bool g_isXrcResourceInitalized;

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
    wxXmlParseError err_details;
    auto xmlDoc = std::make_unique<wxXmlDocument>(wxXmlDocument());
    if (auto result = xmlDoc->Load(stream, wxXMLDOC_NONE, &err_details); !result)
    {
#if __has_include(<format>)
        std::string msg =
            std::format(std::locale(""), "Parsing error: {} at line: {}, column: {}, offset: {:L}\n",
                        err_details.message.ToStdString(), err_details.line, err_details.column, err_details.offset);
#else
        wxString msg;
        msg.Format("Parsing error: %s at line: %d, column: %d, offset: %ld\n", err_details.message, err_details.line,
                   err_details.column, err_details.offset);
#endif
        wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error", wxOK | wxICON_ERROR).ShowModal();
        return;
    }
    if (!xmlDoc->IsOk())
    {
        wxMessageBox("Invalid XRC -- wxXmlDocument can't parse it.", "XRC Dialog Preview");
        return;
    }

    auto xrc_resource = wxXmlResource::Get();
    if (!g_isXrcResourceInitalized)
    {
        g_isXrcResourceInitalized = true;

        xrc_resource->InitAllHandlers();
        xrc_resource->AddHandler(new wxRichTextCtrlXmlHandler);
        xrc_resource->AddHandler(new wxAuiXmlHandler);
        xrc_resource->AddHandler(new wxAuiToolBarXmlHandler);
        xrc_resource->AddHandler(new wxRibbonXmlHandler);
        xrc_resource->AddHandler(new wxStyledTextCtrlXmlHandler);
    }

    wxString res_name("wxuiDlgPreview");

    if (!xrc_resource->LoadDocument(xmlDoc.release(), res_name))
    {
        wxMessageBox("wxWidgets could not parse the XRC data.", "XRC Dialog Preview");
        return;
    }

    tt_cwd cwd(true);
    wxSetWorkingDirectory(Project.ArtDirectory().make_wxString());

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

void XrcPreview::OnVerify(wxCommandEvent& WXUNUSED(event))
{
    // Verify that the XML in the Scintilla control ia valid by parsing it with PugiXML.
    auto xrc_text = m_scintilla->GetText();
    pugi::xml_document doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).
        auto result = doc.load_string(xrc_text.utf8_string());
        if (!result)
        {
            wxMessageBox("Error parsing XML document: " + tt_string(result.description()), "XML Verification Test",
                         wxOK | wxICON_ERROR);
            return;
        }
    }

    auto root = doc.first_child();
    if (!tt::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        wxMessageBox("Invalid XML -- no resource object", "XML Verification Test", wxOK | wxICON_ERROR);
        return;
    }

    wxMessageBox("XML in Contents can be parsed.", "XRC Verification Test", wxOK | wxICON_NONE);
}

void XrcPreview::OnExport(wxCommandEvent& WXUNUSED(event))
{
    tt_string path = Project.getProjectPath();
    wxFileDialog dialog(this, "Export Project As XRC", path.make_wxString(), "preview_test.xrc", "XRC File (*.xrc)|*.xrc",
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        tt_string filename = dialog.GetPath().utf8_string();

        std::string buf;
        buf.reserve(m_scintilla->GetTextLength() + 1);
        auto len = m_scintilla->GetTextLength() + 1;
        m_scintilla->SendMsg(SCI_GETTEXT_MSG, len, (wxIntPtr) buf.data());

        pugi::xml_document doc;
        doc.load_string(buf.c_str());

        if (!doc.save_file(filename))
        {
            wxMessageBox(wxString("An unexpected error occurred exporting ") << filename.make_wxString(), "Export XRC");
        }
    }
}

void XrcPreview::OnDuplicate(wxCommandEvent& WXUNUSED(event))
{
    auto xrc_text = m_scintilla->GetText();
    pugi::xml_document doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).
        auto result = doc.load_string(xrc_text.utf8_string());
        if (!result)
        {
            wxMessageBox("Error parsing XRC document: " + tt_string(result.description()), "XRC Import Test");
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
