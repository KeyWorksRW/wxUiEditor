/////////////////////////////////////////////////////////////////////////////
// Purpose:   ImageHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <format>
#include <utility>
#include <vector>

#include <wx/animate.h>   // wxAnimation and wxAnimationCtrl
#include <wx/artprov.h>   // wxArtProvider class
#include <wx/dirdlg.h>    // wxDirDialog base class
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes
#include <wx/zstream.h>   // zlib stream classes

#include "image_handler.h"

#include "bitmaps.h"           // Contains various images handling functions
#include "mainframe.h"         // MainFrame -- Main window frame
#include "node.h"              // Node class
#include "project_handler.h"   // ProjectHandler -- Project class
#include "pugixml.hpp"         // xml parser
#include "tt_string_vector.h"  // tt_string_vector -- Read/Write line-oriented strings/files
#include "tt_view_vector.h"    // tt_view_vector -- read/write line-oriented strings/files
#include "ui_images.h"         // Contains various images handling functions
#include "utils.h"             // Miscellaneous utility functions

ImageHandler& ProjectImages = ImageHandler::getInstance();

namespace wxue_img
{
    extern const unsigned char pulsing_unknown_gif[377];
}

auto ImageHandler::ConvertToLookup(const tt_string& description) -> tt_string
{
    tt_view_vector parts(description, ';', tt::TRIM::both);
    ASSERT(parts.size() > 1)

    tt_string lookup_str;
    lookup_str << parts[0] << ';' << parts[1].filename();
    return lookup_str;
}

auto ImageHandler::ConvertToLookup(const tt_string_vector* parts) -> tt_string
{
    tt_string lookup_str;
    lookup_str << (*parts)[0] << ';' << (*parts)[1].filename();
    return lookup_str;
}

void ImageHandler::Initialize(NodeSharedPtr project, bool allow_ui)
{
    m_project_node = std::move(project);
    m_allow_ui = allow_ui;

    m_bundles.clear();
    m_images.clear();
    m_map_embedded.clear();
}

auto ImageHandler::UpdateEmbedNodes() -> bool
{
    bool is_changed = false;
    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        if (CheckNode(form))
        {
            is_changed = true;
        }
    }
    return is_changed;
}

auto ImageHandler::FindEmbedded(std::string_view filename) -> EmbeddedImage*
{
    if (auto result = m_map_embedded.find(filename); result != m_map_embedded.end())
    {
        return result->second.get();
    }
    return nullptr;
}

auto ImageHandler::CheckNode(Node* node) -> bool
{
    if (node->is_FormParent())
    {
        return false;
    }

    bool is_changed = false;

    Node* node_form = node->get_Form();

    auto node_position = m_project_node->get_ChildPosition(node_form);
    std::string art_directory;
    if (Project.get_ProjectNode()->HasValue(prop_art_directory))
    {
        art_directory = Project.get_ProjectNode()->as_view(prop_art_directory);
    }

    for (auto& iter: node->get_PropsVector())
    {
        if ((iter.type() == type_image || iter.type() == type_animation) && iter.HasValue())
        {
            tt_view_vector parts(iter.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts[IndexType] != "Embed" || parts.size() <= IndexImage ||
                parts[IndexImage].filename().empty() || parts[IndexImage] == art_directory)
            {
                continue;
            }

            EmbeddedImage* embed = FindEmbedded(parts[IndexImage].filename());
            if (!embed)
            {
                ASSERT_MSG(embed,
                           std::format("Embedded image not found: {}", parts[IndexImage].as_str()));
                continue;
            }

            if (node_form->is_Gen(gen_Images))
            {
                if (embed->get_Form() != node_form)
                {
                    embed->set_Form(node_form);
                    is_changed = true;
                }
            }
            else
            {
                if (auto child_pos = m_project_node->get_ChildPosition(embed->get_Form());
                    child_pos > node_position)
                {
                    // The original embed->get_Form() is setup by parsing all of the nodes. However,
                    // code generation may not actually have a file set for a form, in which
                    // case the first use of the image for generated code can be in a different
                    // image. You'll see this in the python_tests project where some forms are
                    // only generated for C++, but not Python.

                    embed->set_Form(node_form);
                    is_changed = true;
                }
            }
        }
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (CheckNode(child.get()))
        {
            is_changed = true;
        }
    }

    return is_changed;
}

wxImage ImageHandler::GetImage(const tt_string& description)
{
    if (description.starts_with("Embed;") || description.starts_with("XPM;") ||
        description.starts_with("Header;") || description.starts_with("Art;"))
    {
        return GetPropertyBitmap(description);
    }
    return GetInternalImage("unknown");
}

wxBitmapBundle ImageHandler::GetBitmapBundle(const tt_string& description)
{
    if (description.starts_with("Embed;") || description.starts_with("XPM;") ||
        description.starts_with("Header;") || description.starts_with("Art;") ||
        description.starts_with("SVG;"))
    {
        return GetPropertyBitmapBundle(description);
    }

    return wxue_img::bundle_unknown_svg(32, 32);
}

// Wrapper functions that convert string descriptions to tt_string_vector and call pointer versions

auto ImageHandler::GetPropertyBitmap(const tt_string& description, bool check_image) -> wxImage
{
    tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
    return GetPropertyBitmap(&parts, check_image);
}

auto ImageHandler::GetPropertyImageBundle(tt_string_view description, Node* node)
    -> const ImageBundle*
{
    tt_string_vector parts(description, ';', tt::TRIM::both);
    return GetPropertyImageBundle(&parts, node);
}

