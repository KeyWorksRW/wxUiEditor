/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "import_xml.h"

#include "node.h"     // Node class
#include "uifuncs.h"  // Miscellaneous functions for displaying UI
#include "utils.h"    // Utility functions that work with properties

std::optional<pugi::xml_document> ImportXML::LoadDocFile(const ttString& file)
{
    pugi::xml_document doc;

    if (auto result = doc.load_file(file.wx_str()); !result)
    {
        appMsgBox(_ttc(strIdCantOpen) << file.wx_str() << "\n\n" << result.description(), _tt(strIdImportFormBuilder));
        return {};
    }

    m_importProjectFile = file;

    return doc;
}

void ImportXML::HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent)
{
    auto flag_value = xml_prop.text().as_cview();

    ttlib::cstr border_value;
    if (flag_value.contains("wxALL"))
        border_value = "wxALL";
    else
    {
        if (flag_value.contains("wxLEFT"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxLEFT";
        }
        if (flag_value.contains("wxRIGHT"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxRIGHT";
        }
        if (flag_value.contains("wxTOP"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxTOP";
        }
        if (flag_value.contains("wxBOTTOM"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxBOTTOM";
        }
    }

    if (border_value.size())
    {
        node->prop_set_value(prop_borders, border_value);
    }

    bool is_VerticalSizer = false;
    bool is_HorizontalSizer = false;

    if (parent && parent->IsSizer())
    {
        if (parent->prop_as_string(prop_orientation).contains("wxVERTICAL"))
            is_VerticalSizer = true;
        if (parent->prop_as_string(prop_orientation).contains("wxHORIZONTAL"))
            is_HorizontalSizer = true;
    }

    ttlib::cstr align_value;
    if (flag_value.contains("wxALIGN_LEFT") && !is_HorizontalSizer)
    {
        align_value << "wxALIGN_LEFT";
    }
    if (flag_value.contains("wxALIGN_TOP") && !is_VerticalSizer)
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_TOP";
    }
    if (flag_value.contains("wxALIGN_RIGHT") && !is_HorizontalSizer)
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_RIGHT";
    }
    if (flag_value.contains("wxALIGN_BOTTOM") && !is_VerticalSizer)
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_BOTTOM";
    }

    if (flag_value.contains("wxALIGN_CENTER") || flag_value.contains("wxALIGN_CENTRE"))
    {
        if (flag_value.contains("wxALIGN_CENTER_VERTICAL") || flag_value.contains("wxALIGN_CENTRE_VERTICAL"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER_VERTICAL";
        }
        else if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") || flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER_HORIZONTAL";
        }
        if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") || flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER_HORIZONTAL";
        }

        // Because we use contains(), all we know is that a CENTER flag was used, but not which one.
        // If we get here and no CENTER flag has been added, then assume that "wxALIGN_CENTER" or
        // "wxALIGN_CENTRE" was specified.

        if (!align_value.contains("wxALIGN_CENTER"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER";
        }
    }
    if (align_value.size())
    {
        auto prop = node->get_prop_ptr(prop_alignment);
        prop->set_value(align_value);
    }

    ttlib::cstr flags_value;
    if (flag_value.contains("wxEXPAND") || flag_value.contains("wxGROW"))
    {
        // Only add the flag if the expansion will happen in at least one of the directions.

        if (!(flag_value.contains("wxALIGN_BOTTOM") || flag_value.contains("wxALIGN_CENTER_VERTICAL")) &&
            !(flag_value.contains("wxALIGN_RIGHT") || flag_value.contains("wxALIGN_CENTER_HORIZONTAL")))
            flags_value << "wxEXPAND";
    }
    if (flag_value.contains("wxSHAPED"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxSHAPED";
    }
    if (flag_value.contains("wxFIXED_MINSIZE"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxFIXED_MINSIZE";
    }
    if (flag_value.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxRESERVE_SPACE_EVEN_IF_HIDDEN";
    }
    if (flag_value.contains("wxTILE"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxSHAPED|wxFIXED_MINSIZE";
    }
    if (flags_value.size())
    {
        auto prop = node->get_prop_ptr(prop_flags);
        prop->set_value(flags_value);
    }
}

void ImportXML::ProcessStyle(pugi::xml_node& xml_prop, Node* node, NodeProperty* prop)
{
    if (node->isGen(gen_wxListBox) || node->isGen(gen_wxCheckListBox))
    {
        // A list box selection type can only be single, multiple, or extended, so wxUiEditor stores this setting in a
        // type property so that the user can only choose one.

        ttlib::cstr style(xml_prop.text().as_string());
        if (style.contains("wxLB_SINGLE"))
        {
            node->prop_set_value(prop_type, "wxLB_SINGLE");
            if (style.contains("wxLB_SINGLE|"))
                style.Replace("wxLB_SINGLE|", "");
            else
                style.Replace("wxLB_SINGLE", "");
        }
        else if (style.contains("wxLB_MULTIPLE"))
        {
            node->prop_set_value(prop_type, "wxLB_MULTIPLE");
            if (style.contains("wxLB_MULTIPLE|"))
                style.Replace("wxLB_MULTIPLE|", "");
            else
                style.Replace("wxLB_MULTIPLE", "");
        }
        else if (style.contains("wxLB_EXTENDED"))
        {
            node->prop_set_value(prop_type, "wxLB_EXTENDED");
            if (style.contains("wxLB_EXTENDED|"))
                style.Replace("wxLB_EXTENDED|", "");
            else
                style.Replace("wxLB_EXTENDED", "");
        }
        prop->set_value(style);
    }
    else if (node->isGen(gen_wxRadioBox))
    {
        ttlib::cstr style(xml_prop.text().as_string());
        // It's a bug to specifiy both styles, we fix that here
        if (style.contains("wxRA_SPECIFY_ROWS") && style.contains("wxRA_SPECIFY_COLS"))
        {
            prop->set_value("wxRA_SPECIFY_ROWS");
        }
        else
        {
            prop->set_value(style);
        }
    }
    else if (node->isGen(gen_wxGauge))
    {
        // A list box selection type can only be single, multiple, or extended, so wxUiEditor stores this setting in a
        // type property so that the user can only choose one.

        ttlib::cstr style(xml_prop.text().as_string());
        if (style.contains("wxGA_VERTICAL"))
        {
            auto prop_type = node->get_prop_ptr(prop_orientation);
            prop_type->set_value("wxGA_VERTICAL");
            if (style.contains("wxGA_VERTICAL|"))
                style.Replace("wxGA_VERTICAL|", "");
            else
                style.Replace("wxGA_VERTICAL", "");

            // wxFormBuilder allows the user to specify both styles
            if (style.contains("wxGA_HORIZONTAL|"))
                style.Replace("wxGA_HORIZONTAL|", "");
            else
                style.Replace("wxGA_HORIZONTAL", "");
        }
        else if (style.contains("wxGA_HORIZONTAL"))
        {
            auto prop_type = node->get_prop_ptr(prop_orientation);
            prop_type->set_value("wxGA_HORIZONTAL");

            if (style.contains("wxGA_HORIZONTAL|"))
                style.Replace("wxGA_HORIZONTAL|", "");
            else
                style.Replace("wxGA_HORIZONTAL", "");
        }
        prop->set_value(style);
    }
    else if (node->isGen(gen_wxSlider))
    {
        // A list box selection type can only be single, multiple, or extended, so wxUiEditor stores this setting in a
        // type property so that the user can only choose one.

        ttlib::cstr style(xml_prop.text().as_string());
        if (style.contains("wxSL_HORIZONTAL"))
        {
            auto prop_type = node->get_prop_ptr(prop_orientation);
            prop_type->set_value("wxSL_HORIZONTAL");
            if (style.contains("wxSL_HORIZONTAL|"))
                style.Replace("wxSL_HORIZONTAL|", "");
            else
                style.Replace("wxSL_HORIZONTAL", "");
        }
        else if (style.contains("wxSL_VERTICAL"))
        {
            auto prop_type = node->get_prop_ptr(prop_orientation);
            prop_type->set_value("wxSL_VERTICAL");
            if (style.contains("wxSL_VERTICAL|"))
                style.Replace("wxSL_VERTICAL|", "");
            else
                style.Replace("wxSL_VERTICAL", "");
        }
        prop->set_value(style);
    }

    else
    {
        prop->set_value(xml_prop.text().as_cview());
    }
}

std::optional<GenName> ImportXML::ConvertToGenName(const ttlib::cstr& object_name, Node* parent)
{
    if (object_name == "wxBitmapButton")
    {
        return gen_wxButton;
    }
    else if (object_name.is_sameas("wxPanel"))
    {
        if (parent->DeclName().contains("book"))
            return gen_BookPage;
        else if (parent->isGen(gen_Project))
            return gen_PanelForm;
    }
    else if (object_name.contains("bookpage"))
    {
        return gen_oldbookpage;
    }
    else if (object_name.is_sameas("wxScintilla"))
    {
        return gen_wxStyledTextCtrl;
    }
    else if (object_name.is_sameas("wxListCtrl"))
    {
        return gen_wxListView;
    }
    else if (object_name.contains("Dialog"))
    {
        return gen_wxDialog;
    }
    else if (object_name.contains("Wizard") && parent->isGen(gen_Project))
    {
        return gen_wxWizard;
    }
    else if (object_name.contains("wxMenuBar") && parent->isGen(gen_Project))
    {
        return gen_MenuBar;
    }
    else if (object_name.contains("wxToolBar") && parent->isGen(gen_Project))
    {
        return gen_ToolBar;
    }
    else if (object_name.contains("Frame"))
    {
        return gen_wxFrame;
    }
    else if (object_name.contains("Panel") && parent->isGen(gen_Project))
    {
        return gen_PanelForm;
    }

    if (auto result = rmap_GenNames.find(object_name); result != rmap_GenNames.end())
        return result->second;

    return {};
}

// Call this AFTER the node has been hooked up to it's parent to prevent duplicate var_names.
void ImportXML::ProcessAttributes(const pugi::xml_node& xml_obj, Node* new_node)
{
    for (auto& iter: xml_obj.attributes())
    {
        if (iter.cname().is_sameas("name"))
        {
            if (new_node->IsForm())
            {
                if (auto prop = new_node->get_prop_ptr(prop_class_name); prop)
                {
                    prop->set_value(iter.value());
                }
            }
            else if (iter.as_cview().is_sameprefix("wxID_"))
            {
                auto prop = new_node->get_prop_ptr(prop_id);
                if (prop)
                {
                    prop->set_value(iter.value());
                }
                else if (prop = new_node->get_prop_ptr(prop_var_name); prop)
                {
                    prop->set_value(iter.value());
                }
            }

            else
            {
                // In a ImportXML file, name is the ID and variable is the var_name
                if (!xml_obj.attribute("variable").empty())
                {
                    if (auto prop = new_node->get_prop_ptr(prop_id); prop)
                    {
                        prop->set_value(iter.value());
                    }
                    continue;
                }

                if (auto prop = new_node->get_prop_ptr(prop_var_name); prop)
                {
                    ttlib::cstr org_name(iter.value());
                    auto new_name = new_node->GetUniqueName(org_name);
                    prop->set_value(new_name);
                }
            }
        }
        else if (iter.cname().is_sameas("variable"))
        {
            if (auto prop = new_node->get_prop_ptr(prop_var_name); prop)
            {
                ttlib::cstr org_name(iter.value());
                auto new_name = new_node->GetUniqueName(org_name);
                prop->set_value(new_name);
            }
        }
    }
}

void ImportXML::ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent)
{
    for (auto& iter: xml_obj.children())
    {
        if (iter.cname().is_sameas("object"))
        {
            continue;
        }

        // Start by processing names that wxUiEditor might use but that need special processing when importing.

        if (iter.cname().is_sameas("bitmap"))
        {
            ProcessBitmap(iter, node);
            continue;
        }
        else if (iter.cname().is_sameas("content"))
        {
            ProcessContent(iter, node);
            continue;
        }
        else if (iter.cname().is_sameas("value"))
        {
            auto escaped = ConvertEscapeSlashes(iter.text().as_string());
            if (auto prop = node->get_prop_ptr(iter.cname()); prop)
            {
                prop->set_value(escaped);
            }
            continue;
        }
        else if (iter.cname().is_sameas("label"))
        {
            ttlib::cstr label = ConvertEscapeSlashes(iter.text().as_string());
            label.Replace("_", "&");
            auto pos = label.find("\\t");
            if (ttlib::is_found(pos))
            {
                label[pos] = 0;
                node->prop_set_value(prop_shortcut, label.subview(pos + 2));
            }
            if (auto prop = node->get_prop_ptr(prop_label); prop)
            {
                prop->set_value(label);
            }
            continue;
        }
        else if (iter.cname().is_sameas("option"))
        {
            if (auto prop = node->get_prop_ptr(prop_proportion); prop)
            {
                prop->set_value(iter.text().as_string());
                continue;
            }
        }

        // Now process names that are identical.

        auto prop = node->get_prop_ptr(iter.cname());
        if (prop)
        {
            prop->set_value(iter.text().as_string());
            continue;
        }

        // Finally, process names that are unique to XRC/ImportXML

        if (iter.cname().is_sameas("orient"))
        {
            prop = node->get_prop_ptr(prop_orientation);
            if (prop)
            {
                prop->set_value(iter.text().as_string());
            }
        }
        else if (iter.cname().is_sameas("border"))
        {
            node->prop_set_value(prop_border_size, iter.text().as_string());
        }
        else if (iter.cname().is_sameas("flag") && (node->isGen(gen_sizeritem) || node->isGen(gen_gbsizeritem)))
        {
            HandleSizerItemProperty(iter, node, parent);
        }
        else if (iter.cname().is_sameas("handler"))
        {
            ProcessHandler(iter, node);
        }
    }
}

void ImportXML::ProcessContent(const pugi::xml_node& xml_obj, Node* node)
{
    ttlib::cstr choices;
    for (auto& iter: xml_obj.children())
    {
        if (iter.cname().is_sameas("item"))
        {
            auto child = iter.child_as_cstr();
            child.Replace("\"", "\\\"", true);
            if (choices.size())
                choices << " ";
            choices << '\"' << child << '\"';
        }
    }

    if (choices.size())
        node->prop_set_value(prop_choices, choices);
}

void ImportXML::ProcessBitmap(const pugi::xml_node& xml_obj, Node* node)
{
    if (!xml_obj.attribute("stock_id").empty())
    {
        ttlib::cstr bitmap("Art; ");
        bitmap << xml_obj.attribute("stock_id").value() << "; ";
        if (!xml_obj.attribute("stock_client").empty())
            bitmap << xml_obj.attribute("stock_client").value();
        bitmap << "; [-1; -1]";

        if (auto prop = node->get_prop_ptr(prop_bitmap); prop)
        {
            prop->set_value(bitmap);
        }
    }
    else
    {
        auto file = xml_obj.child_as_cstr();
        if (file.contains(".xpm", tt::CASE::either))
        {
            ttlib::cstr bitmap("XPM; ");
            bitmap << file;
            bitmap << "; ; [-1; -1]";

            if (auto prop = node->get_prop_ptr(prop_bitmap); prop)
            {
                prop->set_value(bitmap);
                if (node->isGen(gen_wxButton))
                    node->prop_set_value(prop_markup, true);
            }
        }
    }
}

void ImportXML::ProcessHandler(const pugi::xml_node& xml_obj, Node* node)
{
    if (xml_obj.attribute("function").empty() || xml_obj.attribute("entry").empty())
        return;

    ttlib::cstr event_name("wx");
    event_name << xml_obj.attribute("entry").value();
    auto event = node->GetEvent(event_name);
    if (event)
    {
        event->set_value(xml_obj.attribute("function").value());
        return;
    }
}
