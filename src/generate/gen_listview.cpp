/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxListView generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/listctrl.h>  // wxListCtrl class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties
#include "write_code.h"  // WriteCode -- Write code to Scintilla or file

#include "gen_listview.h"

wxObject* ListViewGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxListView(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->prop_as_string(prop_mode) == "wxLC_REPORT" && node->HasValue(prop_column_labels))
    {
        auto headers = ConvertToArrayString(node->prop_as_string(prop_column_labels));
        for (auto& label: headers)
            widget->AppendColumn(label.wx_str());

        if (node->HasValue(prop_contents))
        {
            wxListItem info;
            info.Clear();

            auto strings = ConvertToArrayString(node->prop_as_string(prop_contents));
            long row_id = -1;
            for (auto& row: strings)
            {
                info.SetId(++row_id);
                auto index = widget->InsertItem(info);
                ttlib::multistr columns(row, ';', tt::TRIM::both);
                for (size_t column = 0; column < columns.size() && column < headers.size(); ++column)
                {
                    widget->SetItem(index, (to_int) column, columns[column].wx_str());
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
        if (node->HasValue(prop_contents))
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
        if (node->HasValue(prop_contents))
        {
            code << "\n\n"
                 << "\twxListItem info;\n"
                 << "\tinfo.Clear();\n\n";
            auto strings = ConvertToArrayString(node->prop_as_string(prop_contents));
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