auto ImageHandler::ProcessBundleProperty(const tt_string& description, Node* node) -> ImageBundle*
{
    tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
    return ProcessBundleProperty(&parts, node);
}

auto ImageHandler::AddNewEmbeddedBundle(const tt_string& description, std::string_view org_path,
                                        Node* form) -> bool
{
    tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
    return AddNewEmbeddedBundle(&parts, org_path, form);
}

// This gets called by PropertyGrid_Image::RefreshChildren() in pg_image.cpp when an XPM file
// is encountered.
//
// Primary caller is ProcessBundleProperty() for retrieving all the images in a bundle.
auto ImageHandler::GetPropertyBitmap(const tt_string_vector* parts, bool check_image) -> wxImage
{
    if (parts->size() <= IndexImage || (*parts)[IndexImage].empty())
    {
        return GetInternalImage("unknown");
    }

    wxImage image;

    tt_string path = (*parts)[IndexImage];

    if (auto result = m_images.find(path.filename()); result != m_images.end())
    {
        image = result->second;
    }
    else if ((*parts)[IndexType].contains("Art"))
    {
        if ((*parts)[IndexArtID].contains("|"))
        {
            tt_string_vector id_client((*parts)[IndexArtID], '|');
            ASSERT_MSG(m_allow_ui, "We should never get here if m_allow_ui is false");
            image = (wxArtProvider::GetBitmapBundle(id_client[0],
                                                    wxART_MAKE_CLIENT_ID_FROM_STR(id_client[1]))
                         .GetBitmapFor(wxGetFrame().getWindow()))
                        .ConvertToImage();
        }
        else
        {
            image = (wxArtProvider::GetBitmapBundle((*parts)[IndexArtID].make_wxString(),
                                                    wxART_MAKE_CLIENT_ID_FROM_STR("wxART_OTHER"))
                         .GetBitmapFor(wxGetFrame().getWindow()))
                        .ConvertToImage();
        }
    }
    else if ((*parts)[IndexType].contains("Embed"))
    {
        if (!path.file_exists())
        {
            path = m_project_node->as_string(prop_art_directory);
            path.append_filename((*parts)[IndexImage]);
        }
        EmbeddedImage* embed = GetEmbeddedImage(path);
        if (!embed)
        {
            if (bool added = AddEmbeddedImage(path, wxGetFrame().getSelectedForm()); added)
            {
                embed = GetEmbeddedImage(path);
            }
        }

        if (embed)
        {
            wxMemoryInputStream stream(embed->base_image().array_data.data(),
                                       embed->base_image().array_size);
            image.LoadFile(stream);
        }
    }
    else
    {
        if (!path.file_exists())
        {
            path = m_project_node->as_string(prop_art_directory);
            path.append_filename((*parts)[IndexImage]);

            if (result = m_images.find(path.filename()); result != m_images.end())
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

    // If it's not embedded, then cache it so that we don't read it from disk again. This will
    // be for xpm files.
    if (!(*parts)[IndexType].contains("Embed") &&
        !m_images.contains((*parts)[IndexImage].filename()))
    {
        m_images[path.filename().as_str()] = image;
    }

    return image;
}

EmbeddedImage* ImageHandler::GetEmbeddedImage(tt_string_view path)
{
    if (auto result = m_map_embedded.find(path.filename()); result != m_map_embedded.end())
    {
        return result->second.get();
    }

    return nullptr;
}

// This is called in BaseCodeGenerator::CollectImageHeaders (gen_base.cpp) when an animation file is
// found that was not previously loaded.
bool ImageHandler::AddEmbeddedImage(tt_string path, Node* form, bool is_animation)
{
    if (!path.file_exists())
    {
        if (m_project_node->HasValue(prop_art_directory))
        {
            tt_string art_path = m_project_node->as_string(prop_art_directory);
            art_path.append_filename(path);
            if (!art_path.file_exists())
            {
                return false;
            }
            path = art_path;
        }
        else
        {
            return false;
        }
    }

    if (m_map_embedded.contains(path.filename().as_str()))
    {
        return false;
    }

    auto final_result = AddNewEmbeddedImage(path, form);
    if (is_animation || !final_result)
    {
        return final_result;
    }

    // Note that path may now contain the prop_art_directory prefix

    if (auto pos = path.find_last_of('.'); ttwx::is_found(pos))
    {
        if (path.contains("_16x16."))
        {
            path.Replace("_16x16.", "_24x24.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
            path.Replace("_24x24.", "_32x32.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
        }
        else if (path.contains("_24x24."))
        {
            path.Replace("_24x24.", "_36x36.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
            path.Replace("_36x36.", "_48x48.");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
        }
        else
        {
            path.insert(pos, "_1_25x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
            path.Replace("_1_25x", "_1_5x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
            path.Replace("_1_5x", "_1_75x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
            path.Replace("_1_75x", "_2x");
            if (path.file_exists())
            {
                AddNewEmbeddedImage(path, form);
            }
        }
    }

    return final_result;
}

auto ImageHandler::AddNewEmbeddedImage(const tt_string& path, Node* form) -> bool
{
    wxFFileInputStream stream(path.make_wxString());
    if (!stream.IsOk())
    {
        return false;
    }

    wxImageHandler* handler = nullptr;
    auto& list = wxImage::GetHandlers();
    for (auto node = list.GetFirst(); node; node = node->GetNext())
    {
        handler = dynamic_cast<wxImageHandler*>(node->GetData());
        if (handler->CanRead(stream))
        {
            wxImage image;
            if (handler->LoadFile(&image, stream))
            {
                auto filename = path.filename().as_str();
                m_map_embedded[filename] = std::make_unique<EmbeddedImage>(path, form);
                auto* embed = m_map_embedded[filename].get();

                // If possible, convert the file to a PNG -- even if the original file is a PNG,
                // since we might end up with better compression.

                if (isConvertibleMime(handler->GetMimeType()))
                {
                    embed->base_image().type = wxBITMAP_TYPE_PNG;

                    wxMemoryOutputStream save_stream;

                    // Maximize compression
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
                    image.SaveFile(save_stream, "image/png");

                    auto* read_stream = save_stream.GetOutputStreamBuffer();
                    stream.SeekI(0);
                    if (read_stream->GetBufferSize() <= (to_size_t) stream.GetLength())
                    {
                        embed->base_image().array_size = read_stream->GetBufferSize();
                        embed->base_image().array_data.resize(embed->base_image().array_size);
                        memcpy(embed->base_image().array_data.data(), read_stream->GetBufferStart(),
                               embed->base_image().array_size);
                    }
                    else
                    {
#if defined(_DEBUG)
                        size_t org_size = (to_size_t) stream.GetLength();
                        auto png_size = read_stream->GetBufferSize();
                        auto size_comparison = std::format(
                            std::locale(""), "Original: {:L}, new: {:L}", org_size, png_size);
                        (void) size_comparison;
#endif  // _DEBUG

                        embed->base_image().type = handler->GetType();
                        embed->base_image().array_size = stream.GetSize();
                        embed->base_image().array_data.resize(embed->base_image().array_size);
                        stream.Read(embed->base_image().array_data.data(),
                                    embed->base_image().array_size);
                    }
                }
                else
                {
                    embed->base_image().type = handler->GetType();

                    stream.SeekI(0);
                    embed->base_image().array_size = stream.GetSize();
                    embed->base_image().array_data.resize(embed->base_image().array_size);
                    stream.Read(embed->base_image().array_data.data(),
                                embed->base_image().array_size);
                }

                return true;
            }
        }
    }

    return false;
}

// This gets called whenever a project is loaded or imported.
void ImageHandler::CollectBundles()
{
    if (m_allow_ui)
    {
        wxBusyCursor wait;
    }

    tt_cwd save_cwd(true);
    Project.get_ProjectPath().ChangeDir();

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        CollectNodeBundles(form, form);

        if (form->HasProp(prop_icon) && form->HasValue(prop_icon))
        {
            if (!m_bundles.contains(ConvertToLookup(form->as_string(prop_icon))))
            {
                ProcessBundleProperty(form->as_string(prop_icon), form);
            }
        }
    }
}

void ImageHandler::CollectNodeBundles(Node* node, Node* form)
{
    for (auto& iter: node->get_PropsVector())
    {
        if (!iter.HasValue())
        {
            continue;
        }

        if (iter.type() == type_image)
        {
            if (!m_bundles.contains(ConvertToLookup(iter.as_string())))
            {
                ProcessBundleProperty(iter.as_string(), form);
            }
        }
        else if (iter.type() == type_animation)
        {
            const auto& value = iter.as_string();
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

    for (const auto& child: node->get_ChildNodePtrs())
    {
        CollectNodeBundles(child.get(), form);
    }
}

// This will call AddSvgBundleImage(), AddXpmBundleImage() or AddEmbeddedBundleImage()
// depending on the type of the image file.
auto ImageHandler::AddNewEmbeddedBundle(const tt_string_vector* parts, std::string_view org_path,
                                        Node* form) -> bool
{
    ASSERT(parts->size() > 1)

    auto lookup_str = ConvertToLookup(parts);

    ImageBundle img_bundle;
    tt_string path(org_path);

    if (!path.file_exists())
    {
        if (m_project_node->HasValue(prop_art_directory))
        {
            tt_string art_path = m_project_node->as_string(prop_art_directory);
            art_path.append_filename(path);
            if (!art_path.file_exists())
            {
                m_bundles[lookup_str] = img_bundle;
                return true;
            }
            path = art_path;
        }
        else
        {
            m_bundles[lookup_str] = img_bundle;
            return true;
        }
    }

    // At this point, the image file has been found.

    if ((*parts)[IndexType].starts_with("SVG"))
    {
        if (AddSvgBundleImage(path, form))
        {
            img_bundle.lst_filenames.emplace_back(path);
            if (auto* embed = GetEmbeddedImage(path); embed)
            {
                m_bundles[lookup_str] = std::move(img_bundle);
                return true;
            }
        }
        return false;  // presumably an invalid SVG file
    }

    if ((*parts)[IndexType].starts_with("XPM"))
    {
        if (AddXpmBundleImage(path, form))
        {
            img_bundle.lst_filenames.emplace_back(path);
            if (auto* embed = GetEmbeddedImage(path); embed)
            {
                m_bundles[lookup_str] = std::move(img_bundle);
                return true;
            }
        }
        return false;  // presumably an invalid XPM file
    }

    auto* embed = AddEmbeddedBundleImage(path, form);
    if (!embed)
    {
        return false;
    }

    img_bundle.lst_filenames.emplace_back(path);

    /*

        Look for suffix combinations -- it's fine if one of them doesn't exist

            _16x16, _24x24, _32x32
            _24x24, _36x36, _48x48
            any, _1_5x, _1_75x, _2x
            any, @1_5x, @1_75, x@2x

    */

    if (tt_string extension = path.extension(); extension.size())
    {
        if (path.contains("_16x16."))
        {
            path.Replace("_16x16.", "_24x24.");
            if (path.file_exists())
            {
                if (auto* added = AddEmbeddedBundleImage(path, form, embed); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
            path.Replace("_24x24.", "_32x32.");
            if (path.file_exists())
            {
                if (auto* added = AddEmbeddedBundleImage(path, form, embed); added)
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
                if (auto* added = AddEmbeddedBundleImage(path, form, embed); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
            path.Replace("_36x36.", "_48x48.");
            if (path.file_exists())
            {
                if (auto* added = AddEmbeddedBundleImage(path, form, embed); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
        }
        else
        {
            tt_string additional_path = path;
            // size_t file_count = 1;
            auto map_pos = map_bundle_extensions.begin();
            for (; map_pos != map_bundle_extensions.end(); ++map_pos)
            {
                if (path.contains(map_pos->first))
                {
                    break;
                }
            }

            // This will be the most common case where the first filename contains no suffix.
            if (map_pos == map_bundle_extensions.end())
            {
                tt_string file_extension = additional_path.extension();
                additional_path.remove_extension();
                auto erase_pos = additional_path.size();
                for (map_pos = map_bundle_extensions.begin();
                     map_pos != map_bundle_extensions.end(); ++map_pos)
                {
                    if (erase_pos < additional_path.size())
                    {
                        additional_path.erase(erase_pos);
                    }
                    additional_path << map_pos->first << file_extension;
                    if (additional_path.file_exists())
                    {
                        if (auto* added = AddEmbeddedBundleImage(additional_path, form, embed);
                            added)
                        {
                            img_bundle.lst_filenames.emplace_back(additional_path);
                            // ++file_count;
                            break;
                        }
                    }
                }
            }

            bool is_at_suffix =
                (map_pos != map_bundle_extensions.end() && map_pos->first.starts_with('@'));
            // while (file_count < 2 && map_pos != map_bundle_extensions.end())
            while (map_pos != map_bundle_extensions.end())
            {
                // If we have a map position, then we have found a suffix, so we now try to find the
                // next matching filename.
                additional_path.Replace(map_pos->first, map_pos->second);
                if (additional_path.file_exists())
                {
                    if (auto* added = AddEmbeddedBundleImage(additional_path, form, embed); added)
                    {
                        img_bundle.lst_filenames.emplace_back(additional_path);
                        // ++file_count;
                    }
                }

                ++map_pos;
                if (is_at_suffix && map_pos != map_bundle_extensions.end() &&
                    !map_pos->first.starts_with('@'))
                {
                    // We have run out of '@' suffixes to look for
                    break;
                }
            }
        }
    }

    m_bundles[lookup_str] = std::move(img_bundle);
    return true;
}

EmbeddedImage* ImageHandler::AddEmbeddedBundleImage(tt_string& path, Node* form,
                                                    EmbeddedImage* embed)
{
    wxFFileInputStream stream(path.make_wxString());
    if (!stream.IsOk())
    {
        return nullptr;
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
                if (embed)
                {
                    embed->add_ImageInfo();
                }
                size_t idx = embed ? embed->get_ImageInfos().size() - 1 : 0;
                if (!embed)
                {
                    m_map_embedded[path.filename().as_str()] =
                        std::make_unique<EmbeddedImage>(path, form);
                    embed = m_map_embedded[path.filename().as_str()].get();
                    embed->SetEmbedSize(image);
                }
                else if (idx != 0)
                {
                    embed->get_ImageInfo(idx).filename = path;
                    embed->get_ImageInfo(idx).file_time =
                        embed->get_ImageInfo(idx).filename.last_write_time();
                    if (auto result = FileNameToVarName(path.filename()); result)
                    {
                        embed->get_ImageInfo(idx).array_name = result.value();
                    }
                    else
                    {
                        embed->get_ImageInfo(idx).array_name = embed->base_image().array_name;
                        embed->get_ImageInfo(idx).array_name << "_" << idx;
                    }
                }

                // If possible, convert the file to a PNG -- even if the original file is a PNG,
                // since we might end up with better compression.

                if (isConvertibleMime(handler->GetMimeType()))
                {
                    embed->get_ImageInfo(idx).type = wxBITMAP_TYPE_PNG;

                    wxMemoryOutputStream save_stream;

                    // Maximize compression
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
                    image.SaveFile(save_stream, "image/png");

                    auto* read_stream = save_stream.GetOutputStreamBuffer();
                    stream.SeekI(0);
                    if (read_stream->GetBufferSize() <= (to_size_t) stream.GetLength())
                    {
                        embed->get_ImageInfo(idx).array_size = read_stream->GetBufferSize();
                        embed->get_ImageInfo(idx).array_data.resize(
                            embed->get_ImageInfo(idx).array_size);
                        memcpy(embed->get_ImageInfo(idx).array_data.data(),
                               read_stream->GetBufferStart(), embed->get_ImageInfo(idx).array_size);
                    }
                    else
                    {
#if defined(_DEBUG)
                        size_t org_size = (to_size_t) stream.GetLength();
                        auto png_size = read_stream->GetBufferSize();
                        auto size_comparison = std::format(
                            std::locale(""), "Original: {:L}, new: {:L}", org_size, png_size);
                        (void) size_comparison;
#endif

                        embed->get_ImageInfo(idx).type = handler->GetType();
                        embed->get_ImageInfo(idx).array_size = stream.GetSize();
                        embed->get_ImageInfo(idx).array_data.resize(
                            embed->get_ImageInfo(idx).array_size);
                        stream.Read(embed->get_ImageInfo(idx).array_data.data(),
                                    embed->get_ImageInfo(idx).array_size);
                    }
                }
                else
                {
                    embed->get_ImageInfo(idx).type = handler->GetType();

                    stream.SeekI(0);
                    embed->get_ImageInfo(idx).array_size = stream.GetSize();
                    embed->get_ImageInfo(idx).array_data.resize(
                        embed->get_ImageInfo(idx).array_size);
                    stream.Read(embed->get_ImageInfo(idx).array_data.data(),
                                embed->get_ImageInfo(idx).array_size);
                }

                // TODO: [Randalphwa - 03-14-2024] Remove this once m_bundles is no longer used
                if (embed->get_ImageInfos().size() > 1)
                {
                    AddEmbeddedBundleImage(path, form);
                }

                return embed;
            }
        }
    }
    return nullptr;
}

auto ImageHandler::ProcessBundleProperty(const tt_string_vector* parts, Node* node) -> ImageBundle*
{
    ASSERT(parts->size() > 1)

    auto lookup_str = ConvertToLookup(parts);

    if ((*parts)[IndexImage].empty())
    {
        return nullptr;
    }

    ImageBundle img_bundle;

    if ((*parts)[IndexType].contains("Art"))
    {
        m_bundles[lookup_str] = std::move(img_bundle);
        return &m_bundles[lookup_str];
    }
    if ((*parts)[IndexType].contains("Embed"))
    {
        if (AddNewEmbeddedBundle(parts, (*parts)[IndexImage], node->get_Form()))
        {
            return &m_bundles[lookup_str];
        }

        return nullptr;
    }
    if ((*parts)[IndexType].contains("SVG") || (*parts)[IndexType].contains("XPM"))
    {
        // The user is allowed to change the dimensions of an SVG image, but that doesn't mean
        // it's a new image. So first check to see if we already have the image.

        if (auto result = m_bundles.find(lookup_str); result != m_bundles.end())
        {
            return &result->second;
        }

        if (AddNewEmbeddedBundle(parts, (*parts)[IndexImage], node->get_Form()))
        {
            return &m_bundles[lookup_str];
        }

        return nullptr;
    }

    auto image_first = GetPropertyBitmap(parts, false);
    if (!image_first.IsOk())
    {
        return nullptr;
    }

    img_bundle.lst_filenames.emplace_back((*parts)[IndexImage]);

    if (auto pos = (*parts)[IndexImage].find_last_of('.'); ttwx::is_found(pos))
    {
        if ((*parts)[IndexImage].contains("_16x16."))
        {
            tt_string path((*parts)[IndexImage]);
            path.Replace("_16x16.", "_24x24.");
            if (!path.file_exists())
            {
                if (m_project_node->HasValue(prop_art_directory))
                {
                    path = m_project_node->as_string(prop_art_directory);
                    path.append_filename((*parts)[IndexImage]);
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
        else if ((*parts)[IndexImage].contains("_24x24."))
        {
            tt_string path((*parts)[IndexImage]);
            path.Replace("_24x24.", "_36x36.");
            if (!path.file_exists())
            {
                if (m_project_node->HasValue(prop_art_directory))
                {
                    path = m_project_node->as_string(prop_art_directory);
                    path.append_filename((*parts)[IndexImage]);
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
            for (const auto& iter: suffixes)
            {
                path = (*parts)[IndexImage];
                path.insert(pos, iter);
                if (!path.file_exists())
                {
                    if (m_project_node->HasValue(prop_art_directory))
                    {
                        tt_string tmp_path = m_project_node->as_string(prop_art_directory);
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

    ASSERT_MSG(img_bundle.lst_filenames.size() > 0,
               "image_first must always have it's filename added.")

    if (img_bundle.lst_filenames.size() == 1)
    {
        // img_bundle.bundle = wxBitmapBundle::FromBitmap(image_first);
    }
    else
    {
        wxVector<wxBitmap> bitmaps;
        bitmaps.emplace_back(image_first);
        tt_string new_description;
        new_description << (*parts)[IndexType] << ';';
        new_description << img_bundle.lst_filenames[1];
        auto image_second = GetPropertyBitmap(new_description, false);
        if (image_second.IsOk())
        {
            bitmaps.push_back(image_second);
        }

        if (img_bundle.lst_filenames.size() > 2)
        {
            new_description.clear();
            new_description << (*parts)[IndexType] << ';';
            new_description << img_bundle.lst_filenames[1];
            auto image_third = GetPropertyBitmap(new_description, false);
            if (image_third.IsOk())
            {
                bitmaps.push_back(image_third);
            }
        }

        // img_bundle.bundle = wxBitmapBundle::FromBitmaps(bitmaps);
    }

    m_bundles[lookup_str] = std::move(img_bundle);
    return &m_bundles[lookup_str];
}

void ImageHandler::UpdateBundle(const tt_string_vector* parts, Node* node)
{
    if (parts->size() < 2 || node->is_FormParent())
    {
        return;
    }

    // ProcessBundleProperty() will add a new bundle, or replace an old bundle if the path has
    // changed.

    ProcessBundleProperty(parts, node);
    auto result = m_bundles.find(ConvertToLookup(parts));

    if (result != m_bundles.end() && result->second.lst_filenames.size())
    {
        auto* form = node->get_Form();
        for (auto& iter: result->second.lst_filenames)
        {
            if (auto* embed = GetEmbeddedImage(iter); embed)
            {
                if (embed->get_Form() != form)
                {
                    // This will happen when a bundle bitmap is added to the Images generator. The
                    // initial bitmap will be correctly changed to use the new form, but we also
                    // need to process  all the sub images as well

                    if (form->is_Gen(gen_Images))
                    {
                        embed->set_Form(form);
                    }
                }
            }
        }
    }
}

wxBitmapBundle ImageHandler::GetPropertyBitmapBundle(tt_string_view description)
{
    tt_string_vector parts(description, ';', tt::TRIM::both);
    if (parts.size() < 2)
    {
        return wxue_img::bundle_unknown_svg(32, 32);
    }

    if (auto* embed = FindEmbedded(parts[IndexImage].filename()); embed)
    {
        return embed->get_bundle(parts.size() > 2 ? GetSizeInfo(parts[IndexSize]) : wxDefaultSize);
    }

    if (parts[IndexType].contains("Art"))
    {
        if (parts[IndexArtID].contains("|"))
        {
            tt_string_vector id_client(parts[IndexArtID], '|');
            return wxArtProvider::GetBitmapBundle(id_client[0],
                                                  wxART_MAKE_CLIENT_ID_FROM_STR(id_client[1]));
        }

        return wxArtProvider::GetBitmapBundle(parts[IndexArtID].make_wxString(),
                                              wxART_MAKE_CLIENT_ID_FROM_STR("wxART_OTHER"));
    }

    if (auto result = m_images.find(parts[IndexImage].filename()); result != m_images.end())
    {
        return wxBitmapBundle::FromBitmap(result->second);
    }

    return wxue_img::bundle_unknown_svg(32, 32);
}

const ImageBundle* ImageHandler::GetPropertyImageBundle(const tt_string_vector* parts, Node* node)
{
    if (parts->size() < 2)
    {
        return nullptr;
    }

    if (auto result = m_bundles.find(ConvertToLookup(parts)); result != m_bundles.end())
    {
        return &result->second;
    }
    if (node)
    {
        return ProcessBundleProperty(parts, node);
    }
    else
    {
        return nullptr;
    }
}

void ImageHandler::GetPropertyAnimation(const tt_string& description, wxAnimation* p_animation)
{
    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts.size() <= IndexImage || parts[IndexImage].empty())
    {
        wxMemoryInputStream stream(wxue_img::pulsing_unknown_gif,
                                   sizeof(wxue_img::pulsing_unknown_gif));
        p_animation->Load(stream);
        return;
    }

    tt_string path = parts[IndexImage];
    if (!path.file_exists())
    {
        if (path == Project.as_string(prop_art_directory))
        {
            wxMemoryInputStream stream(wxue_img::pulsing_unknown_gif,
                                       sizeof(wxue_img::pulsing_unknown_gif));
            p_animation->Load(stream);
            return;
        }
        path = Project.as_string(prop_art_directory);
        path.append_filename(parts[IndexImage]);
    }

    if (parts[IndexType].contains("Embed"))
    {
        EmbeddedImage* embed = GetEmbeddedImage(path);
        if (!embed)
        {
            if (auto added = AddEmbeddedImage(path, wxGetFrame().getSelectedForm()); added)
            {
                embed = GetEmbeddedImage(path);
            }
        }

        if (embed)
        {
            wxMemoryInputStream stream(embed->base_image().array_data.data(),
                                       embed->base_image().array_size);
            p_animation->Load(stream);
        }
    }
}

bool ImageHandler::AddSvgBundleImage(tt_string& path, Node* form)
{
    // Run the file through an XML parser so that we can remove content that isn't used, as well as
    // removing line breaks, leading spaces, etc.
    pugi::xml_document doc;
    if (auto result = doc.load_file_string(path); !result)
    {
        if (!wxGetApp().is_Generating())
        {
            wxMessageDialog(wxGetMainFrame()->getWindow(), result.detailed_msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();
        }
        else
        {
            wxGetApp().get_CmdLineLog().emplace_back(std::string("Error parsing '") +
                                                     path.filename().ToStdString() +
                                                     "': " + result.detailed_msg);
        }
        return false;
    }

    // The InkScape program adds a lot of extra stuff that is not used when rendering the SVG.

    auto root = doc.first_child();  // this should be the <svg> element.
    if (root.name() == "svg")
    {
        root.remove_attribute("inkscape:version");
        root.remove_attribute("sodipodi:docname");
        root.remove_attribute("xml:space");
        root.remove_attribute("xmlns");
        root.remove_attribute("xmlns:inkscape");
        root.remove_attribute("xmlns:sodipodi");
        root.remove_attribute("xmlns:svg");
        root.remove_attribute("xmlns:xlink");
    }

    // Remove some inkscape nodes that we don't need
    root.remove_child("sodipodi:namedview");
    root.remove_child("metadata");
    root.remove_child("title");

    std::ostringstream xml_stream;
    doc.save(xml_stream, "", pugi::format_raw | pugi::format_no_declaration);
    std::string str = xml_stream.str();

    // Include the trailing zero -- we need to read this back as a string, not a data array
    wxMemoryInputStream stream(str.c_str(), str.size() + 1);

    wxMemoryOutputStream memory_stream;
    wxZlibOutputStream save_strem(memory_stream, wxZ_BEST_COMPRESSION);
    m_map_embedded[path.filename().as_str()] = std::make_unique<EmbeddedImage>(path, form);
    auto* embed = m_map_embedded[path.filename().as_str()].get();

    uint64_t org_size = (stream.GetLength() & 0xFFFFFFFF);

    if (!CopyStreamData(&stream, &save_strem, stream.GetLength()))
    {
        FAIL_MSG(tt_string() << "Failed to copy stream data");
        return false;
    }
    save_strem.Close();
    auto compressed_size = static_cast<uint64_t>(memory_stream.TellO());

    auto* read_stream = memory_stream.GetOutputStreamBuffer();
    embed->base_image().type = wxBITMAP_TYPE_SVG;
    embed->base_image().array_size = (compressed_size | (org_size << 32));
    embed->base_image().array_data.resize(compressed_size);
    memcpy(embed->base_image().array_data.data(), read_stream->GetBufferStart(), compressed_size);

    // We don't actually use this size, but we set it here just in case we want it later.
    wxSize size { -1, -1 };
    if (auto width_attribute = root.attribute("width"); width_attribute)
    {
        size.x = width_attribute.as_int();
        if (auto height_attribute = root.attribute("height"); height_attribute)
        {
            size.y = height_attribute.as_int();
        }
    }
    else if (auto viewBox_attribute = root.attribute("viewBox"); viewBox_attribute)
    {
        tt_string_vector parts(viewBox_attribute.as_sview(), ' ', tt::TRIM::left);
        if (parts.size() == 4)
        {
            size.x = parts[2].atoi();
            size.y = parts[3].atoi();
        }
    }

    while (size.x > 256 || size.y > 256)
    {
        size.x /= 2;
        size.y /= 2;
    }
    embed->set_wxSize(size);

#if defined(_DEBUG)
    wxFile file_original(path.make_wxString(), wxFile::read);
    if (file_original.IsOpened())
    {
        auto file_size = file_original.Length();
        tt_string size_comparison;
        int percent = static_cast<int>(100 - (100 / (file_size / compressed_size)));
        size_comparison =
            std::format(std::locale(""), "{} -- Original: {:L}, compressed: {:L}, {} percent",
                        path.filename().ToStdString(), file_size, compressed_size, percent);
        // Enable line below to show results for every file
        // MSG_INFO(size_comparison)
    }
#endif

    return true;
}

bool ImageHandler::AddXpmBundleImage(const tt_string& path, Node* form)
{
    wxFFileInputStream stream(path.make_wxString());
    if (!stream.IsOk())
    {
        return false;
    }
    uint64_t org_size = (stream.GetLength() & 0xFFFFFFFF);

    wxImage image;
    if (!image.LoadFile(stream, wxBITMAP_TYPE_XPM))
    {
        return false;
    }

    m_map_embedded[path.filename().as_str()] = std::make_unique<EmbeddedImage>(path, form);
    auto* embed = m_map_embedded[path.filename().as_str()].get();
    embed->SetEmbedSize(image);

    wxMemoryOutputStream memory_stream;
    wxZlibOutputStream save_strem(memory_stream, wxZ_BEST_COMPRESSION);
    stream.SeekI(0);

    if (!CopyStreamData(&stream, &save_strem, stream.GetLength()))
    {
        // TODO: [KeyWorks - 03-16-2022] This would be really bad, though it should be impossible
        return false;
    }
    save_strem.Close();
    auto compressed_size = memory_stream.TellO();

    auto* read_stream = memory_stream.GetOutputStreamBuffer();
    embed->base_image().type = wxBITMAP_TYPE_XPM;
    embed->base_image().array_size = (compressed_size | (org_size << 32));
    embed->base_image().array_data.resize(compressed_size);
    memcpy(embed->base_image().array_data.data(), read_stream->GetBufferStart(), compressed_size);

    return true;
}

tt_string ImageHandler::GetBundleFuncName(const tt_string& description)
{
    tt_string name;

    for (const auto& form: Project.get_ChildNodePtrs())
    {
        if (form->is_Gen(gen_Images))
        {
            tt_string_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts.size() < 2)
            {
                // caller's description does not include a filename
                return name;
            }

            for (const auto& child: form->get_ChildNodePtrs())
            {
                tt_view_vector form_image_parts(child->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                                tt::TRIM::both);
                if (form_image_parts.size() < 2)
                {
                    continue;
                }

                if (parts[0] == form_image_parts[0] &&
                    parts[1].filename() == form_image_parts[1].filename())
                {
                    auto* embed = GetEmbeddedImage(parts[IndexImage]);
                    // REVIEW: [Randalphwa - 09-16-2025] This can occur when the art directory is
                    // changed.
                    ASSERT(embed);
                    if (!embed)
                    {
                        return name;
                    }
                    if (embed->base_image().type == wxBITMAP_TYPE_SVG)
                    {
                        name << "wxue_img::bundle_" << embed->base_image().array_name << "(";

                        wxSize svg_size { -1, -1 };
                        if (parts[IndexSize].size())
                        {
                            svg_size = GetSizeInfo(parts[IndexSize]);
                        }
                        name << svg_size.x << ", " << svg_size.y << ")";
                    }
                    else
                    {
                        name << "wxue_img::bundle_" << embed->base_image().array_name << "()";
                    }
                    break;
                }
            }
            break;
        }
    }

    return name;
}

tt_string ImageHandler::GetBundleFuncName(const tt_string_vector* parts)
{
    tt_string name;

    for (const auto& form: Project.get_ChildNodePtrs())
    {
        if (form->is_Gen(gen_Images))
        {
            for (const auto& child: form->get_ChildNodePtrs())
            {
                tt_string_vector form_image_parts(child->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                                  tt::TRIM::both);
                if (form_image_parts.size() < 2)
                {
                    continue;
                }

                if ((*parts)[0] == form_image_parts[0] &&
                    (*parts)[1].filename() == form_image_parts[1].filename())
                {
                    if (const auto* bundle = GetPropertyImageBundle(parts);
                        bundle && bundle->lst_filenames.size())
                    {
                        auto* embed = GetEmbeddedImage(bundle->lst_filenames[0]);
                        if (embed->base_image().type == wxBITMAP_TYPE_SVG)
                        {
                            name << "wxue_img::bundle_" << embed->base_image().array_name << "(";

                            wxSize svg_size { -1, -1 };
                            if ((*parts)[IndexSize].size())
                            {
                                svg_size = GetSizeInfo((*parts)[IndexSize]);
                            }
                            name << svg_size.x << ", " << svg_size.y << ")";
                        }
                        else
                        {
                            name << "wxue_img::bundle_" << embed->base_image().array_name << "()";
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

auto ImageHandler::GetBundleFuncName(const EmbeddedImage* embed, wxSize svg_size) -> tt_string
{
    tt_string name;
    if (!embed || embed->get_Form() != Project.get_ImagesForm())
    {
        return name;
    }
    if (embed->base_image().type == wxBITMAP_TYPE_SVG)
    {
        name << "wxue_img::bundle_" << embed->base_image().array_name << "(";
        name << svg_size.x << ", " << svg_size.y << ")";
    }
    else
    {
        name << "wxue_img::bundle_" << embed->base_image().array_name << "()";
    }
    return name;
}

auto ImageHandler::ArtFolderChanged() -> bool
{
    wxFileName path;
    path.Assign(Project.as_string(prop_art_directory));
    if (!path.DirExists())
    {
        wxMessageDialog(wxGetMainFrame()->getWindow(),
                        wxString("The specified Art Directory does not exist:\n")
                            << Project.as_string(prop_art_directory),
                        "Art Directory Not Found", wxOK | wxICON_ERROR)
            .ShowModal();

        // If the directory doesn't exist, then we need to reset it. Otherwise on Windows, the
        // dialog will be for the computer, requiring the user to drill down to where the project
        // file is.
        path = *Project.get_wxFileName();
        path.SetFullName(wxEmptyString);  // clear the project filename

        wxDirDialog dlg(wxGetMainFrame(), wxDirSelectorPromptStr, path.GetPath(),
                        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            path = dlg.GetPath();
            path.MakeRelativeTo(Project.get_ProjectPath());
            m_project_node->set_value(prop_art_directory, path.GetPath());
            return true;
        }
        return false;
    }
    return true;
}

namespace wxue_img
{
    inline const unsigned char pulsing_unknown_gif[377] {
        71,  73,  70,  56,  57,  97,  15,  0,   20,  0,   196, 0,   0,   255, 255, 255, 253, 124,
        134, 253, 118, 129, 253, 115, 126, 252, 108, 120, 252, 105, 117, 255, 102, 102, 251, 100,
        113, 250, 87,  101, 250, 84,  98,  249, 77,  91,  249, 71,  86,  248, 67,  82,  248, 62,
        77,  248, 58,  74,  247, 48,  65,  246, 41,  59,  246, 36,  54,  245, 33,  50,  238, 29,
        47,  230, 28,  45,  222, 27,  43,  214, 26,  42,  206, 25,  40,  198, 24,  39,  189, 23,
        37,  172, 21,  34,  159, 19,  31,  148, 18,  29,  140, 17,  27,  132, 16,  26,  125, 15,
        24,  33,  255, 11,  78,  69,  84,  83,  67,  65,  80,  69,  50,  46,  48,  3,   1,   0,
        0,   0,   33,  249, 4,   9,   40,  0,   0,   0,   44,  0,   0,   0,   0,   15,  0,   20,
        0,   0,   5,   80,  32,  32,  142, 100, 105, 158, 104, 74,  6,   3,   65,  28,  10,  250,
        54,  208, 211, 56,  209, 121, 60,  90,  167, 85,  145, 201, 165, 164, 120, 100, 62,  34,
        137, 228, 178, 41,  93,  56,  31,  36,  64,  65,  129, 170, 8,   140, 141, 84,  150, 192,
        108, 79,  9,   198, 229, 123, 154, 100, 58,  100, 149, 186, 80,  88,  80,  212, 140, 71,
        69,  163, 206, 120, 162, 234, 188, 126, 207, 7,   132, 0,   0,   33,  249, 4,   9,   40,
        0,   0,   0,   44,  0,   0,   0,   0,   15,  0,   20,  0,   0,   5,   126, 32,  32,  2,
        193, 64,  16,  7,   50,  174, 129, 112, 40,  76,  204, 44,  204, 138, 46,  15,  117, 93,
        149, 244, 60,  145, 209, 129, 33,  185, 112, 62,  31,  77,  47,  50,  185, 0,   16,  138,
        71,  101, 243, 25,  73,  34,  146, 138, 6,   48,  169, 96,  52,  213, 81,  116, 130, 233,
        136, 142, 200, 209, 0,   1,   49,  134, 87,  34,  151, 195, 210, 121, 195, 229, 116, 251,
        10,  95,  135, 11,  15,  13,  20,  84,  126, 35,  14,  77,  96,  132, 35,  90,  28,  30,
        122, 137, 143, 35,  5,   5,   8,   11,  20,  144, 8,   9,   12,  16,  25,  144, 12,  15,
        18,  22,  27,  144, 16,  94,  26,  30,  144, 25,  141, 105, 144, 144, 33,  0,   59
    };

}
