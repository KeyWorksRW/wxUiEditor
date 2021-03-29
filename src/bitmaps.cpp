/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains various images handling functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <fstream>
#include <map>

#include <wx/mstream.h>  // Memory stream classes

#include <tttextfile.h>

#include "bitmaps.h"

wxImage convertPngHeader(ttlib::cview filename)
{
    wxImage image;

    if (!ttlib::file_exists(filename))
    {
        MSG_WARNING(ttlib::cstr() << filename << " passed to convertPngHeader doesn't exist")
        return image;
    }

    ttlib::viewfile file;
    if (!file.ReadFile(filename))
    {
        MSG_WARNING(ttlib::cstr() << filename << " passed to convertPngHeader could not be read")
        return image;
    }

    size_t pos = file.FindLineContaining("const unsigned char");
    ASSERT(ttlib::is_found(pos));
    if (!ttlib::is_found(pos))
    {
        MSG_WARNING(ttlib::cstr() << filename << " is not one of our png header files")
        return image;
    }

    size_t buffer_size = file[pos + 1].size();
    if (auto bracket = file[pos].find('['); ttlib::is_found(bracket))
    {
        buffer_size = ttlib::atoi(file[pos].data() + bracket + 1);
    }
    auto data_in = file[pos + 1];
    auto buffer = std::make_unique<unsigned char[]>(buffer_size);
    unsigned char* pOut = buffer.get();

    unsigned char octal;

    // Start pos at 1 to skip over quote, reduce size by -3 is to ignore the leading and trailing quotes and trailing
    // semi-colon
    for (pos = 1; pos < data_in.size() - 3; ++pos)
    {
        if (data_in[pos] == '\\')
        {
            ++pos;
            if (data_in[pos] >= '0' && data_in[pos] <= '7')
            {
                octal = data_in[pos] - '0';
                if (data_in[pos + 1] >= '0' && data_in[pos + 1] <= '7')
                {
                    octal <<= 3;
                    octal += ((data_in[pos + 1] - '0'));
                    ++pos;

                    if (data_in[pos + 1] >= '0' && data_in[pos + 1] <= '7')
                    {
                        octal <<= 3;
                        octal += ((data_in[pos + 1] - '0'));
                        ++pos;
                    }
                }
                *pOut++ = octal;
                continue;
            }
            else
            {
                switch (data_in[pos])
                {
                    case '\'':
                        *pOut++ = 0x27;
                        continue;

                    case '"':
                        *pOut++ = 0x22;
                        continue;

                    case '?':
                        *pOut++ = 0x3f;
                        continue;

                    case '\\':
                        *pOut++ = 0x5c;
                        continue;

                    case 'a':
                        *pOut++ = 0x07;
                        continue;

                    case 'b':
                        *pOut++ = 0x08;
                        continue;

                    case 'f':
                        *pOut++ = 0x0c;
                        continue;

                    case 'n':
                        *pOut++ = 0x0a;
                        continue;

                    case 'r':
                        *pOut++ = 0x0d;
                        continue;

                    case 't':
                        *pOut++ = 0x09;
                        continue;

                    case 'v':
                        *pOut++ = 0x0b;
                        continue;
                }
            }
        }
        else
        {
            *pOut++ = data_in[pos];
        }
    }

    ASSERT((size_t)(pOut - buffer.get()) <= buffer_size);

    wxMemoryInputStream strm(buffer.get(), pOut - buffer.get());
    image.LoadFile(strm, wxBITMAP_TYPE_PNG);

    return image;
}

wxImage GetHeaderImage(ttlib::cview filename)
{
    wxImage image;

    if (!ttlib::file_exists(filename))
    {
        FAIL_MSG(ttlib::cstr() << filename << " passed to GetHeaderImage doesn't exist");
        return image;
    }

    std::ifstream fileOriginal(filename, std::ios::binary | std::ios::in);
    if (!fileOriginal.is_open())
    {
        FAIL_MSG(ttlib::cstr() << filename << " passed to GetHeaderImage could not be read");
        return image;
    }
    std::string in_buf(std::istreambuf_iterator<char>(fileOriginal), {});
    if (in_buf.size() < 1)
    {
        FAIL_MSG(ttlib::cstr() << filename << " is empty!");
        return image;
    }

    // Turn this into a null-terminated string. We don't care what's at the end, because it won't be a number.
    in_buf[in_buf.size() - 1] = 0;

    size_t image_buffer_size = 0;
    size_t actual_size = 0;

    auto buf_ptr = strchr(in_buf.c_str(), '[');
    if (buf_ptr)
    {
        image_buffer_size = ttlib::atoi(++buf_ptr);
    }

    buf_ptr = strchr(buf_ptr, '{');
    if (!buf_ptr)
    {
        FAIL_MSG(ttlib::cstr() << filename << " does contain and opening brace");
        return image;
    }

    if (!image_buffer_size)
    {
        // wxFormBuilder doesn't add the size of the image in brackets so we have to parse it without storing the
        // results in order to calculate the size, then parse it again to actually store the data in an image buffer.

        auto save_ptr = buf_ptr;

        do
        {
            if (ttlib::is_digit(*buf_ptr))
            {
                ++image_buffer_size;

                // Step over any hex digit
                if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                    buf_ptr += 2;

                // Now step over the digit, then fall through and step over the first character following the digit
                do
                {
                    ++buf_ptr;
                } while (ttlib::is_digit(*buf_ptr));

                if (!*buf_ptr)
                    break;  // This will be a corrupted file since it didn't end with a closing '}' character
            }
            ++buf_ptr;
        } while (*buf_ptr);

        buf_ptr = save_ptr;
    }

    auto image_buffer = std::make_unique<unsigned char[]>(image_buffer_size);
    unsigned char* ptr_out_buf = image_buffer.get();

    do
    {
        if (ttlib::is_digit(*buf_ptr))
        {
            ptr_out_buf[actual_size] = static_cast<unsigned char>(ttlib::atoi(buf_ptr));

            if (++actual_size > image_buffer_size)
            {
                FAIL_MSG(ttlib::cstr() << filename << " actual image size is larger that the size specified in brackets");
                return image;
            }

            // Step over any hex digit
            if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                buf_ptr += 2;

            // Now step over the digit, then fall through and step over the first character following the digit
            do
            {
                ++buf_ptr;
            } while (ttlib::is_digit(*buf_ptr));

            if (!*buf_ptr)
                break;  // This will be a corrupted file since it didn't end with a closing '}' character
        }
        ++buf_ptr;
    } while (*buf_ptr);

    wxMemoryInputStream strm(image_buffer.get(), actual_size);
    image.LoadFile(strm);

    return image;
}
