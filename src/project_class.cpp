/////////////////////////////////////////////////////////////////////////////
// Purpose:   Project class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <fstream>
#include <thread>

#include <wx/animate.h>   // wxAnimation and wxAnimationCtrl
#include <wx/artprov.h>   // wxArtProvider class
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/filesys.h>   // class for opening files - virtual file system
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes

#include "project_class.h"

#include "bitmaps.h"        // Map of bitmaps accessed by name
#include "mainapp.h"        // App -- App class
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "project_class.h"  // Project class
#include "utils.h"          // Utility functions that work with properties

// Convert a data array into a wxAnimation
inline wxAnimation GetAnimFromHdr(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream strm(data, size_data);
    wxAnimation animation;
    animation.Load(strm);
    return animation;
};

Project* GetProject()
{
    ASSERT_MSG(wxGetApp().GetProjectPtr(), "Can't request a project pointer before it is created.");
    return wxGetApp().GetProject();
}

namespace wxue_img
{
    extern const unsigned char pulsing_unknown_gif[377];
}

ttlib::cstr& Project::SetProjectFile(const ttString& file)
{
    m_projectFile.clear();
    m_projectFile << file.wx_str();
    return m_projectFile;
}

ttlib::cstr& Project::setProjectFile(const ttlib::cstr& file)
{
    m_projectFile = file;
    return m_projectFile;
}

ttlib::cstr& Project::SetProjectPath(const ttString& file, bool remove_filename)
{
    m_projectPath.clear();
    m_projectPath << file.wx_str();
    if (remove_filename)
        m_projectPath.remove_filename();
#if defined(_WIN32)
    m_projectPath.backslashestoforward();
#endif  // _WIN32
    return m_projectPath;
}

ttlib::cstr& Project::setProjectPath(const ttlib::cstr& file, bool remove_filename)
{
    m_projectPath = file;
    if (remove_filename)
        m_projectPath.remove_filename();
    return m_projectPath;
}

wxImage Project::GetPropertyBitmap(const ttlib::multistr& parts, bool check_image)
{
    if (parts[IndexImage].empty())
    {
        return GetInternalImage("unknown");
    }

    wxImage image;

    ttlib::cstr path = parts[IndexImage];

    auto result = m_images.find(path);
    if (result != m_images.end())
    {
        image = result->second;
    }
    else if (parts[IndexType].contains("Art"))
    {
        if (parts[IndexArtID].contains("|"))
        {
            ttlib::multistr id_client(parts[IndexArtID], '|');
            image = (wxArtProvider::GetBitmapBundle(id_client[0], wxART_MAKE_CLIENT_ID_FROM_STR(id_client[1]))
                         .GetBitmapFor(wxGetFrame().GetWindow()))
                        .ConvertToImage();
        }
        else
        {
            image = (wxArtProvider::GetBitmapBundle(parts[IndexArtID].wx_str(), wxART_MAKE_CLIENT_ID_FROM_STR("wxART_OTHER"))
                         .GetBitmapFor(wxGetFrame().GetWindow()))
                        .ConvertToImage();
        }
    }
    else if (parts[IndexType].contains("Embed"))
    {
        if (!path.file_exists())
        {
            path = prop_as_string(prop_art_directory);
            path.append_filename(parts[IndexImage]);
        }
        auto embed = GetEmbeddedImage(path);
        if (!embed)
        {
            bool added = AddEmbeddedImage(path, wxGetFrame().GetSelectedForm());
            if (added)
            {
                embed = GetEmbeddedImage(path);
            }
        }

        if (embed)
        {
            wxMemoryInputStream stream(embed->array_data.get(), embed->array_size);
            image.LoadFile(stream);
        }
    }
    else
    {
        if (!path.file_exists())
        {
            path = prop_as_string(prop_art_directory);
            path.append_filename(parts[IndexImage]);

            if (result = m_images.find(path); result != m_images.end())
            {
                image = result->second;
            }
        }

        if (!image.IsOk())
        {
            if (path.has_extension(".h_img") || path.has_extension(".h"))
            {
                image = GetHeaderImage(path);
            }
            else
            {
                // Note that this will load an XPM file
                image.LoadFile(path);
            }
        }
    }

    if (!image.IsOk())
    {
        return (check_image ? GetInternalImage("unknown") : image);
    }

    // If it's not embedded, then cache it so that we don't read it from disk again
    if (!parts[IndexType].contains("Embed") && result == m_images.end())
        m_images[path] = image;

    return image;
}

