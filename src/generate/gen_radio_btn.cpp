/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRadioButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/manager.h>  // wxPropertyGridManager
#include <wx/radiobut.h>          // wxRadioButton declaration

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_radio_btn.h"

wxObject* RadioButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRadioButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                    DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->prop_as_bool(prop_checked))
        widget->SetValue(true);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RadioButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_label))
    {
        wxStaticCast(widget, wxRadioButton)->SetLabel(node->prop_as_wxString(prop_label));
        return true;
    }
    else if (prop->isProp(prop_checked))
    {
        wxStaticCast(widget, wxRadioButton)->SetValue(prop->as_bool());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> RadioButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->prop_as_string(prop_label).size())
        code << GenerateQuotedString(node->prop_as_string(prop_label));
    else
        code << "wxEmptyString";

    if (node->prop_as_string(prop_window_name).empty())
        GeneratePosSizeFlags(node, code);
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, since GenSettings will create it, but we have to supply something before the window
        // name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> RadioButtonGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // If a validator has been specified, then the variable will be initialized with the selection variable.
    if (node->prop_as_string(prop_validator_variable).empty())
    {
        if (node->prop_as_bool(prop_checked))
        {
            code << node->get_node_name() << "->SetValue(true);";
        }
    }

    return code;
}

std::optional<ttlib::cstr> RadioButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int RadioButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRadioButton");

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_checked, "value")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void RadioButtonGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxRadioButtonXmlHandler");
}

bool RadioButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/radiobut.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);

    return true;
}

bool RadioButtonGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_style))
    {
        if (m_info_warning)
        {
            wxGetFrame().GetPropInfoBar()->Dismiss();
            m_info_warning = false;
        }

        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        ttString newValue = property->ValueToString(variant);

        if (newValue.contains("wxRB_GROUP"))
        {
            auto parent = node->GetParent();
            auto pos = parent->GetChildPosition(node);
            if (pos > 0 && parent->GetChild(pos - 1)->isGen(gen_wxRadioButton) &&
                parent->GetChild(pos - 1)->prop_as_string(prop_style).contains("wxRB_GROUP"))
            {
                auto info = wxGetFrame().GetPropInfoBar();
                info->ShowMessage("The previous radio button is also set as the start of a group!", wxICON_INFORMATION);
                m_info_warning = true;
            }
            else if (pos + 1 < parent->GetChildCount() && parent->GetChild(pos + 1)->isGen(gen_wxRadioButton) &&
                     parent->GetChild(pos + 1)->prop_as_string(prop_style).contains("wxRB_GROUP"))
            {
                auto info = wxGetFrame().GetPropInfoBar();
                info->ShowMessage("The next radio button is also set as the start of a group!", wxICON_INFORMATION);
                m_info_warning = true;
            }
        }

        // Note that we allow this property change since we don't know which radio button the user will want to change (none
        // if they plan on adding more radio buttons in between the two groups)
        return true;
    }
    else
    {
        return BaseGenerator::AllowPropertyChange(event, prop, node);
    }
}

void RadioButtonGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop)
{
    if (changed_prop->isProp(prop_style))
    {
        if (auto pg_parent = prop_grid->GetProperty("style"); pg_parent)
        {
            for (unsigned int idx = 0; idx < pg_parent->GetChildCount(); ++idx)
            {
                if (auto pg_setting = pg_parent->Item(idx); pg_setting)
                {
                    auto label = pg_setting->GetLabel();
                    if (label == "wxRB_GROUP")
                    {
                        pg_setting->Enable(!changed_prop->as_string().contains("wxRB_SINGLE"));
                    }
                    else if (label == "wxRB_SINGLE")
                    {
                        pg_setting->Enable(!changed_prop->as_string().contains("wxRB_GROUP"));
                    }
                }
            }
        }
    }
    else
    {
        BaseGenerator::ChangeEnableState(prop_grid, changed_prop);
    }
}
