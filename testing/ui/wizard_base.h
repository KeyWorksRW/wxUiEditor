////////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wizard.h>

class WizardBase : public wxWizard
{
public:
    WizardBase(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& title = wxString::FromUTF8("Wizard Tests"),
        const wxBitmap& bitmap = wxNullBitmap, const wxPoint& pos = wxDefaultPosition,
        long style = wxDEFAULT_DIALOG_STYLE);

    bool Run() { return RunWizard((wxWizardPage*) GetPageAreaSizer()->GetItem((size_t) 0)->GetWindow()); }

protected:

    // Class member variables

    wxStaticText* m_staticText2;
    wxStaticText* m_staticText3;
    wxStaticText* m_staticText;

    // Virtual event handlers -- override them in your derived class

    virtual void OnBeforeChange(wxWizardEvent& event) { event.Skip(); }
};
