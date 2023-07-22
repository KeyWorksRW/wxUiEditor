/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for generating embedded images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <vector>

#include "image_gen.h"

#include "code.h"             // Code -- Helper class for generating code
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"       // Common component functions
#include "image_handler.h"    // ImageHandler class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // Write code to Scintilla or file

// Generate extern references to images used in the current form that are defined in the
// gen_Images node. These are written before the class constructor.
void BaseCodeGenerator::WriteImagePreConstruction(Code& code)
{
    code.clear();

    bool is_namespace_written = false;
    for (auto iter_array: m_embedded_images)
    {
        // If the image is defined in this form, then it will already have been declared in the class's header file.
        // For the source code, we only care about images defined in another source module.

        if (iter_array->form == m_form_node)
            continue;

        if (code.is_cpp())
        {
            if (!is_namespace_written)
            {
                is_namespace_written = true;
                code.Str("namespace wxue_img").OpenBrace();
            }
            if (iter_array->filename.size())
            {
                code.Eol(eol_if_needed).Str("// ").Str(iter_array->filename);
            }
            code.Eol(eol_if_needed).Str("extern const unsigned char ").Str(iter_array->array_name);
            code.Str("[").itoa(iter_array->array_size & 0xFFFFFFFF).Str("];");
        }
    }

    if (code.is_cpp() && is_namespace_written)
    {
        code.CloseBrace().Eol();
    }
}

// Generate code after the construcor for embedded images not defined in the gen_Images node.
void BaseCodeGenerator::WriteImageConstruction(Code& code)
{
    WriteCode* save_writer = m_TranslationUnit ? nullptr : m_source;
    bool inlined_warning = false;
    if (!m_TranslationUnit)
    {
        m_source = m_header;
    }

    code.clear();

    bool is_namespace_written = false;
    // -12 to account for 8 indent + max 3 chars for number + comma
    size_t cpp_line_length = Project.as_size_t(prop_cpp_line_length) - 12;

    for (auto iter_array: m_embedded_images)
    {
        if (iter_array->form != m_form_node && code.is_cpp())
            continue;

        if (code.is_cpp())
        {
            if (!is_namespace_written)
            {
                is_namespace_written = true;
                code.Eol().Str("namespace wxue_img").OpenBrace();
            }

            // SVG images store the original size in the high 32 bits
            size_t max_pos = (iter_array->array_size & 0xFFFFFFFF);

            if (iter_array->filename.size())
            {
                code.Eol(eol_if_needed).Str("// ").Str(iter_array->filename);
            }
            code.Eol();

            if (!m_TranslationUnit)
            {
                if (!inlined_warning)
                {
                    inlined_warning = true;
                    code.Str("// WARNING: This will only work if compiled with C++17 or later.");
                    code.Eol().Str("// Create an Images List and check auto_update to prevent the image");
                    code.Eol().Str("// from being added to this header file.").Eol();
                }
                // The header file can be included multiple times, so we need to set this to
                // inline to avoid multiple definitions. Note that this requires C++17 --
                // anything earlier will result in duplication.
                code << "inline ";
            }
            code.Str("const unsigned char ").Str(iter_array->array_name);
            code.Str("[").itoa(max_pos).Str("] {");
            m_source->writeLine(code);
            code.clear();
            // Since we don't call Eol() in the following loop, the indentation is not processed.
            code.Tab(2);

            size_t pos = 0;
            while (pos < max_pos)
            {
                for (; pos < max_pos && code.size() < cpp_line_length; ++pos)
                {
                    code.itoa(iter_array->array_data[pos]) += ",";
                }
                if (pos >= max_pos && code.GetCode().back() == ',')
                {
                    code.GetCode().pop_back();
                }
                m_source->writeLine(code);
                code.clear();
                // Since we don't call Eol() in this loop, the indentation is not processed.
                code.Tab(2);
            }
            if (code.GetCode().back() == '\t')
            {
                code.pop_back();
            }
            code += "};\n";
        }
        else
        {
            if (iter_array->form->isGen(gen_Images))
            {
                continue;
            }
            if (iter_array->filename.size())
            {
                code.Eol().Str("# ").Str(iter_array->filename);
            }
            code.Eol().Str(iter_array->array_name);
            if (iter_array->type == wxBITMAP_TYPE_INVALID)
            {
                code.Str(" = (");
            }
            else
            {
                code.Str(" = PyEmbeddedImage(");
            }
            m_source->writeLine(code);
            code.clear();
            auto encoded = base64_encode(iter_array->array_data.get(), iter_array->array_size & 0xFFFFFFFF);
            if (encoded.size())
            {
                encoded.back() += ")";
                m_source->writeLine(encoded);
            }
        }
    }

    if (code.is_cpp() && is_namespace_written)
    {
        code.ResetBraces();
        code.Eol() += "}";
    }

    if (code.size())
    {
        m_source->writeLine(code);
    }

    if (save_writer)
    {
        m_source = save_writer;
    }
}

