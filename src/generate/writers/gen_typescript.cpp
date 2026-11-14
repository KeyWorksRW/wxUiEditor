/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate TypeScript code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_typescript.h"

#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <wx/utils.h>  // wxGetEnv

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "code.h"             // Code -- Helper class for generating code
#include "ffi_emit.h"         // EmitFfiFormBody, GetFfiFormInfo
#include "ffi_ui.h"           // IsFfiUiLanguage
#include "gen_common.h"       // Common component functions
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "write_code.h"       // Write code to Scintilla or file

#include "wxue_namespace/wxue_string.h"       // wxue::string
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector

using namespace code;
using namespace GenEnum;

namespace gen_typescript_detail
{
    // Collects the creation-entry body in memory so the import block can be assembled once the
    // body -- and therefore the referenced identifiers -- is known.
    class TypeScriptBodyWriter : public WriteCode
    {
    public:
        void Clear() override { m_text.clear(); }
        [[nodiscard]] const wxue::string& GetText() const { return m_text; }

    protected:
        void doWrite(std::string_view text) override { m_text += text; }

    private:
        wxue::string m_text;
    };

    [[nodiscard]] std::string FormType(Node* form_node)
    {
        if (form_node->is_Gen(gen_wxWizard))
        {
            return "wxWizard";
        }
        if (form_node->is_Gen(gen_wxDialog) || form_node->is_Gen(gen_wxPropertySheetDialog))
        {
            return "wxDialog";
        }
        if (form_node->is_Gen(gen_wxPopupWindow))
        {
            return "wxPopupWindow";
        }
        if (form_node->is_Gen(gen_wxPopupTransientWindow))
        {
            return "wxPopupTransientWindow";
        }
        return "wxFrame";
    }

    [[nodiscard]] bool IsIdentifierStart(char character)
    {
        return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
               character == '_' || character == '$';
    }

    [[nodiscard]] bool IsIdentifierChar(char character)
    {
        return IsIdentifierStart(character) || (character >= '0' && character <= '9');
    }

    // A constant such as ID_ANY, DEFAULT_FRAME_STYLE or BOTH -- all A-Z/0-9/_, at least one letter.
    [[nodiscard]] bool IsConstantName(std::string_view name)
    {
        if (name.size() < 2)
        {
            return false;
        }

        bool has_letter = false;
        for (const char character: name)
        {
            if (character >= 'A' && character <= 'Z')
            {
                has_letter = true;
            }
            else if (!(character >= '0' && character <= '9') && character != '_')
            {
                return false;
            }
        }
        return has_letter;
    }

    void ClassifyImportName(std::string_view name, std::set<std::string>& kwx_names,
                            std::set<std::string>& platform_names)
    {
        if (name == "Pointer" || name == "Callback" || name == "platform" || name == "pointerOf")
        {
            platform_names.emplace(name);
            return;
        }

        if (name.starts_with("createWx") || name == "getPtr" || name == "kwx" ||
            name == "exitMainLoop")
        {
            kwx_names.emplace(name);
            return;
        }

        if (name.size() > 2 && name.starts_with("wx") && name[2] >= 'A' && name[2] <= 'Z')
        {
            kwx_names.emplace(name);
            return;
        }

        if (IsConstantName(name))
        {
            kwx_names.emplace(name);
        }
    }

    // Scans generated code for identifiers that must be imported. Line comments, block comments
    // and string literals are skipped so a class name inside a message never becomes an import.
    void CollectImportNames(std::string_view body, std::set<std::string>& kwx_names,
                            std::set<std::string>& platform_names)
    {
        enum class ScanState : std::uint8_t
        {
            normal,
            line_comment,
            block_comment,
            single_quote,
            double_quote,
            backtick
        };

        ScanState state = ScanState::normal;
        size_t index = 0;
        while (index < body.size())
        {
            const char character = body[index];
            switch (state)
            {
                case ScanState::normal:
                    if (character == '/' && index + 1 < body.size() && body[index + 1] == '/')
                    {
                        state = ScanState::line_comment;
                        index += 2;
                        break;
                    }
                    if (character == '/' && index + 1 < body.size() && body[index + 1] == '*')
                    {
                        state = ScanState::block_comment;
                        index += 2;
                        break;
                    }
                    if (character == '\'')
                    {
                        state = ScanState::single_quote;
                        ++index;
                        break;
                    }
                    if (character == '\"')
                    {
                        state = ScanState::double_quote;
                        ++index;
                        break;
                    }
                    if (character == '`')
                    {
                        state = ScanState::backtick;
                        ++index;
                        break;
                    }
                    if (IsIdentifierStart(character))
                    {
                        const size_t start = index;
                        while (index < body.size() && IsIdentifierChar(body[index]))
                        {
                            ++index;
                        }
                        ClassifyImportName(body.substr(start, index - start), kwx_names,
                                           platform_names);
                        break;
                    }
                    ++index;
                    break;

                case ScanState::line_comment:
                    if (character == '\n')
                    {
                        state = ScanState::normal;
                    }
                    ++index;
                    break;

                case ScanState::block_comment:
                    if (character == '*' && index + 1 < body.size() && body[index + 1] == '/')
                    {
                        state = ScanState::normal;
                        index += 2;
                        break;
                    }
                    ++index;
                    break;

                default:
                    // Inside a string literal -- honor backslash escapes.
                    if (character == '\\' && index + 1 < body.size())
                    {
                        index += 2;
                        break;
                    }
                    if ((state == ScanState::single_quote && character == '\'') ||
                        (state == ScanState::double_quote && character == '\"') ||
                        (state == ScanState::backtick && character == '`'))
                    {
                        state = ScanState::normal;
                    }
                    ++index;
                    break;
            }
        }
    }

