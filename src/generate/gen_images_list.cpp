/////////////////////////////////////////////////////////////////////////////
// Purpose:   Images List Embedded images generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>              // Base header for wxPanel
#include <wx/progdlg.h>            // wxProgressDialog
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid
#include <wx/sizer.h>              // provide wxSizer class for layout
#include <wx/statbmp.h>            // wxStaticBitmap class interface
#include <wx/stattext.h>           // wxStaticText base header

#include "gen_images_list.h"

#include "bitmaps.h"          // Contains various images handling functions
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "image_handler.h"    // ImageHandler class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // Write code to Scintilla or file
#include "wxue_namespace/wxue_string.h"         // wxue::SaveCwd
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector
#include "wxue_namespace/wxue_view_vector.h"    // wxue::ViewVector

#include "ui_images.h"

using namespace wxue_img;

#include "../mockup/mockup_content.h"  // MockupContent -- Mockup of a form's contents
#include "../mockup/mockup_parent.h"   // MockupParent -- Top-level MockUp Parent window

///////////////////////////////////  ImagesGenerator //////////////////////////////////////

auto ImagesGenerator::CreateMockup(Node* /* node */, wxObject* wxobject) -> wxObject*
{
    auto* parent = wxStaticCast(wxobject, wxWindow);

    m_image_name = new wxStaticText(parent, wxID_ANY, "Select an image to display it below.");
    m_text_info = new wxStaticText(parent, wxID_ANY, wxEmptyString);
    m_bitmap = new wxStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY,
                                  wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));

    auto* node = wxGetFrame().getSelectedNode();
    if (node->is_Gen(gen_embedded_image))
    {
        const auto* bundle = ProjectImages.GetPropertyImageBundle(node->as_string(prop_bitmap));

        wxue::ViewVector mstr(std::string_view(node->as_string(prop_bitmap)), ';');

        if (mstr.size() > 1)
        {
            if (bundle && bundle->lst_filenames.size())
            {
                wxue::string list;
                for (const auto& iter: bundle->lst_filenames)
                {
                    if (list.size())
                    {
                        list << '\n';
                    }
                    list << iter;
                }
                m_image_name->SetLabel(list.wx());
            }
            else
            {
                m_image_name->SetLabel(mstr[1].wx());
            }
        }
        else
        {
            m_image_name->SetLabel(wxEmptyString);
        }

        auto bmp = node->as_wxBitmapBundle(prop_bitmap);
        ASSERT_MSG(bmp.IsOk(), wxue::string("as_wxBitmapBundle(\"")
                                   << node->as_string(prop_bitmap) << "\") failed");
        if (!bmp.IsOk())
        {
            m_text_info->SetLabel("Cannot locate image!");
            m_bitmap->SetBitmap(wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));
        }
        else
        {
            m_bitmap->SetBitmap(bmp);
            auto default_size = bmp.GetDefaultSize();

            wxue::string info("Default wxSize: ");
            info << default_size.GetWidth() << " x " << default_size.GetHeight();
            m_text_info->SetLabel(info);
        }
    }

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_image_name, wxSizerFlags(0).Border(wxALL).Expand());
    sizer->Add(m_text_info, wxSizerFlags(0).Border(wxALL).Expand());
    sizer->Add(m_bitmap, wxSizerFlags(1).Border(wxALL).Expand());

    return sizer;
}

auto EmbeddedImageGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop,
                                                 Node* node) -> bool
{
    if (prop->isProp(prop_bitmap))
    {
        wxue::string value = event->GetValue().GetString().utf8_string();
        if (value.empty() || value.starts_with("Art"))
        {
            return true;
        }

        wxue::StringVector parts_new(value, BMP_PROP_SEPARATOR, wxue::TRIM::both);
        if (parts_new.size() <= IndexImage || parts_new[IndexImage].empty())
        {
            return true;
        }
        wxue::StringVector parts_old(prop->as_string(), BMP_PROP_SEPARATOR, wxue::TRIM::both);
        if (parts_old.size() <= IndexImage || parts_old[IndexImage].empty())
        {
            return true;
        }

        // This happens for an SVG image where the size is being changed
        if (parts_old[IndexImage] == parts_new[IndexImage])
        {
            return true;
        }

        auto* embed = ProjectImages.GetEmbeddedImage(parts_new[IndexImage]);
        if (embed && embed->get_Form() == node->get_Parent())
        {
            event->SetValidationFailureMessage("You've already added this image!");
            event->Veto();
            return false;
        }
    }
    return true;
}

