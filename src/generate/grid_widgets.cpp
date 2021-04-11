/////////////////////////////////////////////////////////////////////////////
// Purpose:   Grid component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>              // Event classes
#include <wx/grid.h>               // wxGrid base header
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "grid_widgets.h"

//////////////////////////////////////////  GridGenerator  //////////////////////////////////////////

wxObject* GridGenerator::Create(Node* node, wxObject* parent)
{
    auto grid = new wxGrid(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                           node->prop_as_wxSize("size"), node->prop_as_int("window_style"));

    grid->CreateGrid(node->prop_as_int("rows"), node->prop_as_int("cols"));

    grid->EnableDragColMove(node->prop_as_int("drag_col_move") != 0);
    grid->EnableDragColSize(node->prop_as_int("drag_col_size") != 0);
    grid->EnableDragGridSize(node->prop_as_int("drag_grid_size") != 0);
    grid->EnableDragRowSize(node->prop_as_int("drag_row_size") != 0);
    grid->EnableEditing(node->prop_as_int("editing") != 0);
    grid->EnableGridLines(node->prop_as_int("grid_lines") != 0);
    if (node->HasValue("grid_line_color"))
    {
        grid->SetGridLineColour(node->prop_as_wxColour("grid_line_color"));
    }
    grid->SetMargins(node->prop_as_int("margin_width"), node->prop_as_int("margin_height"));

    // Label Properties
    grid->SetColLabelAlignment(node->prop_as_int("col_label_horiz_alignment"),
                               node->prop_as_int("col_label_vert_alignment"));
    grid->SetColLabelSize(node->prop_as_int("col_label_size"));

    wxArrayString columnLabels = node->prop_as_wxArrayString("col_label_values");
    for (int i = 0; i < (int) columnLabels.size() && i < grid->GetNumberCols(); ++i)
    {
        grid->SetColLabelValue(i, columnLabels[i]);
    }

#if 0
    wxArrayInt columnSizes = node->prop_as_wxArrayInt("column_sizes");
    for (int i = 0; i < (int) columnSizes.size() && i < grid->GetNumberCols(); ++i)
    {
        grid->SetColSize(i, columnSizes[i]);
    }
#endif

    grid->SetRowLabelAlignment(node->prop_as_int("row_label_horiz_alignment"),
                               node->prop_as_int("row_label_vert_alignment"));
    grid->SetRowLabelSize(node->prop_as_int("row_label_size"));

    wxArrayString rowLabels = node->prop_as_wxArrayString("row_label_values");
    for (int i = 0; i < (int) rowLabels.size() && i < grid->GetNumberRows(); ++i)
    {
        grid->SetRowLabelValue(i, rowLabels[i]);
    }

#if 0
    wxArrayInt rowSizes = node->prop_as_wxArrayInt("row_sizes");
    for (int i = 0; i < (int) rowSizes.size() && i < grid->GetNumberRows(); ++i)
    {
        grid->SetRowSize(i, rowSizes[i]);
    }
#endif

    if (node->HasValue("label_bg"))
    {
        grid->SetLabelBackgroundColour(node->prop_as_wxColour("label_bg"));
    }
    if (node->HasValue("label_text"))
    {
        grid->SetLabelTextColour(node->prop_as_wxColour("label_text"));
    }
    if (node->HasValue("label_font"))
    {
        grid->SetLabelFont(node->prop_as_font("label_font"));
    }

    // Default Cell Properties
    grid->SetDefaultCellAlignment(node->prop_as_int("cell_horiz_alignment"), node->prop_as_int("cell_vert_alignment"));

    if (node->HasValue("cell_bg"))
    {
        grid->SetDefaultCellBackgroundColour(node->prop_as_wxColour("cell_bg"));
    }
    if (node->HasValue("cell_text"))
    {
        grid->SetDefaultCellTextColour(node->prop_as_wxColour("cell_text"));
    }
    if (node->HasValue("cell_font"))
    {
        grid->SetDefaultCellFont(node->prop_as_font("cell_font"));
    }

    // Example Cell Values
    for (int col = 0; col < grid->GetNumberCols(); ++col)
    {
        for (int row = 0; row < grid->GetNumberRows(); ++row)
        {
            grid->SetCellValue(row, col, grid->GetColLabelValue(col) + "-" + grid->GetRowLabelValue(row));
        }
    }

    if (node->prop_as_int("autosize_rows") != 0)
    {
        grid->AutoSizeRows();
    }
    if (node->prop_as_int("autosize_cols") != 0)
    {
        grid->AutoSizeColumns();
    }

    grid->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return grid;
}

