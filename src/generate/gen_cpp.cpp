/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <set>
#include <thread>

#include "gen_cpp.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "code.h"             // Code -- Helper class for generating code
#include "data_handler.h"     // DataHandler class
#include "file_codewriter.h"  // FileCodeWriter -- Classs to write code to disk
#include "gen_common.h"       // Common component functions
#include "gen_enums.h"
#include "gen_results.h"      // Code generation file writing functions
#include "gen_timer.h"        // TimerGenerator class
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_view_vector -- Read/Write line-oriented strings/files
#include "utils.h"            // Miscellaneous utilities
#include "write_code.h"       // Write code to Scintilla or file

#include "../customprops/eventhandler_dlg.h"  // EventHandlerDlg static functions

using namespace code;

namespace
{
    class GenData
    {
    public:
        GenData(GenResults& results, std::vector<tt_string>* pClassList) :
            pClassList(pClassList), m_results(&results)
        {
        }

        tt_string source_ext;
        tt_string header_ext;
        std::vector<tt_string>* pClassList;

        void AddUpdateFilename(tt_string& path) const
        {
            m_results->updated_files.emplace_back(path);
        };

        void AddResultMsg(tt_string& msg) const { m_results->msgs.emplace_back(msg); };

        void UpdateFileCount() const { m_results->file_count += 1; };

        void AddClassName(const tt_string& class_name) const
        {
            if (pClassList)
            {
                pClassList->emplace_back(class_name);
            }
        };

    private:
        GenResults* m_results { nullptr };
    };
}  // namespace

namespace
{
    void GenCppForm(GenData& gen_data, Node* form);
}

extern std::string_view cpp_rust_end_cmt_line;  // "// ************* End of generated code"

const std::map<wxBitmapType, std::string_view> g_map_handlers = {
    { wxBITMAP_TYPE_ICO, "wxICOHandler" },   { wxBITMAP_TYPE_CUR, "wxCURHandler" },
    { wxBITMAP_TYPE_XPM, "wxXPMHandler" },
#ifndef __WXOSX__
    { wxBITMAP_TYPE_TIFF, "wxTIFFHandler" },
#endif
    { wxBITMAP_TYPE_GIF, "wxGIFHandler" },   { wxBITMAP_TYPE_PNG, "wxPNGHandler" },
    { wxBITMAP_TYPE_JPEG, "wxJPEGHandler" }, { wxBITMAP_TYPE_PNM, "wxPNMHandler" },
    { wxBITMAP_TYPE_ANI, "wxANIHandler" },   { wxBITMAP_TYPE_WEBP, "wxWEBPHandler" }
};

const std::map<wxBitmapType, std::string_view> g_map_types = {
    { wxBITMAP_TYPE_BMP, "wxBITMAP_TYPE_BMP" },
    { wxBITMAP_TYPE_ICO, "wxBITMAP_TYPE_ICO" },
    { wxBITMAP_TYPE_CUR, "wxBITMAP_TYPE_CUR" },
    { wxBITMAP_TYPE_XPM, "wxBITMAP_TYPE_XPM" },
    { wxBITMAP_TYPE_ICO_RESOURCE, "wxBITMAP_TYPE_ICO_RESOURCE" },
#ifndef __WXOSX__
    { wxBITMAP_TYPE_TIFF, "wxBITMAP_TYPE_TIFF" },
#endif
    { wxBITMAP_TYPE_GIF, "wxBITMAP_TYPE_GIF" },
    { wxBITMAP_TYPE_PNG, "wxBITMAP_TYPE_PNG" },
    { wxBITMAP_TYPE_JPEG, "wxBITMAP_TYPE_JPEG" },
    { wxBITMAP_TYPE_PNM, "wxBITMAP_TYPE_PNM" },
    { wxBITMAP_TYPE_ANI, "wxBITMAP_TYPE_ANI" },
    { wxBITMAP_TYPE_WEBP, "wxBITMAP_TYPE_WEBP" }
};

// clang-format off

inline constexpr const auto txt_wxueImageFunction = R"===(
// Convert a data array into a wxImage
#ifdef __cpp_inline_variables
inline wxImage wxueImage(const unsigned char* data, size_t size_data)
#else
static wxImage wxueImage(const unsigned char* data, size_t size_data)
#endif
{
    wxMemoryInputStream strm(data, size_data);
    wxImage image;
    image.LoadFile(strm);
    return image;
};
)===";

inline constexpr const auto txt_GetBundleFromSVG = R"===(
// Convert compressed SVG string into a wxBitmapBundle
#ifdef __cpp_inline_variables
inline wxBitmapBundle wxueBundleSVG(const unsigned char* data,
    size_t size_data, size_t size_svg, wxSize def_size)
#else
static wxBitmapBundle wxueBundleSVG(const unsigned char* data,
    size_t size_data, size_t size_svg, wxSize def_size)
#endif
{
    auto str = std::make_unique<char[]>(size_svg);
    wxMemoryInputStream stream_in(data, size_data);
    wxZlibInputStream zlib_strm(stream_in);
    zlib_strm.Read(str.get(), size_svg);
    return wxBitmapBundle::FromSVG(str.get(), def_size);
};
)===";

inline constexpr const auto txt_GetAnimFromHdrFunction = R"===(
// Convert a data array into a wxAnimation
#ifdef __cpp_inline_variables
inline void wxueAnimation(const unsigned char* data, size_t size_data, wxAnimation& animation)
#else
static void wxueAnimation(const unsigned char* data, size_t size_data, wxAnimation& animation)
#endif
{
    wxMemoryInputStream strm(data, size_data);
    animation.Load(strm);
};
)===";

inline constexpr const auto txt_BaseCmtBlock =
R"===(///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

)===";

// clang-format on

