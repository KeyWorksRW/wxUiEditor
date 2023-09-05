/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTreeListCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/treelist.h>  // wxTreeListCtrl class declaration.

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_tree_list.h"

wxObject* TreeListCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTreeListCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                     DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void TreeListCtrlGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node, bool /* is_preview */)
{
    auto widget = wxStaticCast(wxobject, wxTreeListCtrl);

    for (const auto& iter: node->getChildNodePtrs())
    {
        widget->AppendColumn(iter->as_wxString(prop_label), iter->as_int(prop_width),
                             static_cast<wxAlignment>(iter->as_int(prop_alignment)), iter->as_int(prop_flags));
    }
}

bool TreeListCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(true, "wxTL_DEFAULT_STYLE");

    return true;
}

bool TreeListCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/treelist.h>", set_src, set_hdr);
    return true;
}

std::optional<tt_string> TreeListCtrlGenerator::GetWarning(Node* node, int language)
{
    switch (language)
    {
        case GEN_LANG_PYTHON:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxPython currently does not support wxTreeListCtrl";
                return msg;
            }

        case GEN_LANG_RUBY:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxRuby currently does not support wxTreeListCtrl";
                return msg;
            }
        default:
            return {};
    }
}

//////////////////////////////////////////  TreeListCtrlColumnGenerator  //////////////////////////////////////////

bool TreeListCtrlColumnGenerator::ConstructionCode(Code& code)
{
    code.NodeName().Function("AppendColumn(").QuotedString(prop_label).Comma();
    if (code.IntValue(prop_width) == -2)
    {
        code.Add("wxCOL_WIDTH_AUTOSIZE");
    }
    else
    {
        code.itoa(prop_width);
    }

    if (code.view(prop_alignment) != "wxALIGN_LEFT" || code.view(prop_flags) != "wxCOL_RESIZABLE")
    {
        code.Comma().Add(prop_alignment).Comma();
        if (code.view(prop_flags).size())
        {
            code.Add(prop_flags);
        }
        else
        {
            code += "0";
        }
    }

    code.EndFunction();

    return true;
}
