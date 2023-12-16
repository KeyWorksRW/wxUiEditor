/////////////////////////////////////////////////////////////////////////////
// Purpose:   DataHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "node_classes.h"  // Forward defintions of Node classes

struct EmbeddedData
{
    tt_string filename;
    size_t array_size;
    std::unique_ptr<unsigned char[]> array_data;
    size_t type;  // 0 = string, 1 = xml
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

    void Initialize(NodeSharedPtr project, bool allow_ui = true);

    bool LoadAndCompress(Node* node);

private:
    std::map<std::string, std::unique_ptr<EmbeddedData>, std::less<>> m_embedded_data;
};

extern DataHandler& ProjectData;
