/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ and Python events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <tttextfile_wx.h>  // textfile -- Classes for reading and writing line-oriented files

#include "gen_base.h"

#include "../customprops/eventhandler_dlg.h"  // EventHandlerDlg static functions
#include "base_generator.h"                   // BaseGenerator -- Base Generator class
#include "code.h"                             // Code -- Helper class for generating code
#include "file_codewriter.h"                  // FileCodeWriter -- Classs to write code to disk
#include "lambdas.h"                          // Functions for formatting and storage of lamda events
#include "project_handler.h"                  // ProjectHandler class

using namespace code;

extern const char* python_end_cmt_line;  // "# ************* End of generated code"
extern const char* python_triple_quote;  // "\"\"\"";

/////////////////////////////////////////// Default generator event code ///////////////////////////////////////////

std::optional<ttlib::sview> BaseGenerator::GenEvents(Code& code, NodeEvent* event, const std::string& class_name)
{
    Code handler(event->GetNode(), code.m_language);

    // This is what we normally use if an ID is needed. However, a lambda needs to put the ID on it's own line, so we
    // use a string for this to allow the lambda processing code to replace it.
    std::string comma(", ");

    bool is_lambda { false };

    if (event->get_value().contains("["))
    {
        if (!code.is_cpp())
            return {};
        handler << event->get_name() << ',' << event->get_value();
        // Put the lambda expression on it's own line
        handler.m_code.Replace("[", "\n\t[");
        comma = ",\n\t";
        is_lambda = true;
        ExpandLambda(handler.m_code);
    }
    else if (event->get_value().contains("::"))
    {
        handler.Add(event->get_name()) << ", ";
        if (event->get_value()[0] != '&' && handler.is_cpp())
            handler << '&';
        handler << event->get_value();
    }
    else
    {
        handler.Add(event->get_name());
        if (code.is_cpp())
            handler << ", &" << class_name << "::" << event->get_value() << ", this";
        else
            handler.Add(", self.") << event->get_value();
    }

    // With lambdas, line break have already been added
    code.EnableAutoLineBreak(is_lambda ? false : true);

    // Do *NOT* assume that code.m_node is the same as event->GetNode()!

    if (event->GetNode()->IsStaticBoxSizer())
    {
        if (code.is_python())
            code.Add("self.");
        if (event->get_name() == "wxEVT_CHECKBOX")
        {
            code.Add(event->GetNode()->as_string(prop_checkbox_var_name));
        }
        else if (event->get_name() == "wxEVT_RADIOBUTTON")
        {
            code.Add(event->GetNode()->as_string(prop_radiobtn_var_name));
        }
        else
        {
            code.NodeName().Function("GetStaticBox()");
        }
        code.Function("Bind(") << handler.m_code;
        if (is_lambda)
            code << " ";
        code.EndFunction();
    }
    else if (event->GetNode()->isGen(gen_wxMenuItem) || event->GetNode()->isGen(gen_tool))
    {
        if (code.is_python())
            code.Add("self.");
        code << "Bind(" << handler.m_code << comma;
        if (event->GetNode()->as_string(prop_id) != "wxID_ANY")
        {
            if (code.is_python())
                code.Add("id=");
            code.Add(event->GetNode()->as_string(prop_id));
            code.EndFunction();
        }
        else
        {
            if (code.is_python())
                code.Add("id=");
            code << event->GetNode()->get_node_name();
            code.Function("GetId()").EndFunction();
        }
    }
    else if (event->GetNode()->isGen(gen_ribbonTool))
    {
        if (code.is_python())
            code.Add("self.");
        if (!event->GetNode()->HasValue(prop_id))
        {
            code.m_code += (code.is_cpp() ? "// " : "# ");
            code << "**WARNING** -- tool id not specified, event handler may never be called\n";
            code << "Bind(" << handler.m_code << comma;
            code.Add("wxID_ANY").EndFunction();
        }
        else
        {
            code << "Bind(" << handler.m_code << comma;
            code.Add(event->GetNode()->as_string(prop_id)).EndFunction();
        }
    }
    else if (event->GetNode()->IsForm())
    {
        if (code.is_python())
            code.Add("self.");
        code << "Bind(" << handler.m_code;
        if (is_lambda)
            code << " ";
        code.EndFunction();
    }
    else
    {
        if (code.is_python() && !event->GetNode()->IsLocal())
            code.Add("self.");
        code.Add(event->GetNode()->get_node_name()).Function("Bind(") << handler.m_code;
        if (is_lambda)
            code << " ";
        code.EndFunction();
    }

    code.EnableAutoLineBreak(true);
    return code.m_code;
}

