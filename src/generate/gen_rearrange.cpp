/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRearrangeCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/rearrangectrl.h>  // various controls for rearranging the items interactively
#include <wx/stattext.h>       // wxStaticText base header

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // WriteCode -- Write code to Scintilla or file

#include "gen_rearrange.h"

wxObject* RearrangeCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    if (Project.getCodePreference() == GEN_LANG_RUBY || Project.getCodePreference() == GEN_LANG_XRC)
    {
        tt_string msg = "wxRearrangeCtrl not available in ";
        if (Project.getCodePreference() == GEN_LANG_RUBY)
            msg += "wxRuby3";
        else
            msg += "XRC";
        auto* widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, msg.make_wxString(), wxDefaultPosition,
                                        wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_RAISED);
        widget->Wrap(DlgPoint(parent, 150));
        return widget;
    }
    auto widget = new wxRearrangeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size), wxArrayInt(), wxArrayString(),
                                      node->as_int(prop_type) | GetStyleInt(node));

    auto items = node->as_checklist_items(prop_contents);
    if (items.size())
    {
        for (auto& iter: items)
        {
            auto pos = widget->GetList()->Append(iter.label.make_wxString());
            if (iter.checked == "1")
                widget->GetList()->Check(pos);
        }

        if (node->as_string(prop_selection_string).size())
        {
            widget->GetList()->SetStringSelection(node->as_wxString(prop_selection_string));
        }
        else
        {
            int sel = node->as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) widget->GetList()->GetCount())
                widget->GetList()->SetSelection(sel);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RearrangeCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    code.Comma().Pos().Comma().WxSize();
    code.Comma();
    if (code.is_cpp())
        code += "wxArrayInt(), wxArrayString()";
    else
        code += "[], []";

    Node* node = code.node();
    auto& type = node->as_string(prop_type);
    auto& style = node->as_string(prop_style);
    auto& win_style = node->as_string(prop_window_style);

    if (type == "wxLB_SINGLE" && style.empty() && win_style.empty())
    {
        if (node->hasValue(prop_window_name))
        {
            code += ", 0";
        }
    }
    else
    {
        code.Comma().Add(type).Comma().Style();
    }

    if (node->hasValue(prop_window_name))
    {
        code.Comma().Add("wxDefaultValidator").Comma().QuotedString(prop_window_name);
    }
    code.EndFunction();

    return true;
}

bool RearrangeCtrlGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
        {
            code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
        }
    }
    if (code.hasValue(prop_contents))
    {
        Node* node = code.node();
        auto contents = node->as_checklist_items(prop_contents);
        bool checked_item = false;
        for (auto& iter: contents)
        {
            if (iter.checked == "1")
            {
                checked_item = true;
                break;
            }
        }

        if (!checked_item)
        {
            for (auto& iter: contents)
            {
                code.Eol(eol_if_empty)
                    .NodeName()
                    .Function("GetList()")
                    .Function("Append(")
                    .QuotedString(iter.label)
                    .EndFunction();
            }
        }
        else
        {
            code.OpenBrace();
            if (code.is_cpp())
                code += "int item_position;";
            for (auto& iter: contents)
            {
                code.Eol(eol_if_empty);
                if (iter.checked == "1")
                    code += "item_position = ";
                code.NodeName().Function("GetList()").Function("Append(").QuotedString(iter.label).EndFunction();
                if (iter.checked == "1")
                    code.Eol().NodeName().Function("GetList()").Function("Check(item_position").EndFunction();
            }
            code.CloseBrace();
        }

        if (code.hasValue(prop_selection_string))
        {
            code.Eol(eol_if_empty).NodeName().Function("GetList()").Function("SetStringSelection(");
            code.QuotedString(prop_selection_string).EndFunction();
        }
        else
        {
            int sel = node->as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) contents.size())
            {
                code.Eol(eol_if_empty)
                    .NodeName()
                    .Function("GetList()")
                    .Function("SetSelection(")
                    .as_string(prop_selection_int)
                    .EndFunction();
            }
        }
    }
    return true;
}

int RearrangeCtrlGenerator::GetRequiredVersion(Node* node)
{
    if (node->hasValue(prop_contents))
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }

    return BaseGenerator::GetRequiredVersion(node);
}

bool RearrangeCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                         int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/rearrangectrl.h>", set_src, set_hdr);
    return true;
}
