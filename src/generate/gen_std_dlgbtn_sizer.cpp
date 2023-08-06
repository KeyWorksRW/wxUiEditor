/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStdDialogButtonSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "../customprops/eventhandler_dlg.h"  // EventHandlerDlg static functions
#include "code.h"                             // Code -- Helper class for generating code
#include "gen_common.h"                       // GeneratorLibrary -- Generator classes
#include "lambdas.h"                          // Functions for formatting and storage of lamda events
#include "node.h"                             // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_std_dlgbtn_sizer.h"

wxObject* StdDialogButtonSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto dlg = wxDynamicCast(parent, wxDialog);
    auto sizer = new wxStdDialogButtonSizer();

    sizer->SetMinSize(node->as_wxSize(prop_minimum_size));

    if (node->as_bool(prop_OK))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_OK));
    else if (node->as_bool(prop_Yes))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_YES));
        if (dlg)
            dlg->SetAffirmativeId(wxID_YES);
    }
    else if (node->as_bool(prop_Save))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_SAVE));
        if (dlg)
            dlg->SetAffirmativeId(wxID_SAVE);
    }
    if (node->as_bool(prop_No))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_NO));
        if (dlg)
            dlg->SetEscapeId(wxID_NO);
    }

    if (node->as_bool(prop_Cancel))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CANCEL));
    else if (node->as_bool(prop_Close))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CLOSE));
        if (dlg)
            dlg->SetEscapeId(wxID_CLOSE);
    }

    if (node->as_bool(prop_Apply))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_APPLY));

    if (node->as_bool(prop_Help))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_HELP));
    else if (node->as_bool(prop_ContextHelp))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CONTEXT_HELP));

    sizer->Realize();

    if (node->as_bool(prop_static_line))
    {
        auto topsizer = new wxBoxSizer(wxVERTICAL);
        topsizer->Add(new wxStaticLine(wxDynamicCast(parent, wxWindow)), wxSizerFlags().Expand().DoubleBorder(wxBOTTOM));
        topsizer->Add(sizer, wxSizerFlags().Expand());
        return topsizer;
    }

    return sizer;
}

