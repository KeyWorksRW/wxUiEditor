/////////////////////////////////////////////////////////////////////////////
// Purpose:   ListCtrl component class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/editlbox.h>  // ListBox with editable items
#include <wx/event.h>     // Event classes
#include <wx/listctrl.h>  // wxSearchCtrlBase class

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties
#include "write_code.h"  // WriteCode -- Write code to Scintilla or file

#include "listctrl_widgets.h"

//////////////////////////////////////////  ListViewGenerator  //////////////////////////////////////////

wxObject* ListViewGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxListView(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->prop_as_string(prop_mode) == "wxLC_REPORT" && node->HasValue(prop_column_labels))
    {
        auto headers = ConvertToArrayString(node->prop_as_string(prop_column_labels));
        for (auto& label: headers)
            widget->AppendColumn(label.wx_str());

        if (node->HasValue(prop_strings))
        {
            wxListItem info;
            info.Clear();

            auto strings = ConvertToArrayString(node->prop_as_string(prop_strings));
            long row_id = -1;
            for (auto& row: strings)
            {
                info.SetId(++row_id);
                auto index = widget->InsertItem(info);
                ttlib::multistr columns(row, ';', tt::TRIM::both);
                for (size_t column = 0; column < columns.size() && column < headers.size(); ++column)
                {
                    widget->SetItem(index, static_cast<int>(column), columns[column].wx_str());
                }
            }
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ListViewGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    // Note that the default style is not specified, so that it will always be generated. That makes the generated code
    // easier to understand since you know exactly which type of list view is being created instead of having to know what
    // the default is.
    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> ListViewGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_mode) == "wxLC_REPORT" && node->HasValue(prop_column_labels))
    {
        if (node->HasValue(prop_strings))
        {
            auto_indent = indent::auto_keep_whitespace;
            code << "{";
        }
        auto headers = ConvertToArrayString(node->prop_as_string(prop_column_labels));
        for (auto& iter: headers)
        {
            if (code.size())
                code << "\n\t";
            code << node->get_node_name() << "->AppendColumn(" << GenerateQuotedString(iter) << ");";
        }
        if (node->HasValue(prop_strings))
        {
            code << "\n\n"
                 << "\twxListItem info;\n"
                 << "\tinfo.Clear();\n\n";
            auto strings = ConvertToArrayString(node->prop_as_string(prop_strings));
            int row_id = -1;
            for (auto& row: strings)
            {
                ++row_id;
                code << "\n\tinfo.SetId(" << row_id << ");\n";
                if (row_id == 0)
                    code << "\tauto index = ";
                else
                    code << "\tindex = ";
                code << node->get_node_name() << "->InsertItem(info);\n";
                ttlib::multistr columns(row, ';', tt::TRIM::both);
                for (size_t column = 0; column < columns.size() && column < headers.size(); ++column)
                {
                    code << '\t' << node->get_node_name() << "->SetItem(index, " << column << ", "
                         << GenerateQuotedString(columns[column]) << ");\n";
                }
            }
            code << "}";
        }
    }

    return code;
}

std::optional<ttlib::cstr> ListViewGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ListViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/listctrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  EditListBoxGenerator  //////////////////////////////////////////

wxObject* EditListBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxEditableListBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                              DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_strings))
    {
        auto array = ConvertToWxArrayString(node->prop_as_string(prop_strings));
        widget->SetStrings(array);
    }

    // We don't bind with left-click since the control itself will use it and not pass it to us.

    return widget;
}

std::optional<ttlib::cstr> EditListBoxGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    if (node->HasValue(prop_strings))
    {
        auto_indent = false;
        code << "\t{\n\t\twxArrayString tmp_array;\n";
        auto array = ConvertToArrayString(node->prop_as_string(prop_strings));
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
