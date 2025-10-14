/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ code header content
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_cpp.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "code.h"            // Code -- Helper class for generating code
#include "tt_view_vector.h"  // tt_view_vector -- Read/Write line-oriented strings/files
#include "write_code.h"      // Write code to Scintilla or file

#include "../customprops/eventhandler_dlg.h"  // EventHandlerDlg static functions

using namespace code;

void CppCodeGenerator::GenerateCppClassHeader(bool class_namespace)
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (m_form_node->is_Gen(gen_Images) || m_form_node->is_Gen(gen_Data))
    {
        // There is a header for this, but it's not a class header
        return;
    }

    if (!m_form_node->HasValue(prop_class_name))
    {
        FAIL_MSG(tt_string("Missing \"name\" property in ") << m_form_node->get_DeclName());
        return;
    }

    auto* generator = m_form_node->get_NodeDeclaration()->get_Generator();
    Code code(m_form_node, GEN_LANG_CPLUSPLUS);

    // This may result in two blank lines, but without it there may be a case where there is no
    // blank line at all.
    m_header->writeLine();

    if (generator->PreClassHeaderCode(code))
    {
        m_header->writeLine(code);
        code.clear();
    }

    // If the class has a namespace, then this was already written
    if (!class_namespace && m_embedded_images.size())
    {
        WriteImagePostHeader();
        m_header->writeLine();
    }

    code.Str("class ");
    if (m_form_node->HasValue(prop_class_decoration))
    {
        code.as_string(prop_class_decoration) += " ";
    }
    code.as_string(prop_class_name) += " : public ";
    if (generator->BaseClassNameCode(code))
    {
        if (m_form_node->HasValue(prop_additional_inheritance))
        {
            tt_string_vector class_list(m_form_node->as_string(prop_additional_inheritance), '"',
                                        tt::TRIM::both);
            for (auto& iter: class_list)
            {
                code.Str(", public ").Str(iter);
            }
        }
        m_header->writeLine(code);
        code.clear();
    }
    else
    {
        if (!m_form_node->is_Type(type_DocViewApp) && !m_form_node->is_Type(type_wx_document))
        {
            FAIL_MSG("All form generators need to support BaseClassNameCode() to provide the class "
                     "name to derive from.");
        }
        else
        {
            // The only way this would be valid is if the base class didn't derive from anything.
            m_header->writeLine(tt_string() << "class " << m_form_node->as_string(prop_class_name));
        }
    }

    m_header->writeLine("{");
    m_header->writeLine("public:");
    m_header->Indent();
    m_header->SetLastLineBlank();

    // The set is used to prevent duplicates and to write the lines sorted. Call WriteSetLines()
    // to write the lines and clear the set.
    std::set<std::string> code_lines;
    CollectMemberVariables(m_form_node, Permission::Public, code_lines);
    if (code_lines.size())
    {
        WriteSetLines(m_header, code_lines);
        m_header->writeLine();
    }

    for (auto& member: m_map_public_members)
    {
        code.clear();
        BeginPlatformCode(code, member.first);
        m_header->writeLine(code);
        for (const auto& member_code: member.second)
        {
            m_header->writeLine(member_code);
        }
        m_header->writeLine("#endif  // limited to specific platforms");
    }

    GenCppEnumIds(m_form_node);

    if (m_form_node->as_bool(prop_const_values))
    {
        code.clear();
        if (m_form_node->HasProp(prop_id))
        {
            code.Eol(eol_if_needed).Str("static const int form_id = ");
            if (m_form_node->as_string(prop_id).size())
            {
                code.as_string(prop_id) += ";";
            }
            else
            {
                code.Str("wxID_ANY;");
            }
        }
        if (m_form_node->HasProp(prop_style))
        {
            code.Eol(eol_if_needed).Str("static const int form_style = ");
            if (m_form_node->as_string(prop_style).size())
            {
                code.as_string(prop_style) += ";";
            }
            else
            {
                code.Str("0;");
            }
        }
        else if (m_form_node->HasProp(prop_window_style))
        {
            code.Eol(eol_if_needed).Str("static const int form_style = ");
            if (m_form_node->as_string(prop_window_style).size())
            {
                code.as_string(prop_window_style) += ";";
            }
            else
            {
                code.Str("0;");
            }
        }
        if (m_form_node->HasProp(prop_pos))
        {
            code.Eol(eol_if_needed)
                .Str("static const wxPoint form_pos() { return ")
                .Pos(prop_pos, no_dpi_scaling) += "; }";
        }
        if (m_form_node->HasProp(prop_size))
        {
            code.Eol(eol_if_needed)
                .Str("static const wxSize form_size() { return  ")
                .WxSize(prop_size, no_dpi_scaling) += "; }";
        }
        if (m_form_node->HasProp(prop_title))
        {
            code.Eol(eol_if_needed).Str("static const wxString form_title() { return ");
            if (m_form_node->HasValue(prop_title))
            {
                code.Str("wxString::FromUTF8(\"").as_string(prop_title) += "\"); }";
            }
            else
            {
                code.Str("wxEmptyString; }");
            }
        }

        if (code.size())
        {
            m_header->writeLine(code);
            m_header->writeLine();
            m_header->writeLine();
        }
    }

    code.clear();
    if (generator->HeaderCode(code))
    {
        // MDI View classes need to have most of the code start with 4 spaces so that they can add a
        // non-indented "private:" section followed by wxDECLARE_DYNAMIC_CLASS().
        if (m_form_node->is_Type(type_wx_view))
        {
            if (auto save_indentation = m_header->get_Indentation(); save_indentation > 0)
            {
                m_header->Unindent();
                m_header->writeLine(code);
                m_header->Indent();
            }
            else
            {
                m_header->Indent();
            }
        }
        else
        {
            m_header->writeLine(code);
        }
    }
    else
    {
        m_header->SetLastLineBlank();
    }

    GenCppValidatorFunctions(m_form_node);
    m_header->writeLine();

    if (m_form_node->HasValue(prop_inserted_hdr_code))
    {
        WritePropHdrCode(m_form_node, prop_inserted_hdr_code);
    }

    if (m_form_node->HasValue(prop_class_methods))
    {
        tt_string_vector class_list(m_form_node->as_string(prop_class_methods), '"',
                                    tt::TRIM::both);
        if (class_list.size())
        {
            m_header->writeLine();
            for (auto& iter: class_list)
            {
                m_header->writeLine(iter);
            }
            m_header->writeLine();
        }
    }

    // TODO: There are a lot of function calls and section below that expect a protected
    // section.

    m_header->Unindent();
    m_header->writeLine("protected:");
    m_header->Indent();

    GenHdrEvents();

    if (m_form_node->HasValue(prop_protected_class_methods))
    {
        tt_string_vector class_list(m_form_node->as_string(prop_protected_class_methods), '"',
                                    tt::TRIM::both);
        if (class_list.size())
        {
            m_header->writeLine();
            for (auto& iter: class_list)
            {
                m_header->writeLine(iter);
            }
            m_header->writeLine();
        }
    }

    if (!m_form_node->as_bool(prop_use_derived_class) && m_form_node->as_bool(prop_private_members))
    {
        m_header->Unindent();
        m_header->writeLine();
        m_header->writeLine("private:");
        m_header->Indent();
    }

    CollectValidatorVariables(m_form_node, code_lines);
    if (code_lines.size() || m_map_protected.size())
    {
        m_header->writeLine();
        m_header->writeLine("// Validator variables");
        if (code_lines.size())
        {
            m_header->writeLine();
            WriteSetLines(m_header, code_lines);
        }
    }

    if (m_map_protected.size())
    {
        for (auto& member: m_map_protected)
        {
            code.clear();
            BeginPlatformCode(code, member.first);
            m_header->writeLine(code);
            for (const auto& code_line: member.second)
            {
                m_header->writeLine(code_line);
            }
            m_header->writeLine("#endif  // limited to specific platforms");
        }
        m_map_protected.clear();
    }

    CollectMemberVariables(m_form_node, Permission::Protected, code_lines);
    generator->AddProtectedHdrMembers(code_lines);

    if (code_lines.size() || m_map_protected.size())
    {
        m_header->writeLine();
        m_header->writeLine("// Class member variables");
        if (code_lines.size())
        {
            m_header->writeLine();
            WriteSetLines(m_header, code_lines);
        }
    }

    for (auto& member: m_map_protected)
    {
        code.clear();
        BeginPlatformCode(code, member.first);
        m_header->writeLine(code);
        for (const auto& code_line: member.second)
        {
            m_header->writeLine(code_line);
        }
        m_header->writeLine("#endif  // limited to specific platforms");
    }

    if (m_form_node->HasValue(prop_class_members))
    {
        tt_string_vector class_list(m_form_node->as_string(prop_class_members), '"',
                                    tt::TRIM::both);
        m_header->writeLine();
        for (auto& iter: class_list)
        {
            m_header->writeLine(iter);
        }
    }

    m_header->Unindent();

    if (not m_form_node->as_bool(prop_no_closing_brace))
    {
        m_header->writeLine("};");
    }
}

