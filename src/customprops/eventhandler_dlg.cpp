/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <unordered_map>

#include "eventhandler_dlg.h"  // auto-generated: wxui/eventhandler_dlg_base.h and wxui/eventhandler_dlg_base.cpp

#include "code.h"             // Code -- Helper class for generating code
#include "lambdas.h"          // Functions for formatting and storage of lamda events
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "node_event.h"       // NodeEventInfo -- NodeEvent and NodeEventInfo classes
#include "project_handler.h"  // ProjectHandler class

// List of events and suggested function names
extern const std::unordered_map<std::string_view, const char*> s_EventNames;

// Defined in base_panel.cpp
extern const char* g_u8_cpp_keywords;
extern const char* g_ruby_keywords;

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

constexpr size_t EVENT_PAGE_CPP = 0;
constexpr size_t EVENT_PAGE_PYTHON = 1;
constexpr size_t EVENT_PAGE_RUBY = 2;

EventHandlerDlg::EventHandlerDlg(wxWindow* parent, NodeEvent* event) : EventHandlerDlgBase(parent), m_event(event)
{
    // Page numbers can be reduced if the language before it was removed
    m_python_page = EVENT_PAGE_PYTHON;
    m_ruby_page = EVENT_PAGE_RUBY;

    m_output_type = Project.getOutputType(OUT_FLAG_IGNORE_DERIVED | OUT_FLAG_IGNORE_XRC);
    m_code_preference = Project.getCodePreference();

    m_is_cpp_enabled = (m_code_preference == GEN_LANG_CPLUSPLUS || m_output_type & OUTPUT_CPLUS);
    m_is_python_enabled = (m_code_preference == GEN_LANG_PYTHON || m_output_type & OUTPUT_PYTHON);
    m_is_ruby_enabled = (m_code_preference == GEN_LANG_RUBY || m_output_type & OUTPUT_RUBY);

    if (!m_is_cpp_enabled)
    {
        m_notebook->RemovePage(EVENT_PAGE_CPP);
        m_python_page--;
        m_ruby_page--;
    }
    if (!m_is_python_enabled)
    {
        m_notebook->RemovePage(m_python_page);
        m_ruby_page--;
    }
    if (!m_is_ruby_enabled)
    {
        m_notebook->RemovePage(m_ruby_page);
    }

    m_value = event->get_value().make_wxString();

    if (m_is_cpp_enabled)
    {
        m_cpp_stc_lambda->SetLexer(wxSTC_LEX_CPP);

        // On Windows, this saves converting the UTF16 characters to ANSI.
        m_cpp_stc_lambda->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);
    }

    auto form = event->getNode()->getForm();
    if (form)
    {
        if (m_is_cpp_enabled)
        {
            std::set<std::string> variables;
            CollectMemberVariables(form, variables);
            tt_string keywords;
            for (auto& iter: variables)
            {
                keywords << iter << ' ';
            }
            // Remove any trailing space, if there are any keywords
            if (keywords.size() && keywords.back() == ' ')
                keywords.pop_back();
            m_cpp_stc_lambda->SetKeyWords(1, keywords);
            m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_WORD2, wxColour("#E91AFF"));
        }

        if (m_is_ruby_enabled)
        {
            // Unfortunately, RUBY_LEXER only supports one set of keywords so we either combine
            // the regular keywords with the wxWidgets keywords, or we only use the wxWidgets
            // keywords.

            tt_string wxRuby_keywords(g_ruby_keywords);
            wxRuby_keywords << (" ToolBar MenuBar BitmapBundle Bitmap Window Wx");

            for (auto iter: NodeCreation.getNodeDeclarationArray())
            {
                if (!iter)
                {
                    // This will happen if there is an enumerated value but no generator for it
                    continue;
                }

                if (!iter->declName().starts_with("wx"))
                    continue;
                else if (iter->declName().is_sameas("wxContextMenuEvent") || iter->declName() == "wxTreeCtrlBase" ||
                         iter->declName().starts_with("wxRuby") || iter->declName().starts_with("wxPython"))
                    continue;
                wxRuby_keywords << ' ' << iter->declName().subview(2);
            }
            m_ruby_stc_lambda->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxRuby_keywords.c_str());
        }
    }
    if (m_is_cpp_enabled)
    {
        m_cpp_stc_lambda->StyleSetBold(wxSTC_C_WORD, true);
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_STRING, wxColour(0, 128, 0));
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_STRINGEOL, wxColour(0, 128, 0));
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 128, 0));
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 128, 0));
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour(0, 128, 0));
        m_cpp_stc_lambda->StyleSetForeground(wxSTC_C_NUMBER, *wxRED);
    }

    if (m_is_ruby_enabled)
    {
        m_ruby_stc_lambda->StyleSetForeground(wxSTC_RB_WORD, "#FF00FF");
        m_ruby_stc_lambda->StyleSetForeground(wxSTC_RB_STRING, wxColour(0, 128, 0));
        m_ruby_stc_lambda->StyleSetForeground(wxSTC_RB_COMMENTLINE, wxColour(0, 128, 0));
        m_ruby_stc_lambda->StyleSetForeground(wxSTC_RB_NUMBER, *wxRED);
    }

    if (m_code_preference == GEN_LANG_PYTHON)
        m_notebook->SetSelection(m_python_page);
    else if (m_code_preference == GEN_LANG_RUBY)
        m_notebook->SetSelection(m_ruby_page);
}

