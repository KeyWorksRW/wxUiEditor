/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#include "wxui/eventhandler_dlg_base.h"  // EventHandlerDlgBase

class NodeEvent;
class Node;

class EventHandlerDlg : public EventHandlerDlgBase
{
public:
    EventHandlerDlg(wxWindow* parent, NodeEvent* event);

    /*
        The C++ event string is always added first whether it is a function or a lambda.
        If both languages use the same function name, then the function name will be the
        entire string. If the function names are different or either a C++ or Python lambda
        is used, then the python section will begin with "[python:".


        If wxPthon uses a lambda, it will be indicated with "[python:lambda]". If it's a
        function name which is not the same as the C++ function name (either a name
        difference, or C++ uses a lambda) then the lambda will be "[python:name] with "name"
        being the function name.

        C++ lambda strings are stored with a '@@' to indicate a newline since body of the
        lambda can contain multiple lines. Python lambda's can only use a single expression, so
        newline characters are not preserved.

        It should be rare for projects to have both C++ and wxPython code, but this mechanism
        does allow for both.

        Examples:

            OnButton
            OnButton[python:OnBtnClick]
            [this](wxCommandEvent&)@@{@@m_my_ctrl->SetValue("Hello World!");@@}[python:OnButton]
            OnButton[python:lambda]self.m_my_ctrl.SetValue("Hello World")

        Note that the first example uses OnButton for both C++ and wxPython. This is the most
        common case.

    */

    const wxString& GetResults() { return m_value; }

    // This will return a string as if C++ was the only value specified even if the original
    // value had values for multiple languages. Note that this *will* return a value even if
    // C++ is not enabled and another language specified a value.
    static auto GetCppValue(std::string_view value) -> std::string;

    // This will return a string as if Perl was the only value specified even if the original
    // value had values for multiple languages. Note that this *will* return a value even if
    // Perl is not enabled and another language specified a value.
    static tt_string GetPerlValue(tt_string_view value);

    // This will return a string as if Python was the only value specified even if the original
    // value had values for multiple languages. Note that this *will* return a value even if
    // Python is not enabled and another language specified a value.
    static tt_string GetPythonValue(tt_string_view value);

    // This will return a string as if Ruby was the only value specified even if the original
    // value had values for multiple languages. Note that this *will* return a value even if
    // Ruby is not enabled and another language specified a value.
    static tt_string GetRubyValue(tt_string_view value);

protected:
    // This is used to colorize member variables in the C++ lambda
    void CollectMemberVariables(Node* node, std::set<std::string>& variables);

    // This is called to combine C++/wxPython values into a single string. See comments in
    // eventhandler_dlg.h as to how this string is constructed.
    void Update_m_value();

    void FormatBindText();

    // Handlers for EventHandlerDlgBase events
    void OnChange(wxCommandEvent& /* event unused */) override;
    void OnInit(wxInitDialogEvent& /* event unused */) override;
    void OnOK(wxCommandEvent& event) override;
    void OnPageChanged(wxBookCtrlEvent& event) override;
    void OnUseCppFunction(wxCommandEvent& /* event unused */) override;
    void OnUseCppLambda(wxCommandEvent& /* event unused */) override;
    void OnUsePythonFunction(wxCommandEvent& event) override;
    void OnUsePythonLambda(wxCommandEvent& event) override;
    void OnUseRubyFunction(wxCommandEvent& event) override;
    void OnUseRubyLambda(wxCommandEvent& /* event unused */) override;
    void OnDefault(wxCommandEvent& event) override;
    void OnNone(wxCommandEvent& event) override;

    wxString m_value;

private:
    NodeEvent* m_event;

    int m_perl_page;
    int m_python_page;
    int m_ruby_page;

    size_t m_gen_languages;     // set by Project.get_GenerateLanguages()
    GenLang m_code_preference;  // This will be one of the GEN_LANG values

    bool m_is_cpp_enabled { false };
    bool m_is_perl_enabled { false };
    bool m_is_python_enabled { false };
    bool m_is_ruby_enabled { false };

    bool m_is_cpp_lambda { false };
    // bool m_is_perl_lambda { false };
    bool m_is_python_lambda { false };
    bool m_is_ruby_lambda { false };
};
