/////////////////////////////////////////////////////////////////////////////
// Purpose:   ImageHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <mutex>

#include <wx/bmpbndl.h>  // includes wx/bitmap.h, wxBitmapBundle class interface

#include "node_classes.h"  // Forward defintions of Node classes

class wxAnimation;

struct ImageBundle
{
    wxBitmapBundle bundle;
    std::vector<tt_string> lst_filenames;
};

struct EmbeddedImage
{
    Node* form;  // the form node the image is declared in
    tt_string array_name;
    tt_string filename;  // only filled in if this differs from array_name
    size_t array_size;
    std::unique_ptr<unsigned char[]> array_data;
    wxBitmapType type;
    wxSize size;  // dimensions of the first image in the array
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

    // If there is an Image form containing this bundle, return it's name
    tt_string GetBundleFuncName(const tt_string& description);

    // If there is an Image form containing this bundle, return it's name
    tt_string GetBundleFuncName(const tt_string_vector& parts);

    // If there is an Image form containing this bundle, return it's name
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

    // Convert a filename to a valid variable name. This will handle filnames with leading
    // numbers, utf8 characters, and other characters that are not valid in a variable name.
    //
    // If max_length is exceeded, the name will be have ""_name_truncated" as a suffix
    static std::optional<tt_string> FileNameToVarName(tt_string_view filename, size_t max_length = 100);

protected:
    bool CheckNode(Node* node);

    void CollectNodeBundles(Node* node, Node* form);

    // Converts filename to a valid string name and sets EmbeddedImage::array_name
    void InitializeArrayName(EmbeddedImage* embed, tt_string_view filename);

    bool AddNewEmbeddedImage(tt_string path, Node* form, std::unique_lock<std::mutex>& add_lock);

    // Reads the image and stores it in m_map_embedded
    bool AddEmbeddedBundleImage(tt_string path, Node* form);

    bool AddNewEmbeddedBundle(const tt_string_vector& parts, tt_string path, Node* form);

    inline bool AddNewEmbeddedBundle(const tt_string& description, tt_string path, Node* form)
    {
        tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return AddNewEmbeddedBundle(parts, path, form);
    }

    // Reads the image and stores it in m_map_embedded
    bool AddSvgBundleImage(tt_string path, Node* form);

private:
    NodeSharedPtr m_project_node { nullptr };

    std::mutex m_mutex_embed_add;
    std::mutex m_mutex_embed_retrieve;

    std::map<std::string, wxImage> m_images;

    // std::string is the entire property for the image
    std::map<std::string, ImageBundle> m_bundles;

    // std::string is parts[IndexImage].filename()
    std::map<std::string, std::unique_ptr<EmbeddedImage>, std::less<>> m_map_embedded;

    bool m_allow_ui { true };
};

extern ImageHandler& ProjectImages;
