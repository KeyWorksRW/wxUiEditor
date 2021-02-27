/////////////////////////////////////////////////////////////////////////////
// Purpose:   Automatically Freeze/Thaw a window
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/window.h>  // wxWindowBase class - the interface of wxWindow

class AutoFreeze
{
public:
    AutoFreeze(wxWindow* win) : m_win(win) { m_win->Freeze(); }
    ~AutoFreeze() { m_win->Thaw(); }

private:
    wxWindow* m_win;
};