/////////////////////////////////  Code Generator for Images /////////////////////////////////////

// clang-format off

inline constexpr const auto txt_get_image_function = R"===(
    // Convert a data array into a wxImage
    wxImage get_image(const unsigned char* data, size_t size_data)
    {
        wxMemoryInputStream strm(data, size_data);
        wxImage image;
        image.LoadFile(strm);
        return image;
    };
)===";

inline constexpr const auto txt_get_bundle_svg_function = R"===(
    // Convert compressed SVG string into a wxBitmapBundle
    wxBitmapBundle get_bundle_svg(const unsigned char* data,
        size_t size_data, size_t size_svg, wxSize def_size)
    {
        auto str = std::make_unique<char[]>(size_svg);
        wxMemoryInputStream stream_in(data, size_data);
        wxZlibInputStream zlib_strm(stream_in);
        zlib_strm.Read(str.get(), size_svg);
        return wxBitmapBundle::FromSVG(str.get(), def_size);
    };
)===";

// clang-format on

namespace
{
    // Check if bitmap type should be excluded from bundle generation
    auto ShouldSkipBundleType(wxBitmapType type) -> bool
    {
        return type == wxBITMAP_TYPE_ICO || type == wxBITMAP_TYPE_CUR ||
               type == wxBITMAP_TYPE_GIF || type == wxBITMAP_TYPE_ANI;
    }
}  // namespace

