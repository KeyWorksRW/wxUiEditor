/////////////////////////////////////////////////////////////////////////////
// Purpose:   EmbeddedImage class to manage images stored in the generated code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <vector>

#include <wx/bmpbndl.h>  // wxBitmapBundle class interface
#include <wx/gdicmn.h>   // wxSize class

class Node;
class wxImage;

struct ImageInfo
{
    tt_string filename;  // original filename with extension, but without any path information

    // Valid C++ variable name derived from filename. This name is also compatible with Perl,
    // Python, and Ruby/
    tt_string array_name;

    // SVG and XPM files are stored as zlib-compressed data. All other image types are stored
    // in their original format.
    std::unique_ptr<unsigned char[]> array_data;
    uint64_t array_size;  // size of array_data in bytes

    std::filesystem::file_time_type file_time;  // time the file was last modified

    // E.G. wxBITMAP_TYPE_SVG, wxBITMAP_TYPE_XPM, wxBITMAP_TYPE_PNG, etc.
    wxBitmapType type;
};

class EmbeddedImage
{
public:
    // Converts the filename in path to a valid string name and sets imgs[0].array_name
    EmbeddedImage(tt_string_view path, Node* form);
    ~EmbeddedImage() = default;

    // Note that this will update any file within EmbeddedImage whose file_time has changed
    // since the file was first loaded.
    //
    // size parameter is only used for SVG files
    wxBitmapBundle get_bundle(wxSize size = { -1, -1 });

    // imgs[0] is always the base image that can either be a single image, or the first image
    // in a bundle of images.
    ImageInfo& base_image() { return m_images[0]; }
    const ImageInfo& base_image() const { return m_images[0]; }

    void add_ImageInfo() { m_images.push_back(ImageInfo()); }

    std::vector<ImageInfo>& get_ImageInfos() { return m_images; }
    const std::vector<ImageInfo>& get_ImageInfos() const { return m_images; }

    ImageInfo& get_ImageInfo(size_t index = 0)
    {
        ASSERT_MSG(index < m_images.size(), "EmbeddedImage::get_ImageInfo() index out of range");
        if (index >= m_images.size())
            index = 0;
        return m_images[index];
    }

    Node* get_Form() const { return m_form; }
    void set_Form(Node* form) { m_form = form; }

    const wxSize& get_wxSize() const { return m_size; }
    void set_wxSize(const wxSize& size) { m_size = size; }

    // Uses image.GetSize() to set size member
    void SetEmbedSize(const wxImage& image);

    // Call this is the image file has been modified. This will update the array_data and
    // array_size for the image from the updated image file.
    void UpdateImage(ImageInfo& image_info);

private:
    // ctor/Initialize() will always create at least one ImageInfo structure
    std::vector<ImageInfo> m_images;

    // the form node the image is declared in
    Node* m_form;

    // dimensions of the first image in the array
    wxSize m_size;
};