bool StdDialogButtonSizerGenerator::ConstructionCode(Code& code)
{
    // The Python code for StdDialogButtonSizer cannot be implemented the same way as the C++
    // code, so it needs it's own function. Specifically, wx/sizer.h has several public Get
    // functions for StdDialogButtonSizer, however none of them are documented, so while we
    // can use them in C++ code, wxPython has not implemented them.

    if (code.is_python())
    {
        GenPythonConstruction(code);
        return true;
    }

    else if (code.is_ruby())
    {
        GenRubyConstruction(code);
        return true;
    }

    code.AddAuto();

    Node* node = code.node();  // purely for convenience

    // Unfortunately, the CreateStdDialogButtonSizer() code does not support a wxID_SAVE or wxID_CONTEXT_HELP button
    // even though wxStdDialogButtonSizer does support it. Worse, CreateStdDialogButtonSizer() calls Realize() which
    // means if you add a button afterwards, then it will not be positioned correctly. You can't call Realize() twice
    // without hitting assertion errors in debug builds, and in release builds, the Save button is positioned
    // incorrectly. Unfortunately that means we have to add the buttons one at a time if a Save button is specified.

    auto& def_btn_name = node->as_string(prop_default_button);

    if (node->getForm()->isGen(gen_wxDialog) && (!node->as_bool(prop_Save) && !node->as_bool(prop_ContextHelp)))
    {
        code.NodeName();
        code += " = CreateStdDialogButtonSizer(";

        tt_string flags;

        auto AddBitFlag = [&](tt_string_view flag)
        {
            if (flags.size())
                flags << '|';
            flags << flag;
        };

        if (node->as_bool(prop_OK))
            AddBitFlag("wxOK");
        else if (node->as_bool(prop_Yes))
            AddBitFlag("wxYES");

        if (node->as_bool(prop_No))
            AddBitFlag("wxNO");

        if (node->as_bool(prop_Cancel))
            AddBitFlag("wxCANCEL");
        else if (node->as_bool(prop_Close))
            AddBitFlag("wxCLOSE");

        if (node->as_bool(prop_Apply))
            AddBitFlag("wxAPPLY");

        if (node->as_bool(prop_Help))
            AddBitFlag("wxHELP");

        if (def_btn_name != "OK" && def_btn_name != "Yes")
            AddBitFlag("wxNO_DEFAULT");

        code.Add(flags).EndFunction();
        if (def_btn_name == "Close" || def_btn_name == "Cancel")
            code.Eol().NodeName().Function("GetCancelButton()").Function("SetDefault(").EndFunction();
        else if (def_btn_name == "Apply")
            code.Eol().NodeName().Function("GetApplyButton()").Function("SetDefault(").EndFunction();

        return true;
    }

    code.NodeName().CreateClass(false, "wxStdDialogButtonSizer").EndFunction();

    auto min_size = node->as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code.Eol().NodeName().Function("SetMinSize(") << min_size.GetX() << ", " << min_size.GetY();
        code.EndFunction();
    }

    // You can only have one of: Ok, Yes, Save
    if (node->as_bool(prop_OK))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_OK));";
    }
    else if (node->as_bool(prop_Yes))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_YES));";
    }
    else if (node->as_bool(prop_Save))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_SAVE));";
    }

    if (node->as_bool(prop_No))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_NO));";
    }

    // You can only have one of: Cancel, Close
    if (node->as_bool(prop_Cancel))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_CANCEL));";
    }
    else if (node->as_bool(prop_Close))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_CLOSE));";
    }

    if (node->as_bool(prop_Apply))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_APPLY));";
    }

    // You can only have one of: Help, ContextHelp
    if (node->as_bool(prop_Help))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_HELP));";
    }
    else if (node->as_bool(prop_ContextHelp))
    {
        code.Eol().NodeName().Function("AddButton(");
        code += "new wxButton(this, wxID_CONTEXT_HELP));";
    }

    if (def_btn_name == "OK" || def_btn_name == "Yes" || def_btn_name == "Save")
        code.Eol().NodeName().Function("GetAffirmativeButton()").Function("SetDefault(").EndFunction();
    else if (def_btn_name == "Cancel" || def_btn_name == "Close")
        code.Eol().NodeName().Function("GetCancelButton()").Function("SetDefault(").EndFunction();
    else if (def_btn_name == "No")
        code.Eol().NodeName().Function("GetNegativeButton()").Function("SetDefault(").EndFunction();
    else if (def_btn_name == "Apply")
        code.Eol().NodeName().Function("GetApplyButton()").Function("SetDefault(").EndFunction();
    else if (def_btn_name == "Help" || def_btn_name == "ContextHelp")
        code.Eol().NodeName().Function("GetHelpButton()").Function("SetDefault(").EndFunction();

    code.Eol().NodeName().Function("Realize(").EndFunction();

    if (!node->isLocal())
    {
        code.Eol(eol_if_needed);
        if (node->as_bool(prop_OK))
            code.NodeName() << "OK = wxStaticCast(FindWindowById(wxID_OK), wxButton);\n";
        if (node->as_bool(prop_Yes))
            code.NodeName() << "Yes = wxStaticCast(FindWindowById(wxID_YES), wxButton);\n";
        if (node->as_bool(prop_Save))
            code.NodeName() << "Save = wxStaticCast(FindWindowById(wxID_SAVE), wxButton);\n";
        if (node->as_bool(prop_Apply))
            code.NodeName() << "Apply = wxStaticCast(FindWindowById(wxID_APPLY), wxButton);\n";

        if (node->as_bool(prop_No))
            code.NodeName() << "No = wxStaticCast(FindWindowById(wxID_NO), wxButton);\n";
        if (node->as_bool(prop_Cancel))
            code.NodeName() << "Cancel = wxStaticCast(FindWindowById(wxID_CANCEL), wxButton);\n";
        if (node->as_bool(prop_Close))
            code.NodeName() << "Close = wxStaticCast(FindWindowById(wxID_CLOSE), wxButton);\n";
        if (node->as_bool(prop_Help))
            code.NodeName() << "Help = wxStaticCast(FindWindowById(wxID_HELP), wxButton);\n";
        if (node->as_bool(prop_ContextHelp))
            code.NodeName() << "ContextHelp = wxStaticCast(FindWindowById(wxID_CONTEXT_HELP), wxButton);\n";
    }

    return true;
}

