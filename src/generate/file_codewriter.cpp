//////////////////////////////////////////////////////////////////////////
// Purpose:   Classs to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/file.h>      // wxFile - encapsulates low-level "file descriptor"
#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "file_codewriter.h"

#include "mainapp.h"       // App -- Main application class

int FileCodeWriter::WriteFile(bool test_only)
{
#if defined(_DEBUG)
    hasWriteFileBeenCalled = true;
#endif

    bool file_exists = m_filename.file_exists();
    if (file_exists)
    {
        wxFile file_original(m_filename, wxFile::read);
        if (file_original.IsOpened())
        {
            auto in_size = file_original.Length();
            if (m_buffer.size() == (to_size_t) in_size)
            {
                auto buffer = std::make_unique<unsigned char[]>(in_size);
                if (file_original.Read(buffer.get(), in_size) == in_size)
                {
                    if (std::memcmp(buffer.get(), m_buffer.data(), in_size) == 0)
                        return 0;  // origianal file is the same as current file
                }
            }
        }
    }

    // If we get here, the file needs to be written. If we're only testing if it needs writing, then return immediately.
    if (test_only)
        return 1;

    if (!file_exists)
    {
        ttString copy(m_filename);
        copy.remove_filename();
        if (copy.size() && !copy.dir_exists() && !wxGetApp().AskedAboutMissingDir(copy))
        {
            if (wxMessageBox(wxString() << "The directory " << copy << " doesn't exist.\n\nWould you like it to be created?",
                             "Generate Files", wxICON_WARNING | wxYES_NO) == wxYES)
            {
                wxFileName fn(copy);
                fn.Mkdir();
            }
            else
            {
                wxGetApp().AddMissingDir(copy);
            }
        }
    }

    wxFile fileOut;
    if (!fileOut.Create(m_filename, true))
    {
        return -1;
    }

    if (fileOut.Write(m_buffer.c_str(), m_buffer.length()) != m_buffer.length())
    {
        return -1;
    }

    return 1;  // file was successfully written
}
