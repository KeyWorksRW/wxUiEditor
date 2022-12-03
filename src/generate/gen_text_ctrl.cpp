/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/manager.h>  // wxPropertyGridManager
#include <wx/textctrl.h>          // wxTextAttr and wxTextCtrlBase class - the interface of wxTextCtrl

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_text_ctrl.h"

wxObject* TextCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetMaxLength(node->prop_as_int(prop_maxlength));

    if (node->HasValue(prop_auto_complete))
    {
        auto array = ConvertToWxArrayString(node->prop_as_string(prop_auto_complete));
        widget->AutoComplete(array);
    }

    if (node->HasValue(prop_hint))
        widget->SetHint(node->prop_as_wxString(prop_hint));

    if (node->prop_as_string(prop_spellcheck).contains("enabled"))
    {
        if (node->prop_as_string(prop_spellcheck).contains("grammar"))
            widget->EnableProofCheck(wxTextProofOptions::Default().GrammarCheck());
        else
            widget->EnableProofCheck(wxTextProofOptions::Default());
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool TextCtrlGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_value))
    {
        wxStaticCast(widget, wxTextCtrl)->SetValue(prop->as_wxString());
        return true;
    }
#if defined(_WIN32)
    else if (prop->isProp(prop_spellcheck))
    {
        if (prop->HasValue() && !node->prop_as_string(prop_style).contains("wxTE_RICH2"))
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->ShowMessage("When used on Windows, spell checking requires the style to contain wxTE_RICH2.",
                                     wxICON_INFORMATION);
            }
        }
        else
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->Dismiss();
            }
        }
    }
    else if (prop->isProp(prop_style))
    {
        if (node->HasValue(prop_spellcheck) && !node->prop_as_string(prop_style).contains("wxTE_RICH2"))
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->ShowMessage("When used on Windows, spell checking requires the style to contain wxTE_RICH2.",
                                     wxICON_INFORMATION);
            }
        }
        else
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->Dismiss();
            }
        }
    }
#endif  // _WIN32

    return false;
}

std::optional<ttlib::cstr> TextCtrlGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.GetParentName().Comma().as_string(prop_id).Comma().CheckLineLength();
    code.QuotedString(prop_value).CheckLineLength();
    code.PosSizeFlags(true);

    return code.m_code;
}

std::optional<ttlib::cstr> TextCtrlGenerator::CommonSettings(Code& code, size_t& auto_indent)
{
    if ((code.IsTrue(prop_maxlength) && code.PropContains(prop_style, "wxTE_MULTILINE")) ||
        code.HasValue(prop_auto_complete))
        auto_indent = false;

    if (code.HasValue(prop_hint))
    {
        code.Eol(true);
        if (!auto_indent)
            code.Tab(code.is_cpp() ? 1 : 3);
        code.Eol(true).NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }
    if (code.IsTrue(prop_focus))
    {
        code.Eol(true);
        if (!auto_indent)
            code.Tab(code.is_cpp() ? 1 : 2);
        code.NodeName().Function("SetFocus(").EndFunction();
    }
    if (code.IsTrue(prop_maxlength))
    {
        code.Eol(true);
        if (!auto_indent)
            code.Tab(code.is_cpp() ? 1 : 2);
        if (code.PropContains(prop_style, "wxTE_MULTILINE"))
        {
            if (code.is_cpp())
            {
                code << "#if !defined(__WXGTK__))\n\t";
                code.NodeName().Function("SetMaxLength(").as_string(prop_maxlength).EndFunction().Eol();
                code << "\t#endif";
            }
            else
            {
                code.Add("if wx.Platform != \'__WXGTK__\':\n\t\t\t");
                code.NodeName().Function("SetMaxLength(").as_string(prop_maxlength).EndFunction().Eol();
            }
        }
        else
        {
            code.NodeName().Function("SetMaxLength(").as_string(prop_maxlength).EndFunction().Eol();
        }
    }
    if (code.HasValue(prop_auto_complete))
    {
        if (code.is_cpp())
        {
            code << "\t{\n\t\twxArrayString tmp_array;\n";
            auto array = ConvertToArrayString(code.node()->prop_as_string(prop_auto_complete));
            for (auto& iter: array)
            {
                code << "\t\ttmp_array.push_back(wxString::FromUTF8(\"" << iter << "\"));\n";
            }
            code << "\t\t" << code.node()->get_node_name() << "->AutoComplete(tmp_array);\n";
            code << "\t}";
        }

        // TODO: [Randalphwa - 12-02-2022] Add Python code
    }

    if (code.PropContains(prop_spellcheck, "enabled"))
    {
        if (code.is_cpp())
        {
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#if wxCHECK_VERSION(3, 1, 6)\n\t";
                code << code.node()->get_node_name() << "->EnableProofCheck(wxTextProofOptions::Default()";
                if (code.PropContains(prop_spellcheck, "grammar"))
                    code << ".GrammarCheck()";
                code << ");";
                code << "\n#endif";
            }
            else
            {
                code.NodeName() << "->EnableProofCheck(wxTextProofOptions::Default()";
                if (code.PropContains(prop_spellcheck, "grammar"))
                    code << ".GrammarCheck()";
                code << ");";
            }
        }
        else
        {
            code.Eol();
            if (!auto_indent)
                code.Tab(2);
            code.Add("# wxPython 4.2.0 does not support wxTextProofOptions\n");
        }
    }

    return code.m_code;
}

std::optional<ttlib::cstr> TextCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

void TextCtrlGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop)
{
    if (changed_prop->isProp(prop_spellcheck))
    {
        if (auto pg_parent = prop_grid->GetProperty("spellcheck"); pg_parent)
        {
            for (unsigned int idx = 0; idx < pg_parent->GetChildCount(); ++idx)
            {
                if (auto pg_setting = pg_parent->Item(idx); pg_setting)
                {
                    auto label = pg_setting->GetLabel();
                    if (label == "grammar")
                    {
                        pg_setting->Enable(changed_prop->as_string().contains("enabled"));
                    }
                }
            }
        }
    }
    else
    {
        BaseGenerator::ChangeEnableState(prop_grid, changed_prop);
    }
}

bool TextCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/textctrl.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
    {
        if (node->prop_as_string(prop_validator_type) == "wxGenericValidator")
            InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
        else
            InsertGeneratorInclude(node, "#include <wx/valtext.h>", set_src, set_hdr);
    }

    return true;
}

int TextCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxTextCtrl");

    ADD_ITEM_PROP(prop_maxlength, "maxlength")
    ADD_ITEM_PROP(prop_value, "value")

    GenXrcStylePosSize(node, item);

    if (node->HasValue(prop_value))
    {
        item.append_child("value").text().set(node->prop_as_string(prop_value));
    }
    if (node->HasValue(prop_hint))
    {
        item.append_child("hint").text().set(node->prop_as_string(prop_hint));
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->HasValue(prop_auto_complete))
        {
            item.append_child(pugi::node_comment).set_value(" auto complete cannot be be set in the XRC file. ");
        }
        if (node->HasValue(prop_spellcheck))
        {
            item.append_child(pugi::node_comment).set_value(" spell check cannot be be set in the XRC file. ");
        }
        GenXrcComments(node, item);
    }

    return result;
}

void TextCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxActivityIndicatorXmlHandler");
}
