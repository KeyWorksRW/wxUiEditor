/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>  // wxButtonBase class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties
#include "write_code.h"  // Write code to Scintilla or file

#include "gen_button.h"

wxObject* ButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxButton(wxStaticCast(parent, wxWindow), node->as_id(prop_id), wxEmptyString,
                               DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_label))
    {
        if (node->as_bool(prop_markup))
            widget->SetLabelMarkup(node->as_wxString(prop_label));
        else
            widget->SetLabel(node->as_wxString(prop_label));
    }

    if (node->as_bool(prop_default))
    {
        widget->SetDefault();
        if (auto dlg = wxDynamicCast(parent, wxDialog); dlg && node->as_id(prop_id) != wxID_ANY)
            dlg->SetAffirmativeId(node->as_id(prop_id));
    }
    else
    {
        if (auto dlg = wxDynamicCast(parent, wxDialog); dlg && node->as_id(prop_id) != wxID_ANY)
        {
            switch (node->as_id(prop_id))
            {
                case wxID_OK:
                case wxID_YES:
                case wxID_SAVE:
                    dlg->SetAffirmativeId(node->as_id(prop_id));
                    break;

                case wxID_CANCEL:
                case wxID_CLOSE:
                case wxID_NO:
                    dlg->SetEscapeId(node->as_id(prop_id));
                    break;

                default:
                    break;
            }
        }
    }

    if (node->as_bool(prop_auth_needed))
        widget->SetAuthNeeded();

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->as_wxBitmapBundle(prop_bitmap));

        if (node->HasValue(prop_disabled_bmp))
            widget->SetBitmapDisabled(node->as_wxBitmapBundle(prop_disabled_bmp));

        if (node->HasValue(prop_pressed_bmp))
            widget->SetBitmapPressed(node->as_wxBitmapBundle(prop_pressed_bmp));

        if (node->HasValue(prop_focus_bmp))
            widget->SetBitmapFocus(node->as_wxBitmapBundle(prop_focus_bmp));

        if (node->HasValue(prop_current))
            widget->SetBitmapCurrent(node->as_wxBitmapBundle(prop_current));

        if (node->HasValue(prop_position))
            widget->SetBitmapPosition(static_cast<wxDirection>(node->as_int(prop_position)));

        if (node->HasValue(prop_margins))
            widget->SetBitmapMargins(node->as_wxSize(prop_margins));
    }

    if (!node->isPropValue(prop_variant, "normal"))
    {
        if (node->isPropValue(prop_variant, "small"))
            widget->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
        else if (node->isPropValue(prop_variant, "mini"))
            widget->SetWindowVariant(wxWINDOW_VARIANT_MINI);
        else
            widget->SetWindowVariant(wxWINDOW_VARIANT_LARGE);
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    // In case the widget hasn't been fully specified yet
    if (!widget || !node || !prop)
        return false;

    // We do not support changing the "markup" property because while the control displays correctly when markup is set,
    // it does not revert when markup is cleared (at least on Windows where markup controls whether a generic or native
    // version of the button is displayed).

    if (prop->isProp(prop_label) && prop->HasValue())
    {
        auto ctrl = wxStaticCast(widget, wxButton);
        if (node->as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->as_wxString(prop_label));
        else
            ctrl->SetLabel(node->as_wxString(prop_label));

        return true;
    }
    else if (prop->isProp(prop_markup))
    {
        // Turning markup on switches to generic rending of the button. However, you have to recreate it to switch it
        // off and go back to native rendering.

        if (node->as_bool(prop_markup))
        {
            wxStaticCast(widget, wxButton)->SetLabelMarkup(node->as_wxString(prop_label));
            return true;
        }
    }
    else if (prop->isProp(prop_default))
    {
        // You can change a button to be the default, but you cannot change it back without recreating it.
        if (prop->as_bool())
        {
            wxStaticCast(widget, wxButton)->SetDefault();
            return true;
        }
    }

    return false;
}

bool ButtonGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();

    // If prop_markup is set, then the label will be set in SettingsCode()
    if (code.HasValue(prop_label) && !code.IsTrue(prop_markup))
    {
        code.QuotedString(prop_label);
    }
    else
    {
        code.Add("wxEmptyString");
    }

    code.PosSizeFlags(true);

    return true;
}

bool ButtonGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_markup) && code.HasValue(prop_label))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetLabelMarkup(").QuotedString(prop_label).EndFunction();
    }

    if (code.IsTrue(prop_default))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetDefault(").EndFunction();
    }

    if (code.IsTrue(prop_auth_needed))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetAuthNeeded(").EndFunction();
    }

    if (code.HasValue(prop_bitmap))
    {
        if (code.HasValue(prop_position))
        {
            code.Eol(eol_if_needed).NodeName().Function("SetBitmapPosition(").Str(prop_position).EndFunction();
        }

        if (code.HasValue(prop_margins))
        {
            auto size = code.node()->as_wxSize(prop_margins);
            code.Eol(eol_if_needed)
                .NodeName()
                .Function("SetBitmapMargins(")
                .itoa(size.GetWidth())
                .Comma()
                .itoa(size.GetHeight())
                .EndFunction();
        }

        if (code.is_cpp())
            GenBtnBimapCode(code.node(), code);
        else
            PythonBtnBimapCode(code);
    }

    return true;
}

int ButtonGenerator::GetRequiredVersion(Node* node)
{
    // Code generation was invalid in minRequiredVer when there no label was set
    if (!node->HasValue(prop_label) && !node->as_bool(prop_markup))
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }

    return BaseGenerator::GetRequiredVersion(node);
}

int ButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxButton");

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_markup, "markup")
    ADD_ITEM_BOOL(prop_default, "default")

    GenXrcBitmap(node, item, xrc_flags);
    GenXrcStylePosSize(node, item);

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_bool(prop_markup))
        {
            ADD_ITEM_COMMENT(" markup cannot be be set in the XRC file. ")
        }
        if (node->as_bool(prop_auth_needed))
        {
            ADD_ITEM_COMMENT(" authentication cannot be be set in the XRC file. ")
        }
        GenXrcComments(node, item);
    }

    return result;
}

void ButtonGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    bool old_button = (node->HasValue(prop_disabled_bmp) || node->HasValue(prop_pressed_bmp) ||
                       node->HasValue(prop_focus_bmp) || node->HasValue(prop_current));

    handlers.emplace(old_button ? "wxBitmapButtonXmlHandler" : "wxButtonXmlHandler");
}

bool ButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    return true;
}
