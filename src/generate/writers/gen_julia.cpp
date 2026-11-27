/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Julia code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_julia.h"

#include <string>

#include <wx/utils.h>  // wxGetEnv

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "code.h"             // Code -- Helper class for generating code
#include "comment_blocks.h"   // Shared generated comment blocks
#include "ffi_emit.h"         // EmitFfiFormBody, GetFfiFormInfo
#include "ffi_ui.h"           // IsFfiUiLanguage
#include "gen_common.h"       // Common component functions
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "write_code.h"       // Write code to Scintilla or file

using namespace GenEnum;

JuliaCodeGenerator::JuliaCodeGenerator(Node* form_node) :
    BaseCodeGenerator(GenLang::julia, form_node)
{
}

void JuliaCodeGenerator::GenerateClass(GenLang language, PANEL_PAGE panel_type,
                                       wxProgressDialog* /* progress */)
{
    m_language = language;
    m_panel_type = panel_type;
    ASSERT(m_language == GenLang::julia);
    Code code(m_form_node, m_language);

    m_embedded_images.clear();
    SetImagesForm();
    std::set<std::string> img_include_set;
    CollectImageHeaders(m_form_node, img_include_set);
    ParseImageProperties(m_form_node);

    if (m_header)
    {
        m_header->Clear();
    }
    m_source->Clear();
    SetLanguageIndent(m_source, m_language);
    m_source->SetLastLineBlank();

    // File header comment
    m_source->writeLine(txt_JuliaCmtBlock);

    if (m_form_node->is_Gen(gen_Images))
    {
        return;
    }

    if (!IsFfiUiLanguage(m_language))
    {
        return;
    }

    const FfiFormInfo info = GetFfiFormInfo(m_form_node, m_language);
    const std::string entry_name = "create_" + info.snake;
    // The Julia idiomatic layer names its types after the wx class (`wxFrame`, `wxDialog`);
    // `Frame`/`Dialog` are not defined by the port, and a return-type annotation is
    // evaluated when the function is defined, so the generated file would not load with
    // those names.
    const std::string return_type = m_form_node->is_Gen(gen_wxDialog) ? "wxDialog" : "wxFrame";

    m_source->writeLine("using WxWidgets");
    if (!m_embedded_images.empty())
    {
        // An embedded bitmap is decoded from its base64 constant by BitmapExpr().
        m_source->writeLine("import Base64");
    }
    m_source->writeLine();

    // Every wrapper constructor takes the parent as `Union{wxWindow, Nothing}` (a raw handle
    // cannot be converted back into a wrapper object), which is the shape the port's samples
    // use: `wxFrame(nothing, ...)` for a top-level form, the parent form for a dialog.
    m_source->writeLine("function " + entry_name +
                        "(parent::Union{wxWindow, Nothing})::" + return_type);
    m_source->Indent();

    if (!m_embedded_images.empty())
    {
        WriteImageConstruction(code);
    }

    std::string declarations;
    std::string handlers;
    std::ignore =
        EmitFfiFormBody(code, m_source, m_form_node, info, "frame", declarations, handlers);

    m_source->writeLine();
    m_source->writeLine("return frame");
    m_source->Unindent();
    m_source->writeLine("end");

    if (!handlers.empty())
    {
        m_source->writeLine();
        m_source->writeLine(handlers);
    }

    m_source->ResetIndent();

    if (wxString sourcemap; wxGetEnv("WXUE_FFI_SOURCEMAP", &sourcemap) && !sourcemap.empty())
    {
        auto [output_path, has_base_file] = Project.GetOutputPath(m_form_node, m_language);
        if (has_base_file)
        {
            WriteFfiSourceMap(output_path.c_str());
        }
    }
}