void EventHandlerDlg::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_static_bind_text->SetLabel(wxEmptyString);

    if (m_value.empty())
    {
        if (auto default_name = s_EventNames.find(m_event->get_name()); default_name != s_EventNames.end())
        {
            m_value = default_name->second;
        }
        else
        {
            m_value = "OnEvent";
        }

        if (m_is_cpp_enabled)
        {
            m_cpp_text_function->SetValue(m_value);
            m_cpp_radio_use_function->SetValue(true);
            m_cpp_lambda_box->GetStaticBox()->Enable(false);
        }
        if (m_is_python_enabled)
        {
            m_py_text_function->SetValue(m_value);
            m_py_radio_use_function->SetValue(true);
            m_py_lambda_box->GetStaticBox()->Enable(false);
        }
        if (m_is_ruby_enabled)
        {
            m_ruby_text_function->SetValue(m_value);
            m_ruby_radio_use_function->SetValue(true);
            m_ruby_lambda_box->GetStaticBox()->Enable(false);
        }
    }
    else
    {
        tt_string value;

        if (m_is_cpp_enabled)
        {
            value = GetCppValue(m_value.utf8_string());
            if (value.size())
            {
                if (value.contains("["))
                {
                    m_cpp_radio_use_function->SetValue(false);
                    m_cpp_function_box->GetStaticBox()->Enable(false);

                    m_cpp_radio_use_lambda->SetValue(true);
                    m_cpp_lambda_box->GetStaticBox()->Enable(true);

                    if (value.contains("this"))
                        m_check_capture_this->SetValue(true);
                    if (value.contains("& event)"))
                        m_check_include_event->SetValue(true);

                    if (auto pos = value.find('{'); tt::is_found(pos))
                    {
                        tt_string lamda = value.substr(pos + 1);
                        if (lamda.back() == '}')
                            lamda.pop_back();
                        ExpandLambda(lamda);

                        m_cpp_stc_lambda->AddTextRaw(lamda.c_str());
                    }
                    m_is_cpp_lambda = true;
                }
                else
                {
                    m_cpp_radio_use_lambda->SetValue(false);
                    m_cpp_lambda_box->GetStaticBox()->Enable(false);

                    m_cpp_function_box->GetStaticBox()->Enable(true);
                    m_cpp_radio_use_function->SetValue(true);
                    m_cpp_text_function->SetValue(value.make_wxString());
                }
            }
        }

        if (m_is_python_enabled)
        {
            value = GetPythonValue(m_value.utf8_string());
            if (value.size())
            {
                if (auto pos_lambda = value.find('['); pos_lambda != tt::npos)
                {
                    m_py_radio_use_function->SetValue(false);
                    m_py_radio_use_lambda->SetValue(true);
                    m_py_function_box->GetStaticBox()->Enable(false);
                    m_py_lambda_box->GetStaticBox()->Enable(true);

                    // remove leading and trailing brackets
                    value.erase(pos_lambda, sizeof("[python:lambda]") - 1);

                    m_py_text_lambda->SetValue(value.make_wxString());
                    m_is_python_lambda = true;
                }
                else
                {
                    // GetPythonValue() is a static function, so if the C++ string is a lambda, it
                    // can only return "OnEvent" as the function name.  We check for that here and
                    // replace it if possible.
                    if (value == "OnEvent")
                    {
                        if (auto default_name = s_EventNames.find(m_event->get_name()); default_name != s_EventNames.end())
                        {
                            value = default_name->second;
                        }
                    }

                    m_py_text_function->SetValue(value.make_wxString());
                    m_py_radio_use_function->SetValue(true);
                    m_py_radio_use_lambda->SetValue(false);
                }
            }
        }

        if (m_is_ruby_enabled)
        {
            value = GetRubyValue(m_value.utf8_string());
            if (value.size())
            {
                if (auto pos_lambda = value.find('['); pos_lambda != tt::npos)
                {
                    m_ruby_radio_use_function->SetValue(false);
                    m_ruby_radio_use_lambda->SetValue(true);
                    m_ruby_function_box->GetStaticBox()->Enable(false);
                    m_ruby_lambda_box->GetStaticBox()->Enable(true);

                    // remove leading and trailing brackets
                    value.erase(pos_lambda, sizeof("[python:lambda]") - 1);

                    // m_ruby_text_lambda->SetValue(value.make_wxString());
                    m_is_ruby_lambda = true;
                }
                else
                {
                    m_ruby_text_function->SetValue(value.make_wxString());
                    m_ruby_radio_use_function->SetValue(true);
                    m_ruby_radio_use_lambda->SetValue(false);
                }
            }
        }
    }

    FormatBindText();
}

void EventHandlerDlg::OnUseCppFunction(wxCommandEvent& WXUNUSED(event))
{
    if (m_cpp_radio_use_function->GetValue())
    {
        m_cpp_radio_use_lambda->SetValue(false);
        m_cpp_lambda_box->GetStaticBox()->Enable(false);
        m_cpp_function_box->GetStaticBox()->Enable(true);

        auto value = GetCppValue(m_value.utf8_string());

        if (value.empty() || value.contains("["))
        {
            if (auto default_name = s_EventNames.find(m_event->get_name()); default_name != s_EventNames.end())
            {
                value = default_name->second;
            }
            else
            {
                value = "OnEvent";
            }
        }
        m_cpp_text_function->SetValue(value);
    }

    FormatBindText();
}

void EventHandlerDlg::OnUsePythonFunction(wxCommandEvent& WXUNUSED(event))
{
    if (m_py_radio_use_function->GetValue())
    {
        m_py_radio_use_lambda->SetValue(false);
        m_py_lambda_box->GetStaticBox()->Enable(false);
        m_py_function_box->GetStaticBox()->Enable(true);

        auto value = GetPythonValue(m_value.utf8_string());

        if (value.empty() || value.contains("["))
        {
            if (auto default_name = s_EventNames.find(m_event->get_name()); default_name != s_EventNames.end())
            {
                value = default_name->second;
            }
            else
            {
                value = "OnEvent";
            }
        }
        m_py_text_function->SetValue(value);
    }
    FormatBindText();
}

void EventHandlerDlg::OnUseRubyFunction(wxCommandEvent& WXUNUSED(event))
{
    if (m_ruby_radio_use_function->GetValue())
    {
        m_ruby_radio_use_lambda->SetValue(false);
        m_ruby_lambda_box->GetStaticBox()->Enable(false);
        m_ruby_function_box->GetStaticBox()->Enable(true);

        auto value = GetPythonValue(m_value.utf8_string());

        if (value.empty() || value.contains("["))
        {
            if (auto default_name = s_EventNames.find(m_event->get_name()); default_name != s_EventNames.end())
            {
                value = default_name->second;
            }
            else
            {
                value = "OnEvent";
            }
        }
        m_ruby_text_function->SetValue(value);
    }
    FormatBindText();
}

void EventHandlerDlg::OnUseCppLambda(wxCommandEvent& WXUNUSED(event))
{
    if (m_cpp_radio_use_lambda->GetValue())
    {
        m_cpp_radio_use_function->SetValue(false);
        m_cpp_function_box->GetStaticBox()->Enable(false);
        m_cpp_lambda_box->GetStaticBox()->Enable(true);
        FormatBindText();
    }
}

void EventHandlerDlg::OnUsePythonLambda(wxCommandEvent& WXUNUSED(event))
{
    if (m_py_radio_use_lambda->GetValue())
    {
        m_py_radio_use_function->SetValue(false);
        m_py_function_box->GetStaticBox()->Enable(false);
        m_py_lambda_box->GetStaticBox()->Enable(true);
        FormatBindText();
    }
}

void EventHandlerDlg::OnUseRubyLambda(wxCommandEvent& WXUNUSED(event))
{
    if (m_cpp_radio_use_lambda->GetValue())
    {
        m_cpp_radio_use_function->SetValue(false);
        m_cpp_function_box->GetStaticBox()->Enable(false);
        m_cpp_lambda_box->GetStaticBox()->Enable(true);
        FormatBindText();
    }
}

