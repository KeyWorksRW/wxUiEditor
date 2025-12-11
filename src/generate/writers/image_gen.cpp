/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for generating embedded images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>

#include <array>
#include <format>
#include <vector>

#include "image_gen.h"

#include "code.h"             // Code -- Helper class for generating code
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"       // Common component functions
#include "gen_enums.h"        // Enumerations for generators
#include "image_handler.h"    // ImageHandler class
#include "mainapp.h"          // Main application class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // Write code to Scintilla or file

// Generate code after the construcor for embedded images not defined in the gen_Images node.
void BaseCodeGenerator::WriteImageConstruction(Code& code)
{
    code.clear();

    bool is_namespace_written = false;
    // -12 to account for 8 indent + max 3 chars for number + comma
    size_t cpp_line_length = Project.as_size_t(prop_cpp_line_length) - 12;

    for (auto iter_array: m_embedded_images)
    {
        // The images form contains global images, so no need to generate code for them here.
        // if (iter_array->get_Form() == images_form)
        if (iter_array->get_Form() != m_form_node)
        {
            continue;
        }

        if (code.is_cpp())
        {
            if (!is_namespace_written)
            {
                is_namespace_written = true;
                code.Eol().Str("namespace wxue_img").OpenBrace();
            }

            // SVG images store the original size in the high 32 bits
            size_t max_pos = (iter_array->base_image().array_size & 0xFFFFFFFF);

            if (iter_array->base_image().filename.size())
            {
                code.Eol(eol_if_needed).Str("// ").Str(iter_array->base_image().filename);
            }
            code.Eol();
            code.Str("const unsigned char ").Str(iter_array->base_image().array_name);
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
                    code.itoa(iter_array->base_image().array_data[pos]) += ",";
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
        else if (code.is_python())
        {
            if (iter_array->get_Form()->is_Gen(gen_Images))
            {
                continue;
            }
            if (iter_array->base_image().filename.size())
            {
                code.Eol().Str("# ").Str(iter_array->base_image().filename);
            }
            code.Eol().Str(iter_array->base_image().array_name);
            if (iter_array->base_image().type == wxBITMAP_TYPE_SVG)
            {
                code.Str(" = (");
            }
            else
            {
                code.Str(" = PyEmbeddedImage(");
            }
            m_source->writeLine(code);
            code.clear();
            auto encoded =
                base64_encode(iter_array->base_image().array_data.data(),
                              iter_array->base_image().array_size & 0xFFFFFFFF, GEN_LANG_PYTHON);
            if (encoded.size())
            {
                encoded.back() += ")";
                m_source->writeLine(encoded);
            }
        }
        else if (code.is_ruby())
        {
            if (iter_array->get_Form()->is_Gen(gen_Images))
            {
                continue;
            }
            if (iter_array->base_image().filename.size())
            {
                code.Eol().Str("# ").Str(iter_array->base_image().filename);
            }
            code.Eol().Str("$").Str(iter_array->base_image().array_name);
            if (iter_array->base_image().type == wxBITMAP_TYPE_SVG)
            {
                code.Str(" = (");
            }
            else
            {
                code.Str(" = Base64.decode64(");
            }
            m_source->writeLine(code);
            code.clear();
            auto encoded =
                base64_encode(iter_array->base_image().array_data.data(),
                              iter_array->base_image().array_size & 0xFFFFFFFF, GEN_LANG_RUBY);
            if (encoded.size())
            {
                // Remove the trailing '\' character
                encoded.back().pop_back();
                // and the now trailing space
                encoded.back().pop_back();
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

std::map<GenLang, GenEnum::PropName> map_lang_to_prop = {

    { GEN_LANG_CPLUSPLUS, prop_cpp_line_length },
    { GEN_LANG_PERL, prop_perl_line_length },
    { GEN_LANG_PYTHON, prop_python_line_length },
    { GEN_LANG_RUBY, prop_ruby_line_length  },
};

// clang-format on

auto base64_encode(unsigned char const* data, size_t data_size, GenLang language)
    -> std::vector<std::string>
{
    size_t tab_quote_prefix = 7;  // 4 for tab, 2 for quotes, 1 for 'b' prefix
    if (language == GEN_LANG_RUBY)
    {
        tab_quote_prefix = 6;  // 2 for tab, 2 for quotes, 2 for " \" suffix
    }
    GenEnum::PropName prop = prop_python_line_length;
    if (auto result = map_lang_to_prop.find(language); result != map_lang_to_prop.end())
    {
        prop = result->second;
    }

    size_t line_length = Project.as_size_t(prop) - tab_quote_prefix;

    constexpr auto base64_chars = std::to_array(
        { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
          'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
          'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
          'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' });

    std::vector<std::string> result;

    std::string line;
    line.reserve(line_length + 4);
    std::array<unsigned char, 3> char_array_3 {};
    std::array<unsigned char, 4> char_array_4 {};

    constexpr unsigned char MASK_FC = 0xfc;
    constexpr unsigned char MASK_03 = 0x03;
    constexpr unsigned char MASK_F0 = 0xf0;
    constexpr unsigned char MASK_0F = 0x0f;
    constexpr unsigned char MASK_C0 = 0xc0;
    constexpr unsigned char MASK_3F = 0x3f;
    constexpr int SHIFT_2 = 2;
    constexpr int SHIFT_4 = 4;
    constexpr int SHIFT_6 = 6;

    auto a3_to_a4 = [&]()
    {
        char_array_4[0] = (char_array_3[0] & MASK_FC) >> SHIFT_2;
        char_array_4[1] =
            ((char_array_3[0] & MASK_03) << SHIFT_4) + ((char_array_3[1] & MASK_F0) >> SHIFT_4);
        char_array_4[2] =
            ((char_array_3[1] & MASK_0F) << SHIFT_2) + ((char_array_3[2] & MASK_C0) >> SHIFT_6);
        char_array_4[3] = char_array_3[2] & MASK_3F;
    };

    std::string line_begin = "\tb\"";
    std::string line_end = "\"";
    if (language == GEN_LANG_PYTHON)
    {
        line_begin = "\tb\"";
        line_end = "\"";
    }
    else if (language == GEN_LANG_RUBY)
    {
        line_begin = "  '";
        line_end = "' \\";
    }

    line = line_begin;
    size_t line_pos = line_begin.size();
    size_t a3_pos = 0;
    for (size_t idx = 0; idx < data_size; ++idx)
    {
        char_array_3.at(a3_pos) = data[idx];
        ++a3_pos;
        if (a3_pos == 3)
        {
            a3_to_a4();

            line += base64_chars.at(char_array_4.at(0));
            line += base64_chars.at(char_array_4.at(1));
            line += base64_chars.at(char_array_4.at(2));
            line += base64_chars.at(char_array_4.at(3));

            a3_pos = 0;
            line_pos += 4;
            if (line_pos >= line_length)
            {
                line += line_end;
                result.emplace_back(line);
                line_pos = line_begin.size();
                line.resize(line_pos);
            }
        }
    }

    if (a3_pos)
    {
        for (size_t index = a3_pos; index < 3; index++)
        {
            char_array_3.at(index) = '\0';
        }

        a3_to_a4();

        for (size_t a4_pos = 0; a4_pos < a3_pos + 1; a4_pos++)
        {
            line += base64_chars.at(char_array_4.at(a4_pos));
        }
        while (a3_pos++ < 3)
        {
            line += '=';
        }
    }
    line += line_end;
    result.emplace_back(line);

    return result;
}

// ******************************* Bundle Code Generation *******************************
//
// Call GenerateBundleParameter() to generate the code to create a wxBitmapBundle, or
// optionally a wxBitmap.
//
// **************************************************************************************

namespace
{
    // Helper constants for bundle generation
}  // end anonymous namespace

void Code::GenerateSVGBundle(const tt_string_vector& parts, bool get_bitmap)
{
    wxSize svg_size { -1, -1 };
    if (parts[IndexSize].size())
    {
        svg_size = GetSizeInfo(parts[IndexSize]);
    }

    if (is_cpp())
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(&parts); function_name.size())
        {
            // The function name includes the size, but we need to replace the size with a DIP
            // version.
            function_name.erase_from("(");
            Eol().Tab().Str(function_name);
            if (get_bitmap)
            {
                Str("(FromDIP(").itoa(svg_size.x).Str("), FromDIP(").itoa(svg_size.y) += "))";
                Str(".").Add("GetBitmap(").Add("wxDefaultSize)");
            }
            else
            {
                // For SVG files, just provide the default size, and wxWidgets will scale it
                // before converting it to a bitmap for rendering.
                Str("(").itoa(svg_size.x).Str(", ").itoa(svg_size.y) += ")";
            }
            return;
        }
    }

    auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
    if (!embed)
    {
        MSG_WARNING(tt_string() << parts[IndexImage] << " not embedded!");
        Add("wxNullBitmap");
        return;
    }

    if (is_cpp())
    {
        tt_string name = "wxue_img::" + embed->base_image().array_name;
        Eol() << "\twxueBundleSVG(" << name << ", "
              << (to_size_t) (embed->base_image().array_size & 0xFFFFFFFF) << ", ";
        itoa((to_size_t) (embed->base_image().array_size >> 32)).Comma();
        if (get_bitmap)
        {
            FormFunction("FromDIP(").Add("wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y) +=
                ")))";
            Str(".").Add("GetBitmap(").Add("wxDefaultSize)");
        }
        else
        {
            Add("wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
        }
        return;
    }
    if (is_python())
    {
        tt_string svg_name;

        if (embed->get_Form() != node()->get_Form())
        {
            svg_name = embed->get_Form()->as_string(prop_python_file).filename();
            svg_name.remove_extension();
            svg_name << '.' << embed->base_image().array_name;
        }
        else
        {
            svg_name = embed->base_image().array_name;
        }
        insert(0, tt_string("_svg_string_ = zlib.decompress(base64.b64decode(")
                      << svg_name << "))\n");
        Eol() += "\twx.BitmapBundle.FromSVG(_svg_string_";
    }
    else if (is_ruby())
    {
        tt_string svg_name;
        svg_name = "$" + embed->base_image().array_name;
        insert(0, tt_string("_svg_string_ = Zlib::Inflate.inflate(Base64.decode64(")
                      << svg_name << "))\n");
        *this += "Wx::BitmapBundle.from_svg(_svg_string_";
        Comma().Str("Wx::Size.new(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
    }

    if (get_bitmap)
    {
        if (!is_ruby())
        {
            Comma().CheckLineLength(
                sizeof("FromDIP(wx::Size.new(32, 32))).GetBitmap(wxDefaultSize)"));
            FormFunction("FromDIP(").Add("wxSize(");
            itoa(svg_size.x).Comma().itoa(svg_size.y) += ")))";
        }
        VariableMethod("GetBitmap(").AddType("wxDefaultSize").Str(")");
    }
    else
    {
        // wxSize already added above
        if (!is_ruby())
        {
            Comma().Add("wxSize(");
            itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
        }
    }
}

void Code::GenerateARTBundle(const tt_string_vector& parts, bool get_bitmap)
{
    Class("wxArtProvider");
    if (get_bitmap)
    {
        ClassMethod("GetBitmap(");
    }
    else if (is_perl())
    {
        // Perl doesn't support wxArtProvider.GetBitmapBundle()
        Str("::GetBitmap(");
    }
    else
    {
        ClassMethod("GetBitmapBundle(");
    }

    tt_string art_id(parts[IndexArtID]);
    tt_string art_client;
    if (auto pos = art_id.find('|'); ttwx::is_found(pos))
    {
        art_client = art_id.subview(pos + 1);
        art_id.erase(pos);
    }

    if (is_cpp() || is_perl())
    {
        // No need to change it for C++ or wxPerl
        Str(art_id);
    }
    else
    {
        Add(art_id);
    }

    // Note that current documentation states that the client is required, but the header file
    // says otherwise
    if (art_client.size())
    {
        Comma();
        if (is_cpp() || is_perl())
        {
            // No need to change it for C++ or wxPerl
            Str(art_client);
        }
        else
        {
            Add(art_client);
        }
    }
    if (parts.size() > IndexSize)
    {
        Comma();
        CheckLineLength(sizeof("wxSize(999, 999)))"));
        auto size = GetSizeInfo(parts[IndexSize]);
        WxSize(size, code::no_scaling);
    }
    *this << ')';
}

void Code::GenerateEmbedBundle(const tt_string_vector& parts, bool get_bitmap)
{
    if (is_cpp())
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(&parts); function_name.size())
        {
            Str(function_name);
            if (get_bitmap)
            {
                // BUGBUG: [Randalphwa - 09-19-2023] This is not correct. We need to get the
                // size of the embedded image and use that to get the bitmap including
                // rescaling it if it is a single image.
                Str(".").Add("GetBitmap(").Add("wxDefaultSize)");
            }
            return;
        }
    }

    const auto* bundle = ProjectImages.GetPropertyImageBundle(&parts);
    if (bundle && bundle->lst_filenames.size())
    {
#if defined(_DEBUG)
        auto msg =
            std::format("Missing bundle for {} in {}:{} ({})", std::string_view(parts[IndexImage]),
                        m_node->get_Form()->as_view(prop_class_name),
                        m_node->as_view(prop_var_name), GenLangToString(m_language));
        wxGetApp().DebugOutput(msg);
        MSG_WARNING(msg);
#endif  // _DEBUG
        Add("wxNullBitmap");
        return;
    }

    auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
    if (!embed)
    {
        FAIL_MSG(tt_string("Missing embed for ") << bundle->lst_filenames[0]);
        Add("wxNullBitmap");
        return;
    }

    if (is_ruby())
    {
        // Ruby has a single function that will create a bundle from 1 to 3 images
        Str("wxue_get_bundle(").Str("$").Str(embed->base_image().array_name);
        if (bundle->lst_filenames.size() > 1)
        {
            if (EmbeddedImage* embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]);
                embed2)
            {
                Comma().Str("$").Str(embed2->base_image().array_name);
            }
            if (bundle->lst_filenames.size() > 2)
            {
                if (EmbeddedImage* embed3 =
                        ProjectImages.GetEmbeddedImage(bundle->lst_filenames[2]);
                    embed3)
                {
                    Comma().Str("$").Str(embed3->base_image().array_name);
                }
            }
        }
        *this += ')';

        if (get_bitmap)
        {
            Str(".get_bitmap(").Eol().Tab(2).Str("Wx::Size.new(");
            *this << ("from_dip(") << embed->get_wxSize().x << "), from_dip("
                  << embed->get_wxSize().y << ")))";
            // TODO: [Randalphwa - 09-19-2023] If it's a single image, then it may need to be
            // rescaled using wxIMAGE_QUALITY_BILINEAR rather than letting the wxBitmapBundle do
            // it. However, the only embedded images we support are bundles, so this probably
            // isn't practical.
        }
        return;
    }

    if (is_perl())
    {
        Str("wxue_get_bundle(").Str("$").Str(embed->base_image().array_name);
        if (bundle->lst_filenames.size() > 1)
        {
            if (EmbeddedImage* embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]);
                embed2)
            {
                Comma().Str("$").Str(embed2->base_image().array_name);
            }
            if (bundle->lst_filenames.size() > 2)
            {
                if (EmbeddedImage* embed3 =
                        ProjectImages.GetEmbeddedImage(bundle->lst_filenames[2]);
                    embed3)
                {
                    Comma().Str("$").Str(embed3->base_image().array_name);
                }
            }
        }
        *this += ')';

        if (get_bitmap)
        {
            Str(".get_bitmap(").Eol().Tab(2).Str("wxSize.new(");
            *this << ("from_dip(") << embed->get_wxSize().x << "), from_dip("
                  << embed->get_wxSize().y << ")))";
            // TODO: [Randalphwa - 09-19-2023] If it's a single image, then it may need to be
            // rescaled using wxIMAGE_QUALITY_BILINEAR rather than letting the wxBitmapBundle do
            // it. However, the only embedded images we support are bundles, so this probably
            // isn't practical.
        }
    }

    tt_string path;
    if (is_python())
    {
        path = MakePythonPath(node());
    }
    else if (is_ruby())
    {
        path = MakeRubyPath(node());
    }

    tt_string name(bundle->lst_filenames[0]);
    name.make_absolute();
    name.make_relative(path);
    name.backslashestoforward();

    if (is_cpp() && get_bitmap)
    {
        Eol().Tab() << "wxueImage(";

        name = "wxue_img::" + embed->base_image().array_name;

        *this << name << ", sizeof(" << name << "))";
        *this << ".Rescale(";
        Eol() << "\tFromDIP(" << embed->get_wxSize().x << "), FromDIP(" << embed->get_wxSize().y
              << "), wxIMAGE_QUALITY_BILINEAR)";
    }
    else if (bundle->lst_filenames.size() == 1)
    {
        if (is_cpp())
        {
            Eol().Tab() << "wxueImage(";

            name = "wxue_img::" + embed->base_image().array_name;

            *this << name << ", sizeof(" << name << "))";
        }
        else if (is_python())
        {
            if (get_bitmap)
            {
                Str("wx.Bitmap(");
            }
            AddPythonImageName(embed);
            *this += get_bitmap ? ".Image" : ".Bitmap";
            if (get_bitmap)
            {
                Str(".Rescale(").Eol().Tab();
                FormFunction("FromDIP(")
                    .itoa(embed->get_wxSize().x)
                    .Str("), ")
                    .FormFunction("FromDIP(");
                itoa(embed->get_wxSize().y) << "), wx.IMAGE_QUALITY_BILINEAR))";
            }
        }
    }
    else if (bundle->lst_filenames.size() == 2)
    {
        Add("wxBitmapBundle").ClassMethod("FromBitmaps(");
        if (is_cpp())
        {
            *this << "wxueImage(";
            name = "wxue_img::" + embed->base_image().array_name;
            *this << name << ", sizeof(" << name << ")), wxueImage(";

            if (auto* embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed2)
            {
                name = "wxue_img::" + embed2->base_image().array_name;
                name.remove_extension();
                *this << name << ", sizeof(" << name << ")))";
            }
            else
            {
                *this << "wxNullBitmap))";
            }
        }
        else if (is_python())
        {
            CheckLineLength(embed->base_image().array_name.size() + sizeof(".Bitmap)"));
            AddPythonImageName(embed);
            *this += ".Bitmap";
            if (auto* embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed2)
            {
                Comma().CheckLineLength(embed2->base_image().array_name.size() +
                                        sizeof(".Bitmap)"));
                AddPythonImageName(embed2);
                *this += ".Bitmap";
            }
            else
            {
                Comma().Str("wx.NullBitmap");
            }
            if (get_bitmap)
            {
                CheckLineLength(sizeof("FromDIP(wx::Size.new(32, 32))).GetBitmap(wxDefaultSize)"));
                Str(").GetBitmap(").Add("wxSize(");
                Eol()
                    .Tab()
                    .FormFunction("FromDIP(")
                    .itoa(embed->get_wxSize().x)
                    .Str("), ")
                    .FormFunction("FromDIP(");
                itoa(embed->get_wxSize().y) << "))";
            }
            *this << ')';
        }
    }
    else
    {
        if (is_cpp())
        {
            Indent();
            Eol().Str("[&]()");
            OpenBrace().Add("wxVector<wxBitmap> bitmaps;");

            for (const auto& iter: bundle->lst_filenames)
            {
                tt_string name_img(iter.filename());
                name_img.remove_extension();
                name_img.Replace(".", "_", true);
                if (parts[IndexType].starts_with("Embed"))
                {
                    auto* embed_img = ProjectImages.GetEmbeddedImage(iter);
                    if (embed_img)
                    {
                        name_img = "wxue_img::" + embed_img->base_image().array_name;
                    }
                }
                Eol().Str("bitmaps.push_back(wxueImage(")
                    << name_img << ", sizeof(" << name_img << ")));";
            }
            Eol();
            Str("return wxBitmapBundle::FromBitmaps(bitmaps);");
            CloseBrace().Str("()").Eol();
        }
        else if (is_python())
        {
            bool is_xpm = (parts[IndexType].is_sameas("XPM"));

            *this += "wx.BitmapBundle.FromBitmaps([ ";
            bool needs_comma = false;
            for (const auto& iter: bundle->lst_filenames)
            {
                if (needs_comma)
                {
                    UpdateBreakAt();
                    Comma(false).Eol().Tab(3);
                }

                bool is_embed_success = false;
                if (auto* embed_img = ProjectImages.GetEmbeddedImage(iter); embed_img)
                {
                    AddPythonImageName(embed_img);
                    *this += ".Bitmap";
                    needs_comma = true;
                    is_embed_success = true;
                }

                if (!is_embed_success)
                {
                    tt_string name_img(iter);
                    name.make_absolute();
                    name.make_relative(path);
                    name.backslashestoforward();

                    Str("wx.Bitmap(").QuotedString(name);
                    if (is_xpm)
                    {
                        Comma().Str("wx.BITMAP_TYPE_XPM");
                    }
                    *this += ")";
                    needs_comma = true;
                }
            }
            *this += " ])";
            if (get_bitmap)
            {
                CheckLineLength(sizeof("FromDIP(wx::Size.new(32, 32))).GetBitmap(wxDefaultSize)"));
                Str(".GetBitmap(").Add("wxSize(");
                FormFunction("FromDIP(")
                    .itoa(embed->get_wxSize().x)
                    .Str("), ")
                    .FormFunction("FromDIP(");
                itoa(embed->get_wxSize().y) << ")))";
            }
        }
    }
}