void CppCodeGenerator::GenHdrEvents()
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (m_events.size() || m_ctx_menu_events.size())
    {
        std::set<tt_string> code_lines;

        for (auto& event: m_events)
        {
            auto event_code = EventHandlerDlg::GetCppValue(event->get_value());
            // Ignore lambda's and functions in another class
            if (event_code.find("[") != std::string::npos || event_code.find("::") != std::string::npos)
            {
                continue;
            }

            tt_string code;

            // If the form has a wxContextMenuEvent node, then the handler for the form's
            // wxEVT_CONTEXT_MENU is a method of the base class and is not virtual.

            if (event->getNode()->is_Form() && event->get_name() == "wxEVT_CONTEXT_MENU")
            {
                bool has_handler = false;

                for (const auto& child: event->getNode()->get_ChildNodePtrs())
                {
                    if (child->is_Gen(gen_wxContextMenuEvent))
                    {
                        has_handler = true;
                        break;
                    }
                }

                if (has_handler)
                {
                    code << "void " << event_code << "("
                         << event->get_EventInfo()->get_event_class() << "& event);";
                    code_lines.insert(code);
                    continue;
                }
            }
            if (m_form_node->as_bool(prop_use_derived_class))
            {
                if (!m_form_node->as_bool(GenEnum::prop_pure_virtual_functions))
                {
                    code << "virtual void " << event_code << "("
                         << event->get_EventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "virtual void " << event_code << "("
                         << event->get_EventInfo()->get_event_class() << "& event) = 0;";
                }
            }
            else
            {
                code << "void " << event_code << "(" << event->get_EventInfo()->get_event_class()
                     << "& event);";
            }
            code_lines.insert(code);
        }

        // Unlike the above code, there shouldn't be any wxEVT_CONTEXT_MENU events since
        // m_ctx_menu_events should only contain menu items events.

        for (const auto& event: m_ctx_menu_events)
        {
            auto event_code = EventHandlerDlg::GetCppValue(event->get_value());
            // Ignore lambda's and functions in another class
            if (event_code.find("[") != std::string::npos || event_code.find("::") != std::string::npos)
            {
                continue;
            }

            tt_string code;

            if (m_form_node->as_bool(prop_use_derived_class))
            {
                if (!m_form_node->as_bool(GenEnum::prop_pure_virtual_functions))
                {
                    code << "virtual void " << event_code << "("
                         << event->get_EventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "virtual void " << event_code << "("
                         << event->get_EventInfo()->get_event_class() << "& event) = 0;";
                }
            }
            else
            {
                code << "void " << event_code << "(" << event->get_EventInfo()->get_event_class()
                     << "& event);";
            }

            code_lines.insert(code);
        }

        if (code_lines.size())
        {
            m_header->writeLine();
            if (m_form_node->as_bool(prop_use_derived_class))
            {
                m_header->writeLine(
                    "// Virtual event handlers -- override them in your derived class");
                m_header->writeLine();
            }
            else
            {
                m_header->writeLine("// Event handlers");
                m_header->writeLine();
            }
            for (const auto& iter: code_lines)
            {
                m_header->writeLine(iter.subview());
            }
        }
    }

    if (m_map_conditional_events.size())
    {
        auto sort_events_by_handler = [](NodeEvent* event_a, NodeEvent* event_b)
        {
            return (event_a->get_value() < event_b->get_value());
        };

        if (m_events.empty() && m_ctx_menu_events.empty())
        {
            m_header->writeLine();
            if (m_form_node->as_bool(prop_use_derived_class))
            {
                m_header->writeLine(
                    "// Virtual event handlers -- override them in your derived class");
            }
            else
            {
                m_header->writeLine("// Event handlers");
            }
        }

        for (auto& iter: m_map_conditional_events)
        {
            auto& events = iter.second;
            std::sort(events.begin(), events.end(), sort_events_by_handler);
            Code code(nullptr, GEN_LANG_CPLUSPLUS);
            BeginPlatformCode(code, iter.first);
            code.Eol();
            for (auto& event: events)
            {
                auto event_code = EventHandlerDlg::GetCppValue(event->get_value());
                // Ignore lambda's and functions in another class
                if (event_code.find("[") != std::string::npos || event_code.find("::") != std::string::npos)
                {
                    continue;
                }

                if (m_form_node->as_bool(prop_use_derived_class))
                {
                    code << "virtual void " << event_code << "("
                         << event->get_EventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "void " << event_code << "("
                         << event->get_EventInfo()->get_event_class() << "& event);";
                }
                code.Eol();
            }
            code << "#endif  // limited to specific platforms";
            code.Eol();
            m_header->writeLine(code);
        }
    }
}

