/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStdDialogButtonSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "lambdas.h"     // Functions for formatting and storage of lamda events
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_std_dlgbtn_sizer.h"

wxObject* StdDialogButtonSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto dlg = wxDynamicCast(parent, wxDialog);
    auto sizer = new wxStdDialogButtonSizer();

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));

    if (node->prop_as_bool(prop_OK))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_OK));
    else if (node->prop_as_bool(prop_Yes))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_YES));
        if (dlg)
            dlg->SetAffirmativeId(wxID_YES);
    }
    else if (node->prop_as_bool(prop_Save))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_SAVE));
        if (dlg)
            dlg->SetAffirmativeId(wxID_SAVE);
    }
    if (node->prop_as_bool(prop_No))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_NO));
        if (dlg)
            dlg->SetEscapeId(wxID_NO);
    }

    if (node->prop_as_bool(prop_Cancel))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CANCEL));
    else if (node->prop_as_bool(prop_Close))
    {
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CLOSE));
        if (dlg)
            dlg->SetEscapeId(wxID_CLOSE);
    }

    if (node->prop_as_bool(prop_Apply))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_APPLY));

    if (node->prop_as_bool(prop_Help))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_HELP));
    else if (node->prop_as_bool(prop_ContextHelp))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CONTEXT_HELP));

    sizer->Realize();

    if (node->prop_as_bool(prop_static_line))
    {
        auto topsizer = new wxBoxSizer(wxVERTICAL);
        topsizer->Add(new wxStaticLine(wxDynamicCast(parent, wxWindow)), wxSizerFlags().Expand().DoubleBorder(wxBOTTOM));
        topsizer->Add(sizer, wxSizerFlags().Expand());
        return topsizer;
    }

    return sizer;
}