void Project::UpdateBundle(const ttlib::multistr& parts, Node* node)
{
    if (parts.size() < 2 || node->IsFormParent())
        return;

    ttlib::cstr lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    auto result = m_bundles.find(lookup_str);
    if (result == m_bundles.end())
    {
        ProcessBundleProperty(parts, node);
        result = m_bundles.find(lookup_str);
    }

    if (result != m_bundles.end() && result->second.lst_filenames.size())
    {
        auto form = node->get_form();
        for (auto& iter: result->second.lst_filenames)
        {
            if (auto embed = GetEmbeddedImage(iter); embed)
            {
                if (embed->form != form)
                {
                    // This will happen when a bundle bitmap is added to the Images generator. The initial bitmap will be
                    // correctly changed to use the new form, but we also need to process  all the sub images as well

                    if (form->isGen(gen_Images))
                    {
                        embed->form = form;
                    }
                }
            }
        }
    }
}

wxBitmapBundle Project::GetPropertyBitmapBundle(const ttlib::cstr& description, Node* node)
{
    ttlib::multistr parts(description, ';', tt::TRIM::both);
    if (parts.size() < 2)
    {
        return GetInternalImage("unknown");
    }

    ttlib::cstr lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    if (auto result = m_bundles.find(lookup_str); result != m_bundles.end())
    {
        return result->second.bundle;
    }

    if (auto result = ProcessBundleProperty(parts, node); result)
    {
        return result->bundle;
    }

    return GetInternalImage("unknown");
}

const ImageBundle* Project::GetPropertyImageBundle(const ttlib::multistr& parts, Node* node)
{
    if (parts.size() < 2)
    {
        return nullptr;
    }

    ttlib::cstr lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    if (auto result = m_bundles.find(lookup_str); result != m_bundles.end())
    {
        return &result->second;
    }
    else if (node)
    {
        return ProcessBundleProperty(parts, node);
    }
    else
    {
        return nullptr;
    }
}

wxAnimation Project::GetPropertyAnimation(const ttlib::cstr& description)
{
    ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    wxAnimation image;

    if (parts.size() <= IndexImage || parts[IndexImage].empty())
    {
        return GetAnimFromHdr(wxue_img::pulsing_unknown_gif, sizeof(wxue_img::pulsing_unknown_gif));
    }

    ttlib::cstr path = parts[IndexImage];
    if (!path.file_exists())
    {
        path = prop_as_string(prop_art_directory);
        path.append_filename(parts[IndexImage]);
    }

    if (parts[IndexType].contains("Embed"))
    {
        auto embed = GetEmbeddedImage(path);
        if (!embed)
        {
            bool added = AddEmbeddedImage(path, wxGetFrame().GetSelectedForm());
            if (added)
            {
                embed = GetEmbeddedImage(path);
            }
        }

        if (embed)
        {
            wxMemoryInputStream stream(embed->array_data.get(), embed->array_size);
            image.Load(stream);
        }
    }
    else
    {
        // This handles Header files
        GetAnimationImage(image, path);
    }

    if (!image.IsOk())
    {
        return GetAnimFromHdr(wxue_img::pulsing_unknown_gif, sizeof(wxue_img::pulsing_unknown_gif));
    }

    return image;
}

