/////////////////////////////////////////////////////////////////////////////
// Purpose:   Sizer component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>       // wxButtonBase class
#include <wx/checkbox.h>     // wxCheckBox class interface
#include <wx/gbsizer.h>      // wxGridBagSizer:  A sizer that can lay out items in a grid,
#include <wx/radiobut.h>     // wxRadioButton declaration
#include <wx/statbox.h>      // wxStaticBox base header
#include <wx/statline.h>     // wxStaticLine class interface
#include <wx/textwrapper.h>  // declaration of wxTextWrapper class
#include <wx/wrapsizer.h>    // provide wrapping sizer for layout (wxWrapSizer)

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "gen_common.h"   // GeneratorLibrary -- Generator classes
#include "gen_inherit.h"  // Inherited class code generation
#include "node.h"         // Node class

#include "sizer_widgets.h"

//////////////////////////////////////////  BoxSizerGenerator  //////////////////////////////////////////

wxObject* BoxSizerGenerator::CreateMockup(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxBoxSizer(node->prop_as_int(prop_orientation));
    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    return sizer;
}

std::optional<ttlib::cstr> BoxSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxBoxSizer(" << node->prop_as_string(prop_orientation) << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool BoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  GridSizerGenerator  //////////////////////////////////////////

wxObject* GridSizerGenerator::CreateMockup(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxGridSizer(node->prop_as_int(prop_rows), node->prop_as_int(prop_cols), node->prop_as_int(prop_vgap),
                                 node->prop_as_int(prop_hgap));

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));

    return sizer;
}

std::optional<ttlib::cstr> GridSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxGridSizer(";
    auto rows = node->prop_as_int(prop_rows);
    auto cols = node->prop_as_int(prop_cols);
    auto vgap = node->prop_as_int(prop_vgap);
    auto hgap = node->prop_as_int(prop_hgap);

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

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool GridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  WrapSizerGenerator  //////////////////////////////////////////

wxObject* WrapSizerGenerator::CreateMockup(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxWrapSizer(node->prop_as_int(prop_orientation), node->prop_as_int(prop_flags));
    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    return sizer;
}

std::optional<ttlib::cstr> WrapSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxWrapSizer(" << node->prop_as_string(prop_orientation);
    auto wrap_flags = node->prop_as_string(prop_wrap_flags);
    if (wrap_flags.empty())
        wrap_flags = "0";
    code << ", " << wrap_flags << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

bool WrapSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/wrapsizer.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  StaticBoxSizerGenerator  //////////////////////////////////////////

wxObject* StaticBoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto sizer = new wxStaticBoxSizer(node->prop_as_int(prop_orientation), wxStaticCast(parent, wxWindow),
                                      node->prop_as_wxString(prop_label));

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
        sizer->SetMinSize(min_size);

    if (node->prop_as_bool(prop_hidden) && !GetMockup()->IsShowingHidden())
        sizer->GetStaticBox()->Hide();

    return sizer;
}

std::optional<ttlib::cstr> StaticBoxSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    ttlib::cstr parent_name("this");
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
            else if (parent->isGen(gen_wxStaticBoxSizer) || parent->isGen(gen_StaticCheckboxBoxSizer) ||
                     parent->isGen(gen_StaticRadioBtnBoxSizer))
            {
                parent_name.clear();
                parent_name << parent->get_node_name() << "->GetStaticBox()";
                break;
            }
            parent = parent->GetParent();
        }
    }

    code << node->get_node_name() << " = new wxStaticBoxSizer(" << node->prop_as_string(prop_orientation) << ", "
         << parent_name;

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
    {
        code << ", " << GenerateQuotedString(label);
    }
    code << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

std::optional<ttlib::cstr> StaticBoxSizerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;
    if (node->prop_as_bool(prop_disabled))
    {
        code << node->get_node_name() << "->GetStaticBox()->Enable(false);";
    }
    if (node->prop_as_bool(prop_hidden))
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->GetStaticBox()->Hide();";
    }

    return code;
}

std::optional<ttlib::cstr> StaticBoxSizerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StaticBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  StaticCheckboxBoxSizerGenerator  //////////////////////////////////////////

