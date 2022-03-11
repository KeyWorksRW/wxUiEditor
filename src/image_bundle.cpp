/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for working with wxBitmapBundle
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if wxCHECK_VERSION(3, 1, 6)
    #include <filesystem>
    #include <fstream>
    #include <thread>

    #include <wx/artprov.h>   // wxArtProvider class
    #include <wx/bmpbndl.h>   // includes wx/bitmap.h, wxBitmapBundle class interface
    #include <wx/mstream.h>   // Memory stream classes
    #include <wx/wfstream.h>  // File stream classes

    #include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

    #include "image_bundle.h"

    #include "mainapp.h"      // compiler_standard -- Main application class
    #include "node.h"         // Node class
    #include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
    #include "utils.h"        // Utility functions that work with properties

bool isConvertibleMime(const ttString& suffix);  // declared in embedimg.cpp

void ProjectSettings::ParseBundles()
{
    FinishThreads();

    m_collect_bundle_thread = new std::thread(&ProjectSettings::CollectBundles, this);
}

void ProjectSettings::CollectBundles()
{
    std::unique_lock<std::mutex> add_lock(m_mutex_init_bundles);

    // We need the shared ptr rather than the raw pointer because we can't let the pointer become invalid while we're still
    // processing nodes.
    auto project = wxGetApp().GetProjectPtr();

    for (size_t pos = 0; pos < project->GetChildCount(); ++pos)
    {
        if (m_is_terminating || m_cancel_collection)
            return;

        auto form = project->GetChildPtr(pos);

        for (auto& iter: form->GetChildNodePtrs())
        {
            if (m_is_terminating || m_cancel_collection)
                return;
            CollectNodeBundles(iter.get(), form.get());
        }
    }
}

void ProjectSettings::CollectNodeBundles(Node* node, Node* form)
{
    for (auto& iter: node->get_props_vector())
    {
        if (iter.type() == type_image || iter.type() == type_animation)
        {
            if (!iter.HasValue())
                continue;
            auto& value = iter.as_string();
            if (value.is_sameprefix("Embed"))
            {
                if (m_is_terminating || m_cancel_collection)
                    return;

                ttlib::multiview parts(value, BMP_PROP_SEPARATOR, tt::TRIM::both);
                if (parts[IndexImage].size())
                {
                    if (auto result = m_map_embedded.find(parts[IndexImage].filename()); result == m_map_embedded.end())
                    {
                        if (m_is_terminating)
                            return;

                        if (iter.type() == type_animation)
                        {
                            AddEmbeddedImage(parts[IndexImage], form);
                        }
                        else
                        {
                            AddNewEmbeddedBundle(value, parts[IndexImage], form);
                        }

                        if (m_is_terminating)
                            return;
                    }
                }
            }
        }
    }
}

