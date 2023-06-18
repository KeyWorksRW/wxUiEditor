/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxListView generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
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
            widget->AppendColumn(label.make_wxString());

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
                tt_string_vector columns(row, ';', tt::TRIM::both);
                for (size_t column = 0; column < columns.size() && column < headers.size(); ++column)
                {
                    widget->SetItem(index, (to_int) column, columns[column].make_wxString());
                }
            }
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ListViewGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().Add(prop_id);

    // Note that the default style is not specified, so that it will always be generated. That makes the generated code
    // easier to understand since you know exactly which type of list view is being created instead of having to know what
    // the default is.
    code.PosSizeFlags(true);

    return true;
}

bool ListViewGenerator::SettingsCode(Code& code)
{
    if (code.is_value(prop_mode, "wxLC_REPORT") && code.HasValue(prop_column_labels))
    {
        if (code.HasValue(prop_contents))
        {
            code.OpenBrace();
        }

        auto headers = ConvertToArrayString(code.view(prop_column_labels));
        for (auto& iter: headers)
        {
            code.Eol(eol_if_needed).NodeName().Function("AppendColumn(").QuotedString(iter).EndFunction();
        }

        if (code.HasValue(prop_contents))
        {
            code.Eol(eol_if_needed);
            if (code.is_cpp())
                code.Str("auto ");
            code.Str("info = ").Add("wxListItem(").EndFunction();
            code.Eol().Str("info.Clear(").EndFunction();
            auto strings = ConvertToArrayString(code.view(prop_contents));
            int row_id = -1;
            for (auto& row: strings)
            {
                ++row_id;
                code.Eol().Str("info.SetId(").itoa(row_id).EndFunction();
                if (row_id == 0)
                    code.Eol().Str(code.is_cpp() ? "auto " : "").Str("idx = ");
                else
                    code.Eol().Str("idx = ");
                code.NodeName().Function("InsertItem(info").EndFunction();
                tt_string_vector columns(row, ';', tt::TRIM::both);
                for (size_t column = 0; column < columns.size() && column < headers.size(); ++column)
                {
                    code.Eol().NodeName().Function("SetItem(idx").Comma().itoa(column);
                    code.Comma().QuotedString(columns[column]).EndFunction();
                }
            }
            code.CloseBrace();
        }
    }

    return true;
}

bool ListViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/listctrl.h>", set_src, set_hdr);
    return true;
}

int ListViewGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    // XRC doesn't support wxListView
    GenXrcObjectAttributes(node, item, "wxListCtrl");

    GenXrcStylePosSize(node, item, prop_mode);
    GenXrcWindowSettings(node, item);

    auto headers = ConvertToArrayString(node->value(prop_column_labels));
    for (auto& iter: headers)
    {
        auto child = item.append_child("object");
        child.append_attribute("class").set_value("listcol");
        auto text = child.append_child("text");
        text.text().set(iter);
    }

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ListViewGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxListCtrlXmlHandler");
}
