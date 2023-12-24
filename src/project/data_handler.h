/////////////////////////////////////////////////////////////////////////////
// Purpose:   DataHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "node_classes.h"  // Forward defintions of Node classes

class Code;
class WriteCode;

struct EmbeddedData
{
    tt_string filename;
    size_t array_size;
    std::unique_ptr<unsigned char[]> array_data;
    size_t type;  // 0 = string, 1 = xml, tt::npos = not_found
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

    // Generate extern references to all the data variables.
    //
    // This will call code.clear() before writing any code.
    void WriteDataPreConstruction(Code& code);

    // Generate data list construction code in source
    //
    // This will call code.clear() before writing any code.
    void WriteDataConstruction(Code& code, WriteCode* source);

    // Write extern statements to the header file
    void WriteImagePostHeader(WriteCode* header);

protected:
    bool LoadAndCompress(const Node* node);

private:
    std::map<std::string, EmbeddedData, std::less<>> m_embedded_data;
};

extern DataHandler& ProjectData;

namespace data_list
{
    Node* FindDataList();

};  // namespace data_list
