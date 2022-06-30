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
        auto array = ConvertToWxArrayString(node->prop_as_string(prop_contents));
        widget->SetStrings(array);
    }

    // We don't bind with left-click since the control itself will use it and not pass it to us.

    return widget;
}

std::optional<ttlib::cstr> EditListBoxGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    if (node->HasValue(prop_contents))
    {
        auto_indent = false;
        code << "\t{\n\t\twxArrayString tmp_array;\n";
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
        for (auto& iter: array)
        {
            code << "\t\ttmp_array.push_back(wxString::FromUTF8(\"" << iter << "\"));\n";
        }
        code << "\t\t" << node->get_node_name() << "->SetStrings(tmp_array);\n";
        code << "\t}";
    }

    return code;
}

std::optional<ttlib::cstr> EditListBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->prop_as_string(prop_label).size())
        code << GenerateQuotedString(node->prop_as_string(prop_label));
    else
        code << "wxEmptyString";

    // Note that the default style is not specified, so that it will always be generated. That makes the generated code
    // easier to understand since you know exactly which type of list box is being created instead of having to know what
    // the default is.
    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> EditListBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
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
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
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
