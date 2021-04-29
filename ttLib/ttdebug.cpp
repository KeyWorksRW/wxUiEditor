/////////////////////////////////////////////////////////////////////////////
// Purpose:   Various debugging functionality
// Author:    Ralph Walden
// Copyright: Copyright (c) 1998-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"  // precompiled header

#if !defined(_WIN32)
    #error "This module can only be compiled for Windows"
#endif

#include <mutex>

namespace ttdbg
{
    std::mutex mutexAssert;
}  // namespace ttdbg

bool ttAssertionMsg(const char* filename, const char* function, int line, const char* cond, const std::string& msg)
{
    std::unique_lock<std::mutex> classLock(ttdbg::mutexAssert);

    ttlib::cstr str;
    if (cond)
        str << "Expression: " << cond << "\n\n";
    if (!msg.empty())
        str << "Comment: " << msg << "\n\n";

    str << "File: " << filename << "\n";
    str << "Function: " << function << "\n";
    str << "Line: " << line << "\n\n";
    str << "Press Retry to break into a debugger.";

    auto answer = MessageBoxW(GetActiveWindow(), str.to_utf16().c_str(), L"Assertion failed!", MB_ABORTRETRYIGNORE | MB_ICONSTOP);

    if (answer == IDRETRY)
    {
        return true;
    }
    else if (answer == IDABORT)
    {
        ExitProcess(static_cast<UINT>(-1));
    }

    return false;
}