void EventHandlerDlg::OnPageChanged(wxBookCtrlEvent& event)
{
    // There is some weirdness with page changes using wxWidgets 3.2 -- the first time the page
    // changes, the setting for the function/radio button is not correct. After it has changed
    // once and been corrected, then further changes work fine. I have not been able to figure
    // out why this is happening, but this code works around it.

    if (m_is_python_enabled && event.GetSelection() == m_python_page)
    {
        if (m_is_python_lambda)
        {
            m_py_radio_use_lambda->SetValue(true);
            m_py_radio_use_function->SetValue(false);
            m_py_function_box->GetStaticBox()->Enable(false);
            m_py_lambda_box->GetStaticBox()->Enable(true);

            m_is_python_lambda = false;
        }
        m_is_cpp_lambda = m_cpp_radio_use_lambda->GetValue();
        m_is_ruby_lambda = m_ruby_radio_use_lambda->GetValue();
    }
    else if (m_is_cpp_enabled && event.GetSelection() == EVENT_PAGE_CPP)
    {
        if (m_is_cpp_lambda)
        {
            m_cpp_radio_use_lambda->SetValue(true);
            m_cpp_radio_use_function->SetValue(false);
            m_cpp_function_box->GetStaticBox()->Enable(false);
            m_cpp_lambda_box->GetStaticBox()->Enable(true);

            m_is_cpp_lambda = false;
        }
        m_is_python_lambda = m_py_radio_use_lambda->GetValue();
        m_is_ruby_lambda = m_ruby_radio_use_lambda->GetValue();
    }
    else if (m_is_ruby_enabled && event.GetSelection() == m_ruby_page)
    {
        if (m_is_ruby_lambda)
        {
            m_ruby_radio_use_lambda->SetValue(true);
            m_ruby_radio_use_function->SetValue(false);
            m_ruby_function_box->GetStaticBox()->Enable(false);
            m_ruby_lambda_box->GetStaticBox()->Enable(true);

            m_is_ruby_lambda = false;
        }
        m_is_cpp_lambda = m_cpp_radio_use_lambda->GetValue();
        m_is_python_lambda = m_py_radio_use_lambda->GetValue();
    }

    FormatBindText();
}

void EventHandlerDlg::OnChange(wxCommandEvent& WXUNUSED(event))
{
    FormatBindText();
}

void EventHandlerDlg::FormatBindText()
{
    auto page = m_notebook->GetSelection();
    int language;
    if (m_is_cpp_enabled && page == EVENT_PAGE_CPP)
        language = GEN_LANG_CPLUSPLUS;
    else if (m_is_python_enabled && page == m_python_page)
        language = GEN_LANG_PYTHON;
    else if (m_is_ruby_enabled && page == m_ruby_page)
        language = GEN_LANG_RUBY;
    else
        return;

    Code handler(m_event->getNode(), language);

    if (language == GEN_LANG_CPLUSPLUS)
    {
        if (m_cpp_radio_use_function->GetValue())
        {
            auto value = m_cpp_text_function->GetValue().utf8_string();
            handler << m_event->get_name() << ", &" << m_event->getNode()->getFormName() << "::" << value += ", this";
        }
        else
        {
            handler << m_event->get_name() += ", ";
            if (m_check_capture_this->GetValue())
                handler += "[this](";
            else
                handler += "[](";

            // Note that we have to double the '&' character since it is being sent to a static
            // text control that will think it is an accelerator.
            handler << m_event->getEventInfo()->get_event_class() << "&&";
            if (m_check_include_event->GetValue())
                handler += " event";

            // We don't display the code's body in the static text control since it's visible
            // in the control below and it's almost certainly going to be too long to fit in
            // the static text control.
            handler += ") { body }";
        }
    }
    else if (language == GEN_LANG_PYTHON)
    {
        if (m_py_radio_use_function->GetValue())
        {
            auto value = m_py_text_function->GetValue().utf8_string();
            handler.Add(m_event->get_name()) << ", self." += value;
        }
        else
        {
            handler.Add(m_event->get_name()) += ", lambda event: ";

            handler += "body";
        }
    }
    else if (language == GEN_LANG_RUBY)
    {
        // BUGBUG: [Randalphwa - 07-24-2023] This is a placeholder until we can get the Ruby
        // code
        if (m_ruby_radio_use_function->GetValue())
        {
            auto value = m_ruby_text_function->GetValue().utf8_string();
            handler.Add(m_event->get_name()) << ", self." += value;
        }
        else
        {
            handler.Add(m_event->get_name()) += ", lambda event: ";

            handler += "body";
        }
    }

    Code code(m_event->getNode(), language);

    if (m_event->getNode()->isForm())
    {
        code.Add("Bind(").Add(handler).EndFunction();
    }
    else if (m_event->getNode()->isGen(gen_wxMenuItem) || m_event->getNode()->isGen(gen_tool))
    {
        code << "Bind(" << handler << ", ";
        if (m_event->getNode()->as_string(prop_id) != "wxID_ANY")
            code.as_string(prop_id).EndFunction();
        else
            code.Add(m_event->getNode()->getNodeName()).Function("GetId()").EndFunction();
    }
    else if (m_event->getNode()->isGen(gen_ribbonTool))
    {
        if (m_event->getNode()->as_string(prop_id).empty())
        {
            code.Add("Bind(").Add(handler).Comma().Add("wxID_ANY").EndFunction();
        }
        else
        {
            code.Add("Bind(").Add(handler).Comma().as_string(prop_id).EndFunction();
        }
    }
    else
    {
        code.Add(m_event->getNode()->getNodeName()).Function("Bind(").Add(handler).EndFunction();
    }

    m_static_bind_text->SetLabel(code.make_wxString());
}

void EventHandlerDlg::CollectMemberVariables(Node* node, std::set<std::string>& variables)
{
    if (node->hasValue(prop_class_access) && node->as_string(prop_class_access) != "none")
    {
        if (node->hasValue(prop_var_name))
        {
            variables.insert(node->as_string(prop_var_name));
        }
    }

    if (node->hasValue(prop_validator_variable))
    {
        variables.insert(node->as_string(prop_validator_variable));
    }

    if (node->hasValue(prop_checkbox_var_name))
    {
        variables.insert(node->as_string(prop_checkbox_var_name));
    }
    else if (node->hasValue(prop_radiobtn_var_name))
    {
        variables.insert(node->as_string(prop_radiobtn_var_name));
    }

    for (const auto& child: node->getChildNodePtrs())
    {
        CollectMemberVariables(child.get(), variables);
    }
}

void EventHandlerDlg::OnOK(wxCommandEvent& event)
{
    Update_m_value();
    event.Skip();
}

// We could just call m_cpp_stc_lambda->GetTextRaw() however this method minimizes both the
// amount of memory copying done as well as the amount of memory moving.

const int SCI_GETTEXT_MSG = 2182;