void ProjectSettings::AddNewEmbeddedBundle(const ttlib::cstr& description, ttlib::cstr path, Node* form)
{
    ImageBundle img_bundle;

    if (!path.file_exists())
    {
        if (wxGetApp().GetProject()->HasValue(prop_art_directory))
        {
            ttlib::cstr art_path = wxGetApp().GetProject()->prop_as_string(prop_art_directory);
            art_path.append_filename(path);
            if (!art_path.file_exists())
            {
                m_bundles[description] = img_bundle;
                return;
            }
            path = std::move(art_path);
        }
        else
        {
            m_bundles[description] = img_bundle;
            return;
        }
    }

    if (!AddEmbeddedBundleImage(path, form))
    {
        m_bundles[description] = img_bundle;
        return;
    }

    img_bundle.lst_filenames.emplace_back(path);

    /*

        Look for suffix combinations -- it's fine if one of them doesn't exist

            _16x16, _24x24, _32x32
            _24x24, _36x36, _48x48
            any, _1_5x, _2x

    */

    if (auto pos = path.find_last_of('.'); ttlib::is_found(pos))
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
            path.insert(pos, "_1_5x");
            if (path.file_exists())
            {
                if (auto added = AddEmbeddedBundleImage(path, form); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
                }
            }
            path.Replace("_1_5x", "_2x");
            if (path.file_exists())
            {
                if (auto added = AddEmbeddedBundleImage(path, form); added)
                {
                    img_bundle.lst_filenames.emplace_back(path);
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

    m_bundles[description] = std::move(img_bundle);
}

bool ProjectSettings::AddEmbeddedBundleImage(ttlib::cstr path, Node* form)
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
                m_map_embedded[path.filename().c_str()] = std::make_unique<EmbededImage>();
                auto embed = m_map_embedded[path.filename().c_str()].get();
                embed->array_name = path.filename();
                embed->array_name.Replace(".", "_", true);
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
                    if (read_stream->GetBufferSize() <= static_cast<size_t>(stream.GetLength()))
                    {
                        embed->array_size = read_stream->GetBufferSize();
                        embed->array_data = std::make_unique<unsigned char[]>(embed->array_size);
                        memcpy(embed->array_data.get(), read_stream->GetBufferStart(), embed->array_size);
                    }
                    else
                    {
    #if defined(_DEBUG)
                        auto org_size = static_cast<size_t>(stream.GetLength());
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
    return false;
}

ImageBundle* ProjectSettings::ProcessBundleProperty(const ttlib::cstr& description, Node* node)
{
    ASSERT_MSG(m_bundles.find(description) == m_bundles.end(),
               "ProcessBundleProperty should not be called if bundle already exists!")

    ttlib::multistr parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty())
    {
        return nullptr;
    }

    ImageBundle img_bundle;

    if (parts[IndexType].contains("Art"))
    {
        if (parts[IndexArtID].contains("|"))
        {
            ttlib::multistr id_client(parts[IndexArtID], '|');
            img_bundle.bundle = wxArtProvider::GetBitmapBundle(id_client[0], wxART_MAKE_CLIENT_ID_FROM_STR(id_client[1]));
        }
        else
        {
            img_bundle.bundle =
                wxArtProvider::GetBitmapBundle(parts[IndexArtID].wx_str(), wxART_MAKE_CLIENT_ID_FROM_STR("wxART_OTHER"));
        }

        m_bundles[description] = std::move(img_bundle);
        return &m_bundles[description];
    }
    else if (parts[IndexType].contains("Embed"))
    {
        AddNewEmbeddedBundle(description, parts[IndexImage], node->GetForm());
        return &m_bundles[description];
    }

    auto image_first = wxGetApp().GetProjectSettings()->GetPropertyBitmap(description, false, false);
    if (!image_first.IsOk())
    {
        return nullptr;
    }

    img_bundle.lst_filenames.emplace_back(parts[IndexImage]);

    if (auto pos = parts[IndexImage].find_last_of('.'); ttlib::is_found(pos))
    {
        if (parts[IndexImage].contains("_16x16."))
        {
            ttlib::cstr path(parts[IndexImage]);
            path.Replace("_16x16.", "_24x24.");
            if (!path.file_exists())
            {
                if (wxGetApp().GetProjectPtr()->HasValue(prop_art_directory))
                {
                    path = wxGetApp().GetProjectPtr()->prop_as_string(prop_art_directory);
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
            ttlib::cstr path(parts[IndexImage]);
            path.Replace("_24x24.", "_36x36.");
            if (!path.file_exists())
            {
                if (wxGetApp().GetProjectPtr()->HasValue(prop_art_directory))
                {
                    path = wxGetApp().GetProjectPtr()->prop_as_string(prop_art_directory);
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
            ttlib::cstr path(parts[IndexImage]);
            path.insert(pos, "_1_5x");
            if (!path.file_exists())
            {
                if (wxGetApp().GetProjectPtr()->HasValue(prop_art_directory))
                {
                    ttlib::cstr tmp_path = wxGetApp().GetProjectPtr()->prop_as_string(prop_art_directory);
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

            path = parts[IndexImage];
            path.Replace("_1_5x", "_2x");
            if (!path.file_exists())
            {
                if (wxGetApp().GetProjectPtr()->HasValue(prop_art_directory))
                {
                    ttlib::cstr tmp_path = wxGetApp().GetProjectPtr()->prop_as_string(prop_art_directory);
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

    ASSERT_MSG(img_bundle.lst_filenames.size() > 0, "image_first must always have it's filename added.")

    if (img_bundle.lst_filenames.size() == 1)
    {
        img_bundle.bundle = wxBitmapBundle::FromBitmap(image_first);
    }
    else
    {
        wxVector<wxBitmap> bitmaps;
        bitmaps.push_back(image_first);
        ttlib::cstr new_description;
        new_description << parts[IndexType] << ';';
        new_description << img_bundle.lst_filenames[1];
        auto image_second = GetPropertyBitmap(new_description, false, false);
        if (image_second.IsOk())
        {
            bitmaps.push_back(image_second);
        }

        if (img_bundle.lst_filenames.size() > 2)
        {
            new_description.clear();
            new_description << parts[IndexType] << ';';
            new_description << img_bundle.lst_filenames[1];
            auto image_third = GetPropertyBitmap(new_description, false, false);
            if (image_third.IsOk())
            {
                bitmaps.push_back(image_third);
            }
        }

        img_bundle.bundle = wxBitmapBundle::FromBitmaps(bitmaps);
    }

    m_bundles[description] = std::move(img_bundle);
    return &m_bundles[description];
}

#endif
