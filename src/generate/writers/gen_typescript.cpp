/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate TypeScript code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_typescript.h"

#include <set>
#include <string>
#include <string_view>

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "code.h"             // Code -- Helper class for generating code
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

// Collects generated construction code in memory so the module scaffold can be assembled around it
// once the body -- and therefore the required imports -- is known.
class TypeScriptBodyWriter : public WriteCode
{
public:
    void Clear() override { m_text.clear(); }
    [[nodiscard]] const wxue::string& GetText() const { return m_text; }

protected:
    void doWrite(std::string_view code) override { m_text += code; }

private:
    wxue::string m_text;
};

// Name and declared type of the single widget field held by the generated class.
struct TypeScriptFormField
{
    std::string field_name;
    std::string field_type;
};

static TypeScriptFormField GetFormField(Node* form_node)
{
    if (form_node->is_Gen(gen_wxWizard))
    {
        return TypeScriptFormField { "wizard", "wxWizard" };
    }
    if (form_node->is_Gen(gen_wxDialog) || form_node->is_Gen(gen_wxPropertySheetDialog))
    {
        return TypeScriptFormField { "dialog", "wxDialog" };
    }
    if (form_node->is_Gen(gen_wxPopupWindow))
    {
        return TypeScriptFormField { "frame", "wxPopupWindow" };
    }
    if (form_node->is_Gen(gen_wxPopupTransientWindow))
    {
        return TypeScriptFormField { "frame", "wxPopupTransientWindow" };
    }
    return TypeScriptFormField { "frame", "wxFrame" };
}

// Converts "kwxMainFrame" to "KwxMainFrame".
static std::string MakeClassName(wxue::string_view name)
{
    std::string class_name(name);
    if (!class_name.empty() && class_name[0] >= 'a' && class_name[0] <= 'z')
    {
        class_name[0] = static_cast<char>(class_name[0] - 'a' + 'A');
    }
    return class_name;
}

// Converts "kwxMainFrame" to "createMainFrame" and "MyFrame" to "createMyFrame".
static std::string MakeFactoryName(wxue::string_view name)
{
    std::string_view view = name;
    if (view.starts_with("kwx"))
    {
        view.remove_prefix(3);
    }
    if (view.starts_with("wx"))
    {
        view.remove_prefix(2);
    }
    return "create" + std::string(view);
}

static bool IsIdentifierStart(char character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
           character == '_' || character == '$';
}

static bool IsIdentifierChar(char character)
{
    return IsIdentifierStart(character) || (character >= '0' && character <= '9');
}

// A constant such as ID_ANY, DEFAULT_FRAME_STYLE or BOTH -- all A-Z/0-9/_, at least one letter.
static bool IsConstantName(std::string_view name)
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

static void ClassifyImportName(std::string_view name, std::set<std::string>& kwx_names,
                               std::set<std::string>& platform_names)
{
    if (name == "Pointer" || name == "Callback" || name == "platform")
    {
        platform_names.emplace(name);
        return;
    }

    if (name.starts_with("createWx"))
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

// Scans generated code for identifiers that must be imported. Line comments, block comments and
// string literals are skipped so a class name inside a message never becomes an import.
static void CollectImportNames(std::string_view body, std::set<std::string>& kwx_names,
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
                if (character == '"')
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
                    (state == ScanState::double_quote && character == '"') ||
                    (state == ScanState::backtick && character == '`'))
                {
                    state = ScanState::normal;
                }
                ++index;
                break;
        }
    }
}

static void WriteImportStatement(WriteCode* writer, const std::set<std::string>& names,
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

    const LanguageTraits* traits = GetLanguageTraits(m_language);
    ASSERT_MSG(traits != nullptr, "TypeScript language traits must be available");
    std::string_view stmt_end = ";";
    if (traits)
    {
        stmt_end = traits->stmt_end;
    }

    const TypeScriptFormField field = GetFormField(m_form_node);
    const std::string class_name = MakeClassName(m_form_node->as_string(prop_class_name));
    const std::string factory_name = MakeFactoryName(m_form_node->as_string(prop_class_name));

    // Build the constructor body into memory first -- the imports can only be determined once the
    // generated text is known.
    TypeScriptBodyWriter body_writer;
    body_writer.SetTabToSpaces(GetLanguageIndentSize(m_language),
                               GetLanguageIndentWithTabs(m_language));
    // Two levels: one for the class body, one for the constructor body.
    body_writer.Indent();
    body_writer.Indent();

    WriteCode* const real_source = m_source;
    m_source = &body_writer;
    GenerateConstructionCode(code);

    // Write any embedded images that aren't declared in any gen_Images List.
    if (!m_embedded_images.empty())
    {
        WriteImageConstruction(code);
        m_source->writeLine();
    }
    m_source = real_source;

    std::set<std::string> kwx_names;
    std::set<std::string> platform_names;
    CollectImportNames(body_writer.GetText(), kwx_names, platform_names);

    // The field declaration and constructor signature live outside the scanned body.
    ClassifyImportName(field.field_type, kwx_names, platform_names);
    ClassifyImportName("Pointer", kwx_names, platform_names);

    WriteImportStatement(m_source, kwx_names, "kwx");
    WriteImportStatement(m_source, platform_names, "kwx/platform");
    m_source->writeLine();

    // Class definition
    m_source->writeLine(wxue::string("export class ") << class_name);
    m_source->writeLine("{");
    m_source->Indent();
    m_source->writeLine(wxue::string("readonly ")
                        << field.field_name << ": " << field.field_type << stmt_end);
    m_source->writeLine();
    m_source->writeLine("constructor(parent: Pointer | null)");
    m_source->writeLine("{");

    // The body already carries its own leading spaces, so write each line verbatim.
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

    m_source->writeLine("}");
    m_source->Unindent();
    m_source->writeLine("}");
    m_source->writeLine();

    // Factory function so callers can create the form with a single call.
    m_source->writeLine(wxue::string("export function ")
                        << factory_name << "(): " << field.field_type);
    m_source->writeLine("{");
    m_source->Indent();
    m_source->writeLine(wxue::string("return new ")
                        << class_name << "(null)." << field.field_name << stmt_end);
    m_source->Unindent();
    m_source->writeLine("}");

    // Reset indentation
    m_source->ResetIndent();
}

void TypeScriptCodeGenerator::GenerateConstructionCode(Code& code)
{
    BaseGenerator* generator = m_form_node->get_NodeDeclaration()->get_Generator();
    code.clear();
    if (generator->ConstructionCode(code))
    {
        m_source->writeLine(code);
        m_source->writeLine();
    }

    code.clear();
    if (generator->SettingsCode(code))
    {
        if (!code.empty())
        {
            m_source->writeLine(code);
            m_source->writeLine();
        }
    }

    if (m_form_node->get_PropPtr(prop_window_extra_style))
    {
        code.clear();
        code.GenWindowSettings();
        if (!code.empty())
        {
            m_source->writeLine(code);
        }
    }

    m_source->SetLastLineBlank();
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
        if (!code.empty())
        {
            m_source->writeLine();
            m_source->writeLine(code);
        }
    }
}
