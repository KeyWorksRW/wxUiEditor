/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common functions for generating Script Languages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wx/msgdlg.h>

#include "gen_script_common.h"  // Common functions for generating Script Languages

namespace ScriptCommon
{
    void JoinThreadSafely(std::thread& thread)
    {
        if (!thread.joinable())
        {
            return;
        }

        try
        {
            thread.join();
        }
        catch (const std::system_error& err)
        {
#if defined(_DEBUG)
            MSG_ERROR(err.what());
#else
            wxMessageDialog dlg_error(nullptr, wxString::FromUTF8(err.what()),
                                      "Internal Thread Error", wxICON_ERROR | wxOK);
            dlg_error.ShowModal();
#endif  // _DEBUG
        }
    }

}  // namespace ScriptCommon
