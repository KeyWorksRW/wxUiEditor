/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStdDialogButtonSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>

#include <wx/button.h>
#include <wx/cshelp.h>  // Context-sensitive help support classes
#include <wx/sizer.h>
#include <wx/statline.h>

#include "gen_std_dlgbtn_sizer.h"

#include "code.h"              // Code -- Helper class for generating code
#include "eventhandler_dlg.h"  // EventHandlerDlg static functions
#include "gen_common.h"        // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"     // Common XRC generating functions
#include "lambdas.h"           // Functions for formatting and storage of lamda events
#include "node.h"              // Node class
#include "utils.h"             // Utility functions that work with properties

#include "pugixml.hpp"  // xml read/write/create/process

namespace
{
    auto CanUseCreateStdDialogButtonSizer(Node* node) -> bool
    {
        // The CreateStdDialogButtonSizer() code does not support a wxID_SAVE or
        // wxID_CONTEXT_HELP button even though wxStdDialogButtonSizer does support it.

        return node->get_Form()->is_Gen(gen_wxDialog) &&
               (!node->as_bool(prop_Save) && !node->as_bool(prop_ContextHelp));
    }
}  // namespace

auto StdDialogButtonSizerGenerator::CreateMockup(Node* node, wxObject* parent) -> wxObject*
{
    auto* dlg = wxDynamicCast(parent, wxDialog);
    auto* sizer = new wxStdDialogButtonSizer();

    sizer->SetMinSize(node->as_wxSize(prop_minimum_size));

    if (node->as_bool(prop_OK))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_OK));
    }
    else if (node->as_bool(prop_Yes))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_YES));
        if (dlg)
        {
            dlg->SetAffirmativeId(wxID_YES);
        }
    }
    else if (node->as_bool(prop_Save))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_SAVE));
        if (dlg)
        {
            dlg->SetAffirmativeId(wxID_SAVE);
        }
    }
    if (node->as_bool(prop_No))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_NO));
        if (dlg)
        {
            dlg->SetEscapeId(wxID_NO);
        }
    }

    if (node->as_bool(prop_Cancel))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CANCEL));
    }
    else if (node->as_bool(prop_Close))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CLOSE));
        if (dlg)
        {
            dlg->SetEscapeId(wxID_CLOSE);
        }
    }

    if (node->as_bool(prop_Apply))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_APPLY));
    }

    if (node->as_bool(prop_Help))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_HELP));
    }
    else if (node->as_bool(prop_ContextHelp))
    {
        // sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CONTEXT_HELP, "?"));
        sizer->AddButton(
            new wxContextHelpButton(wxStaticCast(parent, wxWindow), wxID_CONTEXT_HELP));
    }

    sizer->Realize();

    if (node->as_bool(prop_static_line))
    {
        auto* topsizer = new wxBoxSizer(wxVERTICAL);
        topsizer->Add(new wxStaticLine(wxDynamicCast(parent, wxWindow)),
                      wxSizerFlags().Expand().DoubleBorder(wxBOTTOM));
        topsizer->Add(sizer, wxSizerFlags().Expand());
        return topsizer;
    }

    return sizer;
}

