/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGrid generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/grid.h>  // wxGrid base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_grid.h"

wxObject* GridGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto grid = new wxGrid(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                           DlgSize(parent, node, prop_size), GetStyleInt(node));

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
    if (node->prop_as_string(prop_cell_fit) != "overflow")
    {
        if (node->prop_as_string(prop_cell_fit) == "clip")
            grid->SetDefaultCellFitMode(wxGridFitMode::Clip());
        else if (node->prop_as_string(prop_cell_fit) == "ellipsize")
            grid->SetDefaultCellFitMode(wxGridFitMode::Ellipsize());
    }

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

bool GridGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().Add(prop_id);
    code.PosSizeFlags(false, "wxWANTS_CHARS");

    return true;
}

bool GridGenerator::SettingsCode(Code& code)
{
    code.OpenBrace().NodeName().Function("CreateGrid(").Str(prop_rows).Comma().Str(prop_cols).EndFunction();

    if (code.IsFalse(prop_editing))
        code.Eol().NodeName().Function("EnableEditing(").AddFalse().EndFunction();
    if (code.IsFalse(prop_grid_lines))
        code.Eol().NodeName().Function("EnableGridLines(").AddFalse().EndFunction();
    if (code.HasValue(prop_grid_line_color))
        code.Eol().NodeName().Function("SetGridLineColour(").ColourCode(prop_grid_line_color).EndFunction();

    code.Eol().NodeName().Function("EnableDragGridSize(").TrueFalseIf(prop_drag_grid_size).EndFunction();
    code.Eol().NodeName().Function("SetMargins(").itoa(prop_margin_width, prop_margin_height).EndFunction();

    if (!code.is_value(prop_cell_fit, "overflow"))
    {
        if (code.is_value(prop_cell_fit, "clip"))
        {
            if (code.is_cpp() && wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code.Eol().Str("#if wxCHECK_VERSION(3, 1, 4)");
                code.Eol().Tab().NodeName().Function("SetDefaultCellFitMode(");
                code.Add("wxGridFitMode").ClassMethod("Clip()").EndFunction();
                code.Eol().Str("#endif");
            }
            else
            {
                code.Eol().NodeName().Function("SetDefaultCellFitMode(");
                code.Add("wxGridFitMode").ClassMethod("Clip()").EndFunction();
            }
        }
        else if (code.is_value(prop_cell_fit, "ellipsize"))
        {
            if (code.is_cpp() && wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code.Eol().Str("#if wxCHECK_VERSION(3, 1, 4)");
                code.Eol().Tab().NodeName().Function("SetDefaultCellFitMode(");
                code.Add("wxGridFitMode").ClassMethod("Ellipsize()").EndFunction();
                code.Eol().Str("#endif");
            }
            else
            {
                code.Eol().NodeName().Function("SetDefaultCellFitMode(");
                code.Add("wxGridFitMode").ClassMethod("Ellipsize()").EndFunction();
            }
        }
    }

    if (code.IntValue(prop_selection_mode) != 0)
    {
        if (code.is_cpp() && wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            if (code.is_value(prop_selection_mode, "wxGridSelectNone"))
            {
                if (code.is_cpp() && wxGetProject().value(prop_wxWidgets_version) == "3.1")
                {
                    code.Eol().Str("#if wxCHECK_VERSION(3, 1, 5)");
                    code.Eol().Tab().NodeName().Function("SetSelectionMode(");
                    code.Add("wxGrid").ClassMethod(code.view(prop_selection_mode)).EndFunction();
                    code.Eol().Str("#endif");
                }
                else
                {
                    code.Eol().NodeName().Function("SetSelectionMode(");
                    code.Add("wxGrid").ClassMethod(code.view(prop_selection_mode)).EndFunction();
                }
            }
            else
            {
                code.Eol().NodeName().Function("SetSelectionMode(");
                code.Add("wxGrid").ClassMethod(code.view(prop_selection_mode)).EndFunction();
            }
        }
        else
        {
            code.Eol().NodeName().Function("SetSelectionMode(");
            code.Add("wxGrid").ClassMethod(code.view(prop_selection_mode)).EndFunction();
        }
    }

    // Label category

    if (code.IsTrue(prop_native_col_header))
        code.Eol().NodeName().Function("UseNativeColHeader(").EndFunction();
    else if (code.IsTrue(prop_native_col_labels))
        code.Eol().NodeName().Function("SetUseNativeColLabels(").EndFunction();

    if (code.HasValue(prop_label_bg))
    {
        code.Eol().NodeName().Function("SetLabelBackgroundColour(").ColourCode(prop_label_bg).EndFunction();
    }

    // TODO: [KeyWorks - 02-27-2021] GenerateFontCode() was removed because it was obsolete and broken. It needs to
    // be replaced, but it should be part of an entire wxGrid overhaul.

#if 0
    if (node->HasValue(prop_label_font))
        code << braced_indent << node->get_node_name() << "->SetLabelFont(" << GenerateFontCode(node, "label_font") << ");";
#endif
    if (code.HasValue(prop_label_text))
        code.Eol().NodeName().Function("SetLabelTextColour(").ColourCode(prop_label_text).EndFunction();

    // Cell category

    if (code.HasValue(prop_cell_bg))
        code.Eol().NodeName().Function("SetDefaultCellBackgroundColour(").ColourCode(prop_cell_bg).EndFunction();
    if (code.HasValue(prop_cell_text))
        code.Eol().NodeName().Function("SetDefaultCellTextColour(").ColourCode(prop_cell_text).EndFunction();

#if 0
    if (node->HasValue(prop_cell_font))
        code << braced_indent << node->get_node_name() << "->SetDefaultCellFont(" << GenerateFontCode(node, "cell_font")
             << ");";
#endif

    code.Eol()
        .NodeName()
        .Function("SetDefaultCellAlignment(")
        .itoa(prop_cell_horiz_alignment, prop_cell_vert_alignment)
        .EndFunction();

    // Columns category

    if (code.IntValue(prop_default_col_size) > 0)
    {
        code.Eol().NodeName().Function("SetDefaultColSize(").Str(prop_default_col_size).EndFunction();
    }
    else if (code.IsTrue(prop_autosize_cols))
    {
        code.Eol().NodeName().Function("AutoSizeColumns(").EndFunction();
    }

    if (code.IsTrue(prop_drag_col_move))
        code.Eol().NodeName().Function("EnableDragColMove(").AddTrue().EndFunction();

    if (code.IsFalse(prop_drag_col_size))
        code.Eol().NodeName().Function("EnableDragColSize(").AddFalse().EndFunction();

    code.Eol().NodeName().Function("SetColLabelAlignment(");
    code.itoa(prop_col_label_horiz_alignment, prop_col_label_vert_alignment).EndFunction();

    if (code.IntValue(prop_col_label_size) == -1)
        code.Eol().NodeName().Function("SetColLabelSize(").Add("wxGRID_AUTOSIZE").EndFunction();
    else if (code.IntValue(prop_col_label_size) == 0)
        code.Eol().NodeName().Function("HideColLabels(").EndFunction();
    else
        code.Eol().NodeName().Function("SetColLabelSize(").Str(prop_col_label_size).EndFunction();

    if (code.HasValue(prop_col_label_values))
    {
        auto labels = code.node()->as_wxArrayString(prop_col_label_values);
        int num_cols = code.IntValue(prop_cols);
        for (int col = 0; col < (int) labels.size() && col < num_cols; ++col)
        {
            code.Eol().NodeName().Function("SetColLabelValue(").itoa(col);
            code.Comma().QuotedString(ttlib::cstr() << labels[col].wx_str()).EndFunction();
        }
    }

    // Rows category

    if (code.IntValue(prop_default_row_size) > 0)
    {
        code.Eol().NodeName().Function("SetDefaultRowSize(").Str(prop_default_row_size).EndFunction();
    }
    else if (code.IsTrue(prop_autosize_rows))
    {
        code.Eol().NodeName().Function("AutoSizeRows(").EndFunction();
    }

    if (code.IsFalse(prop_drag_row_size))
        code.Eol().NodeName().Function("EnableDragRowSize(").AddFalse().EndFunction();

    code.Eol().NodeName().Function("SetRowLabelAlignment(");
    code.itoa(prop_row_label_horiz_alignment, prop_row_label_vert_alignment).EndFunction();

    if (code.IntValue(prop_row_label_size) == -1)
        code.Eol().NodeName().Function("SetRowLabelSize(").Add("wxGRID_AUTOSIZE").EndFunction();
    else if (code.IntValue(prop_row_label_size) == 0)
        code.Eol().NodeName().Function("HideRowLabels(").EndFunction();
    else
        code.Eol().NodeName().Function("SetRowLabelSize(").Str(prop_row_label_size).EndFunction();

    if (code.HasValue(prop_col_label_values))
    {
        auto labels = code.node()->as_wxArrayString(prop_col_label_values);
        int num_cols = code.IntValue(prop_cols);
        for (int col = 0; col < (int) labels.size() && col < num_cols; ++col)
        {
            code.Eol().NodeName().Function("SetColLabelValue(").itoa(col);
            code.Comma().QuotedString(ttlib::cstr() << labels[col].wx_str()).EndFunction();
        }
    }

    code.CloseBrace();

    return true;
}

int GridGenerator::GetRequiredVersion(Node* node)
{
    // Code generation was invalid in minRequiredVer, so a newer version is required if this property is set.
    if (node->prop_as_int(prop_selection_mode) != 0)
        return minRequiredVer + 1;

    // There was no code generation for a non-default setting in minRequiredVer
    if (node->prop_as_int(prop_row_label_size) != -1)
        return minRequiredVer + 1;

    return minRequiredVer;
}

bool GridGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/grid.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp

int GridGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxGrid");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        ADD_ITEM_COMMENT(" XRC doesn't support any properties for wxGrid. ")
        GenXrcComments(node, item);
    }

    return result;
}

void GridGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxGridXmlHandler");
}