// If more than one language is enabled, then the additional languages are added with a
// [lang:...] prefix. The order if fixed: C++, Python, Ruby. If only one language is enabled,
// then the prefix is not used.

void EventHandlerDlg::Update_m_value()
{
    tt_string cpp_value;
    tt_string py_value;
    tt_string ruby_value;
    m_value.clear();

    if (m_is_cpp_enabled)
    {
        if (m_cpp_radio_use_function->GetValue())
        {
            cpp_value = m_cpp_text_function->GetValue();
        }
        else
        {
            tt_string handler;

            if (m_check_capture_this->GetValue())
                handler << "[this](";
            else
                handler << "[](";
            handler << m_event->getEventInfo()->get_event_class() << "&";
            if (m_check_include_event->GetValue())
                handler << " event";

            // We use \r\n because it allows us to convert them in place to @@
            m_cpp_stc_lambda->ConvertEOLs(wxSTC_EOL_CRLF);

            auto len = m_cpp_stc_lambda->GetTextLength() + 1;
            auto buf = std::make_unique<char[]>(len);
            m_cpp_stc_lambda->SendMsg(SCI_GETTEXT_MSG, len, (wxIntPtr) buf.get());
            handler << ")@@{@@" << std::string_view(buf.get(), len - 1);
            handler.Replace("\r\n", "@@", tt::REPLACE::all);
            handler.RightTrim();
            handler << "@@}";
            cpp_value = handler;
        }

        // If a single language, get the value and return
        if (!m_is_python_enabled && !m_is_ruby_enabled)
        {
            m_value = cpp_value.make_wxString();
            return;
        }
    }

    if (m_is_python_enabled)
    {
        if (m_py_radio_use_function->GetValue())
        {
            if (!m_is_cpp_enabled)
                py_value = m_py_text_function->GetValue().utf8_string();
            else
                py_value << "[python:" << m_py_text_function->GetValue().utf8_string() << "]";
        }
        else
        {
            py_value << "[python:lambda]" << m_py_text_lambda->GetValue().utf8_string();
        }

        if (!m_is_cpp_enabled && !m_is_ruby_enabled)
        {
            m_value = py_value.make_wxString();
            return;
        }
    }

    if (m_is_ruby_enabled)
    {
        if (m_ruby_radio_use_function->GetValue())
        {
            if (!m_is_cpp_enabled && !m_is_python_enabled)
                ruby_value = m_ruby_text_function->GetValue().utf8_string();
            else
                ruby_value << "[ruby:" << m_ruby_text_function->GetValue().utf8_string() << "]";
        }
        else
        {
            tt_string handler;

            // We use \r\n because it allows us to convert them in place to @@
            m_ruby_stc_lambda->ConvertEOLs(wxSTC_EOL_CRLF);

            auto len = m_ruby_stc_lambda->GetTextLength() + 1;
            auto buf = std::make_unique<char[]>(len);
            m_ruby_stc_lambda->SendMsg(SCI_GETTEXT_MSG, len, (wxIntPtr) buf.get());
            handler << ")@@{@@" << std::string_view(buf.get(), len - 1);
            handler.Replace("\r\n", "@@", tt::REPLACE::all);
            handler.RightTrim();
            handler << "@@}";
            ruby_value = "[ruby:lambda]" + handler;
        }

        if (!m_is_cpp_enabled && !m_is_python_enabled)
        {
            m_value = ruby_value.make_wxString();
            return;
        }
    }

    // If we get here, then more than one language has been specified.

    if (m_is_cpp_enabled)
    {
        m_value = cpp_value.make_wxString();
    }
    if (m_is_python_enabled)
    {
        m_value << py_value.make_wxString();
    }
    if (m_is_ruby_enabled)
    {
        m_value << ruby_value.make_wxString();
    }
}

// For a single language, there is no language prefix unless it's a lambda. For multiple
// languages, the prefix specifies the language ([python:...] and/or [ruby:...]). Since C++ is
// the default, it is not specified.

// This is a static function

tt_string EventHandlerDlg::GetCppValue(tt_string_view value)
{
    if (auto pos_python = value.find("[python:"); pos_python != tt::npos)
    {
        value.remove_suffix(value.size() - pos_python);
    }
    else if (auto pos_ruby = value.find("[ruby:"); pos_ruby != tt::npos)
    {
        value.remove_suffix(value.size() - pos_ruby);
    }

    tt_string result(value);
    return result;
}

// This is a static function

tt_string EventHandlerDlg::GetPythonValue(tt_string_view value)
{
    tt_string result;
    auto pos_python = value.find("[python:");
    if (pos_python == tt::npos)
    {
        if (value.front() == '[')
        {
            // Unfortunately, this is a static function, so we have no access to m_event.
            result = "OnEvent";
        }
        else
        {
            result = value;
        }
        return result;
    }
    else
    {
        value.remove_prefix(pos_python);
    }

    if (!value.starts_with("[python:lambda]"))
    {
        // This is just a function name, so remove the "[python:" and the trailing ']'
        value.remove_prefix(sizeof("[python:") - 1);
        if (auto end = value.find(']'); end != tt::npos)
        {
            value.remove_suffix(value.size() - end);
        }
    }

    result << value;
    return result;
}

// This is a static function

tt_string EventHandlerDlg::GetRubyValue(tt_string_view value)
{
    tt_string result;
    auto pos_python = value.find("[ruby:");
    if (pos_python == tt::npos)
    {
        if (value.front() == '[')
        {
            // Unfortunately, this is a static function, so we have no access to m_event.
            result = "OnEvent";
        }
        else
        {
            result = value;
        }
        return result;
    }
    else
    {
        value.remove_prefix(pos_python);
    }

    if (!value.starts_with("[ruby:lambda]"))
    {
        value.remove_prefix(sizeof("[ruby:") - 1);
        if (auto end = value.find(']'); end != tt::npos)
        {
            value.remove_suffix(value.size() - end);
        }
    }

    result << value;
    return result;
}

