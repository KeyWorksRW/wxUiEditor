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

wxObject* GridGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto grid = new wxGrid(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                           node->prop_as_wxSize(prop_size), node->prop_as_int(prop_window_style));

    wxGridUpdateLocker prevent_updates(grid);

    if (node->prop_as_int(prop_default_row_size) > 0)
        grid->SetDefaultRowSize(node->prop_as_int(prop_default_row_size));
    if (node->prop_as_int(prop_default_col_size) > 0)
        grid->SetDefaultColSize(node->prop_as_int(prop_default_col_size));

    grid->CreateGrid(node->prop_as_int(prop_rows), node->prop_as_int(prop_cols));

    // Grid category
    grid->EnableGridLines(node->prop_as_bool(prop_grid_lines));
    if (node->HasValue(prop_grid_line_color))
    {
        grid->SetGridLineColour(node->prop_as_wxColour(prop_grid_line_color));
    }
    grid->EnableDragGridSize(node->prop_as_bool(prop_drag_grid_size));
    grid->SetMargins(node->prop_as_int(prop_margin_width), node->prop_as_int(prop_margin_height));

    if (node->prop_as_int(prop_selection_mode) != 0)
        grid->SetSelectionMode(static_cast<wxGrid::wxGridSelectionModes>(node->prop_as_int(prop_selection_mode)));

    // Label category
    if (node->prop_as_bool(prop_native_col_header))
        grid->UseNativeColHeader();
    else if (node->prop_as_bool(prop_native_col_labels))
        grid->SetUseNativeColLabels();

    grid->SetColLabelAlignment(node->prop_as_int(prop_col_label_horiz_alignment),
                               node->prop_as_int(prop_col_label_vert_alignment));
    grid->SetColLabelSize(node->prop_as_int(prop_col_label_size));

    if (node->HasValue(prop_label_bg))
    {
        grid->SetLabelBackgroundColour(node->prop_as_wxColour(prop_label_bg));
    }
    if (node->HasValue(prop_label_text))
    {
        grid->SetLabelTextColour(node->prop_as_wxColour(prop_label_text));
    }
    if (node->HasValue(prop_label_font))
    {
        grid->SetLabelFont(node->prop_as_font(prop_label_font));
    }
    // Columns category

    grid->EnableDragColMove(node->prop_as_bool(prop_drag_col_move));
    grid->EnableDragColSize(node->prop_as_bool(prop_drag_col_size));

    wxArrayString columnLabels = node->prop_as_wxArrayString(prop_col_label_values);
    for (int i = 0; i < (int) columnLabels.size() && i < grid->GetNumberCols(); ++i)
    {
        grid->SetColLabelValue(i, columnLabels[i]);
    }

    // Rows category

    if (!node->prop_as_string(prop_tab_behaviour).is_sameas("Tab_Stop"))
        grid->SetTabBehaviour(static_cast<wxGrid::TabBehaviour>(node->prop_as_int(prop_tab_behaviour)));

    grid->SetRowLabelAlignment(node->prop_as_int(prop_row_label_horiz_alignment),
                               node->prop_as_int(prop_row_label_vert_alignment));
    grid->SetRowLabelSize(node->prop_as_int(prop_row_label_size));

    grid->EnableDragRowSize(node->prop_as_bool(prop_drag_row_size));

    wxArrayString labels = node->prop_as_wxArrayString(prop_row_label_values);
    for (int i = 0; i < (int) labels.size() && i < grid->GetNumberRows(); ++i)
    {
        grid->SetRowLabelValue(i, labels[i]);
    }


    // Cell Properties
    grid->SetDefaultCellAlignment(node->prop_as_int(prop_cell_horiz_alignment), node->prop_as_int(prop_cell_vert_alignment));

    if (node->HasValue(prop_cell_bg))
    {
        grid->SetDefaultCellBackgroundColour(node->prop_as_wxColour(prop_cell_bg));
    }
    if (node->HasValue(prop_cell_text))
    {
        grid->SetDefaultCellTextColour(node->prop_as_wxColour(prop_cell_text));
    }
    if (node->HasValue(prop_cell_font))
    {
        grid->SetDefaultCellFont(node->prop_as_font(prop_cell_font));
    }

    if (node->prop_as_int(prop_default_row_size) > 0)
        grid->SetDefaultRowSize(node->prop_as_int(prop_default_row_size));
    else if (node->prop_as_bool(prop_autosize_rows))
    {
        grid->AutoSizeRows();
    }

    if (node->prop_as_int(prop_default_col_size) > 0)
        grid->SetDefaultColSize(node->prop_as_int(prop_default_col_size));
    else if (node->prop_as_int(prop_autosize_cols))
    {
        grid->AutoSizeColumns();
    }

    grid->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return grid;
}

