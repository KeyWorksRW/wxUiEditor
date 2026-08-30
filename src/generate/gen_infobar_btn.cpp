///////////////////////////////////////////////////////////////////////////////
// Purpose:   wxInfoBar button generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
///////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>  // declaration of wxInfoBarBase defining common API of wxInfoBar

#include <algorithm>  // std::transform, std::tolower

#include "code.h"              // Code -- Helper class for generating code
#include "eventhandler_dlg.h"  // EventHandlerDlg static functions
#include "gen_common.h"        // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"     // Common XRC generating functions
#include "lambdas.h"           // Functions for formatting and storage of lambda events
#include "node.h"              // Node class
#include "project_handler.h"   // ProjectHandler class
#include "utils.h"             // Utility functions that work with properties

#include "gen_infobar_btn.h"

bool InfoBarBtnGenerator::ConstructionCode(Code& code)
{
    code.ParentName()
        .Function("AddButton(")
        .as_string(prop_id)
        .Comma()
        .QuotedString(prop_label)
        .EndFunction();

    return true;
}

void InfoBarBtnGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    Code handler(event->getNode(), code.get_language());
    wxue::string event_code;
    if (code.get_language() == GenLang::cplusplus)
    {
        event_code = EventHandlerDlg::GetCppValue(event->get_value());
    }
    else if (code.get_language() == GenLang::python)
    {
        event_code = EventHandlerDlg::GetPythonValue(event->get_value());
    }
    else if (code.get_language() == GenLang::ruby)
    {
        event_code = EventHandlerDlg::GetRubyValue(event->get_value());
    }
    else
    {
        FAIL_MSG("Unknown language");
        event_code.clear();
    }

    if (event_code.empty() || event_code == "none")
    {
        return;
    }

    std::string comma(", ");

    // An opening bracket ('[') indicates a lambda expression
    if (event_code.contains("["))
    {
        if (code.is_cpp())
        {
            handler << event->get_name() << ',';
            handler.ExpandEventLambda(event_code);
            comma = ",\n\t";
        }
        else if (code.is_python())
        {
            handler.Add(event->get_name()) += ", lambda event:";
            const size_t body_pos = event_code.find(']') + 1;
            event_code.erase(0, body_pos);
            handler.Str(event_code);
        }
        else if (code.is_ruby())
        {
            handler << event->get_name() << ',' << event_code;
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
            {
                handler << '&';
            }
        }
        else
        {
            std::ignore = event_code.Replace("::", ".");
        }

        handler << event_code;
    }
    else
    {
        wxue::string event_name = event->get_name();
        if (event_name == "wxEVT_BUTTON" && code.is_python())
        {
            event_name = "wxEVT_BUTTON";
        }

        if (code.is_cpp() || code.is_python())
        {
            handler.Add(event_name);
            if (code.is_cpp())
            {
                handler << ", &" << class_name << "::" << event_code << ", this";
            }
            else if (code.is_python())
            {
                handler.Add(", self.") << event_code;
            }
        }
        else if (code.is_ruby())
        {
            // remove "wx" prefix, make the rest of the name lower case
            event_name.erase(0, 2);
            std::transform(event_name.begin(), event_name.end(), event_name.begin(),
                           [](unsigned char character) -> unsigned char
                           {
                               return std::tolower(character);
                           });
            handler.Str(event_name).Str("(: ") << event_code << ')';
        }
    }

    // With lambdas, line breaks have already been added
    code.EnableAutoLineBreak(!event_code.contains("["));

    if (code.is_cpp() || code.is_python())
    {
        // The button's event must be bound to the parent wxInfoBar, not to a
        // non-existent child variable. Note that Code's node is the form, so
        // resolve the parent via the event node.
        code.NodeName(event->getNode()->get_Parent()).Function("Bind(")
            << handler.GetCode() << comma;
        code.Add(event->getNode()->as_string(prop_id));
        code.EndFunction();
    }
    else if (code.is_ruby())
    {
        // wxRuby3's evt_button macro takes the button id and the handler symbol, e.g.
        // @infoBar.evt_button(Wx::ID_SAVE, :on_info_bar_save)
        code.NodeName(event->getNode()->get_Parent())
                .Str(".evt_button(")
                .Add(event->getNode()->as_string(prop_id))
                .Str(", :")
            << event_code << ')';
    }
}

bool InfoBarBtnGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                      std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/infobar.h>", set_src, set_hdr);

    return true;
}

int InfoBarBtnGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "button");
    ADD_ITEM_PROP(prop_label, "label")

    return BaseGenerator::xrc_updated;
}

void InfoBarBtnGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& /* handlers */)
{
    // The parent wxInfoBar's RequiredHandlers() adds wxInfoBarXmlHandler, which handles the
    // "button" child objects.
}