void Code::GenerateXpmBitmap(const tt_string_vector& parts, bool /* get_bitmap */)
{
    // We only marginally support XPM files -- we only allow a single file, and we don't attempt
    // to scale it.

    tt_string name(parts[IndexImage].filename());
    if (is_cpp())
    {
        Str("wxBitmap(");
        name.remove_extension();
        *this << name << "_xpm)";
    }
    else if (is_perl())
    {
        auto path = MakePerlPath(node());
        name.make_absolute();
        if (!name.file_exists())
        {
            name = Project.ArtDirectory();
            name.append_filename(parts[IndexImage].filename());
        }
        name.make_relative(path);
        name.backslashestoforward();

        Str("Wx::Bitmap->new(");
        CheckLineLength(name.size() + 3);
        QuotedString(name);
        Comma().Str("wxBITMAP_TYPE_XPM)");
    }
    else if (is_python())
    {
        auto path = MakePythonPath(node());
        name.make_absolute();
        if (!name.file_exists())
        {
            name = Project.ArtDirectory();
            name.append_filename(parts[IndexImage].filename());
        }
        name.make_relative(path);
        name.backslashestoforward();

        Str("wx.Bitmap(");
        CheckLineLength(name.size() + 2);
        QuotedString(name);
        Comma().Str("wx.BITMAP_TYPE_XPM)");
    }
    else if (is_ruby())
    {
        auto path = MakeRubyPath(node());
        name.make_absolute();
        if (!name.file_exists())
        {
            name = Project.ArtDirectory();
            name.append_filename(parts[IndexImage].filename());
        }
        name.make_relative(path);
        name.backslashestoforward();

        Str("Wx::Bitmap.new(");
        CheckLineLength(name.size() + 2);
        QuotedString(name);
        Comma().Str("Wx::BITMAP_TYPE_XPM)");
    }
}

void Code::GenerateBundleParameter(const tt_string_vector& parts, bool get_bitmap)
{
    if (parts.size() <= 1 || parts[IndexImage].empty())
    {
        Add("wxNullBitmap");
        return;
    }

    if (parts[IndexType].starts_with("SVG"))
    {
        GenerateSVGBundle(parts, get_bitmap);
    }
    else if (parts[IndexType].contains("Art"))
    {
        GenerateARTBundle(parts, get_bitmap);
    }
    else if (parts[IndexType].starts_with("Embed"))
    {
        GenerateEmbedBundle(parts, get_bitmap);
    }
    else if (parts[IndexType].starts_with("XPM"))
    {
        GenerateXpmBitmap(parts, get_bitmap);
    }
}
