/////////////////////////////////////////////////////////////////////////////
// Purpose:   DataHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "data_handler.h"

#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes
#include <wx/zstream.h>   // zlib stream classes

#include <fstream>

// Normally, wxMemoryInputStream inputStream, wxZlibOutputStream outputStream
bool CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream, size_t original_size);

DataHandler& ProjectData = DataHandler::getInstance();

void DataHandler::Initialize(NodeSharedPtr /* project */, bool /* allow_ui */)
{
    m_embedded_data.clear();
}

bool DataHandler::LoadAndCompress(Node* node)
{
    ASSERT(node->isGen(gen_data_string) || node->isGen(gen_data_xml));
    auto filename = node->as_string(prop_data_file);
    if (!filename.size())
    {
        return false;
    }
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);
        file.close();

        // Include the trailing zero -- we need to read this back as a string, not a data array
        wxMemoryInputStream stream(buffer.data(), fileSize);

        wxMemoryOutputStream memory_stream;
        wxZlibOutputStream save_strem(memory_stream, wxZ_BEST_COMPRESSION);
        if (!CopyStreamData(&stream, &save_strem, stream.GetLength()))
        {
            // TODO: [KeyWorks - 03-16-2022] This would be really bad, though it should be impossible
            return false;
        }
        save_strem.Close();
        size_t org_size = (buffer.size() & 0xFFFFFFFF);
        size_t compressed_size = memory_stream.TellO();
        auto read_stream = memory_stream.GetOutputStreamBuffer();

        m_embedded_data[node->as_string(prop_string_name)] = std::make_unique<EmbeddedData>();
        auto embed = m_embedded_data[node->as_string(prop_string_name)].get();
        embed->type = 0;
        embed->array_size = (compressed_size | (org_size << 32));
        embed->array_data = std::make_unique<unsigned char[]>(compressed_size);
        memcpy(embed->array_data.get(), read_stream->GetBufferStart(), compressed_size);
        embed->filename = filename;
    }
    else
    {
        return false;
    }

    return true;
}
