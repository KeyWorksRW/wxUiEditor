/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert compiled GZIP arrays into original strings/images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/archive.h>  // Streams for archive formats
#include <wx/image.h>    // wxImage class
#include <wx/mstream.h>  // Memory stream classes
#include <wx/stream.h>   // stream classes

static bool CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream, size_t size = tt::npos);

// Data must point to a .gz compressed array containing the image

wxImage LoadGzipImage(const unsigned char* data, size_t size_data)
{
    ASSERT(data);

    wxImage image;

    auto filterClassFactory = wxFilterClassFactory::Find(".gz", wxSTREAM_FILEEXT);
    if (!filterClassFactory)
        return image;

    wxMemoryInputStream stream_in(data, size_data);
    wxScopedPtr<wxFilterInputStream> filterStream(filterClassFactory->NewStream(stream_in));

    wxMemoryOutputStream stream_out;
    if (!CopyStreamData(filterStream.get(), &stream_out))
    {
        return image;
    }
    stream_out.Close();

    auto strm_buffer = stream_out.GetOutputStreamBuffer();
    strm_buffer->Seek(0, wxFromStart);

    wxMemoryInputStream strm_image(strm_buffer->GetBufferStart(), strm_buffer->GetBufferSize());
    image.LoadFile(strm_image);
    return image;
}

// Data must point to a .gz compressed array containing the string

std::string LoadGzipString(const unsigned char* data, size_t size_data)
{
    ASSERT(data);

    std::string str;

    auto filterClassFactory = wxFilterClassFactory::Find(".gz", wxSTREAM_FILEEXT);
    if (!filterClassFactory)
        return str;

    wxMemoryInputStream stream_in(data, size_data);
    wxScopedPtr<wxFilterInputStream> filterStream(filterClassFactory->NewStream(stream_in));

    wxMemoryOutputStream stream_out;
    if (!CopyStreamData(filterStream.get(), &stream_out))
    {
        return str;
    }
    stream_out.Close();

    auto strm_buffer = stream_out.GetOutputStreamBuffer();
    strm_buffer->Seek(0, wxFromStart);

    str.assign(static_cast<char*>(strm_buffer->GetBufferStart()), strm_buffer->GetBufferSize());
    return str;
}

static bool CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream, size_t size)
{
    size_t buf_size;
    if (size == tt::npos || size > (16 * 1024))
        buf_size = (16 * 1024);
    else
        buf_size = static_cast<size_t>(size);

    auto read_buf = std::make_unique<unsigned char[]>(buf_size);
    auto read_size = buf_size;

    size_t copied_data = 0;
    for (;;)
    {
        if (size != tt::npos && copied_data + read_size > size)
            read_size = size - copied_data;
        inputStream->Read(read_buf.get(), read_size);

        auto actually_read = inputStream->LastRead();
        outputStream->Write(read_buf.get(), actually_read);
        if (outputStream->LastWrite() != actually_read)
        {
            FAIL_MSG("Failed to output data");
            return false;
        }

        if (size == tt::npos)
        {
            if (inputStream->Eof())
                break;
        }
        else
        {
            copied_data += actually_read;
            if (copied_data >= size)
                break;
        }
    }

    return true;
}
