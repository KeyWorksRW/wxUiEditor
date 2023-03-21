/////////////////////////////////////////////////////////////////////////////
// Purpose:   ImageHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>

#include <wx/animate.h>   // wxAnimation and wxAnimationCtrl
#include <wx/artprov.h>   // wxArtProvider class
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes
#include <wx/zstream.h>   // zlib stream classes

#include "image_handler.h"

#include "bitmaps.h"          // Contains various images handling functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler -- Project class
#include "utils.h"            // Miscellaneous utilities

#include "pugixml.hpp"  // xml parser

ImageHandler& ProjectImages = ImageHandler::getInstance();

// Note that we do *not* support @1_5x or @2x as suffixes. Since these suffixes will become part of the string name when
// converted to an embedded character array, the compiler will not accept the '@' character. We could of course change it,
// but then we don't know if it's unique if there is an actual filename that used a leading suffix '_' character instead of a
// leading '@'.

inline const std::array<const char*, 4> suffixes {
    "_1_5x",
    "_2x",
    "@1_5x",
    "@2x",
};

namespace wxue_img
{
    extern const unsigned char pulsing_unknown_gif[377];
}

// Convert a data array into a wxAnimation
inline wxAnimation GetAnimFromHdr(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream strm(data, size_data);
    wxAnimation animation;
    animation.Load(strm);
    return animation;
};

inline tt_string ConvertToLookup(const tt_string& description)
{
    tt_view_vector parts(description, ';', tt::TRIM::both);
    ASSERT(parts.size() > 1)

    tt_string lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();
    return lookup_str;
}

void ImageHandler::Initialize(NodeSharedPtr project, bool allow_ui)
{
    m_project_node = project;
    m_allow_ui = allow_ui;

    m_bundles.clear();
    m_images.clear();
    m_map_embedded.clear();
}

bool ImageHandler::UpdateEmbedNodes()
{
    bool is_changed = false;
    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        if (CheckNode(form))
            is_changed = true;
    }
    return is_changed;
}

// REVIEW: [KeyWorks - 04-07-2022] We should eliminate this call if possible -- ImageHandler::CollectBundles() processed
// all nodes initially, and the only reason this would be needed is if adding or changing a bitmap property did not get set
// up correctly (highly unlikely).