void MainFrame::OnGenSingleCpp(wxCommandEvent& /* event unused */)
{
    auto* form = wxGetMainFrame()->getSelectedNode();
    if (form && !form->is_Form())
    {
        form = form->get_Form();
    }
    if (!form)
    {
        wxMessageBox("You must select a form before you can generate code.", "Code Generation");
        return;
    }

    GenResults results;
    GenData gen_data(results, nullptr);

    if (const auto& extProp = Project.as_string(prop_source_ext); extProp.size())
    {
        gen_data.source_ext = extProp;
    }
    else
    {
        gen_data.source_ext = ".cpp";
    }

    if (const auto& extProp = Project.as_string(prop_header_ext); extProp.size())
    {
        gen_data.header_ext = extProp;
    }
    else
    {
        gen_data.header_ext = ".h";
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);
    Project.FindWxueFunctions(forms);

    GenCppForm(gen_data, form);

    tt_string msg;
    if (results.updated_files.size())
    {
        if (results.updated_files.size() == 1)
        {
            msg << "1 file was updated";
        }
        else
        {
            msg << results.updated_files.size() << " files were updated";
        }
        msg << '\n';
    }
    else
    {
        msg << "All " << results.file_count << " generated files are current";
    }

    if (results.msgs.size())
    {
        for (auto& iter: results.msgs)
        {
            msg << '\n';
            msg << iter;
        }
    }

    wxMessageBox(msg, "C++ Code Generation", wxOK | wxICON_INFORMATION);
}

namespace
{

    void GenCppForm(GenData& gen_data, Node* form)
    {
        // These are just defined for convenience.
        tt_string& source_ext = gen_data.source_ext;
        tt_string& header_ext = gen_data.header_ext;

        auto [path, has_base_file] = Project.GetOutputPath(form, GEN_LANG_CPLUSPLUS);
        if (!has_base_file)
        {
            tt_string msg("No filename specified for ");
            if (form->HasValue(prop_class_name))
            {
                msg += form->as_string(prop_class_name);
            }
            else
            {
                msg += map_GenNames.at(form->get_GenName());
            }
            msg += '\n';
            gen_data.AddResultMsg(msg);
            return;
        }

        CppCodeGenerator codegen(form);

        path.replace_extension(header_ext);
        auto h_cw = std::make_unique<FileCodeWriter>(path);
        codegen.SetHdrWriteCode(h_cw.get());

        path.replace_extension(source_ext);
        auto cpp_cw = std::make_unique<FileCodeWriter>(path);
        codegen.SetSrcWriteCode(cpp_cw.get());

        codegen.GenerateClass();

        path.replace_extension(header_ext);

        int flags = flag_no_ui;
        if (gen_data.pClassList)
        {
            flags |= flag_test_only;
        }
        if (form->as_bool(prop_no_closing_brace))
        {
            flags |= flag_add_closing_brace;
        }
        auto retval = h_cw->WriteFile(GEN_LANG_CPLUSPLUS, flags, form);
        if (form->as_bool(prop_no_closing_brace))
        {
            flags = flags & ~flag_add_closing_brace;
        }

        if (retval > 0)
        {
            if (!gen_data.pClassList)
            {
                gen_data.AddUpdateFilename(path);
            }
            else
            {
                if (form->is_Gen(gen_Images))
                {
                    // While technically this is a "form" it doesn't have the usual properties set
                    gen_data.AddClassName(GenEnum::map_GenNames.at(gen_Images));
                }
                else if (form->is_Gen(gen_Data))
                {
                    gen_data.AddClassName(GenEnum::map_GenNames.at(gen_Data));
                }
                else
                {
                    gen_data.AddClassName(form->as_string(prop_class_name));
                }
                return;
            }
        }
        else if (retval < 0)
        {
            gen_data.AddResultMsg(tt_string()
                                  << "Cannot create or write to the file " << path << '\n');
        }
        else  // retval == result::exists)
        {
            gen_data.UpdateFileCount();
        }

        path.replace_extension(source_ext);
        retval = cpp_cw->WriteFile(GEN_LANG_CPLUSPLUS, flags, form);

        if (retval > 0)
        {
            if (!gen_data.pClassList)
            {
                gen_data.AddUpdateFilename(path);
            }
            else
            {
                if (form->is_Gen(gen_Images))
                {
                    // While technically this is a "form" it doesn't have the usual properties set
                    gen_data.AddClassName("Images List");
                }
                else if (form->is_Gen(gen_Data))
                {
                    gen_data.AddClassName("Data List");
                }
                else
                {
                    gen_data.AddClassName(form->as_string(prop_class_name));
                }
                return;
            }
        }

        else if (retval < 0)
        {
            gen_data.AddResultMsg(tt_string()
                                  << "Cannot create or write to the file " << path << '\n');
        }
        else  // retval == result::exists
        {
            gen_data.UpdateFileCount();
        }
    }
}  // namespace