/////////////////////////////////////////// C++ event code generation ///////////////////////////////////////////

void BaseCodeGenerator::GenSrcEventBinding(Node* node, EventVector& events)
{
    ASSERT_MSG(events.size(), "GenSrcEventBinding() shouldn't be called if there are no events");
    if (events.empty())
    {
        return;
    }

    auto propName = node->get_prop_ptr(prop_class_name);
    if (!propName)
    {
        FAIL_MSG(ttlib::cstr("Missing \"name\" property in ") << node->DeclName() << " class.");
        return;
    }

    auto& class_name = propName->as_string();
    if (class_name.empty())
    {
        FAIL_MSG("Property name cannot be null");
        return;
    }

    for (auto& iter: events)
    {
        auto lambda = [](NodeEvent* a, NodeEvent* b)
        {
            return (a->get_name() < b->get_name());
        };

        // Sort events by event name
        std::sort(events.begin(), events.end(), lambda);

        if (auto generator = iter->GetNode()->GetNodeDeclaration()->GetGenerator(); generator)
        {
            Code code(node, m_language);
            auto scode = generator->GenEvents(code, iter, class_name);
#if defined(_DEBUG)
            if (is_cpp())
            {
                if (auto result = generator->GenEvents(iter, class_name); result)
                {
                }
            }
#endif  // _DEBUG

            if (scode)
            {
                if (!scode->contains("["))
                {
                    size_t indentation = scode->contains("\n") ? indent::auto_keep_whitespace : indent::auto_no_whitespace;
                    m_source->writeLine(scode.value(), indentation);
                }
                else  // this is a lambda
                {
                    if (!is_cpp())
                    {
                        m_source->writeLine("# You cannot use C++ lambda functions as an event handler in wxPython.");
                    }
                    else
                    {
                        ttlib::cstr convert(scode.value());
                        convert.Replace("@@", "\n", tt::REPLACE::all);
                        ttlib::multistr lines(convert, '\n');
                        bool initial_bracket = false;
                        for (auto& line: lines)
                        {
                            if (line.contains("}"))
                            {
                                m_source->Unindent();
                            }
                            else if (!initial_bracket && line.contains("["))
                            {
                                initial_bracket = true;
                                m_source->Indent();
                            }

                            size_t indentation = indent::auto_no_whitespace;
                            m_source->writeLine(line, indentation);

                            if (line.contains("{"))
                            {
                                m_source->Indent();
                            }
                        }
                        m_source->Unindent();
                    }
                }
            }
        }
    }
}

void BaseCodeGenerator::GenHdrEvents(const EventVector& events)
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (events.size() || m_CtxMenuEvents.size())
    {
        std::set<ttlib::cstr> code_lines;

        for (auto& event: events)
        {
            // Ignore lambda's and functions in another class
            if (event->get_value().contains("[") || event->get_value().contains("::"))
                continue;

            ttlib::cstr code;

            // If the form has a wxContextMenuEvent node, then the handler for the form's wxEVT_CONTEXT_MENU is a method of
            // the base class and is not virtual.

            if (event->GetNode()->IsForm() && event->get_name() == "wxEVT_CONTEXT_MENU")
            {
                bool has_handler = false;

                for (const auto& child: event->GetNode()->GetChildNodePtrs())
                {
                    if (child->isGen(gen_wxContextMenuEvent))
                    {
                        has_handler = true;
                        break;
                    }
                }

                if (has_handler)
                {
                    code << "void " << event->get_value() << "(" << event->GetEventInfo()->get_event_class() << "& event);";
                    code_lines.insert(code);
                    continue;
                }
            }
            if ((event->get_name() == "wxEVT_WEBVIEW_FULL_SCREEN_CHANGED" ||
                 event->get_name() == "wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED") &&
                Project.value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#if wxCHECK_VERSION(3, 1, 5)\n";
                if (m_form_node->prop_as_bool(prop_use_derived_class))
                {
                    code << "virtual void " << event->get_value() << "(" << event->GetEventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "void " << event->get_value() << "(" << event->GetEventInfo()->get_event_class() << "& event);";
                }
                code << "\n#endif";
            }
            else
            {
                if (m_form_node->prop_as_bool(prop_use_derived_class))
                {
                    code << "virtual void " << event->get_value() << "(" << event->GetEventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "void " << event->get_value() << "(" << event->GetEventInfo()->get_event_class() << "& event);";
                }
            }
            code_lines.insert(code);
        }

        // Unlike the above code, there shouldn't be any wxEVT_CONTEXT_MENU events since m_CtxMenuEvents should only contain
        // menu items events.

        for (const auto& event: m_CtxMenuEvents)
        {
            // Ignore lambda's and functions in another class
            if (event->get_value().contains("[") || event->get_value().contains("::"))
                continue;

            ttlib::cstr code;

            if (m_form_node->prop_as_bool(prop_use_derived_class))
            {
                code << "virtual void " << event->get_value() << "(" << event->GetEventInfo()->get_event_class()
                     << "& event) { event.Skip(); }";
            }
            else
            {
                code << "void " << event->get_value() << "(" << event->GetEventInfo()->get_event_class() << "& event);";
            }

            code_lines.insert(code);
        }

        if (code_lines.size())
        {
            m_header->writeLine();
            if (m_form_node->prop_as_bool(prop_use_derived_class))
            {
                m_header->writeLine("// Virtual event handlers -- override them in your derived class");
                m_header->writeLine();
            }
            else
            {
                m_header->writeLine("// Event handlers");
                m_header->writeLine();
            }
            for (auto& iter: code_lines)
            {
                m_header->writeLine(iter.subview());
            }
        }
    }
}