std::optional<ttlib::sview> StdDialogButtonSizerGenerator::CommonConstruction(Code& code)
{
    // The Python code for StdDialogButtonSizer cannot be implemented the same way as the C++
    // code, so it needs it's own function. Specifically, wx/sizer.h has several public Get
    // functions for StdDialogButtonSizer, however none of them are documented, so while we
    // can use them in C++ code, wxPython has not implemented them.

    if (code.is_python())
    {
        GenPythonConstruction(code);
        return code.m_code;
    }

    if (code.is_local_var())
        code << "auto* ";

    Node* node = code.node();  // purely for convenience

    // Unfortunately, the CreateStdDialogButtonSizer() code does not support a wxID_SAVE or wxID_CONTEXT_HELP button
    // even though wxStdDialogButtonSizer does support it. Worse, CreateStdDialogButtonSizer() calls Realize() which
    // means if you add a button afterwards, then it will not be positioned correctly. You can't call Realize() twice
    // without hitting assertion errors in debug builds, and in release builds, the Save button is positioned
    // incorrectly. Unfortunately that means we have to add the buttons one at a time if a Save button is specified.

    auto& def_btn_name = node->prop_as_string(prop_default_button);

    if (!node->get_form()->isGen(gen_wxDialog) || node->as_bool(prop_Save) || node->as_bool(prop_ContextHelp))
    {
        code.NodeName();
        if (code.is_cpp())
            code += " = CreateStdDialogButtonSizer(";
        else
            code += " = self.CreateStdDialogButtonSizer(";

        ttlib::cstr flags;
        if (node->prop_as_bool(prop_OK))
            AddBitFlag(flags, "wxOK");
        else if (node->prop_as_bool(prop_Yes))
            AddBitFlag(flags, "wxYES");

        if (node->prop_as_bool(prop_No))
            AddBitFlag(flags, "wxNO");

        if (node->prop_as_bool(prop_Cancel))
            AddBitFlag(flags, "wxCANCEL");
        else if (node->prop_as_bool(prop_Close))
            AddBitFlag(flags, "wxCLOSE");

        if (node->prop_as_bool(prop_Apply))
            AddBitFlag(flags, "wxAPPLY");

        if (node->prop_as_bool(prop_Help))
            AddBitFlag(flags, "wxHELP");

        if (def_btn_name != "OK" && def_btn_name != "Yes")
            AddBitFlag(flags, "wxNO_DEFAULT");

        code.Add(flags).EndFunction();
        if (def_btn_name == "Close" || def_btn_name == "Cancel")
            code.Eol().NodeName().Function("GetCancelButton()").Function("SetDefault(").EndFunction();
        else if (def_btn_name == "Apply")
            code.Eol().NodeName().Function("GetApplyButton()").Function("SetDefault(").EndFunction();

        return code.m_code;
    }

    code.NodeName().CreateClass(false, "wxStdDialogButtonSizer").EndFunction();

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code.Eol().NodeName().Function("SetMinSize(") << min_size.GetX() << ", " << min_size.GetY();
        code.EndFunction();
    }

    // You can only have one of: Ok, Yes, Save
    if (node->prop_as_bool(prop_OK))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_OK));";
        else
            code += "wx.Button(self, wx.ID_OK))";
    }
    else if (node->prop_as_bool(prop_Yes))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_YES));";
        else
            code += "wx.Button(self, wx.ID_YES))";
    }
    else if (node->prop_as_bool(prop_Save))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_SAVE));";
        else
            code += "wx.Button(self, wx.ID_SAVE))";
    }

    if (node->prop_as_bool(prop_No))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_NO));";
        else
            code += "wx.Button(self, wx.ID_NO))";
    }

    // You can only have one of: Cancel, Close
    if (node->prop_as_bool(prop_Cancel))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_CANCEL));";
        else
            code += "wx.Button(self, wx.ID_CANCEL))";
    }
    else if (node->prop_as_bool(prop_Close))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_CLOSE));";
        else
            code += "wx.Button(self, wx.ID_CLOSE))";
    }

    if (node->prop_as_bool(prop_Apply))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_APPLY));";
        else
            code += "wx.Button(self, wx.ID_APPLY))";
    }

    // You can only have one of: Help, ContextHelp
    if (node->prop_as_bool(prop_Help))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_HELP));";
        else
            code += "wx.Button(self, wx.ID_HELP))";
    }
    else if (node->prop_as_bool(prop_ContextHelp))
    {
        code.Eol().NodeName().Function("AddButton(");
        if (code.is_cpp())
            code += "new wxButton(this, wxID_CONTEXT_HELP));";
        else
            code += "wx.Button(self, wx.ID_CONTEXT_HELP))";
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

    // REVIEW: [Randalphwa - 12-09-2022] Do we need this for Python?
    if (!node->IsLocal() && code.is_cpp())
    {
        if (node->prop_as_bool(prop_OK))
            code << node->get_node_name() << "OK = wxStaticCast(FindWindowById(wxID_OK), wxButton);\n";
        if (node->prop_as_bool(prop_Yes))
            code << node->get_node_name() << "Yes = wxStaticCast(FindWindowById(wxID_YES), wxButton);\n";
        if (node->prop_as_bool(prop_Save))
            code << node->get_node_name() << "Save = wxStaticCast(FindWindowById(wxID_SAVE), wxButton);\n";
        if (node->prop_as_bool(prop_Apply))
            code << node->get_node_name() << "Apply = wxStaticCast(FindWindowById(wxID_APPLY), wxButton);\n";

        if (node->prop_as_bool(prop_No))
            code << node->get_node_name() << "No = wxStaticCast(FindWindowById(wxID_NO), wxButton);\n";
        if (node->prop_as_bool(prop_Cancel))
            code << node->get_node_name() << "Cancel = wxStaticCast(FindWindowById(wxID_CANCEL), wxButton);\n";
        if (node->prop_as_bool(prop_Close))
            code << node->get_node_name() << "Close = wxStaticCast(FindWindowById(wxID_CLOSE), wxButton);\n";
        if (node->prop_as_bool(prop_Help))
            code << node->get_node_name() << "Help = wxStaticCast(FindWindowById(wxID_HELP), wxButton);\n";
        if (node->prop_as_bool(prop_ContextHelp))
            code << node->get_node_name() << "ContextHelp = wxStaticCast(FindWindowById(wxID_CONTEXT_HELP), wxButton);\n";
    }

    return code.m_code;
}

