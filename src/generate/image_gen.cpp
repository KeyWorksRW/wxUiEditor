/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for generating embedded images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <vector>

#include <tttextfile_wx.h>  // textfile -- Classes for reading and writing line-oriented files

#include "gen_base.h"

#include "code.h"           // Code -- Helper class for generating code
#include "project_class.h"  // Project class
#include "write_code.h"     // Write code to Scintilla or file

std::vector<std::string> base64_encode(unsigned char const* data, size_t data_size);

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
            code.Eol(eol_if_needed).Str("extern const unsigned char ").Str(iter_array->array_name);
            code.Str("[").itoa(iter_array->array_size & 0xFFFFFFFF).Str("];");
        }
    }

    if (code.is_cpp() && is_namespace_written)
    {
        code.CloseBrace();
    }
    // m_source->writeLine(code);
}

// Generate code after the construcor for embedded images not defined in the gen_Images node.
void BaseCodeGenerator::WriteImagePostConstruction(Code& code)
{
    code.clear();

    bool is_namespace_written = false;
    // -12 to account for 8 indent + max 3 chars for number + comma
    size_t cpp_line_length = (to_size_t) GetProject()->as_int(prop_cpp_line_length) - 12;

    for (auto iter_array: m_embedded_images)
    {
        if (iter_array->form != m_form_node)
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

            code.Eol().Str("const unsigned char ").Str(iter_array->array_name);
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
                if (pos >= max_pos && code.m_code.back() == ',')
                {
                    code.m_code.pop_back();
                }
                m_source->writeLine(code);
                code.clear();
                // Since we don't call Eol() in this loop, the indentation is not processed.
                code.Tab(2);
            }
            if (code.m_code.back() == '\t')
            {
                code.m_code.pop_back();
            }
            code += "};\n";
        }
        else
        {
            code.Eol().Str(iter_array->array_name).Str(" = PyEmbeddedImage(");
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
                ttlib::viewfile function;
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

std::vector<std::string> base64_encode(unsigned char const* data, size_t data_size)
{
    const size_t tab_quote_prefix = 7;  // 4 for tab, 2 for quotes, 1 for 'b' prefix
    size_t line_length = (to_size_t) GetProject()->as_int(prop_python_line_length) - tab_quote_prefix;

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
