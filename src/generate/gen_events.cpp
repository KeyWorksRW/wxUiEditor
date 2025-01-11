/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ and Python events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include <frozen/map.h>
#include <frozen/set.h>
#include <frozen/string.h>

#include "gen_base.h"

#include "../customprops/eventhandler_dlg.h"  // EventHandlerDlg static functions
#include "base_generator.h"                   // BaseGenerator -- Base Generator class
#include "base_generator.h"                   // BaseGenerator -- Base widget generator class
#include "code.h"                             // Code -- Helper class for generating code
#include "file_codewriter.h"                  // FileCodeWriter -- Classs to write code to disk
#include "lambdas.h"                          // Functions for formatting and storage of lamda events
#include "project_handler.h"                  // ProjectHandler class

using namespace code;

extern const char* cpp_rust_end_cmt_line;          // "// ************* End of generated code"
extern const char* perl_begin_cmt_block;           // "=pod" -- must be written with no indentation
extern const char* perl_end_cmt_block;             // "=cut" -- must be written with no indentation
extern const char* python_perl_ruby_end_cmt_line;  // "# ************* End of generated code"
extern const char* python_triple_quote;            // "\"\"\"";
extern const char* ruby_begin_cmt_block;           // "=begin";
extern const char* ruby_end_cmt_block;             // "=end";

/////////////////////////////////////////// Default generator event code ///////////////////////////////////////////

constexpr auto prop_sheet_events =
    frozen::make_map<std::string_view, std::string_view>({ { "OKButtonClicked", "wxID_OK" },
                                                           { "YesButtonClicked", "wxID_YES" },
                                                           { "ApplyButtonClicked", "wxID_APPLY" },
                                                           { "NoButtonClicked", "wxID_NO" },
                                                           { "CancelButtonClicked", "wxID_CANCEL" },
                                                           { "CloseButtonClicked", "wxID_CLOSE" },
                                                           { "HelpButtonClicked", "wxID_HELP" } });

void BaseGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    if (auto generator = event->getNode()->getGenerator();
        !generator || !generator->isLanguageVersionSupported(code.get_language()).first)
        return;  // Current language does not support this node

    Code handler(event->getNode(), code.m_language);
    tt_string event_code;
    if (code.m_language == GEN_LANG_CPLUSPLUS)
    {
        event_code = EventHandlerDlg::GetCppValue(event->get_value());
    }
    else if (code.m_language == GEN_LANG_PERL)
    {
        event_code = EventHandlerDlg::GetPerlValue(event->get_value());
    }
    else if (code.m_language == GEN_LANG_PYTHON)
    {
        event_code = EventHandlerDlg::GetPythonValue(event->get_value());
    }
    else if (code.m_language == GEN_LANG_RUBY)
    {
        event_code = EventHandlerDlg::GetRubyValue(event->get_value());
    }
    else if (code.m_language == GEN_LANG_RUST)
    {
        event_code = EventHandlerDlg::GetRustValue(event->get_value());
    }

    // REVIEW: [Randalphwa - 01-09-2025] Support for these is not currently planned, but they are
    // here in case they do get supported in the future.
#if GENERATE_NEW_LANG_CODE

    else if (code.m_language == GEN_LANG_FORTRAN)
    {
        event_code = EventHandlerDlg::GetFortranValue(event->get_value());
    }
    else if (code.m_language == GEN_LANG_HASKELL)
    {
        event_code = EventHandlerDlg::GetHaskellValue(event->get_value());
    }
    else if (code.m_language == GEN_LANG_LUA)
    {
        event_code = EventHandlerDlg::GetLuaValue(event->get_value());
    }
