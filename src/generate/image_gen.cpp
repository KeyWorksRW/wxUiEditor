/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for generating embedded images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>

#include <array>
#include <vector>

#include "image_gen.h"

#include "code.h"             // Code -- Helper class for generating code
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"       // Common component functions
#include "gen_enums.h"        // Enumerations for generators
#include "image_handler.h"    // ImageHandler class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // Write code to Scintilla or file

// Generate extern references to images used in the current form that are defined in the
// gen_Images node. These are written before the class constructor.
void BaseCodeGenerator::WriteImagePreConstruction(Code& code)
{
    ASSERT_MSG(code.is_cpp(), "This function is only used for C++ code generation");
    code.clear();

    bool is_namespace_written = false;
    for (auto iter_array: m_embedded_images)
    {
        // If the image is in ImagesForm then it's header file will be included which already
        // has the extern declarations.
        if (iter_array->form == Project.getImagesForm())
            continue;

        if (!is_namespace_written)
        {
            is_namespace_written = true;
            code.Str("namespace wxue_img").OpenBrace();
        }
        code.Eol(eol_if_needed).Str("extern const unsigned char ").Str(iter_array->imgs[0].array_name);
        code.Str("[").itoa((to_size_t) (iter_array->imgs[0].array_size & 0xFFFFFFFF)).Str("];");
        if (iter_array->imgs[0].filename.size())
        {
            code.Str("  // ").Str(iter_array->imgs[0].filename);
        }
    }

    if (is_namespace_written)
    {
        code.CloseBrace().Eol();
    }
}

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
        // if (iter_array->form == images_form)
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
            size_t max_pos = (iter_array->imgs[0].array_size & 0xFFFFFFFF);

            if (iter_array->imgs[0].filename.size())
            {
                code.Eol(eol_if_needed).Str("// ").Str(iter_array->imgs[0].filename);
            }
            code.Eol();
            code.Str("const unsigned char ").Str(iter_array->imgs[0].array_name);
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
                    code.itoa(iter_array->imgs[0].array_data[pos]) += ",";
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
            if (iter_array->form->isGen(gen_Images))
            {
                continue;
            }
            if (iter_array->imgs[0].filename.size())
            {
                code.Eol().Str("# ").Str(iter_array->imgs[0].filename);
            }
            code.Eol().Str(iter_array->imgs[0].array_name);
            if (iter_array->imgs[0].type == wxBITMAP_TYPE_SVG)
            {
                code.Str(" = (");
            }
            else
            {
                code.Str(" = PyEmbeddedImage(");
            }
            m_source->writeLine(code);
            code.clear();
            auto encoded = base64_encode(iter_array->imgs[0].array_data.get(), iter_array->imgs[0].array_size & 0xFFFFFFFF,
                                         GEN_LANG_PYTHON);
            if (encoded.size())
            {
                encoded.back() += ")";
                m_source->writeLine(encoded);
            }
        }
        else if (code.is_ruby())
        {
            if (iter_array->form->isGen(gen_Images))
            {
                continue;
            }
            if (iter_array->imgs[0].filename.size())
            {
                code.Eol().Str("# ").Str(iter_array->imgs[0].filename);
            }
            code.Eol().Str("$").Str(iter_array->imgs[0].array_name);
            if (iter_array->imgs[0].type == wxBITMAP_TYPE_SVG)
            {
                code.Str(" = (");
            }
            else
            {
                code.Str(" = Base64.decode64(");
            }
            m_source->writeLine(code);
            code.clear();
            auto encoded = base64_encode(iter_array->imgs[0].array_data.get(), iter_array->imgs[0].array_size & 0xFFFFFFFF,
                                         GEN_LANG_RUBY);
            if (encoded.size())
            {
                // Remove the trailing '+' character
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

void BaseCodeGenerator::WriteImagePostHeader()
{
    auto images_form = Project.getImagesForm();
    if (!images_form)
        return;

    bool is_namespace_written = false;
    for (auto iter_array: m_embedded_images)
    {
        if (iter_array->form == images_form)
            continue;

        if (!is_namespace_written)
        {
            m_header->writeLine();
            m_header->writeLine("namespace wxue_img\n{");

            m_header->Indent();
            is_namespace_written = true;
        }
        if (iter_array->imgs[0].filename.size())
        {
            m_header->writeLine(tt_string("// ") << iter_array->imgs[0].filename);
        }
        m_header->writeLine(tt_string("extern const unsigned char ")
                            << iter_array->imgs[0].array_name << '['
                            << (to_size_t) (iter_array->imgs[0].array_size & 0xFFFFFFFF) << "];");
    }

    if (is_namespace_written)
    {
        m_header->Unindent();
        m_header->writeLine("}\n");
    }
}

// clang-format off

std::map<GenLang, GenEnum::PropName> map_lang_to_prop = {

    { GEN_LANG_CPLUSPLUS, prop_cpp_line_length },
    { GEN_LANG_PERL, prop_perl_line_length },
    { GEN_LANG_PYTHON, prop_python_line_length },
    { GEN_LANG_RUBY, prop_ruby_line_length  },
    { GEN_LANG_RUST, prop_rust_line_length },

#if GENERATE_NEW_LANG_CODE
    { GEN_LANG_FORTRAN, prop_fortran_line_length },
    { GEN_LANG_HASKELL, prop_haskell_line_length },
    { GEN_LANG_LUA, prop_lua_line_length },
#endif  // GENERATE_NEW_LANG_CODE
};

// clang-format on

std::vector<std::string> base64_encode(unsigned char const* data, size_t data_size, GenLang language)
{
    size_t tab_quote_prefix = 7;  // 4 for tab, 2 for quotes, 1 for 'b' prefix
    if (language == GEN_LANG_RUBY)
        tab_quote_prefix = 6;  // 2 for tab, 2 for quotes, 2 for " +" suffix
    GenEnum::PropName prop = prop_python_line_length;
    if (auto result = map_lang_to_prop.find(language); result != map_lang_to_prop.end())
        prop = result->second;

    size_t line_length = Project.as_size_t(prop) - tab_quote_prefix;

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
        line_end = "' +";
    }

    line = line_begin;
    size_t line_pos = line_begin.size();
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
    line += line_end;
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

        if (iter_array->imgs[0].filename.size())
        {
            code.Eol().Str("# ").Str(iter_array->imgs[0].filename);
        }
        code.Eol().Str(iter_array->imgs[0].array_name);
        if (iter_array->imgs[0].type == wxBITMAP_TYPE_SVG)
        {
            code.Str(" = (");
        }
        else
        {
            code.Str(" = PyEmbeddedImage(");
        }

        m_source->writeLine(code);
        code.clear();
        auto encoded = base64_encode(iter_array->imgs[0].array_data.get(), iter_array->imgs[0].array_size & 0xFFFFFFFF,
                                     GEN_LANG_PYTHON);
        if (encoded.size())
        {
            encoded.back() += ")";
            m_source->writeLine(encoded);
        }
    }

    m_source->writeLine();
}

