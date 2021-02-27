/////////////////////////////////////////////////////////////////////////////
// Name:      CStartup
// Purpose:   Dialog to display is wxUE is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dialog.h>

class CStartup : public wxDialog
{
public:
    CStartup();

    void LinkCommand(const wxString& cmd);
    void OnButtonEvent(wxCommandEvent&) { Close(); };

    enum : size_t
    {
        START_MRU,
        START_CONVERT,
        START_OPEN,
        START_EMPTY,
    };

    size_t GetCommandType() { return m_cmdType; }
    ttlib::cview GetMruFilename() { return m_mruFile.subview(); }

private:
    size_t m_cmdType { START_EMPTY };
    ttlib::cstr m_mruFile;
};