std::optional<ttlib::cstr> GridGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxGrid(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, " wxWANTS_CHARS", " wxWANTS_CHARS");

    return code;
}

static constexpr const char* braced_indent = "\n\t\t";

std::optional<ttlib::cstr> GridGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    auto_indent = false;
    code << "    {";

    code << braced_indent << node->get_node_name() << "->CreateGrid(" << node->prop_as_string(prop_rows) << ", "
         << node->prop_as_string(prop_cols) << ");";

    if (!node->prop_as_bool(prop_editing))
        code << braced_indent << node->get_node_name() << "->EnableEditing(false);";
    if (!node->prop_as_bool(prop_grid_lines))
        code << braced_indent << node->get_node_name() << "->EnableGridLines(false);";
    if (node->HasValue(prop_grid_line_color))
        code << braced_indent << node->get_node_name() << "->SetGridLineColour("
             << GenerateColorCode(node, prop_grid_line_color) << ");";

    code << braced_indent << node->get_node_name() << "->EnableDragGridSize("
         << (node->prop_as_bool(prop_drag_grid_size) ? "true" : "false") << ");";
    code << braced_indent << node->get_node_name() << "->SetMargins(" << node->prop_as_string(prop_margin_width) << ", "
         << node->prop_as_string(prop_margin_height) << ");";

    if (node->prop_as_int(prop_selection_mode) != 0)
        code << braced_indent << node->get_node_name() << "->SetSelectionMode(" << node->prop_as_string(prop_selection_mode)
             << ");";

    code << '\n';

    // Label category

    if (node->prop_as_bool(prop_native_col_header))
        code << braced_indent << node->get_node_name() << "->UseNativeColHeader();";
    else if (node->prop_as_bool(prop_native_col_labels))
        code << braced_indent << node->get_node_name() << "->SetUseNativeColLabels();";

    if (node->HasValue(prop_label_bg))
    {
        code << braced_indent << node->get_node_name() << "->SetLabelBackgroundColour("
             << GenerateColorCode(node, prop_label_bg) << ");";
    }

    // TODO: [KeyWorks - 02-27-2021] GenerateFontCode() was removed because it was obsolete and broken. It needs to be
    // replaced, but it should be part of an entire wxGrid overhaul.

#if 0
    if (node->HasValue(prop_label_font))
        code << braced_indent << node->get_node_name() << "->SetLabelFont(" << GenerateFontCode(node, "label_font") << ");";
#endif
    if (node->HasValue(prop_label_text))
        code << braced_indent << node->get_node_name() << "->SetLabelTextColour(" << GenerateColorCode(node, prop_label_text)
             << ");";

    // Cell category

    if (node->HasValue(prop_cell_bg))
        code << braced_indent << node->get_node_name() << "->SetDefaultCellBackgroundColour("
             << GenerateColorCode(node, prop_cell_bg) << ");";
    if (node->HasValue(prop_cell_text))
        code << braced_indent << node->get_node_name() << "->SetDefaultCellTextColour("
             << GenerateColorCode(node, prop_cell_text) << ");";
