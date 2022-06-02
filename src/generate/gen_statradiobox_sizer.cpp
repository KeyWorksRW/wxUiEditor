/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBoxSizer with wxRadioButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_statradiobox_sizer.h"

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

    if (auto result = GenValidatorSettings(node); result)
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

    code << node->get_node_name() << " = new wxStaticBoxSizer(new wxStaticBox(" << parent_name << ", wxID_ANY,";
    if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
    {
        code << "\n#if wxCHECK_VERSION(3, 1, 1)\n\t";
        code << node->prop_as_string(prop_radiobtn_var_name) << "),";
        code << "\n#else\n\t";
        code << "wxEmptyString),";
        code << "\n#endif\n";
        code << node->prop_as_string(prop_orientation) << ");";
    }
    else
    {
        code << node->prop_as_string(prop_radiobtn_var_name) << "), " << node->prop_as_string(prop_orientation) << ");";
    }

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