bool GenerateCppFiles(GenResults& results, std::vector<tt_string>* pClassList)
{
    if (Project.as_bool(prop_generate_cmake))
    {
        auto is_testing = pClassList ? 1 : 0;
        for (auto& iter: Project.get_ChildNodePtrs())
        {
            if (iter->is_Gen(gen_folder) && iter->HasValue(prop_folder_cmake_file))
            {
                auto result = WriteCMakeFile(iter.get(), results, is_testing);
                if (result == result::created || result == result::needs_writing)
                {
                    ++results.file_count;
                    if (is_testing)
                    {
                        pClassList->emplace_back(iter.get()->as_string(prop_cmake_file));
                    }
                }
            }
        }
        if (Project.HasValue(prop_cmake_file))
        {
            auto result = WriteCMakeFile(Project.get_ProjectNode(), results, is_testing);
            if (result == result::created || result == result::needs_writing)
            {
                ++results.file_count;
                if (is_testing)
                {
                    pClassList->emplace_back(Project.get_ProjectNode()->as_string(prop_cmake_file));
                }
            }
        }
    }

    tt_string path;

    tt_string source_ext(".cpp");
    tt_string header_ext(".h");

    if (const auto& extProp = Project.as_string(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }

    if (const auto& extProp = Project.as_string(prop_header_ext); extProp.size())
    {
        header_ext = extProp;
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);
    Project.FindWxueFunctions(forms);

    GenData gen_data(results, pClassList);
    gen_data.source_ext = source_ext;
    gen_data.header_ext = header_ext;

    for (const auto& form: forms)
    {
        GenCppForm(gen_data, form);
    }

    if (pClassList)
    {
        return pClassList->size() > 0;
    }
    return results.updated_files.size() > 0;
}

void CppCodeGenerator::GenCppImageFunctions()
{
    // First, generate the header files needed

    m_source->writeLine();
    if (m_NeedAnimationFunction)
    {
        m_source->writeLine("#include <wx/animate.h>", indent::none);
        m_source->writeLine("\n#include <wx/mstream.h>  // memory stream classes", indent::none);
        if (!m_NeedSVGFunction)
        {
            m_source->writeLine("#include <wx/zstream.h>  // zlib stream classes", indent::none);

            m_source->writeLine();
            m_source->writeLine("#include <memory>  // for std::make_unique", indent::none);
        }
    }
    else if (m_NeedImageFunction || m_NeedHeaderFunction || m_NeedSVGFunction)
    {
        m_source->writeLine();
        if (m_NeedSVGFunction)
        {
            m_source->writeLine("#include <wx/bmpbndl.h>  // wxBitmapBundle class", indent::none);
        }
        m_source->writeLine("#include <wx/mstream.h>  // memory stream classes", indent::none);
    }

    if (m_NeedSVGFunction)
    {
        m_source->writeLine("#include <wx/zstream.h>  // zlib stream classes", indent::none);

        m_source->writeLine();
        m_source->writeLine("#include <memory>  // for std::make_unique", indent::none);
    }
    m_source->writeLine();

    // m_NeedImageFunction and m_NeedSVGFunction will be set to true if there is an image
    // that is not added to an Image List where it can be loaded via a wxue_img:: function.

    if (m_NeedImageFunction || m_NeedHeaderFunction)
    {
        tt_string_vector function;
        function.ReadString(txt_wxueImageFunction);
        for (auto& iter: function)
        {
            m_source->writeLine(iter, indent::none);
        }
        m_source->writeLine();
    }

    if (m_NeedSVGFunction)
    {
        tt_string_vector function;
        function.ReadString(txt_GetBundleFromSVG);
        for (auto& iter: function)
        {
            m_source->writeLine(iter, indent::none);
        }
        m_source->writeLine();
    }

    if (m_NeedAnimationFunction)
    {
        // Note that we write the function even if the Image List file also has the
        // function. It won't matter for C++17, and for C++14 the animation isn't likely to
        // appear in a lot of forms, so any duplication of the function won't matter very
        // much.
        tt_string_vector function;
        function.ReadString(txt_GetAnimFromHdrFunction);
        for (auto& iter: function)
        {
            m_source->writeLine(iter, indent::none);
        }
        m_source->writeLine();
    }

    if (m_embedded_images.size())
    {
        Code code(m_form_node, GEN_LANG_CPLUSPLUS);
        WriteImagePreConstruction(code);
        if (code.size())
        {
            m_source->writeLine(code);
        }
    }

    GenerateCppClassConstructor();

    if (m_embedded_images.size())
    {
        Code code(m_form_node, GEN_LANG_CPLUSPLUS);
        WriteImageConstruction(code);
    }
}

CppCodeGenerator::CppCodeGenerator(Node* form_node) :
    BaseCodeGenerator(GEN_LANG_CPLUSPLUS, form_node)
{
}

void CppCodeGenerator::GenerateClass(PANEL_PAGE panel_type)
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS)
    if (m_form_node->is_Gen(gen_Data))
    {
        GenerateDataClassConstructor(panel_type);
        return;
    }

    Code code(m_form_node, GEN_LANG_CPLUSPLUS);

    m_ctx_menu_events.clear();
    m_embedded_images.clear();
    m_type_generated.clear();

    // If there is an Images form, then calculate the #include file relative to the current
    // form's output file.
    m_include_images_statement.clear();
    SetImagesForm();
    if (m_ImagesForm && m_ImagesForm->HasValue(prop_base_file))
    {
        auto [path, has_base_file] = Project.GetOutputPath(m_ImagesForm, GEN_LANG_CPLUSPLUS);
        if (has_base_file)
        {
            path.make_relative(Project.get_BaseDirectory(m_form_node).make_absolute());
            path.backslashestoforward();
            path.replace_extension(m_header_ext);
            m_include_images_statement << "#include \"" << path << '\"';
        }
    }

    // Initialize these values before calling ParseImageProperties
    m_NeedAnimationFunction = false;
    m_NeedArtProviderHeader = false;
    m_NeedHeaderFunction = false;
    m_NeedImageFunction = false;
    m_NeedSVGFunction = false;

    std::set<std::string> img_include_set;

    std::thread thrd_get_events(&CppCodeGenerator::CollectEventHandlers, this, m_form_node,
                                std::ref(m_events));
    std::thread thrd_collect_img_headers(&CppCodeGenerator::CollectImageHeaders, this, m_form_node,
                                         std::ref(img_include_set));
    std::thread thrd_need_img_func(&CppCodeGenerator::ParseImageProperties, this, m_form_node);

    // If the code files are being written to disk, then UpdateEmbedNodes() has already been
    // called.
    if (panel_type != NOT_PANEL)
    {
        ProjectImages.UpdateEmbedNodes();
    }

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();
    m_source->SetLastLineBlank();
    m_header->SetLastLineBlank();

#if !defined(_DEBUG)
    if (m_panel_type == NOT_PANEL)
#endif  // _DEBUG
    {
        m_header->writeLine(txt_BaseCmtBlock);
        m_source->writeLine(txt_BaseCmtBlock);
    }

    // Generate #include statements in both source and header files
    GenerateClassIncludes(code, panel_type, &thrd_get_events);

    try
    {
        thrd_collect_img_headers.join();
    }
    catch (const std::system_error& err)
    {
#if defined(_DEBUG)
        MSG_ERROR(err.what());
#else
        wxMessageDialog dlg_error(nullptr, wxString::FromUTF8(err.what()), "Internal Thread Error",
                                  wxICON_ERROR | wxOK);
        dlg_error.ShowModal();
#endif  // _DEBUG
    }

    if (m_embedded_images.size())
    {
        std::sort(m_embedded_images.begin(), m_embedded_images.end(),
                  [](const EmbeddedImage* image_a, const EmbeddedImage* image_b)
                  {
                      return (image_a->base_image().array_name.compare(
                                  image_b->base_image().array_name) < 0);
                  });
    }

    if (m_panel_type != HDR_PANEL)
    {
        if (!img_include_set.empty())
        {
            for (const auto& iter: img_include_set)
            {
                m_source->writeLine(iter.c_str());
            }

            m_source->writeLine();
        }
    }

    // Make a copy of the string so that we can tweak it
    tt_string namespace_prop = m_form_node->HasValue(prop_name_space) ?
                                   m_form_node->as_string(prop_name_space) :
                                   Project.as_string(prop_name_space);
    if (auto* node_namespace = m_form_node->get_Folder();
        node_namespace && node_namespace->HasValue(prop_folder_namespace))
    {
        namespace_prop = node_namespace->as_string(prop_folder_namespace);
    }

    if (m_form_node->is_Gen(gen_Images))
    {
        // thrd_collect_img_headers.join() has already been called
        thrd_need_img_func.join();
        GenerateImagesForm();
        return;
    }
    if (m_form_node->is_Gen(gen_Data))
    {
        thrd_need_img_func.join();
        GenerateDataForm();
        return;
    }

    // There can be nested namespaces, so GenHdrNameSpace() will parse those into a vector that we
    // provide. The indent will be updated to tell us how much the generated code should be indented
    // to account for the namespace(s).
    size_t indent = 0;
    tt_string_vector names;
    if (namespace_prop.size())
    {
        if (m_embedded_images.size())
        {
            WriteImagePostHeader();
            m_header->writeLine();
        }

        GenHdrNameSpace(namespace_prop, names, indent);
    }

    if (m_panel_type != CPP_PANEL)
    {
        GenerateCppClassHeader(namespace_prop.size());
    }

    thrd_need_img_func.join();
    if (m_panel_type != HDR_PANEL)
    {
        GenCppImageFunctions();
    }

    // If there was a namespace, then GenHdrNameSpace() will have increased the indent level.
    if (indent > 0)
    {
        while (indent > 0)
        {
            m_header->Unindent();
            m_header->writeLine(tt_string() << "} // namespace " << names[--indent]);
        }
        m_header->writeLine();
    }

    if (m_form_node->HasValue(prop_cpp_conditional))
    {
        code.Eol().Str("#endif  // ").Str(m_form_node->as_string(prop_cpp_conditional));
        m_source->writeLine(code);
    }
}

