/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <optional>

#pragma once

#include "ttcvector.h"
#include "tttextfile.h"

#include "../import/import_xml.h"  // ImportXML -- Base class for XML importing
#include "winres_form.h"           // resForm -- Process a Windows Resource form  (usually a dialog)

class NodeCreator;
class Node;

class WinResource : public ImportXML
{
public:
    WinResource();

    bool Import(const ttString& filename, bool write_doc) override;

    // If forms is empty, then all forms will be parsed
    bool ImportRc(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& forms);
    void InsertDialogs(std::vector<ttlib::cstr>& dialogs);

    std::optional<ttlib::cstr> FindIcon(const std::string& id);
    std::optional<ttlib::cstr> FindBitmap(const std::string& id);

    auto& GetIncludeLines() { return m_include_lines; }

protected:
    void FormToNode(resForm& form);
    void ParseDialog();

private:
    ttlib::cstr m_RcFilename;
    ttlib::cstr m_OutDirectory;
    ttlib::cstr m_outProjectName;

    wxString m_strErrorMsg;

    std::string strLanguage;

    ttlib::textfile m_file;

    std::vector<resForm> m_forms;
    std::set<ttlib::cstr> m_include_lines;

    std::map<std::string, ttlib::cstr> m_map_icons;
    std::map<std::string, ttlib::cstr> m_map_bitmaps;

    size_t m_curline;

    int m_codepage;
};
