/////////////////////////////////////////////////////////////////////////////
// Purpose:   Hold data for currently loaded project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <mutex>
#include <thread>

#include <wx/bitmap.h>

class Node;
class wxAnimation;

struct EmbededImage
{
    Node* form;  // the form node the image is declared in
    ttlib::cstr array_name;
    size_t array_size;
    std::unique_ptr<unsigned char[]> array_data;
    wxBitmapType type;
};

class ProjectSettings
{
public:
    ProjectSettings();
    ~ProjectSettings();

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

    ttlib::cstr& getProjectPath() { return m_projectPath; }
    ttString GetProjectPath() { return ttString() << m_projectPath.wx_str(); }

    // This takes the full bitmap property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    wxImage GetPropertyBitmap(const ttlib::cstr& description, bool want_scaled = true);

    // This takes the full animation property description and uses that to determine the image
    // to load. The image is cached for as long as the project is open.
    wxAnimation GetPropertyAnimation(const ttlib::cstr& description);

    bool AddEmbeddedImage(ttlib::cstr path, Node* form);
    EmbededImage* GetEmbeddedImage(ttlib::sview path);

    // This will launch a thread to start collecting all the embedded images in the project
    void ParseEmbeddedImages();

protected:
    bool CheckNode(Node* node);
    void CollectEmbeddedImages();
    void CollectNodeImages(Node* node, Node* form);

private:
    ttlib::cstr m_projectFile;
    ttlib::cstr m_projectPath;

    std::mutex m_mutex_embed_add;
    std::mutex m_mutex_embed_retrieve;

    std::map<std::string, wxImage> m_images;

    std::thread* m_collect_thread { nullptr };

    std::map<std::string, std::unique_ptr<EmbededImage>, std::less<>> m_map_embedded;

    bool m_is_terminating { false };
};
