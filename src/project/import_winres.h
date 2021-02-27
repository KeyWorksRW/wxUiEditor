/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ttcvector.h>
#include <tttextfile.h>

#include "import_interface.h"  // ImportInterface -- Import interface
#include "winres_form.h"       // rcForm -- Process a Windows Resource form  (usually a dialog)

class NodeCreator;
class Node;

class WinResource : public ImportInterface
{
public:
    WinResource();

    bool Import(const ttString& filename) override;
    pugi::xml_document& GetDocument() override { return m_docOut; }

    bool ImportRc(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& m_dialogs);
    void InsertDialogs(std::vector<ttlib::cstr>& dialogs);

protected:
    void FormToNode(rcForm& form);
    void ParseDialog();

private:
    ttlib::cstr m_RcFilename;
    ttlib::cstr m_OutDirectory;
    ttlib::cstr m_outProjectName;

    pugi::xml_document m_docOut;

    wxString m_strErrorMsg;

    std::string strLanguage;

    ttlib::textfile m_file;

    std::vector<rcForm> m_forms;
    Node* m_project;

    size_t m_curline;

    int m_codepage;
};
