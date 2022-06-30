/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGrid generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
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

std::optional<ttlib::cstr> GridGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxWANTS_CHARS");

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
             << GenerateColourCode(node, prop_grid_line_color) << ");";

    code << braced_indent << node->get_node_name() << "->EnableDragGridSize("
         << (node->prop_as_bool(prop_drag_grid_size) ? "true" : "false") << ");";
    code << braced_indent << node->get_node_name() << "->SetMargins(" << node->prop_as_string(prop_margin_width) << ", "
         << node->prop_as_string(prop_margin_height) << ");";

    if (node->prop_as_string(prop_cell_fit) != "overflow")
    {
        if (node->prop_as_string(prop_cell_fit) == "clip")
        {
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#if wxCHECK_VERSION(3, 1, 4)";
                code << braced_indent << node->get_node_name() << "->SetDefaultCellFitMode(wxGridFitMode::Clip());";
                code << "\n#endif";
            }
            else
            {
                code << braced_indent << node->get_node_name() << "->SetDefaultCellFitMode(wxGridFitMode::Clip());";
            }
        }
        else if (node->prop_as_string(prop_cell_fit) == "ellipsize")
        {
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#if wxCHECK_VERSION(3, 1, 4)";
                code << braced_indent << node->get_node_name() << "->SetDefaultCellFitMode(wxGridFitMode::Ellipsize());";
                code << "\n#endif";
            }
            else
            {
                code << braced_indent << node->get_node_name() << "->SetDefaultCellFitMode(wxGridFitMode::Ellipsize());";
            }
        }
    }

    if (node->prop_as_int(prop_selection_mode) != 0)
    {
        if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1" &&
            node->prop_as_string(prop_selection_mode) == "wxGridSelectNone")
        {
            code << "\n#if wxCHECK_VERSION(3, 1, 5)";
            code << braced_indent << node->get_node_name() << "->SetSelectionMode("
                 << node->prop_as_string(prop_selection_mode) << ");";
            code << "\n#endif";
        }
        else
        {
            code << braced_indent << node->get_node_name() << "->SetSelectionMode("
                 << node->prop_as_string(prop_selection_mode) << ");";
        }
    }

    code << '\n';

    // Label category

    if (node->prop_as_bool(prop_native_col_header))
        code << braced_indent << node->get_node_name() << "->UseNativeColHeader();";
    else if (node->prop_as_bool(prop_native_col_labels))
        code << braced_indent << node->get_node_name() << "->SetUseNativeColLabels();";

    if (node->HasValue(prop_label_bg))
    {
        code << braced_indent << node->get_node_name() << "->SetLabelBackgroundColour("
             << GenerateColourCode(node, prop_label_bg) << ");";
    }

    // TODO: [KeyWorks - 02-27-2021] GenerateFontCode() was removed because it was obsolete and broken. It needs to be
    // replaced, but it should be part of an entire wxGrid overhaul.

#if 0
    if (node->HasValue(prop_label_font))
        code << braced_indent << node->get_node_name() << "->SetLabelFont(" << GenerateFontCode(node, "label_font") << ");";
#endif
    if (node->HasValue(prop_label_text))
        code << braced_indent << node->get_node_name() << "->SetLabelTextColour("
             << GenerateColourCode(node, prop_label_text) << ");";

    // Cell category

    if (node->HasValue(prop_cell_bg))
        code << braced_indent << node->get_node_name() << "->SetDefaultCellBackgroundColour("
             << GenerateColourCode(node, prop_cell_bg) << ");";
    if (node->HasValue(prop_cell_text))
        code << braced_indent << node->get_node_name() << "->SetDefaultCellTextColour("
             << GenerateColourCode(node, prop_cell_text) << ");";
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