auto StdDialogButtonSizerGenerator::ConstructionCode(Code& code) -> bool
{
    code.AddAuto();

    Node* node = code.node();  // purely for convenience

    // Unfortunately, the CreateStdDialogButtonSizer() code does not support a wxID_SAVE or
    // wxID_CONTEXT_HELP button even though wxStdDialogButtonSizer does support it. Worse,
    // CreateStdDialogButtonSizer() calls Realize() which means if you add a button afterwards, then
    // it will not be positioned correctly. You can't call Realize() twice without hitting assertion
    // errors in debug builds, and in release builds, the Save button is positioned incorrectly.
    // Unfortunately that means we have to add the buttons one at a time if a Save button is
    // specified.

    const auto& def_btn_name = node->as_string(prop_default_button);

    if (CanUseCreateStdDialogButtonSizer(node))
    {
        code.NodeName().Assign().FormFunction("CreateStdDialogButtonSizer(");

        tt_string flags;

        auto AddBitFlag = [&](tt_string_view flag)
        {
            if (flags.size())
            {
                flags << '|';
            }
            flags << flag;
        };

        if (node->as_bool(prop_OK))
        {
            AddBitFlag("wxOK");
        }
        else if (node->as_bool(prop_Yes))
        {
            AddBitFlag("wxYES");
        }

        if (node->as_bool(prop_No))
        {
            AddBitFlag("wxNO");
        }

        if (node->as_bool(prop_Cancel))
        {
            AddBitFlag("wxCANCEL");
        }
        else if (node->as_bool(prop_Close))
        {
            AddBitFlag("wxCLOSE");
        }

        if (node->as_bool(prop_Apply))
        {
            AddBitFlag("wxAPPLY");
        }

        if (node->as_bool(prop_Help))
        {
            AddBitFlag("wxHELP");
        }

        if (def_btn_name != "OK" && def_btn_name != "Yes")
        {
            AddBitFlag("wxNO_DEFAULT");
        }

        code.Add(flags).EndFunction();
        if (def_btn_name == "Close" || def_btn_name == "Cancel")
        {
            code.Eol()
                .NodeName()
                .Function("GetCancelButton()")
                .Function("SetDefault(")
                .EndFunction();
        }
        else if (def_btn_name == "Apply")
        {
            code.Eol()
                .NodeName()
                .Function("GetApplyButton()")
                .Function("SetDefault(")
                .EndFunction();
        }

        return true;
    }

    code.NodeName().CreateClass(false, "wxStdDialogButtonSizer").EndFunction();

    auto min_size = node->as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code.Eol().NodeName().Function("SetMinSize(") << min_size.GetX() << ", " << min_size.GetY();
        code.EndFunction();
    }

    auto lambda_AddButton = [&](std::string_view var_name, std::string_view id)
    {
        if (id == "wxID_CONTEXT_HELP")
        {
            code.Eol().NodeName().Function("AddButton(");
            code.CreateClass(false, "wxContextHelpButton", false);
            code.FormParent().Comma().Add(id).Str(")").EndFunction();
            return;
        }
        if (!code.is_local_var() || def_btn_name == var_name)
        {
            tt_string btn_name;
            if (code.is_cpp())
            {
                // For Python, Ruby, and Perl, we use the variable name as the button name
                btn_name = var_name;
            }
            else
            {
                // For non-C++ languages, we convert the variable name to lower case and prepend an
                // underscore.
                if (code.is_perl())
                {
                    btn_name = "$self->{";
                    btn_name += code.node()->get_NodeName(GEN_LANG_PERL);
                    btn_name += "_";
                }
                else
                {
                    btn_name = "_";
                }
                for (const auto& character: var_name)
                {
                    btn_name += static_cast<char>(std::tolower(character));
                }
                if (code.is_perl())
                {
                    btn_name += "}";
                }
            }

            code.Eol();
            // In Perl, the variable name is in {} brackets, so we had to add the underscore
            // and suffix id above, so the btn_name is now complete.
            if (!code.is_perl())
            {
                code.NodeName();
            }
            code.Str(btn_name).CreateClass(false, "wxButton");
            code.FormParent().Comma().Add(id).EndFunction();
            code.Eol().NodeName().Function("AddButton(");
            if (!code.is_perl())
            {
                code.NodeName();
            }
            code.Str(btn_name).EndFunction();
            if (def_btn_name == var_name)
            {
                code.Eol();
                if (!code.is_perl())
                {
                    code.NodeName();
                }
                code.Str(btn_name).Function("SetDefault(").EndFunction();
            }
        }
        else
        {
            code.Eol().NodeName().Function("AddButton(");
            code.CreateClass(false, "wxButton", false);
            code.FormParent().Comma().Add(id).Str(")").EndFunction();
        }
    };

    // You can only have one of: Ok, Yes, Save
    if (node->as_bool(prop_OK))
    {
        lambda_AddButton("OK", "wxID_OK");
    }
    else if (node->as_bool(prop_Yes))
    {
        lambda_AddButton("Yes", "wxID_YES");
    }
    else if (node->as_bool(prop_Save))
    {
        lambda_AddButton("Save", "wxID_SAVE");
    }

    if (node->as_bool(prop_No))
    {
        lambda_AddButton("No", "wxID_NO");
    }

    // You can only have one of: Cancel, Close
    if (node->as_bool(prop_Cancel))
    {
        lambda_AddButton("Cancel", "wxID_CANCEL");
    }
    else if (node->as_bool(prop_Close))
    {
        lambda_AddButton("Close", "wxID_CLOSE");
    }

    if (node->as_bool(prop_Apply))
    {
        lambda_AddButton("Apply", "wxID_APPLY");
    }

    // You can only have one of: Help, ContextHelp
    if (node->as_bool(prop_Help))
    {
        lambda_AddButton("Help", "wxID_HELP");
    }
    else if (node->as_bool(prop_ContextHelp))
    {
        lambda_AddButton("ContextHelp", "wxID_CONTEXT_HELP");
    }
    code.Eol().NodeName().Function("Realize(").EndFunction();
    return true;
}

