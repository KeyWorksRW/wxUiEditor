/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ttcvector.h>  // cstrVector -- Vector of ttlib::cstr strings

class MsgFrame;

class MsgLogging
{
public:
    void ShowLogger();
    void CloseLogger();

    void AddInfoMsg(ttlib::cview msg);
    void AddEventMsg(ttlib::cview msg);
    void AddWarningMsg(ttlib::cview msg);
    void AddErrorMsg(ttlib::cview msg);

private:
    MsgFrame* m_msgFrame { nullptr };
    ttlib::cstrVector m_Msgs;

    bool m_bDestroyed { true };
    bool m_isFirstShown { false };  // If false, and PREFS_MSG_WINDOW is set, then show the window
};

extern MsgLogging* g_pMsgLogging;
