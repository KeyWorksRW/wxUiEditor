/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code::Bundle() functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "code.h"

#include "gen_common.h"       // Common component functions
#include "image_gen.h"        // Functions for generating embedded images
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Miscellaneous utilities

Code& Code::Bundle(GenEnum::PropName prop_name)
{
    tt_string_vector parts(m_node->as_string(prop_name), BMP_PROP_SEPARATOR, tt::TRIM::both);
    if (parts[IndexType].contains("SVG"))
    {
        GenerateBundleParameter(*this, parts);
    }

    else
    {
        switch (m_language)
        {
            case GEN_LANG_PYTHON:
                BundlePython(parts);
                break;

            case GEN_LANG_RUBY:
                BundleRuby(parts);
                break;

            case GEN_LANG_PERL:
                BundlePerl(parts);
                break;

            case GEN_LANG_RUST:
                // RustBundleCode(*this, prop_name);
                break;

#if GENERATE_NEW_LANG_CODE
            case GEN_LANG_HASKELL:
                // HaskellBundleCode(*this, prop_name);
                break;

            case GEN_LANG_LUA:
                // LuaBundleCode(*this, prop_name);
                break;
#endif

            default:
                break;
        }
    }

    return *this;
}

void Code::BundlePerl(const tt_string_vector& parts)
{
    if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        *this << "Wx::ArtProvider::GetBitmap(" << art_id;
        if (art_client.size())
            *this << ", " << art_client;
        wxSize art_size { 16, 16 };
        art_size = GetSizeInfo(parts[IndexSize]);

        if (parts.size() > IndexSize && parts[IndexSize].size())
        {
            wxSize svg_size { -1, -1 };
            svg_size = GetSizeInfo(parts[IndexSize]);

            if (svg_size != wxDefaultSize)
            {
                art_size = svg_size;
            }
        }
        CheckLineLength(sizeof(", Wx::Size->new((999, 999)))"));

        *this << ", Wx::Size->new(" << art_size.x << ", " << art_size.y << "))";
        return;
    }

    auto path = MakePerlPath(node());

    // TODO: [Randalphwa - 06-30-2025] wxPerl3 currently does not support SVG images, so we need to
    // do something here...

    if (auto bundle = ProjectImages.GetPropertyImageBundle(parts);
        bundle && bundle->lst_filenames.size())
    {
        tt_string name(bundle->lst_filenames[0]);
        name.make_absolute();
        if (!name.file_exists())
        {
            name = Project.ArtDirectory();
            name.append_filename(bundle->lst_filenames[0]);
        }
        name.make_relative(path);
        name.backslashestoforward();

        if (parts[IndexType].contains("XPM"))
        {
            Str("Wx::Bitmap->new(");
            CheckLineLength(name.size() + 3);
            QuotedString(name).Comma().Str("wxBITMAP_TYPE_XPM)");
        }

        else if (bundle->lst_filenames.size() == 1)
        {
            *this += "Wx::BitmapBundle.FromBitmap(";
            bool is_embed_success = false;

            if (parts[IndexType].starts_with("Embed"))
            {
                if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                {
                    CheckLineLength(embed->imgs[0].array_name.size() + sizeof(".Bitmap)"));
                    AddPerlImageName(embed);
                    *this += ".Bitmap)";
                    is_embed_success = true;
                }
            }

            if (!is_embed_success)
            {
                CheckLineLength(name.size() + sizeof("Wx::Bitmap->new()"));
                Str("Wx::Bitmap->new(").QuotedString(name) += "))";
            }
        }
    }
}