namespace
{
    void AddXrcButton(pugi::xml_node& parent_item, const char* button_id, Node* node,
                      const char* default_name)
    {
        auto button_parent = parent_item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");
        button.append_attribute("name").set_value(button_id);

        if (node->is_PropValue(prop_default_button, default_name))
        {
            button.append_child("default").text().set(1);
        }
    }

    void AddXrcContextHelpButton(pugi::xml_node& parent_item)
    {
        auto button_parent = parent_item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");
        button.append_attribute("name").set_value("wxID_CONTEXT_HELP");
        button.append_child("label").text().set("?");
    }

    auto SetupXrcItemWithStaticLine(Node* node, pugi::xml_node& object) -> pugi::xml_node
    {
        object.append_attribute("class").set_value("sizeritem");
        object.append_child("flag").text().set("wxLEFT|wxRIGHT|wxTOP|wxEXPAND");
        object.append_child("border").text().set("5");

        auto item = object.append_child("object");
        item.append_attribute("class").set_value("wxBoxSizer");
        item.append_child("orient").text().set("wxVERTICAL");

        item = item.append_child("object");
        item.append_attribute("class").set_value("sizeritem");
        item.append_child("flag").text().set("wxBOTTOM|wxEXPAND");
        item.append_child("border").text().set("5");

        item = item.append_child("object");
        item.append_attribute("class").set_value("wxStaticLine");

        item = object.parent().append_child("object");
        GenXrcSizerItem(node, item);
        return item.append_child("object");
    }
}  // namespace

auto StdDialogButtonSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object,
                                                 size_t /* xrc_flags */) -> int
{
    pugi::xml_node item;
    auto result = BaseGenerator::xrc_sizer_item_created;

    if (node->get_Parent()->is_Sizer())
    {
        // BUGBUG: [Randalphwa - 08-06-2023] This should be added with a platform directive

        // In C++, we would call CreateSeparatedSizer to get the line on Windows and Unix, but not
        // on Mac. XRC doesn't support this, so we emulate it by adding the line. That's not correct
        // on a Mac, though...

        if (node->as_bool(prop_static_line))
        {
            item = SetupXrcItemWithStaticLine(node, object);
        }
        else
        {
            GenXrcSizerItem(node, object);
            item = object.append_child("object");
        }
    }
    else
    {
        item = object;
        result = BaseGenerator::xrc_updated;
    }

    item.append_attribute("class").set_value("wxStdDialogButtonSizer");
    item.append_attribute("name").set_value(node->as_string(prop_var_name));

    // BUGBUG: [Randalphwa - 08-06-2023] Need to set min size if specified

    // You can only have one of: Ok, Yes, Save
    if (node->as_bool(prop_OK))
    {
        AddXrcButton(item, "wxID_OK", node, "OK");
    }
    else if (node->as_bool(prop_Yes))
    {
        AddXrcButton(item, "wxID_YES", node, "Yes");
    }
    else if (node->as_bool(prop_Save))
    {
        AddXrcButton(item, "wxID_SAVE", node, "Save");
    }

    if (node->as_bool(prop_No))
    {
        AddXrcButton(item, "wxID_NO", node, "No");
    }

    // You can only have one of: Cancel, Close
    if (node->as_bool(prop_Cancel))
    {
        AddXrcButton(item, "wxID_CANCEL", node, "Cancel");
    }
    else if (node->as_bool(prop_Close))
    {
        AddXrcButton(item, "wxID_CLOSE", node, "Close");
    }

    if (node->as_bool(prop_Apply))
    {
        AddXrcButton(item, "wxID_APPLY", node, "Apply");
    }

    // You can only have one of: Help, ContextHelp
    if (node->as_bool(prop_Help))
    {
        AddXrcButton(item, "wxID_HELP", node, "Help");
    }
    else if (node->as_bool(prop_ContextHelp))
    {
        AddXrcContextHelpButton(item);
    }

    return result;
}

