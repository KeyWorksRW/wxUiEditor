///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#pragma once

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/grid.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>

class GlobalCustomIDSBase : public wxDialog
{
public:
    GlobalCustomIDSBase() {}
    GlobalCustomIDSBase(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& title =
        "Globally Add Prefix/Suffix to Custom IDs", const wxPoint& pos = wxDefaultPosition, const wxSize& size =
        wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, const wxString &name = wxDialogNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& title =
        "Globally Add Prefix/Suffix to Custom IDs", const wxPoint& pos = wxDefaultPosition, const wxSize& size =
        wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, const wxString &name = wxDialogNameStr);

protected:

    // Virtual event handlers -- override them in your derived class

    virtual void OnAddPrefix(wxCommandEvent& event) { event.Skip(); }
    virtual void OnAddSuffix(wxCommandEvent& event) { event.Skip(); }
    virtual void OnCommit(wxCommandEvent& event) { event.Skip(); }
    virtual void OnInit(wxInitDialogEvent& event) { event.Skip(); }
    virtual void OnRemovePrefix(wxCommandEvent& event) { event.Skip(); }
    virtual void OnRemoveSuffix(wxCommandEvent& event) { event.Skip(); }
    virtual void OnSelectAllFolders(wxCommandEvent& event) { event.Skip(); }
    virtual void OnSelectAllForms(wxCommandEvent& event) { event.Skip(); }
    virtual void OnSelectFolders(wxCommandEvent& event) { event.Skip(); }
    virtual void OnSelectForms(wxCommandEvent& event) { event.Skip(); }
    virtual void OnSelectNoFolders(wxCommandEvent& event) { event.Skip(); }
    virtual void OnSelectNoForms(wxCommandEvent& event) { event.Skip(); }

    // Class member variables

    wxButton* m_btn_Add_new_prefix;
    wxButton* m_btn_add_new_suffix_2;
    wxButton* m_btn_commit;
    wxButton* m_btn_remove_old_suffix;
    wxButton* m_btn_remove_prefix;
    wxComboBox* m_combo_prefixes;
    wxComboBox* m_combo_suffixes;
    wxGrid* m_grid;
    wxListBox* m_lb_folders;
    wxListBox* m_lb_forms;
    wxTextCtrl* m_text_old_prefix;
    wxTextCtrl* m_text_old_suffix;
};

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************