/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRadioButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/manager.h>  // wxPropertyGridManager
#include <wx/radiobut.h>          // wxRadioButton declaration

#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_radio_btn.h"

wxObject* RadioButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRadioButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_label),
                                    DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));

    if (node->as_bool(prop_checked))
        widget->SetValue(true);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RadioButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_label))
    {
        wxStaticCast(widget, wxRadioButton)->SetLabel(node->as_wxString(prop_label));
        return true;
    }
    else if (prop->isProp(prop_checked))
    {
        wxStaticCast(widget, wxRadioButton)->SetValue(prop->as_bool());
        return true;
    }

    return false;
}

bool RadioButtonGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
    code.PosSizeFlags(code::allow_scaling, true);

    return true;
}

bool RadioButtonGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_checked))
    {
        code.NodeName().Function("SetValue(").True().EndFunction();
    }
    return true;
}

int RadioButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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

bool RadioButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                       GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/radiobut.h>", set_src, set_hdr);
    if (node->as_string(prop_validator_variable).size())
        set_src.insert("#include <wx/valgen.h>");

    return true;
}

bool RadioButtonGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_style))
    {
        if (m_info_warning)
        {
            wxGetFrame().DismissInfoBar();
            m_info_warning = false;
        }

        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();

        if (newValue.contains("wxRB_GROUP"))
        {
            auto parent = node->getParent();
            auto pos = parent->getChildPosition(node);
            if (pos > 0 && parent->getChild(pos - 1)->isGen(gen_wxRadioButton) &&
                parent->getChild(pos - 1)->as_string(prop_style).contains("wxRB_GROUP"))
            {
                wxGetFrame().ShowInfoBarMsg("The previous radio button is also set as the start of a group!",
                                            wxICON_INFORMATION);
                m_info_warning = true;
            }
            else if (pos + 1 < parent->getChildCount() && parent->getChild(pos + 1)->isGen(gen_wxRadioButton) &&
                     parent->getChild(pos + 1)->as_string(prop_style).contains("wxRB_GROUP"))
            {
                wxGetFrame().ShowInfoBarMsg("The next radio button is also set as the start of a group!",
                                            wxICON_INFORMATION);
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
