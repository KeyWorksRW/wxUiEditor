/////////////////////////////////////////////////////////////////////////////
// Purpose:   Sizer component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/button.h>       // wxButtonBase class
#include <wx/gbsizer.h>      // wxGridBagSizer:  A sizer that can lay out items in a grid,
#include <wx/statline.h>     // wxStaticLine class interface
#include <wx/textwrapper.h>  // declaration of wxTextWrapper class
#include <wx/wrapsizer.h>    // provide wrapping sizer for layout (wxWrapSizer)

#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "sizer_widgets.h"

wxObject* BoxSizerGenerator::Create(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxBoxSizer(node->prop_as_int(txt_orientation));
    sizer->SetMinSize(node->prop_as_wxSize(txt_minimum_size));
    return sizer;
}

std::optional<ttlib::cstr> BoxSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxBoxSizer(" << node->prop_as_string(txt_orientation) << ");";

    auto min_size = node->prop_as_wxSize(txt_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n    " << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool BoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

wxObject* GridSizerGenerator::Create(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxGridSizer(node->prop_as_int("rows"), node->prop_as_int("cols"), node->prop_as_int(txt_vgap),
                                 node->prop_as_int(txt_hgap));

    sizer->SetMinSize(node->prop_as_wxSize(txt_minimum_size));

    return sizer;
}

std::optional<ttlib::cstr> GridSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxGridSizer(";
    auto rows = node->prop_as_int("rows");
    auto cols = node->prop_as_int("cols");
    auto vgap = node->prop_as_int(txt_vgap);
    auto hgap = node->prop_as_int(txt_hgap);

    if (rows != 0)
    {
        code << rows << ", ";
    }
    code << cols;

    if (vgap != 0 || hgap != 0)
    {
        code << ", " << vgap << ", " << hgap;
    }
    code << ");";

    auto min_size = node->prop_as_wxSize(txt_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n    " << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool GridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

wxObject* WrapSizerGenerator::Create(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxWrapSizer(node->prop_as_int(txt_orientation), node->prop_as_int(txt_flags));
    sizer->SetMinSize(node->prop_as_wxSize(txt_minimum_size));
    return sizer;
}

std::optional<ttlib::cstr> WrapSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxWrapSizer(" << node->prop_as_string(txt_orientation);
    auto wrap_flags = node->prop_as_string(txt_wrap_flags);
    if (wrap_flags.empty())
        wrap_flags = "0";
    code << ", " << wrap_flags << ");";

    auto min_size = node->prop_as_wxSize(txt_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
    {
        code << "\n    " << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool WrapSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/wrapsizer.h>", set_src, set_hdr);
    return true;
}

wxObject* StaticBoxSizerGenerator::Create(Node* node, wxObject* parent)
{
    auto sizer = new wxStaticBoxSizer(node->prop_as_int(txt_orientation), wxStaticCast(parent, wxWindow),
                                      node->GetPropertyAsString(txt_label));

    auto min_size = node->prop_as_wxSize(txt_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
        sizer->SetMinSize(min_size);

    return sizer;
}

std::optional<ttlib::cstr> StaticBoxSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    std::string parent_name("this");
    if (!node->GetParent()->IsForm())
    {
        auto parent = node->GetParent();
        while (parent)
        {
            if (parent->IsContainer())
            {
                parent_name = parent->get_node_name();
                break;
            }
            else if (parent->GetClassName() == "wxStaticBoxSizer")
            {
                parent_name = parent->get_node_name() + "->GetStaticBox()";
                break;
            }
            parent = parent->GetParent();
        }
    }

    code << node->get_node_name() << " = new wxStaticBoxSizer(" << node->prop_as_string(txt_orientation) << ", " << parent_name;

    auto& label = node->prop_as_string(txt_label);
    if (label.size())
    {
        code << ", " << GenerateQuotedString(label);
    }
    code << ");";

    auto min_size = node->prop_as_wxSize(txt_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n    " << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

std::optional<ttlib::cstr> StaticBoxSizerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    ttlib::cstr code;
    auto node = event->GetNode();

    code << node->get_node_name() << "->GetStaticBox()->Bind(wxEVT_UPDATE_UI, &" << class_name << "::" << event->get_value()
         << ", this);";

    return code;
}

bool StaticBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);
    return true;
}

wxObject* FlexGridSizerGenerator::Create(Node* node, wxObject* /*parent*/)
{
    wxFlexGridSizer* sizer = new wxFlexGridSizer(node->prop_as_int("rows"), node->prop_as_int("cols"),
                                                 node->prop_as_int(txt_vgap), node->prop_as_int(txt_hgap));

#if 0
    for (auto& col: node->GetPropertyAsVectorIntPair(txt_growablecols))
    {
        sizer->AddGrowableCol(col.first, col.second);
    }
    for (auto& row: node->GetPropertyAsVectorIntPair(txt_growablerows))
    {
        sizer->AddGrowableRow(row.first, row.second);
    }
#endif

    sizer->SetMinSize(node->prop_as_wxSize(txt_minimum_size));
    sizer->SetFlexibleDirection(node->prop_as_int(txt_flexible_direction));
    sizer->SetNonFlexibleGrowMode((wxFlexSizerGrowMode) node->prop_as_int(txt_non_flexible_grow_mode));

    return sizer;
}

std::optional<ttlib::cstr> FlexGridSizerGenerator::GenConstruction(Node* node)
{
    // The leading tab is in case we indent in a brace block later on
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxFlexGridSizer(";
    auto rows = node->prop_as_int("rows");
    auto cols = node->prop_as_int("cols");
    auto vgap = node->prop_as_int(txt_vgap);
    auto hgap = node->prop_as_int(txt_hgap);

    // If rows is empty, only columns are supplied and wxFlexGridSizer will deduece the number of rows to use
    if (rows != 0)
    {
        code << rows << ", ";
    }
    code << cols << ", " << vgap << ", " << hgap << ");";

    // If growable settings are used, there can be a lot of lines of code generated. To make it a bit clearer, we put it in
    // braces
    bool isExpanded = false;

    if (auto& growable = node->prop_as_string(txt_growablecols); growable.size())
    {
        ttlib::multistr values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n    {";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.c_str() + pos + 1));
            }
            code << "\n        " << node->get_node_name() << "->AddGrowableCol(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    if (auto& growable = node->prop_as_string(txt_growablerows); growable.size())
    {
        ttlib::multistr values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n    {";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.c_str() + pos + 1));
            }
            code << "\n        " << node->get_node_name() << "->AddGrowableRow(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    auto& direction = node->prop_as_string(txt_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (isExpanded)
            code << "\n    }";
        return code;
    }

    code << (isExpanded ? "\n        " : "\n    ") << node->get_node_name() << "->SetFlexibleDirection(" << direction
         << ");";

    auto& non_flex_growth = node->prop_as_string(txt_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        if (isExpanded)
            code << "\n    }";
        return code;
    }
    code << (isExpanded ? "\n        " : "\n    ") << node->get_node_name() << "->SetNonFlexibleGrowMode(" << non_flex_growth
         << ");";

    if (isExpanded)
        code << "\n    }";
    return code;
}

bool FlexGridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

wxObject* GridBagSizerGenerator::Create(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxGridBagSizer(node->prop_as_int(txt_vgap), node->prop_as_int(txt_hgap));

#if 0
    for (auto& col: node->GetPropertyAsVectorIntPair(txt_growablecols))
    {
        sizer->AddGrowableCol(col.first, col.second);
    }
    for (auto& row: node->GetPropertyAsVectorIntPair(txt_growablerows))
    {
        sizer->AddGrowableRow(row.first, row.second);
    }
#endif

    sizer->SetMinSize(node->prop_as_wxSize(txt_minimum_size));
    sizer->SetFlexibleDirection(node->prop_as_int(txt_flexible_direction));
    sizer->SetNonFlexibleGrowMode((wxFlexSizerGrowMode) node->prop_as_int(txt_non_flexible_grow_mode));

    if (node->HasValue(txt_empty_cell_size))
    {
        sizer->SetEmptyCellSize(node->prop_as_wxSize(txt_empty_cell_size));
    }

    return sizer;
}

void GridBagSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto mockup = GetMockup();
    // For storing objects whose postion needs to be determined
    std::vector<std::pair<wxObject*, wxGBSizerItem*>> newNodes;
    wxGBPosition lastPosition(0, 0);

    auto sizer = wxStaticCast(wxobject, wxGridBagSizer);
    if (!sizer)
    {
        FAIL_MSG("This should be a wxGridBagSizer!");
        return;
    }

    auto count = mockup->GetNode(wxobject)->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto wxsizerItem = mockup->GetChild(wxobject, i);
        if (!wxsizerItem)
            continue;  // spacer's don't have objects
        auto node = mockup->GetNode(wxsizerItem);

        // Get the location of the item
        wxGBSpan span(node->prop_as_int(txt_rowspan), node->prop_as_int(txt_colspan));

        int column = node->prop_as_int(txt_column);
        if (column < 0)
        {
            // Needs to be auto positioned after the other children are added

            if (auto item = GetGBSizerItem(node, lastPosition, span, wxsizerItem); item)
            {
                newNodes.push_back(std::pair<wxObject*, wxGBSizerItem*>(wxsizerItem, item));
            }
            continue;
        }

        wxGBPosition position(node->prop_as_int(txt_row), column);

        if (sizer->CheckForIntersection(position, span))
        {
            // REVIEW: [KeyWorks - 10-22-2020] Not creating it in the Mockup isn't very helpful to the user since
            // they won't be able to see why the item hasn't been created.
            continue;
        }

        lastPosition = position;

        if (auto item = GetGBSizerItem(node, position, span, wxsizerItem); item)
        {
            sizer->Add(item);
        }
    }

    for (auto& iter: newNodes)
    {
        wxGBPosition position = iter.second->GetPos();
        wxGBSpan span = iter.second->GetSpan();
        int column = position.GetCol();
        while (sizer->CheckForIntersection(position, span))
        {
            column++;
            position.SetCol(column);
        }
        iter.second->SetPos(position);
        sizer->Add(iter.second);
    }
}

std::optional<ttlib::cstr> GridBagSizerGenerator::GenConstruction(Node* node)
{
    // The leading tab is in case we indent in a brace block later on
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxGridBagSizer(";

    auto vgap = node->prop_as_int(txt_vgap);
    auto hgap = node->prop_as_int(txt_hgap);
    if (vgap != 0 || hgap != 0)
    {
        code << vgap << ", " << hgap;
    }
    code << ");";

    // If growable settings are used, there can be a lot of lines of code generated. To make it a bit clearer, we put it in
    // braces
    bool isExpanded = false;

    if (auto& growable = node->prop_as_string(txt_growablecols); growable.size())
    {
        ttlib::multistr values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n    {";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.c_str() + pos + 1));
            }
            code << "\n        " << node->get_node_name() << "->AddGrowableCol(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    if (auto& growable = node->prop_as_string(txt_growablerows); growable.size())
    {
        ttlib::multistr values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n    {";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.c_str() + pos + 1));
            }
            code << "\n        " << node->get_node_name() << "->AddGrowableRow(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    auto& direction = node->prop_as_string(txt_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (isExpanded)
            code << "\n    }";
        return code;
    }

    code << (isExpanded ? "\n        " : "\n    ") << node->get_node_name() << "->SetFlexibleDirection(" << direction
         << ");";

    auto non_flex_growth = node->prop_as_string(txt_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        if (isExpanded)
            code << "\n    }";
        return code;
    }
    code << (isExpanded ? "\n        " : "\n    ") << node->get_node_name() << "->SetNonFlexibleGrowMode(" << non_flex_growth
         << ");";

    if (isExpanded)
        code << "\n    }";
    return code;
}

bool GridBagSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/gbsizer.h>", set_src, set_hdr);
    return true;
}

wxGBSizerItem* GridBagSizerGenerator::GetGBSizerItem(Node* sizeritem, const wxGBPosition& position, const wxGBSpan& span,
                                                     wxObject* child)
{
    auto sizer_flags = sizeritem->GetSizerFlags();

    if (sizeritem->GetClassName() == "spacer")
    {
        return new wxGBSizerItem(sizeritem->prop_as_int(txt_width), sizeritem->prop_as_int(txt_height), position, span,
                                 sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
    }

    // Add the child (window or sizer) to the sizer
    auto windowChild = wxDynamicCast(child, wxWindow);
    auto sizerChild = wxDynamicCast(child, wxSizer);

    if (windowChild)
    {
        return new wxGBSizerItem(windowChild, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
    }
    else if (sizerChild)
    {
        return new wxGBSizerItem(sizerChild, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
    }
    else
    {
        FAIL_MSG(
            "The GBSizerItem component's child is not a wxWindow or a wxSizer or a Spacer - this should not be possible!");
        return NULL;
    }
}

//////////////////////////////////////////  SpacerGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> SpacerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->GetParent()->get_node_name() << "->Add(" << node->prop_as_string(txt_width) << ", "
         << node->prop_as_string(txt_height) << ", ";

    if (node->GetParent()->GetClassName() == "wxGridBagSizer")
    {
        code << "wxGBPosition(" << node->prop_as_string(txt_row) << ", " << node->prop_as_string(txt_column) << "), ";

        // Only write the span if it's not a default value.
        auto row_span = node->prop_as_int(txt_rowspan);
        auto col_span = node->prop_as_int(txt_colspan);
        if (row_span <= 1 && col_span <= 1)
            code << "wxGBSpan(), ";
        else
            code << "wxGBSpan(" << row_span << ", " << col_span << "), ";

        auto alignment = node->prop_as_string(txt_alignment);
        auto flags = node->prop_as_string(txt_flags);
        auto borders = node->prop_as_string(txt_borders);

        if (alignment.empty() && flags.empty())
        {
            if (borders.empty())
                borders = "0";
            code << borders << ", ";
        }
        else
        {
            // As of wxWidgets 3.1.4, wxGridBaseSizer doesn't support passing in a wxSizerFlags parameter. Since
            // wxSizerFlags avoids compiler warnings with C++20 compilers (combining different enumeration flags)
            // and is arguably more readable, we use it anyway and just call GetFlags().

            code << "wxSizerFlags()";
            if (alignment.contains("wxALIGN_CENTER"))
            {
                // Note that CenterHorizontal() and CenterVertical() require wxWidgets 3.1 or higher.
                code << ".Center()";
            }
            else
            {
                if (alignment.contains("wxALIGN_LEFT"))
                {
                    code << ".Left()";
                }
                else if (alignment.contains("wxALIGN_RIGHT"))
                {
                    code << ".Right()";
                }

                if (alignment.contains("wxALIGN_TOP"))
                {
                    code << ".Top()";
                }
                else if (alignment.contains("wxALIGN_BOTTOM"))
                {
                    code << ".Bottom()";
                }
            }

            // REVIEW: [KeyWorks - 10-26-2020] Do all of these flags actually work in a wxGridBagSizer?

            if (flags.contains("wxEXPAND"))
            {
                code << ".Expand()";
            }
            if (flags.contains("wxSHAPED"))
            {
                code << ".Shaped()";
            }
            if (flags.contains("wxFIXED_MINSIZE"))
            {
                code << ".FixedMinSize()";
            }
            if (flags.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
            {
                code << ".ReserveSpaceEvenIfHidden()";
            }

            if (borders.contains("wxALL"))
            {
                code << ".Border(wxALL";
            }
            else
            {
                code << ".Border(";
                ttlib::cstr border_flags;

                if (borders.contains("wxLEFT"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << "wxLEFT";
                }
                if (borders.contains("wxRIGHT"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << "wxRIGHT";
                }
                if (borders.contains("wxTOP"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << "wxTOP";
                }
                if (borders.contains("wxBOTTOM"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << "wxBOTTOM";
                }

                if (border_flags.empty())
                    border_flags = "0";

                code << border_flags;
            }

            code << ").GetFlags(), ";
        }

        auto border_size = node->prop_as_string(txt_border_size);

        // Using GetDefaultBorder() means it will change correctly on high DPI displays.
        if (border_size == "5")
        {
            code << "wxSizerFlags::GetDefaultBorder()";
        }
        else if (border_size == "10")
        {
            code << "wxSizerFlags::GetDefaultBorder() * 2";
        }
        else if (border_size == "15")
        {
            code << "wxSizerFlags::GetDefaultBorder() * 5";
        }
        else
        {
            code << border_size;
        }
        code << ");";
    }
    else
    {
        code << GenerateSizerFlags(node) << ");";
    }

    return code;
}

//////////////////////////////////////////  StdDialogButtonSizerGenerator  //////////////////////////////////////////

wxObject* StdDialogButtonSizerGenerator::Create(Node* node, wxObject* parent)
{
    auto sizer = new wxStdDialogButtonSizer();

    sizer->SetMinSize(node->prop_as_wxSize(txt_minimum_size));

    if (node->prop_as_bool("OK"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_OK));
    else if (node->prop_as_bool("Yes"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_YES));
    else if (node->prop_as_bool("Save"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_SAVE));

    if (node->prop_as_bool("No"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_NO));

    if (node->prop_as_bool("Cancel"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CANCEL));
    else if (node->prop_as_bool("Close"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CLOSE));

    if (node->prop_as_bool("Apply"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_APPLY));

    if (node->prop_as_bool("Help"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_HELP));
    else if (node->prop_as_bool("ContextHelp"))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CONTEXT_HELP));

    sizer->Realize();

    if (node->prop_as_bool("static_line"))
    {
        auto topsizer = new wxBoxSizer(wxVERTICAL);
        topsizer->Add(new wxStaticLine(wxDynamicCast(parent, wxWindow)), wxSizerFlags().Expand().DoubleBorder(wxBOTTOM));
        topsizer->Add(sizer, wxSizerFlags().Expand());
        return topsizer;
    }

    return sizer;
}

std::optional<ttlib::cstr> StdDialogButtonSizerGenerator::GenConstruction(Node* node)
{
    auto& def_btn_name = node->prop_as_string("default_button");

    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";

    // Unfortunately, the CreateStdDialogButtonSizer() code does not support a wxID_SAVE or wxID_CONTEXT_HELP button
    // even though wxStdDialogButtonSizer does support it. Worse, CreateStdDialogButtonSizer() calls Realize() which
    // means if you add a button afterwards, then it will not be positioned correctly. You can't call Realize() twice
    // without hitting assertion errors in debug builds, and in release builds, the Save button is positioned
    // incorrectly. Unfortunately that means we have to add the buttons one at a time if a Save button is specified.

    if (node->FindParentForm()->GetClassName() == "wxDialog" && !node->prop_as_bool("Save") &&
        !node->prop_as_bool("ContextHelp"))
    {
        code << node->get_node_name() << " = CreateStdDialogButtonSizer(";
        ttlib::cstr flags;

        if (node->prop_as_bool("OK"))
            AddBitFlag(flags, "wxOK");
        else if (node->prop_as_bool("Yes"))
            AddBitFlag(flags, "wxYES");

        if (node->prop_as_bool("No"))
            AddBitFlag(flags, "wxNO");

        if (node->prop_as_bool("Cancel"))
            AddBitFlag(flags, "wxCANCEL");
        else if (node->prop_as_bool("Close"))
            AddBitFlag(flags, "wxCLOSE");

        if (node->prop_as_bool("Apply"))
            AddBitFlag(flags, "wxAPPLY");

        if (node->prop_as_bool("Help"))
            AddBitFlag(flags, "wxHELP");

        if (def_btn_name != "OK" && def_btn_name != "Yes")
            AddBitFlag(flags, "wxNO_DEFAULT");

        code << flags << ");";

        // If wxNO_DEFAULT is specified and a No button is used, then it will be set as the default
        if (def_btn_name == "Close" || def_btn_name == "Cancel")
            code << "\n    " << node->get_node_name() << "->GetCancelButton()->SetDefault();";
        else if (def_btn_name == "Apply")
            code << "\n    " << node->get_node_name() << "->GetApplyButton()->SetDefault();";

        return code;
    }

    // The following code is used if a Save or ContextHelp button is requrested, or the parent form is not a Dialog
    code << node->get_node_name() << " = new wxStdDialogButtonSizer();";

    auto min_size = node->prop_as_wxSize(txt_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n    " << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    // You can only have one of: Ok, Yes, Save
    if (node->prop_as_bool("OK"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_OK));";
    else if (node->prop_as_bool("Yes"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_YES));";
    else if (node->prop_as_bool("Save"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_SAVE));";

    if (node->prop_as_bool("No"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_NO));";

    // You can only have one of: Cancel, Close
    if (node->prop_as_bool("Cancel"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CANCEL));";
    else if (node->prop_as_bool("Close"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CLOSE));";

    if (node->prop_as_bool("Apply"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_APPLY));";

    // You can only have one of: Help, ContextHelp
    if (node->prop_as_bool("Help"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_HELP));";
    else if (node->prop_as_bool("ContextHelp"))
        code << "\n    " << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CONTEXT_HELP));";

    if (def_btn_name == "OK" || def_btn_name == "Yes" || def_btn_name == "Save")
        code << "\n    " << node->get_node_name() << "->GetAffirmativeButton()->SetDefault();";
    else if (def_btn_name == "Cancel" || def_btn_name == "Close")
        code << "\n    " << node->get_node_name() << "->GetCancelButton()->SetDefault();";
    else if (def_btn_name == "Apply")
        code << "\n    " << node->get_node_name() << "->GetApplyButton()->SetDefault();";
    else if (def_btn_name == "No")
        code << "\n    " << node->get_node_name() << "->GetNegativeButton()->SetDefault();";
    else if (def_btn_name == "Help" || def_btn_name == "ContextHelp")
        code << "\n    " << node->get_node_name() << "->GetHelpButton()->SetDefault();";

    code << "\n    " << node->get_node_name() << "->Realize();\n";

    return code;
}

std::optional<ttlib::cstr> StdDialogButtonSizerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    ttlib::cstr code;
    ttlib::cstr handler;

    // This is what we normally use if an ID is needed. However, a lambda needs to put the ID on it's own line, so we
    // use a string for this to allow the lambda processing code to replace it.
    std::string comma(", ");

    bool is_lambda { false };

    if (event->get_value().contains("["))
    {
        handler << event->get_name() << ',' << event->get_value();
        // Put the lambda expression on it's own line
        handler.Replace("[", "\n        [");
        comma = ",\n        ";
        is_lambda = true;
    }
    else if (event->get_value().contains("::"))
    {
        if (event->get_value()[0] != '&')
            handler << '&';
        handler << event->get_value();
    }
    else
    {
        // code << "Bind(" << evt_str << ", &" << class_name << "::" << event->get_value() << ", this, ";
        handler << "&" << class_name << "::" << event->get_value() << ", this";
    }

    ttlib::cstr evt_str =
        (event->GetEventInfo()->get_event_class() == "wxCommandEvent" ? "wxEVT_BUTTON" : "wxEVT_UPDATE_UI");
    code << "Bind(" << evt_str << comma << handler << comma;

    if (event->get_name().is_sameprefix("OKButton"))
        code << "wxID_OK);";
    else if (event->get_name().is_sameprefix("YesButton"))
        code << "wxID_YES);";
    else if (event->get_name().is_sameprefix("SaveButton"))
        code << "wxID_SAVE);";
    else if (event->get_name().is_sameprefix("ApplyButton"))
        code << "wxID_APPLY);";
    else if (event->get_name().is_sameprefix("NoButton"))
        code << "wxID_NO);";
    else if (event->get_name().is_sameprefix("CancelButton"))
        code << "wxID_CANCEL);";
    else if (event->get_name().is_sameprefix("CloseButton"))
        code << "wxID_CLOSE);";
    else if (event->get_name().is_sameprefix("HelpButton"))
        code << "wxID_HELP);";
    else if (event->get_name().is_sameprefix("ContextHelpButton"))
        code << "wxID_CONTEXT_HELP);";

    return code;
}

bool StdDialogButtonSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  TextSizerGenerator  //////////////////////////////////////////

wxObject* TextSizerGenerator::Create(Node* node, wxObject* parent)
{
    wxTextSizerWrapper wrapper(wxStaticCast(parent, wxWindow));
    return wrapper.CreateSizer(node->prop_as_wxString("text"), node->prop_as_int("wrap"));
}

std::optional<ttlib::cstr> TextSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name();
    auto parent = node->GetParent();
    while (parent->IsSizer())
        parent = parent->GetParent();

    if (parent->GetClassName() == "wxDialog")
    {
        code << " = CreateTextSizer(";
    }
    else
    {
        code << " = wxTextSizerWrapper(" << parent->get_node_name() << ").CreateSizer(";
    }

    code << GenerateQuotedString(node->prop_as_string("text")) << ", " << node->prop_as_string(txt_width) << ");";

    return code;
}

bool TextSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    auto parent = node->GetParent();
    while (parent->IsSizer())
        parent = parent->GetParent();

    if (parent->GetClassName() == "wxDialog")
        InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/textwrapper.h>", set_src, set_hdr);

    return true;
}