wxObject* StaticCheckboxBoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    long style_value = 0;
    if (node->prop_as_string(prop_style).contains("wxALIGN_RIGHT"))
        style_value |= wxALIGN_RIGHT;

    m_checkbox = new wxCheckBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                wxDefaultPosition, wxDefaultSize, style_value);
    if (node->prop_as_bool(prop_checked))
        m_checkbox->SetValue(true);

    auto staticbox = new wxStaticBox(wxStaticCast(parent, wxWindow), wxID_ANY, m_checkbox);

    auto sizer = new wxStaticBoxSizer(staticbox, node->prop_as_int(prop_orientation));

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
        sizer->SetMinSize(min_size);

    if (node->prop_as_bool(prop_hidden) && !GetMockup()->IsShowingHidden())
        sizer->GetStaticBox()->Hide();

    if (node->HasValue(prop_tooltip))
        m_checkbox->SetToolTip(node->prop_as_wxString(prop_tooltip));

    return sizer;
}

bool StaticCheckboxBoxSizerGenerator::OnPropertyChange(wxObject* /* widget */, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_tooltip))
    {
        m_checkbox->SetToolTip(node->prop_as_wxString(prop_tooltip));
    }

    return false;
}

std::optional<ttlib::cstr> StaticCheckboxBoxSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << node->prop_as_string(prop_checkbox_var_name) << " = new wxCheckBox(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->prop_as_string(prop_label).size())
    {
        code << GenerateQuotedString(node->prop_as_string(prop_label));
    }
    else
    {
        code << "wxEmptyString";
    }

    code << ");\n";

    if (auto result = GenInheritSettings(node); result)
    {
        code << result.value() << '\n';
    }

    if (node->IsLocal())
        code << "auto ";

    ttlib::cstr parent_name("this");
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
            else if (parent->isGen(gen_wxStaticBoxSizer) || parent->isGen(gen_StaticCheckboxBoxSizer) ||
                     parent->isGen(gen_StaticRadioBtnBoxSizer))
            {
                parent_name.clear();
                parent_name << parent->get_node_name() << "->GetStaticBox()";
                break;
            }
            parent = parent->GetParent();
        }
    }

    code << node->get_node_name() << " = new wxStaticBoxSizer(new wxStaticBox(" << parent_name << ", wxID_ANY, ";
    code << node->prop_as_string(prop_checkbox_var_name) << "), " << node->prop_as_string(prop_orientation) << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

std::optional<ttlib::cstr> StaticCheckboxBoxSizerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;
    if (node->prop_as_bool(prop_disabled))
    {
        code << node->get_node_name() << "->GetStaticBox()->Enable(false);";
    }
    if (node->prop_as_bool(prop_hidden))
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->GetStaticBox()->Hide();";
    }
    if (node->HasValue(prop_tooltip))
    {
        if (code.size())
            code << "\n\t";
        code << node->prop_as_string(prop_checkbox_var_name) << "->SetToolTip("
             << GenerateQuotedString(node->prop_as_string(prop_tooltip)) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> StaticCheckboxBoxSizerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StaticCheckboxBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);

    // The checkbox is always a class member, so we need to force it to be added to the header set
    set_hdr.insert("#include <wx/checkbox.h>");
    return true;
}

//////////////////////////////////////////  StaticRadioBtnBoxSizerGenerator  //////////////////////////////////////////

wxObject* StaticRadioBtnBoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    m_radiobtn = new wxRadioButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label));
    if (node->prop_as_bool(prop_checked))
        m_radiobtn->SetValue(true);

    auto staticbox = new wxStaticBox(wxStaticCast(parent, wxWindow), wxID_ANY, m_radiobtn);

    auto sizer = new wxStaticBoxSizer(staticbox, node->prop_as_int(prop_orientation));

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
        sizer->SetMinSize(min_size);

    if (node->prop_as_bool(prop_hidden) && !GetMockup()->IsShowingHidden())
        sizer->GetStaticBox()->Hide();

    if (node->HasValue(prop_tooltip))
        m_radiobtn->SetToolTip(node->prop_as_wxString(prop_tooltip));

    return sizer;
}

bool StaticRadioBtnBoxSizerGenerator::OnPropertyChange(wxObject* /* widget */, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_tooltip))
    {
        m_radiobtn->SetToolTip(node->prop_as_wxString(prop_tooltip));
    }

    return false;
}