inline constexpr const auto txt_ruby_get_bundle =
    R"===(
# Loads image(s) from a string and returns a Wx::BitmapBundle object.
def wxue_get_bundle(image_name1, image_name2 = nil, image_name3 = nil)
  image1 = Wx::Image.new
  image1.load_stream(StringIO.new(image_name1))
  if (image_name2)
    image2 = Wx::Image.new
    image2.load_stream(StringIO.new(image_name2))
    if (image_name3)
      image3 = Wx::Image.new
      image3.load_stream(StringIO.new(image_name3))
      bitmaps = [Wx::Bitmap.new(image1),
                 Wx::Bitmap.new(image2),
                 Wx::Bitmap.new(image3)]
      bundle = Wx::BitmapBundle.from_bitmaps(bitmaps)
      return bundle
    else
      bundle = Wx::BitmapBundle.from_bitmaps(Wx::Bitmap.new(image1),
                                             Wx::Bitmap.new(image2))
      return bundle
    end
  end
  bundle = Wx::BitmapBundle.from_image(image1)
  return bundle
end
)===";

void BaseCodeGenerator::GenerateRubyImagesForm()
{
    if (m_embedded_images.empty() || !m_form_node->getChildCount())
    {
        return;
    }

    m_source->writeLine(txt_ruby_get_bundle, indent::auto_keep_whitespace);

    Code code(m_form_node, GEN_LANG_RUBY);

    for (auto iter_array: m_embedded_images)
    {
        if (iter_array->form != m_form_node)
            continue;

        if (iter_array->imgs[0].filename.size())
        {
            code.Eol().Str("# ").Str(iter_array->imgs[0].filename);
        }
        code.Eol().Str("$").Str(iter_array->imgs[0].array_name);
        if (iter_array->imgs[0].type == wxBITMAP_TYPE_SVG)
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
            base64_encode(iter_array->imgs[0].array_data.get(), iter_array->imgs[0].array_size & 0xFFFFFFFF, GEN_LANG_RUBY);
        if (encoded.size())
        {
            // Remove the trailing '+' character
            encoded.back().pop_back();
            // and the now trailing space
            encoded.back().pop_back();
            encoded.back() += ")";
            m_source->writeLine(encoded);
        }
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
    code.Str(embed->imgs[0].array_name);
}

// ******************************* Bundle Code Generation *******************************
//
// Call GenerateBundleParameter() to generate the code to create a wxBitmapBundle, or
// optionally a wxBitmap.
//
// **************************************************************************************

static void GenerateSVGBundle(Code& code, const tt_string_vector& parts, bool get_bitmap)
{
    if (code.is_cpp() && Project.is_wxWidgets31())
    {
        code.Eol().Tab().Str("wxNullBitmap /* SVG images require wxWidgets 3.2 or higher */").Eol().Tab();
        return;
    }

    wxSize svg_size { -1, -1 };
    if (parts[IndexSize].size())
    {
        svg_size = GetSizeInfo(parts[IndexSize]);
    }

    if (code.is_cpp())
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(parts); function_name.size())
        {
            // The function name includes the size, but we need to replace the size with a DIP version.
            function_name.erase_from("(");
            code.Eol().Tab().Str(function_name).Str("(FromDIP(").itoa(svg_size.x).Str("), FromDIP(").itoa(svg_size.y);
            code += "))";
            if (get_bitmap)
            {
                code.Str(".").Add("GetBitmap(").Add("wxDefaultSize)");
            }
            return;
        }
    }

    auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
    if (!embed)
    {
        MSG_WARNING(tt_string() << parts[IndexImage] << " not embedded!");
        code.Add("wxNullBitmap");
        return;
    }

    if (code.is_cpp())
    {
        tt_string name = "wxue_img::" + embed->imgs[0].array_name;
        code.Eol() << "\twxueBundleSVG(" << name << ", " << (to_size_t) (embed->imgs[0].array_size & 0xFFFFFFFF) << ", ";
        code.itoa((to_size_t) (embed->imgs[0].array_size >> 32)).Comma();
        if (get_bitmap)
        {
            code.FormFunction("FromDIP(").Add("wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y) += ")))";
            code.Str(".").Add("GetBitmap(").Add("wxDefaultSize)");
        }
        else
        {
            code.Add("FromDIP(wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y) += ")))";
        }
        return;
    }
    else if (code.is_python())
    {
        tt_string svg_name;

        if (embed->form != code.node()->getForm())
        {
            svg_name = embed->form->as_string(prop_python_file).filename();
            svg_name.remove_extension();
            svg_name << '.' << embed->imgs[0].array_name;
        }
        else
        {
            svg_name = embed->imgs[0].array_name;
        }
        code.insert(0, tt_string("_svg_string_ = zlib.decompress(base64.b64decode(") << svg_name << "))\n");
        code.Eol() += "\twx.BitmapBundle.FromSVG(_svg_string_";
    }
    else if (code.is_ruby())
    {
        tt_string svg_name;
        svg_name = "$" + embed->imgs[0].array_name;
        code.insert(0, tt_string("_svg_string_ = Zlib::Inflate.inflate(Base64.decode64(") << svg_name << "))\n");
        code += "Wx::BitmapBundle.from_svg(_svg_string_";
        code.Comma().Str("Wx::Size.new(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
    }

    if (get_bitmap)
    {
        if (!code.is_ruby())
        {
            code.Comma().CheckLineLength(sizeof("FromDIP(wx::Size.new(32, 32))).GetBitmap(wxDefaultSize)"));
            code.FormFunction("FromDIP(").Add("wxSize(");
            code.itoa(svg_size.x).Comma().itoa(svg_size.y) += ")))";
        }
        code.VariableMethod("GetBitmap(").AddType("wxDefaultSize").Str(")");
    }
    else
    {
        // wxSize already added above
        if (!code.is_ruby())
        {
            code.Comma().Add("wxSize(");
            code.itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
        }
    }
}