std::optional<ttlib::cstr> GridGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxGrid(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, false, " wxWANTS_CHARS", " wxWANTS_CHARS");

    return code;
}

std::optional<ttlib::cstr> GridGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    auto_indent = false;
    code << "    {";

    code << "\n        " << node->get_node_name() << "->CreateGrid(" << node->prop_as_string("rows") << ", "
         << node->prop_as_string("cols") << ");";
    code << "\n        " << node->get_node_name() << "->EnableEditing(" << (node->prop_as_bool("editing") ? "true" : "false")
         << ");";
    code << "\n        " << node->get_node_name() << "->EnableGridLines("
         << (node->prop_as_bool("grid_lines") ? "true" : "false") << ");";
    code << "\n        " << node->get_node_name() << "->EnableDragGridSize("
         << (node->prop_as_bool("drag_grid_size") ? "true" : "false") << ");";

    if (node->HasValue("grid_line_color"))
        code << "\n        " << node->get_node_name() << "->SetGridLineColour(" << GenerateColorCode(node, "grid_line_color")
             << ");";
    code << "\n        " << node->get_node_name() << "->SetMargins(" << node->prop_as_string("margin_width") << ", "
         << node->prop_as_string("margin_height") << ");";

    code << '\n';

    if (node->HasValue("column_sizes"))
    {
        ttlib::multistr sizes(node->prop_as_string("column_sizes"), ',');
        for (size_t pos = 0; pos < sizes.size(); ++pos)
        {
            code << "\n        " << node->get_node_name() << "->SetColSize(" << pos << ", " << sizes[pos] << ");";
        }
    }

    if (node->prop_as_bool("autosize_cols"))
        code << "\n        " << node->get_node_name() << "->AutoSizeColumns();";
    code << "\n        " << node->get_node_name() << "->EnableDragColMove("
         << (node->prop_as_bool("drag_col_move") ? "true" : "false") << ");";
    code << "\n        " << node->get_node_name() << "->EnableDragColSize("
         << (node->prop_as_bool("drag_col_size") ? "true" : "false") << ");";
    code << "\n        " << node->get_node_name() << "->SetColLabelSize(" << node->prop_as_string("col_label_size") << ");";
    code << "\n        " << node->get_node_name() << "->SetColLabelAlignment("
         << node->prop_as_string("col_label_horiz_alignment") << ", " << node->prop_as_string("col_label_vert_alignment")
         << ");";

    if (node->HasValue("col_label_values"))
    {
        // TODO: [KeyWorks - 12-14-2020] This needs to be reworked
    }

    code << '\n';

    if (node->HasValue("row_sizes"))
    {
        ttlib::multistr sizes(node->prop_as_string("row_sizes"), ',');
        for (size_t pos = 0; pos < sizes.size(); ++pos)
        {
            code << "\n        " << node->get_node_name() << "->SetRowSize(" << pos << ", " << sizes[pos] << ");";
        }
    }
    if (node->prop_as_bool("autosize_rows"))
        code << "\n        " << node->get_node_name() << "->AutoSizeRows();";
    code << "\n        " << node->get_node_name() << "->EnableDragRowSize("
         << (node->prop_as_bool("row_label_size") ? "true" : "false") << ");";
    code << "\n        " << node->get_node_name() << "->SetRowLabelAlignment("
         << node->prop_as_string("row_label_horiz_alignment") << ", " << node->prop_as_string("row_label_vert_alignment")
         << ");";

    if (node->HasValue("row_label_values"))
    {
        // TODO: [KeyWorks - 12-14-2020] This needs to be reworked
    }

    if (node->HasValue("label_bg"))
        code << "\n        " << node->get_node_name() << "->SetLabelBackgroundColour(" << GenerateColorCode(node, "label_bg")
             << ");";

        // TODO: [KeyWorks - 02-27-2021] GenerateFontCode() was removed because it was obsolete and broken. It needs to be
        // replaced, but it should be part of an entire wxGrid overhaul.

