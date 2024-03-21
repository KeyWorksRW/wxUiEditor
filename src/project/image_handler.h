/////////////////////////////////////////////////////////////////////////////
// Purpose:   ImageHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <map>
#include <mutex>

#include <wx/bmpbndl.h>  // includes wx/bitmap.h, wxBitmapBundle class interface

#include "node_classes.h"  // Forward defintions of Node classes

class wxAnimation;

// This simply contains a list of the filenames that would be used to create a bundle.
struct ImageBundle
{
    std::vector<tt_string> lst_filenames;
};

struct ImageInfo
{
    tt_string filename;
    tt_string array_name;
    size_t array_size;
    std::unique_ptr<unsigned char[]> array_data;
    std::filesystem::file_time_type file_time;  // time the file was last modified
    wxBitmapType type;
};

struct EmbeddedImage
{
    Node* form;                   // the form node the image is declared in
    std::vector<ImageInfo> imgs;  // InitializeEmbedStructure() will always create at least one entry
    wxSize size;                  // dimensions of the first image in the array

    // Note that this will update any file within EmbeddedImage whose file_time has changed
    // since the file was first loaded.
    //
    // size parameter is only used for SVG files
    wxBitmapBundle get_bundle(wxSize size = { -1, -1 });
};

wxBitmapBundle LoadSVG(EmbeddedImage* embed, tt_string_view size_description);

class ImageHandler
{
private:
    ImageHandler() {}

public:
    ImageHandler(ImageHandler const&) = delete;

    void operator=(ImageHandler const&) = delete;

    static ImageHandler& getInstance()
    {
        static ImageHandler instance;
        return instance;
    }

    void Initialize(NodeSharedPtr project, bool allow_ui = true);

    // This will parse the entire project, and ensure that each embedded image is associated
    // with the form node of the form it first appears in.
    //
    // Returns true if an associated node changed
    bool UpdateEmbedNodes();

    // Call this is the image file has been modified. This will update the array_data and
    // array_size for the image from the updated image file.
    static void UpdateEmbeddedImage(EmbeddedImage* embed, size_t index = 0);

    wxImage GetImage(const tt_string& description);

    wxBitmapBundle GetBitmapBundle(const tt_string& description, Node* node);

    // This takes the full bitmap property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    //
    // If check_image is true, and !image.IsOK(), GetInternalImage() is returned
    wxImage GetPropertyBitmap(const tt_string_vector& parts, bool check_image = true);

    inline wxImage GetPropertyBitmap(const tt_string& description, bool check_image = true)
    {
        tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return GetPropertyBitmap(parts, check_image);
    }

    wxBitmapBundle GetPropertyBitmapBundle(tt_string_view description, Node* node);

    // ImageBundle contains the filenames of each image in the bundle, needed to generate the
    // code for the bundle.
    //
    // Returns nullptr if there is no ImageBundle
    const ImageBundle* GetPropertyImageBundle(const tt_string_vector& parts, Node* node = nullptr);
    const ImageBundle* GetPropertyImageBundle(tt_string_view description, Node* node = nullptr)
    {
        tt_string_vector parts(description, ';', tt::TRIM::both);
        return GetPropertyImageBundle(parts, node);
    }

    // If there is an Image form containing this bundle, return it's function name
    tt_string GetBundleFuncName(const tt_string& description);

    // If there is an Images List containing this bundle, return it's function name
    tt_string GetBundleFuncName(const tt_string_vector& parts);

    // If there is an Images List containing this image, return it's function name
    tt_string GetBundleFuncName(const EmbeddedImage* embed, wxSize svg_size = wxDefaultSize);

    ImageBundle* ProcessBundleProperty(const tt_string_vector& parts, Node* node);

    inline ImageBundle* ProcessBundleProperty(const tt_string& description, Node* node)
    {
        tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return ProcessBundleProperty(parts, node);
    }

    // This adds the bundle if new, or updates the embed->form if the node has changed
    void UpdateBundle(const tt_string_vector& parts, Node* node);

    // This takes the full animation property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    wxAnimation GetPropertyAnimation(const tt_string& description);

    bool AddEmbeddedImage(tt_string path, Node* form, bool is_animation = false);
    EmbeddedImage* GetEmbeddedImage(tt_string_view path);

    // This will collect bundles for the entire project -- it initializes
    // std::map<std::string, ImageBundle> m_bundles for every image.
    void CollectBundles();

    // Returns nullptr if the image is not found
    EmbeddedImage* FindEmbedded(std::string_view);

protected:
    bool CheckNode(Node* node);

    void CollectNodeBundles(Node* node, Node* form);

    // Converts filename to a valid string name and sets EmbeddedImage::array_name
    void InitializeEmbedStructure(EmbeddedImage* embed, tt_string_view path, Node* form);

    bool AddNewEmbeddedImage(tt_string path, Node* form);

    // Reads the image and stores it in m_map_embedded
    EmbeddedImage* AddEmbeddedBundleImage(tt_string path, Node* form, EmbeddedImage* embed = nullptr);

    bool AddNewEmbeddedBundle(const tt_string_vector& parts, tt_string path, Node* form);

    inline bool AddNewEmbeddedBundle(const tt_string& description, tt_string path, Node* form)
    {
        tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return AddNewEmbeddedBundle(parts, path, form);
    }

    // Reads the image, remove unused metadat, compresses it and stores it in m_map_embedded
    bool AddSvgBundleImage(tt_string path, Node* form);

    // Read the image, compresses it and stores it in m_map_embedded
    bool AddXpmBundleImage(tt_string path, Node* form);

private:
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
};

extern ImageHandler& ProjectImages;