void Code::BundlePython(const tt_string_vector& parts)
{
    if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        Add("wxArtProvider.GetBitmapBundle(").Add(art_id);

        // Note that current documentation states that the client is required, but the header file
        // says otherwise
        if (art_client.size())
            Comma().Add(art_client);

        if (parts.size() > IndexSize && parts[IndexSize].size())
        {
            wxSize svg_size { -1, -1 };
            svg_size = GetSizeInfo(parts[IndexSize]);

            if (svg_size != wxDefaultSize)
            {
                Comma();
                CheckLineLength(sizeof("wx.Size(999, 999)))"));
                *this << "wx.Size(" << svg_size.x << ", " << svg_size.y << ')';
            }
        }

        *this << ')';
        return;
    }

    auto path = MakePythonPath(node());

    if (auto bundle = ProjectImages.GetPropertyImageBundle(parts);
        bundle && bundle->lst_filenames.size())
    {
        tt_string name(bundle->lst_filenames[0]);
        name.make_absolute();
        if (!name.file_exists())
        {
            name = Project.ArtDirectory();
            name.append_filename(bundle->lst_filenames[0]);
        }
        name.make_relative(path);
        name.backslashestoforward();

        if (parts[IndexType].contains("SVG"))
        {
            auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            ASSERT(embed);
            tt_string svg_name;
            if (embed->form != node()->getForm())
            {
                svg_name = embed->form->as_string(prop_python_file).filename();
                svg_name.remove_extension();
                svg_name << '.' << embed->imgs[0].array_name;
            }
            else
            {
                svg_name = embed->imgs[0].array_name;
            }
            insert(0, tt_string("_svg_string_ = zlib.decompress(base64.b64decode(")
                          << svg_name << "))\n");
            *this += "wx.BitmapBundle.FromSVG(_svg_string_";
            wxSize svg_size { -1, -1 };
            if (parts[IndexSize].size())
            {
                svg_size = GetSizeInfo(parts[IndexSize]);
            }
            Comma().Str("wx.Size(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
        }

        else if (parts[IndexType].contains("XPM"))
        {
            CheckLineLength(name.size() + sizeof("wx.Bitmap()") + sizeof("wx.BITMAP_TYPE_XPM)"));
            Str("wx.Bitmap(").QuotedString(name).Comma().Str("wx.BITMAP_TYPE_XPM)");
        }

        else if (bundle->lst_filenames.size() == 1)
        {
            *this += "wx.BitmapBundle.FromBitmap(";
            bool is_embed_success = false;

            if (parts[IndexType].starts_with("Embed"))
            {
                if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                {
                    CheckLineLength(embed->imgs[0].array_name.size() + sizeof(".Bitmap)"));
                    AddPythonImageName(embed);
                    *this += ".Bitmap)";
                    is_embed_success = true;
                }
            }

            if (!is_embed_success)
            {
                CheckLineLength(name.size() + sizeof("wx.Bitmap()"));
                Str("wx.Bitmap(").QuotedString(name) += "))";
            }
        }
        else if (bundle->lst_filenames.size() == 2)
        {
            *this += "wx.BitmapBundle.FromBitmaps(";
            bool is_embed_success = false;

            if (parts[IndexType].starts_with("Embed"))
            {
                if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                {
                    CheckLineLength(embed->imgs[0].array_name.size() + sizeof(".Bitmap"));
                    AddPythonImageName(embed);

                    *this += ".Bitmap";

                    if (auto embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]);
                        embed2)
                    {
                        Comma().CheckLineLength(embed2->imgs[0].array_name.size() +
                                                sizeof(".Bitmap)"));
                        AddPythonImageName(embed2);
                        *this += ".Bitmap)";
                    }
                    else
                    {
                        Comma().Str("wx.NullBitmap)");
                    }
                    is_embed_success = true;
                }
            }
            if (!is_embed_success)
            {
                tt_string name2(bundle->lst_filenames[1]);
                name2.make_absolute();
                name2.make_relative(path);
                name2.backslashestoforward();

                CheckLineLength(name.size() + name2.size() + 27);
                Str("wx.Bitmap(")
                    .QuotedString(name)
                    .Str(", wx.Bitmap(")
                    .QuotedString(name2)
                    .Str("))");
            }
        }
        else
        {
            *this += "wx.BitmapBundle.FromBitmaps([";
            for (size_t idx = 0; idx < bundle->lst_filenames.size(); ++idx)
            {
                if (parts[IndexType].starts_with("Embed"))
                {
                    if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[idx]);
                        embed)
                    {
                        CheckLineLength(embed->imgs[0].array_name.size() + sizeof(".Bitmap"));
                        AddPythonImageName(embed);

                        *this += ".Bitmap";
                        if (idx < bundle->lst_filenames.size() - 1)
                        {
                            Comma();
                        }
                    }
                }
            }
            *this += "])";
        }
    }
    else
    {
        FAIL_MSG("Missing bundle description");
        Add("wxNullBitmap");
    }

    return;
}

void Code::AddPythonImageName(const EmbeddedImage* embed)
{
    if (embed->form->isGen(gen_Images))
    {
        tt_string import_name = embed->form->as_string(prop_python_file).filename();
        import_name.remove_extension();

        Str(import_name).Str(".");
    }
    Str(embed->imgs[0].array_name);
}

void Code::AddPerlImageName(const EmbeddedImage* embed)
{
    if (embed->form->isGen(gen_Images))
    {
        tt_string import_name = embed->form->as_string(prop_perl_file).filename();
        import_name.remove_extension();

        Str(import_name).Str(".");
    }
    Str(embed->imgs[0].array_name);
}

