/////////////////////////////////////////////////////////////////////////////
// Purpose:   Various debugging functionality
// Author:    Ralph Walden
// Copyright: Copyright (c) 1998-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"  // precompiled header

#if !defined(_WIN32)
    #error "This module can only be compiled for Windows"
#endif

#include <mutex>

#include <stdexcept>
#include <stdio.h>

#include "ttcstr.h"   // Classes for handling zero-terminated char strings.
#include "ttdebug.h"  // ttASSERT macros

// DO NOT CHANGE THESE TWO NAMES! Multiple applications expect these names and will no longer display trace messages
// if you change them.

const char* ttlib::txtTraceClass = "KeyViewMsgs";
const char* ttlib::txtTraceShareName = "hhw_share";
HWND ttlib::hwndTrace { reinterpret_cast<HWND>(-1) };  // -1 means search for the ttTrace.exe main window

namespace ttdbg
{
    std::mutex mutexAssert;
    std::mutex mutexTrace;

    bool allowAsserts { true };     // Setting this to true will cause ttAssertionMsg to return without doing anything
    bool allowDuplicates { true };  // Setting this to false will prevent any assert msg from being displayed more than once

    HANDLE hTraceMapping { NULL };
    char* g_pszTraceMap { nullptr };
    std::vector<ttlib::cstr> g_priorAsserts;
}  // namespace ttdbg

bool ttAssertionMsg(const char* filename, const char* function, int line, const char* cond, const std::string& msg)
{
    if (!ttdbg::allowAsserts)
        return false;

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

    if (!ttdbg::allowDuplicates)
    {
        for (auto& iter: ttdbg::g_priorAsserts)
        {
            if (iter.is_sameas(msg))
                return false;
        }
        ttdbg::g_priorAsserts.emplace_back(msg);
    }

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

void ttlib::allow_asserts(bool allowAsserts)
{
    ttdbg::allowAsserts = allowAsserts;
}

void ttlib::duplicate_asserts(bool allowDuplicates)
{
    if (allowDuplicates == ttdbg::allowDuplicates)
        return;

    if (!ttdbg::allowDuplicates)
        ttdbg::g_priorAsserts.clear();

    ttdbg::allowDuplicates = allowDuplicates;
}

bool ttdoReportLastError(const char* filename, const char* function, int line)
{
    char* pszMsg;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &pszMsg, 0, NULL);

    auto result = ttAssertionMsg(filename, function, line, nullptr, pszMsg);

    LocalFree((HLOCAL) pszMsg);
    return result;
}

int ttlib::CheckItemID(HWND hwnd, int id, const char* pszID, const char* filename, const char* function, int line)
{
    if (::GetDlgItem(hwnd, id) == NULL)
    {
        ttlib::cstr msg;
        msg << "Invalid dialog control id: " << pszID << " (" << id << ')';
        if (ttAssertionMsg(filename, function, line, pszID, msg.c_str()))
        {
            __debugbreak();
        }
    }
    return id;
}

// WARNING! Do not call ttASSERT in this function or you will end up with a recursive call.

void ttlib::wintrace(const std::string& msg, unsigned int type)
{
    if (msg.empty())
        return;

    // We don't want two threads trying to send text at the same time. The lock prevents a second call
    // to the function until the first call has completed.

    std::unique_lock<std::mutex> classLock(ttdbg::mutexTrace);

    // Until wintrace() is called for the first time, hwndTrace == -1, which means we search once, and only once to see
    // if it is running. If it's not running before the first call to wintrace(), then ttTRACE_FILTER(WMP_LAUNCH_TRACE)
    // needs to be called either after ttTrace.exe is run, or to have ttLib try to launch it.

    if (ttlib::hwndTrace == reinterpret_cast<HWND>(-1))
        ttlib::hwndTrace = FindWindowA(ttlib::txtTraceClass, NULL);

    if (!ttlib::hwndTrace)
        return;

    // If the trace program was closed, the window handle will be invalid, so set it to NULL and stop trying to send it
    // messages.
    if (!IsWindow(ttlib::hwndTrace))
    {
        ttlib::hwndTrace = NULL;
        return;
    }

    if (!ttdbg::hTraceMapping)
    {
        ttdbg::hTraceMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, ttlib::txtTraceShareName);
        if (!ttdbg::hTraceMapping)
        {
            ttlib::hwndTrace = NULL;
            return;
        }
    }

    if (!ttdbg::g_pszTraceMap)
    {
        ttdbg::g_pszTraceMap = (char*) MapViewOfFile(ttdbg::hTraceMapping, FILE_MAP_WRITE, 0, 0, 0);
        if (!ttdbg::g_pszTraceMap)
        {
            ttlib::hwndTrace = NULL;
            return;
        }
    }

    if (msg.size() > 4092)
        throw std::invalid_argument("wintrace msg must not exceed 4092 bytes");

    std::strcpy(ttdbg::g_pszTraceMap, msg.c_str());

    // For compatability with KeyView, ttTrace always add it's own \n character after receiving a WMP_GENERAL_MSG
    if (type != WMP_TRACE_GENERAL && type != WMP_SET_TITLE)
        std::strcat(ttdbg::g_pszTraceMap, "\n");

    SendMessageW(ttlib::hwndTrace, type, 0, 0);

    UnmapViewOfFile(ttdbg::g_pszTraceMap);
    ttdbg::g_pszTraceMap = nullptr;
}