void StdDialogButtonSizerGenerator::GenPythonConstruction(Code& code)
{
    Node* node = code.node();  // purely for convenience

    // In wxPython, if you create wx.StdDialogButtonSizer() and pass it to
    // CreateSeparatedSizer, the dialog will not display correctly and will shortly exit. To
    // work around this, we create the line ourselves (except on MAC).

    code += "if \"wxMac\" not in wx.PlatformInfo:";
    code.Eol().Tab().NodeName().Add("_line = wx.StaticLine(self, wx.ID_ANY, wx.DefaultPosition, wx.Size(20, -1))");
    code.Eol().Tab().ParentName().Function("Add(").NodeName() += "_line, wx.SizerFlags().Expand().Border(wx.ALL))";

    code.Eol().Eol().NodeName().Add(" = wx.StdDialogButtonSizer()");

    auto min_size = node->as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code.Eol().NodeName().Function("SetMinSize(") << min_size.GetX() << ", " << min_size.GetY();
        code.EndFunction();
    }

    // You can only have one of: Ok, Yes, Save
    if (node->as_bool(prop_OK))
    {
        code.Eol().NodeName().Add("_OK = wx.Button(self, wx.ID_OK)");
        code.Eol().NodeName().Function("SetAffirmativeButton(").NodeName().Add("_OK").EndFunction();
    }
    else if (node->as_bool(prop_Yes))
    {
        code.Eol().NodeName().Add("_Yes = wx.Button(self, wx.ID_YES)");
        code.Eol().NodeName().Function("SetAffirmativeButton(").NodeName().Add("_Yes").EndFunction();
    }
    else if (node->as_bool(prop_Save))
    {
        code.Eol().NodeName().Add("_Save = wx.Button(self, wx.ID_SAVE)");
        code.Eol().NodeName().Function("SetAffirmativeButton(").NodeName().Add("_Save").EndFunction();
    }

    if (node->as_bool(prop_No))
    {
        code.Eol().NodeName().Add("_No = wx.Button(self, wx.ID_NO)");
        code.Eol().NodeName().Function("SetNegativeButton(").NodeName().Add("_No").EndFunction();
    }

    // You can only have one of: Cancel, Close
    if (node->as_bool(prop_Cancel))
    {
        code.Eol().NodeName().Add("_Cancel = wx.Button(self, wx.ID_CANCEL)");
        code.Eol().NodeName().Function("SetCancelButton(").NodeName().Add("_Cancel").EndFunction();
    }
    else if (node->as_bool(prop_Close))
    {
        code.Eol().NodeName().Add("_Close = wx.Button(self, wx.ID_CLOSE)");
        code.Eol().NodeName().Function("SetCancelButton(").NodeName().Add("_Close").EndFunction();
    }

    if (node->as_bool(prop_Help))
    {
        code.Eol().NodeName().Add("_Help = wx.Button(self, wx.ID_HELP)");
        code.Eol().NodeName().Function("AddButton(").NodeName().Add("_Help").EndFunction();
    }
    else if (node->as_bool(prop_ContextHelp))
    {
        code.Eol().NodeName().Add("_ContextHelp = wx.Button(self, wx.ID_CONTEXT_HELP)");
        code.Eol().NodeName().Function("AddButton(").NodeName().Add("_ContextHelp").EndFunction();
    }

    auto& def_btn_name = node->as_string(prop_default_button);

    if (def_btn_name == "OK")
        code.Eol().NodeName().Add("_OK").Function("SetDefault()");
    else if (def_btn_name == "Yes")
        code.Eol().NodeName().Add("_Yes").Function("SetDefault()");
    else if (def_btn_name == "Save")
        code.Eol().NodeName().Add("_Save").Function("SetDefault()");
    else if (def_btn_name == "No")
        code.Eol().NodeName().Add("_No").Function("SetDefault()");
    else if (def_btn_name == "Cancel")
        code.Eol().NodeName().Add("_Cancel").Function("SetDefault()");
    else if (def_btn_name == "Close")
        code.Eol().NodeName().Add("_Close").Function("SetDefault()");
    else if (def_btn_name == "Help")
        code.Eol().NodeName().Add("_Help").Function("SetDefault()");
    else if (def_btn_name == "ContextHelp")
        code.Eol().NodeName().Add("_ContextHelp").Function("SetDefault()");

    code.Eol().NodeName().Function("Realize()");
}