#endif  // GENERATE_NEW_LANG_CODE

    else
    {
        FAIL_MSG("Unknown language");
        event_code.clear();
    }

    if (event_code.empty() || event_code == "none")
        return;

    // This is what we normally use if an ID is needed. However, a C++ lambda needs to put the
    // ID on it's own line, so we use a string for this to allow the lambda processing code to
    // replace it.

    std::string comma(", ");

    bool is_lambda { false };

    if (event_code.contains("["))
    {
        if (code.is_cpp())
        {
            handler << event->get_name() << ',' << event_code;
            // Put the lambda expression on it's own line
            handler.GetCode().Replace("[", "\n\t[");
            comma = ",\n\t";
            ExpandLambda(handler.GetCode());
            is_lambda = true;
        }
        else if (code.is_python())
        {
            handler.Add(event->get_name()) += ", lambda event:";
            auto body_pos = event_code.find(']') + 1;
            event_code.erase(0, body_pos);
            handler.Str(event_code);
            is_lambda = true;
        }
        else if (code.is_ruby())
        {
            handler << event->get_name() << ',' << event_code;
            // Put the lambda expression on it's own line
            handler.GetCode().Replace("[", "\n\t{");
            comma = ",\n\t";
            ExpandLambda(handler.GetCode());
            is_lambda = true;
        }
    }
    else if (event_code.contains("::"))
    {
        handler.Add(event->get_name()) << ", ";
        if (code.is_cpp())
        {
            if (event_code[0] != '&' && handler.is_cpp())
                handler << '&';
        }
        else
        {
            // We don't know what module this function is in, so I'm not sure this will
            // acutally work. The user will instead need to create a Python function
            // that starts with the module name.
            event_code.Replace("::", ".");
        }

        handler << event_code;
    }
    else
    {
        tt_string event_name = event->get_name();
        if (auto result = prop_sheet_events.find(event_name); result != prop_sheet_events.end())
        {
            event_name = "wxEVT_BUTTON";
        }
        if (code.is_cpp() || code.is_python())
        {
            handler.Add(event_name);
            if (code.is_cpp())
                handler << ", &" << class_name << "::" << event_code << ", this";
            else if (code.is_python())
                handler.Add(", self.") << event_code;
        }
        else if (code.is_perl())
        {
            // remove "wx" prefix
            event_name.erase(0, 2);
            handler.Str(event_name).Str("($self, ").NodeName().Str("->GetId(), $self->can('") << event_code << "'));";
        }
        else if (code.is_ruby())
        {
            // remove "wx" prefix, make the rest of the name lower case
            event_name.erase(0, 2);
            std::transform(event_name.begin(), event_name.end(), event_name.begin(),
                           [](unsigned char c)
                           {
                               return std::tolower(c);
                           });

            if (event->getNode()->isForm())
            {
                if (event->get_name().starts_with("wxEVT_WIZARD"))
                {
                    handler.Str(event_name).Str("(get_id, ").Str(":") << event_code << ')';
                }
                else
                {
                    handler.Str(event_name).Str("(:") << event_code << ')';
                }
            }
            else if (event->getEventInfo()->get_name() == "wxEVT_SIZE" ||
                     event->getEventInfo()->get_name() == "wxEVT_GRID_COL_SIZE")
            {
                // wxRuby3 doesn't allow an id for this event
                handler.Str(event_name).Str("(:") << event_code << ')';
            }
            else if (event->getNode()->isGen(gen_StaticCheckboxBoxSizer))
            {
                code.Str(event_name).Str("(").VarName(event->getNode()->as_string(prop_checkbox_var_name)).Str(".get_id, :")
                    << event_code << ')';
                return;
            }
            else if (event->getNode()->isGen(gen_StaticRadioBtnBoxSizer))
            {
                code.Str(event_name).Str("(").VarName(event->getNode()->as_string(prop_radiobtn_var_name)).Str(".get_id, :")
                    << event_code << ')';
                return;
            }
            else
            {
                handler.Str(event_name).Str("(").NodeName().Str(".get_id, :") << event_code << ')';
            }
        }
        else if (code.is_rust())
        {
            // wxRust1 uses contro.connect and a generated extern "C" function
            // wxRust2 uses form.base.bind(wx::RustEvent::..., move |event: &wx::EventType| { ... })
        }
    }

    // With lambdas, line breaks have already been added
    code.EnableAutoLineBreak(is_lambda ? false : true);

    // Do *NOT* assume that code.m_node is the same as event->getNode()!

    if (event->getNode()->isStaticBoxSizer())
    {
        ASSERT_MSG(!code.is_ruby(), "StaticBoxSizer events have already been handled for Ruby");
        code.AddIfPython("self.");
        if (event->get_name() == "wxEVT_CHECKBOX")
        {
            code.Add(event->getNode()->as_string(prop_checkbox_var_name));
        }
        else if (event->get_name() == "wxEVT_RADIOBUTTON")
        {
            code.Add(event->getNode()->as_string(prop_radiobtn_var_name));
        }
        else
        {
            code.NodeName(event->getNode()).Function("GetStaticBox()");
        }

        if (code.is_cpp() || code.is_python())
        {
            code.Function("Bind(") << handler.GetCode();
        }
        else if (code.is_ruby())
        {
            code.Function("") << handler.GetCode();
        }
        code.EndFunction();
    }
    else if (event->getNode()->isGen(gen_wxMenuItem) || event->getNode()->isGen(gen_tool) ||
             event->getNode()->isGen(gen_auitool))
    {
        if (code.is_cpp() || code.is_python())
        {
            code.AddIfPython("self.");
            code << "Bind(" << handler.GetCode() << comma;
            if (event->getNode()->as_string(prop_id) != "wxID_ANY")
            {
                auto id = event->getNode()->getPropId();
                code.AddIfPython("id=").Add(id).EndFunction();
            }
            else
            {
                code.AddIfPython("id=").NodeName(event->getNode()).Function("GetId()").EndFunction();
            }
        }
        else
        {
            code << handler;
        }
    }
    else if (event->getNode()->isGen(gen_ribbonTool))
    {
        if (code.is_python())
            code.Add("self.");
        if (!event->getNode()->hasValue(prop_id))
        {
            code.AddComment("**WARNING** -- tool id not specified, event handler may never be called.", true);
            if (code.is_cpp() || code.is_python())
            {
                code << "Bind(" << handler.GetCode() << comma;
                code.Add("wxID_ANY").EndFunction();
            }
            else if (code.is_ruby())
            {
                code << handler;
            }
        }
        else
        {
            if (code.is_cpp() || code.is_python())
            {
                code << "Bind(" << handler.GetCode() << comma;
                code.Add(event->getNode()->as_string(prop_id)).EndFunction();
            }
            else if (code.is_ruby())
            {
                code << handler;
            }
        }
    }
    else if (event->getNode()->isForm())
    {
        if (code.is_cpp() || code.is_python())
        {
            code.AddIfPython("self.");
            code << "Bind(" << handler.GetCode();
            if (auto result = prop_sheet_events.find(event->get_name()); result != prop_sheet_events.end())
            {
                code.Comma() << result->second;
            }
            code.EndFunction();
        }
        else if (code.is_ruby())
        {
            code << handler;
            if (auto result = prop_sheet_events.find(event->get_name()); result != prop_sheet_events.end())
            {
                code.Comma() << result->second;
            }
        }
    }
    else
    {
        if (code.is_cpp() || code.is_python())
        {
            code.NodeName(event->getNode()).Function("Bind(") << handler.GetCode();
            code.EndFunction();
        }
        else if (code.is_ruby())
        {
            code << handler;
        }
    }

    code.EnableAutoLineBreak(true);
}