void StdDialogButtonSizerGenerator::RequiredHandlers(Node* /* node unused */,
                                                     std::set<std::string>& handlers)
{
    handlers.emplace("wxStdDialogButtonSizerXmlHandler");
}

namespace
{
    [[nodiscard]] auto GetEventCodeForLanguage(GenLang language, std::string_view value)
        -> std::string
    {
        switch (language)
        {
            case GEN_LANG_CPLUSPLUS:
                return EventHandlerDlg::GetCppValue(value);
            case GEN_LANG_PYTHON:
                return EventHandlerDlg::GetPythonValue(value);
            case GEN_LANG_RUBY:
                return EventHandlerDlg::GetRubyValue(value);
            case GEN_LANG_PERL:
                return EventHandlerDlg::GetPerlValue(value);
            case GEN_LANG_RUST:
                return EventHandlerDlg::GetRustValue(value);
            default:
                FAIL_MSG(tt_string() << "No event handlers for " << GenLangToString(language)
                                     << " (" << language << ")");
                return EventHandlerDlg::GetCppValue(value);
        }
    }

    const std::array<std::pair<std::string_view, std::string_view>, 10> button_suffix_map = { {
        { "OKButton", "_ok" },
        { "YesButton", "_yes" },
        { "SaveButton", "_save" },
        { "NoButton", "_no" },
        { "CancelButton", "_cancel" },
        { "CloseButton", "_close" },
        { "HelpButton", "_help" },
        { "ContextHelpButton", "_ctx_help" },
        { "ApplyButton", "_apply" },
    } };

    [[nodiscard]] auto GetButtonIdSuffix(std::string_view event_name) -> std::string_view
    {
        for (const auto& [prefix, suffix]: button_suffix_map)
        {
            if (event_name.starts_with(prefix))
            {
                return suffix;
            }
        }
        return {};
    }

    const std::array<std::pair<std::string_view, std::string_view>, 10> button_id_map = { {
        { "OKButton", "wxID_OK" },
        { "YesButton", "wxID_YES" },
        { "SaveButton", "wxID_SAVE" },
        { "ApplyButton", "wxID_APPLY" },
        { "NoButton", "wxID_NO" },
        { "CancelButton", "wxID_CANCEL" },
        { "CloseButton", "wxID_CLOSE" },
        { "HelpButton", "wxID_HELP" },
        { "ContextHelpButton", "wxID_CONTEXT_HELP" },
    } };

    [[nodiscard]] auto GetButtonIdConstant(std::string_view event_name) -> std::string_view
    {
        for (const auto& [prefix, id_btn]: button_id_map)
        {
            if (event_name.starts_with(prefix))
            {
                return id_btn;
            }
        }
        return {};
    }

    void GenerateHandlerCode(Code& handler, const Code& code, const std::string& event_code,
                             NodeEvent* event, const std::string& class_name, std::string& comma)
    {
        if (event_code.find('[') != std::string::npos || event_code.find("::") != std::string::npos)
        {
            // BUGBUG: [Randalphwa - 08-19-2025] Why aren't we supporting Python and Ruby lambdas
            // like we do in gen_events.cpp
            if (!code.is_cpp())
            {
                return;
            }
            handler << event->get_name() << ',' << event->get_value();
            handler.GetCode().Replace("[", "\n\t[");
            comma = ",\n\t";
            ExpandLambda(handler.GetCode());
        }
        else if (event_code.find("::") != std::string::npos)
        {
            // REVIEW: [Randalphwa - 08-19-2025] This needs to be updated to match the code in
            // gen_events.cpp
            handler.Add(event->get_name()) << ", ";
            if (event->get_value()[0] != '&' && handler.is_cpp())
            {
                handler << '&';
            }
            handler << event->get_value();
        }
        else
        {
            if (code.is_cpp())
            {
                handler << "&" << class_name << "::" << event_code << ", this";
            }
            else if (code.is_python())
            {
                handler.Add("self.") << event_code;
            }
            else if (code.is_ruby())
            {
                handler << event_code;
            }
        }
    }

