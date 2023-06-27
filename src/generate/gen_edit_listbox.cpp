/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxEditableListBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/editlbox.h>  // ListBox with editable items

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_edit_listbox.h"

wxObject* EditListBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxEditableListBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                              DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_contents))
    {
        auto array = node->as_wxArrayString(prop_auto_complete);
        widget->SetStrings(array);
    }

    // We don't bind with left-click since the control itself will use it and not pass it to us.

    return widget;
}

bool EditListBoxGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
    code.PosSizeFlags(true);

    return true;
}

bool EditListBoxGenerator::SettingsCode(Code& code)
{
    if (code.HasValue(prop_contents))
    {
        auto array = code.node()->as_ArrayString(prop_contents);
        if (code.is_cpp())
        {
            code.OpenBrace().Str("wxArrayString tmp_array;");
            for (auto& iter: array)
            {
                code.Eol().Str("tmp_array.push_back(wxString::FromUTF8(\"") << iter << "\"));";
            }
            code.Eol().NodeName() += "->SetStrings(tmp_array);";
            code.CloseBrace();
        }
        else
        {
            code.NodeName().Function("SetStrings([");
            for (auto& iter: array)
            {
                code.Eol().Tab().QuotedString(iter) += ",";
            }
            code.GetCode().pop_back();  // remove trailing comma
            code.Eol().Tab().Str("]").EndFunction();
        }
    }

    return true;
}

bool EditListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/editlbox.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/listbase.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_editlbox.cpp
// ../../../wxWidgets/src/xrc/xh_editlbox.cpp

int EditListBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxEditableListBox");

    ADD_ITEM_PROP(prop_label, "label")

    if (node->HasValue(prop_contents))
    {
        auto content = item.append_child("content");
        auto array = node->as_ArrayString(prop_contents);
        for (auto& iter: array)
        {
            content.append_child("item").text().set(iter);
        }
    }

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void EditListBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxEditableListBoxXmlHandler");
}