void Code::BundleRuby(const tt_string_vector& parts)
{
    if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        Str("Wx::ArtProvider.get_bitmap_bundle(").Add(art_id);

        // Note that current documentation states that the client is required, but the header file
        // says otherwise
        if (art_client.size())
            Comma().Add(art_client);

        if (parts.size() > IndexSize && parts[IndexSize].size())
        {
            wxSize svg_size { -1, -1 };
            svg_size = GetSizeInfo(parts[IndexSize]);

            if (svg_size != wxDefaultSize)
            {
                Comma();
                CheckLineLength(sizeof("Wx::Size.new(999, 999)))"));
                *this << "Wx::Size.new(" << svg_size.x << ", " << svg_size.y << ')';
            }
        }
        *this << ')';
        return;
    }

    if (auto bundle = ProjectImages.GetPropertyImageBundle(parts);
        bundle && bundle->lst_filenames.size())
    {
        if (parts[IndexType].contains("SVG"))
        {
            auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            ASSERT(embed);
            tt_string svg_name;
            if (embed->form != node()->getForm())
            {
                svg_name = embed->form->as_string(prop_ruby_file).filename();
                svg_name.remove_extension();
                svg_name << ".$" << embed->imgs[0].array_name;
            }
            else
            {
                svg_name = "$" + embed->imgs[0].array_name;
            }
            insert(0, tt_string("_svg_string_ = Zlib::Inflate.inflate(Base64.decode64(")
                          << svg_name << "))\n");
            *this += "Wx::BitmapBundle.from_svg(_svg_string_";
            wxSize svg_size { -1, -1 };
            if (parts[IndexSize].size())
            {
                svg_size = GetSizeInfo(parts[IndexSize]);
            }
            Comma().Str("Wx::Size.new(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
        }

        else if (parts[IndexType].contains("XPM"))
        {
            auto path = MakeRubyPath(node());
            tt_string name(bundle->lst_filenames[0]);
            name.make_absolute();
            if (!name.file_exists())
            {
                name = Project.ArtDirectory();
                name.append_filename(bundle->lst_filenames[0]);
            }
            name.make_relative(path);
            name.backslashestoforward();

            CheckLineLength(name.size() + sizeof("Wx::Bitmap.new()") +
                            sizeof("wx.BITMAP_TYPE_XPM)"));
            Str("Wx::Bitmap.new(").QuotedString(name).Comma().Str("Wx::BITMAP_TYPE_XPM)");
        }

        else if (parts[IndexType].starts_with("Embed"))
        {
            if (bundle->lst_filenames.empty())
            {
                Add("wxNullBitmap");
                return;
            }
            if (const EmbeddedImage* embed1 =
                    ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                embed1)
            {
                Str("wxue_get_bundle(").Str("$").Str(embed1->imgs[0].array_name);
                if (bundle->lst_filenames.size() > 1)
                {
                    if (EmbeddedImage* embed2 =
                            ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]);
                        embed2)
                    {
                        Comma().Str("$").Str(embed2->imgs[0].array_name);
                    }
                    if (bundle->lst_filenames.size() > 2)
                    {
                        if (EmbeddedImage* embed3 =
                                ProjectImages.GetEmbeddedImage(bundle->lst_filenames[2]);
                            embed3)
                        {
                            Comma().Str("$").Str(embed3->imgs[0].array_name);
                        }
                    }
                }
                *this += ')';
                return;
            }
        }
        else if (bundle->lst_filenames.size() == 1)
        {
            auto path = Project.getBaseDirectory(node(), GEN_LANG_RUBY);

            tt_string name(bundle->lst_filenames[0]);
            name.make_absolute();
            name.make_relative(path);
            name.backslashestoforward();

            CheckLineLength(name.size() + name.size() + 27);
            Str("Wx::Bitmap(").QuotedString(name).Str("))");
        }
        else if (bundle->lst_filenames.size() == 2)
        {
            auto path = Project.getBaseDirectory(node(), GEN_LANG_RUBY);

            tt_string name(bundle->lst_filenames[0]);
            name.make_absolute();
            name.make_relative(path);
            name.backslashestoforward();

            tt_string name2(bundle->lst_filenames[1]);
            name2.make_absolute();
            name2.make_relative(path);
            name2.backslashestoforward();

            CheckLineLength(name.size() + name2.size() + 27);
            Str("Wx::Bitmap(")
                .QuotedString(name)
                .Str(", Wx::Bitmap(")
                .QuotedString(name2)
                .Str("))");
        }

        else
        {
            FAIL_MSG("Unexpected number of images in bundle -- should be <= 2");
            Add("wxNullBitmap");
            return;
        }
    }
    else
    {
        FAIL_MSG("Missing bundle description");
        Add("wxNullBitmap");
        return;
    }

    return;
}
