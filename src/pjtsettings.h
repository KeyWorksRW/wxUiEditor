/////////////////////////////////////////////////////////////////////////////
// Purpose:   Hold data for currently loaded project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <mutex>
#include <thread>

#include "image_bundle.h"  // This will #include wx/bmpbndl.h and wx/bitmap.h

class Node;
class wxAnimation;

struct EmbeddedImage
{
    Node* form;  // the form node the image is declared in
    ttlib::cstr array_name;
    size_t array_size;
    std::unique_ptr<unsigned char[]> array_data;
    wxBitmapType type;
};

wxBitmapBundle LoadSVG(EmbeddedImage* embed, ttlib::sview size_description);

class ProjectSettings
{
public:
    ProjectSettings();

    // This will parse the entire project, and ensure that each embedded image is associated
    // with the form node of the form it first appears in.
    //
    // Returns true if an associated node changed
    bool UpdateEmbedNodes();

    ttlib::cstr& getProjectFile() { return m_projectFile; }
    ttString GetProjectFile() { return ttString() << m_projectFile.wx_str(); }

    ttlib::cstr& SetProjectFile(const ttString& file);
    ttlib::cstr& setProjectFile(const ttlib::cstr& file);
    ttlib::cstr& SetProjectPath(const ttString& path, bool remove_filename = true);
    ttlib::cstr& setProjectPath(const ttlib::cstr& path, bool remove_filename = true);

    // Returns the directory the project file is in as a ttlib::cstr
    ttlib::cstr& getProjectPath() { return m_projectPath; }

    // Returns the directory the project file is in as a ttString
    ttString GetProjectPath() { return ttString() << m_projectPath.wx_str(); }

    // This takes the full bitmap property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    //
    // If check_image is true, and !image.IsOK(), GetInternalImage() is returned
    wxImage GetPropertyBitmap(const ttlib::multistr& parts, bool check_image = true);

    inline wxImage GetPropertyBitmap(const ttlib::cstr& description, bool check_image = true)
    {
        ttlib::multistr parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return GetPropertyBitmap(parts, check_image);
    }

    wxBitmapBundle GetPropertyBitmapBundle(const ttlib::cstr& description, Node* node);

    // ImageBundle contains the filenames of each image in the bundle, needed to generate the
    // code for the bundle.
    //
    // Returns nullptr if there is no ImageBundle
    const ImageBundle* GetPropertyImageBundle(const ttlib::multistr& parts, Node* node = nullptr);
    const ImageBundle* GetPropertyImageBundle(const ttlib::cstr& description, Node* node = nullptr)
    {
        ttlib::multistr parts(description, ';', tt::TRIM::both);
        return GetPropertyImageBundle(parts, node);
    }

    ImageBundle* ProcessBundleProperty(const ttlib::multistr& parts, Node* node);

    inline ImageBundle* ProcessBundleProperty(const ttlib::cstr& description, Node* node)
    {
        ttlib::multistr parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return ProcessBundleProperty(parts, node);
    }

    // This adds the bundle if new, or updates the embed->form if the node has changed
    void UpdateBundle(const ttlib::multistr& parts, Node* node);

    // This takes the full animation property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    wxAnimation GetPropertyAnimation(const ttlib::cstr& description);

    bool AddEmbeddedImage(ttlib::cstr path, Node* form, bool is_animation = false);
    EmbeddedImage* GetEmbeddedImage(ttlib::sview path);

    // Converts filename to a valid string name and sets EmbeddedImage::array_name
    void InitializeArrayName(EmbeddedImage* embed, ttlib::sview filename);

    // This will collect bundles for the entire project -- it initializes
    // std::map<std::string, ImageBundle> m_bundles for every image.
    void CollectBundles();

protected:
    bool CheckNode(Node* node);

    void CollectNodeBundles(Node* node, Node* form);

    bool AddNewEmbeddedBundle(const ttlib::multistr& parts, ttlib::cstr path, Node* form);

    inline bool AddNewEmbeddedBundle(const ttlib::cstr& description, ttlib::cstr path, Node* form)
    {
        ttlib::multistr parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
        return AddNewEmbeddedBundle(parts, path, form);
    }

    // Reads the image and stores it in m_map_embedded
    bool AddEmbeddedBundleImage(ttlib::cstr path, Node* form);

    // Reads the image and stores it in m_map_embedded
    bool AddSvgBundleImage(ttlib::cstr path, Node* form);

    bool AddNewEmbeddedImage(ttlib::cstr path, Node* form, std::unique_lock<std::mutex>& add_lock);

private:
    ttlib::cstr m_projectFile;
    ttlib::cstr m_projectPath;

    std::mutex m_mutex_embed_add;
    std::mutex m_mutex_embed_retrieve;

    std::map<std::string, wxImage> m_images;

    // std::string is the entire property for the image
    std::map<std::string, ImageBundle> m_bundles;

    // std::string is parts[IndexImage].filename()
    std::map<std::string, std::unique_ptr<EmbeddedImage>, std::less<>> m_map_embedded;
};