bool ImageHandler::CheckNode(Node* node)
{
    if (node->IsFormParent())
        return false;

    bool is_changed = false;

    Node* node_form = node->get_form();

    auto node_position = m_project_node->GetChildPosition(node_form);

    for (auto& iter: node->get_props_vector())
    {
        if ((iter.type() == type_image || iter.type() == type_animation) && iter.HasValue())
        {
            tt_view_vector parts(iter.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts[IndexType] != "Embed" || parts.size() <= IndexImage)
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
                auto child_pos = m_project_node->GetChildPosition(embed->form);
                if (child_pos > node_position)
                {
                    // The original embed->form is setup by parsing all of the nodes. However,
                    // code generation may not actually have a file set for a form, in which
                    // case the first use of the image for generated code can be in a different
                    // image. You'll see this in the python_tests project where some forms are
                    // only generated for C++, but not Python.

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

wxImage ImageHandler::GetImage(const tt_string& description)
{
    if (description.starts_with("Embed;") || description.starts_with("XPM;") || description.starts_with("Header;") ||
        description.starts_with("Art;"))
    {
        return GetPropertyBitmap(description);
    }
    else
        return GetInternalImage("unknown");
}

wxBitmapBundle ImageHandler::GetBitmapBundle(const tt_string& description, Node* node)
{
    if (description.starts_with("Embed;") || description.starts_with("XPM;") || description.starts_with("Header;") ||
        description.starts_with("Art;") || description.starts_with("SVG;"))
    {
        return GetPropertyBitmapBundle(description, node);
    }
    else
        return GetInternalImage("unknown");
}

wxImage ImageHandler::GetPropertyBitmap(const tt_string_vector& parts, bool check_image)
{
    if (parts.size() <= IndexImage || parts[IndexImage].empty())
    {
        return GetInternalImage("unknown");
    }

    wxImage image;

    tt_string path = parts[IndexImage];

    auto result = m_images.find(path);
    if (result != m_images.end())
    {
        image = result->second;
    }
    else if (parts[IndexType].contains("Art"))
    {
        if (parts[IndexArtID].contains("|"))
        {
            tt_string_vector id_client(parts[IndexArtID], '|');
            ASSERT_MSG(m_allow_ui, "We should never get here if m_allow_ui is false");
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
            path = m_project_node->as_string(prop_art_directory);
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
            path = m_project_node->as_string(prop_art_directory);
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

EmbeddedImage* ImageHandler::GetEmbeddedImage(tt_string_view path)
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

bool ImageHandler::AddEmbeddedImage(tt_string path, Node* form, bool is_animation)
{
    std::unique_lock<std::mutex> add_lock(m_mutex_embed_add);

    if (!path.file_exists())
    {
        if (m_project_node->HasValue(prop_art_directory))
        {
            tt_string art_path = m_project_node->prop_as_string(prop_art_directory);
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

    if (auto pos = path.find_last_of('.'); tt::is_found(pos))
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

bool ImageHandler::AddNewEmbeddedImage(tt_string path, Node* form, std::unique_lock<std::mutex>& add_lock)
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
                        tt_string size_comparison;
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

void ImageHandler::InitializeArrayName(EmbeddedImage* embed, tt_string_view filename)
{
    auto result = FileNameToVarName(filename);
    embed->array_name = result.value_or("image_");
    if (embed->array_name != filename)
    {
        embed->filename = filename;
    }

    for (size_t idx = 0; idx < embed->array_name.size(); ++idx)
    {
        if (tt::is_alnum(embed->array_name[idx]) || embed->array_name[idx] == '_')
        {
            continue;
        }
        embed->array_name[idx] = '_';
    }
}

static bool CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream, size_t size)
{
    size_t buf_size;
    if (size == tt::npos || size > (64 * 1024))
        buf_size = (64 * 1024);
    else
        buf_size = size;

    auto read_buf = std::make_unique<unsigned char[]>(buf_size);
    auto read_size = buf_size;

    size_t copied_data = 0;
    for (;;)
    {
        if (size != tt::npos && copied_data + read_size > size)
            read_size = size - copied_data;
        inputStream->Read(read_buf.get(), read_size);

        auto actually_read = inputStream->LastRead();
        outputStream->Write(read_buf.get(), actually_read);
        if (outputStream->LastWrite() != actually_read)
        {
            return false;
        }

        if (size == tt::npos)
        {
            if (inputStream->Eof())
                break;
        }
        else
        {
            copied_data += actually_read;
            if (copied_data >= size)
                break;
        }
    }

    return true;
}

void ImageHandler::CollectBundles()
{
    if (m_allow_ui)
        wxBusyCursor wait;

    tt_cwd save_cwd(true);
    Project.ProjectPath().ChangeDir();

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        CollectNodeBundles(form, form);

        if (form->HasProp(prop_icon) && form->HasValue(prop_icon))
        {
            if (!m_bundles.contains(ConvertToLookup(form->prop_as_string(prop_icon))))
            {
                ProcessBundleProperty(form->prop_as_string(prop_icon), form);
            }
        }
    }
}

void ImageHandler::CollectNodeBundles(Node* node, Node* form)
{
    for (auto& iter: node->get_props_vector())
    {
        if (!iter.HasValue())
            continue;

        if (iter.type() == type_image)
        {
            if (!m_bundles.contains(ConvertToLookup(iter.as_string())))
            {
                ProcessBundleProperty(iter.as_string(), form);
            }
        }
        else if (iter.type() == type_animation)
        {
            auto& value = iter.as_string();
            if (value.starts_with("Embed"))
            {
                tt_view_vector parts(value, BMP_PROP_SEPARATOR, tt::TRIM::both);
                if (parts[IndexImage].size())
                {
                    if (!m_map_embedded.contains(parts[IndexImage].filename()))
                    {
                        AddEmbeddedImage(parts[IndexImage], form);
                    }
                }
            }
        }
    }

    for (const auto& child: node->GetChildNodePtrs())
    {
        CollectNodeBundles(child.get(), form);
    }
}

bool ImageHandler::AddNewEmbeddedBundle(const tt_string_vector& parts, tt_string path, Node* form)
{
    ASSERT(parts.size() > 1)

    tt_string lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    ImageBundle img_bundle;

    if (!path.file_exists())
    {
        if (m_project_node->HasValue(prop_art_directory))
        {
            tt_string art_path = m_project_node->prop_as_string(prop_art_directory);
            art_path.append_filename(path);
            if (!art_path.file_exists())
            {
                m_bundles[lookup_str] = img_bundle;
                return true;
            }
            path = std::move(art_path);
        }
        else
        {
            m_bundles[lookup_str] = img_bundle;
            return true;
        }
    }

    if (parts[IndexType].starts_with("SVG"))
    {
        if (AddSvgBundleImage(path, form))
        {
            img_bundle.lst_filenames.emplace_back(path);
            if (auto embed = GetEmbeddedImage(path); embed)
            {
                img_bundle.bundle = LoadSVG(embed, parts[IndexSize]);
                m_bundles[lookup_str] = std::move(img_bundle);
                return true;
            }
        }
    }

    if (!AddEmbeddedBundleImage(path, form))
    {
        return false;
    }

    img_bundle.lst_filenames.emplace_back(path);

    /*

        Look for suffix combinations -- it's fine if one of them doesn't exist

            _16x16, _24x24, _32x32
            _24x24, _36x36, _48x48
            any, _1_5x, _2x

    */

    if (auto pos = path.find_last_of('.'); tt::is_found(pos))
    {
        if (path.contains("_16x16."))
        {
            path.Replace("_16x16.", "_24x24.");
            if (path.file_exists())
            {
                if (auto added = AddEmbeddedBundleImage(path, form); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
            path.Replace("_24x24.", "_32x32.");
            if (path.file_exists())
            {
                if (auto added = AddEmbeddedBundleImage(path, form); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
        }
        else if (path.contains("_24x24."))
        {
            path.Replace("_24x24.", "_36x36.");
            if (path.file_exists())
            {
                if (auto added = AddEmbeddedBundleImage(path, form); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
            path.Replace("_36x36.", "_48x48.");
            if (path.file_exists())
            {
                if (auto added = AddEmbeddedBundleImage(path, form); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
        }
        else
        {
            tt_string additional_path;
            for (auto& iter: suffixes)
            {
                additional_path = path;
                additional_path.insert(pos, iter);
                if (additional_path.file_exists())
                {
                    if (auto added = AddEmbeddedBundleImage(additional_path, form); added)
                    {
                        img_bundle.lst_filenames.emplace_back(additional_path);
                    }
                }
            }
        }
    }

    if (img_bundle.lst_filenames.size() == 1)
    {
        if (auto embed = GetEmbeddedImage(img_bundle.lst_filenames[0]); embed)
        {
            wxMemoryInputStream stream(embed->array_data.get(), embed->array_size);
            wxImage image;
            image.LoadFile(stream);
            img_bundle.bundle = wxBitmapBundle::FromBitmap(image);
        }
    }
    else
    {
        wxVector<wxBitmap> bitmaps;
        for (auto& iter: img_bundle.lst_filenames)
        {
            if (auto embed = GetEmbeddedImage(iter); embed)
            {
                wxMemoryInputStream stream(embed->array_data.get(), embed->array_size);
                wxImage image;
                image.LoadFile(stream);
                ASSERT(image.IsOk())
                bitmaps.push_back(image);
            }
        }
        img_bundle.bundle = wxBitmapBundle::FromBitmaps(bitmaps);
    }

    m_bundles[lookup_str] = std::move(img_bundle);
    return true;
}

bool ImageHandler::AddEmbeddedBundleImage(tt_string path, Node* form)
{
    wxFFileInputStream stream(path.wx_str());
    if (!stream.IsOk())
    {
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
                        tt_string size_comparison;
                        size_comparison.Format("Original: %ku, new: %ku", org_size, png_size);
#endif

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
    return false;
}

ImageBundle* ImageHandler::ProcessBundleProperty(const tt_string_vector& parts, Node* node)
{
    ASSERT(parts.size() > 1)

    tt_string lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();

    ASSERT_MSG(!m_bundles.contains(lookup_str), "ProcessBundleProperty should not be called if bundle already exists!")

    if (parts[IndexImage].empty())
    {
        return nullptr;
    }

    ImageBundle img_bundle;

    if (parts[IndexType].contains("Art"))
    {
        if (parts[IndexArtID].contains("|"))
        {
            tt_string_vector id_client(parts[IndexArtID], '|');
            img_bundle.bundle = wxArtProvider::GetBitmapBundle(id_client[0], wxART_MAKE_CLIENT_ID_FROM_STR(id_client[1]));
        }
        else
        {
            img_bundle.bundle =
                wxArtProvider::GetBitmapBundle(parts[IndexArtID].wx_str(), wxART_MAKE_CLIENT_ID_FROM_STR("wxART_OTHER"));
        }

        m_bundles[lookup_str] = std::move(img_bundle);
        return &m_bundles[lookup_str];
    }
    else if (parts[IndexType].contains("Embed"))
    {
        if (AddNewEmbeddedBundle(parts, parts[IndexImage], node->get_form()))
        {
            return &m_bundles[lookup_str];
        }
        else
        {
            return nullptr;
        }
    }
    else if (parts[IndexType].contains("SVG"))
    {
        if (AddNewEmbeddedBundle(parts, parts[IndexImage], node->get_form()))
        {
            return &m_bundles[lookup_str];
        }
        else
        {
            return nullptr;
        }
    }

    auto image_first = GetPropertyBitmap(parts, false);
    if (!image_first.IsOk())
    {
        return nullptr;
    }

    img_bundle.lst_filenames.emplace_back(parts[IndexImage]);

    if (auto pos = parts[IndexImage].find_last_of('.'); tt::is_found(pos))
    {
        if (parts[IndexImage].contains("_16x16."))
        {
            tt_string path(parts[IndexImage]);
            path.Replace("_16x16.", "_24x24.");
            if (!path.file_exists())
            {
                if (m_project_node->HasValue(prop_art_directory))
                {
                    path = m_project_node->prop_as_string(prop_art_directory);
                    path.append_filename(parts[IndexImage]);
                    path.Replace("_16x16.", "_24x24.");
                    if (path.file_exists())
                    {
                        img_bundle.lst_filenames.emplace_back(path);
                    }
                }
            }
            else
            {
                img_bundle.lst_filenames.emplace_back(path);
            }

            // Note that path may now contain the prop_art_directory prefix
            path.Replace("_24x24.", "_32x32.");
            if (path.file_exists())
            {
                img_bundle.lst_filenames.emplace_back(path);
            }
        }
        else if (parts[IndexImage].contains("_24x24."))
        {
            tt_string path(parts[IndexImage]);
            path.Replace("_24x24.", "_36x36.");
            if (!path.file_exists())
            {
                if (m_project_node->HasValue(prop_art_directory))
                {
                    path = m_project_node->prop_as_string(prop_art_directory);
                    path.append_filename(parts[IndexImage]);
                    path.Replace("_24x24.", "_36x36.");
                    if (path.file_exists())
                    {
                        img_bundle.lst_filenames.emplace_back(path);
                    }
                }
            }
            else
            {
                img_bundle.lst_filenames.emplace_back(path);
            }

            // Note that path may now contain the prop_art_directory prefix
            path.Replace("_36x36.", "_48x48.");
            if (path.file_exists())
            {
                img_bundle.lst_filenames.emplace_back(path);
            }
        }
        else
        {
            tt_string path;
            for (auto& iter: suffixes)
            {
                path = parts[IndexImage];
                path.insert(pos, iter);
                if (!path.file_exists())
                {
                    if (m_project_node->HasValue(prop_art_directory))
                    {
                        tt_string tmp_path = m_project_node->prop_as_string(prop_art_directory);
                        tmp_path.append_filename(path);
                        if (tmp_path.file_exists())
                        {
                            img_bundle.lst_filenames.emplace_back(tmp_path);
                        }
                    }
                }
                else
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
        }
    }

    ASSERT_MSG(img_bundle.lst_filenames.size() > 0, "image_first must always have it's filename added.")

    if (img_bundle.lst_filenames.size() == 1)
    {
        img_bundle.bundle = wxBitmapBundle::FromBitmap(image_first);
    }
    else
    {
        wxVector<wxBitmap> bitmaps;
        bitmaps.push_back(image_first);
        tt_string new_description;
        new_description << parts[IndexType] << ';';
        new_description << img_bundle.lst_filenames[1];
        auto image_second = GetPropertyBitmap(new_description, false);
        if (image_second.IsOk())
        {
            bitmaps.push_back(image_second);
        }

        if (img_bundle.lst_filenames.size() > 2)
        {
            new_description.clear();
            new_description << parts[IndexType] << ';';
            new_description << img_bundle.lst_filenames[1];
            auto image_third = GetPropertyBitmap(new_description, false);
            if (image_third.IsOk())
            {
                bitmaps.push_back(image_third);
            }
        }

        img_bundle.bundle = wxBitmapBundle::FromBitmaps(bitmaps);
    }

    m_bundles[lookup_str] = std::move(img_bundle);
    return &m_bundles[lookup_str];
}

void ImageHandler::UpdateBundle(const tt_string_vector& parts, Node* node)
{
    if (parts.size() < 2 || node->IsFormParent())
        return;

    tt_string lookup_str;
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

wxBitmapBundle ImageHandler::GetPropertyBitmapBundle(const tt_string& description, Node* node)
{
    tt_string_vector parts(description, ';', tt::TRIM::both);
    if (parts.size() < 2)
    {
        return GetInternalImage("unknown");
    }

    tt_string lookup_str;
    lookup_str << parts[IndexType] << ';' << parts[IndexImage].filename();

    if (auto result = m_bundles.find(lookup_str); result != m_bundles.end())
    {
        // At this point we know that the bundle has been stored, but the actual size for
        // display can change any time the property is used to retrieve the bundle.
        if (description.starts_with("SVG;"))
        {
            if (auto* embed = GetEmbeddedImage(parts[IndexImage]); embed)
            {
                ASSERT(parts.size() > IndexSize);
                return LoadSVG(embed, parts[IndexSize]);
            }
        }
        return result->second.bundle;
    }

    if (auto result = ProcessBundleProperty(parts, node); result)
    {
        return result->bundle;
    }

    return GetInternalImage("unknown");
}

const ImageBundle* ImageHandler::GetPropertyImageBundle(const tt_string_vector& parts, Node* node)
{
    if (parts.size() < 2)
    {
        return nullptr;
    }

    tt_string lookup_str;
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

wxAnimation ImageHandler::GetPropertyAnimation(const tt_string& description)
{
    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    wxAnimation image;

    if (parts.size() <= IndexImage || parts[IndexImage].empty())
    {
        return GetAnimFromHdr(wxue_img::pulsing_unknown_gif, sizeof(wxue_img::pulsing_unknown_gif));
    }

    tt_string path = parts[IndexImage];
    if (!path.file_exists())
    {
        path = Project.value(prop_art_directory);
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
bool ImageHandler::AddSvgBundleImage(tt_string path, Node* form)
{
    // Run the file through an XML parser so that we can remove content that isn't used, as well as removing line breaks,
    // leading spaces, etc.
    pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());
    if (!result)
    {
        return false;
    }

    auto root = doc.first_child();  // this should be the <svg> element.
    root.remove_attributes();       // we don't need any of the attributes

    // Remove some inkscape nodes that we don't need
    root.remove_child("sodipodi:namedview");
    root.remove_child("metadata");

    std::ostringstream xml_stream;
    doc.save(xml_stream, "", pugi::format_raw | pugi::format_no_declaration);
    std::string str = xml_stream.str();

    // Include the trailing zero -- we need to read this back as a string, not a data array
    wxMemoryInputStream stream(str.c_str(), str.size() + 1);

    wxMemoryOutputStream memory_stream;
    wxZlibOutputStream save_strem(memory_stream, wxZ_BEST_COMPRESSION);
    m_map_embedded[path.filename().as_str()] = std::make_unique<EmbeddedImage>();
    auto embed = m_map_embedded[path.filename().as_str()].get();
    InitializeArrayName(embed, path.filename());
    embed->form = form;

    size_t org_size = (stream.GetLength() & 0xFFFFFFFF);

    if (!CopyStreamData(&stream, &save_strem, stream.GetLength()))
    {
        // TODO: [KeyWorks - 03-16-2022] This would be really bad, though it should be impossible
        return false;
    }
    save_strem.Close();
    auto compressed_size = memory_stream.TellO();

    auto read_stream = memory_stream.GetOutputStreamBuffer();
    embed->type = wxBITMAP_TYPE_INVALID;
    embed->array_size = (compressed_size | (org_size << 32));
    embed->array_data = std::make_unique<unsigned char[]>(compressed_size);
    memcpy(embed->array_data.get(), read_stream->GetBufferStart(), compressed_size);

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    wxFile file_original(path.wx_str(), wxFile::read);
    if (file_original.IsOpened())
    {
        auto file_size = file_original.Length();
        tt_string size_comparison;
        int percent = static_cast<int>(100 - (100 / (file_size / compressed_size)));
        size_comparison.Format("%v -- Original: %ku, compressed: %ku, %u percent", path.filename(), file_size,
                               compressed_size, percent);
        // Enable line below to show results for every file
        // MSG_INFO(size_comparison)
    }
#endif

    return true;
}

wxBitmapBundle LoadSVG(EmbeddedImage* embed, tt_string_view size_description)
{
    size_t org_size = (embed->array_size >> 32);
    auto str = std::make_unique<char[]>(org_size);
    wxMemoryInputStream stream_in(embed->array_data.get(), embed->array_size & 0xFFFFFFFF);
    wxZlibInputStream zlib_strm(stream_in);
    zlib_strm.Read(str.get(), org_size);
    return wxBitmapBundle::FromSVG(str.get(), get_image_prop_size(size_description));
}

tt_string ImageHandler::GetBundleFuncName(const tt_string& description)
{
    tt_string name;

    for (const auto& form: Project.ChildNodePtrs())
    {
        if (form->isGen(gen_Images))
        {
            tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts.size() < 2)
            {
                // caller's description does not include a filename
                return name;
            }

            for (const auto& child: form->GetChildNodePtrs())
            {
                tt_view_vector form_image_parts(child->prop_as_string(prop_bitmap), BMP_PROP_SEPARATOR, tt::TRIM::both);
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

std::optional<tt_string> ImageHandler::FileNameToVarName(tt_string_view filename)
{
    if (filename.empty())
    {
        // caller's description does not include a filename
        return {};
    }

    tt_string var_name;

    if (tt::is_digit(filename[0]))
    {
        var_name += "img_";
    }

    for (auto iter: filename)
    {
        if (tt::is_alnum(iter) || iter == '_')
        {
            var_name += iter;
        }
        else
        {
            if (iter == '.')
            {
                // Always convert a period to an underscore in case it is preceeding the extension
                var_name += '_';
            }
            else if (var_name.back() != '_')
            {
                var_name += '_';
            }
            else
            {
                // convert char to hex string
                tt_string hex;
                hex.Format("%x", static_cast<int>(iter) & 0xFF);

                // Ignore any 0xff characters which are utf-8 bytes
                if (hex != "ff")
                {
                    var_name += hex;
                }
            }
        }

        if (var_name.size() > 135)
        {
            // We don't want to create a variable name that is too long
            var_name += "_name_truncated";
            break;
        }
    }

    return var_name;
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