void GenerateSingleBitmapCode(Code& code, const tt_string& description)
{
    if (description.empty())
    {
        code += "wxNullBitmap";
        return;
    }
    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexType].starts_with("SVG"))
    {
        if (code.is_cpp())
        {
            if (Project.as_string(prop_wxWidgets_version) == "3.1")
            {
                code += "wxNullBitmap /* SVG images require wxWidgets 3.1.6 */";
                return;
            }
        }
        tt_string name(parts[IndexImage]);

        if (code.is_python())
        {
            name.make_absolute();
            auto path = MakePythonPath(code.node());
            name.make_relative(path);
        }
        name.backslashestoforward();

        // SVG files don't have an innate size, so we must rely on the size specified in the property

        code.Add("wx.BitmapBundle.FromSVGFile(");
        code.QuotedString(name);
        wxSize svg_size { -1, -1 };
        if (parts[IndexSize].size())
        {
            svg_size = GetSizeInfo(parts[IndexSize]);
        }
        code.Comma().Add("wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
        code.Str("GetBitmap(").Add("wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y).EndFunction();
        return;
    }
    else if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code.Add("wxArtProvider").ClassMethod("GetBitmap(").Add(art_id);

        // Note that current documentation states that the client is required, but the header file says otherwise
        if (art_client.size())
            code.Comma().Add(art_client);
        code += ")";
    }
    else if (parts[IndexType].is_sameas("XPM"))
    {
        if (code.is_cpp())
        {
            code.Add("wxImage(");

            tt_string name(parts[IndexImage].filename());
            name.remove_extension();
            code << name << "_xpm)";
        }
        else
        {
            tt_string name(parts[IndexImage]);
            name.make_absolute();
            auto path = MakePythonPath(code.node());
            name.make_relative(path);
            name.backslashestoforward();

            code.Str("wx.Image(").QuotedString(name) += ")";
        }
    }
    else if (parts[IndexImage].empty())
    {
        code.Add("wxNullBitmap");
    }
    else
    {
        if (code.is_cpp())
        {
            code << "wxueImage(";

            tt_string name(parts[1].filename());
            name.remove_extension();
            name.Replace(".", "_", true);  // wxFormBuilder writes files with the extra dots that have to be converted to '_'

            if (parts[IndexType].starts_with("Embed"))
            {
                auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                if (embed)
                {
                    name = "wxue_img::" + embed->array_name;
                }
            }

            code.Str(name).Comma().Str("sizeof(").Str(name) += "))";
        }
        else
        {
            if (auto bundle = ProjectImages.GetPropertyImageBundle(description); bundle && bundle->lst_filenames.size())
            {
                bool is_embed_success = false;

                if (parts[IndexType].starts_with("Embed"))
                {
                    if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                    {
                        code.CheckLineLength(embed->array_name.size() + sizeof(".Bitmap)"));
                        AddPythonImageName(code, embed);
                        code += ".Bitmap";
                        is_embed_success = true;
                    }
                }

                if (!is_embed_success)
                {
                    tt_string name(bundle->lst_filenames[0]);
                    name.make_absolute();
                    auto path = MakePythonPath(code.node());
                    name.make_relative(path);
                    name.backslashestoforward();
                    code.Str("wx.Bitmap(").QuotedString(name) += ")";
                }
            }
        }
    }
}