static void GenerateARTBundle(Code& code, const tt_string_vector& parts, bool get_bitmap)
{
    code.Add("wxArtProvider");
    if (get_bitmap || (code.is_cpp() && Project.is_wxWidgets31()))
    {
        code.ClassMethod("GetBitmap(");
    }
    else
    {
        code.ClassMethod("GetBitmapBundle(");
    }

    tt_string art_id(parts[IndexArtID]);
    tt_string art_client;
    if (auto pos = art_id.find('|'); tt::is_found(pos))
    {
        art_client = art_id.subview(pos + 1);
        art_id.erase(pos);
    }

    code.Add(art_id);
    // Note that current documentation states that the client is required, but the header file says otherwise
    if (art_client.size())
        code.Comma().Add(art_client);
    if (parts.size() > IndexSize)
    {
        code.Comma();
        code.CheckLineLength(sizeof("wxSize(999, 999)))"));
        auto size = GetSizeInfo(parts[IndexSize]);
        code.WxSize(size, code::no_scaling);
    }
    code << ')';
}

static void GenerateEmbedBundle(Code& code, const tt_string_vector& parts, bool get_bitmap)
{
    if (code.is_cpp())
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(parts); function_name.size())
        {
            code.Str(function_name);
            if (get_bitmap)
            {
                // BUGBUG: [Randalphwa - 09-19-2023] This is not correct. We need to get the
                // size of the embedded image and use that to get the bitmap including
                // rescaling it if it is a single image.
                code.Str(".").Add("GetBitmap(").Add("wxDefaultSize)");
            }
            return;
        }
    }

    auto bundle = ProjectImages.GetPropertyImageBundle(parts);
    if (!bundle || !bundle->lst_filenames.size())
    {
        MSG_WARNING(tt_string("Missing bundle for ") << parts[IndexImage]);
        code.Add("wxNullBitmap");
        return;
    }

    auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
    if (!embed)
    {
        FAIL_MSG(tt_string("Missing embed for ") << bundle->lst_filenames[0]);
        code.Add("wxNullBitmap");
        return;
    }

    if (code.is_ruby())
    {
        // Ruby has a single function that will create a bundle from 1 to 3 images
        code.Str("wxue_get_bundle(").Str("$").Str(embed->imgs[0].array_name);
        if (bundle->lst_filenames.size() > 1)
        {
            if (EmbeddedImage* embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed2)
            {
                code.Comma().Str("$").Str(embed2->imgs[0].array_name);
            }
            if (bundle->lst_filenames.size() > 2)
            {
                if (EmbeddedImage* embed3 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[2]); embed3)
                {
                    code.Comma().Str("$").Str(embed3->imgs[0].array_name);
                }
            }
        }
        code += ')';

        if (get_bitmap)
        {
            code.Str(".get_bitmap(").Eol().Tab(2).Str("Wx::Size.new(");
            code << ("from_dip(") << embed->size.x << "), from_dip(" << embed->size.y << ")))";
            // TODO: [Randalphwa - 09-19-2023] If it's a single image, then it may need to be rescaled
            // using wxIMAGE_QUALITY_BILINEAR rather than letting the wxBitmapBundle do it. However, the
            // only embedded images we support are bundles, so this probably isn't practical.
        }
        return;
    }

    tt_string path;
    if (code.is_python())
    {
        path = MakePythonPath(code.node());
    }
    else if (code.is_ruby())
    {
        path = MakeRubyPath(code.node());
    }

    tt_string name(bundle->lst_filenames[0]);
    name.make_absolute();
    name.make_relative(path);
    name.backslashestoforward();

    if (code.is_cpp() && get_bitmap)
    {
        code.Eol().Tab() << "wxueImage(";

        name = "wxue_img::" + embed->imgs[0].array_name;

        code << name << ", sizeof(" << name << "))";
        code << ".Rescale(";
        code.Eol() << "\tFromDIP(" << embed->size.x << "), FromDIP(" << embed->size.y << "), wxIMAGE_QUALITY_BILINEAR)";
    }
    else if (bundle->lst_filenames.size() == 1)
    {
        if (code.is_cpp())
        {
            code.Eol().Tab() << "wxueImage(";

            name = "wxue_img::" + embed->imgs[0].array_name;

            code << name << ", sizeof(" << name << "))";
        }
        else if (code.is_python())
        {
            if (get_bitmap)
            {
                code.Str("wx.Bitmap(");
            }
            AddPythonImageName(code, embed);
            code += get_bitmap ? ".Image" : ".Bitmap";
            if (get_bitmap)
            {
                code.Str(".Rescale(").Eol().Tab();
                code.FormFunction("FromDIP(").itoa(embed->size.x).Str("), ").FormFunction("FromDIP(");
                code.itoa(embed->size.y) << "), wx.IMAGE_QUALITY_BILINEAR))";
            }
        }
    }
    else if (bundle->lst_filenames.size() == 2)
    {
        code.Add("wxBitmapBundle").ClassMethod("FromBitmaps(");
        if (code.is_cpp())
        {
            code << "wxueImage(";
            name = "wxue_img::" + embed->imgs[0].array_name;
            code << name << ", sizeof(" << name << ")), wxueImage(";

            if (auto embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed2)
            {
                name = "wxue_img::" + embed2->imgs[0].array_name;
                name.remove_extension();
                code << name << ", sizeof(" << name << ")))";
            }
            else
            {
                code << "wxNullBitmap))";
            }
        }
        else if (code.is_python())
        {
            code.CheckLineLength(embed->imgs[0].array_name.size() + sizeof(".Bitmap)"));
            AddPythonImageName(code, embed);
            code += ".Bitmap";
            if (auto embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed2)
            {
                code.Comma().CheckLineLength(embed2->imgs[0].array_name.size() + sizeof(".Bitmap)"));
                AddPythonImageName(code, embed2);
                code += ".Bitmap";
            }
            else
            {
                code.Comma().Str("wx.NullBitmap");
            }
            if (get_bitmap)
            {
                code.CheckLineLength(sizeof("FromDIP(wx::Size.new(32, 32))).GetBitmap(wxDefaultSize)"));
                code.Str(").GetBitmap(").Add("wxSize(");
                code.Eol().Tab().FormFunction("FromDIP(").itoa(embed->size.x).Str("), ").FormFunction("FromDIP(");
                code.itoa(embed->size.y) << "))";
            }
            code << ')';
        }
    }
    else
    {
        if (code.is_cpp())
        {
            code.Indent();
            code.Eol().Str("[&]()");
            code.OpenBrace().Add("wxVector<wxBitmap> bitmaps;");

            for (auto& iter: bundle->lst_filenames)
            {
                tt_string name_img(iter.filename());
                name_img.remove_extension();
                name_img.Replace(".", "_", true);
                if (parts[IndexType].starts_with("Embed"))
                {
                    auto embed_img = ProjectImages.GetEmbeddedImage(iter);
                    if (embed_img)
                    {
                        name_img = "wxue_img::" + embed_img->imgs[0].array_name;
                    }
                }
                code.Eol().Str("bitmaps.push_back(wxueImage(") << name_img << ", sizeof(" << name_img << ")));";
            }
            code.Eol();
            code.Str("return wxBitmapBundle::FromBitmaps(bitmaps);");
            code.CloseBrace().Str("()").Eol();
        }
        else if (code.is_python())
        {
            bool is_xpm = (parts[IndexType].is_sameas("XPM"));

            code += "wx.BitmapBundle.FromBitmaps([ ";
            bool needs_comma = false;
            for (auto& iter: bundle->lst_filenames)
            {
                if (needs_comma)
                {
                    code.UpdateBreakAt();
                    code.Comma(false).Eol().Tab(3);
                }

                bool is_embed_success = false;
                if (auto embed_img = ProjectImages.GetEmbeddedImage(iter); embed_img)
                {
                    AddPythonImageName(code, embed_img);
                    code += ".Bitmap";
                    needs_comma = true;
                    is_embed_success = true;
                }

                if (!is_embed_success)
                {
                    tt_string name_img(iter);
                    name.make_absolute();
                    name.make_relative(path);
                    name.backslashestoforward();

                    code.Str("wx.Bitmap(").QuotedString(name);
                    if (is_xpm)
                        code.Comma().Str("wx.BITMAP_TYPE_XPM");
                    code += ")";
                    needs_comma = true;
                }
            }
            code += " ])";
            if (get_bitmap)
            {
                code.CheckLineLength(sizeof("FromDIP(wx::Size.new(32, 32))).GetBitmap(wxDefaultSize)"));
                code.Str(".GetBitmap(").Add("wxSize(");
                code.FormFunction("FromDIP(").itoa(embed->size.x).Str("), ").FormFunction("FromDIP(");
                code.itoa(embed->size.y) << ")))";
            }
        }
    }
}

