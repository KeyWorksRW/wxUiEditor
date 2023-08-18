/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Ruby code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <set>
#include <unordered_set>

#include "mainframe.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "code.h"             // Code -- Helper class for generating code
#include "file_codewriter.h"  // FileCodeWriter -- Classs to write code to disk
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"       // Common component functions
#include "gen_results.h"      // Code generation file writing functions
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Miscellaneous utilities
#include "write_code.h"       // Write code to Scintilla or file

#include "pugixml.hpp"

using namespace code;
using namespace GenEnum;

// clang-format off

inline constexpr const auto txt_PyPerlRubyCmtBlock =
R"===(###############################################################################
# Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
#
# Do not edit any code above the "End of generated code" comment block.
# Any changes before that block will be lost if it is re-generated!
###############################################################################

)===";

inline constexpr const auto txt_ruby_get_bundle =
R"===(
# Loads image(s) from a string and returns a Wx::BitmapBundle object.
def get_bundle(image_name1, image_name2 = nil, image_name3 = nil)
  image1 = Wx::Image.new
  image1.load_stream(StringIO.new(image_name1))
  if (image_name2)
    image2 = Wx::Image.new
    image2.load_stream(StringIO.new(image_name2))
    if (image_name3)
      image3 = Wx::Image.new
      image3.load_stream(StringIO.new(image_name3))
      bitmaps = [image1, image2, image3]
      bundle = Wx::BitmapBundle.new.from_bitmaps(bitmaps)
      return bundle
    else
      bundle = Wx::BitmapBundle.new.from_bitmaps(image1, image2)
      return bundle
    end
  end
  bundle = Wx::BitmapBundle.new(image1)
  return bundle
end
)===";
// clang-format on

// This *must* be written on a line by itself with *no* indentation.
const char* ruby_begin_cmt_block = "=begin";

// This *must* be written on a line by itself with *no* indentation.
const char* ruby_end_cmt_block = "=end";

#if defined(_DEBUG)
static const std::vector<tt_string> disable_list = {
    "Metrics/MethodLength",
    "Metrics/ParameterLists",
    "Style/Documentation",
    "Metrics/AbcSize",
};
#endif  // _DEBUG

bool GenerateRubyFiles(GenResults& results, std::vector<tt_string>* pClassList)
{
    if (Project.getChildCount() == 0)
    {
        wxMessageBox("You cannot generate any code until you have added a top level form.", "Code Generation");
        return false;
    }
    tt_cwd cwd(true);
    Project.ChangeDir();
    tt_string path;

    bool generate_result = true;
    std::vector<Node*> forms;
    Project.CollectForms(forms);

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    results.StartClock();
#endif

    for (const auto& form: forms)
    {
        if (auto& base_file = form->as_string(prop_ruby_file); base_file.size())
        {
            path = Project.getBaseDirectory(form, GEN_LANG_RUBY);
            if (path.size())
            {
                path.append_filename(base_file);
            }
            else
            {
                path = base_file;
            }

            path.make_absolute();
            path.backslashestoforward();
        }
        else
        {
#if !defined(_DEBUG)
            // For a lot of wxRuby testing of projects with multiple dialogs, there may
            // only be a few forms where wxRuby generation is being tested, so don't nag in
            // Debug builds. :-)
            results.msgs.emplace_back() << "No Ruby filename specified for " << form->as_string(prop_class_name) << '\n';
#endif  // _DEBUG
            continue;
        }

        try
        {
            BaseCodeGenerator codegen(GEN_LANG_RUBY);

            auto h_cw = std::make_unique<FileCodeWriter>(path);
            h_cw->SetTabToSpaces(2);
            codegen.SetHdrWriteCode(h_cw.get());

            // REVIEW: [Randalphwa - 07-13-2023] The .rb extension should work, however on
            // Windows, a .rbw extension can be used as well to launch in a new console window.
            path.replace_extension(".rb");
            auto cpp_cw = std::make_unique<FileCodeWriter>(path);
            cpp_cw->SetTabToSpaces(2);
            codegen.SetSrcWriteCode(cpp_cw.get());

            codegen.GenerateRubyClass(form);
            int flags = flag_no_ui;
            if (pClassList)
                flags |= flag_test_only;
            auto retval = cpp_cw->WriteFile(GEN_LANG_RUBY, flags);

            if (retval > 0)
            {
                if (!pClassList)
                {
                    results.updated_files.emplace_back(path);
                }
                else
                {
                    if (form->isGen(gen_Images))
                        pClassList->emplace_back(GenEnum::map_GenNames[gen_Images]);
                    else
                        pClassList->emplace_back(form->as_string(prop_class_name));
                    continue;
                }
            }

            else if (retval < 0)
            {
                results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
            }
            else  // retval == result::exists
            {
                ++results.file_count;
            }
        }
        catch (const std::exception& TESTING_PARAM(e))
        {
            MSG_ERROR(e.what());
            wxMessageBox(tt_string("An internal error occurred generating code files for ")
                             << form->as_string(prop_ruby_file),
                         "Code generation");
            continue;
        }
    }

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    results.EndClock();
#endif

    return generate_result;
}