bool Project::AddEmbeddedImage(ttlib::cstr path, Node* form, bool is_animation)
{
    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

    if (!path.file_exists())
    {
        if (GetProject()->HasValue(prop_art_directory))
        {
            ttlib::cstr art_path = GetProject()->prop_as_string(prop_art_directory);
            art_path.append_filename(path);
            if (!art_path.file_exists())
                return false;
            path = std::move(art_path);
        }
        else
        {
            return false;
        }
    }

    if (m_map_embedded.find(path.filename().as_str()) != m_map_embedded.end())
        return false;

    auto final_result = AddNewEmbeddedImage(path, form, add_lock);
    if (is_animation || !final_result)
        return final_result;

    // Note that path may now contain the prop_art_directory prefix

    add_lock.lock();

    if (auto pos = path.find_last_of('.'); ttlib::is_found(pos))
    {
        if (path.contains("_16x16."))
        {
            path.Replace("_16x16.", "_24x24.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
            path.Replace("_24x24.", "_32x32.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
        }
        else if (path.contains("_24x24."))
        {
            path.Replace("_24x24.", "_36x36.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
            path.Replace("_36x36.", "_48x48.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
        }
        else
        {
            path.insert(pos, "_1_5x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
            path.Replace("_1_5x", "_2x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form, add_lock);
                add_lock.lock();
            }
        }
    }

    return final_result;
}

bool Project::AddNewEmbeddedImage(ttlib::cstr path, Node* form, std::unique_lock<std::mutex>& add_lock)
{
    wxFFileInputStream stream(path.wx_str());
    if (!stream.IsOk())
    {
        add_lock.unlock();
        return false;
    }

    wxImageHandler* handler;
    auto& list = wxImage::GetHandlers();
    for (auto node = list.GetFirst(); node; node = node->GetNext())
    {
        handler = (wxImageHandler*) node->GetData();
        if (handler->CanRead(stream))
        {
            wxImage image;
            if (handler->LoadFile(&image, stream))
            {
                m_map_embedded[path.filename().as_str()] = std::make_unique<EmbeddedImage>();
                auto embed = m_map_embedded[path.filename().as_str()].get();
                InitializeArrayName(embed, path.filename());
                embed->form = form;

                // At this point, other threads can lookup and add an embedded image, they just can't access the data of this
                // image until we're done. I.e., GetEmbeddedImage() won't return until retrieve_lock is released.

                std::unique_lock<std::mutex> retrieve_lock(m_mutex_embed_retrieve);
                add_lock.unlock();

                // If possible, convert the file to a PNG -- even if the original file is a PNG, since we might end up with
                // better compression.

                if (isConvertibleMime(handler->GetMimeType()))
                {
                    embed->type = wxBITMAP_TYPE_PNG;

                    wxMemoryOutputStream save_stream;

                    // Maximize compression
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
                    image.SaveFile(save_stream, "image/png");

                    auto read_stream = save_stream.GetOutputStreamBuffer();
                    stream.SeekI(0);
                    if (read_stream->GetBufferSize() <= (to_size_t) stream.GetLength())
                    {
                        embed->array_size = read_stream->GetBufferSize();
                        embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                        memcpy(embed->array_data.get(), read_stream->GetBufferStart(), embed->array_size);
                    }
                    else
                    {
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
                        size_t org_size = (to_size_t) stream.GetLength();
                        auto png_size = read_stream->GetBufferSize();
                        ttlib::cstr size_comparison;
                        size_comparison.Format("Original: %ku, new: %ku", org_size, png_size);
#endif  // _DEBUG

                        embed->type = handler->GetType();
                        embed->array_size = stream.GetSize();
                        embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                        stream.Read(embed->array_data.get(), embed->array_size);
                    }
                }
                else
                {
                    embed->type = handler->GetType();

                    stream.SeekI(0);
                    embed->array_size = stream.GetSize();
                    embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                    stream.Read(embed->array_data.get(), embed->array_size);
                }

                return true;
            }
        }
    }

    add_lock.unlock();
    return false;
}

void Project::InitializeArrayName(EmbeddedImage* embed, ttlib::sview filename)
{
    embed->array_name = filename;
    for (size_t idx = 0; idx < embed->array_name.size(); ++idx)
    {
        if (ttlib::is_alnum(embed->array_name[idx]) || embed->array_name[idx] == '_')
        {
            continue;
        }
        embed->array_name[idx] = '_';
    }
}

EmbeddedImage* Project::GetEmbeddedImage(ttlib::sview path)
{
    // REVIEW: [KeyWorks - 05-03-2022] Do we still need this lock?
    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

    if (auto result = m_map_embedded.find(path.filename()); result != m_map_embedded.end())
    {
        std::unique_lock<std::mutex> retrieve_lock(m_mutex_embed_retrieve);
        return result->second.get();
    }
    else
    {
        return nullptr;
    }
}

bool Project::UpdateEmbedNodes()
{
    bool is_changed = false;
    std::vector<Node*> forms;
    GetProject()->CollectForms(forms);

    for (const auto& form: forms)
    {
        if (CheckNode(form))
            is_changed = true;
    }
    return is_changed;
}

// REVIEW: [KeyWorks - 04-07-2022] We should eliminate this call if possible -- Project::CollectBundles() processed
// all nodes initially, and the only reason this would be needed is if adding or changing a bitmap property did not get set
// up correctly (highly unlikely).

bool Project::CheckNode(Node* node)
{
    if (node->IsFormParent())
        return false;

    bool is_changed = false;

    Node* node_form = node->get_form();

    auto node_position = GetProject()->GetChildPosition(node_form);

    for (auto& iter: node->get_props_vector())
    {
        if ((iter.type() == type_image || iter.type() == type_animation) && iter.HasValue())
        {
            ttlib::multiview parts(iter.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts[IndexType] != "Embed")
                continue;

            auto result = m_map_embedded.find(parts[IndexImage].filename());
            if (result == m_map_embedded.end())
            {
                // If the image file could not be loaded, we end up here. This can happen by trying to add a SVG image to
                // a Embed image type
                // FAIL_MSG("We get here if a bitmap did not get added to m_map_embedded -- that shouldn't happen")
                // AddEmbeddedImage(parts[IndexImage], node_form);
                continue;
            }

            auto embed = result->second.get();
            ASSERT(embed)

            if (node_form->isGen(gen_Images))
            {
                if (embed->form != node_form)
                {
                    FAIL_MSG("Can we fix this without walking through every form?")
                    embed->form = node_form;
                    is_changed = true;
                }
            }
            else
            {
                auto child_pos = GetProject()->GetChildPosition(embed->form);
                if (child_pos > node_position)
                {
                    FAIL_MSG("If this is valid, we need to document why.")
                    embed->form = node_form;
                    is_changed = true;
                }
            }
        }
    }

    for (const auto& child: node->GetChildNodePtrs())
    {
        if (CheckNode(child.get()))
            is_changed = true;
    }

    return is_changed;
}

ttlib::cstr Project::getArtDirectory()
{
    if (HasValue(prop_art_directory))
        return value(prop_art_directory);
    else
        return m_projectPath;
}

ttString Project::GetArtDirectory()
{
    if (HasValue(prop_art_directory))
        return prop_as_wxString(prop_art_directory);
    else
        return ttString() << m_projectPath.wx_str();
}

ttString Project::GetBaseDirectory(int language)
{
    if (language == GEN_LANG_CPLUSPLUS && HasValue(prop_base_directory))
        return as_wxString(prop_base_directory);
    else if (language == GEN_LANG_PYTHON && HasValue(prop_python_output_folder))
        return as_wxString(prop_python_output_folder);
    else if (language == GEN_LANG_XRC && HasValue(prop_xrc_directory))
        return as_wxString(prop_xrc_directory);
    else
        return GetProjectPath();
}

ttString Project::GetDerivedDirectory()
{
    if (HasValue(prop_derived_directory))
        return as_wxString(prop_derived_directory);
    else
        return GetProjectPath();
}

Node* Project::GetFirstFormChild(Node* node)
{
    if (!node)
        node = this;
    for (const auto& child: node->GetChildNodePtrs())
    {
        if (child->IsForm())
        {
            return child.get();
        }
        else if (child->isGen(gen_folder) || child->isGen(gen_sub_folder))
        {
            return GetFirstFormChild(child.get());
        }
    }

    return nullptr;
}

inline static void CreateUniqueName(std::set<std::string_view>& set_names, PropName prop, Node* new_node)
{
    if (new_node->HasValue(prop))
    {
        if (set_names.contains(new_node->value(prop)))
        {
            ttlib::cstr new_name = new_node->value(prop);
            if (!new_name.contains("_copy"))
                new_name << "_copy";
            if (set_names.contains(new_name))
            {
                ttlib::cstr copy_name = new_name;
                while (ttlib::is_digit(copy_name.back()))
                {
                    // remove any trailing digits
                    copy_name.erase(copy_name.size() - 1, 1);
                }
                if (copy_name.back() == '_')
                {
                    copy_name.erase(copy_name.size() - 1, 1);
                }

                for (int i = 2;; ++i)
                {
                    new_name.clear();
                    new_name << copy_name << '_' << i;
                    if (!set_names.contains(new_name))
                        break;
                }
            }
            new_node->set_value(prop, new_name);
        }
    }
}

void Project::FixupDuplicatedNode(Node* new_node)
{
    std::set<std::string_view> base_classnames;
    std::set<std::string_view> derived_classnames;
    std::set<std::string_view> base_filename;
    std::set<std::string_view> derived_filename;
    std::set<std::string_view> xrc_filename;

    // Collect all of the class and filenames in use by each form so we can make sure the new
    // form doesn't use any of them.
    std::vector<Node*> forms;
    CollectForms(forms);
    for (auto& iter: forms)
    {
        if (iter->HasValue(prop_class_name))
            base_classnames.insert(iter->value(prop_class_name));
        if (iter->HasValue(prop_derived_class_name))
            derived_classnames.insert(iter->value(prop_derived_class_name));
        if (iter->HasValue(prop_base_file))
            base_filename.insert(iter->value(prop_base_file));
        if (iter->HasValue(prop_derived_file))
            derived_filename.insert(iter->value(prop_derived_file));
        if (iter->HasValue(prop_xrc_file))
            xrc_filename.insert(iter->value(prop_xrc_file));
    }

    CreateUniqueName(base_classnames, prop_class_name, new_node);
    CreateUniqueName(derived_classnames, prop_derived_class_name, new_node);
    CreateUniqueName(base_filename, prop_base_file, new_node);
    CreateUniqueName(derived_filename, prop_derived_file, new_node);
    CreateUniqueName(xrc_filename, prop_xrc_file, new_node);
}

wxBitmapBundle Project::GetBitmapBundle(const ttlib::cstr& description, Node* node)
{
    if (description.starts_with("Embed;") || description.starts_with("XPM;") || description.starts_with("Header;") ||
        description.starts_with("Art;") || description.starts_with("SVG;"))
    {
        return GetPropertyBitmapBundle(description, node);
    }
    else
        return GetInternalImage("unknown");
}

wxImage Project::GetImage(const ttlib::cstr& description)
{
    if (description.starts_with("Embed;") || description.starts_with("XPM;") || description.starts_with("Header;") ||
        description.starts_with("Art;"))
    {
        return GetPropertyBitmap(description);
    }
    else
        return GetInternalImage("unknown");
}

ttlib::cstr Project::GetBundleFuncName(const ttlib::cstr& description)
{
    ttlib::cstr name;

    for (const auto& form: GetChildNodePtrs())
    {
        if (form->isGen(gen_Images))
        {
            ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts.size() < 2)
            {
                // caller's description does not include a filename
                return name;
            }

            for (const auto& child: form->GetChildNodePtrs())
            {
                ttlib::multiview form_image_parts(child->prop_as_string(prop_bitmap), BMP_PROP_SEPARATOR, tt::TRIM::both);
                if (form_image_parts.size() < 2)
                {
                    continue;
                }

                if (parts[0] == form_image_parts[0] && parts[1].filename() == form_image_parts[1].filename())
                {
                    if (auto bundle = GetPropertyImageBundle(description); bundle && bundle->lst_filenames.size())
                    {
                        auto embed = GetEmbeddedImage(bundle->lst_filenames[0]);
                        if (embed->type == wxBITMAP_TYPE_INVALID)
                        {
                            name << "wxue_img::bundle_" << embed->array_name << "(";

                            wxSize svg_size { -1, -1 };
                            if (parts[IndexSize].size())
                            {
                                GetSizeInfo(svg_size, parts[IndexSize]);
                            }
                            name << svg_size.x << ", " << svg_size.y << ")";
                        }
                        else
                        {
                            name << "wxue_img::bundle_" << embed->array_name << "()";
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    return name;
}

namespace wxue_img
{
    inline const unsigned char pulsing_unknown_gif[377] {
        71,  73,  70,  56,  57,  97,  15,  0,   20,  0,   196, 0,   0,   255, 255, 255, 253, 124, 134, 253, 118, 129, 253,
        115, 126, 252, 108, 120, 252, 105, 117, 255, 102, 102, 251, 100, 113, 250, 87,  101, 250, 84,  98,  249, 77,  91,
        249, 71,  86,  248, 67,  82,  248, 62,  77,  248, 58,  74,  247, 48,  65,  246, 41,  59,  246, 36,  54,  245, 33,
        50,  238, 29,  47,  230, 28,  45,  222, 27,  43,  214, 26,  42,  206, 25,  40,  198, 24,  39,  189, 23,  37,  172,
        21,  34,  159, 19,  31,  148, 18,  29,  140, 17,  27,  132, 16,  26,  125, 15,  24,  33,  255, 11,  78,  69,  84,
        83,  67,  65,  80,  69,  50,  46,  48,  3,   1,   0,   0,   0,   33,  249, 4,   9,   40,  0,   0,   0,   44,  0,
        0,   0,   0,   15,  0,   20,  0,   0,   5,   80,  32,  32,  142, 100, 105, 158, 104, 74,  6,   3,   65,  28,  10,
        250, 54,  208, 211, 56,  209, 121, 60,  90,  167, 85,  145, 201, 165, 164, 120, 100, 62,  34,  137, 228, 178, 41,
        93,  56,  31,  36,  64,  65,  129, 170, 8,   140, 141, 84,  150, 192, 108, 79,  9,   198, 229, 123, 154, 100, 58,
        100, 149, 186, 80,  88,  80,  212, 140, 71,  69,  163, 206, 120, 162, 234, 188, 126, 207, 7,   132, 0,   0,   33,
        249, 4,   9,   40,  0,   0,   0,   44,  0,   0,   0,   0,   15,  0,   20,  0,   0,   5,   126, 32,  32,  2,   193,
        64,  16,  7,   50,  174, 129, 112, 40,  76,  204, 44,  204, 138, 46,  15,  117, 93,  149, 244, 60,  145, 209, 129,
        33,  185, 112, 62,  31,  77,  47,  50,  185, 0,   16,  138, 71,  101, 243, 25,  73,  34,  146, 138, 6,   48,  169,
        96,  52,  213, 81,  116, 130, 233, 136, 142, 200, 209, 0,   1,   49,  134, 87,  34,  151, 195, 210, 121, 195, 229,
        116, 251, 10,  95,  135, 11,  15,  13,  20,  84,  126, 35,  14,  77,  96,  132, 35,  90,  28,  30,  122, 137, 143,
        35,  5,   5,   8,   11,  20,  144, 8,   9,   12,  16,  25,  144, 12,  15,  18,  22,  27,  144, 16,  94,  26,  30,
        144, 25,  141, 105, 144, 144, 33,  0,   59
    };

}

void Project::CollectForms(std::vector<Node*>& forms, Node* node_start)
{
    if (!node_start)
    {
        node_start = this;
    }

    for (const auto& child: node_start->GetChildNodePtrs())
    {
        if (child->IsForm())
        {
            forms.push_back(child.get());
        }
        else
        {
            if (child->isGen(gen_folder) || child->isGen(gen_sub_folder))
            {
                CollectForms(forms, child.get());
            }
        }
    }
}