#if 0
    if (node->HasValue(prop_cell_font))
        code << braced_indent << node->get_node_name() << "->SetDefaultCellFont(" << GenerateFontCode(node, "cell_font")
             << ");";
#endif

    code << braced_indent << node->get_node_name() << "->SetDefaultCellAlignment("
         << node->prop_as_string(prop_cell_horiz_alignment) << ", " << node->prop_as_string(prop_cell_vert_alignment)
         << ");";

    // Columns category

    if (node->prop_as_int(prop_default_col_size) > 0)
    {
        code << braced_indent << node->get_node_name() << "->SetDefaultColSize("
             << node->prop_as_string(prop_default_col_size) << ");";
    }
    else if (node->prop_as_bool(prop_autosize_cols))
    {
        code << braced_indent << node->get_node_name() << "->AutoSizeColumns();";
    }

    if (node->prop_as_bool(prop_drag_col_move))
        code << braced_indent << node->get_node_name() << "->EnableDragColMove(true);";

    if (!node->prop_as_bool(prop_drag_col_size))
        code << braced_indent << node->get_node_name() << "->EnableDragColSize(false);";

    code << braced_indent << node->get_node_name() << "->SetColLabelSize(" << node->prop_as_string(prop_col_label_size)
         << ");";
    code << braced_indent << node->get_node_name() << "->SetColLabelAlignment("
         << node->prop_as_string(prop_col_label_horiz_alignment) << ", "
         << node->prop_as_string(prop_col_label_vert_alignment) << ");";

    if (node->HasValue(prop_col_label_values))
    {
        wxArrayString labels = node->prop_as_wxArrayString(prop_col_label_values);
        for (int i = 0; i < (int) labels.size() && i < node->prop_as_int(prop_cols); ++i)
        {
            code << braced_indent << node->get_node_name() << "->SetColLabelValue(" << i << ", ";
            code << GenerateQuotedString(ttlib::cstr() << labels[i].wx_str()) << ");";
        }
    }

    code << '\n';

    // Rows category

    if (node->prop_as_int(prop_default_row_size) > 0)
    {
        code << braced_indent << node->get_node_name() << "->SetDefaultRowSize("
             << node->prop_as_string(prop_default_row_size) << ");";
    }
    else if (node->prop_as_bool(prop_autosize_rows))
    {
        code << braced_indent << node->get_node_name() << "->AutoSizeRows();";
    }

    if (!node->prop_as_bool(prop_drag_row_size))
        code << braced_indent << node->get_node_name() << "->EnableDragRowSize(false);";

    code << braced_indent << node->get_node_name() << "->SetRowLabelAlignment("
         << node->prop_as_string(prop_row_label_horiz_alignment) << ", "
         << node->prop_as_string(prop_row_label_vert_alignment) << ");";

    if (node->HasValue(prop_row_label_values))
    {
        wxArrayString labels = node->prop_as_wxArrayString(prop_row_label_values);
        for (int i = 0; i < (int) labels.size() && i < node->prop_as_int(prop_rows); ++i)
        {
            code << braced_indent << node->get_node_name() << "->SetRowLabelValue(" << i << ", ";
            code << GenerateQuotedString(ttlib::cstr() << labels[i].wx_str()) << ");";
        }
    }

    code << "\n\t}";

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

wxObject* PropertyGridGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPropertyGrid(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                     node->prop_as_wxSize(prop_size),
                                     node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    if (node->HasValue(prop_extra_style))
    {
        widget->SetExtraStyle(node->prop_as_int(prop_extra_style));
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
        if (childObj->isGen(gen_propGridItem))
        {
            if (childObj->prop_as_string(prop_type) == "Category")
            {
                pg->Append(
                    new wxPropertyCategory(childObj->prop_as_wxString(prop_label), childObj->prop_as_wxString(prop_label)));
            }
            else
            {
                wxPGProperty* prop = wxDynamicCast(
                    wxCreateDynamicObject("wx" + (childObj->prop_as_string(prop_type)) + "Property"), wxPGProperty);
                if (prop)
                {
                    prop->SetLabel(childObj->prop_as_wxString(prop_label));
                    prop->SetName(childObj->prop_as_wxString(prop_label));
                    pg->Append(prop);

                    if (childObj->HasValue(prop_help))
                    {
                        pg->SetPropertyHelpString(prop, childObj->prop_as_wxString(prop_help));
                    }
                }
            }
        }
    }
}