void BaseCodeGenerator::GenerateImagesForm(wxProgressDialog* progress)
{
    ASSERT_MSG(m_form_node, "Attempting to generate Images List when no form was located.");

    if (m_embedded_images.empty() || !m_form_node->get_ChildCount())
    {
        return;
    }

    if (m_panel_type != PANEL_PAGE::HDR_INFO_PANEL)
    {
        m_source->writeLine("#include <wx/mstream.h>  // memory stream classes", indent::none);
        m_source->writeLine("#include <wx/zstream.h>  // zlib stream classes", indent::none);

        m_source->writeLine();
        m_source->writeLine("#include <memory>  // for std::make_unique", indent::none);

        m_source->writeLine();
        m_source->writeLine("namespace wxue_img\n{");
        m_source->Indent();
        m_source->SetLastLineBlank();
        {
            wxue::StringVector function;
            function.ReadString(std::string_view(txt_get_image_function));
            for (const auto& iter: function)
            {
                m_source->writeLine(iter, indent::none);
            }

            function.clear();
            function.ReadString(std::string_view(txt_get_bundle_svg_function));
            for (const auto& iter: function)
            {
                m_source->writeLine(iter, indent::none);
            }
        }

        // Write all of the image data followed by the functions to access them.

        int progress_count = 0;
        for (const auto* iter_array: m_embedded_images)
        {
            if (iter_array->get_Form() != m_form_node)
            {
                continue;
            }

            m_source->writeLine();
            wxue::string code;
            code.reserve(Project.as_size_t(prop_cpp_line_length) + 16);
            // SVG images store the original size in the high 32 bits
            size_t max_pos = (iter_array->base_image().array_size & 0xFFFFFFFF);
            code << "const unsigned char " << iter_array->base_image().array_name << '[' << max_pos
                 << "] {";
            m_source->writeLine(code);
            if (progress && ++progress_count % result::progress_image_step == 0)
            {
                wxString msg;
                msg << "Generating embedded images: " << progress_count << " of "
                    << (m_embedded_images.size());
                progress->Update(progress->GetValue() + 1, msg);
                progress_count = 0;
            }

            size_t pos = 0;
            while (pos < max_pos)
            {
                code.clear();
                // -8 to account for 4 indent + max 3 chars for number + comma
                for (; pos < max_pos && code.size() < Project.as_size_t(prop_cpp_line_length) - 8;
                     ++pos)
                {
                    code << (to_int) iter_array->base_image().array_data[pos] << ',';
                }
                if (pos >= max_pos && code.back() == ',')
                {
                    code.pop_back();
                }
                m_source->writeLine(code);
            }
            m_source->writeLine("};");
        }

        m_source->writeLine();

        for (const auto& child: m_form_node->get_ChildNodePtrs())
        {
            if (const auto* bundle =
                    ProjectImages.GetPropertyImageBundle(child->as_string(prop_bitmap));
                bundle && bundle->lst_filenames.size())
            {
                auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                if (ShouldSkipBundleType(embed->base_image().type))
                {
                    // Ignore types that can't be placed into a bundle. Technically, a .gif
                    // file could be added to a bundle, but use of .git instead of .png
                    // would be highly unusual. A more common scenario is that any .git
                    // file is being used for an animation control, which doesn't use a
                    // bundle.
                    continue;
                }

                m_source->writeLine();
                wxue::string code("wxBitmapBundle bundle_");

                if (embed->base_image().type == wxBITMAP_TYPE_SVG)
                {
                    code << embed->base_image().array_name << "(int width, int height)";
                    m_source->writeLine(code);
                    m_source->writeLine("{");
                    m_source->Indent();
                    code = "return get_bundle_svg(";
                    code << embed->base_image().array_name << ", "
                         << static_cast<size_t>(embed->base_image().array_size &
                                                0x00000000FFFFFFFFULL)
                         << ", ";
                    code << (embed->base_image().array_size >> (to_size_t) 32)
                         << ", wxSize(width, height));";
                    m_source->writeLine(code);
                    m_source->Unindent();
                    m_source->writeLine("}");
                    m_source->writeLine();
                }
                else
                {
                    code << embed->base_image().array_name << "()";
                    m_source->writeLine(code);
                    m_source->writeLine("{");
                    m_source->Indent();
                    if (bundle->lst_filenames.size() == 1)
                    {
                        code = "return wxBitmapBundle::FromBitmap(wxBitmap(get_image(";
                        code << embed->base_image().array_name << ", "
                             << embed->base_image().array_size << ")));";
                        m_source->writeLine(code);
                    }
                    else
                    {
                        code = "wxVector<wxBitmap> bitmaps;\n";
                        for (const auto& iter: bundle->lst_filenames)
                        {
                            // wxue::string name(iter.filename());
                            // name.remove_extension();
                            // name.Replace(".", "_", true);  // fix wxFormBuilder header files
                            embed = ProjectImages.GetEmbeddedImage(iter);
                            ASSERT_MSG(embed, wxue::string("Unable to locate embedded image for ")
                                                  << iter);
                            if (embed)
                            {
                                code << "\t\tbitmaps.push_back(get_image("
                                     << embed->base_image().array_name << ", sizeof("
                                     << embed->base_image().array_name << ")));\n";
                            }
                        }
                        code += "return wxBitmapBundle::FromBitmaps(bitmaps);";
                        m_source->writeLine(code);
                    }
                    m_source->Unindent();  // end function block
                    m_source->writeLine("}");
                }
            }
        }

        for (const auto* embed: m_embedded_images)
        {
            // Unlike the wxBitmapBundle functions above, the wxImage functions work on a much wider
            // variety of images, including ICO, CUR, and GIT. The only types that don't work are
            // .svg and .ani.
            if (embed->base_image().type == wxBITMAP_TYPE_SVG ||
                embed->base_image().type == wxBITMAP_TYPE_ANI)
            {
                continue;
            }

            m_source->writeLine();
            wxue::string code("wxImage image_");
            code << embed->base_image().array_name << "()";
            m_source->writeLine(code);
            m_source->writeLine("{");
            m_source->Indent();
            code = "return get_image(";
            code << embed->base_image().array_name << ", " << embed->base_image().array_size
                 << ");";
            m_source->writeLine(code);
            m_source->Unindent();
            m_source->writeLine("}");
        }

        m_source->Unindent();
        m_source->writeLine("}\n");
    }

    /////////////// Header code ///////////////

    if (m_panel_type != PANEL_PAGE::SOURCE_PANEL)
    {
        m_header->writeLine("#include <wx/bmpbndl.h>");

        m_header->writeLine();
        m_header->writeLine("namespace wxue_img\n{");
        m_header->Indent();
        m_header->SetLastLineBlank();
        m_header->writeLine("wxImage get_image(const unsigned char* data, size_t size_data);");

        m_header->writeLine();

        for (const auto& child: m_form_node->get_ChildNodePtrs())
        {
            if (const auto* bundle =
                    ProjectImages.GetPropertyImageBundle(child->as_string(prop_bitmap));
                bundle && bundle->lst_filenames.size())
            {
                auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                if (ShouldSkipBundleType(embed->base_image().type))
                {
                    // Don't generate bundle functions for image types that are probably being used
                    // for something else
                    continue;
                }

                wxue::string code("wxBitmapBundle bundle_");
                if (embed->base_image().type == wxBITMAP_TYPE_SVG)
                {
                    code << embed->base_image().array_name << "(int width, int height);";
                }
                else
                {
                    code << embed->base_image().array_name << "();";
                    if (bundle->lst_filenames[0].size())
                    {
                        code << "  // " << bundle->lst_filenames[0].filename();
                    }
                }
                m_header->writeLine(code);
            }
        }

        m_header->writeLine();
        for (const auto* embed: m_embedded_images)
        {
            // Unlike the wxBitmapBundle functions above, the wxImage functions work on a much wider
            // variety of images, including ICO, CUR, and GIT. The only types that don't work are
            // .svg and .ani.
            if (embed->base_image().type == wxBITMAP_TYPE_SVG ||
                embed->base_image().type == wxBITMAP_TYPE_ANI)
            {
                continue;
            }
            wxue::string code("wxImage image_");
            code << embed->base_image().array_name << "();";
            m_header->writeLine(code);
        }

        if (m_form_node->as_bool(prop_add_externs))
        {
            m_header->writeLine();
            for (const auto* iter_array: m_embedded_images)
            {
                if (iter_array->get_Form() != m_form_node)
                {
                    continue;
                }

                wxue::string line("extern const unsigned char ");
                line << iter_array->base_image().array_name << '['
                     << static_cast<size_t>(iter_array->base_image().array_size & 0xFFFFFFFF)
                     << "];";
                if (iter_array->base_image().filename.size())
                {
                    line << "  // " << iter_array->base_image().filename;
                }
                m_header->writeLine(line);
            }
        }

        m_header->Unindent();
        m_header->writeLine("}\n");
    }
}

