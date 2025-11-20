/////////////////////////////////////////////////////////////////////////////
// Purpose:   ImageHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

// AI Context [imagehandler]: ImageHandler is a global singleton accessed via ProjectImages. It
// manages all project images with these key responsibilities:
//
// 1. Image Storage & Caching:
//    - m_map_embedded: Stores compressed embedded images (PNG, SVG, XPM) in EmbeddedImage objects
//    - m_bundles: Maps image property descriptions to ImageBundle (multi-resolution image sets)
//    - m_images: Caches non-embedded images (XPM, Art provider images)
//
// 2. Bundle Management:
//    - Automatically detects and groups related images by suffix (_16x16, _24x24, _32x32)
//    - Supports HiDPI scaling suffixes (@1_5x, @2x, _1_5x, _2x)
//    - Creates wxBitmapBundle from multiple resolutions for sharp rendering at any DPI
//
// 3. Image Property Format (semicolon-separated):
//    - Type;Path[;Size] where Type = "Embed", "Art", "SVG", "XPM", "Header"
//    - Example: "Embed;images/icon_16x16.png" or "Art;wxART_FILE_OPEN|wxART_TOOLBAR"
//
// 4. Compression & Optimization:
//    - SVG files: XML parsing removes metadata, zlib compression
//    - XPM files: zlib compression
//    - PNG files: Re-compressed if smaller than original
//
// 5. Form Association:
//    - Each EmbeddedImage tracks which form node first uses it
//    - Images List (gen_Images) nodes can contain shared embedded images
//
// Key Methods:
//    - CollectBundles(): Scans entire project to build bundle maps (called on project load)
//    - ProcessBundleProperty(): Creates/updates bundles for a specific property
//    - GetPropertyBitmapBundle(): Retrieves cached bundle for UI display
//    - AddEmbeddedImage(): Adds new image and auto-detects multi-resolution variants

#include <array>
#include <map>

#include <wx/bmpbndl.h>  // includes wx/bitmap.h, wxBitmapBundle class interface

#include "tt_string_vector.h"  // tt_string_vector -- Read/Write line-oriented strings/files

#include "embed_image.h"  // EmbeddedImage class

class Node;
class wxAnimation;

using NodeSharedPtr = std::shared_ptr<Node>;

// This simply contains a list of the filenames that would be used to create a bundle.
struct ImageBundle
{
    std::vector<tt_string> lst_filenames;
};

class ImageHandler
{
private:
    ImageHandler() = default;

public:
    ImageHandler(ImageHandler const&) = delete;
    ImageHandler(ImageHandler&&) = delete;
    auto operator=(ImageHandler const&) -> ImageHandler& = delete;
    auto operator=(ImageHandler&&) -> ImageHandler& = delete;

    ~ImageHandler() = default;

    static auto getInstance() -> ImageHandler&
    {
        static ImageHandler instance;
        return instance;
    }

    void Initialize(NodeSharedPtr project, bool allow_ui = true);

    // This will parse the entire project, and ensure that each embedded image is associated
    // with the form node of the form it first appears in.
    //
    // Returns true if an associated node changed
    auto UpdateEmbedNodes() -> bool;

    auto GetImage(const tt_string& description) -> wxImage;

    auto GetBitmapBundle(const tt_string& description) -> wxBitmapBundle;

    // This takes the full bitmap property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    //
    // If check_image is true, and !image.IsOK(), GetInternalImage() is returned
    wxImage GetPropertyBitmap(const tt_string_vector& parts, bool check_image = true);

    auto GetPropertyBitmap(const tt_string& description, bool check_image = true) -> wxImage
    {
        tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return GetPropertyBitmap(parts, check_image);
    }

    wxBitmapBundle GetPropertyBitmapBundle(tt_string_view description);

    // ImageBundle contains the filenames of each image in the bundle, needed to generate the
    // code for the bundle.
    //
    // Returns nullptr if there is no ImageBundle
    auto GetPropertyImageBundle(const tt_string_vector& parts, Node* node = nullptr)
        -> const ImageBundle*;
    auto GetPropertyImageBundle(tt_string_view description, Node* node = nullptr)
        -> const ImageBundle*
    {
        tt_string_vector parts(description, ';', tt::TRIM::both);
        return GetPropertyImageBundle(parts, node);
    }

