/////////////////////////////////////////////////////////////////////////////
// Purpose:   Images List Embedded images generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>     // Base header for wxPanel
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/stattext.h>  // wxStaticText base header

#include "gen_images_list.h"

#include "bitmaps.h"          // Contains various images handling functions
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "image_handler.h"    // ImageHandler class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // Write code to Scintilla or file

#include "ui_images.h"

using namespace wxue_img;

#include "../mockup/mockup_content.h"  // MockupContent -- Mockup of a form's contents
#include "../mockup/mockup_parent.h"   // MockupParent -- Top-level MockUp Parent window

//////////////////////////////////////////  ImagesGenerator  //////////////////////////////////////////

wxObject* ImagesGenerator::CreateMockup(Node* /* node */, wxObject* wxobject)
{
    auto parent = wxStaticCast(wxobject, wxWindow);

    m_image_name = new wxStaticText(parent, wxID_ANY, "Select an image to display it below.");
    m_text_info = new wxStaticText(parent, wxID_ANY, wxEmptyString);
    m_bitmap = new wxStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY,
                                  wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));

    auto node = wxGetFrame().getSelectedNode();
    if (node->isGen(gen_embedded_image))
    {
        auto bundle = ProjectImages.GetPropertyImageBundle(node->as_string(prop_bitmap));

        tt_view_vector mstr(node->as_string(prop_bitmap), ';');

        if (mstr.size() > 1)
        {
            if (bundle && bundle->lst_filenames.size())
            {
                tt_string list;
                for (auto& iter: bundle->lst_filenames)
                {
                    if (list.size())
                    {
                        list << '\n';
                    }
                    list << iter;
                }
                m_image_name->SetLabel(list.make_wxString());
            }
            else
            {
                m_image_name->SetLabel(mstr[1].make_wxString());
            }
        }
        else
        {
            m_image_name->SetLabel(wxEmptyString);
        }

        auto bmp = node->as_wxBitmapBundle(prop_bitmap);
        ASSERT_MSG(bmp.IsOk(), tt_string("as_wxBitmapBundle(\"") << node->as_string(prop_bitmap) << "\") failed");
        if (!bmp.IsOk())
        {
            m_text_info->SetLabel("Cannot locate image!");
            m_bitmap->SetBitmap(wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));
        }
        else
        {
            m_bitmap->SetBitmap(bmp);
            auto default_size = bmp.GetDefaultSize();

            tt_string info("Default wxSize: ");
            info << default_size.GetWidth() << " x " << default_size.GetHeight();
            m_text_info->SetLabel(info);
        }
    }

    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_image_name, wxSizerFlags(0).Border(wxALL).Expand());
    sizer->Add(m_text_info, wxSizerFlags(0).Border(wxALL).Expand());
    sizer->Add(m_bitmap, wxSizerFlags(1).Border(wxALL).Expand());

    return sizer;
}

//////////////////////////////////////////  Code Generator for Images  //////////////////////////////////////////

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

