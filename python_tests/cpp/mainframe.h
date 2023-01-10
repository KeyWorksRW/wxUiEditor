///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#pragma once

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/statusbr.h>
#include <wx/toolbar.h>

class MainFrame : public wxFrame
{
public:
    MainFrame(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "wxUiTesting", const wxPoint& pos =
        wxDefaultPosition, const wxSize& size = wxSize(500, 300), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL,
        const wxString &name = wxFrameNameStr);

protected:

    // Event handlers

    void OnChoicebook(wxCommandEvent& event);
    void OnCommonDialog(wxCommandEvent& event);
    void OnImportTest(wxCommandEvent& event);
    void OnListbook(wxCommandEvent& event);
    void OnMultiTestDialog(wxCommandEvent& event);
    void OnNotebook(wxCommandEvent& event);
    void OnOtherCtrls(wxCommandEvent& event);
    void OnPythonDlg(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnRibbonDialog(wxCommandEvent& event);
    void OnToolbook(wxCommandEvent& event);
    void OnTreebook(wxCommandEvent& event);
    void OnWizard(wxCommandEvent& event);

    // Class member variables

    wxStatusBar* m_statusBar;
    wxToolBar* m_toolBar;
};

namespace wxue_img
{
    // Images declared in this class module:

    extern const unsigned char wxPython_1_5x_png[765];
    extern const unsigned char wxPython_2x_png[251];
    extern const unsigned char wxPython_png[399];
}

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************