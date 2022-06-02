/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStdDialogButtonSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_std_dlgbtn_sizer.h"

wxObject* StdDialogButtonSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto sizer = new wxStdDialogButtonSizer();

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));

    if (node->prop_as_bool(prop_OK))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_OK));
    else if (node->prop_as_bool(prop_Yes))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_YES));
    else if (node->prop_as_bool(prop_Save))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_SAVE));

    if (node->prop_as_bool(prop_No))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_NO));

    if (node->prop_as_bool(prop_Cancel))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CANCEL));
    else if (node->prop_as_bool(prop_Close))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CLOSE));

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

std::optional<ttlib::cstr> StdDialogButtonSizerGenerator::GenConstruction(Node* node)
{
    auto& def_btn_name = node->prop_as_string(prop_default_button);

    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";

    // Unfortunately, the CreateStdDialogButtonSizer() code does not support a wxID_SAVE or wxID_CONTEXT_HELP button
    // even though wxStdDialogButtonSizer does support it. Worse, CreateStdDialogButtonSizer() calls Realize() which
    // means if you add a button afterwards, then it will not be positioned correctly. You can't call Realize() twice
    // without hitting assertion errors in debug builds, and in release builds, the Save button is positioned
    // incorrectly. Unfortunately that means we have to add the buttons one at a time if a Save button is specified.

    if (node->get_form()->isGen(gen_wxDialog) && !node->prop_as_bool(prop_Save) && !node->prop_as_bool(prop_ContextHelp))
    {
        code << node->get_node_name() << " = CreateStdDialogButtonSizer(";
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

        code << flags << ");";

        // If wxNO_DEFAULT is specified and a No button is used, then it will be set as the default
        if (def_btn_name == "Close" || def_btn_name == "Cancel")
            code << "\n\t" << node->get_node_name() << "->GetCancelButton()->SetDefault();";
        else if (def_btn_name == "Apply")
            code << "\n\t" << node->get_node_name() << "->GetApplyButton()->SetDefault();";

        return code;
    }

    // The following code is used if a Save or ContextHelp button is requrested, or the parent form is not a Dialog
    code << node->get_node_name() << " = new wxStdDialogButtonSizer();";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    // You can only have one of: Ok, Yes, Save
    if (node->prop_as_bool(prop_OK))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_OK));";
    else if (node->prop_as_bool(prop_Yes))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_YES));";
    else if (node->prop_as_bool(prop_Save))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_SAVE));";

    if (node->prop_as_bool(prop_No))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_NO));";

    // You can only have one of: Cancel, Close
    if (node->prop_as_bool(prop_Cancel))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CANCEL));";
    else if (node->prop_as_bool(prop_Close))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CLOSE));";

    if (node->prop_as_bool(prop_Apply))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_APPLY));";

    // You can only have one of: Help, ContextHelp
    if (node->prop_as_bool(prop_Help))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_HELP));";
    else if (node->prop_as_bool(prop_ContextHelp))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CONTEXT_HELP));";

    if (def_btn_name == "OK" || def_btn_name == "Yes" || def_btn_name == "Save")
        code << "\n\t" << node->get_node_name() << "->GetAffirmativeButton()->SetDefault();";
    else if (def_btn_name == "Cancel" || def_btn_name == "Close")
        code << "\n\t" << node->get_node_name() << "->GetCancelButton()->SetDefault();";
    else if (def_btn_name == "Apply")
        code << "\n\t" << node->get_node_name() << "->GetApplyButton()->SetDefault();";
    else if (def_btn_name == "No")
        code << "\n\t" << node->get_node_name() << "->GetNegativeButton()->SetDefault();";
    else if (def_btn_name == "Help" || def_btn_name == "ContextHelp")
        code << "\n\t" << node->get_node_name() << "->GetHelpButton()->SetDefault();";

    code << "\n\t" << node->get_node_name() << "->Realize();\n";

    if (!node->IsLocal())
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

    return code;
}

int StdDialogButtonSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool /* add_comments */)
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
            object.append_child("flag").text().set("wxALL|wxEXPAND");

            item = object.append_child("object");
            item.append_attribute("class").set_value("wxBoxSizer");
            item.append_child("orient").text().set("wxHORIZONTAL");

            item = item.append_child("object");
            item.append_attribute("class").set_value("sizeritem");
            item.append_child("flag").text().set("wxTOP|wxRIGHT|wxLEFT|wxALIGN_BOTTOM");
            item.append_child("option").text().set(1);

            item = item.append_child("object");
            item.append_attribute("class").set_value("wxStaticLine");
            item.append_child("size").text().set("20,-1");

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

std::optional<ttlib::cstr> StdDialogButtonSizerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    ttlib::cstr code;
    ttlib::cstr handler;

    // This is what we normally use if an ID is needed. However, a lambda needs to put the ID on it's own line, so we
    // use a string for this to allow the lambda processing code to replace it.
    std::string comma(", ");

    if (event->get_value().contains("["))
    {
        handler << event->get_name() << ',' << event->get_value();
        // Put the lambda expression on it's own line
        handler.Replace("[", "\n\t\t[");
        comma = ",\n\t";
    }
    else if (event->get_value().contains("::"))
    {
        if (event->get_value()[0] != '&')
            handler << '&';
        handler << event->get_value();
    }
    else
    {
        // code << "Bind(" << evt_str << ", &" << class_name << "::" << event->get_value() << ", this, ";
        handler << "&" << class_name << "::" << event->get_value() << ", this";
    }

    ttlib::cstr evt_str =
        (event->GetEventInfo()->get_event_class() == "wxCommandEvent" ? "wxEVT_BUTTON" : "wxEVT_UPDATE_UI");
    code << "Bind(" << evt_str << comma << handler << comma;

    if (event->get_name().starts_with("OKButton"))
        code << "wxID_OK);";
    else if (event->get_name().starts_with("YesButton"))
        code << "wxID_YES);";
    else if (event->get_name().starts_with("SaveButton"))
        code << "wxID_SAVE);";
    else if (event->get_name().starts_with("ApplyButton"))
        code << "wxID_APPLY);";
    else if (event->get_name().starts_with("NoButton"))
        code << "wxID_NO);";
    else if (event->get_name().starts_with("CancelButton"))
        code << "wxID_CANCEL);";
    else if (event->get_name().starts_with("CloseButton"))
        code << "wxID_CLOSE);";
    else if (event->get_name().starts_with("HelpButton"))
        code << "wxID_HELP);";
    else if (event->get_name().starts_with("ContextHelpButton"))
        code << "wxID_CONTEXT_HELP);";

    return code;
}

bool StdDialogButtonSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);

    return true;
}