// clang-format off

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

// clang-format on

void BaseCodeGenerator::WriteImagePostHeader()
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
                tt_view_vector function;
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
        if (iter_array->filename.size())
        {
            m_header->writeLine(tt_string("// ") << iter_array->filename);
        }
        m_header->writeLine(tt_string("extern const unsigned char ")
                            << iter_array->array_name << '[' << (iter_array->array_size & 0xFFFFFFFF) << "];");
    }
    if (is_namespace_written)
    {
        m_header->Unindent();
        m_header->writeLine("}\n");
    }
}

std::vector<std::string> base64_encode(unsigned char const* data, size_t data_size)
{
    const size_t tab_quote_prefix = 7;  // 4 for tab, 2 for quotes, 1 for 'b' prefix
    size_t line_length = Project.as_size_t(prop_python_line_length) - tab_quote_prefix;

    const std::array<char, 64> base64_chars = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                                'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                                'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

    std::vector<std::string> result;

    std::string line;
    line.reserve(line_length + 4);
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    auto a3_to_a4 = [&]()
    {
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
    };

    line = "\tb\"";
    size_t line_pos = 3;
    size_t a3_pos = 0;
    for (size_t idx = 0; idx < data_size; ++idx)
    {
        char_array_3[a3_pos++] = data[idx];
        if (a3_pos == 3)
        {
            a3_to_a4();

            line += base64_chars[char_array_4[0]];
            line += base64_chars[char_array_4[1]];
            line += base64_chars[char_array_4[2]];
            line += base64_chars[char_array_4[3]];

            a3_pos = 0;
            line_pos += 4;
            if (line_pos >= line_length)
            {
                line += "\"";
                result.emplace_back(line);
                line_pos = 3;
                line.resize(line_pos);
            }
        }
    }

    if (a3_pos)
    {
        for (size_t index = a3_pos; index < 3; index++)
        {
            char_array_3[index] = '\0';
        }

        a3_to_a4();

        for (size_t a4_pos = 0; a4_pos < a3_pos + 1; a4_pos++)
        {
            line += base64_chars[char_array_4[a4_pos]];
        }
        while (a3_pos++ < 3)
        {
            line += '=';
        }
    }
    line += "\"";
    result.emplace_back(line);

    return result;
}

void BaseCodeGenerator::GeneratePythonImagesForm()
{
    if (m_embedded_images.empty() || !m_form_node->getChildCount())
    {
        return;
    }

    m_source->writeLine();
    m_source->writeLine("from wx.lib.embeddedimage import PyEmbeddedImage");

    Code code(m_form_node, GEN_LANG_PYTHON);

    for (auto iter_array: m_embedded_images)
    {
        if (iter_array->form != m_form_node)
            continue;

        if (iter_array->filename.size())
        {
            code.Eol().Str("# ").Str(iter_array->filename);
        }
        code.Eol().Str(iter_array->array_name);
        if (iter_array->type == wxBITMAP_TYPE_INVALID)
        {
            code.Str(" = (");
        }
        else
        {
            code.Str(" = PyEmbeddedImage(");
        }

        m_source->writeLine(code);
        code.clear();
        auto encoded = base64_encode(iter_array->array_data.get(), iter_array->array_size & 0xFFFFFFFF);
        if (encoded.size())
        {
            encoded.back() += ")";
            m_source->writeLine(encoded);
        }
    }

    m_source->writeLine();
}

void BaseCodeGenerator::GenerateRubyImagesForm()
{
    if (m_embedded_images.empty() || !m_form_node->getChildCount())
    {
        return;
    }

    m_source->writeLine();
}

void AddPythonImageName(Code& code, const EmbeddedImage* embed)
{
    if (embed->form->isGen(gen_Images))
    {
        tt_string import_name = embed->form->as_string(prop_python_file).filename();
        import_name.remove_extension();

        code.Str(import_name).Str(".");
    }
    code.Str(embed->array_name);
}
