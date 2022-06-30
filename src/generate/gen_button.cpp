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
    auto widget = new wxButton(wxStaticCast(parent, wxWindow), node->prop_as_id(prop_id), wxEmptyString,
                               DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_label))
    {
        if (node->prop_as_bool(prop_markup))
            widget->SetLabelMarkup(node->prop_as_wxString(prop_label));
        else
            widget->SetLabel(node->prop_as_wxString(prop_label));
    }

    if (node->prop_as_bool(prop_default))
    {
        widget->SetDefault();
        if (auto dlg = wxDynamicCast(parent, wxDialog); dlg && node->prop_as_id(prop_id) != wxID_ANY)
            dlg->SetAffirmativeId(node->prop_as_id(prop_id));
    }
    else
    {
        if (auto dlg = wxDynamicCast(parent, wxDialog); dlg && node->prop_as_id(prop_id) != wxID_ANY)
        {
            switch (node->prop_as_id(prop_id))
            {
                case wxID_OK:
                case wxID_YES:
                case wxID_SAVE:
                    dlg->SetAffirmativeId(node->prop_as_id(prop_id));
                    break;

                case wxID_CANCEL:
                case wxID_CLOSE:
                case wxID_NO:
                    dlg->SetEscapeId(node->prop_as_id(prop_id));
                    break;

                default:
                    break;
            }
        }
    }

    if (node->prop_as_bool(prop_auth_needed))
        widget->SetAuthNeeded();

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->prop_as_wxBitmapBundle(prop_bitmap));

        if (node->HasValue(prop_disabled_bmp))
            widget->SetBitmapDisabled(node->prop_as_wxBitmapBundle(prop_disabled_bmp));

        if (node->HasValue(prop_pressed_bmp))
            widget->SetBitmapPressed(node->prop_as_wxBitmapBundle(prop_pressed_bmp));

        if (node->HasValue(prop_focus_bmp))
            widget->SetBitmapFocus(node->prop_as_wxBitmapBundle(prop_focus_bmp));

        if (node->HasValue(prop_current))
            widget->SetBitmapCurrent(node->prop_as_wxBitmapBundle(prop_current));

        if (node->HasValue(prop_position))
            widget->SetBitmapPosition(static_cast<wxDirection>(node->prop_as_int(prop_position)));

        if (node->HasValue(prop_margins))
            widget->SetBitmapMargins(node->prop_as_wxSize(prop_margins));
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
        if (node->prop_as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->prop_as_wxString(prop_label));
        else
            ctrl->SetLabel(node->prop_as_wxString(prop_label));

        return true;
    }
    else if (prop->isProp(prop_markup))
    {
        // Turning markup on switches to generic rending of the button. However, you have to recreate it to switch it
        // off and go back to native rendering.

        if (node->prop_as_bool(prop_markup))
        {
            wxStaticCast(widget, wxButton)->SetLabelMarkup(node->prop_as_wxString(prop_label));
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

std::optional<ttlib::cstr> ButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->HasValue(prop_label))
    {
        if (!node->prop_as_bool(prop_markup))
        {
            code << GenerateQuotedString(node, prop_label);
        }
        else
        {
            // prop_markup is set, so the actual label will be set in GenSettings()
            code << "wxEmptyString";
        }
    }

    GeneratePosSizeFlags(node, code, true);
    if (code.contains("wxEmptyString)"))
        code.Replace(", wxEmptyString", "");

    return code;
}

std::optional<ttlib::cstr> ButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> ButtonGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_markup) && node->HasValue(prop_label))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetLabelMarkup(" << GenerateQuotedString(node->prop_as_string(prop_label))
             << ");";
    }

    if (node->prop_as_bool(prop_default))
    {
        if (code.size())
            code << '\n';

        code << node->get_node_name() << "->SetDefault();";
    }

    if (node->prop_as_bool(prop_auth_needed))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetAuthNeeded();";
    }

    if (node->HasValue(prop_bitmap))
    {
        auto_indent = indent::auto_keep_whitespace;

        if (node->HasValue(prop_position))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapPosition(" << node->prop_as_string(prop_position) << ");";
        }

        if (node->HasValue(prop_margins))
        {
            if (code.size())
                code << '\n';
            auto size = node->prop_as_wxSize(prop_margins);
            code << node->get_node_name() << "->SetBitmapMargins(" << size.GetWidth() << ", " << size.GetHeight() << ");";
        }

        GenBtnBimapCode(node, code);
    }
    return code;
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
        if (node->prop_as_bool(prop_markup))
        {
            ADD_ITEM_COMMENT(" markup cannot be be set in the XRC file. ")
        }
        if (node->prop_as_bool(prop_auth_needed))
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