// Called from GenerateClass() to generate #include statements in both source and header
// files
void CppCodeGenerator::GenerateClassIncludes(Code& code, PANEL_PAGE panel_type,
                                             std::thread* thrd_get_events)
{
    tt_string file;
    if (const auto& base_file = m_form_node->as_string(prop_base_file); base_file.size())
    {
        tt_cwd cwd(true);
        Project.ChangeDir();
        file = base_file;
        file.make_relative(Project.get_ProjectPath());
        file.backslashestoforward();
        file.remove_extension();

        m_baseFullPath = base_file;
        m_baseFullPath.make_absolute();
        m_baseFullPath.remove_filename();
    }

    m_header->writeLine("#pragma once");
    m_header->writeLine();

    std::set<std::string> src_includes;
    std::set<std::string> hdr_includes;
    if (Project.as_string(prop_help_provider) != "none")
    {
        src_includes.insert("#include <wx/cshelp.h>");
    }
    if (Project.as_bool(prop_internationalize))
    {
        hdr_includes.insert("#include <wx/intl.h>");
    }

    // This will almost always be needed, and it in turn includes a bunch of other files like
    // string.h which are also almost always needed.
    hdr_includes.insert("#include <wx/gdicmn.h>");

    CollectIncludes(m_form_node, src_includes, hdr_includes);

    if (m_form_node->as_bool(prop_persist))
    {
        src_includes.insert("#include <wx/persist.h>");
        src_includes.insert("#include <wx/persist/toplevel.h>");
    }

    if (m_form_node->HasValue(prop_icon))
    {
        src_includes.insert("#include <wx/icon.h>");
    }

    if (m_NeedArtProviderHeader)
    {
        src_includes.insert("#include <wx/artprov.h>");
    }

    // Delay calling join() for as long as possible to increase the chance that the thread will
    // have already completed.
    thrd_get_events->join();
    if (m_events.size() || m_map_conditional_events.size() || m_ctx_menu_events.size())
    {
        hdr_includes.insert("#include <wx/event.h>");
    }

    if (panel_type != CPP_PANEL)
    {
        // Write the #include files to m_header
        GenInitHeaderFile(hdr_includes);
    }

    if (m_form_node->HasValue(prop_cpp_conditional))
    {
        if (!m_form_node->as_string(prop_cpp_conditional).starts_with("#"))
        {
            code.Str("#if ");
        }
        code.Str(m_form_node->as_string(prop_cpp_conditional));
        m_source->writeLine(code);
        m_source->writeLine();
        code.clear();
    }

    if (Project.HasValue(prop_local_pch_file))
    {
        m_source->writeLine(tt_string()
                            << "#include \"" << Project.as_string(prop_local_pch_file) << '"');
        m_source->writeLine();
    }

    // Make certain there is a blank line before the the wxWidget #includes
    m_source->writeLine();

    // All generators that use a wxBitmapBundle should add "#include <wx/bmpbndl.h>" to the
    // header set.

    if (const auto& hdr_extension = Project.as_string(prop_header_ext); hdr_extension.size())
    {
        m_header_ext = hdr_extension;
    }
    if (Project.HasValue(prop_src_preamble))
    {
        WritePropSourceCode(Project.get_ProjectNode(), prop_src_preamble);
    }

    std::vector<std::string> ordered_includes;
    if (auto pos = src_includes.find("#include <wx/generic/stattextg.h>");
        pos != src_includes.end())
    {
        src_includes.erase(pos);
        if (pos = src_includes.find("#include <wx/stattext.h>"); pos != src_includes.end())
        {
            src_includes.erase(pos);
        }

        if (ordered_includes.empty())
        {
            ordered_includes.emplace_back("// Order dependent includes");
        }

        ordered_includes.emplace_back("#include <wx/stattext.h>");
        ordered_includes.emplace_back("#include <wx/generic/stattextg.h>");
    }
    if (auto pos = src_includes.find("#include <wx/generic/treectlg.h>"); pos != src_includes.end())
    {
        src_includes.erase(pos);
        if (pos = src_includes.find("#include <wx/treectrl.h>"); pos != src_includes.end())
        {
            src_includes.erase(pos);
        }

        if (ordered_includes.empty())
        {
            ordered_includes.emplace_back("// Order dependent includes");
        }

        ordered_includes.emplace_back("#include <wx/treectrl.h>");
        ordered_includes.emplace_back("#include <wx/generic/treectlg.h>");
    }

    if (ordered_includes.size())
    {
        for (auto& iter: ordered_includes)
        {
            m_source->writeLine(iter);
        }
        m_source->writeLine();
    }

    for (const auto& iter: src_includes)
    {
        if (tt::contains(iter, "<wx"))
        {
            m_source->writeLine(iter);
        }
    }

    m_source->writeLine();

    if (Project.get_ProjectNode()->HasValue(prop_project_src_includes))
    {
        m_source->writeLine();
        tt_view_vector list;
        list.SetString(Project.get_ProjectNode()->as_string(prop_project_src_includes));
        for (auto& iter: list)
        {
            tt_string include = iter;
            include.make_absolute();
            include.make_relative(Project.get_BaseDirectory(m_form_node));
            include.backslashestoforward();
            m_source->writeLine(tt_string("#include \"") << include << '"');
        }

        m_source->writeLine();
    }

    // Now output all the other header files (this will include derived_class header files)
    for (const auto& iter: src_includes)
    {
        if (!tt::contains(iter, "<wx"))
        {
            m_source->writeLine(iter);
        }
    }

    m_source->writeLine();

    if (m_form_node->HasValue(prop_source_preamble))
    {
        WritePropSourceCode(m_form_node, prop_source_preamble);
    }

    if (m_form_node->HasValue(prop_system_src_includes))
    {
        m_source->writeLine();
        tt_view_vector list;
        list.SetString(m_form_node->as_string(prop_system_src_includes));
        for (auto& iter: list)
        {
            m_source->writeLine(tt_string("#include <") << iter << '>');
        }
    }

    if (file.empty())
    {
        m_source->writeLine();
        m_source->writeLine("// Specify the filename to use in the base_file property");
        m_source->writeLine("#include \"Your filename here\"");
    }
    else
    {
        file.replace_extension(m_header_ext);
        m_source->writeLine();
        m_source->writeLine(tt_string() << "#include \"" << file.filename() << "\"");
    }

    if (m_form_node->HasValue(prop_local_src_includes))
    {
        m_source->writeLine();
        tt_view_vector list;
        list.SetString(m_form_node->as_string(prop_local_src_includes));
        for (auto& iter: list)
        {
            m_source->writeLine(tt_string("#include \"") << iter << '"');
        }
    }

    m_source->writeLine();
}