const std::unordered_map<std::string_view, const char*> s_EventNames = {

    { "wxEVT_ACTIVATE", "OnActivate" },
    { "wxEVT_AUITOOLBAR_BEGIN_DRAG", "OnAuiToolBarBeginDrag" },
    { "wxEVT_AUITOOLBAR_MIDDLE_CLICK", "OnAuiToolBarMiddleClick" },
    { "wxEVT_AUITOOLBAR_OVERFLOW_CLICK", "OnAuiToolBarOverflowClick" },
    { "wxEVT_AUITOOLBAR_RIGHT_CLICK", "OnAuiToolBarRightClick" },
    { "wxEVT_AUITOOLBAR_TOOL_DROPDOWN", "OnAuiToolBarToolDropDown" },
    { "wxEVT_AUI_PANE_ACTIVATED", "OnAuiPaneActivated" },
    { "wxEVT_AUI_PANE_BUTTON", "OnAuiPaneButton" },
    { "wxEVT_AUI_PANE_CLOSE", "OnAuiPaneClose" },
    { "wxEVT_AUI_PANE_MAXIMIZE", "OnAuiPaneMaximize" },
    { "wxEVT_AUI_PANE_RESTORE", "OnAuiPaneRestore" },
    { "wxEVT_AUI_RENDER", "OnAuiRender" },
    { "wxEVT_AUX1_DCLICK", "OnAux1DClick" },
    { "wxEVT_AUX1_DOWN", "OnAux1Down" },
    { "wxEVT_AUX1_UP", "OnAux1Up" },
    { "wxEVT_AUX2_DCLICK", "OnAux2DClick" },
    { "wxEVT_AUX2_DOWN", "OnAux2Down" },
    { "wxEVT_AUX2_UP", "OnAux2Up" },
    { "wxEVT_BUTTON", "OnButton" },
    { "wxEVT_CALENDAR_DAY_CHANGED", "OnCalendarDay" },
    { "wxEVT_CALENDAR_DOUBLECLICKED", "OnCalendar" },
    { "wxEVT_CALENDAR_MONTH_CHANGED", "OnCalendarMonth" },
    { "wxEVT_CALENDAR_PAGE_CHANGED", "OnCalendarPageChanged" },
    { "wxEVT_CALENDAR_SEL_CHANGED", "OnCalendarSelChanged" },
    { "wxEVT_CALENDAR_WEEKDAY_CLICKED", "OnCalendarWeekDayClicked" },
    { "wxEVT_CALENDAR_WEEK_CLICKED", "OnCalendarWeekClicked" },
    { "wxEVT_CALENDAR_YEAR_CHANGED", "OnCalendarYear" },
    { "wxEVT_CHAR", "OnChar" },
    { "wxEVT_CHAR_HOOK", "OnCharHook" },
    { "wxEVT_CHECKBOX", "OnCheckBox" },
    { "wxEVT_CHECKLISTBOX", "OnCheckListBoxToggled" },
    { "wxEVT_CHOICE", "OnChoice" },
    { "wxEVT_CLOSE_WINDOW", "OnClose" },
    { "wxEVT_COLOURPICKER_CHANGED", "OnColourChanged" },
    { "wxEVT_COMBOBOX", "OnCombobox" },
    { "wxEVT_COMBOBOX_CLOSEUP", "OnComboboxCloseup" },
    { "wxEVT_COMBOBOX_DROPDOWN", "OnComboboxDropdown" },
    { "wxEVT_CONTEXT_MENU", "OnContextMenu" },
    { "wxEVT_DATAVIEW_COLUMN_HEADER_CLICK", "OnColumnHeaderClick" },
    { "wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK", "OnColumnHeaderRightClick" },
    { "wxEVT_DATAVIEW_COLUMN_REORDERED", "OnColumnReordered" },
    { "wxEVT_DATAVIEW_COLUMN_SORTED", "OnColumnSorted" },
    { "wxEVT_DATAVIEW_ITEM_ACTIVATED", "OnItemActivated" },
    { "wxEVT_DATAVIEW_ITEM_BEGIN_DRAG", "OnItemBeginDrag" },
    { "wxEVT_DATAVIEW_ITEM_COLLAPSED", "OnItemCollapsed" },
    { "wxEVT_DATAVIEW_ITEM_COLLAPSING", "OnItemCollapsing" },
    { "wxEVT_DATAVIEW_ITEM_CONTEXT_MENU", "OnItemContextMenu" },
    { "wxEVT_DATAVIEW_ITEM_DROP", "OnItemDrop" },
    { "wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE", "OnItemDropPossible" },
    { "wxEVT_DATAVIEW_ITEM_EDITING_DONE", "OnItemEditingDone" },
    { "wxEVT_DATAVIEW_ITEM_EDITING_STARTED", "OnItemEditingStarted" },
    { "wxEVT_DATAVIEW_ITEM_EXPANDED", "OnItemExpanded" },
    { "wxEVT_DATAVIEW_ITEM_EXPANDING", "OnItemExpanding" },
    { "wxEVT_DATAVIEW_ITEM_START_EDITING", "OnItemStartEditing" },
    { "wxEVT_DATAVIEW_ITEM_VALUE_CHANGED", "OnItemValueChanged" },
    { "wxEVT_DATAVIEW_SELECTION_CHANGED", "OnDataViewCtrlSelectionChanged" },
    { "wxEVT_DATE_CHANGED", "OnDateChanged" },
    { "wxEVT_DESTROY", "OnDestroyed" },
    { "wxEVT_DIRCTRL_FILEACTIVATED", "OnDirctrlFileActivated" },
    { "wxEVT_DIRCTRL_SELECTIONCHANGED", "OnDirctrlSelectionChanged" },
    { "wxEVT_DIRPICKER_CHANGED", "OnDirChanged" },
    { "wxEVT_ENTER_WINDOW", "OnEnterWindow" },
    { "wxEVT_ERASE_BACKGROUND", "OnEraseBackground" },
    { "wxEVT_FILEPICKER_CHANGED", "OnFileChanged" },
    { "wxEVT_FONTPICKER_CHANGED", "OnFontChanged" },
    { "wxEVT_GRID_CELL_CHANGED", "OnGridCellChange" },
    { "wxEVT_GRID_CELL_CHANGED", "OnGridCellChange" },
    { "wxEVT_GRID_CELL_LEFT_CLICK", "OnGridCellLeftClick" },
    { "wxEVT_GRID_CELL_LEFT_DCLICK", "OnGridCellLeftDClick" },
    { "wxEVT_GRID_CELL_RIGHT_CLICK", "OnGridCellRightClick" },
    { "wxEVT_GRID_CELL_RIGHT_DCLICK", "OnGridCellRightDClick" },
    { "wxEVT_GRID_COL_SIZE", "OnGridColSize" },
    { "wxEVT_GRID_EDITOR_CREATED", "OnGridEditorCreated" },
    { "wxEVT_GRID_EDITOR_HIDDEN", "OnGridEditorHidden" },
    { "wxEVT_GRID_EDITOR_SHOWN", "OnGridEditorShown" },
    { "wxEVT_GRID_LABEL_LEFT_CLICK", "OnGridLabelLeftClick" },
    { "wxEVT_GRID_LABEL_LEFT_DCLICK", "OnGridLabelLeftDClick" },
    { "wxEVT_GRID_LABEL_RIGHT_CLICK", "OnGridLabelRightClick" },
    { "wxEVT_GRID_LABEL_RIGHT_DCLICK", "OnGridLabelRightDClick" },
    { "wxEVT_GRID_RANGE_SELECT", "OnGridRangeSelect" },
    { "wxEVT_GRID_ROW_SIZE", "OnGridRowSize" },
    { "wxEVT_GRID_SELECT_CELL", "OnGridSelectCell" },
    { "wxEVT_HELP", "OnHelp" },
    { "wxEVT_HTML_CELL_CLICKED", "OnHtmlCellClicked" },
    { "wxEVT_HTML_CELL_HOVER", "OnHtmlCellHover" },
    { "wxEVT_HTML_LINK_CLICKED", "OnHtmlLinkClicked" },
    { "wxEVT_HYPERLINK", "OnHyperlink" },
    { "wxEVT_ICONIZE", "OnIconize" },
    { "wxEVT_IDLE", "OnIdle" },
    { "wxEVT_INIT_DIALOG", "OnInit" },
    { "wxEVT_KEY_DOWN", "OnKeyDown" },
    { "wxEVT_KEY_UP", "OnKeyUp" },
    { "wxEVT_KILL_FOCUS", "OnKillFocus" },
    { "wxEVT_LEAVE_WINDOW", "OnLeaveWindow" },
    { "wxEVT_LEFT_DCLICK", "OnLeftDClick" },
    { "wxEVT_LEFT_DOWN", "OnLeftDown" },
    { "wxEVT_LEFT_UP", "OnLeftUp" },
    { "wxEVT_LISTBOX", "OnCheckListBox" },
    { "wxEVT_LISTBOX", "OnListBox" },
    { "wxEVT_LISTBOX_DCLICK", "OnListBoxDClick" },
    { "wxEVT_LIST_BEGIN_DRAG", "OnListBeginDrag" },
    { "wxEVT_LIST_BEGIN_LABEL_EDIT", "OnListBeginLabelEdit" },
    { "wxEVT_LIST_BEGIN_RDRAG", "OnListBeginRDrag" },
    { "wxEVT_LIST_CACHE_HINT", "OnListCacheHint" },
    { "wxEVT_LIST_COL_BEGIN_DRAG", "OnListColBeginDrag" },
    { "wxEVT_LIST_COL_CLICK", "OnListColClick" },
    { "wxEVT_LIST_COL_DRAGGING", "OnListColDragging" },
    { "wxEVT_LIST_COL_END_DRAG", "OnListColEndDrag" },
    { "wxEVT_LIST_COL_RIGHT_CLICK", "OnListColRightClick" },
    { "wxEVT_LIST_DELETE_ALL_ITEMS", "OnListDeleteAllItems" },
    { "wxEVT_LIST_DELETE_ITEM", "OnListDeleteItem" },
    { "wxEVT_LIST_END_LABEL_EDIT", "OnListEndLabelEdit" },
    { "wxEVT_LIST_INSERT_ITEM", "OnListInsertItem" },
    { "wxEVT_LIST_ITEM_ACTIVATED", "OnListItemActivated" },
    { "wxEVT_LIST_ITEM_DESELECTED", "OnListItemDeselected" },
    { "wxEVT_LIST_ITEM_FOCUSED", "OnListItemFocused" },
    { "wxEVT_LIST_ITEM_MIDDLE_CLICK", "OnListItemMiddleClick" },
    { "wxEVT_LIST_ITEM_RIGHT_CLICK", "OnListItemRightClick" },
    { "wxEVT_LIST_ITEM_SELECTED", "OnListItemSelected" },
    { "wxEVT_LIST_KEY_DOWN", "OnListKeyDown" },
    { "wxEVT_MAXIMIZE", "OnMaximize" },
    { "wxEVT_MENU", "OnMenuSelection" },
    { "wxEVT_MIDDLE_DCLICK", "OnMiddleDClick" },
    { "wxEVT_MIDDLE_DOWN", "OnMiddleDown" },
    { "wxEVT_MIDDLE_UP", "OnMiddleUp" },
    { "wxEVT_MOTION", "OnMotion" },
    { "wxEVT_MOUSEWHEEL", "OnMouseWheel" },
    { "wxEVT_MOVE", "OnMove" },
    { "wxEVT_MOVE_END", "OnMoveEnd" },
    { "wxEVT_MOVE_START", "OnMoveStart" },
    { "wxEVT_MOVING", "OnMoving" },
    { "wxEVT_PAINT", "OnPaint" },
    { "wxEVT_PG_CHANGED", "OnPropertyGridChanged" },
    { "wxEVT_PG_CHANGING", "OnPropertyGridChanging" },
    { "wxEVT_RADIOBOX", "OnRadioBox" },
    { "wxEVT_RADIOBUTTON", "OnRadioButton" },
    { "wxEVT_RIBBONBAR_HELP_CLICKED", "OnRibbonBarHelpClick" },
    { "wxEVT_RIBBONBAR_PAGE_CHANGED", "OnRibbonBarPageChanged" },
    { "wxEVT_RIBBONBAR_PAGE_CHANGING", "OnRibbonBarPageChanging" },
    { "wxEVT_RIBBONBAR_TAB_LEFT_DCLICK", "OnRibbonBarTabLeftDClick" },
    { "wxEVT_RIBBONBAR_TAB_MIDDLE_DOWN", "OnRibbonBarTabMiddleDown" },
    { "wxEVT_RIBBONBAR_TAB_MIDDLE_UP", "OnRibbonBarTabMiddleUp" },
    { "wxEVT_RIBBONBAR_TAB_RIGHT_DOWN", "OnRibbonBarTabRightDown" },
    { "wxEVT_RIBBONBAR_TAB_RIGHT_UP", "OnRibbonBarTabRightUp" },
    { "wxEVT_RIBBONBAR_TOGGLED", "OnRibbonBarToggled" },
    { "wxEVT_RIBBONBUTTON_CLICKED", "OnRibbonButtonClicked" },
    { "wxEVT_RIBBONBUTTON_DROPDOWN_CLICKED", "OnRibbonButtonDropdownClicked" },
    { "wxEVT_RIBBONGALLERY_CLICKED", "OnRibbonGalleryClicked" },
    { "wxEVT_RIBBONGALLERY_HOVER_CHANGED", "OnRibbonGalleryHoverChanged" },
    { "wxEVT_RIBBONGALLERY_SELECTED", "OnRibbonGallerySelected" },
    { "wxEVT_RIBBONPANEL_EXTBUTTON_ACTIVATED", "OnRibbonPanelExtbuttonActivated" },
    { "wxEVT_RIBBONTOOL_DROPDOWN_CLICKED", "OnRibbonToolClicked" },
    { "wxEVT_RIBBONTOOL_DROPDOWN_CLICKED", "OnRibbonToolDropdownClicked" },

    { "wxEVT_RICHTEXT_LEFT_CLICK", "OnRichTextLeftClick" },
    { "wxEVT_RICHTEXT_RIGHT_CLICK", "OnRichTextRightClick" },
    { "wxEVT_RICHTEXT_MIDDLE_CLICK", "OnRichTextMiddleClick" },
    { "wxEVT_RICHTEXT_LEFT_DCLICK", "OnRichTextDblClick" },

    { "wxEVT_RICHTEXT_BUFFER_RESET", "OnRichTextBufferReset" },
    { "wxEVT_RICHTEXT_CHARACTER", "OnRichTextCharacter" },
    { "wxEVT_RICHTEXT_CONSUMING_CHARACTER", "OnRichTextConsumingChar" },
    { "wxEVT_RICHTEXT_CONTENT_DELETED", "OnRichTextContentDeleted" },
    { "wxEVT_RICHTEXT_CONTENT_INSERTED", "OnRichTextContentInserted" },
    { "wxEVT_RICHTEXT_DELETE", "OnRichTextDeleteKey" },
    { "wxEVT_RICHTEXT_FOCUS_OBJECT_CHANGED", "OnRichTextFocusObjectChanged" },
    { "wxEVT_RICHTEXT_PROPERTIES_CHANGED", "OnRichTextPropChanged" },
    { "wxEVT_RICHTEXT_RETURN", "OnRichTextReturnKey" },
    { "wxEVT_RICHTEXT_SELECTION_CHANGED", "OnRichTextSelChanged" },
    { "wxEVT_RICHTEXT_STYLESHEET_CHANGED", "OnRichTextStyleSheetChanged" },
    { "wxEVT_RICHTEXT_STYLESHEET_CHANGING", "OnRichTextStyleSheetChanging" },
    { "wxEVT_RICHTEXT_STYLESHEET_REPLACED", "OnRichTextStyleSheetReplaced" },
    { "wxEVT_RICHTEXT_STYLESHEET_REPLACING", "OnRichTextStyleSheetReplacing" },
    { "wxEVT_RICHTEXT_STYLE_CHANGED", "OnRichTextStyleChanged" },

    { "wxEVT_RIGHT_DCLICK", "OnRightDClick" },
    { "wxEVT_RIGHT_DOWN", "OnRightDown" },
    { "wxEVT_RIGHT_UP", "OnRightUp" },
    { "wxEVT_SCROLL_BOTTOM", "OnCommandScrollBottom" },
    { "wxEVT_SCROLL_BOTTOM", "OnScrollBottom" },
    { "wxEVT_SCROLL_CHANGED", "OnCommandScrollChanged" },
    { "wxEVT_SCROLL_CHANGED", "OnScrollChanged" },
    { "wxEVT_SCROLL_LINEDOWN", "OnCommandScrollLineDown" },
    { "wxEVT_SCROLL_LINEDOWN", "OnScrollLineDown" },
    { "wxEVT_SCROLL_LINEUP", "OnCommandScrollLineUp" },
    { "wxEVT_SCROLL_LINEUP", "OnScrollLineUp" },
    { "wxEVT_SCROLL_PAGEDOWN", "OnCommandScrollPageDown" },
    { "wxEVT_SCROLL_PAGEDOWN", "OnScrollPageDown" },
    { "wxEVT_SCROLL_PAGEUP", "OnCommandScrollPageUp" },
    { "wxEVT_SCROLL_PAGEUP", "OnScrollPageUp" },
    { "wxEVT_SCROLL_THUMBRELEASE", "OnCommandScrollThumbRelease" },
    { "wxEVT_SCROLL_THUMBRELEASE", "OnScrollThumbRelease" },
    { "wxEVT_SCROLL_THUMBTRACK", "OnCommandScrollThumbTrack" },
    { "wxEVT_SCROLL_THUMBTRACK", "OnScrollThumbTrack" },
    { "wxEVT_SCROLL_TOP", "OnCommandScrollTop" },
    { "wxEVT_SCROLL_TOP", "OnScrollTop" },
    { "wxEVT_SEARCHCTRL_CANCEL_BTN", "OnCancelButton" },
    { "wxEVT_SEARCHCTRL_SEARCH_BTN", "OnSearchButton" },
    { "wxEVT_SET_FOCUS", "OnSetFocus" },
    { "wxEVT_SHOW", "OnShow" },
    { "wxEVT_SIZE", "OnSize" },
    { "wxEVT_SLIDER", "OnSlider" },
    { "wxEVT_SPIN", "OnSpin" },
    { "wxEVT_SPINCTRL", "OnSpinCtrl" },
    { "wxEVT_SPINCTRLDOUBLE", "OnSpinCtrlDouble" },
    { "wxEVT_SPIN_DOWN", "OnSpinDown" },
    { "wxEVT_SPIN_UP", "OnSpinUp" },
    { "wxEVT_TEXT", "OnText" },
    { "wxEVT_TEXT_ENTER", "OnTextEnter" },
    { "wxEVT_TEXT_MAXLEN", "OnTextMaxLen" },
    { "wxEVT_TEXT_URL", "OnTextURL" },
    { "wxEVT_TIMER", "OnTimer" },
    { "wxEVT_TIME_CHANGED", "OnTimeChanged" },
    { "wxEVT_TOGGLEBUTTON", "OnToggleButton" },
    { "wxEVT_TOOL", "OnToolClicked" },
    { "wxEVT_TOOL_DROPDOWN", "OnToolDropdown" },
    { "wxEVT_TOOL_ENTER", "OnToolEnter" },
    { "wxEVT_TOOL_RCLICKED", "OnToolRClicked" },
    { "wxEVT_TREELIST_COLUMN_SORTED", "OnTreelistColumnSorted" },
    { "wxEVT_TREELIST_ITEM_ACTIVATED", "OnTreelistItemActivated" },
    { "wxEVT_TREELIST_ITEM_CHECKED", "OnTreelistItemChecked" },
    { "wxEVT_TREELIST_ITEM_CONTEXT_MENU", "OnTreelistItemContextMenu" },
    { "wxEVT_TREELIST_ITEM_EXPANDED", "OnTreelistItemExpanded" },
    { "wxEVT_TREELIST_ITEM_EXPANDING", "OnTreelistItemExpanding" },
    { "wxEVT_TREELIST_SELECTION_CHANGED", "OnTreelistSelectionChanged" },
    { "wxEVT_TREE_BEGIN_DRAG", "OnTreeBeginDrag" },
    { "wxEVT_TREE_BEGIN_LABEL_EDIT", "OnTreeBeginLabelEdit" },
    { "wxEVT_TREE_BEGIN_RDRAG", "OnTreeBeginRDrag" },
    { "wxEVT_TREE_DELETE_ITEM", "OnTreeDeleteItem" },
    { "wxEVT_TREE_END_DRAG", "OnTreeEndDrag" },
    { "wxEVT_TREE_END_LABEL_EDIT", "OnTreeEndLabelEdit" },
    { "wxEVT_TREE_GET_INFO", "OnTreeGetInfo" },
    { "wxEVT_TREE_ITEM_ACTIVATED", "OnTreeItemActivated" },
    { "wxEVT_TREE_ITEM_COLLAPSED", "OnTreeItemCollapsed" },
    { "wxEVT_TREE_ITEM_COLLAPSING", "OnTreeItemCollapsing" },
    { "wxEVT_TREE_ITEM_EXPANDED", "OnTreeItemExpanded" },
    { "wxEVT_TREE_ITEM_EXPANDING", "OnTreeItemExpanding" },
    { "wxEVT_TREE_ITEM_GETTOOLTIP", "OnTreeItemGetTooltip" },
    { "wxEVT_TREE_ITEM_MENU", "OnTreeItemMenu" },
    { "wxEVT_TREE_ITEM_MIDDLE_CLICK", "OnTreeItemMiddleClick" },
    { "wxEVT_TREE_ITEM_RIGHT_CLICK", "OnTreeItemRightClick" },
    { "wxEVT_TREE_KEY_DOWN", "OnTreeKeyDown" },
    { "wxEVT_TREE_SEL_CHANGED", "OnTreeSelChanged" },
    { "wxEVT_TREE_SEL_CHANGING", "OnTreeSelChanging" },
    { "wxEVT_TREE_SET_INFO", "OnTreeSetInfo" },
    { "wxEVT_TREE_STATE_IMAGE_CLICK", "OnTreeStateImageClick" },
    { "wxEVT_UPDATE_UI", "OnUpdateUI" },
    { "wxEVT_WIZARD_BEFORE_PAGE_CHANGED", "OnWizardBeforePageChanged" },
    { "wxEVT_WIZARD_CANCEL", "OnWizardCancel" },
    { "wxEVT_WIZARD_FINISHED", "OnWizardFinished" },
    { "wxEVT_WIZARD_HELP", "OnWizardHelp" },
    { "wxEVT_WIZARD_PAGE_CHANGED", "OnWizardPageChanged" },
    { "wxEVT_WIZARD_PAGE_CHANGING", "OnWizardPageChanging" },
    { "wxEVT_WIZARD_PAGE_SHOWN", "OnWizardPageShown" },

    { "wxEVT_TREEBOOK_PAGE_CHANGED", "OnTreeBookPageChanged" },
    { "wxEVT_TREEBOOK_PAGE_CHANGING", "OnTreeBookPageChanging" },
    { "wxEVT_TREEBOOK_NODE_COLLAPSED", "OnTreeBookPageCollapsed" },
    { "wxEVT_TREEBOOK_NODE_EXPANDED", "OnTreeBookPageExpanded" },

    { "wxEVT_STC_AUTOCOMP_CANCELLED", "OnAutoCompleteCancelled" },
    { "wxEVT_STC_AUTOCOMP_CHAR_DELETED", "OnAutoCompleteDeleted" },
    { "wxEVT_STC_AUTOCOMP_COMPLETED", "OnAutoCompleteCompleted" },
    { "wxEVT_STC_AUTOCOMP_SELECTION", "OnAutoCompleteSelection" },
    { "wxEVT_STC_AUTOCOMP_SELECTION_CHANGE", "OnAutoCompleteSelChange" },
    { "wxEVT_STC_CALLTIP_CLICK", "OnCallTip" },
    { "wxEVT_STC_CHANGE", "OnChange" },
    { "wxEVT_STC_CHARADDED", "OnCharAdd" },
    { "wxEVT_STC_CLIPBOARD_COPY", "OnCopy" },
    { "wxEVT_STC_CLIPBOARD_PASTE", "OnPaste" },
    { "wxEVT_STC_DOUBLECLICK", "OnDoubleCLick" },
    { "wxEVT_STC_DO_DROP", "OnDrop" },
    { "wxEVT_STC_DRAG_OVER", "OnDragOver" },
    { "wxEVT_STC_DWELLEND", "OnDwellEnd" },
    { "wxEVT_STC_DWELLSTART", "OnDwellStart" },
    { "wxEVT_STC_HOTSPOT_CLICK", "OnHotspot" },
    { "wxEVT_STC_HOTSPOT_DCLICK", "OnHotspotDblClick" },
    { "wxEVT_STC_HOTSPOT_RELEASE_CLICK", "OnHotspotRelease" },
    { "wxEVT_STC_INDICATOR_CLICK", "OnIndicatorClick" },
    { "wxEVT_STC_INDICATOR_RELEASE", "OnIndicatorRelease" },
    { "wxEVT_STC_MACRORECORD", "OnMacroRecord" },
    { "wxEVT_STC_MARGINCLICK", "OnMarginClick" },
    { "wxEVT_STC_MARGIN_RIGHT_CLICK", "OnMarginRightClick" },
    { "wxEVT_STC_MODIFIED", "OnModified" },
    { "wxEVT_STC_NEEDSHOWN", "OnNeedShow" },
    { "wxEVT_STC_PAINTED", "OnPainted" },
    { "wxEVT_STC_ROMODIFYATTEMPT", "OnModifyAttempt" },
    { "wxEVT_STC_SAVEPOINTLEFT", "OnSavePointLeft" },
    { "wxEVT_STC_SAVEPOINTREACHED", "OnSavePointReached" },
    { "wxEVT_STC_START_DRAG", "OnStartDrag" },
    { "wxEVT_STC_STYLENEEDED", "OnStyleNeeded" },
    { "wxEVT_STC_UPDATEUI", "OnUpdateUI" },
    { "wxEVT_STC_USERLISTSELECTION", "OnUserListSelection" },
    { "wxEVT_STC_ZOOM", "OnZoom" },

    { "wxEVT_WEBVIEW_NAVIGATING", "OnNavigating" },
    { "wxEVT_WEBVIEW_NAVIGATED", "OnNavigated" },
    { "wxEVT_WEBVIEW_LOADED", "OnLoaded" },
    { "wxEVT_WEBVIEW_ERROR", "OnWebViewError" },
    { "wxEVT_WEBVIEW_NEWWINDOW", "OnWebViewWindow" },
    { "wxEVT_WEBVIEW_TITLE_CHANGED", "OnWebViewTitleChanged" },
    { "wxEVT_WEBVIEW_FULL_SCREEN_CHANGED", "OnFullScreen" },
    { "wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED", "OnScriptMessage" },

    { "ApplyButtonClicked", "OnApply" },
    { "CancelButtonClicked", "OnCancel" },
    { "CloseButtonClicked", "OnClose" },
    { "ContextHelpButtonClicked", "OnContextHelp" },
    { "HelpButtonClicked", "OnHelp" },
    { "NoButtonClicked", "OnNo" },
    { "OKButtonClicked", "OnOK" },
    { "SaveButtonClicked", "OnSave" },
    { "YesButtonClicked", "OnYes" },

};