std::optional<ttlib::cstr> StaticRadioBtnBoxSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << node->prop_as_string(prop_radiobtn_var_name) << " = new wxRadioButton(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->prop_as_string(prop_label).size())
    {
        code << GenerateQuotedString(node->prop_as_string(prop_label));
    }
    else
    {
        code << "wxEmptyString";
    }
    code << ");\n";

    if (node->IsLocal())
        code << "auto ";

    ttlib::cstr parent_name("this");
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
            else if (parent->isGen(gen_wxStaticBoxSizer) || parent->isGen(gen_StaticCheckboxBoxSizer) ||
                     parent->isGen(gen_StaticRadioBtnBoxSizer))
            {
                parent_name.clear();
                parent_name << parent->get_node_name() << "->GetStaticBox()";
                break;
            }
            parent = parent->GetParent();
        }
    }

    code << node->get_node_name() << " = new wxStaticBoxSizer(new wxStaticBox(" << parent_name << ", wxID_ANY, ";
    code << node->prop_as_string(prop_radiobtn_var_name) << "), " << node->prop_as_string(prop_orientation) << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

std::optional<ttlib::cstr> StaticRadioBtnBoxSizerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;
    if (node->prop_as_bool(prop_disabled))
    {
        code << node->get_node_name() << "->GetStaticBox()->Enable(false);";
    }
    if (node->prop_as_bool(prop_hidden))
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->GetStaticBox()->Hide();";
    }
    if (node->HasValue(prop_tooltip))
    {
        if (code.size())
            code << "\n\t";
        code << node->prop_as_string(prop_radiobtn_var_name) << "->SetToolTip("
             << GenerateQuotedString(node->prop_as_string(prop_tooltip)) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> StaticRadioBtnBoxSizerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StaticRadioBtnBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);

    // The radiobtn is always a class member, so we need to force it to be added to the header set
    set_hdr.insert("#include <wx/radiobut.h>");
    return true;
}

//////////////////////////////////////////  FlexGridSizerGenerator  //////////////////////////////////////////

wxObject* FlexGridSizerGenerator::CreateMockup(Node* node, wxObject* /*parent*/)
{
    wxFlexGridSizer* sizer = new wxFlexGridSizer(node->prop_as_int(prop_rows), node->prop_as_int(prop_cols),
                                                 node->prop_as_int(prop_vgap), node->prop_as_int(prop_hgap));

#if 0
    for (auto& col: node->GetPropertyAsVectorIntPair("growablecols"))
    {
        sizer->AddGrowableCol(col.first, col.second);
    }
    for (auto& row: node->GetPropertyAsVectorIntPair("growablerows"))
    {
        sizer->AddGrowableRow(row.first, row.second);
    }
#endif

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    sizer->SetFlexibleDirection(node->prop_as_int(prop_flexible_direction));
    sizer->SetNonFlexibleGrowMode((wxFlexSizerGrowMode) node->prop_as_int(prop_non_flexible_grow_mode));

    return sizer;
}