void CppCodeGenerator::GenerateCppClassConstructor()
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);
    m_source->writeLine();

    auto* generator = m_form_node->get_Generator();
    Code code(m_form_node, GEN_LANG_CPLUSPLUS);
    if (generator->ConstructionCode(code))
    {
        m_source->writeLine(code);
        m_source->Indent();

        if (m_form_node->is_Type(type_frame_form) || m_form_node->is_Gen(gen_wxDialog) ||
            m_form_node->is_Gen(gen_wxPropertySheetDialog) || m_form_node->is_Gen(gen_wxWizard))
        {
            // Write code to m_source that will load any image handlers needed by the form's
            // class
            GenerateCppHandlers();
            if (m_form_node->HasValue(prop_icon))
            {
                auto icon_code = GenerateIconCode(m_form_node->as_string(prop_icon));
                m_source->writeLine(icon_code, indent::auto_keep_whitespace);
                m_source->writeLine();
            }
        }

        code.clear();
        if (generator->SettingsCode(code))
        {
            m_source->writeLine(code);
            m_source->writeLine();
        }
    }
    else
    {
        m_source->Indent();
    }

    if (!m_form_node->is_Gen(gen_wxWizard) && !m_form_node->is_Type(type_frame_form))
    {
        // Write code to m_source that will load any image handlers needed by the form's class
        GenerateCppHandlers();
    }

    if (m_form_node->get_PropPtr(prop_window_extra_style))
    {
        code.clear();
        code.GenWindowSettings();
        if (code.size())
        {
            // GenerateWindowSettings() can result in code within braces, so keep any leading
            // whitespace.
            m_source->writeLine(code.GetView(), indent::auto_keep_whitespace);
        }
    }

    m_source->SetLastLineBlank();
    if (!m_form_node->is_Gen(gen_DocViewApp))
    {
        for (const auto& child: m_form_node->get_ChildNodePtrs())
        {
            if (child->is_Gen(gen_wxContextMenuEvent))
            {
                continue;
            }
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

        if (m_form_node->as_bool(prop_persist))
        {
            m_source->writeLine();
            tt_string tmp("wxPersistentRegisterAndRestore(this, \"");
            tmp << m_form_node->get_NodeName() << "\");";
            m_source->writeLine(tmp);
        }

        AddPersistCode(m_form_node);

        if (m_events.size() || m_map_conditional_events.size())
        {
            m_source->writeLine();
            m_source->writeLine("// Event handlers");
            GenSrcEventBinding(m_form_node, m_events);

            // Only generate potential events if no derived class is being
            // created. If a derived class is being created, then we don't know
            // the name of that class's file, and therefore have no idea if the
            // event has been implemented or not.
            if (m_events.size() && !m_form_node->as_bool(prop_derived_class))
            {
                m_source->writeLine();
                m_source->ResetIndent();
                GenUnhandledEvents(m_events);
                m_source->Indent();
            }
        }

        code.clear();
        if (TimerGenerator::StartIfChildTimer(m_form_node, code))
        {
            m_source->writeLine(code);
            m_source->writeLine();
        }
    }
    if (m_form_node->is_Gen(gen_wxDialog) || m_form_node->is_Type(type_frame_form) ||
        m_form_node->is_Gen(gen_PanelForm) || m_form_node->is_Gen(gen_wxPropertySheetDialog))
    {
        m_source->writeLine("\nreturn true;");
    }

    m_source->Unindent();
    if (m_form_node->is_Gen(gen_DocViewApp))
    {
        m_source->SetLastLineBlank();
    }
    m_source->writeLine("}");

    if (m_form_node->is_Gen(gen_DocViewApp))
    {
        code.clear();
        if (generator->AfterConstructionCode(code))
        {
            m_source->writeLine();
            m_source->writeLine(code);
        }

        code.clear();
    }
    else
    {
        code.clear();
    }

    Node* node_ctx_menu = nullptr;
    for (const auto& child: m_form_node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_wxContextMenuEvent))
        {
            node_ctx_menu = child.get();
            break;
        }
    }

    if (node_ctx_menu)
    {
        GenContextMenuHandler(node_ctx_menu);
    }
}

void CppCodeGenerator::GenerateCppHandlers()
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (m_embedded_images.size())
    {
        for (auto& iter_img: m_embedded_images)
        {
            if (iter_img->base_image().type != wxBITMAP_TYPE_BMP &&
                iter_img->base_image().type != wxBITMAP_TYPE_SVG &&
                !m_type_generated.contains(iter_img->base_image().type))
            {
                m_source->writeLine(tt_string("if (!wxImage::FindHandler(")
                                    << g_map_types.at(iter_img->base_image().type) << "))");
                m_source->Indent();
                m_source->writeLine(tt_string("\twxImage::AddHandler(new ")
                                    << g_map_handlers.at(iter_img->base_image().type) << ");");
                m_source->Unindent();
                m_type_generated.insert(iter_img->base_image().type);
            }
        }
        m_source->writeLine();
    }
}