void StdDialogButtonSizerGenerator::GenRubyConstruction(Code& code)
{
    // Note that wxRuby3 does not support CreateStdDialogButtonSizer or CreateSeparatedSizer

    Node* node = code.node();  // purely for convenience

    // Add a static line above the buttons unless it is a Mac

    code += "if Wx::PLATFORM != 'WXMAC'";
    code.Eol().Tab().NodeName().Str("_line = Wx::StaticLine.new(self, Wx::ID_ANY, Wx::DEFAULT_POSITION, ");
    code.Eol().Tab(9 + (to_int) node->getNodeName().size()).Str("Wx::Size.new(20, -1))");
    code.Eol().Tab().ParentName().Function("Add(").NodeName() += "_line, Wx::SizerFlags.new.expand.border(Wx::ALL))";
    code.Eol() += "end";
    code.Eol().NodeName().Add(" = Wx::StdDialogButtonSizer.new");

    auto min_size = node->as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code.Eol().NodeName().Function("SetMinSize(") << min_size.GetX() << ", " << min_size.GetY();
        code.EndFunction();
    }

    auto& default_btn_name = node->as_string(prop_default_button);

    auto gen_btn_code = [&](std::string_view def_btn_name, std::string_view btn_name, std::string_view id)
    {
        if (def_btn_name == default_btn_name)
        {
            code.Eol().Str("_").Str(btn_name).Str(" = Wx::Button.new(self, ").Str(id).Str(")");
            code.Eol().NodeName().Function("add_button(_").Str(btn_name).Str(")");
            code.Eol().Str("_").Str(btn_name).Str(".set_default");
        }
        else
        {
            code.Eol().NodeName().Function("add_button(").Str("Wx::Button.new(self, ").Str(id).Str("))");
        }
    };

    // You can only have one of: Ok, Yes, Save
    if (node->as_bool(prop_OK))
    {
        gen_btn_code("OK", "ok_btn", "Wx::ID_OK");
    }
    else if (node->as_bool(prop_Yes))
    {
        gen_btn_code("Yes", "yes_btn", "Wx::ID_YES");
    }
    else if (node->as_bool(prop_Save))
    {
        gen_btn_code("Save", "save_btn", "Wx::ID_SAVE");
    }

    if (node->as_bool(prop_No))
    {
        gen_btn_code("No", "no_btn", "Wx::ID_NO");
    }

    // You can only have one of: Cancel, Close
    if (node->as_bool(prop_Cancel))
    {
        gen_btn_code("Cancel", "cancel_btn", "Wx::ID_CANCEL");
    }
    else if (node->as_bool(prop_Close))
    {
        gen_btn_code("Close", "close_btn", "Wx::ID_CLOSE");
    }

    if (node->as_bool(prop_Help))
    {
        gen_btn_code("Help", "help_btn", "Wx::ID_HELP");
    }
    else if (node->as_bool(prop_ContextHelp))
    {
        gen_btn_code("ContextHelp", "context_help_btn", "Wx::ID_CONTEXT_HELP");
    }

    code.Eol().NodeName().Function("realize");
}

int StdDialogButtonSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item;
    auto result = BaseGenerator::xrc_sizer_item_created;

    if (node->getParent()->isSizer())
    {
        // BUGBUG: [Randalphwa - 08-06-2023] This should be added with a platform directive

        // In C++, we would call CreateSeparatedSizer to get the line on Windows and Unix, but not on Mac. XRC doesn't
        // support this, so we emulate it by adding the line. That's not correct on a Mac, though...

        if (node->as_bool(prop_static_line))
        {
            object.append_attribute("class").set_value("sizeritem");
            object.append_child("flag").text().set("wxLEFT|wxRIGHT|wxTOP|wxEXPAND");
            object.append_child("border").text().set("5");

            item = object.append_child("object");
            item.append_attribute("class").set_value("wxBoxSizer");
            item.append_child("orient").text().set("wxVERTICAL");

            item = item.append_child("object");
            item.append_attribute("class").set_value("sizeritem");
            item.append_child("flag").text().set("wxBOTTOM|wxEXPAND");
            item.append_child("border").text().set("5");

            item = item.append_child("object");
            item.append_attribute("class").set_value("wxStaticLine");
            // item.append_child("size").text().set("20,-1");

            item = object.parent().append_child("object");
            GenXrcSizerItem(node, item);
            item = item.append_child("object");
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
    if (node->as_bool(prop_OK) || node->as_bool(prop_Yes) || node->as_bool(prop_Save))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        if (node->as_bool(prop_OK))
        {
            button.append_attribute("name").set_value("wxID_OK");
            if (node->isPropValue(prop_default_button, "OK"))
            {
                button.append_child("default").text().set(1);
            }
        }
        else if (node->as_bool(prop_Yes))
        {
            button.append_attribute("name").set_value("wxID_YES");
            if (node->isPropValue(prop_default_button, "Yes"))
            {
                button.append_child("default").text().set(1);
            }
        }
        else if (node->as_bool(prop_Save))
        {
            button.append_attribute("name").set_value("wxID_SAVE");
            if (node->isPropValue(prop_default_button, "Save"))
            {
                button.append_child("default").text().set(1);
            }
        }
    }

    if (node->as_bool(prop_No))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        button.append_attribute("name").set_value("wxID_NO");
        if (node->isPropValue(prop_default_button, "No"))
        {
            button.append_child("default").text().set(1);
        }
    }

    // You can only have one of: Cancel, Close
    if (node->as_bool(prop_Cancel) || node->as_bool(prop_Close))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        if (node->as_bool(prop_Cancel))
        {
            button.append_attribute("name").set_value("wxID_CANCEL");
            if (node->isPropValue(prop_default_button, "Cancel"))
            {
                button.append_child("default").text().set(1);
            }
        }
        else if (node->as_bool(prop_Close))
        {
            button.append_attribute("name").set_value("wxID_CLOSE");
            if (node->isPropValue(prop_default_button, "Close"))
            {
                button.append_child("default").text().set(1);
            }
        }
    }

    if (node->as_bool(prop_Apply))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        button.append_attribute("name").set_value("wxID_APPLY");
    }

    // You can only have one of: Help, ContextHelp
    if (node->as_bool(prop_Help) || node->as_bool(prop_ContextHelp))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        if (node->as_bool(prop_Help))
        {
            button.append_attribute("name").set_value("wxID_HELP");
        }
        else if (node->as_bool(prop_ContextHelp))
        {
            button.append_attribute("name").set_value("wxID_CONTEXT_HELP");
        }
    }

    return result;
}

void StdDialogButtonSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStdDialogButtonSizerXmlHandler");
}

void StdDialogButtonSizerGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    Code handler(event->getNode(), code.m_language);
    tt_string event_code;
    if (code.m_language == GEN_LANG_CPLUSPLUS)
    {
        event_code = EventHandlerDlg::GetCppValue(event->get_value());
    }
    else
    {
        event_code = EventHandlerDlg::GetPythonValue(event->get_value());
    }

    // This is what we normally use if an ID is needed. However, a lambda needs to put the ID on it's own line, so we
    // use a string for this to allow the lambda processing code to replace it.
    std::string comma(", ");
    if (event_code.contains("["))
    {
        if (!code.is_cpp())
            return;
        handler << event->get_name() << ',' << event->get_value();
        // Put the lambda expression on it's own line
        handler.GetCode().Replace("[", "\n\t[");
        comma = ",\n\t";
        ExpandLambda(handler.GetCode());
    }
    else if (event_code.contains("::"))
    {
        handler.Add(event->get_name()) << ", ";
        if (event->get_value()[0] != '&' && handler.is_cpp())
            handler << '&';
        handler << event->get_value();
    }
    else
    {
        if (code.is_cpp())
            handler << "&" << class_name << "::" << event_code << ", this";
        else
            handler.Add("self.") << event_code;
    }

    tt_string evt_str = (event->getEventInfo()->get_event_class() == "wxCommandEvent" ? "wxEVT_BUTTON" : "wxEVT_UPDATE_UI");
    if (code.is_python())
        code.Add("self.");
    code.Add("Bind(").Add(evt_str) << comma << handler.GetCode() << comma;

    if (event->get_name().starts_with("OKButton"))
        code.Add("wxID_OK");
    else if (event->get_name().starts_with("YesButton"))
        code.Add("wxID_YES");
    else if (event->get_name().starts_with("SaveButton"))
        code.Add("wxID_SAVE");
    else if (event->get_name().starts_with("ApplyButton"))
        code.Add("wxID_APPLY");
    else if (event->get_name().starts_with("NoButton"))
        code.Add("wxID_NO");
    else if (event->get_name().starts_with("CancelButton"))
        code.Add("wxID_CANCEL");
    else if (event->get_name().starts_with("CloseButton"))
        code.Add("wxID_CLOSE");
    else if (event->get_name().starts_with("HelpButton"))
        code.Add("wxID_HELP");
    else if (event->get_name().starts_with("ContextHelpButton"))
        code.Add("wxID_CONTEXT_HELP");

    code.EndFunction();
}

bool StdDialogButtonSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);

    return true;
}