/////////////////////////////////////////// Python event code generation ///////////////////////////////////////////

void BaseCodeGenerator::GenPythonEventHandlers(EventVector& events)
{
    // Multiple events can be bound to the same function, so use a set to make sure we only generate each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_PYTHON);
    if (events.size())
    {
        auto lambda = [](NodeEvent* a, NodeEvent* b)
        {
            return (a->get_value() < b->get_value());
        };

        // Sort events by function name
        std::sort(events.begin(), events.end(), lambda);

        bool inherited_class = m_form_node->HasValue(prop_python_inherit_name);
        if (!inherited_class)
        {
            m_header->Indent();
        }
        else
        {
            m_header->Unindent();
            m_header->writeLine();
        }

        bool found_user_handlers = false;
        if (m_panel_type == NOT_PANEL)
        {
            ttlib::viewfile org_file;
            ttlib::cstr path;
            if (auto& base_file = m_form_node->prop_as_string(prop_python_file); base_file.size())
            {
                path = base_file;
                if (path.size())
                {
                    if (auto* node_folder = m_form_node->get_folder();
                        node_folder && node_folder->HasValue(prop_folder_python_output_folder))
                    {
                        path = node_folder->as_string(prop_folder_python_output_folder);
                        path.append_filename(base_file.filename());
                    }
                    else if (Project.HasValue(prop_python_output_folder) && !path.contains("/"))
                    {
                        path = Project.BaseDirectory(GEN_LANG_PYTHON).utf8_string();
                        path.append_filename(base_file);
                        path += ".py";
                    }
                    path.backslashestoforward();
                }
            }

            if (path.size() && org_file.ReadFile(path))
            {
                size_t line_index;
                for (line_index = 0; line_index < org_file.size(); ++line_index)
                {
                    if (org_file[line_index].is_sameprefix(python_end_cmt_line))
                    {
                        break;
                    }
                }
                for (++line_index; line_index < org_file.size(); ++line_index)
                {
                    auto def = org_file[line_index].view_nonspace();
                    if (org_file[line_index].view_nonspace().starts_with("def "))
                    {
                        code_lines.emplace(def);
                        found_user_handlers = true;
                    }
                }
            }
        }

        if (found_user_handlers)
        {
            code.Str("# Unimplemented Event handler functions\n# Copy any listed and paste them below the comment block, or "
                     "to your inherited class.");
            code.Eol().Str(python_triple_quote).Eol();
        }
        else
        {
            code.Str("# Event handler functions\n# Add these below the comment block, or to your inherited class.");
            code.Eol().Str(python_triple_quote).Eol();
        }
        m_source->writeLine(code);

        code.clear();
        for (auto& event: events)
        {
            // Ignore lambda's and functions in another class
            if (event->get_value().contains("[") || event->get_value().contains("::"))
                continue;

            ttlib::cstr set_code;
            set_code << "def " << event->get_value() << "(self, event):";
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            code_lines.emplace(set_code);

            code.Str(set_code).Eol();
            code.Tab().Str("event.Skip()").Eol().Eol();
        }

        if (found_user_handlers)
        {
            m_header->writeLine("# Unimplemented Event handler functions");
        }
        else
        {
            m_header->writeLine("# Event handler functions");
        }
        m_header->writeLine(code);

        if (!inherited_class)
        {
            m_header->Unindent();
        }
        code.Eol(eol_if_needed).Str(python_triple_quote).Eol().Eol();
        m_source->writeLine(code);
    }
}