    // If there is an Image form containing this bundle, return it's function name
    auto GetBundleFuncName(const tt_string& description) -> tt_string;

    // If there is an Images List containing this bundle, return it's function name
    auto GetBundleFuncName(const tt_string_vector& parts) -> tt_string;

    // If there is an Images List containing this image, return it's function name
    auto GetBundleFuncName(const EmbeddedImage* embed, wxSize svg_size = wxDefaultSize)
        -> tt_string;

    auto ProcessBundleProperty(const tt_string_vector& parts, Node* node) -> ImageBundle*;

    auto ProcessBundleProperty(const tt_string& description, Node* node) -> ImageBundle*
    {
        tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return ProcessBundleProperty(parts, node);
    }

    // This adds the bundle if new, or updates the embed->form if the node has changed
    void UpdateBundle(const tt_string_vector& parts, Node* node);

    // This takes the full animation property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    void GetPropertyAnimation(const tt_string& description, wxAnimation* p_animate);

    auto AddEmbeddedImage(tt_string path, Node* form, bool is_animation = false) -> bool;
    auto GetEmbeddedImage(tt_string_view path) -> EmbeddedImage*;

    // This will collect bundles for the entire project -- it initializes m_bundles and
    // m_map_embedded for every image.
    void CollectBundles();

    // Returns nullptr if the image is not found
    auto FindEmbedded(std::string_view) -> EmbeddedImage*;

    // Returns false if the new art folder does not exist.
    auto ArtFolderChanged() -> bool;

protected:
    auto CheckNode(Node* node) -> bool;

    void CollectNodeBundles(Node* node, Node* form);

    // This will update both m_bundles and m_map_embedded
    auto AddNewEmbeddedImage(const tt_string& path, Node* form) -> bool;

    // Reads the image and stores it in m_map_embedded
    auto AddEmbeddedBundleImage(tt_string& path, Node* form, EmbeddedImage* embed = nullptr)
        -> EmbeddedImage*;
    // This will call AddSvgBundleImage(), AddXpmBundleImage() or AddEmbeddedBundleImage()
    // depending on the type of the image file.
    auto AddNewEmbeddedBundle(const tt_string_vector& parts, std::string_view org_path, Node* form)
        -> bool;

    auto AddNewEmbeddedBundle(const tt_string& description, std::string_view org_path, Node* form)
        -> bool
    {
        tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return AddNewEmbeddedBundle(parts, org_path, form);
    }

    // Reads the image, remove unused metadat, compresses it and stores it in m_map_embedded
    auto AddSvgBundleImage(tt_string& path, Node* form) -> bool;

    // Read the image, compresses it and stores it in m_map_embedded
    auto AddXpmBundleImage(const tt_string& path, Node* form) -> bool;

private:
    static auto ConvertToLookup(const tt_string& description) -> tt_string;
    static auto ConvertToLookup(const tt_string_vector& parts) -> tt_string;

    NodeSharedPtr m_project_node { nullptr };

    // std::string is the entire property for the image
    std::map<std::string, ImageBundle> m_bundles;

    // This stores XPM images or any other non-embedded, non-art images
    //
    // std::string is parts[IndexImage].filename()
    std::map<std::string, wxImage, std::less<>> m_images;

    // std::string is parts[IndexImage].filename()
    std::map<std::string, std::unique_ptr<EmbeddedImage>, std::less<>> m_map_embedded;

    bool m_allow_ui { true };

    static constexpr std::array<std::pair<std::string_view, std::string_view>, 6>
        map_bundle_extensions = { { { "@1_25x", "@1_5x" },
                                    { "@1_5x", "@1_75x" },
                                    { "@1_75x", "@2x" },
                                    { "_1_25x", "_1_5x" },
                                    { "_1_5x", "_1_75x" },
                                    { "_1_75x", "_2x" } } };

    static constexpr std::array<const char*, 8> suffixes = { "_1_25x", "_1_5x", "_1_75x", "_2x",
                                                             "@1_25x", "@1_5x", "@1_75x", "@2x" };
};

extern ImageHandler& ProjectImages;  // NOLINT (cppcheck-suppress)