void StdDialogButtonSizerGenerator::GenPythonConstruction(Code& code)
{
    Node* node = code.node();  // purely for convenience

    // In wxPython, if you create wx.StdDialogButtonSizer() and pass it to
    // CreateSeparatedSizer, the dialog will not display correctly and will shortly exit. To
    // work around this, we create the line ourselves (except on MAC).

    code += "if not \" wxMac \" in wx.PlatformInfo:";
    code.Eol().Tab().NodeName().Add("_line = wx.StaticLine(self, wx.ID_ANY, wx.DefaultPosition, wx.Size(20, -1))");
    code.Eol().Tab().ParentName().Function("Add(").NodeName() += "_line, wx.SizerFlags().Expand().Border(wx.ALL))";

    code.Eol().Eol().NodeName().Add(" = wx.StdDialogButtonSizer()");

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code.Eol().NodeName().Function("SetMinSize(") << min_size.GetX() << ", " << min_size.GetY();
        code.EndFunction();
    }

    // You can only have one of: Ok, Yes, Save
    if (node->prop_as_bool(prop_OK))
    {
        code.Eol().NodeName().Add("_OK = wx.Button(self, wx.ID_OK)");
        code.Eol().NodeName().Function("SetAffirmativeButton(").NodeName().Add("_OK").EndFunction();
    }
    else if (node->prop_as_bool(prop_Yes))
    {
        code.Eol().NodeName().Add("_Yes = wx.Button(self, wx.ID_YES)");
        code.Eol().NodeName().Function("SetAffirmativeButton(").NodeName().Add("_Yes").EndFunction();
    }
    else if (node->prop_as_bool(prop_Save))
    {
        code.Eol().NodeName().Add("_Save = wx.Button(self, wx.ID_SAVE)");
        code.Eol().NodeName().Function("SetAffirmativeButton(").NodeName().Add("_Save").EndFunction();
    }

    if (node->prop_as_bool(prop_No))
    {
        code.Eol().NodeName().Add("_No = wx.Button(self, wx.ID_NO)");
        code.Eol().NodeName().Function("SetNegativeButton(").NodeName().Add("_No").EndFunction();
    }

    // You can only have one of: Cancel, Close
    if (node->prop_as_bool(prop_Cancel))
    {
        code.Eol().NodeName().Add("_Cancel = wx.Button(self, wx.ID_CANCEL)");
        code.Eol().NodeName().Function("SetCancelButton(").NodeName().Add("_Cancel").EndFunction();
    }
    else if (node->prop_as_bool(prop_Close))
    {
        code.Eol().NodeName().Add("_Close = wx.Button(self, wx.ID_CLOSE)");
        code.Eol().NodeName().Function("SetCancelButton(").NodeName().Add("_Close").EndFunction();
    }

    if (node->prop_as_bool(prop_Help))
    {
        code.Eol().NodeName().Add("_Help = wx.Button(self, wx.ID_HELP)");
        code.Eol().NodeName().Function("AddButton(").NodeName().Add("_Help").EndFunction();
    }
    else if (node->prop_as_bool(prop_ContextHelp))
    {
        code.Eol().NodeName().Add("_ContextHelp = wx.Button(self, wx.ID_CONTEXT_HELP)");
        code.Eol().NodeName().Function("AddButton(").NodeName().Add("_ContextHelp").EndFunction();
    }

    auto& def_btn_name = node->prop_as_string(prop_default_button);

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

int StdDialogButtonSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item;
    auto result = BaseGenerator::xrc_sizer_item_created;

    if (node->GetParent()->IsSizer())
    {
        // In C++, we would call CreateSeparatedSizer to get the line on Windows and Unix, but not on Mac. XRC doesn't
        // support this, so we emulate it by adding the line. That's not correct on a Mac, though...

        if (node->prop_as_bool(prop_static_line))
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
    item.append_attribute("name").set_value(node->prop_as_string(prop_var_name));
    // You can only have one of: Ok, Yes, Save
    if (node->prop_as_bool(prop_OK) || node->prop_as_bool(prop_Yes) || node->prop_as_bool(prop_Save))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        if (node->prop_as_bool(prop_OK))
        {
            button.append_attribute("name").set_value("wxID_OK");
            if (node->isPropValue(prop_default_button, "OK"))
            {
                button.append_child("default").text().set(1);
            }
        }
        else if (node->prop_as_bool(prop_Yes))
        {
            button.append_attribute("name").set_value("wxID_YES");
            if (node->isPropValue(prop_default_button, "Yes"))
            {
                button.append_child("default").text().set(1);
            }
        }
        else if (node->prop_as_bool(prop_Save))
        {
            button.append_attribute("name").set_value("wxID_SAVE");
            if (node->isPropValue(prop_default_button, "Save"))
            {
                button.append_child("default").text().set(1);
            }
        }
    }

    if (node->prop_as_bool(prop_No))
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
    if (node->prop_as_bool(prop_Cancel) || node->prop_as_bool(prop_Close))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        if (node->prop_as_bool(prop_Cancel))
        {
            button.append_attribute("name").set_value("wxID_CANCEL");
            if (node->isPropValue(prop_default_button, "Cancel"))
            {
                button.append_child("default").text().set(1);
            }
        }
        else if (node->prop_as_bool(prop_Close))
        {
            button.append_attribute("name").set_value("wxID_CLOSE");
            if (node->isPropValue(prop_default_button, "Close"))
            {
                button.append_child("default").text().set(1);
            }
        }
    }

    if (node->prop_as_bool(prop_Apply))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        button.append_attribute("name").set_value("wxID_APPLY");
    }

    // You can only have one of: Help, ContextHelp
    if (node->prop_as_bool(prop_Help) || node->prop_as_bool(prop_ContextHelp))
    {
        auto button_parent = item.append_child("object");
        button_parent.append_attribute("class").set_value("button");

        auto button = button_parent.append_child("object");
        button.append_attribute("class").set_value("wxButton");

        if (node->prop_as_bool(prop_Help))
        {
            button.append_attribute("name").set_value("wxID_HELP");
        }
        else if (node->prop_as_bool(prop_ContextHelp))
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

std::optional<ttlib::sview> StdDialogButtonSizerGenerator::GenEvents(Code& code, NodeEvent* event,
                                                                     const std::string& class_name)
{
    Code handler(event->GetNode(), code.m_language);

    // This is what we normally use if an ID is needed. However, a lambda needs to put the ID on it's own line, so we
    // use a string for this to allow the lambda processing code to replace it.
    std::string comma(", ");
    if (event->get_value().contains("["))
    {
        if (!code.is_cpp())
            return {};
        handler << event->get_name() << ',' << event->get_value();
        // Put the lambda expression on it's own line
        handler.m_code.Replace("[", "\n\t[");
        comma = ",\n\t";
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
        if (code.is_cpp())
            handler << "&" << class_name << "::" << event->get_value() << ", this";
        else
            handler.Add("self.") << event->get_value();
    }

    ttlib::cstr evt_str =
        (event->GetEventInfo()->get_event_class() == "wxCommandEvent" ? "wxEVT_BUTTON" : "wxEVT_UPDATE_UI");
    if (code.is_python())
        code.Add("self.");
    code.Add("Bind(").Add(evt_str) << comma << handler.m_code << comma;

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

    return code.m_code;
}
bool StdDialogButtonSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);

    return true;
}