void CppCodeGenerator::GenUnhandledEvents(EventVector& events)
{
    ASSERT_MSG(events.size(), "GenUnhandledEvents() shouldn't be called if there are no events");
    if (events.empty() || m_form_node->as_bool(prop_use_derived_class))
    {
        return;
    }

    // Multiple events can be bound to the same function, so use a set to make sure we only
    // generate each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_CPLUSPLUS);
    auto sort_event_handlers = [](NodeEvent* event_a, NodeEvent* event_b)
    {
        return (EventHandlerDlg::GetCppValue(event_a->get_value()) <
                EventHandlerDlg::GetCppValue(event_b->get_value()));
    };

    // Sort events by function name
    std::sort(events.begin(), events.end(), sort_event_handlers);

    bool found_user_handlers = false;

    // In Debug mode, always compare to see if the event handler has been implemented
#if !defined(_DEBUG)
    if (m_panel_type == NOT_PANEL)
#endif  // _DEBUG
    {
        tt_view_vector org_file;
        auto [path, has_base_file] = Project.GetOutputPath(m_form_node, GEN_LANG_CPLUSPLUS);

        if (has_base_file && path.extension().empty())
        {
            if (const auto& extProp = Project.as_string(prop_source_ext); extProp.size())
            {
                path += extProp;
            }
            else
            {
                path += ".cpp";
            }
        }

        // If the user has defined any event handlers, add them to the code_lines set so we
        // don't generate them again.
        if (has_base_file && org_file.ReadFile(path))
        {
            size_t line_index = 0;
            for (; line_index < org_file.size(); ++line_index)
            {
                if (org_file[line_index].is_sameprefix(cpp_rust_end_cmt_line))
                {
                    break;
                }
            }
            for (++line_index; line_index < org_file.size(); ++line_index)
            {
                auto handler = org_file[line_index].view_nonspace();
                if (org_file[line_index].view_nonspace().starts_with("void "))
                {
                    code_lines.emplace(handler);
                    found_user_handlers = true;
                }
            }
        }
    }

    bool is_all_events_implemented = true;
    if (found_user_handlers)
    {
        // Determine whether the user has implemented all of the event handlers in this module
        for (auto& event: events)
        {
            auto handler = EventHandlerDlg::GetCppValue(event->get_value());
            // Ignore lambda's
            if (handler.starts_with("["))
            {
                continue;
            }

            tt_string set_code;
            set_code << "void " << m_form_node->get_NodeName() << "::" << handler;
            for (const auto& iter: code_lines)
            {
                if (iter.starts_with(set_code))
                {
                    // This event handler has already been created by the user
                    set_code.clear();
                    break;
                }
            }
            if (set_code.empty())
            {
                continue;
            }

            // At least one event wasn't implemented, so stop looking for more
            is_all_events_implemented = false;

            code.Str("// Unimplemented Event handler functions\n// Copy any of the following and "
                     "paste them below the "
                     "comment block, or "
                     "to your inherited class.");
            code.Eol().Str("\n/*").Eol();
            break;
        }
        if (is_all_events_implemented)
        {
            // If the user has defined all the event handlers, then we don't need to output
            // anything else.
            return;
        }
    }
    else
    {
        // The user hasn't defined their own event handlers in this module
        is_all_events_implemented = false;

        code.Str("// Unimplemented Event handler functions\n// Copy any of the following and paste "
                 "them below the comment "
                 "block, or "
                 "to your inherited class.");
        code.Eol().Str("\n/*").Eol();
    }
    m_source->writeLine(code);

    code.clear();
    if (!is_all_events_implemented)
    {
        for (auto& event: events)
        {
            auto handler = EventHandlerDlg::GetCppValue(event->get_value());
            // Ignore lambda's
            if (handler.empty() || handler.starts_with("["))
            {
                continue;
            }

            // The user's declaration will typically include the event parameter, often
            tt_string set_code;
            set_code << "void " << m_form_node->get_NodeName() << "::" << handler << '(';
            for (const auto& iter: code_lines)
            {
                if (iter.starts_with(set_code))
                {
                    // This event handler has already been created by the user
                    set_code.clear();
                    break;
                }
            }
            if (set_code.empty())
            {
                continue;
            }

            // Add it to our set of handled events in case the user specified
            // the same event handler for multiple events.
            code_lines.emplace(set_code);

            tt_string event_function = set_code;
            code.Str(event_function) << event->get_EventInfo()->get_event_class() << "& event)";
            code.Eol().OpenBrace();
#if defined(_DEBUG)
            const auto& dbg_event_name = event->get_name();
            wxUnusedVar(dbg_event_name);
#endif  // _DEBUG
            if (event->get_name() == "CloseButtonClicked")
            {
                code.Str("EndModal(wxID_CLOSE);").Eol().Eol();
            }
            else if (event->get_name() == "YesButtonClicked")
            {
                code.Str("EndModal(wxID_YES);").Eol().Eol();
            }
            else if (event->get_name() == "NoButtonClicked")
            {
                code.Str("EndModal(wxID_NO);").Eol().Eol();
            }
            else
            {
                code.Str("event.Skip();").Eol().Eol();
            }
            code.CloseBrace().Eol();
        }
    }

    if (!is_all_events_implemented)
    {
        m_source->writeLine(code);
        m_source->writeLine("\n*/");
    }
}

// This should only be called to generate C++ code.
void CppCodeGenerator::GenCppEnumIds(Node* class_node)
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (!class_node->as_bool(prop_generate_ids))
    {
        return;
    }

    std::set<std::string> set_enum_ids;
    std::set<std::string> set_const_ids;
    CollectIDs(class_node, set_enum_ids, set_const_ids);

    if (set_const_ids.size())
    {
        for (const auto& iter: set_const_ids)
        {
            const std::string new_id = "static const int ";
            if (iter.starts_with("self."))
            {
                m_header->write(new_id + iter.substr(sizeof("self.") - 1));
            }
            else
            {
                m_header->write(new_id + iter);
            }
            m_header->writeLine(";");
        }
        m_header->writeLine();
    }

    if (set_enum_ids.size())
    {
        m_header->writeLine("enum");
        m_header->writeLine("{");
        m_header->Indent();

        size_t item = 0;
        for (const auto& iter: set_enum_ids)
        {
            if (iter.starts_with("self."))
            {
                m_header->write(iter.substr(sizeof("self.") - 1));
            }
            else
            {
                m_header->write(iter);
            }
            if (item == 0)
            {
                if (class_node->HasValue(prop_initial_enum_string))
                {
                    m_header->write(" = " + class_node->as_string(prop_initial_enum_string));
                }
                else
                {
                    m_header->write(" = wxID_HIGHEST + 1", true);
                }
            }

            if (item < set_enum_ids.size() - 1)
            {
                m_header->writeLine(",");
            }
            ++item;
        }

        m_header->Unindent();
        m_header->writeLine();
        m_header->writeLine("};");
        m_header->writeLine();
    }
}