void CppCodeGenerator::GenHdrNameSpace(tt_string& namespace_prop, tt_string_vector& names,
                                       size_t& indent)
{
    // namespace_prop can be a single or multiple namespaces separated by either :: or ;.
    // Replace both separator types with a single ':' character.
    namespace_prop.Replace("::", ":");
    namespace_prop.Replace(";", ":");
    names.SetString(namespace_prop, ':');

    tt_string using_name;
    m_header->writeLine();  // start with a blank line
    for (auto& iter: names)
    {
        m_header->writeLine(tt_string() << "namespace " << iter);
        m_header->writeLine("{");
        m_header->Indent();
        // This lets the caller know how much to indent the code inside the namespace
        ++indent;

        if (using_name.empty())
        {
            using_name = "using namespace ";
        }
        else
        {
            using_name += "::";
        }
        using_name += iter;
    }
    m_header->SetLastLineBlank();

    if (using_name.size())
    {
        using_name << ';';
        m_source->writeLine(using_name);
    }
}

void CppCodeGenerator::GenInitHeaderFile(std::set<std::string>& hdr_includes)
{
    std::vector<std::string> ordered_includes;
    if (auto pos = hdr_includes.find("#include <wx/generic/stattextg.h>");
        pos != hdr_includes.end())
    {
        hdr_includes.erase(pos);
        if (pos = hdr_includes.find("#include <wx/stattext.h>"); pos != hdr_includes.end())
        {
            hdr_includes.erase(pos);
        }

        if (ordered_includes.empty())
        {
            ordered_includes.emplace_back("// Order dependent includes");
        }

        ordered_includes.emplace_back("#include <wx/stattext.h>");
        ordered_includes.emplace_back("#include <wx/generic/stattextg.h>");
    }
    if (auto pos = hdr_includes.find("#include <wx/generic/treectlg.h>"); pos != hdr_includes.end())
    {
        hdr_includes.erase(pos);
        if (pos = hdr_includes.find("#include <wx/treectrl.h>"); pos != hdr_includes.end())
        {
            hdr_includes.erase(pos);
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
            m_header->writeLine(iter);
        }
        m_header->writeLine();
    }

    // First output all the wxWidget header files
    for (const auto& iter: hdr_includes)
    {
        if (tt::contains(iter, "<wx"))
        {
            m_header->writeLine(iter);
        }
    }

    m_header->writeLine();

    std::vector<std::string> namespaces;
    for (auto iter = hdr_includes.begin(); iter != hdr_includes.end();)
    {
        if (iter->starts_with("namespace "))
        {
            namespaces.emplace_back(*iter);
            iter = hdr_includes.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    // Now output all the other header files (this will include forward class declarations)
    for (const auto& iter: hdr_includes)
    {
        if (!tt::contains(iter, "<wx"))
        {
            m_header->writeLine(iter);
        }
    }

    m_header->writeLine();

    if (m_form_node->HasValue(prop_header_preamble))
    {
        WritePropHdrCode(m_form_node, prop_header_preamble);
    }

    if (m_form_node->HasValue(prop_system_hdr_includes))
    {
        m_header->writeLine();
        tt_view_vector list;
        list.SetString(m_form_node->as_string(prop_system_hdr_includes));
        for (auto& iter: list)
        {
            m_header->writeLine(tt_string("#include <") << iter << '>');
        }
    }

    if (m_form_node->HasValue(prop_local_hdr_includes))
    {
        m_header->writeLine();
        tt_view_vector list;
        list.SetString(m_form_node->as_string(prop_local_hdr_includes));
        for (auto& iter: list)
        {
            m_header->writeLine(tt_string("#include \"") << iter << '"');
        }
    }

    if (namespaces.size())
    {
        m_header->writeLine();
        for (auto& iter: namespaces)
        {
            tt_view_vector list(iter, '\n');

            // See gen_custom_ctrl.cpp -- GetIncludes(). Format is namespace name\n{\nclass
            // name;\n}
            m_header->writeLine(list[0]);
            m_header->writeLine(list[1]);
            m_header->Indent();

            for (size_t idx = 2; idx < list.size(); ++idx)
            {
                if (list[idx].starts_with("}"))
                {
                    m_header->Unindent();
                    m_header->writeLine(list[idx]);
                    break;
                }
                m_header->writeLine(list[idx]);
            }
        }
    }
}

void CppCodeGenerator::WritePropHdrCode(Node* node, GenEnum::PropName prop)
{
    tt_string convert(node->as_string(prop));
    convert.Replace("@@", "\n", tt::REPLACE::all);
    tt_string_vector lines(convert, '\n', tt::TRIM::right);
    bool initial_bracket = false;
    for (auto& code: lines)
    {
        if (code.contains("}") && !code.contains("{"))
        {
            m_header->Unindent();
        }
        else if (!initial_bracket && code.contains("["))
        {
            initial_bracket = true;
            m_header->Indent();
        }

        if (code.is_sameas("public:") || code.is_sameas("protected:") || code.is_sameas("private:"))
        {
            m_header->Unindent();
            m_header->writeLine(code, indent::auto_no_whitespace);
            m_header->Indent();
        }
        else
        {
            m_header->writeLine(code, indent::auto_no_whitespace);
        }

        if (code.contains("{") && !code.contains("}"))
        {
            m_header->Indent();
        }
    }
    m_header->writeLine();
}
