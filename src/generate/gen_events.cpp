/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ and Python events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include <frozen/map.h>
#include <frozen/set.h>
#include <frozen/string.h>

#include "gen_base.h"

#include "../customprops/eventhandler_dlg.h"  // EventHandlerDlg static functions
#include "base_generator.h"                   // BaseGenerator -- Base widget generator class
#include "code.h"                             // Code -- Helper class for generating code
#include "common_strings.h"                   // Common strings used in code generation
#include "file_codewriter.h"                  // FileCodeWriter -- Classs to write code to disk
#include "lambdas.h"         // Functions for formatting and storage of lamda events
#include "tt_view_vector.h"  // tt_view_vector -- Read/Write line-oriented strings/files

using namespace code;

extern std::string_view cpp_rust_end_cmt_line;  // "// ************* End of generated code"

/////////////////////////////////////////// Default generator event code
//////////////////////////////////////////////

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
    if (auto generator = event->getNode()->get_Generator();
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

    // An opening bracket ('[') indicates a lambda expression
    if (event_code.contains("["))
    {
        if (code.is_cpp())
        {
            handler << event->get_name() << ',';
            handler.ExpandEventLambda(event_code);
            is_lambda = true;
            comma = ",\n\t";
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
    // BUGBUG: [Randalphwa - 08-19-2025] Need to add a comment as to what this is for -- months and
    // months later, even I don't remember what it does...
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
        else if (event_name == "wxEVT_CLOSE_WINDOW")
        {
            if (code.is_python() || code.is_ruby() || code.is_perl())
            {
                // wxPerl, wxPython, and wxRuby use EVT_CLOSE instead of EVT_CLOSE_WINDOW
                event_name = "wxEVT_CLOSE";
            }
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
            if (event_name == "EVT_CLOSE")
            {
                handler.Str(event_name).Str("($self, $self->can('") << event_code << "'));";
            }
            else
            {
                handler.Str(event_name).Str("($self, ").NodeName().Str("->GetId(), $self->can('")
                    << event_code << "'));";
            }
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

            if (event->getNode()->is_Form())
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
            else if (event->get_EventInfo()->get_name() == "wxEVT_SIZE" ||
                     event->get_EventInfo()->get_name() == "wxEVT_GRID_COL_SIZE")
            {
                // wxRuby3 doesn't allow an id for this event
                handler.Str(event_name).Str("(:") << event_code << ')';
            }
            else if (event->getNode()->is_Gen(gen_StaticCheckboxBoxSizer))
            {
                code.Str(event_name)
                        .Str("(")
                        .VarName(event->getNode()->as_string(prop_checkbox_var_name))
                        .Str(".get_id, :")
                    << event_code << ')';
                return;
            }
            else if (event->getNode()->is_Gen(gen_StaticRadioBtnBoxSizer))
            {
                code.Str(event_name)
                        .Str("(")
                        .VarName(event->getNode()->as_string(prop_radiobtn_var_name))
                        .Str(".get_id, :")
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

    if (event->getNode()->is_StaticBoxSizer())
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
    else if (event->getNode()->is_Gen(gen_wxMenuItem) || event->getNode()->is_Gen(gen_tool) ||
             event->getNode()->is_Gen(gen_auitool))
    {
        if (code.is_cpp() || code.is_python())
        {
            code.AddIfPython("self.");
            code << "Bind(" << handler.GetCode() << comma;
            if (event->getNode()->as_string(prop_id) != "wxID_ANY")
            {
                auto id = event->getNode()->get_PropId();
                code.AddIfPython("id=").Add(id).EndFunction();
            }
            else
            {
                code.AddIfPython("id=")
                    .NodeName(event->getNode())
                    .Function("GetId()")
                    .EndFunction();
            }
        }
        else
        {
            code << handler;
        }
    }
    else if (event->getNode()->is_Gen(gen_ribbonTool))
    {
        if (code.is_python())
            code.Add("self.");
        if (!event->getNode()->HasValue(prop_id))
        {
            code.AddComment(
                "**WARNING** -- tool id not specified, event handler may never be called.", true);
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
    else if (event->getNode()->is_Form())
    {
        if (code.is_cpp() || code.is_python())
        {
            code.AddIfPython("self.");
            code << "Bind(" << handler.GetCode();
            if (auto result = prop_sheet_events.find(event->get_name());
                result != prop_sheet_events.end())
            {
                code.Comma() << result->second;
            }
            code.EndFunction();
        }
        else if (code.is_ruby())
        {
            code << handler;
            if (auto result = prop_sheet_events.find(event->get_name());
                result != prop_sheet_events.end())
            {
                code.Comma() << result->second;
            }
        }
        else if (code.is_perl())
        {
            code << handler;
        }
    }
    else
    {
        if (code.is_cpp() || code.is_python())
        {
            code.NodeName(event->getNode()).Function("Bind(") << handler.GetCode();
            code.EndFunction();
        }
        else if (code.is_ruby() || code.is_perl())
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

    auto propName = node->get_PropPtr(prop_class_name);
    if (!propName)
    {
        FAIL_MSG(tt_string("Missing \"name\" property in ") << node->get_DeclName() << " class.");
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
                    return (a->getNode()->as_string(prop_var_name) <
                            b->getNode()->as_string(prop_var_name));
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
        if (auto generator = event->getNode()->get_Generator(); generator)
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
                    if (!code.is_cpp())
                    {
                        m_source->writeLine("# You can only use C++ lambda functions as an event "
                                            "handler C++ code.");
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
            if (auto generator = conditional_event->getNode()->get_Generator(); generator)
            {
                if (generator->GenEvent(code, conditional_event, class_name); code.size())
                {
                    if (!code.GetCode().contains("["))
                    {
                        m_source->writeLine(code);
                    }
                    else  // this is a lambda
                    {
                        if (!code.is_cpp())
                        {
                            m_source->writeLine("# You can only use C++ lambda functions as an "
                                                "event handler C++ code.");
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