void CppCodeGenerator::GenerateDataClassConstructor(PANEL_PAGE panel_type)
{
    Code code(m_form_node, GEN_LANG_CPLUSPLUS);

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();
    m_source->SetLastLineBlank();
    m_header->SetLastLineBlank();

#if !defined(_DEBUG)
    if (m_panel_type == NOT_PANEL)
#endif  // _DEBUG
    {
        m_header->writeLine(txt_BaseCmtBlock);
        m_source->writeLine(txt_BaseCmtBlock);
    }

    auto [path, has_base_file] = Project.GetOutputPath(m_form_node, GEN_LANG_CPLUSPLUS);
    // tt_string file;
    m_baseFullPath = path;
    if (has_base_file)
    {
        m_baseFullPath.remove_filename();
    }

    m_header->writeLine("#pragma once");
    m_header->writeLine();

    if (Project.HasValue(prop_local_pch_file))
    {
        m_source->writeLine(tt_string()
                            << "#include \"" << Project.as_string(prop_local_pch_file) << '"');
        m_source->writeLine();
    }

    // Make certain there is a blank line before the the wxWidget #includes
    m_source->writeLine();

    if (Project.HasValue(prop_src_preamble))
    {
        WritePropSourceCode(Project.get_ProjectNode(), prop_src_preamble);
    }

    m_source->writeLine();

    if (Project.get_ProjectNode()->HasValue(prop_project_src_includes))
    {
        m_source->writeLine();
        tt_view_vector list;
        list.SetString(Project.get_ProjectNode()->as_string(prop_project_src_includes));
        for (auto& iter: list)
        {
            tt_string include = iter;
            include.make_absolute();
            include.make_relative(Project.get_BaseDirectory(m_form_node));
            include.backslashestoforward();
            m_source->writeLine(tt_string("#include \"") << include << '"');
        }

        m_source->writeLine();
    }

    m_source->writeLine();

    if (m_form_node->HasValue(prop_source_preamble))
    {
        WritePropSourceCode(m_form_node, prop_source_preamble);
    }

    if (!has_base_file)
    {
        m_source->writeLine();
        m_source->writeLine("// Specify the filename to use in the base_file property");
        m_source->writeLine("#include \"Your filename here\"");
    }
    else
    {
        path.replace_extension(m_header_ext);
        m_source->writeLine();
        m_source->writeLine(tt_string() << "#include \"" << path.filename() << "\"");
    }

    if (m_form_node->HasValue(prop_local_src_includes))
    {
        m_source->writeLine();
        tt_view_vector list;
        list.SetString(m_form_node->as_string(prop_local_src_includes));
        for (auto& iter: list)
        {
            m_source->writeLine(tt_string("#include \"") << iter << '"');
        }
    }

    m_source->writeLine();
    GenerateDataForm();
}

// clang-format off

inline constexpr const auto txt_get_data_function = R"===(
    // Convert compressed data string into a char array
    std::unique_ptr<unsigned char[]> get_data(const unsigned char* data,
        size_t size_data, size_t size_data_uncompressed)
    {
        auto str = std::unique_ptr<unsigned char[]>(new unsigned char[size_data_uncompressed]);
        wxMemoryInputStream stream_in(data, size_data);
        wxZlibInputStream zlib_strm(stream_in);
        zlib_strm.Read(str.get(), size_data_uncompressed);
        return str;
    };
)===";

// clang-format on

void CppCodeGenerator::GenerateDataForm()
{
    ASSERT_MSG(m_form_node, "Attempting to generate Data List when no form was located.");

    if (!m_form_node->get_ChildCount())
    {
        return;
    }
    if (m_panel_type != HDR_PANEL)
    {
        m_source->writeLine("#include <wx/mstream.h>  // memory stream classes", indent::none);
        m_source->writeLine("#include <wx/zstream.h>  // zlib stream classes", indent::none);

        m_source->writeLine();
        m_source->writeLine("namespace wxue_data\n{");
        m_source->Indent();
        m_source->SetLastLineBlank();

        tt_string_vector function;
        function.ReadString(txt_get_data_function);
        for (auto& iter: function)
        {
            m_source->writeLine(iter, indent::none);
        }

        Code code(m_form_node, m_language);

        ProjectData.WriteDataConstruction(code, m_source);

        m_source->Unindent();
        m_source->writeLine("}\n");
    }

    /////////////// Header code ///////////////

    if (m_panel_type != CPP_PANEL)
    {
        m_header->writeLine();
        m_header->writeLine("#include <memory>  // for std::make_unique", indent::none);

        if (ProjectData.NeedsUtilityHeader())
        {
            m_header->writeLine("#include <utility>  // for std::pair", indent::none);
        }

        m_header->writeLine();
        m_header->writeLine("namespace wxue_data\n{");
        m_header->Indent();
        m_header->SetLastLineBlank();
        m_header->writeLine("std::unique_ptr<unsigned char[]> get_data(const unsigned char* data, "
                            "size_t size_data, size_t "
                            "size_data_uncompressed);");

        m_header->writeLine();

        m_header->Unindent();
        m_header->writeLine("}\n");
    }
    ProjectData.WriteImagePostHeader(m_header);
}

void CppCodeGenerator::CollectIncludes(Node* form, std::set<std::string>& set_src,
                                       std::set<std::string>& set_hdr)
{
    ASSERT_MSG(form->is_Form(), "Only forms should be passed to CollectIncludes()");
    if (form->is_Gen(gen_Images) || form->is_Gen(gen_Data))
    {
        return;
    }

    GatherGeneratorIncludes(form, set_src, set_hdr);

    // If an include is going to be generated in the header file, then don't also generate it
    // in the src file.
    for (const auto& iter: set_hdr)
    {
        if (auto pos = set_src.find(iter); pos != set_src.end())
        {
            set_src.erase(pos);
        }
    }
}