#if 0
    if (node->HasValue("label_font"))
        code << "\n        " << node->get_node_name() << "->SetLabelFont(" << GenerateFontCode(node, "label_font") << ");";
#endif
    if (node->HasValue("label_text"))
        code << "\n        " << node->get_node_name() << "->SetLabelTextColour(" << GenerateColorCode(node, "label_text")
             << ");";

    if (node->HasValue("cell_bg"))
        code << "\n        " << node->get_node_name() << "->SetDefaultCellBackgroundColour("
             << GenerateColorCode(node, "cell_bg") << ");";
#if 0
    if (node->HasValue("cell_font"))
        code << "\n        " << node->get_node_name() << "->SetDefaultCellFont(" << GenerateFontCode(node, "cell_font")
             << ");";
#endif
    if (node->HasValue("cell_font"))
        code << "\n        " << node->get_node_name() << "->SetDefaultCellFont(" << GenerateColorCode(node, "cell_font")
             << ");";

    code << "\n        " << node->get_node_name() << "->SetDefaultCellAlignment("
         << node->prop_as_string("cell_horiz_alignment") << ", " << node->prop_as_string("cell_vert_alignment") << ");";

    code << "\n    }";

    return code;
}

std::optional<ttlib::cstr> GridGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool GridGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/grid.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  PropertyGridGenerator  //////////////////////////////////////////

wxObject* PropertyGridGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxPropertyGrid(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                           node->prop_as_wxSize("size"), node->prop_as_int(prop_style) | node->prop_as_int("window_style"));

    if (!node->GetPropertyAsString("extra_style").empty())
    {
        widget->SetExtraStyle(node->prop_as_int("extra_style"));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void PropertyGridGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */)
{
    auto pg = wxStaticCast(wxobject, wxPropertyGrid);
    auto node = GetMockup()->GetNode(wxobject);
    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->GetClassName() == "propGridItem")
        {
            if (childObj->prop_as_string("type") == "Category")
            {
                pg->Append(
                    new wxPropertyCategory(childObj->prop_as_wxString(prop_label), childObj->prop_as_wxString(prop_label)));
            }
            else
            {
                wxPGProperty* prop = wxDynamicCast(
                    wxCreateDynamicObject("wx" + (childObj->GetPropertyAsString("type")) + "Property"), wxPGProperty);
                if (prop)
                {
                    prop->SetLabel(childObj->prop_as_wxString(prop_label));
                    prop->SetName(childObj->prop_as_wxString(prop_label));
                    pg->Append(prop);

                    if (childObj->GetPropertyAsString("help") != wxEmptyString)
                    {
                        pg->SetPropertyHelpString(prop, childObj->GetPropertyAsString("help"));
                    }
                }
            }
        }
    }
}

std::optional<ttlib::cstr> PropertyGridGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxPropertyGrid(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, false, "wxPG_DEFAULT_STYLE", "wxPG_DEFAULT_STYLE");

    code.Replace(", wxID_ANY);", ");");

    if (node->HasValue("extra_style"))
        code << "\n    " << node->get_node_name() << "->SetExtraStyle(" << node->prop_as_string("extra_style") << ")";

    return code;
}