void BaseCodeGenerator::GenSrcEventBinding(Node* node, EventVector& events)
{
    ASSERT_MSG(events.size() || m_map_conditional_events.size(),
               "GenSrcEventBinding() shouldn't be called if there are no events");
    if (events.empty() && m_map_conditional_events.empty())
    {
        return;
    }

    auto propName = node->getPropPtr(prop_class_name);
    if (!propName)
    {
        FAIL_MSG(tt_string("Missing \"name\" property in ") << node->declName() << " class.");
        return;
    }

    auto& class_name = propName->as_string();
    if (class_name.empty())
    {
        FAIL_MSG("Property name cannot be null");
        return;
    }

    Code code(node, m_language);

    auto sort_by_event_name = [](NodeEvent* a, NodeEvent* b)
    {
        // If the event names are the same, then sort by the event handler name
        if (a->get_name() == b->get_name())
        {
            // If the event handler names are the same, then sort by the property id
            if (a->get_value() == b->get_value())
            {
                // If the property id's are the same, then sort by the node's var_name
                if (a->getNode()->as_string(prop_id) == b->getNode()->as_string(prop_id))
                    return (a->getNode()->as_string(prop_var_name) < b->getNode()->as_string(prop_var_name));
                else
                    return (a->getNode()->as_string(prop_id) < b->getNode()->as_string(prop_id));
            }
            else
                return (a->get_value() < b->get_value());
        }
        else
            return (a->get_name() < b->get_name());
    };

    // Sort events by event name
    std::sort(events.begin(), events.end(), sort_by_event_name);

    // The node for each event handler might be conditionalized, or the node might be in a
    // container that is conditionalized. If so, then events need to be grouped into their
    // conditional sections, and written out within a conditional block.

    for (auto& event: events)
    {
        if (auto generator = event->getNode()->getGenerator(); generator)
        {
            code.clear();
            if (generator->GenEvent(code, event, class_name); code.size())
            {
                if (!code.GetCode().contains("["))
                {
                    m_source->writeLine(code);
                }
                else  // this is a lambda
                {
                    if (!is_cpp())
                    {
                        m_source->writeLine("# You can only use C++ lambda functions as an event handler C++ code.");
                    }
                    else
                    {
                        tt_string convert(code.GetCode());
                        convert.Replace("@@", "\n", tt::REPLACE::all);
                        tt_string_vector lines(convert, '\n');
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

    for (auto& map_entry: m_map_conditional_events)
    {
        auto& conditional_events = map_entry.second;
        std::sort(conditional_events.begin(), conditional_events.end(), sort_by_event_name);

        code.clear();
        BeginPlatformCode(code, map_entry.first);
        code.Eol();
        m_source->writeLine(code);
        if (m_language == GEN_LANG_PYTHON || m_language == GEN_LANG_RUBY)
            m_source->Indent();

        for (auto& conditional_event: conditional_events)
        {
            code.clear();
            if (auto generator = conditional_event->getNode()->getGenerator(); generator)
            {
                if (generator->GenEvent(code, conditional_event, class_name); code.size())
                {
                    if (!code.GetCode().contains("["))
                    {
                        m_source->writeLine(code);
                    }
                    else  // this is a lambda
                    {
                        if (!is_cpp())
                        {
                            m_source->writeLine("# You can only use C++ lambda functions as an event handler C++ code.");
                        }
                        else
                        {
                            tt_string convert(code.GetCode());
                            convert.Replace("@@", "\n", tt::REPLACE::all);
                            tt_string_vector lines(convert, '\n');
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

        EndPlatformCode();
        m_source->writeLine();
    }
}

void BaseCodeGenerator::GenHdrEvents()
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (m_events.size() || m_ctx_menu_events.size())
    {
        std::set<tt_string> code_lines;

        for (auto& event: m_events)
        {
            auto event_code = EventHandlerDlg::GetCppValue(event->get_value());
            // Ignore lambda's and functions in another class
            if (event_code.contains("[") || event_code.contains("::"))
                continue;

            tt_string code;

            // If the form has a wxContextMenuEvent node, then the handler for the form's wxEVT_CONTEXT_MENU is a method
            // of the base class and is not virtual.

            if (event->getNode()->isForm() && event->get_name() == "wxEVT_CONTEXT_MENU")
            {
                bool has_handler = false;

                for (const auto& child: event->getNode()->getChildNodePtrs())
                {
                    if (child->isGen(gen_wxContextMenuEvent))
                    {
                        has_handler = true;
                        break;
                    }
                }

                if (has_handler)
                {
                    code << "void " << event_code << "(" << event->getEventInfo()->get_event_class() << "& event);";
                    code_lines.insert(code);
                    continue;
                }
            }
            if ((event->get_name() == "wxEVT_WEBVIEW_FULL_SCREEN_CHANGED" ||
                 event->get_name() == "wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED") &&
                Project.is_wxWidgets31())
            {
                code << "\n#if wxCHECK_VERSION(3, 1, 5)\n";
                if (m_form_node->as_bool(prop_use_derived_class))
                {
                    code << "virtual void " << event->get_value() << "(" << event->getEventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "void " << event_code << "(" << event->getEventInfo()->get_event_class() << "& event);";
                }
                code << "\n#endif";
            }
            else
            {
                if (m_form_node->as_bool(prop_use_derived_class))
                {
                    code << "virtual void " << event_code << "(" << event->getEventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "void " << event_code << "(" << event->getEventInfo()->get_event_class() << "& event);";
                }
            }
            code_lines.insert(code);
        }

        // Unlike the above code, there shouldn't be any wxEVT_CONTEXT_MENU events since m_ctx_menu_events should only
        // contain menu items events.

        for (const auto& event: m_ctx_menu_events)
        {
            auto event_code = EventHandlerDlg::GetCppValue(event->get_value());
            // Ignore lambda's and functions in another class
            if (event_code.contains("[") || event_code.contains("::"))
                continue;

            tt_string code;

            if (m_form_node->as_bool(prop_use_derived_class))
            {
                code << "virtual void " << event_code << "(" << event->getEventInfo()->get_event_class()
                     << "& event) { event.Skip(); }";
            }
            else
            {
                code << "void " << event_code << "(" << event->getEventInfo()->get_event_class() << "& event);";
            }

            code_lines.insert(code);
        }

        if (code_lines.size())
        {
            m_header->writeLine();
            if (m_form_node->as_bool(prop_use_derived_class))
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

    if (m_map_conditional_events.size())
    {
        auto sort_events_by_handler = [](NodeEvent* a, NodeEvent* b)
        {
            return (a->get_value() < b->get_value());
        };

        if (m_events.empty() && m_ctx_menu_events.empty())
        {
            m_header->writeLine();
            if (m_form_node->as_bool(prop_use_derived_class))
            {
                m_header->writeLine("// Virtual event handlers -- override them in your derived class");
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
                if (event_code.contains("[") || event_code.contains("::"))
                    continue;

                if (m_form_node->as_bool(prop_use_derived_class))
                {
                    code << "virtual void " << event_code << "(" << event->getEventInfo()->get_event_class()
                         << "& event) { event.Skip(); }";
                }
                else
                {
                    code << "void " << event_code << "(" << event->getEventInfo()->get_event_class() << "& event);";
                }
                code.Eol();
            }
            code << "#endif  // limited to specific platforms";
            code.Eol();
            m_header->writeLine(code);
        }
    }
}

// This function simply generates unhandled event handlers in a multi-string comment.
void BaseCodeGenerator::GenCppEventHandlers(EventVector& events)
{
    ASSERT_MSG(events.size(), "GenCppEventHandlers() shouldn't be called if there are no events");
    if (events.empty() || m_form_node->as_bool(prop_use_derived_class))
    {
        return;
    }

    // Multiple events can be bound to the same function, so use a set to make sure we only generate each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_CPLUSPLUS);
    auto sort_event_handlers = [](NodeEvent* a, NodeEvent* b)
    {
        return (EventHandlerDlg::GetCppValue(a->get_value()) < EventHandlerDlg::GetCppValue(b->get_value()));
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
            if (auto& extProp = Project.as_string(prop_source_ext); extProp.size())
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
            size_t line_index;
            for (line_index = 0; line_index < org_file.size(); ++line_index)
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
                continue;

            tt_string set_code;
            set_code << "void " << m_form_node->getNodeName() << "::" << handler;
            for (auto& iter: code_lines)
            {
                if (iter.starts_with(set_code))
                {
                    // This event handler has already been created by the user
                    set_code.clear();
                    break;
                }
            }
            if (set_code.empty())
                continue;

            // At least one event wasn't implemented, so stop looking for more
            is_all_events_implemented = false;

            code.Str("// Unimplemented Event handler functions\n// Copy any of the following and paste them below the "
                     "comment block, or "
                     "to your inherited class.");
            code.Eol().Str("\n/*").Eol();
            break;
        }
        if (is_all_events_implemented)
        {
            // If the user has defined all the event handlers, then we don't need to output anything else.
            return;
        }
    }
    else
    {
        // The user hasn't defined their own event handlers in this module
        is_all_events_implemented = false;

        code.Str("// Unimplemented Event handler functions\n// Copy any of the following and paste them below the comment "
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
                continue;

            // The user's declaration will typically include the event parameter, often
            tt_string set_code;
            set_code << "void " << m_form_node->getNodeName() << "::" << handler << '(';
            for (auto& iter: code_lines)
            {
                if (iter.starts_with(set_code))
                {
                    // This event handler has already been created by the user
                    set_code.clear();
                    break;
                }
            }
            if (set_code.empty())
                continue;

            // Add it to our set of handled events in case the user specified
            // the same event handler for multiple events.
            code_lines.emplace(set_code);

            code.Str(set_code).Eol();
            code.OpenBrace();
#if defined(_DEBUG)
            auto& dbg_event_name = event->get_name();
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

// This function simply generates unhandled event handlers in a multi-string comment.

void BaseCodeGenerator::GenPerlEventHandlers(EventVector& events)
{
    ASSERT_MSG(events.size(), "GenPerlEventHandlers() shouldn't be called if there are no events");
    if (events.empty())
    {
        return;
    }

    // Multiple events can be bound to the same function, so use a set to make sure we only generate each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_PERL);
    auto sort_event_handlers = [](NodeEvent* a, NodeEvent* b)
    {
        return (EventHandlerDlg::GetPerlValue(a->get_value()) < EventHandlerDlg::GetPerlValue(b->get_value()));
    };

    // Sort events by function name
    std::sort(events.begin(), events.end(), sort_event_handlers);

    bool found_user_handlers = false;
    if (m_panel_type == NOT_PANEL)
    {
        tt_view_vector org_file;
        auto [path, has_base_file] = Project.GetOutputPath(m_form_node, GEN_LANG_PERL);

        if (has_base_file && path.extension().empty())
        {
            path += ".pl";
        }

        // If the user has defined any event handlers, add them to the code_lines set so we
        // don't generate them again.
        if (has_base_file && org_file.ReadFile(path))
        {
            size_t line_index;
            for (line_index = 0; line_index < org_file.size(); ++line_index)
            {
                if (org_file[line_index].is_sameprefix(python_perl_ruby_end_cmt_line))
                {
                    break;
                }
            }
            for (++line_index; line_index < org_file.size(); ++line_index)
            {
                auto handler = org_file[line_index].view_nonspace();
                if (org_file[line_index].view_nonspace().starts_with("sub "))
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
        for (auto& event: events)
        {
            auto handler = EventHandlerDlg::GetPerlValue(event->get_value());
            // Ignore lambda's
            if (handler.starts_with("[perl:lambda]"))
                continue;

            tt_string set_code;
            // If the user doesn't use the `event` parameter, they may use '_' instead to indicate
            // an unused parameter.
            set_code << "sub " << handler << " {";
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            set_code << "sub " << handler << " {";
            if (code_lines.find(set_code) != code_lines.end())
                continue;

            // At least one event wasn't implemented, so stop looking for more
            is_all_events_implemented = false;

            code.Str("# Unimplemented Event handler functions\n# Copy any listed and paste them below the comment block, or "
                     "to your inherited class.");
            break;
        }
        if (is_all_events_implemented)
        {
            // If the user has defined all the event handlers, then we don't need to output anything else.
            return;
        }
    }
    else
    {
        // The user hasn't defined their own event handlers in this module
        is_all_events_implemented = false;

        code.Str("# Event handler functions\n# Add these below the comment block, or to your inherited class.");
    }
    m_source->writeLine(code);

    code.clear();
    if (!is_all_events_implemented)
    {
        for (auto& event: events)
        {
            auto handler = EventHandlerDlg::GetPerlValue(event->get_value());
            // Ignore lambda's
            if (handler.empty() || handler.starts_with("[perl:lambda]"))
                continue;

            tt_string set_code;
            // If the user doesn't use the `event` parameter, they may use '_' instead to indicate
            // an unused parameter.
            set_code << "sub " << handler << " {";
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            code_lines.emplace(set_code);

            code.Str(set_code).Eol();
#if defined(_DEBUG)
            auto& dbg_event_name = event->get_name();
            wxUnusedVar(dbg_event_name);
#endif  // _DEBUG
            if (event->get_name() == "CloseButtonClicked")
            {
                code.Tab().Str("$self->EndModal(wxID_CLOSE);").Eol().Eol();
            }
            else if (event->get_name() == "YesButtonClicked")
            {
                code.Tab().Str("$self->EndModal(wxID_YES);").Eol().Eol();
            }
            else if (event->get_name() == "NoButtonClicked")
            {
                code.Tab().Str("$self->EndModal(wxID_NO);").Eol().Eol();
            }
            else
            {
                code.Tab().Str("$event->Skip();").Eol().Eol();
            }
        }
    }

    // Write the unimplemented event handlers to the source file without the
    // comment block to make it easier for the user to cut and paste and/or add
    // them to an inherited class.
    if (found_user_handlers && !is_all_events_implemented)
    {
        m_header->writeLine("# Unimplemented Event handler functions");
    }
    else
    {
        m_header->writeLine("# Event handler functions");
    }
    m_header->writeLine(code);

    if (!is_all_events_implemented)
    {
        m_source->ResetIndent();
        m_source->writeLine(perl_begin_cmt_block);
        m_source->Indent();
        m_source->writeLine(code);
        m_source->Unindent();
        m_source->writeLine(perl_end_cmt_block);
        m_source->Indent();
    }
}

// This function simply generates unhandled event handlers in a multi-string comment.

void BaseCodeGenerator::GenPythonEventHandlers(EventVector& events)
{
    ASSERT_MSG(events.size(), "GenPythonEventHandlers() shouldn't be called if there are no events");
    if (events.empty())
    {
        return;
    }

    // Multiple events can be bound to the same function, so use a set to make sure we only generate each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_PYTHON);
    auto sort_event_handlers = [](NodeEvent* a, NodeEvent* b)
    {
        return (EventHandlerDlg::GetPythonValue(a->get_value()) < EventHandlerDlg::GetPythonValue(b->get_value()));
    };

    // Sort events by function name
    std::sort(events.begin(), events.end(), sort_event_handlers);

    bool inherited_class = m_form_node->hasValue(prop_python_inherit_name);
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
        tt_view_vector org_file;
        auto [path, has_base_file] = Project.GetOutputPath(m_form_node, GEN_LANG_PYTHON);

        if (has_base_file && path.extension().empty())
        {
            path += ".py";
        }

        // If the user has defined any event handlers, add them to the code_lines set so we
        // don't generate them again.
        if (has_base_file && org_file.ReadFile(path))
        {
            size_t line_index;
            for (line_index = 0; line_index < org_file.size(); ++line_index)
            {
                if (org_file[line_index].is_sameprefix(python_perl_ruby_end_cmt_line))
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

    bool is_all_events_implemented = true;
    if (found_user_handlers)
    {
        for (auto& event: events)
        {
            auto python_handler = EventHandlerDlg::GetPythonValue(event->get_value());
            // Ignore lambda's
            if (python_handler.starts_with("[python:lambda]"))
                continue;

            tt_string set_code;
            // If the user doesn't use the `event` parameter, they may use '_' instead to indicate
            // an unused parameter.
            set_code << "def " << python_handler << "(self, _):";
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            set_code << "def " << python_handler << "(self, event):";
            if (code_lines.find(set_code) != code_lines.end())
                continue;

            // At least one event wasn't implemented, so stop looking for more
            is_all_events_implemented = false;

            code.Str("# Unimplemented Event handler functions\n# Copy any listed and paste them below the comment block, or "
                     "to your inherited class.");
            code.Eol().Str(python_triple_quote).Eol();
            break;
        }
        if (is_all_events_implemented)
        {
            // If the user has defined all the event handlers, then we don't need to output anything else.
            return;
        }
    }
    else
    {
        // The user hasn't defined their own event handlers in this module
        is_all_events_implemented = false;

        code.Str("# Event handler functions\n# Add these below the comment block, or to your inherited class.");
        code.Eol().Str(python_triple_quote).Eol();
    }
    m_source->writeLine(code);

    code.clear();
    if (!is_all_events_implemented)
    {
        for (auto& event: events)
        {
            auto python_handler = EventHandlerDlg::GetPythonValue(event->get_value());
            // Ignore lambda's
            if (python_handler.empty() || python_handler.starts_with("[python:lambda]"))
                continue;

            tt_string set_code;
            // If the user doesn't use the `event` parameter, they may use '_' instead to indicate
            // an unused parameter.
            set_code << "def " << python_handler << "(self, _):";
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            set_code.Replace("_)", "event)");
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            code_lines.emplace(set_code);

            code.Str(set_code).Eol();
#if defined(_DEBUG)
            auto& dbg_event_name = event->get_name();
            wxUnusedVar(dbg_event_name);
#endif  // _DEBUG
            if (event->get_name() == "CloseButtonClicked")
            {
                code.Tab().Str("self.EndModal(wx.ID_CLOSE)").Eol().Eol();
            }
            else if (event->get_name() == "YesButtonClicked")
            {
                code.Tab().Str("self.EndModal(wx.ID_YES)").Eol().Eol();
            }
            else if (event->get_name() == "NoButtonClicked")
            {
                code.Tab().Str("self.EndModal(wx.ID_NO)").Eol().Eol();
            }
            else
            {
                code.Tab().Str("event.Skip()").Eol().Eol();
            }
        }
    }

    if (found_user_handlers && !is_all_events_implemented)
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

    if (!is_all_events_implemented)
    {
        code.Eol(eol_if_needed).Str(python_triple_quote).Eol().Eol();
        m_source->writeLine(code);
    }
}

// This function simply generates unhandled event handlers in a multi-string comment.

void BaseCodeGenerator::GenRubyEventHandlers(EventVector& events)
{
    ASSERT_MSG(events.size(), "GenRubyEventHandlers() shouldn't be called if there are no events");
    if (events.empty())
    {
        return;
    }

    // Multiple events can be bound to the same function, so use a set to make sure we only generate each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_RUBY);
    auto sort_event_handlers = [](NodeEvent* a, NodeEvent* b)
    {
        return (EventHandlerDlg::GetRubyValue(a->get_value()) < EventHandlerDlg::GetRubyValue(b->get_value()));
    };

    // Sort events by function name
    std::sort(events.begin(), events.end(), sort_event_handlers);

    bool inherited_class = m_form_node->hasValue(prop_ruby_inherit_name);
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
        tt_view_vector org_file;
        auto [path, has_base_file] = Project.GetOutputPath(m_form_node, GEN_LANG_RUBY);

        if (has_base_file && path.extension().empty())
        {
            path += ".rb";
        }

        // If the user has defined any event handlers, add them to the code_lines set so we
        // don't generate them again.
        if (has_base_file && org_file.ReadFile(path))
        {
            size_t line_index;
            for (line_index = 0; line_index < org_file.size(); ++line_index)
            {
                if (org_file[line_index].is_sameprefix(python_perl_ruby_end_cmt_line))
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
        code.Eol().Eol();
    }
    else
    {
        code.Str("# Event handler functions\n# Add these below the comment block, or to your inherited class.");
        code.Eol().Eol();
    }

    Code undefined_handlers(m_form_node, GEN_LANG_RUBY);
    for (auto& event: events)
    {
        auto ruby_handler = EventHandlerDlg::GetRubyValue(event->get_value());
        // Ignore lambda's
        if (ruby_handler.empty() || ruby_handler.starts_with("[ruby:lambda]"))
            continue;

        tt_string set_code;
        set_code << "def " << ruby_handler << "(event)";
        if (code_lines.find(set_code) != code_lines.end())
            continue;
        code_lines.emplace(set_code);

        undefined_handlers.Str(set_code).Eol();
        if (event->get_name() == "CloseButtonClicked")
        {
            undefined_handlers.Tab().Str("end_modal(Wx::ID_CLOSE)");
        }
        else if (event->get_name() == "YesButtonClicked")
        {
            undefined_handlers.Tab().Str("end_modal(Wx::ID_YES)");
        }
        else if (event->get_name() == "NoButtonClicked")
        {
            undefined_handlers.Tab().Str("end_modal(Wx::ID_NO)");
        }
        else
        {
            undefined_handlers.Tab().Str("event.skip");
        }
        undefined_handlers.Eol().Unindent();
        undefined_handlers.Str("end").Eol();
    }

    if (undefined_handlers.size())
    {
        m_source->writeLine(code, indent::none);
        m_source->writeLine(ruby_begin_cmt_block, indent::none);
        m_source->writeLine(undefined_handlers);
        m_source->writeLine("end", indent::none);
        m_source->writeLine(ruby_end_cmt_block, indent::none);

        m_header->writeLine("# Event handler functions");
        m_header->writeLine(undefined_handlers);
    }
    m_header->Unindent();
}

// This function simply generates unhandled event handlers in a multi-string comment.

void BaseCodeGenerator::GenRustEventHandlers(EventVector& events)
{
    ASSERT_MSG(events.size(), "GenRustEventHandlers() shouldn't be called if there are no events");
    if (events.empty())
    {
        return;
    }

    // Multiple events can be bound to the same function, so use a set to make sure we only generate each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_RUST);
    auto sort_event_handlers = [](NodeEvent* a, NodeEvent* b)
    {
        return (EventHandlerDlg::GetRustValue(a->get_value()) < EventHandlerDlg::GetRustValue(b->get_value()));
    };

    // Sort events by function name
    std::sort(events.begin(), events.end(), sort_event_handlers);

    bool found_user_handlers = false;
    if (m_panel_type == NOT_PANEL)
    {
        tt_view_vector org_file;
        auto [path, has_base_file] = Project.GetOutputPath(m_form_node, GEN_LANG_PERL);

        if (has_base_file && path.extension().empty())
        {
            path += ".rs";
        }

        // If the user has defined any event handlers, add them to the code_lines set so we
        // don't generate them again.
        if (has_base_file && org_file.ReadFile(path))
        {
            size_t line_index;
            for (line_index = 0; line_index < org_file.size(); ++line_index)
            {
                if (org_file[line_index].is_sameprefix(cpp_rust_end_cmt_line))
                {
                    break;
                }
            }
            for (++line_index; line_index < org_file.size(); ++line_index)
            {
                auto handler = org_file[line_index].view_nonspace();
                if (org_file[line_index].view_nonspace().starts_with("fn "))
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
        for (auto& event: events)
        {
            auto handler = EventHandlerDlg::GetRustValue(event->get_value());
            // Ignore lambda's
            if (handler.starts_with("[rust:lambda]"))
                continue;

            tt_string set_code;
            // If the user doesn't use the `event` parameter, they may use '_' instead to indicate
            // an unused parameter.
            set_code << "fn " << handler << " {";
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            set_code << "fn " << handler << " {";
            if (code_lines.find(set_code) != code_lines.end())
                continue;

            // At least one event wasn't implemented, so stop looking for more
            is_all_events_implemented = false;

            code.Str(
                "// Unimplemented Event handler functions\n// Copy any listed and paste them below the comment block, or "
                "to your inherited class.");
            code.Eol().Str("/*").Eol();
            break;
        }
        if (is_all_events_implemented)
        {
            // If the user has defined all the event handlers, then we don't need to output anything else.
            return;
        }
    }
    else
    {
        // The user hasn't defined their own event handlers in this module
        is_all_events_implemented = false;

        code.Str("// Unimplemented Event handler functions\n// Copy any listed and paste them below the comment block, or "
                 "to your inherited class.");
        code.Eol().Str("/*").Eol();
    }
    m_source->writeLine(code);

    code.clear();
    if (!is_all_events_implemented)
    {
        for (auto& event: events)
        {
            auto handler = EventHandlerDlg::GetRustValue(event->get_value());
            // Ignore lambda's
            if (handler.empty() || handler.starts_with("[rust:lambda]"))
                continue;

            tt_string set_code;
            // If the user doesn't use the `event` parameter, they may use '_' instead to indicate
            // an unused parameter.
            set_code << "fn " << handler << " {";
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            if (code_lines.find(set_code) != code_lines.end())
                continue;
            code_lines.emplace(set_code);

            code.Str(set_code).Eol();
#if defined(_DEBUG)
            auto& dbg_event_name = event->get_name();
            wxUnusedVar(dbg_event_name);
#endif  // _DEBUG
            code.OpenBrace();
            if (event->get_name() == "CloseButtonClicked")
            {
                code.Tab().Str("self.EndModal(wxID_CLOSE);").Eol().Eol();
            }
            else if (event->get_name() == "YesButtonClicked")
            {
                code.Tab().Str("self.EndModal(wxID_YES);").Eol().Eol();
            }
            else if (event->get_name() == "NoButtonClicked")
            {
                code.Tab().Str("self.EndModal(wxID_NO);").Eol().Eol();
            }
            else
            {
                code.Tab().Str("event.Skip();").Eol().Eol();
            }
            code.CloseBrace();
        }
    }

    // Write the unimplemented event handlers to the source file without the
    // comment block to make it easier for the user to cut and paste and/or add
    // them to an inherited class.
    if (found_user_handlers && !is_all_events_implemented)
    {
        m_header->writeLine("# Unimplemented Event handler functions");
    }
    else
    {
        m_header->writeLine("# Event handler functions");
    }
    m_header->writeLine(code);

    if (!is_all_events_implemented)
    {
        code.Eol(eol_if_needed).Str("/*").Eol().Eol();
        m_source->writeLine(code);
        m_source->writeLine("*/");
    }
}
