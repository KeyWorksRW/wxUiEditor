/////////////////////////////////////////////////////////////////////////////
// Purpose:   Embedded images generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>     // Base header for wxPanel
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/stattext.h>  // wxStaticText base header

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings
#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "images_form.h"

#include "bitmaps.h"      // Contains various images handling functions
#include "gen_base.h"     // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainapp.h"      // compiler_standard -- Main application class
#include "mainframe.h"    // MainFrame -- Main window frame
#include "node.h"         // Node class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "write_code.h"   // Write code to Scintilla or file

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

    auto node = wxGetFrame().GetSelectedNode();
    if (node->isGen(gen_embedded_image))
    {
        auto bundle = wxGetApp().GetProjectSettings()->GetPropertyImageBundle(node->prop_as_string(prop_bitmap));

        ttlib::multiview mstr(node->prop_as_string(prop_bitmap), ';');

        if (mstr.size() > 1)
        {
            if (bundle && bundle->lst_filenames.size())
            {
                ttlib::cstr list;
                for (auto& iter: bundle->lst_filenames)
                {
                    if (list.size())
                    {
                        list << '\n';
                    }
                    list << iter;
                }
                m_image_name->SetLabel(list.wx_str());
            }
            else
            {
                m_image_name->SetLabel(mstr[1].wx_str());
            }
        }
        else
        {
            m_image_name->SetLabel(wxEmptyString);
        }

        auto bmp = node->prop_as_wxBitmapBundle(prop_bitmap);
        ASSERT(bmp.IsOk());
        if (!bmp.IsOk())
        {
            m_text_info->SetLabel("Cannot locate image!");
            m_bitmap->SetBitmap(wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));
        }
        else
        {
            m_bitmap->SetBitmap(bmp);
            auto default_size = bmp.GetDefaultSize();

            ttlib::cstr info("Default wxSize: ");
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

// clang-format off

// These strings are also in gen_base.cpp

inline constexpr const auto txt_wxueImageFunction = R"===(
// Convert a data array into a wxImage
inline wxImage wxueImage(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream strm(data, size_data);
    wxImage image;
    image.LoadFile(strm);
    return image;
};
)===";

inline constexpr const auto txt_GetBundleFromSVG = R"===(
// Convert compressed SVG string into a wxBitmapBundle
inline wxBitmapBundle wxueBundleSVG(const unsigned char* data,
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
    if (m_embedded_images.empty())
    {
        return;
    }

    if (m_panel_type != HDR_PANEL)
    {
        bool is_namespace_written = false;
        for (auto iter_array: m_embedded_images)
        {
            if (iter_array->form != m_form_node)
                continue;

            if (!is_namespace_written)
            {
                m_source->writeLine();
                m_source->writeLine("namespace wxue_img\n{");
                m_source->Indent();
                is_namespace_written = true;
            }
            m_source->writeLine();
            ttlib::cstr code;
            code.reserve(max_image_line_length + 16);
            // SVG images store the original size in the high 32 bits
            size_t max_pos = (iter_array->array_size & 0xFFFFFFFF);
            code << "const unsigned char " << iter_array->array_name << '[' << max_pos << "] {";

            m_source->writeLine(code);

            size_t pos = 0;
            while (pos < max_pos)
            {
                code.clear();
                // -8 to account for 4 indent + max 3 chars for number + comma
                for (; pos < max_pos && code.size() < static_cast<size_t>(max_image_line_length - 8); ++pos)
                {
                    code << static_cast<int>(iter_array->array_data[pos]) << ',';
                }
                if (pos >= max_pos && code.back() == ',')
                    code.pop_back();
                m_source->writeLine(code);
            }
            m_source->writeLine("};");
        }
        if (is_namespace_written)
        {
            m_source->writeLine();
            m_source->Unindent();
            m_source->writeLine("}\n");
        }
    }

    if (m_panel_type != CPP_PANEL)
    {
        bool is_namespace_written = false;
        for (auto iter_array: m_embedded_images)
        {
            if (iter_array->form != m_form_node)
                continue;

            if (!is_namespace_written)
            {
                m_header->writeLine();
                m_header->writeLine("namespace wxue_img\n{");

                if (m_form_node->isType(type_images))
                {
                    ttlib::textfile function;
                    function.ReadString(txt_wxueImageFunction);
                    for (auto& iter: function)
                    {
                        m_header->write("\t");
                        if (iter.size() && iter.at(0) == ' ')
                            m_header->write("\t");
                        m_header->writeLine(iter);
                    }
                    m_header->writeLine();
                }

                m_header->Indent();
                if (!m_form_node->isType(type_images))
                {
                    m_header->writeLine("// Images declared in this class module:");
                    m_header->writeLine();
                }
                is_namespace_written = true;
            }
            m_header->writeLine(ttlib::cstr("extern const unsigned char ")
                                << iter_array->array_name << '[' << (iter_array->array_size & 0xFFFFFFFF) << "];");
        }
        if (is_namespace_written)
        {
            m_header->Unindent();
            m_header->writeLine("}\n");
        }
    }
}