    void WriteImportStatement(WriteCode* writer, const std::set<std::string>& names,
                              std::string_view module)
    {
        if (names.empty())
        {
            return;
        }

        if (names.size() <= 3)
        {
            wxue::string line = "import { ";
            bool is_first = true;
            for (const std::string& name: names)
            {
                if (!is_first)
                {
                    line << ", ";
                }
                line << name;
                is_first = false;
            }
            line << " } from \"" << module << "\";";
            writer->writeLine(line);
            return;
        }

        writer->writeLine("import {");
        writer->Indent();
        for (const std::string& name: names)
        {
            writer->writeLine(wxue::string(name) << ",");
        }
        writer->Unindent();
        writer->writeLine(wxue::string("} from \"") << module << "\";");
    }
}  // namespace gen_typescript_detail

using namespace gen_typescript_detail;

TypeScriptCodeGenerator::TypeScriptCodeGenerator(Node* form_node) :
    BaseCodeGenerator(GenLang::typescript, form_node)
{
}

void TypeScriptCodeGenerator::GenerateClass(GenLang language, PANEL_PAGE panel_type,
                                            wxProgressDialog* /* progress */)
{
    m_language = language;
    m_panel_type = panel_type;
    ASSERT(m_language == GenLang::typescript);
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
    m_source->writeLine("// This file was generated by wxUiEditor. Do not edit.");
    m_source->writeLine();

    if (m_form_node->is_Gen(gen_Images))
    {
        return;
    }

    if (!IsFfiUiLanguage(m_language))
    {
        return;
    }

    const FfiFormInfo info = GetFfiFormInfo(m_form_node, m_language);
    const std::string entry_name = "create" + info.pascal_name;
    const std::string return_type = FormType(m_form_node);

    // Build the creation body first -- the imports can only be determined once the generated
    // text is known.
    TypeScriptBodyWriter body_writer;
    body_writer.SetTabToSpaces(GetLanguageIndentSize(m_language),
                               GetLanguageIndentWithTabs(m_language));
    body_writer.Indent();

    std::string declarations;
    std::string handlers;
    if (!m_embedded_images.empty())
    {
        WriteImageConstruction(code);
    }
    std::ignore =
        EmitFfiFormBody(code, &body_writer, m_form_node, info, "frame", declarations, handlers);

    std::set<std::string> kwx_names;
    std::set<std::string> platform_names;
    CollectImportNames(body_writer.GetText(), kwx_names, platform_names);
    // Handler functions are written after the body but name the same wrapper types.
    CollectImportNames(handlers, kwx_names, platform_names);
    ClassifyImportName(return_type, kwx_names, platform_names);
    platform_names.emplace("Pointer");

    WriteImportStatement(m_source, kwx_names, "kwx");
    WriteImportStatement(m_source, platform_names, "kwx/platform");
    m_source->writeLine();

    m_source->writeLine("export function " + entry_name +
                        "(parent: Pointer | null): " + return_type);
    m_source->writeLine("{");

    const wxue::ViewVector body_lines(body_writer.GetText(), '\n');
    size_t line_count = body_lines.size();
    while (line_count > 0 && body_lines[line_count - 1].empty())
    {
        --line_count;
    }
    for (size_t index = 0; index < line_count; ++index)
    {
        m_source->writeLine(body_lines[index], indent::none);
    }

    m_source->Indent();
    m_source->writeLine("return frame;");
    m_source->Unindent();
    m_source->writeLine("}");

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
