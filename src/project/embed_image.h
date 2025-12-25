/////////////////////////////////////////////////////////////////////////////
// Purpose:   EmbeddedImage class to manage images stored in the generated code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements EmbeddedImage, managing embedded bitmap resources for code
// generation. Each instance stores a vector of ImageInfo structs (m_images) representing
// multi-resolution image bundles where base_image() (m_images[0]) is the primary image and
// additional entries support HiDPI (@2x, _1_5x suffixes). ImageInfo contains filename (without
// path), array_name (valid C++ identifier derived from filename), array_data (zlib-compressed for
// SVG/XPM, original format for PNG/etc), array_size (lower 32 bits = compressed size, upper 32 bits
// = original size), file_time (modification tracking), and type (wxBitmapType enum). The class
// associates with m_form (Node* to owning form), tracks m_size (wxSize of base image), and provides
// get_bundle() which loads/updates images if file_time changed. UpdateImage() reloads modified
// files. Constructor converts filename to valid variable name handling UTF-8 and special characters
// for cross-language compatibility.

#pragma once

// SVG and XPM files only contain a single image. All other image types can contain multiple images,
// generally with different sizes to support different display resolutions. EmbeddedImage always
// stores the first image in the file as imgs[0] accessed via base_image().

#include <filesystem>
#include <vector>

#include <wx/bmpbndl.h>  // wxBitmapBundle class interface
#include <wx/gdicmn.h>   // wxSize class

#include "wxue_namespace/wxue_string.h"

class Node;
class wxImage;

struct ImageInfo
{
    wxue::string filename;  // original filename with extension, but without any path information

    // Valid C++ variable name derived from filename. This name is also compatible with Perl,
    // Python, and Ruby/
    wxue::string array_name;

    // SVG and XPM files are stored as zlib-compressed data. All other image types are stored
    // in their original format.
    std::vector<unsigned char> array_data;

    // The lower 32-bits contains the compressed size of array_data. The upper 32-bits contains the
    // original size of the image file before compression.
    uint64_t array_size;  // size of array_data in bytes

    wxDateTime file_time;  // time the file was last modified

    // E.G. wxBITMAP_TYPE_SVG, wxBITMAP_TYPE_XPM, wxBITMAP_TYPE_PNG, etc.
    wxBitmapType type;
};

class EmbeddedImage
{
public:
    // Converts the filename in path to a valid string name and sets imgs[0].array_name
    EmbeddedImage(wxue::string_view path, Node* form);
    ~EmbeddedImage() = default;

    // Note that this will update any file within EmbeddedImage whose file_time has changed
    // since the file was first loaded.
    //
    // size parameter is only used for SVG files
    auto get_bundle(wxSize size = { -1, -1 }) -> wxBitmapBundle;

    // imgs[0] is always the base image that can either be a single image, or the first image
    // in a bundle of images.
    auto base_image() -> ImageInfo& { return m_images[0]; }
    auto base_image() const -> const ImageInfo& { return m_images[0]; }

    auto add_ImageInfo() -> void { m_images.push_back(ImageInfo()); }

    auto get_ImageInfos() -> std::vector<ImageInfo>& { return m_images; }
    auto get_ImageInfos() const -> const std::vector<ImageInfo>& { return m_images; }

    auto get_ImageInfo(size_t index = 0) -> ImageInfo&
    {
        ASSERT_MSG(index < m_images.size(), "EmbeddedImage::get_ImageInfo() index out of range");
        if (index >= m_images.size())
        {
            index = 0;
        }
        return m_images[index];
    }

    [[nodiscard]] auto get_Form() const -> Node* { return m_form; }
    auto set_Form(Node* form) -> void { m_form = form; }

    [[nodiscard]] auto get_wxSize() const -> const wxSize& { return m_size; }
    auto set_wxSize(const wxSize& size) -> void { m_size = size; }

    // Uses image.GetSize() to set size member
    auto SetEmbedSize(const wxImage& image) -> void;

    // Call this is the image file has been modified. This will update the array_data and
    // array_size for the image from the updated image file.
    auto UpdateImage(ImageInfo& image_info) -> void;

private:
    // ctor/Initialize() will always create at least one ImageInfo structure
    std::vector<ImageInfo> m_images;

    // the form node the image is declared in
    Node* m_form;

    // dimensions of the first image in the array
    wxSize m_size;
};