// Equivalent to GenerateBaseClass in gen_base.cpp

void BaseCodeGenerator::GenerateRubyClass(Node* form_node, PANEL_PAGE panel_type)
{
    Code code(form_node, GEN_LANG_RUBY);

    m_embedded_images.clear();

    m_form_node = form_node;
    m_ImagesForm = nullptr;

    for (const auto& form: Project.getChildNodePtrs())
    {
        if (form->isGen(gen_folder))
        {
            for (const auto& child_form: form->getChildNodePtrs())
            {
                if (child_form->isGen(gen_Images))
                {
                    m_ImagesForm = child_form.get();
                    break;
                }
            }
            break;
        }

        else if (form->isGen(gen_Images))
        {
            m_ImagesForm = form.get();
            break;
        }
    }

    EventVector events;
    std::thread thrd_get_events(&BaseCodeGenerator::CollectEventHandlers, this, form_node, std::ref(events));

    // Caution! CollectImageHeaders() needs access to m_baseFullPath, so don't start this
    // thread until it has been set!
    //
    // thrd_collect_img_headers will populate m_embedded_images;

    std::set<std::string> img_include_set;
    std::thread thrd_collect_img_headers(&BaseCodeGenerator::CollectImageHeaders, this, form_node,
                                         std::ref(img_include_set));

    // If the code files are being written to disk, then UpdateEmbedNodes() has already been called.
    if (panel_type != NOT_PANEL)
    {
        ProjectImages.UpdateEmbedNodes();
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();
    m_source->SetTabToSpaces(2);
    m_source->SetLastLineBlank();

    if (m_panel_type == NOT_PANEL)
    {
        m_source->writeLine(txt_PyPerlRubyCmtBlock);

        if (Project.as_bool(prop_disable_rubo_cop))
        {
#if defined(_DEBUG)
            for (auto& iter: disable_list)
            {
                m_source->writeLine("# rubocop:disable " + iter);
            }
#else
            m_source->writeLine("# rubocop:disable all");
#endif
            m_source->writeLine();
        }
    }

    if (form_node->isGen(gen_Images))
    {
        thrd_get_events.join();
        thrd_collect_img_headers.join();
        GenerateRubyImagesForm();
        return;
    }

    m_header->writeLine(tt_string("# Sample inherited class from ") << form_node->as_string(prop_class_name));
    m_header->writeLine();
    m_source->writeLine("WX_GLOBAL_CONSTANTS = true unless defined? WX_GLOBAL_CONSTANTS\n\nrequire 'wx/core'");
    m_header->writeLine("WX_GLOBAL_CONSTANTS = true unless defined? WX_GLOBAL_CONSTANTS\n\nrequire 'wx/core'");

    std::set<std::string> imports;

    auto GatherImportModules = [&](Node* node, auto&& GatherImportModules) -> void
    {
        if (auto* gen = node->getGenerator(); gen)
        {
            gen->GetRubyImports(node, imports);
        }
        for (auto& child: node->getChildNodePtrs())
        {
            GatherImportModules(child.get(), GatherImportModules);
        }
    };
    GatherImportModules(form_node, GatherImportModules);

    for (const auto& import: imports)
    {
        m_source->writeLine(import);
        m_header->writeLine(import);
    }
    m_source->writeLine();
    m_header->writeLine();

    if (form_node->hasValue(prop_relative_require_list))
    {
        tt_string_vector list;
        list.SetString(form_node->as_string(prop_relative_require_list));
        for (auto& iter: list)
        {
            iter.remove_extension();
            m_source->writeLine(tt_string("require_relative '") << iter << '\'');
        }
        if (list.size())
        {
            m_source->writeLine();
        }
    }

    if (form_node->isGen(gen_wxFrame) && form_node->as_bool(prop_import_all_dialogs))
    {
        for (auto& form: forms)
        {
            if ((form->isGen(gen_wxDialog) || form->isGen(gen_wxWizard)) && form->hasValue(prop_ruby_file))
            {
                tt_string import_name(form->as_string(prop_ruby_file).filename());
                import_name.remove_extension();
                m_source->writeLine(tt_string("require '") << import_name << "'");
            }
        }
    }

    m_set_enum_ids.clear();
    m_set_const_ids.clear();
    BaseCodeGenerator::CollectIDs(form_node, m_set_enum_ids, m_set_const_ids);

    int id_value = wxID_HIGHEST;
    for (auto& iter: m_set_enum_ids)
    {
        m_source->writeLine(tt_string() << '$' << iter << " = " << id_value++);
    }
    for (auto& iter: m_set_const_ids)
    {
        if (tt::contains(iter, " wx"))
        {
            tt_string id = '$' + iter;
            id.Replace(" wx", " Wx::", true, tt::CASE::exact);
            m_source->writeLine(id);
        }
        else
        {
            m_source->writeLine('$' + iter);
        }
    }

    thrd_collect_img_headers.join();
    m_NeedImageFunction = false;
    if (m_embedded_images.size())
    {
        m_source->writeLine();

        // First see if we need to import the gen_Images List
        bool images_file_imported = false;
        bool svg_import_libs = false;
        for (auto& iter: m_embedded_images)
        {
            if (iter->form == m_ImagesForm)
            {
                if (!images_file_imported)
                {
                    tt_string import_name = iter->form->as_string(prop_python_file).filename();
                    import_name.remove_extension();
                    code.Str("import ").Str(import_name);
                    m_source->writeLine(code);
                    code.clear();
                    images_file_imported = true;
                }
                if (iter->type == wxBITMAP_TYPE_INVALID)
                {
                    // m_source->writeLine("import zlib");
                    // m_source->writeLine("import base64");
                    svg_import_libs = true;
                }
            }
            else if (!svg_import_libs)
            {
                // SVG images have a wxBITMAP_TYPE_INVALID type
                if (iter->type == wxBITMAP_TYPE_INVALID)
                {
                    // m_source->writeLine("require 'base64'");
                    // m_source->writeLine("require 'stringio'");
                    svg_import_libs = true;
                }
                if (iter->form != m_ImagesForm)
                {
                    // If the image isn't in the images file, then we need to add the base64 version
                    // of the bitmap
                    m_source->writeLine("require 'base64'");

                    // At this point we know that some method is required, but until we have
                    // processed all the images, we won't know if the images file is required.
                    // The images file provides it's own function for loading images, so we can
                    // use that if it's available.
                    m_NeedImageFunction = true;
                }
            }
        }
        if (m_NeedImageFunction)
        {
            if (images_file_imported)
                // The images file supplies the function we need
                m_NeedImageFunction = false;
            else
                // We have to provide our own method, and that requires this library
                m_source->writeLine("require 'stringio'");
        }
    }

    m_source->writeLine();
    m_header->writeLine();
    m_header->writeLine(tt_string("requires '") << form_node->as_string(prop_python_file) << "'\n");
    m_header->writeLine();

    if (m_form_node->hasValue(prop_ruby_insert))
    {
        tt_string convert(m_form_node->as_string(prop_ruby_insert));
        convert.Replace("@@", "\n", tt::REPLACE::all);
        tt_string_vector lines(convert, '\n', tt::TRIM::right);
        for (auto& line: lines)
        {
            m_source->doWrite(line);
            m_source->doWrite("\n");
        }
        m_source->doWrite("\n");
    }

    tt_string inherit_name = form_node->as_string(prop_ruby_inherit_name);
    if (inherit_name.empty())
    {
        inherit_name += " < " + form_node->as_string(prop_class_name);
    }
    if (inherit_name.size())
    {
        tt_string inherit("class ");
        inherit << inherit_name;
        inherit << form_node->as_string(prop_ruby_file) << "." << form_node->as_string(prop_class_name) << "):";

        m_header->writeLine(inherit);
        m_header->Indent();
        m_header->writeLine("def initialize(parent)");
        m_header->writeLine("end");
        m_header->Unindent();
        m_header->writeLine();
    }

    thrd_get_events.join();

    auto generator = form_node->getNodeDeclaration()->getGenerator();
    code.clear();
    if (generator->ConstructionCode(code))
    {
        m_source->writeLine(code);
        m_source->writeLine();
        m_source->Indent();
        m_source->Indent();

        id_value = wxID_HIGHEST;
        for (auto& iter: m_set_enum_ids)
        {
            m_source->writeLine(tt_string() << '@' << iter << id_value++);
        }

        if (id_value > 1)
        {
            // If at least one id was set, add a blank line
            m_source->writeLine();
        }
    }

    code.clear();
    if (generator->SettingsCode(code))
    {
        if (code.size())
        {
            m_source->writeLine(code);
            m_source->writeLine();
        }
    }

    if (form_node->getPropPtr(prop_window_extra_style))
    {
        code.clear();
        code.GenWindowSettings();
        if (code.size())
        {
            m_source->writeLine(code);
        }
    }

    m_source->SetLastLineBlank();
    for (const auto& child: form_node->getChildNodePtrs())
    {
        if (child->isGen(gen_wxContextMenuEvent))
            continue;
        GenConstruction(child.get());
    }

    code.clear();
    if (generator->AfterChildrenCode(code))
    {
        if (code.size())
        {
            m_source->writeLine();
            m_source->writeLine(code);
        }
    }

    // TODO: [Randalphwa - 07-13-2023] Need to figure out if wxRuby supports persistence

    if (events.size())
    {
        m_source->writeLine();
        m_source->writeLine("# Event handlers");
        GenSrcEventBinding(form_node, events);
        m_source->writeLine("\tend", indent::none);
        m_source->SetLastLineBlank();

        m_source->ResetIndent();
        m_source->writeLine();
        m_source->Indent();
        GenRubyEventHandlers(events);
    }
    else
    {
        m_source->ResetIndent();
        m_source->writeLine("\tend", indent::none);
    }

    if (m_NeedImageFunction)
    {
        m_source->doWrite("\n");  // force an extra line break
        m_source->writeLine(txt_ruby_get_bundle, indent::auto_keep_whitespace);
    }

    if (m_form_node->isGen(gen_wxWizard))
    {
        code.clear();
        // see for an example C:\rwCode\wxRuby3\samples\dialogs\wizard.rb
        // w = MyWizard.new(self)
        // w.run_wizard(w.get_page_area_sizer.get_item(0).get_window)
    }

    // Make certain indentation is reset after all construction code is written
    m_source->ResetIndent();
    m_source->writeLine("end\n", indent::none);

    m_header->ResetIndent();

    code.clear();
    // Now write any embedded images that aren't declared in the gen_Images List
    for (auto& iter: m_embedded_images)
    {
        // Only write the images that aren't declared in any gen_Images List. Note that
        // this *WILL* result in duplicate images being written to different forms.
        if (iter->form != m_ImagesForm)
        {
            WriteImageConstruction(code);
            m_source->doWrite("\n");  // force an extra line break
            m_source->SetLastLineBlank();
            break;
        }
    }

    if (m_panel_type == NOT_PANEL)
    {
        if (Project.as_bool(prop_disable_rubo_cop))
        {
            m_source->writeLine();
#if defined(_DEBUG)
            for (auto& iter: disable_list)
            {
                m_source->writeLine("# rubocop:enable " + iter);
            }
#else
            m_source->writeLine("# rubocop:enable all");
#endif  // _DEBUG
            m_source->writeLine();
        }
    }
}

tt_string MakeRubyPath(Node* node)
{
    tt_string path;
    Node* form = node->getForm();

    if (auto& base_file = form->as_string(prop_ruby_file); base_file.size())
    {
        path = Project.getBaseDirectory(form, GEN_LANG_RUBY);
        if (path.size())
        {
            path.append_filename(base_file);
        }
        else
        {
            path = base_file;
        }

        path.make_absolute();
        path.backslashestoforward();
    }

    if (path.empty())
        path = "./";
    path.make_absolute();
    path.remove_filename();
    return path;
}

bool RubyBundleCode(Code& code, GenEnum::PropName prop)
{
    auto& description = code.node()->as_string(prop);
    if (description.empty())
    {
        code.Add("wxNullBitmap");
        return false;
    }

    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts.size() <= 1 || parts[IndexImage].empty())
    {
        code.Add("wxNullBitmap");
        return false;
    }

    if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code.Str("Wx::ArtProvider.get_bitmap_bundle(").Add(art_id);

        // Note that current documentation states that the client is required, but the header file says otherwise
        if (art_client.size())
            code.Comma().Add(art_client);
        code << ')';
        return true;
    }

    if (const ImageBundle* bundle = ProjectImages.GetPropertyImageBundle(description);
        bundle && bundle->lst_filenames.size())
    {
        if (description.starts_with("SVG"))
        {
            // TODO: [Randalphwa - 08-17-2023] This is waiting for wxRuby3 to implement the .from_...() methods
#if 0
            auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            ASSERT(embed);
            tt_string svg_name;
            if (embed->form != code.node()->getForm())
            {
                svg_name = embed->form->as_string(prop_python_file).filename();
                svg_name.remove_extension();
                svg_name << '.' << embed->array_name;
            }
            else
            {
                svg_name = embed->array_name;
            }
            code.insert(0, tt_string("_svg_string_ = zlib.decompress(base64.b64decode(") << svg_name << "))\n");
            code += "wx.BitmapBundle.FromSVG(_svg_string_";
            wxSize svg_size { -1, -1 };
            if (parts[IndexSize].size())
            {
                svg_size = GetSizeInfo(parts[IndexSize]);
            }
            code.Comma().Add("wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
#else
            return false;
#endif
        }

        else if (parts[IndexType].starts_with("Embed"))
        {
            if (bundle->lst_filenames.empty())
            {
                code.Add("wxNullBitmap");
                return false;
            }
            if (const EmbeddedImage* embed1 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed1)
            {
                code.Str("get_bundle(").Str("$").Str(embed1->array_name);
                if (bundle->lst_filenames.size() > 1)
                {
                    if (EmbeddedImage* embed2 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed2)
                    {
                        code.Comma().Str("$").Str(embed2->array_name);
                    }
                    if (bundle->lst_filenames.size() > 2)
                    {
                        if (EmbeddedImage* embed3 = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[2]); embed3)
                        {
                            code.Comma().Str("$").Str(embed3->array_name);
                        }
                    }
                }
                code += ')';
                return true;
            }
        }
        else if (bundle->lst_filenames.size() == 1)
        {
            auto path = Project.getBaseDirectory(code.node(), GEN_LANG_RUBY);

            tt_string name(bundle->lst_filenames[0]);
            name.make_absolute();
            name.make_relative(path);
            name.backslashestoforward();

            code.CheckLineLength(name.size() + name.size() + 27);
            code.Str("Wx::Bitmap(").QuotedString(name).Str("))");
        }
        else if (bundle->lst_filenames.size() == 2)
        {
            auto path = Project.getBaseDirectory(code.node(), GEN_LANG_RUBY);

            tt_string name(bundle->lst_filenames[0]);
            name.make_absolute();
            name.make_relative(path);
            name.backslashestoforward();

            tt_string name2(bundle->lst_filenames[1]);
            name2.make_absolute();
            name2.make_relative(path);
            name2.backslashestoforward();

            code.CheckLineLength(name.size() + name2.size() + 27);
            code.Str("Wx::Bitmap(").QuotedString(name).Str(", Wx::Bitmap(").QuotedString(name2).Str("))");
        }

        else
        {
            FAIL_MSG("Unexpected number of images in bundle -- should be <= 2");
            code.Add("wxNullBitmap");
            return false;
        }
    }
    else
    {
        FAIL_MSG("Missing bundle description");
        code.Add("wxNullBitmap");
        return false;
    }

    return false;
}
