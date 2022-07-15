/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <optional>

#pragma once

#include <tttextfile_wx.h>

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
    bool ImportRc(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& forms, bool isNested = false);
    void InsertDialogs(std::vector<ttlib::cstr>& dialogs);

    std::optional<ttlib::cstr> FindBitmap(const std::string& id);
    std::optional<ttlib::cstr> FindIcon(const std::string& id);

    // The strings have already been run through ConvertCodePageString().
    std::optional<ttlib::cstr> FindStringID(const std::string& id);

    auto& GetIncludeLines() { return m_include_lines; }

    // When compiled for Windows, this will convert the string to UTF8 using the current codepage.
    //
    // For all other platforms, this will assume a utf8 string -- which may be invalid.
    ttlib::cstr ConvertCodePageString(std::string_view str);

protected:
    void FormToNode(resForm& form);
    void ParseDialog(ttlib::textfile& file);
    void ParseMenu(ttlib::textfile& file);
    void ParseStringTable(ttlib::textfile& file);

private:
    ttlib::cstr m_RcFilename;
    ttlib::cstr m_OutDirectory;
    ttlib::cstr m_outProjectName;

    wxString m_strErrorMsg;

    std::string strLanguage;

    std::vector<resForm> m_forms;
    std::set<ttlib::cstr> m_include_lines;

    std::map<std::string, ttlib::cstr> m_map_bitmaps;
    std::map<std::string, ttlib::cstr> m_map_icons;
    std::map<std::string, ttlib::cstr> m_map_stringtable;

    size_t m_curline;

    int m_codepage;
};