std::optional<ttlib::cstr> FlexGridSizerGenerator::GenConstruction(Node* node)
{
    // The leading tab is in case we indent in a brace block later on
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxFlexGridSizer(";
    auto rows = node->prop_as_int(prop_rows);
    auto cols = node->prop_as_int(prop_cols);
    auto vgap = node->prop_as_int(prop_vgap);
    auto hgap = node->prop_as_int(prop_hgap);

    // If rows is empty, only columns are supplied and wxFlexGridSizer will deduece the number of rows to use
    if (rows != 0)
    {
        code << rows << ", ";
    }
    code << cols << ", " << vgap << ", " << hgap << ");";

    // If growable settings are used, there can be a lot of lines of code generated. To make it a bit clearer, we put it in
    // braces
    bool isExpanded = false;

    if (auto& growable = node->prop_as_string(prop_growablecols); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n\t{";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
            }
            code << "\n\t    " << node->get_node_name() << "->AddGrowableCol(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    if (auto& growable = node->prop_as_string(prop_growablerows); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n\t{";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
            }
            code << "\n\t    " << node->get_node_name() << "->AddGrowableRow(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    auto& direction = node->prop_as_string(prop_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (isExpanded)
            code << "\n\t}";
        return code;
    }

    code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetFlexibleDirection(" << direction << ");";

    auto& non_flex_growth = node->prop_as_string(prop_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        if (isExpanded)
            code << "\n\t}";
        return code;
    }
    code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetNonFlexibleGrowMode(" << non_flex_growth
         << ");";

    if (isExpanded)
        code << "\n\t}";
    return code;
}

bool FlexGridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  GridBagSizerGenerator  //////////////////////////////////////////

wxObject* GridBagSizerGenerator::CreateMockup(Node* node, wxObject* /*parent*/)
{
    auto sizer = new wxGridBagSizer(node->prop_as_int(prop_vgap), node->prop_as_int(prop_hgap));

#if 0
    for (auto& col: node->GetPropertyAsVectorIntPair(map_PropNames[prop_growablecols]))
    {
        sizer->AddGrowableCol(col.first, col.second);
    }
    for (auto& row: node->GetPropertyAsVectorIntPair(map_PropNames[prop_growablerows]))
    {
        sizer->AddGrowableRow(row.first, row.second);
    }
#endif

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    sizer->SetFlexibleDirection(node->prop_as_int(prop_flexible_direction));
    sizer->SetNonFlexibleGrowMode((wxFlexSizerGrowMode) node->prop_as_int(prop_non_flexible_grow_mode));

    if (node->HasValue(prop_empty_cell_size))
    {
        sizer->SetEmptyCellSize(node->prop_as_wxSize(prop_empty_cell_size));
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
        wxGBSpan span(node->prop_as_int(prop_rowspan), node->prop_as_int(prop_colspan));

        int column = node->prop_as_int(prop_column);
        if (column < 0)
        {
            // Needs to be auto positioned after the other children are added

            if (auto item = GetGBSizerItem(node, lastPosition, span, wxsizerItem); item)
            {
                newNodes.push_back(std::pair<wxObject*, wxGBSizerItem*>(wxsizerItem, item));
            }
            continue;
        }

        wxGBPosition position(node->prop_as_int(prop_row), column);

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
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxGridBagSizer(";

    auto vgap = node->prop_as_int(prop_vgap);
    auto hgap = node->prop_as_int(prop_hgap);
    if (vgap != 0 || hgap != 0)
    {
        code << vgap << ", " << hgap;
    }
    code << ");";

    // If growable settings are used, there can be a lot of lines of code generated. To make it a bit clearer, we put it in
    // braces
    bool isExpanded = false;

    if (auto& growable = node->prop_as_string(prop_growablecols); growable.size())
    {
        ttlib::multistr values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n\t{";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.c_str() + pos + 1));
            }
            code << "\n\t    " << node->get_node_name() << "->AddGrowableCol(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    if (auto& growable = node->prop_as_string(prop_growablerows); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n\t{";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
            }
            code << "\n\t    " << node->get_node_name() << "->AddGrowableRow(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    auto& direction = node->prop_as_string(prop_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (isExpanded)
            code << "\n\t}";
        return code;
    }

    code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetFlexibleDirection(" << direction << ");";

    auto non_flex_growth = node->prop_as_string(prop_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        if (isExpanded)
            code << "\n\t}";
        return code;
    }
    code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetNonFlexibleGrowMode(" << non_flex_growth
         << ");";

    if (isExpanded)
        code << "\n\t}";
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

    if (sizeritem->isGen(gen_spacer))
    {
        return new wxGBSizerItem(sizeritem->prop_as_int(prop_width), sizeritem->prop_as_int(prop_height), position, span,
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
        return nullptr;
    }
}

//////////////////////////////////////////  SpacerGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> SpacerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << node->GetParent()->get_node_name();

    if (node->GetParent()->isGen(gen_wxGridBagSizer))
    {
        auto flags = node->GetSizerFlags();

        code << "->Add(" << node->prop_as_int(prop_width) << ", " << node->prop_as_int(prop_height);
        code << ", wxGBPosition(" << node->prop_as_int(prop_row) << ", " << node->prop_as_int(prop_column);
        code << "), wxGBSpan(" << node->prop_as_int(prop_rowspan) << ", " << node->prop_as_int(prop_colspan);
        code << "), " << flags.GetFlags() << ", " << node->prop_as_int(prop_border_size);
        if (node->prop_as_bool(prop_add_default_border))
            code << " + wxSizerFlags::GetDefaultBorder()";
        code << ");";
    }
    else
    {
        if (node->prop_as_int(prop_proportion) != 0)
        {
            code << "->AddStretchSpacer(" << node->prop_as_string(prop_proportion) << ");";
        }
        else
        {
            if (node->prop_as_int(prop_width) == node->prop_as_int(prop_height))
            {
                code << "->AddSpacer(" << node->prop_as_string(prop_width);
            }
            else if (node->GetParent()->HasValue(prop_orientation))
            {
                code << "->AddSpacer(";
                if (node->GetParent()->prop_as_string(prop_orientation) == "wxVERTICAL")
                {
                    code << node->prop_as_string(prop_height);
                }
                else
                {
                    code << node->prop_as_string(prop_width);
                }
            }

            else
            {
                code << "->Add(" << node->prop_as_string(prop_width);
                if (node->prop_as_bool(prop_add_default_border))
                    code << " + wxSizerFlags::GetDefaultBorder()";
                code << ", " << node->prop_as_string(prop_height);
            }

            if (node->prop_as_bool(prop_add_default_border))
                code << " + wxSizerFlags::GetDefaultBorder()";

            code << ");";
        }
    }

    return code;
}

//////////////////////////////////////////  StdDialogButtonSizerGenerator  //////////////////////////////////////////

wxObject* StdDialogButtonSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto sizer = new wxStdDialogButtonSizer();

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));

    if (node->prop_as_bool(prop_OK))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_OK));
    else if (node->prop_as_bool(prop_Yes))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_YES));
    else if (node->prop_as_bool(prop_Save))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_SAVE));

    if (node->prop_as_bool(prop_No))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_NO));

    if (node->prop_as_bool(prop_Cancel))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CANCEL));
    else if (node->prop_as_bool(prop_Close))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CLOSE));

    if (node->prop_as_bool(prop_Apply))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_APPLY));

    if (node->prop_as_bool(prop_Help))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_HELP));
    else if (node->prop_as_bool(prop_ContextHelp))
        sizer->AddButton(new wxButton(wxStaticCast(parent, wxWindow), wxID_CONTEXT_HELP));

    sizer->Realize();

    if (node->prop_as_bool(prop_static_line))
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
    auto& def_btn_name = node->prop_as_string(prop_default_button);

    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";

    // Unfortunately, the CreateStdDialogButtonSizer() code does not support a wxID_SAVE or wxID_CONTEXT_HELP button
    // even though wxStdDialogButtonSizer does support it. Worse, CreateStdDialogButtonSizer() calls Realize() which
    // means if you add a button afterwards, then it will not be positioned correctly. You can't call Realize() twice
    // without hitting assertion errors in debug builds, and in release builds, the Save button is positioned
    // incorrectly. Unfortunately that means we have to add the buttons one at a time if a Save button is specified.

    if (node->FindParentForm()->isGen(gen_wxDialog) && !node->prop_as_bool(prop_Save) &&
        !node->prop_as_bool(prop_ContextHelp))
    {
        code << node->get_node_name() << " = CreateStdDialogButtonSizer(";
        ttlib::cstr flags;

        if (node->prop_as_bool(prop_OK))
            AddBitFlag(flags, "wxOK");
        else if (node->prop_as_bool(prop_Yes))
            AddBitFlag(flags, "wxYES");

        if (node->prop_as_bool(prop_No))
            AddBitFlag(flags, "wxNO");

        if (node->prop_as_bool(prop_Cancel))
            AddBitFlag(flags, "wxCANCEL");
        else if (node->prop_as_bool(prop_Close))
            AddBitFlag(flags, "wxCLOSE");

        if (node->prop_as_bool(prop_Apply))
            AddBitFlag(flags, "wxAPPLY");

        if (node->prop_as_bool(prop_Help))
            AddBitFlag(flags, "wxHELP");

        if (def_btn_name != "OK" && def_btn_name != "Yes")
            AddBitFlag(flags, "wxNO_DEFAULT");

        code << flags << ");";

        // If wxNO_DEFAULT is specified and a No button is used, then it will be set as the default
        if (def_btn_name == "Close" || def_btn_name == "Cancel")
            code << "\n\t" << node->get_node_name() << "->GetCancelButton()->SetDefault();";
        else if (def_btn_name == "Apply")
            code << "\n\t" << node->get_node_name() << "->GetApplyButton()->SetDefault();";

        return code;
    }

    // The following code is used if a Save or ContextHelp button is requrested, or the parent form is not a Dialog
    code << node->get_node_name() << " = new wxStdDialogButtonSizer();";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    // You can only have one of: Ok, Yes, Save
    if (node->prop_as_bool(prop_OK))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_OK));";
    else if (node->prop_as_bool(prop_Yes))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_YES));";
    else if (node->prop_as_bool(prop_Save))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_SAVE));";

    if (node->prop_as_bool(prop_No))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_NO));";

    // You can only have one of: Cancel, Close
    if (node->prop_as_bool(prop_Cancel))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CANCEL));";
    else if (node->prop_as_bool(prop_Close))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CLOSE));";

    if (node->prop_as_bool(prop_Apply))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_APPLY));";

    // You can only have one of: Help, ContextHelp
    if (node->prop_as_bool(prop_Help))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_HELP));";
    else if (node->prop_as_bool(prop_ContextHelp))
        code << "\n\t" << node->get_node_name() << "->AddButton(new wxButton(this, wxID_CONTEXT_HELP));";

    if (def_btn_name == "OK" || def_btn_name == "Yes" || def_btn_name == "Save")
        code << "\n\t" << node->get_node_name() << "->GetAffirmativeButton()->SetDefault();";
    else if (def_btn_name == "Cancel" || def_btn_name == "Close")
        code << "\n\t" << node->get_node_name() << "->GetCancelButton()->SetDefault();";
    else if (def_btn_name == "Apply")
        code << "\n\t" << node->get_node_name() << "->GetApplyButton()->SetDefault();";
    else if (def_btn_name == "No")
        code << "\n\t" << node->get_node_name() << "->GetNegativeButton()->SetDefault();";
    else if (def_btn_name == "Help" || def_btn_name == "ContextHelp")
        code << "\n\t" << node->get_node_name() << "->GetHelpButton()->SetDefault();";

    code << "\n\t" << node->get_node_name() << "->Realize();\n";

    if (!node->IsLocal())
    {
        if (node->prop_as_bool(prop_OK))
            code << node->get_node_name() << "OK = wxStaticCast(FindWindowById(wxID_OK), wxButton);\n";
        if (node->prop_as_bool(prop_Yes))
            code << node->get_node_name() << "Yes = wxStaticCast(FindWindowById(wxID_YES), wxButton);\n";
        if (node->prop_as_bool(prop_Save))
            code << node->get_node_name() << "Save = wxStaticCast(FindWindowById(wxID_SAVE), wxButton);\n";
        if (node->prop_as_bool(prop_Apply))
            code << node->get_node_name() << "Apply = wxStaticCast(FindWindowById(wxID_APPLY), wxButton);\n";

        if (node->prop_as_bool(prop_No))
            code << node->get_node_name() << "No = wxStaticCast(FindWindowById(wxID_NO), wxButton);\n";
        if (node->prop_as_bool(prop_Cancel))
            code << node->get_node_name() << "Cancel = wxStaticCast(FindWindowById(wxID_CANCEL), wxButton);\n";
        if (node->prop_as_bool(prop_Close))
            code << node->get_node_name() << "Close = wxStaticCast(FindWindowById(wxID_CLOSE), wxButton);\n";
        if (node->prop_as_bool(prop_Help))
            code << node->get_node_name() << "Help = wxStaticCast(FindWindowById(wxID_HELP), wxButton);\n";
        if (node->prop_as_bool(prop_ContextHelp))
            code << node->get_node_name() << "ContextHelp = wxStaticCast(FindWindowById(wxID_CONTEXT_HELP), wxButton);\n";
    }

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
        handler.Replace("[", "\n\t\t[");
        comma = ",\n\t";
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

wxObject* TextSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxTextSizerWrapper wrapper(wxStaticCast(parent, wxWindow));
    return wrapper.CreateSizer(node->prop_as_wxString(prop_text), node->prop_as_int(prop_wrap));
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

    if (parent->isGen(gen_wxDialog))
    {
        code << " = CreateTextSizer(";
    }
    else
    {
        code << " = wxTextSizerWrapper(" << parent->get_node_name() << ").CreateSizer(";
    }

    code << GenerateQuotedString(node->prop_as_string(prop_text)) << ", " << node->prop_as_string(prop_width) << ");";

    return code;
}

bool TextSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    auto parent = node->GetParent();
    while (parent->IsSizer())
        parent = parent->GetParent();

    if (parent->isGen(gen_wxDialog))
        InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/textwrapper.h>", set_src, set_hdr);

    return true;
}
