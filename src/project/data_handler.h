/////////////////////////////////////////////////////////////////////////////
// Purpose:   DataHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements DataHandler, a singleton manager (Meyer's pattern) for embedded
// data files in wxUiEditor projects. The class stores compressed data in m_embedded_data
// (map<string, EmbeddedData>) where each entry contains compressed array_data, original filename,
// file modification time, and type flags (string, xml, or binary). Initialize() scans Data List
// nodes, LoadAndCompress() handles file reading and zlib compression with optional XML condensing
// (removing whitespace/headers), and WriteDataConstruction() generates code declaring extern
// unsigned char arrays. The class tracks file_time to detect modifications and supports xml_flags
// for format control (xml_condense_format, xml_trim_whitespace). NeedsUtilityHeader() determines if
// zlib decompression helpers are needed. This enables embedding configuration files, XML resources,
// and text data into generated code.

#pragma once

#include <wx/datetime.h>  // declarations of time/date related classes (wxDateTime,

#include <filesystem>
#include <map>

class Code;
class WriteCode;

enum xml_flags : uint32_t
{
    xml_none = 0,
    xml_condense_format = 1 << 0,  // Don't write the XML header
    xml_trim_whitespace = 1 << 1,  // Don't indent the XML
};

struct EmbeddedData
{
    tt_string filename;
    size_t array_size;
    std::vector<unsigned char> array_data;
    size_t type;                                // 0 = string, 1 = xml, tt::npos = not_found
    std::filesystem::file_time_type file_time;  // time the file was last modified
    bool xml_condensed;  // true if node->as_bool(prop_xml_condensed_format) is true
};

class DataHandler
{
private:
    DataHandler() {}

public:
    DataHandler(DataHandler const&) = delete;

    void operator=(DataHandler const&) = delete;

    static DataHandler& getInstance()
    {
        static DataHandler instance;
        return instance;
    }

    // Call this whenever a project is loaded
    void Clear() { m_embedded_data.clear(); }

    // Only call this when the datalist code needs to be generated.
    void Initialize();

    // Generate data list construction code in source
    //
    // This will call code.clear() before writing any code.
    void WriteDataConstruction(Code& code, WriteCode* source);

    // Write extern statements to the header file
    void WriteImagePostHeader(WriteCode* header);

    bool NeedsUtilityHeader() const;

protected:
    bool LoadAndCompress(Node* node);

private:
    std::map<std::string, EmbeddedData, std::less<>> m_embedded_data;
};

extern DataHandler& ProjectData;

namespace data_list
{
    Node* FindDataList();

};  // namespace data_list