std::optional<ttlib::cstr> PropertyGridGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool PropertyGridGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/propgrid/propgrid.h>", set_src, set_hdr);

    if (node->prop_as_bool("include_advanced"))
        InsertGeneratorInclude(node, "#include <wx/propgrid/advprops.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  PropertyGridManagerGenerator  //////////////////////////////////////////

wxObject* PropertyGridManagerGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxPropertyGridManager(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                                            node->prop_as_wxSize("size"),
                                            node->prop_as_int(prop_style) | node->prop_as_int("window_style"));

    if (!node->GetPropertyAsString("extra_style").empty())
    {
        widget->SetExtraStyle(node->prop_as_int("extra_style"));
    }

    widget->ShowHeader(node->prop_as_int("show_header") != 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void PropertyGridManagerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */)
{
    auto pgm = wxStaticCast(wxobject, wxPropertyGridManager);

    auto node = GetMockup()->GetNode(wxobject);
    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->GetClassName() == "propGridPage")
        {
            wxPropertyGridPage* page =
                pgm->AddPage(childObj->prop_as_wxString(prop_label), childObj->prop_as_wxBitmap("bitmap"));

            for (size_t j = 0; j < childObj->GetChildCount(); ++j)
            {
                auto innerChildObj = childObj->GetChild(j);
                if (innerChildObj->GetClassName() == "propGridItem")
                {
                    if (innerChildObj->GetPropertyAsString("type") == "Category")
                    {
                        page->Append(new wxPropertyCategory(innerChildObj->prop_as_wxString(prop_label),
                                                            innerChildObj->prop_as_wxString(prop_label)));
                    }
                    else
                    {
                        wxPGProperty* prop = wxDynamicCast(
                            wxCreateDynamicObject("wx" + (innerChildObj->GetPropertyAsString("type")) + "Property"),
                            wxPGProperty);
                        if (prop)
                        {
                            prop->SetLabel(innerChildObj->prop_as_wxString(prop_label));
                            prop->SetName(innerChildObj->prop_as_wxString(prop_label));
                            page->Append(prop);

                            if (innerChildObj->GetPropertyAsString("help") != wxEmptyString)
                            {
                                page->SetPropertyHelpString(prop, innerChildObj->GetPropertyAsString("help"));
                            }
                        }
                    }
                }
            }
        }
    }

    if (count)
    {
        pgm->SelectPage(0);
    }

    pgm->Update();
}

std::optional<ttlib::cstr> PropertyGridManagerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxPropertyGridManager(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, false, "wxPGMAN_DEFAULT_STYLE", "wxPGMAN_DEFAULT_STYLE");

    code.Replace(", wxID_ANY);", ");");

    if (node->HasValue("extra_style"))
        code << "\n    " << node->get_node_name() << "->SetExtraStyle(" << node->prop_as_string("extra_style") << ")";

    return code;
}

std::optional<ttlib::cstr> PropertyGridManagerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool PropertyGridManagerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/propgrid/propgrid.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/propgrid/manager.h>", set_src, set_hdr);

    if (node->prop_as_bool("include_advanced"))
        InsertGeneratorInclude(node, "#include <wx/propgrid/advprops.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  PropertyGridItemGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> PropertyGridItemGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = " << node->get_parent_name();

    if (node->prop_as_string("type") == "Category")
    {
        code << "->Append(new wxPropertyCategory(";
        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
             << GenerateQuotedString(node->prop_as_string(prop_label)) << ");";
    }
    else
    {
        code << "->Append(new wx" << node->prop_as_string("type") << "Property(";
        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
             << GenerateQuotedString(node->prop_as_string("help")) << ");";
    }

    return code;
}

//////////////////////////////////////////  PropertyGridPageGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> PropertyGridPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = " << node->get_parent_name() << "->AddPage(";
    code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
         << GenerateBitmapCode(node->prop_as_string("bitmap")) << ");";

    return code;
}
