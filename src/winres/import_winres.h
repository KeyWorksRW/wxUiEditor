/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements WinResource, an importer for Windows .rc resource files. The
// class extends ImportXML and parses text-based .rc format (not XML despite base class) using
// tt_string_vector line processing. Import/ImportRc load .rc files, InsertDialogs filters dialog
// selection, ParseDialog/ParseMenu/ParseStringTable extract resources, and FormToNode converts
// resForm structures to Nodes. Resource lookup methods (FindBitmap, FindIcon, FindStringID) access
// m_map_bitmaps, m_map_icons, m_map_stringtable populated during parsing. The class handles Windows
// code page conversion via ConvertCodePageString (m_codepage tracking) and #include directive
// tracking (m_include_lines). m_forms stores parsed resForm objects before Node conversion. The
// importer translates Windows Resource format (dialog units, Windows controls, resource IDs,
// STRINGTABLE) to wxWidgets equivalents, supporting legacy Windows application migration to
// wxWidgets with wxUiEditor.

#include <optional>

#pragma once

#include "../import/import_xml.h"  // ImportXML -- Base class for XML importing
#include "winres_form.h"           // resForm -- Process a Windows Resource form  (usually a dialog)

class NodeCreator;
class Node;

class WinResource : public ImportXML
{
public:
    WinResource();

    bool Import(const std::string& filename, bool write_doc) override;

    // If forms is empty, then all forms will be parsed
    bool ImportRc(const wxue::string& rc_file, std::vector<wxue::string>& forms,
                  bool isNested = false);
    void InsertDialogs(std::vector<wxue::string>& dialogs);

    std::optional<wxue::string> FindBitmap(const std::string& id);
    std::optional<wxue::string> FindIcon(const std::string& id);

    // The strings have already been run through ConvertCodePageString().
    std::optional<wxue::string> FindStringID(const std::string& id);

    auto& GetIncludeLines() { return m_include_lines; }

    // When compiled for Windows, this will convert the string to UTF8 using the current codepage.
    //
    // For all other platforms, this will assume a utf8 string -- which may be invalid.
    wxue::string ConvertCodePageString(std::string_view str);

protected:
    void FormToNode(resForm& form);
    void ParseDialog(wxue::StringVector& file);
    void ParseMenu(wxue::StringVector& file);
    void ParseStringTable(wxue::StringVector& file);

private:
    wxue::string m_RcFilename;
    wxue::string m_OutDirectory;
    wxue::string m_outProjectName;

    wxString m_strErrorMsg;

    std::string strLanguage;

    std::vector<resForm> m_forms;
    std::set<wxue::string> m_include_lines;

    std::map<std::string, wxue::string> m_map_bitmaps;
    std::map<std::string, wxue::string> m_map_icons;
    std::map<std::string, wxue::string> m_map_stringtable;

    size_t m_curline;

    int m_codepage;
};