static void GenerateXpmBitmap(Code& code, const tt_string_vector& parts, bool /* get_bitmap */)
{
    // We only marginally support XPM files -- we only allow a single file, and we don't attempt to scale it.

    tt_string name(parts[IndexImage].filename());
    if (code.is_cpp())
    {
        code.Str("wxBitmap(");
        name.remove_extension();
        code << name << "_xpm)";
    }
    else if (code.is_python())
    {
        auto path = MakePythonPath(code.node());
        name.make_absolute();
        if (!name.file_exists())
        {
            name = Project.ArtDirectory();
            name.append_filename(parts[IndexImage].filename());
        }
        name.make_relative(path);
        name.backslashestoforward();

        code.Str("wx.Bitmap(");
        code.QuotedString(name);
        code.Comma().Str("wx.BITMAP_TYPE_XPM)");
    }
    else if (code.is_ruby())
    {
        auto path = MakeRubyPath(code.node());
        name.make_absolute();
        if (!name.file_exists())
        {
            name = Project.ArtDirectory();
            name.append_filename(parts[IndexImage].filename());
        }
        name.make_relative(path);
        name.backslashestoforward();

        code.Str("Wx::Bitmap.new(");
        code.QuotedString(name);
        code.Comma().Str("Wx::BITMAP_TYPE_XPM)");
    }
}

void GenerateBundleParameter(Code& code, const tt_string_vector& parts, bool get_bitmap)
{
    if (parts.size() <= 1 || parts[IndexImage].empty())
    {
        code.Add("wxNullBitmap");
        return;
    }

    if (parts[IndexType].starts_with("SVG"))
    {
        if (code.is_cpp() && Project.is_wxWidgets31())
        {
            code += "wxNullBitmap";
        }
        else
        {
            GenerateSVGBundle(code, parts, get_bitmap);
        }
    }
    else if (parts[IndexType].contains("Art"))
    {
        GenerateARTBundle(code, parts, get_bitmap);
    }
    else if (parts[IndexType].starts_with("Embed"))
    {
        GenerateEmbedBundle(code, parts, get_bitmap);
    }
    else if (parts[IndexType].starts_with("XPM"))
    {
        GenerateXpmBitmap(code, parts, get_bitmap);
    }
}
