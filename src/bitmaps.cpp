/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains various images handling functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// GetHeaderImage() reads the actual header file and converts that text into binary data and then loads the image.

// LoadHeaderImage() is used to load the image from a #included header file (the compiler already converted it into binary
// data)

#include <fstream>

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl
#include <wx/mstream.h>  // Memory stream classes

#include "bitmaps.h"

// [KeyWorks - 05-04-2021] Note that we don't display warnings or errors to the user since this will be called during project
// loading, and there could be dozens of calls to the same problem file(s).

wxImage GetHeaderImage(ttlib::cview filename, size_t* p_original_size, ttString* p_mime_type)
{
    wxImage image;

    if (!ttlib::file_exists(filename))
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetHeaderImage doesn't exist");
        return image;
    }

    std::ifstream fileOriginal(filename, std::ios::binary | std::ios::in);
    if (!fileOriginal.is_open())
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetHeaderImage could not be read");
        return image;
    }
    std::string in_buf(std::istreambuf_iterator<char>(fileOriginal), {});
    if (in_buf.size() < 1)
    {
        MSG_ERROR(ttlib::cstr() << filename << " is empty!");
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
        MSG_ERROR(ttlib::cstr() << filename << " doesn't contain an opening brace");
        return image;
    }

    bool isUiditorFile { true };

    if (!image_buffer_size)
    {
        // wxFormBuilder doesn't add the size of the image in brackets so we have to parse it without storing the
        // results in order to calculate the size, then parse it again to actually store the data in an image buffer.
        isUiditorFile = false;

        auto save_ptr = buf_ptr;

        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
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
                {
                    FAIL_MSG(ttlib::cstr() << filename << " doesn't contain a closing brace");
                    wxMessageBox((ttlib::cstr() << filename << " doesn't contain a closing brace").wx_str());
                    return image;
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);

        buf_ptr = save_ptr;
    }

    // We don't have a reliable way of identifying either wxFormBuilder or wxUiEditor generated graphics header files, so we
    // start by making certain the image buffer is at least a possible size. It might still be wrong, but we'll fail to find
    // a handler to load it, so that should be a sufficient way of trying to load some random  header file.

    if (image_buffer_size < 4 || image_buffer_size > in_buf.size() / 2)
    {
        MSG_ERROR(ttlib::cstr() << filename << " is not a valid graphics header file");
        return image;
    }

    auto image_buffer = std::make_unique<unsigned char[]>(image_buffer_size);
    unsigned char* ptr_out_buf = image_buffer.get();

    if (isUiditorFile)
    {
        // This section is for wxUiEditor which uses decimal and no spaces
        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                unsigned char value = (to_uchar) (*buf_ptr - '0');
                for (++buf_ptr; *buf_ptr >= '0' && *buf_ptr <= '9'; ++buf_ptr)
                {
                    value = (value * 10) + (to_uchar) (*buf_ptr - '0');
                }
                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
                    return image;
                }
            }
            else
            {
                ++buf_ptr;
            }
        } while (*buf_ptr != '}' && *buf_ptr);
    }

    else
    {
        // This section is for wxFormBuilder which uses hexadecimal and a lot of whitespace
        do
        {
            // The format is assumed to be "0x" followed by two hexadcimal digits

            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                    buf_ptr += 2;

                unsigned char value = 0;

                // Get the high value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value = (to_uchar) (*buf_ptr - '0') * 16;
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value = (to_uchar) ((*buf_ptr - 'A') + 10) * 16;
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value = (to_uchar) ((*buf_ptr - 'a') + 10) * 16;

                ++buf_ptr;

                // Get the low value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value += (to_uchar) (*buf_ptr - '0');
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value += (to_uchar) ((*buf_ptr - 'A') + 10);
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value += (to_uchar) ((*buf_ptr - 'a') + 10);

                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
                    return image;
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);
    }
    wxMemoryInputStream stream(image_buffer.get(), actual_size);

    wxImageHandler* handler;
    auto& list = wxImage::GetHandlers();
    for (auto node = list.GetFirst(); node; node = node->GetNext())
    {
        handler = (wxImageHandler*) node->GetData();
        if (handler->CanRead(stream))
        {
            if (handler->LoadFile(&image, stream))
            {
                if (p_original_size)
                    *p_original_size = actual_size;
                if (p_mime_type)
                    *p_mime_type = handler->GetMimeType();

                return image;
            }
        }
    }

    return image;
}