std::optional<ttlib::cstr> PropertyGridGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxPropertyGrid(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxPG_DEFAULT_STYLE", "wxPG_DEFAULT_STYLE");

    code.Replace(", wxID_ANY);", ");");

    if (node->HasValue(prop_extra_style))
        code << "\n\t" << node->get_node_name() << "->SetExtraStyle(" << node->prop_as_string(prop_extra_style) << ")";

    return code;
}

std::optional<ttlib::cstr> PropertyGridGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool PropertyGridGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/propgrid/propgrid.h>", set_src, set_hdr);

    if (node->prop_as_bool(prop_include_advanced))
        InsertGeneratorInclude(node, "#include <wx/propgrid/advprops.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  PropertyGridManagerGenerator  //////////////////////////////////////////

wxObject* PropertyGridManagerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPropertyGridManager(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                            node->prop_as_wxSize(prop_size),
                                            node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    if (node->HasValue(prop_extra_style))
    {
        widget->SetExtraStyle(node->prop_as_int(prop_extra_style));
    }

    // BUGBUG: [KeyWorks - 04-11-2021] There is no "show_header" property
    // widget->ShowHeader(node->prop_as_int(prop_show_header) != 0);

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
        if (childObj->isGen(gen_propGridPage))
        {
            wxPropertyGridPage* page =
                pgm->AddPage(childObj->prop_as_wxString(prop_label), childObj->prop_as_wxBitmap(prop_bitmap));

            for (size_t j = 0; j < childObj->GetChildCount(); ++j)
            {
                auto innerChildObj = childObj->GetChild(j);
                if (innerChildObj->isGen(gen_propGridItem))
                {
                    if (innerChildObj->prop_as_string(prop_type) == "Category")
                    {
                        page->Append(new wxPropertyCategory(innerChildObj->prop_as_wxString(prop_label),
                                                            innerChildObj->prop_as_wxString(prop_label)));
                    }
                    else
                    {
                        wxPGProperty* prop = wxDynamicCast(
                            wxCreateDynamicObject("wx" + (innerChildObj->prop_as_string(prop_type)) + "Property"),
                            wxPGProperty);
                        if (prop)
                        {
                            prop->SetLabel(innerChildObj->prop_as_wxString(prop_label));
                            prop->SetName(innerChildObj->prop_as_wxString(prop_label));
                            page->Append(prop);

                            if (innerChildObj->HasValue(prop_help))
                            {
                                page->SetPropertyHelpString(prop, innerChildObj->prop_as_wxString(prop_help));
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
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxPropertyGridManager(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxPGMAN_DEFAULT_STYLE", "wxPGMAN_DEFAULT_STYLE");

    code.Replace(", wxID_ANY);", ");");

    if (node->HasValue(prop_extra_style))
        code << "\n\t" << node->get_node_name() << "->SetExtraStyle(" << node->prop_as_string(prop_extra_style) << ")";

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

    if (node->prop_as_bool(prop_include_advanced))
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

    if (node->prop_as_string(prop_type) == "Category")
    {
        code << "->Append(new wxPropertyCategory(";
        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
             << GenerateQuotedString(node->prop_as_string(prop_label)) << ");";
    }
    else
    {
        code << "->Append(new wx" << node->prop_as_string(prop_type) << "Property(";
        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
             << GenerateQuotedString(node->prop_as_string(prop_help)) << ");";
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
         << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";

    return code;
}
