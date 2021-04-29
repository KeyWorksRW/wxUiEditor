/////////////////////////////////////////////////////////////////////////////
// Name:      ttcwd.h
// Purpose:   Class for storing and optionally restoring the current directory
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

// Note: this is a header-only class

#include "ttcstr.h"

namespace ttlib
{
    /// Retrieves the current working directory. Construct with (true) to restore the
    /// directory in the dtor.
    class cwd : public ttlib::cstr
    {
    public:
        // Specify true to restore the directory in the destructor
        cwd(bool restore = false)
        {
            assignCwd();
            if (restore)
                m_restore.assign(*this);
        }

        ~cwd()
        {
            if (!m_restore.empty())
                ttlib::ChangeDir(m_restore);
        }

        /// Assumes the current string is a directory path, and changes the directory to it.
        bool ChangeDir() { return ttlib::ChangeDir(*this); }

    private:
        std::string m_restore;
    };
}  // namespace ttlib