auto ImagesGenerator::GetRequiredVersion(Node* node) -> int
{
    if (node->HasValue(prop_auto_add) && node->as_bool(prop_auto_add))
    {
        // return minRequiredVer + 5;  // 1.2.1 release
        return minRequiredVer + 4;  // 1.2.0 release
    }

    return minRequiredVer;
}

//////////////////////////////////////////  Image List Functions /////////////////////////////////

// clang-format off
inline const auto lstBitmapoProps = {
    prop_bitmap,
    prop_current,
    prop_disabled_bmp,
    prop_focus_bmp,
    prop_inactive_bitmap,
    prop_pressed_bmp,
};
// clang-format on

void img_list::GatherImages(Node* parent, std::set<std::string>& images,
                            std::vector<std::string>& new_images)
{
    if (parent->is_Gen(gen_Images))
    {
        return;
    }

    for (const auto& child: parent->get_ChildNodePtrs())
    {
        for (const auto& iter: lstBitmapoProps)
        {
            auto* prop_ptr = child->get_PropPtr(iter);
            if (prop_ptr && prop_ptr->HasValue())
            {
                if (images.contains(prop_ptr->value()))
                {
                    continue;
                }
                const auto& description = prop_ptr->value();
                // We need the size for bundle processing, but we don't need every possible size
                // added to gen_Images, so we simply force it to be 16x16 to avoid duplication.
                if (description.starts_with("SVG;"))
                {
                    wxue::string new_description(description);
                    new_description.erase(new_description.find_last_of(';'));
                    new_description << ";[16,16]";
                    if (!images.contains(new_description))
                    {
                        images.insert(new_description);
                        new_images.push_back(new_description);
                    }
                }
                else if (description.starts_with("Embed"))
                {
                    images.insert(description);
                    new_images.push_back(description);
                }
            }
        }
        if (child->get_ChildCount())
        {
            GatherImages(child.get(), images, new_images);
        }
    }
}