void BaseCodeGenerator::GenerateImagesForm()
{
    ASSERT_MSG(m_form_node, "Attempting to generate Images List when no form was located.");

    if (m_embedded_images.empty() || !m_form_node->getChildCount())
    {
        return;
    }

    bool is_old_widgets = (Project.is_wxWidgets31());

    if (m_panel_type != HDR_PANEL)
    {
        m_source->writeLine("#include <wx/mstream.h>  // memory stream classes", indent::none);

        if (m_NeedSVGFunction || Project.getForm_BundleSVG() == m_form_node)
        {
            m_source->writeLine("#include <wx/zstream.h>  // zlib stream classes", indent::none);

            m_source->writeLine();
            m_source->writeLine("#include <memory>  // for std::make_unique", indent::none);
        }

        m_source->writeLine();
        m_source->writeLine("namespace wxue_img\n{");
        m_source->Indent();
        m_source->SetLastLineBlank();
        {
            tt_string_vector function;
            function.ReadString(txt_get_image_function);
            for (auto& iter: function)
            {
                m_source->writeLine(iter, indent::none);
            }

            if (is_old_widgets)
            {
                m_source->writeLine();
                m_source->writeLine("#if wxCHECK_VERSION(3, 1, 6)", indent::none);
            }

            function.clear();
            function.ReadString(txt_get_bundle_svg_function);
            for (auto& iter: function)
            {
                m_source->writeLine(iter, indent::none);
            }

            if (is_old_widgets)
            {
                m_source->writeLine("#endif // wxCHECK_VERSION(3, 1, 6)", indent::none);
            }
        }

        // Write all of the image data followed by the functions to access them.

        for (auto iter_array: m_embedded_images)
        {
            if (iter_array->form != m_form_node)
                continue;

            m_source->writeLine();
            tt_string code;
            code.reserve(Project.as_size_t(prop_cpp_line_length) + 16);
            // SVG images store the original size in the high 32 bits
            size_t max_pos = (iter_array->array_size & 0xFFFFFFFF);
            code << "const unsigned char " << iter_array->array_name << '[' << max_pos << "] {";
            m_source->writeLine(code);

            size_t pos = 0;
            while (pos < max_pos)
            {
                code.clear();
                // -8 to account for 4 indent + max 3 chars for number + comma
                for (; pos < max_pos && code.size() < Project.as_size_t(prop_cpp_line_length) - 8; ++pos)
                {
                    code << (to_int) iter_array->array_data[pos] << ',';
                }
                if (pos >= max_pos && code.back() == ',')
                    code.pop_back();
                m_source->writeLine(code);
            }
            m_source->writeLine("};");
        }

        m_source->writeLine();

        if (is_old_widgets)
        {
            m_source->writeLine("#if wxCHECK_VERSION(3, 1, 6)", indent::none);
            m_source->SetLastLineBlank();
        }

        for (const auto& child: m_form_node->getChildNodePtrs())
        {
            if (auto bundle = ProjectImages.GetPropertyImageBundle(child->as_string(prop_bitmap));
                bundle && bundle->lst_filenames.size())
            {
                auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                if (embed->type == wxBITMAP_TYPE_ICO || embed->type == wxBITMAP_TYPE_CUR ||
                    embed->type == wxBITMAP_TYPE_GIF || embed->type == wxBITMAP_TYPE_ANI)
                {
                    // Ignore types that can't be placed into a bundle. Technically, a .gif
                    // file could be added to a bundle, but use of .git instead of .png
                    // would be highly unusual. A more common scenario is that any .git
                    // file is being used for an animation control, which doesn't use a
                    // bundle.
                    continue;
                }

                m_source->writeLine();
                tt_string code("wxBitmapBundle bundle_");

                if (embed->type == wxBITMAP_TYPE_INVALID)
                {
                    code << embed->array_name << "(int width, int height)";
                    m_source->writeLine(code);
                    m_source->writeLine("{");
                    m_source->Indent();
                    code = "return get_bundle_svg(";
                    code << embed->array_name << ", " << (embed->array_size & 0xFFFFFFFF) << ", ";
                    code << (embed->array_size >> (to_size_t) 32) << ", wxSize(width, height));";
                    m_source->writeLine(code);
                    m_source->Unindent();
                    m_source->writeLine("}");
                    m_source->writeLine();
                }
                else
                {
                    code << embed->array_name << "()";
                    m_source->writeLine(code);
                    m_source->writeLine("{");
                    m_source->Indent();
                    if (bundle->lst_filenames.size() == 1)
                    {
                        code = "return wxBitmapBundle::FromBitmap(wxBitmap(get_image(";
                        code << embed->array_name << ", " << embed->array_size << ")));";
                        m_source->writeLine(code);
                    }
                    else
                    {
                        code = "wxVector<wxBitmap> bitmaps;\n";
                        for (auto& iter: bundle->lst_filenames)
                        {
                            // tt_string name(iter.filename());
                            // name.remove_extension();
                            // name.Replace(".", "_", true);  // fix wxFormBuilder header files
                            embed = ProjectImages.GetEmbeddedImage(iter);
                            ASSERT_MSG(embed, tt_string("Unable to locate embedded image for ") << iter);
                            if (embed)
                            {
                                code << "\t\tbitmaps.push_back(get_image(" << embed->array_name << ", sizeof("
                                     << embed->array_name << ")));\n";
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

        if (is_old_widgets)
        {
            m_source->writeLine("#endif  // wxCHECK_VERSION(3, 1, 6)", indent::none);
        }

        for (auto embed: m_embedded_images)
        {
            // Unlike the wxBitmapBundle functions above, the wxImage functions work on a much wider variety of
            // images, including ICO, CUR, and GIT. The only types that don't work are .svg and .ani.
            if (embed->type == wxBITMAP_TYPE_INVALID || embed->type == wxBITMAP_TYPE_SVG || embed->type == wxBITMAP_TYPE_ANI)
                continue;

            m_source->writeLine();
            tt_string code("wxImage image_");
            code << embed->array_name << "()";
            m_source->writeLine(code);
            m_source->writeLine("{");
            m_source->Indent();
            code = "return get_image(";
            code << embed->array_name << ", " << embed->array_size << ");";
            m_source->writeLine(code);
            m_source->Unindent();
            m_source->writeLine("}");
        }

        m_source->Unindent();
        m_source->writeLine("}\n");
    }

    /////////////// Header code ///////////////

    if (m_panel_type != CPP_PANEL)
    {
        if (m_NeedSVGFunction && is_old_widgets)
        {
            m_source->writeLine();
            m_header->writeLine("#if !wxCHECK_VERSION(3, 1, 6)", indent::none);
            m_header->Indent();
            m_header->writeLine("#error \"You must build with wxWidgets 3.1.6 or later to use SVG images.\"");
            m_header->Unindent();
            m_header->writeLine("#endif", indent::none);
        }
        else if (!m_NeedSVGFunction && is_old_widgets)
        {
            tt_string code("#if wxCHECK_VERSION(3, 1, 6)\n\t");
            code << "#include <wx/bmpbndl.h>";
            code << "\n#else\n\t";
            code << "#include <wx/image.h>";
            code << "\n#endif";
            m_header->writeLine(code, indent::auto_keep_whitespace);
        }
        else
        {
            m_header->writeLine("#include <wx/bmpbndl.h>");
        }

        m_header->writeLine();
        m_header->writeLine("namespace wxue_img\n{");
        m_header->Indent();
        m_header->SetLastLineBlank();
        m_header->writeLine("wxImage get_image(const unsigned char* data, size_t size_data);");

        m_header->writeLine();
        if (is_old_widgets)
        {
            m_header->writeLine("#if wxCHECK_VERSION(3, 1, 6)", indent::none);
            m_header->writeLine("wxBitmapBundle get_bundle_svg(const unsigned char* data, size_t size_data, size_t "
                                "size_svg, wxSize def_size);");
        }

        for (const auto& child: m_form_node->getChildNodePtrs())
        {
            if (auto bundle = ProjectImages.GetPropertyImageBundle(child->as_string(prop_bitmap));
                bundle && bundle->lst_filenames.size())
            {
                auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                if (embed->type == wxBITMAP_TYPE_ICO || embed->type == wxBITMAP_TYPE_CUR ||
                    embed->type == wxBITMAP_TYPE_GIF || embed->type == wxBITMAP_TYPE_ANI)
                {
                    // Don't generate bundle functions for image types that are probably being used for something else
                    continue;
                }

                tt_string code("wxBitmapBundle bundle_");
                if (embed->type == wxBITMAP_TYPE_INVALID)
                {
                    code << embed->array_name << "(int width, int height);";
                }
                else
                {
                    code << embed->array_name << "();";
                    if (bundle->lst_filenames[0].size())
                    {
                        code << "  // " << bundle->lst_filenames[0].filename();
                    }
                }
                m_header->writeLine(code);
            }
        }

        if (is_old_widgets)
        {
            m_header->writeLine("#endif", indent::none);
        }

        m_header->writeLine();
        for (auto embed: m_embedded_images)
        {
            // Unlike the wxBitmapBundle functions above, the wxImage functions work on a much wider variety of
            // images, including ICO, CUR, and GIT. The only types that don't work are .svg and .ani.
            if (embed->type == wxBITMAP_TYPE_INVALID || embed->type == wxBITMAP_TYPE_SVG || embed->type == wxBITMAP_TYPE_ANI)
                continue;
            tt_string code("wxImage image_");
            code << embed->array_name << "();";
            m_header->writeLine(code);
        }
    }

    if (m_form_node->as_bool(prop_add_externs))
    {
        m_header->writeLine();
        for (auto iter_array: m_embedded_images)
        {
            if (iter_array->form != m_form_node)
                continue;

            tt_string line("extern const unsigned char ");
            line << iter_array->array_name << '[' << (iter_array->array_size & 0xFFFFFFFF) << "];";
            if (iter_array->filename.size())
            {
                line << "  // " << iter_array->filename;
            }
            m_header->writeLine(line);
        }
    }

    m_header->Unindent();
    m_header->writeLine("}\n");
}

int ImagesGenerator::GetRequiredVersion(Node* node)
{
    if (node->hasValue(prop_auto_update))
    {
        return minRequiredVer + 2;  // 1.1.1 release
    }

    return minRequiredVer;
}

// Called by MainFrame when the user modifies a property. Return true if the generator handles
// pushing to the undo stack.
bool ImagesGenerator::modifyProperty(NodeProperty* prop, tt_string_view value)
{
    if (prop->isProp(prop_auto_update) && value != "0")
    {
        auto undo_update_images = std::make_shared<AutoImagesAction>(prop->getNode());
        wxGetFrame().PushUndoAction(undo_update_images);
        return true;
    }
    else
    {
        return false;
    }
}

//////////////////////////////////////////  Image List Functions  //////////////////////////////////////////

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

void img_list::GatherImages(Node* parent, std::set<std::string>& images, std::vector<std::string>& new_images)
{
    if (parent->isGen(gen_Images))
    {
        return;
    }

    for (const auto& child: parent->getChildNodePtrs())
    {
        for (auto& iter: lstBitmapoProps)
        {
            auto prop_ptr = child->getPropPtr(iter);
            if (prop_ptr && prop_ptr->hasValue())
            {
                if (images.contains(prop_ptr->value()))
                {
                    continue;
                }
                auto& description = prop_ptr->value();
                // We need the size for bundle processing, but we don't need every possible size added
                // to gen_Images, so we simply force it to be 16x16 to avoid duplication.
                if (description.starts_with("SVG;"))
                {
                    tt_string new_description(description);
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
        if (child->getChildCount())
        {
            GatherImages(child.get(), images, new_images);
        }
    }
}

void img_list::FixPropBitmap(Node* parent)
{
    tt_cwd cwd(tt_cwd::restore);
    Project.ChangeDir();

    tt_string art_directory = Project.as_string(prop_art_directory);

    for (const auto& child: parent->getChildNodePtrs())
    {
        for (auto& iter: lstBitmapoProps)
        {
            auto prop_ptr = child->getPropPtr(iter);
            if (prop_ptr && prop_ptr->hasValue())
            {
                auto& description = prop_ptr->value();
                if (description.starts_with("Embed") || description.starts_with("SVG") || description.starts_with("XPM"))
                {
                    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);
                    if (parts.size() > IndexImage && parts[IndexImage].size())
                    {
                        tt_string check_path(art_directory);
                        tt_string file_part = parts[IndexImage].filename();
                        check_path.append_filename(file_part);
                        if (check_path.file_exists() && file_part != parts[IndexImage])
                        {
                            tt_string new_description;
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
        if (child->getChildCount())
        {
            FixPropBitmap(child.get());
        }
    }
}

bool img_list::CompareImageNames(NodeSharedPtr a, NodeSharedPtr b)
{
    auto& description_a = a->as_string(prop_bitmap);
    tt_view_vector parts_a(description_a, BMP_PROP_SEPARATOR, tt::TRIM::both);
    if (parts_a.size() <= IndexImage || parts_a[IndexImage].empty())
        return true;

    auto& description_b = b->as_string(prop_bitmap);
    tt_view_vector parts_b(description_b, BMP_PROP_SEPARATOR, tt::TRIM::both);
    if (parts_b.size() <= IndexImage || parts_b[IndexImage].empty())
        return false;

    return (parts_a[IndexImage].filename().compare(parts_b[IndexImage].filename()) < 0);
}

Node* img_list::FindImageList()
{
    Node* image_node = nullptr;
    if (Project.getChildCount() > 0)
    {
        if (Project.getChild(0)->isGen(gen_Images))
        {
            image_node = Project.getChild(0);
        }
        else
        {
            for (const auto& iter: Project.getChildNodePtrs())
            {
                if (iter->isGen(gen_Images))
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
            Project.getProjectNode()->set_value(prop_art_directory, "./");
        }

        FixPropBitmap(Project.getProjectNode());
    }

    Node* image_node = FindImageList();
    if (!image_node)
    {
        return;
    }

    if (Project.getProjectNode()->getChildCount() != 0)
    {
        Project.getProjectNode()->changeChildPosition(image_node->getSharedPtr(), 0);
    }

    auto& children = image_node->getChildNodePtrs();
    std::sort(children.begin(), children.end(), img_list::CompareImageNames);
}
