/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ttcvector.h>
#include <tttextfile.h>

#include "winres/winres_form.h"       // rcForm -- Process a Windows Resource form  (usually a dialog)
#include "import_xml.h"        // ImportXML -- Base class for XML importing

class NodeCreator;
class Node;

class WinResource : public ImportXML
{
public:
    WinResource();

    bool Import(const ttString& filename, bool write_doc) override;
    bool ImportRc(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& m_dialogs);
    void InsertDialogs(std::vector<ttlib::cstr>& dialogs);

protected:
    void FormToNode(rcForm& form);
    void ParseDialog();

private:
    ttlib::cstr m_RcFilename;
    ttlib::cstr m_OutDirectory;
    ttlib::cstr m_outProjectName;

    wxString m_strErrorMsg;

    std::string strLanguage;

    ttlib::textfile m_file;

    std::vector<rcForm> m_forms;

    size_t m_curline;

    int m_codepage;
};