void img_list::FixPropBitmap(Node* parent)
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    Project.ChangeDir();

    const auto& art_directory = Project.as_string(prop_art_directory);

    for (const auto& child: parent->get_ChildNodePtrs())
    {
        for (const auto& iter: lstBitmapoProps)
        {
            auto* prop_ptr = child->get_PropPtr(iter);
            if (prop_ptr && prop_ptr->HasValue())
            {
                const auto& description = prop_ptr->value();
                if (description.starts_with("Embed") || description.starts_with("SVG") ||
                    description.starts_with("XPM"))
                {
                    wxue::ViewVector parts(description, BMP_PROP_SEPARATOR, wxue::TRIM::both);
                    if (parts.size() > IndexImage && parts[IndexImage].size())
                    {
                        wxue::string check_path(art_directory);
                        wxue::string file_part = parts[IndexImage].filename();
                        check_path.append_filename(file_part);
                        if (check_path.file_exists() && file_part != parts[IndexImage])
                        {
                            wxue::string new_description;
                            new_description << parts[IndexType] << BMP_PROP_SEPARATOR << file_part;
                            for (size_t idx = IndexImage + 1; idx < parts.size(); idx++)
                            {
                                new_description << BMP_PROP_SEPARATOR << parts[idx];
                            }

                            prop_ptr->set_value(new_description);
                        }
                    }
                }
            }
        }
        if (child->get_ChildCount())
        {
            FixPropBitmap(child.get());
        }
    }
}

auto img_list::CompareImageNames(const NodeSharedPtr& lhs, const NodeSharedPtr& rhs) -> bool
{
    const auto& description_a = lhs->as_string(prop_bitmap);
    wxue::ViewVector parts_a(description_a, BMP_PROP_SEPARATOR, wxue::TRIM::both);
    if (parts_a.size() <= IndexImage || parts_a[IndexImage].empty())
    {
        return true;
    }

    const auto& description_b = rhs->as_string(prop_bitmap);
    wxue::ViewVector parts_b(description_b, BMP_PROP_SEPARATOR, wxue::TRIM::both);
    if (parts_b.size() <= IndexImage || parts_b[IndexImage].empty())
    {
        return false;
    }

    return (parts_a[IndexImage].filename().compare(parts_b[IndexImage].filename()) < 0);
}

auto img_list::FindImageList() -> Node*
{
    Node* image_node = nullptr;
    if (Project.get_ChildCount() > 0)
    {
        if (Project.get_Child(0)->is_Gen(gen_Images))
        {
            image_node = Project.get_Child(0);
        }
        else
        {
            for (const auto& iter: Project.get_ChildNodePtrs())
            {
                if (iter->is_Gen(gen_Images))
                {
                    image_node = iter.get();
                    break;
                }
            }
        }
    }
    return image_node;
}

void img_list::UpdateImagesList(int ProjectVersion)
{
    if (ProjectVersion < curSupportedVer)
    {
        if (Project.as_string(prop_art_directory).empty())
        {
            Project.get_ProjectNode()->set_value(prop_art_directory, "./");
        }

        FixPropBitmap(Project.get_ProjectNode());
    }

    Node* image_node = FindImageList();
    if (!image_node)
    {
        return;
    }

    if (Project.get_ProjectNode()->get_ChildCount() != 0)
    {
        Project.get_ProjectNode()->ChangeChildPosition(image_node->get_SharedPtr(), 0);
    }

    auto& children = image_node->get_ChildNodePtrs();
    std::ranges::sort(children, img_list::CompareImageNames);
}