// This is almost identical to GetImageFromArray() -- the only difference is that this one
// first tries to load the image via the PNG handler.
wxImage LoadHeaderImage(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream stream(data, size_data);
    wxImage image;

    // Images are almost always in PNG format, so check that first. If it fails, then let wxWidgets figure out the format.

    auto handler = wxImage::FindHandler(wxBITMAP_TYPE_PNG);
    if (handler && handler->CanRead(stream) && handler->LoadFile(&image, stream))
    {
        return image;
    }

    image.LoadFile(stream);
    return image;
};

bool GetAnimationImage(wxAnimation& animation, ttlib::cview filename)
{
    if (!ttlib::file_exists(filename))
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetAnimationanimation doesn't exist");
        return animation.IsOk();
    }

    std::ifstream fileOriginal(filename, std::ios::binary | std::ios::in);
    if (!fileOriginal.is_open())
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetAnimationImage could not be read");
        return animation.IsOk();
    }
    std::string in_buf(std::istreambuf_iterator<char>(fileOriginal), {});
    if (in_buf.size() < 1)
    {
        MSG_ERROR(ttlib::cstr() << filename << " is empty!");
        return animation.IsOk();
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
        MSG_ERROR(ttlib::cstr() << filename << " doesn't contain an opening brace");
        return animation.IsOk();
    }

    bool isUiditorFile { true };

    if (!image_buffer_size)
    {
        // wxFormBuilder doesn't add the size of the animation in brackets so we have to parse it without storing the
        // results in order to calculate the size, then parse it again to actually store the data in an animation buffer.
        isUiditorFile = false;

        auto save_ptr = buf_ptr;

        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
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
                {
                    FAIL_MSG(ttlib::cstr() << filename << " doesn't contain a closing brace");
                    wxMessageBox((ttlib::cstr() << filename << " doesn't contain a closing brace").wx_str());
                    return animation.IsOk();
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);

        buf_ptr = save_ptr;
    }

    // We don't have a reliable way of identifying either wxFormBuilder or wxUiEditor generated graphics header files, so we
    // start by making certain the image buffer is at least a possible size. It might still be wrong, but we'll fail to find
    // a handler to load it, so that should be a sufficient way of trying to load some random  header file.

    if (image_buffer_size < 4 || image_buffer_size > in_buf.size() / 2)
    {
        MSG_ERROR(ttlib::cstr() << filename << " is not a valid graphics header file");
        return animation.IsOk();
    }

    auto image_buffer = std::make_unique<unsigned char[]>(image_buffer_size);
    unsigned char* ptr_out_buf = image_buffer.get();

    if (isUiditorFile)
    {
        // This section is for wxUiEditor which uses decimal and no spaces
        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                unsigned char value = (to_uchar) (*buf_ptr - '0');
                for (++buf_ptr; *buf_ptr >= '0' && *buf_ptr <= '9'; ++buf_ptr)
                {
                    value = (value * 10) + (to_uchar) (*buf_ptr - '0');
                }
                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
                    return animation.IsOk();
                }
            }
            else
            {
                ++buf_ptr;
            }
        } while (*buf_ptr != '}' && *buf_ptr);
    }

    else
    {
        // This section is for wxFormBuilder which uses hexadecimal and a lot of whitespace
        do
        {
            // The format is assumed to be "0x" followed by two hexadcimal digits

            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                    buf_ptr += 2;

                unsigned char value = 0;

                // Get the high value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value = (to_uchar) (*buf_ptr - '0') * 16;
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value = (to_uchar) ((*buf_ptr - 'A') + 10) * 16;
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value = (to_uchar) ((*buf_ptr - 'a') + 10) * 16;

                ++buf_ptr;

                // Get the low value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value += (to_uchar) (*buf_ptr - '0');
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value += (to_uchar) ((*buf_ptr - 'A') + 10);
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value += (to_uchar) ((*buf_ptr - 'a') + 10);

                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
                    return animation.IsOk();
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);
    }
    wxMemoryInputStream stream(image_buffer.get(), actual_size);
    animation.Load(stream);
    return animation.IsOk();
}

wxAnimation LoadAnimationImage(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream stream(data, size_data);
    wxAnimation animation;
    animation.Load(stream);
    return animation;
};