    void GenerateEventBinding(Code& code, std::string_view event_name,
                              const std::string& handler_code, const std::string& comma)
    {
        if (code.is_python())
        {
            code.Add("self.");
        }

        if (code.is_ruby() && (event_name == "wxEVT_BUTTON" || event_name == "wxEVT_UPDATE_UI"))
        {
            code.Str(event_name == "wxEVT_BUTTON" ? "evt_button(" : "evt_update_ui(");
        }
        else if (code.is_perl())
        {
            event_name.remove_prefix(2);  // remove "wx" prefix
            code.Str(event_name).Str("($self, ");
        }
        else
        {
            code.Add("Bind(").Add(event_name) << comma << handler_code << comma;
        }
    }

    // Returns true if the Bind code is complete, false if more needs to be added
    [[nodiscard]] auto AddButtonIdentifier(Code& code, NodeEvent* event) -> bool
    {
        const auto& event_name = event->get_name();
        const auto is_script_lang =
            (code.m_language == GEN_LANG_PERL || code.m_language == GEN_LANG_PYTHON ||
             code.m_language == GEN_LANG_RUBY);

        if (is_script_lang)
        {
            if (CanUseCreateStdDialogButtonSizer(code.node()))
            {
                if (code.is_python())
                {
                    code.Str("self");
                    if (const auto id_btn = GetButtonIdConstant(event_name); !id_btn.empty())
                    {
                        code.Comma().Add(id_btn);
                    }
                    return true;
                }
            }

            if (const auto suffix = GetButtonIdSuffix(event_name); !suffix.empty())
            {
                code.NodeName(event->getNode()).Add(suffix);
            }
        }
        else
        {
            const auto id_constant = GetButtonIdConstant(event_name);
            if (!id_constant.empty())
            {
                code.Add(id_constant);
                return true;
            }
        }
        return false;
    }

    void FinalizeEventCode(Code& code, const std::string& event_code, const Code& handler_code)
    {
        if (code.is_ruby())
        {
            code.Str(".get_id").Comma().Str(":") << handler_code;
        }
        else if (code.is_python())
        {
            code.Comma() << handler_code;
        }
        else if (code.is_perl())
        {
            code.Replace("}", "");
            code.Str("}->GetId(), $self->can('") << event_code << "')";
        }
    }
}  // namespace

void StdDialogButtonSizerGenerator::GenEvent(Code& code, NodeEvent* event,
                                             const std::string& class_name)
{
    Code handler(event->getNode(), code.m_language);
    const auto event_code = GetEventCodeForLanguage(code.m_language, event->get_value());

    // This is what we normally use if an ID is needed. However, a lambda needs to put the ID on
    // it's own line, so we use a string for this to allow the lambda processing code to replace it.
    std::string comma(", ");
    GenerateHandlerCode(handler, code, event_code, event, class_name, comma);

    std::string_view event_name =
        (event->get_EventInfo()->get_event_class() == "wxCommandEvent" ? "wxEVT_BUTTON" :
                                                                         "wxEVT_UPDATE_UI");
    GenerateEventBinding(code, event_name, handler.GetCode(), comma);
    if (!AddButtonIdentifier(code, event))
    {
        FinalizeEventCode(code, event_code, handler);
    }
    code.EndFunction();
}

auto StdDialogButtonSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                                std::set<std::string>& set_hdr,
                                                GenLang /* language */) -> bool
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    if (node->as_bool(prop_ContextHelp))
    {
        set_src.insert("#include <wx/cshelp.h>");
    }

    return true;
}

auto StdDialogButtonSizerGenerator::GetImports(Node* node, std::set<std::string>& set_imports,
                                               GenLang language) -> bool
{
    if (language == GEN_LANG_PERL)
    {
        if (node->as_bool(prop_ContextHelp))
        {
            set_imports.emplace("use Wx::Help;");
            return true;
        }
    }

    return false;
}
