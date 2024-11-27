/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGrid generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/grid.h>  // wxGrid base header

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties

#include "gen_grid.h"

wxObject* GridGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto grid = new wxGrid(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                           GetStyleInt(node));

    wxGridUpdateLocker prevent_updates(grid);

    if (node->as_int(prop_default_row_size) > 0)
        grid->SetDefaultRowSize(node->as_int(prop_default_row_size));
    if (node->as_int(prop_default_col_size) > 0)
        grid->SetDefaultColSize(node->as_int(prop_default_col_size));

    grid->CreateGrid(node->as_int(prop_rows), node->as_int(prop_cols));

    // Grid category
    grid->EnableGridLines(node->as_bool(prop_grid_lines));
    if (node->hasValue(prop_grid_line_color))
    {
        grid->SetGridLineColour(node->as_wxColour(prop_grid_line_color));
    }
    grid->EnableDragGridSize(node->as_bool(prop_drag_grid_size));
    grid->SetMargins(node->as_int(prop_margin_width), node->as_int(prop_margin_height));
    if (node->as_string(prop_cell_fit) != "overflow")
    {
        if (node->as_string(prop_cell_fit) == "clip")
            grid->SetDefaultCellFitMode(wxGridFitMode::Clip());
        else if (node->as_string(prop_cell_fit) == "ellipsize")
            grid->SetDefaultCellFitMode(wxGridFitMode::Ellipsize());
    }

    if (node->as_int(prop_selection_mode) != 0)
        grid->SetSelectionMode(static_cast<wxGrid::wxGridSelectionModes>(node->as_int(prop_selection_mode)));

    // Label category
    if (node->as_bool(prop_native_col_header))
        grid->UseNativeColHeader();
    else if (node->as_bool(prop_native_col_labels))
        grid->SetUseNativeColLabels();

    grid->SetColLabelAlignment(node->as_int(prop_col_label_horiz_alignment), node->as_int(prop_col_label_vert_alignment));
    grid->SetColLabelSize(node->as_int(prop_col_label_size));

    if (node->hasValue(prop_label_bg))
    {
        grid->SetLabelBackgroundColour(node->as_wxColour(prop_label_bg));
    }
    if (node->hasValue(prop_label_text))
    {
        grid->SetLabelTextColour(node->as_wxColour(prop_label_text));
    }
    if (node->hasValue(prop_label_font))
    {
        grid->SetLabelFont(node->as_wxFont(prop_label_font));
    }
    // Columns category

    grid->EnableDragColMove(node->as_bool(prop_drag_col_move));
    grid->EnableDragColSize(node->as_bool(prop_drag_col_size));

    if (node->hasValue(prop_column_sizes))
    {
        int index = 0;
        for (auto& iter: node->as_wxArrayString(prop_column_sizes))
        {
            grid->SetColSize(index++, wxAtoi(iter));
        }
    }

    if (node->hasValue(prop_col_label_values))
    {
        int index = 0;
        for (auto& iter: node->as_wxArrayString(prop_col_label_values))
        {
            grid->SetColLabelValue(index++, iter);
        }
    }

    // Rows category

    if (!node->as_string(prop_tab_behaviour).is_sameas("Tab_Stop"))
        grid->SetTabBehaviour(static_cast<wxGrid::TabBehaviour>(node->as_int(prop_tab_behaviour)));

    grid->SetRowLabelAlignment(node->as_int(prop_row_label_horiz_alignment), node->as_int(prop_row_label_vert_alignment));
    grid->SetRowLabelSize(node->as_int(prop_row_label_size));

    grid->EnableDragRowSize(node->as_bool(prop_drag_row_size));

    if (node->hasValue(prop_row_sizes))
    {
        int index = 0;
        for (auto& iter: node->as_wxArrayString(prop_row_sizes))
        {
            grid->SetRowSize(index++, wxAtoi(iter));
        }
    }

    if (node->hasValue(prop_row_label_values))
    {
        int index = 0;
        for (auto& iter: node->as_wxArrayString(prop_row_label_values))
        {
            grid->SetRowLabelValue(index++, iter);
        }
    }

    // Cell Properties
    grid->SetDefaultCellAlignment(node->as_int(prop_cell_horiz_alignment), node->as_int(prop_cell_vert_alignment));

    if (node->hasValue(prop_cell_bg))
    {
        grid->SetDefaultCellBackgroundColour(node->as_wxColour(prop_cell_bg));
    }
    if (node->hasValue(prop_cell_text))
    {
        grid->SetDefaultCellTextColour(node->as_wxColour(prop_cell_text));
    }
    if (node->hasValue(prop_cell_font))
    {
        grid->SetDefaultCellFont(node->as_wxFont(prop_cell_font));
    }

    if (node->as_int(prop_default_row_size) > 0)
        grid->SetDefaultRowSize(node->as_int(prop_default_row_size));
    else if (node->as_bool(prop_autosize_rows))
    {
        grid->AutoSizeRows();
    }

    if (node->as_int(prop_default_col_size) > 0)
        grid->SetDefaultColSize(node->as_int(prop_default_col_size));
    else if (node->as_int(prop_autosize_cols))
    {
        grid->AutoSizeColumns();
    }

    grid->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return grid;
}