// Generate extern references to images used in the current form that are defined in the
// gen_Images node. These are written before the class constructor.
void CppCodeGenerator::WriteImagePreConstruction(Code& code)
{
    ASSERT_MSG(code.is_cpp(), "This function is only used for C++ code generation");
    code.clear();

    bool is_namespace_written = false;
    for (const auto* iter_array: m_embedded_images)
    {
        // If the image is in ImagesForm then it's header file will be included which already
        // has the extern declarations.
        if (iter_array->get_Form() == Project.get_ImagesForm())
        {
            continue;
        }

        if (!is_namespace_written)
        {
            is_namespace_written = true;
            code.Str("namespace wxue_img").OpenBrace();
        }
        code.Eol(eol_if_needed)
            .Str("extern const unsigned char ")
            .Str(iter_array->base_image().array_name);
        code.Str("[")
            .itoa((to_size_t) (iter_array->base_image().array_size & 0xFFFFFFFF))
            .Str("];");
        if (iter_array->base_image().filename.size())
        {
            code.Str("  // ").Str(iter_array->base_image().filename);
        }
    }

    if (is_namespace_written)
    {
        code.CloseBrace().Eol();
    }
}

void CppCodeGenerator::GatherGeneratorIncludes(Node* node, std::set<std::string>& set_src,
                                               std::set<std::string>& set_hdr)
{
    ASSERT_MSG(!node->is_NonWidget(),
               "Non-widget nodes should not be passed to GatherGeneratorIncludes()");

    bool isAddToSrc = false;

    // If the component is set for local access only, then add the header file to the source
    // set. Once all processing is done, if this header was also used by a component with
    // non-local access, then it will be removed from the source set.
    if (node->is_PropValue(prop_class_access, "none"))
    {
        isAddToSrc = true;
    }

    auto* generator = node->get_NodeDeclaration()->get_Generator();
    ASSERT(generator);
    if (!generator)
    {
        return;
    }

    generator->GetIncludes(node, set_src, set_hdr, m_language);

    if (node->HasValue(prop_subclass_header))
    {
        tt_string header("#include \"");
        header << node->as_string(prop_subclass_header) << '"';
        if (node->is_Form())
        {
            set_hdr.insert(header);
        }
        else
        {
            set_src.insert(header);
        }
    }

    if (!node->is_Form() && node->HasValue(prop_subclass) &&
        !node->is_PropValue(prop_class_access, "none"))
    {
        set_hdr.insert(tt_string() << "class " << node->as_string(prop_subclass) << ';');
    }

    // A lot of widgets have wxWindow and/or wxAnyButton as derived classes, and those classes
    // contain properties for font, color, and bitmaps. If the property is used, then we add a
    // matching header file.

    for (auto& iter: node->get_PropsVector())
    {
        if (iter.HasValue())
        {
            if (iter.type() == type_wxFont)
            {
                if (isAddToSrc)
                {
                    set_src.insert("#include <wx/font.h>");
                }
                else
                {
                    set_hdr.insert("#include <wx/font.h>");
                }
                FontProperty fontprop(iter);
                if (fontprop.isDefGuiFont())
                {
                    set_src.insert("#include <wx/settings.h>");
                }
                else
                {
                    fontprop.Convert(iter.get_value());
                    const auto point_size = fontprop.GetFractionalPointSize();
                    if (point_size <= 0)
                    {
                        set_src.insert("#include <wx/settings.h>");
                    }
                }
            }
            else if (iter.type() == type_wxColour)
            {
                if (isAddToSrc)
                {
                    set_src.insert("#include <wx/colour.h>");
                    set_src.insert(
                        "#include <wx/settings.h>");  // This is needed for the system colours
                }
                else
                {
                    set_hdr.insert("#include <wx/colour.h>");
                    set_hdr.insert(
                        "#include <wx/settings.h>");  // This is needed for the system colours
                }
            }
            else if (iter.type() == type_image)
            {
                if (m_ImagesForm && m_include_images_statement.size() &&
                    (iter.as_string().starts_with("Embed") || iter.as_string().starts_with("SVG")))
                {
                    set_src.insert(m_include_images_statement);
                }

                if (iter.as_string().starts_with("Art"))
                {
                    m_NeedArtProviderHeader = true;
                }

                if (auto function_name = ProjectImages.GetBundleFuncName(iter.as_string());
                    function_name.size())
                {
                    continue;
                }

                // The problem at this point is that we don't know how the bitmap will be used.
                // It could be just a wxBitmap, or it could be handed to a wxImage for sizing,
                // or it might be handed to wxWindow->SetIcon(). We play it safe and supply all
                // three header files.

                if (isAddToSrc)
                {
                    set_src.insert("#include <wx/bitmap.h>");
                    set_src.insert("#include <wx/icon.h>");
                    set_src.insert("#include <wx/image.h>");
                }
                else
                {
                    set_hdr.insert("#include <wx/bitmap.h>");
                    set_hdr.insert("#include <wx/icon.h>");
                    set_hdr.insert("#include <wx/image.h>");
                }
            }
        }
    }

    // Now parse all the children
    for (const auto& child: node->get_ChildNodePtrs())
    {
        GatherGeneratorIncludes(child.get(), set_src, set_hdr);
    }
}

void CppCodeGenerator::WriteImagePostHeader()
{
    auto* images_form = Project.get_ImagesForm();
    if (!images_form)
    {
        return;
    }

    bool is_namespace_written = false;
    for (const auto* iter_array: m_embedded_images)
    {
        if (iter_array->get_Form() == images_form)
        {
            continue;
        }

        if (!is_namespace_written)
        {
            m_header->writeLine();
            m_header->writeLine("namespace wxue_img\n{");

            m_header->Indent();
            is_namespace_written = true;
        }
        if (iter_array->base_image().filename.size())
        {
            m_header->writeLine(tt_string("// ") << iter_array->base_image().filename);
        }
        m_header->writeLine(tt_string("extern const unsigned char ")
                            << iter_array->base_image().array_name << '['
                            << (to_size_t) (iter_array->base_image().array_size & 0xFFFFFFFF)
                            << "];");
    }

    if (is_namespace_written)
    {
        m_header->Unindent();
        m_header->writeLine("}\n");
    }
}
