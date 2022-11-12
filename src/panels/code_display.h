/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/panel.h>

#include <tttextfile_wx.h>  // ttlib::viewfile

#include "codedisplay_base.h"

#include "write_code.h"  // WriteCode -- Write code to Scintilla or file

class wxFindDialogEvent;
class Node;

// CodeDisplayBase creates and initializes a wxStyledTextCtrl (scintilla) control, and places it in a sizer.
//
// WriteCode expects a class to override the doWrite() method, which in this case sends the text to the scinitilla control
// created by CodeDisplayBase.

class CodeDisplay : public CodeDisplayBase, public WriteCode
{
public:
    CodeDisplay(wxWindow* parent, int panel_type);

    // Clears scintilla and internal buffer, removes read-only flag in scintilla
    void Clear() override;

    // Transfers code from buffer to scintilla, sets scintilla markers to current node, marks
    // scintilla as read-only
    void CodeGenerationComplete();

    void OnNodeSelected(Node* node);

    wxStyledTextCtrl* GetTextCtrl() { return m_scintilla; };

protected:
    void OnFind(wxFindDialogEvent& event);

    // The following two functions are required to inherit from WriteCode

    void doWrite(ttlib::sview code) override;

private:
    ttlib::viewfile m_view;
    int m_panel_type;
};
