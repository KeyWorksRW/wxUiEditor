/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common functions for generating Script Languages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <thread>

namespace ScriptCommon
{
    // Safely joins a thread with proper error handling.
    // Returns immediately if thread is not joinable.
    // Catches std::system_error and displays appropriate error message.
    void JoinThreadSafely(std::thread& thread);

}  // namespace ScriptCommon