bool GridGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, false, "wxWANTS_CHARS");

    return true;
}

bool GridGenerator::SettingsCode(Code& code)
{
    code.OpenBrace().NodeName().Function("CreateGrid(").as_string(prop_rows).Comma().as_string(prop_cols).EndFunction();

    if (code.IsFalse(prop_editing))
        code.Eol().NodeName().Function("EnableEditing(").False().EndFunction();
    if (code.IsFalse(prop_grid_lines))
        code.Eol().NodeName().Function("EnableGridLines(").False().EndFunction();
    if (code.hasValue(prop_grid_line_color))
        code.Eol().NodeName().Function("SetGridLineColour(").ColourCode(prop_grid_line_color).EndFunction();

    code.Eol().NodeName().Function("EnableDragGridSize(").TrueFalseIf(prop_drag_grid_size).EndFunction();
    code.Eol().NodeName().Function("SetMargins(").itoa(prop_margin_width, prop_margin_height).EndFunction();

    if (!code.isPropValue(prop_cell_fit, "overflow"))
    {
        if (code.isPropValue(prop_cell_fit, "clip"))
        {
            if (code.is_cpp() && Project.is_wxWidgets31())
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
        else if (code.isPropValue(prop_cell_fit, "ellipsize"))
        {
            if (code.is_cpp() && Project.is_wxWidgets31())
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
        if (code.is_cpp() && Project.is_wxWidgets31())
        {
            if (code.isPropValue(prop_selection_mode, "wxGridSelectNone"))
            {
                if (code.is_cpp() && Project.is_wxWidgets31())
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

    if (code.hasValue(prop_label_bg))
    {
        code.Eol().NodeName().Function("SetLabelBackgroundColour(").ColourCode(prop_label_bg).EndFunction();
    }

    // TODO: [KeyWorks - 02-27-2021] GenerateFontCode() was removed because it was obsolete and broken. It needs to
    // be replaced, but it should be part of an entire wxGrid overhaul.

    if (code.hasValue(prop_label_font))
    {
        code.GenFont(prop_label_font, "SetLabelFont(");
    }
    if (code.hasValue(prop_label_text))
        code.Eol().NodeName().Function("SetLabelTextColour(").ColourCode(prop_label_text).EndFunction();

    // Cell category

    if (code.hasValue(prop_cell_bg))
        code.Eol().NodeName().Function("SetDefaultCellBackgroundColour(").ColourCode(prop_cell_bg).EndFunction();
    if (code.hasValue(prop_cell_text))
        code.Eol().NodeName().Function("SetDefaultCellTextColour(").ColourCode(prop_cell_text).EndFunction();

    if (code.hasValue(prop_cell_font))
    {
        code.GenFont(prop_label_font, "SetDefaultCellFont");
    }

    code.Eol().NodeName().Function("SetDefaultCellAlignment(");
    code.itoa(prop_cell_horiz_alignment, prop_cell_vert_alignment).EndFunction();

    // Columns category

    if (code.IntValue(prop_default_col_size) > 0)
    {
        code.Eol().NodeName().Function("SetDefaultColSize(").as_string(prop_default_col_size).EndFunction();
    }
    else if (code.IsTrue(prop_autosize_cols))
    {
        code.Eol().NodeName().Function("AutoSizeColumns(").EndFunction();
    }

    if (code.IsTrue(prop_drag_col_move))
        code.Eol().NodeName().Function("EnableDragColMove(").True().EndFunction();

    if (code.IsFalse(prop_drag_col_size))
        code.Eol().NodeName().Function("EnableDragColSize(").False().EndFunction();

    code.Eol().NodeName().Function("SetColLabelAlignment(");
    code.itoa(prop_col_label_horiz_alignment, prop_col_label_vert_alignment).EndFunction();

    if (code.IntValue(prop_col_label_size) == -1)
        code.Eol().NodeName().Function("SetColLabelSize(").Add("wxGRID_AUTOSIZE").EndFunction();
    else if (code.IntValue(prop_col_label_size) == 0)
        code.Eol().NodeName().Function("HideColLabels(").EndFunction();
    else
        code.Eol().NodeName().Function("SetColLabelSize(").as_string(prop_col_label_size).EndFunction();

    if (code.hasValue(prop_column_sizes))
    {
        int index = 0;
        for (auto& iter: code.node()->as_ArrayString(prop_column_sizes))
        {
            code.Eol().NodeName().Function("SetColSize(").itoa(index++);
            code.Comma().Str(iter).EndFunction();
        }
    }

    if (code.hasValue(prop_col_label_values))
    {
        int index = 0;
        for (auto& iter: code.node()->as_ArrayString(prop_col_label_values))
        {
            code.Eol().NodeName().Function("SetColLabelValue(").itoa(index++);
            code.Comma().QuotedString(iter).EndFunction();
        }
    }

    // Rows category

    code.Str("\n\n");  // Force a break between column and row settings

    if (code.IntValue(prop_default_row_size) > 0)
    {
        code.Eol(eol_if_needed).NodeName().Function("SetDefaultRowSize(").as_string(prop_default_row_size).EndFunction();
    }
    else if (code.IsTrue(prop_autosize_rows))
    {
        code.Eol(eol_if_needed).NodeName().Function("AutoSizeRows(").EndFunction();
    }

    if (code.IsFalse(prop_drag_row_size))
        code.Eol(eol_if_needed).NodeName().Function("EnableDragRowSize(").False().EndFunction();

    code.Eol(eol_if_needed).NodeName().Function("SetRowLabelAlignment(");
    code.itoa(prop_row_label_horiz_alignment, prop_row_label_vert_alignment).EndFunction();

    if (code.IntValue(prop_row_label_size) == -1)
        code.Eol(eol_if_needed).NodeName().Function("SetRowLabelSize(").Add("wxGRID_AUTOSIZE").EndFunction();
    else if (code.IntValue(prop_row_label_size) == 0)
        code.Eol(eol_if_needed).NodeName().Function("HideRowLabels(").EndFunction();
    else
        code.Eol(eol_if_needed).NodeName().Function("SetRowLabelSize(").as_string(prop_row_label_size).EndFunction();

    if (code.hasValue(prop_row_sizes))
    {
        int index = 0;
        for (auto& iter: code.node()->as_ArrayString(prop_row_sizes))
        {
            code.Eol().NodeName().Function("SetRowSize(").itoa(index++);
            code.Comma().Str(iter).EndFunction();
        }
    }

    if (code.hasValue(prop_row_label_values))
    {
        int index = 0;
        for (auto& iter: code.node()->as_ArrayString(prop_row_label_values))
        {
            code.Eol().NodeName().Function("SetRowLabelValue(").itoa(index++);
            code.Comma().QuotedString(iter).EndFunction();
        }
    }

    code.CloseBrace();

    return true;
}

int GridGenerator::GetRequiredVersion(Node* node)
{
    // Code generation was invalid in minRequiredVer, so a newer version is required if this property is set.
    if (node->as_int(prop_selection_mode) != 0)
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }
    // There was no code generation for a non-default setting in minRequiredVer
    if (node->as_int(prop_row_label_size) != -1)
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }
    return BaseGenerator::GetRequiredVersion(node);
}

bool GridGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/grid.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp

int GridGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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

bool GridGenerator::GetImports(Node*, std::set<std::string>& set_imports, GenLang language)
{
    if (language == GEN_LANG_RUBY)
    {
        set_imports.insert("require 'wx/grid'");
        return true;
    }
    else
    {
    }
    return false;
}
