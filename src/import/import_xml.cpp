/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <frozen/map.h>

#include "import_xml.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "gen_enums.h"       // Enumerations for generators
#include "mainframe.h"       // Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "utils.h"           // Utility functions that work with properties

using namespace GenEnum;

namespace xrc_import
{
    constexpr auto map_import_prop_names = frozen::make_map<std::string_view, GenEnum::PropName>({
        { "accel", prop_shortcut },
        { "art-provider", prop_art_provider },
        { "bg", prop_background_colour },
        { "bitmap-bg", prop_bmp_background_colour },
        { "bitmap-minwidth", prop_bmp_min_width },
        { "bitmap-placement", prop_bmp_placement },
        { "bitmapposition", prop_position },
        { "bitmapsize", prop_image_size },  // BUGBUG: [Randalphwa - 06-17-2022] should this be prop_bitmapsize?
        { "choices", prop_contents },
        { "class", prop_class_name },
        { "content", prop_contents },
        { "defaultdirectory", prop_initial_folder },
        { "defaultfilename", prop_initial_filename },
        { "dimension", prop_majorDimension },
        { "effectduration", prop_duration },
        { "empty_cellsize", prop_empty_cell_size },
        { "extra-accels", prop_extra_accels },
        { "fg", prop_foreground_colour },
        { "flexibledirection", prop_flexible_direction },
        { "gradient-end", prop_end_colour },
        { "gradient-start", prop_start_colour },
        { "gravity", prop_sashgravity },
        { "hideeffect", prop_hide_effect },
        { "hover", prop_current },
        { "htmlcode", prop_html_content },
        { "inactive-bitmap", prop_inactive_bitmap },
        { "include_file", prop_derived_header },
        { "linesize", prop_line_size },
        { "longhelp", prop_statusbar },  // Used by toolbar tools
        { "minsize", prop_min_size },
        { "nonflexiblegrowmode", prop_non_flexible_grow_mode },
        { "pagesize", prop_page_size },
        { "running", prop_auto_start },
        { "selmax", prop_sel_end },
        { "selmin", prop_sel_start },
        { "settings", prop_settings_code },
        { "showeffect", prop_show_effect },
        { "tab_ctrl_height", prop_tab_height },
        { "thumb", prop_thumb_length },
        { "tickfreq", prop_tick_frequency },
        { "windowlabel", prop_tab_height },
        { "wrapmode", prop_stc_wrap_mode },
    });

    constexpr auto import_GenNames = frozen::make_map<std::string_view, GenEnum::GenName>({
        { "Custom", gen_CustomControl },
        { "CustomWidget", gen_CustomControl },
        { "Dialog", gen_wxDialog },
        { "Frame", gen_wxFrame },
        { "Panel", gen_PanelForm },
        { "Wizard", gen_wxWizard },
        { "WizardPageSimple", gen_wxWizardPageSimple },
        { "bookpage", gen_oldbookpage },
        { "panewindow", gen_VerticalBoxSizer },
        { "unknown", gen_CustomControl },
        { "wxBitmapButton", gen_wxButton },
        { "wxListCtrl", gen_wxListView },
        { "wxScintilla", gen_wxStyledTextCtrl },

        // DialogBlocks proxy conversion
        { "wxSpacer", gen_spacer },
        { "wxMenuSeparator", gen_separator },
        { "wxSubmenu", gen_submenu },
        { "wxToolBarSeparator", gen_toolSeparator },
        { "wxToolBarButton", gen_tool },
    });

    constexpr auto map_old_events = frozen::make_map<std::string_view, std::string_view>({
        { "wxEVT_COMMAND_BUTTON_CLICKED", "wxEVT_BUTTON" },
        { "wxEVT_COMMAND_CHECKBOX_CLICKED", "wxEVT_CHECKBOX" },
        { "wxEVT_COMMAND_CHECKLISTBOX_TOGGLED", "wxEVT_CHECKLISTBOX" },
        { "wxEVT_COMMAND_CHOICE_SELECTED", "wxEVT_CHOICE" },
        { "wxEVT_COMMAND_COMBOBOX_CLOSEUP", "wxEVT_COMBOBOX_CLOSEUP" },
        { "wxEVT_COMMAND_COMBOBOX_DROPDOWN", "wxEVT_COMBOBOX_DROPDOWN" },
        { "wxEVT_COMMAND_COMBOBOX_SELECTED", "wxEVT_COMBOBOX" },
        { "wxEVT_COMMAND_LISTBOX_DOUBLECLICKED", "wxEVT_LISTBOX_DCLICK" },
        { "wxEVT_COMMAND_LISTBOX_SELECTED", "wxEVT_LISTBOX" },
        { "wxEVT_COMMAND_MENU_SELECTED", "wxEVT_MENU" },
        { "wxEVT_COMMAND_RADIOBOX_SELECTED", "wxEVT_RADIOBOX" },
        { "wxEVT_COMMAND_RADIOBUTTON_SELECTED", "wxEVT_RADIOBUTTON" },
        { "wxEVT_COMMAND_SCROLLBAR_UPDATED", "wxEVT_SCROLLBAR" },
        { "wxEVT_COMMAND_SLIDER_UPDATED", "wxEVT_SLIDER" },
        { "wxEVT_COMMAND_TEXT_COPY", "wxEVT_TEXT_COPY" },
        { "wxEVT_COMMAND_TEXT_CUT", "wxEVT_TEXT_CUT" },
        { "wxEVT_COMMAND_TEXT_ENTER", "wxEVT_TEXT_ENTER" },
        { "wxEVT_COMMAND_TEXT_MAXLEN", "wxEVT_TEXT_MAXLEN" },
        { "wxEVT_COMMAND_TEXT_PASTE", "wxEVT_TEXT_PASTE" },
        { "wxEVT_COMMAND_TEXT_UPDATED", "wxEVT_TEXT" },
        { "wxEVT_COMMAND_TEXT_URL", "wxEVT_TEXT_URL" },
        { "wxEVT_COMMAND_THREAD", "wxEVT_THREAD" },
        { "wxEVT_COMMAND_TOOL_CLICKED", "wxEVT_TOOL" },
        { "wxEVT_COMMAND_TOOL_DROPDOWN_CLICKED", "wxEVT_TOOL_DROPDOWN" },
        { "wxEVT_COMMAND_TOOL_ENTER", "wxEVT_TOOL_ENTER" },
        { "wxEVT_COMMAND_TOOL_RCLICKED", "wxEVT_TOOL_RCLICKED" },
        { "wxEVT_COMMAND_VLBOX_SELECTED", "wxEVT_VLBOX" },
    });

};  // namespace xrc_import

using namespace xrc_import;

std::optional<pugi::xml_document> ImportXML::LoadDocFile(const tt_string& file)
{
    pugi::xml_document doc;

    if (auto result = doc.load_file(file.c_str()); !result)
    {
        wxMessageBox(wxString("Cannot open ") << file.make_wxString() << "\n\n"
                                              << result.description(),
                     "Import wxFormBuilder project");
        return {};
    }

    m_importProjectFile = file;

    return doc;
}

void ImportXML::HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent)
{
    auto flag_value = xml_prop.text().as_sview();

    tt_string border_value;
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

    // Always set this even if it is empty
    node->set_value(prop_borders, border_value);

    bool is_VerticalSizer = false;
    bool is_HorizontalSizer = false;

    if (parent && parent->isSizer())
    {
        if (parent->as_string(prop_orientation).contains("wxVERTICAL"))
            is_VerticalSizer = true;
        if (parent->as_string(prop_orientation).contains("wxHORIZONTAL"))
            is_HorizontalSizer = true;
    }

    tt_string align_value;
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
        node->set_value(prop_alignment, align_value);
    }

    tt_string flags_value;
    if (flag_value.contains("wxEXPAND") || flag_value.contains("wxGROW"))
    {
        // You can't use wxEXPAND with any alignment flags
        node->set_value(prop_alignment, "");
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
        node->set_value(prop_flags, flags_value);
    }
}

void ImportXML::ProcessStyle(pugi::xml_node& xml_prop, Node* node, NodeProperty* prop)
{
    if (node->isGen(gen_wxListBox) || node->isGen(gen_wxCheckListBox))
    {
        // A list box selection type can only be single, multiple, or extended, so wxUiEditor stores this setting in a
        // type property so that the user can only choose one.

        tt_string style(xml_prop.text().as_string());
        if (style.contains("wxLB_SINGLE"))
        {
            node->set_value(prop_type, "wxLB_SINGLE");
            if (style.contains("wxLB_SINGLE|"))
                style.Replace("wxLB_SINGLE|", "");
            else
                style.Replace("wxLB_SINGLE", "");
        }
        else if (style.contains("wxLB_MULTIPLE"))
        {
            node->set_value(prop_type, "wxLB_MULTIPLE");
            if (style.contains("wxLB_MULTIPLE|"))
                style.Replace("wxLB_MULTIPLE|", "");
            else
                style.Replace("wxLB_MULTIPLE", "");
        }
        else if (style.contains("wxLB_EXTENDED"))
        {
            node->set_value(prop_type, "wxLB_EXTENDED");
            if (style.contains("wxLB_EXTENDED|"))
                style.Replace("wxLB_EXTENDED|", "");
            else
                style.Replace("wxLB_EXTENDED", "");
        }
        prop->set_value(style);
    }
    else if (node->isGen(gen_wxRadioBox))
    {
        tt_string style(xml_prop.text().as_string());
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

        tt_string style(xml_prop.text().as_string());
        if (style.contains("wxGA_VERTICAL"))
        {
            auto prop_type = node->getPropPtr(prop_orientation);
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
            auto prop_type = node->getPropPtr(prop_orientation);
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

        tt_string style(xml_prop.text().as_string());
        if (style.contains("wxSL_HORIZONTAL"))
        {
            auto prop_type = node->getPropPtr(prop_orientation);
            prop_type->set_value("wxSL_HORIZONTAL");
            if (style.contains("wxSL_HORIZONTAL|"))
                style.Replace("wxSL_HORIZONTAL|", "");
            else
                style.Replace("wxSL_HORIZONTAL", "");
        }
        else if (style.contains("wxSL_VERTICAL"))
        {
            auto prop_type = node->getPropPtr(prop_orientation);
            prop_type->set_value("wxSL_VERTICAL");
            if (style.contains("wxSL_VERTICAL|"))
                style.Replace("wxSL_VERTICAL|", "");
            else
                style.Replace("wxSL_VERTICAL", "");
        }
        prop->set_value(style);
    }
    else if (node->isGen(gen_wxFontPickerCtrl))
    {
        tt_string style(xml_prop.text().as_string());
        if (style.contains("wxFNTP_DEFAULT_STYLE"))
        {
            node->set_value(prop_style, "wxFNTP_FONTDESC_AS_LABEL|wxFNTP_USEFONT_FOR_LABEL");
        }
    }
    else if (node->isGen(gen_wxListView))
    {
        tt_string style(xml_prop.text().as_string());
        tt_string_vector mstr(style, '|');
        style.clear();
        for (auto& iter: mstr)
        {
            if (iter.starts_with("wxLC_ICON") || iter.starts_with("wxLC_SMALL_ICON") || iter.starts_with("wxLC_LIST") ||
                iter.starts_with("wxLC_REPORT"))
            {
                node->set_value(prop_mode, iter);
            }
            else
            {
                if (style.size())
                {
                    style << '|';
                }
                style << iter;
            }
        }
        if (style.size())
            prop->set_value(style);
    }
    else if (node->isGen(gen_wxToolBar))
    {
        tt_string style(xml_prop.text().as_string());
        style.Replace("wxAUI_TB_DEFAULT_STYLE", "wxTB_HORIZONTAL");
        style.Replace("wxAUI_TB_HORZ_LAYOUT", "wxTB_HORZ_LAYOUT");
        style.Replace("wxAUI_TB_TEXT", "wxTB_TEXT");
        style.Replace("wxAUI_TB_VERTICAL", "wxTB_VERTICAL");
        style.Replace("wxAUI_TB_NO_TOOLTIPS", "wxTB_NO_TOOLTIPS");
        style.Replace("wxAUI_TB_NO_TOOLTIPS", "wxTB_NO_TOOLTIPS");
        if (style.size())
            prop->set_value(style);
    }
    else
    {
        auto view_value = xml_prop.text().as_sview();
        if (view_value.contains("wxST_SIZEGRIP"))
        {
            auto value = xml_prop.text().as_cstr();
            value.Replace("wxST_SIZEGRIP", "wxSTB_SIZEGRIP");
            prop->set_value(value);
        }
        else if (view_value.contains("wxTE_CENTRE"))
        {
            auto value = xml_prop.text().as_cstr();
            value.Replace("wxTE_CENTRE", "wxTE_CENTER");
            prop->set_value(value);
        }

        // Eliminate obsolete styles

        else if (view_value.contains("wxBU_AUTODRAW"))
        {
            auto value = xml_prop.text().as_cstr();
            value.Replace("wxBU_AUTODRAW", "");  // this style is obsolete
            if (value.size())
            {
                prop->set_value(value);
            }
        }
        else if (view_value.contains("wxRA_USE_CHECKBOX"))
        {
            auto value = xml_prop.text().as_cstr();
            value.Replace("wxRA_USE_CHECKBOX", "");  // this style is obsolete
            if (value.size())
            {
                prop->set_value(value);
            }
        }
        else if (view_value.contains("wxRB_USE_CHECKBOX"))
        {
            auto value = xml_prop.text().as_cstr();
            value.Replace("wxRB_USE_CHECKBOX", "");  // this style is obsolete
            if (value.size())
            {
                prop->set_value(value);
            }
        }
        else if (view_value.contains("wxNB_FLAT"))
        {
            auto value = xml_prop.text().as_cstr();
            value.Replace("wxNB_FLAT", "");  // this style is obsolete
            if (value.size())
            {
                prop->set_value(value);
            }
        }

        else
        {
            prop->set_value(view_value);
        }
    }
}

GenEnum::GenName ImportXML::ConvertToGenName(const tt_string& object_name, Node* parent)
{
    auto getGenName = MapClassName(object_name);

    if (getGenName == gen_wxPanel)
    {
        if (!parent)
        {
            auto owner = wxGetFrame().getSelectedNode();
            while (owner->getGenType() == type_sizer)
                owner = owner->getParent();
            if (owner->declName().contains("book"))
            {
                return gen_BookPage;
            }
            else
            {
                return gen_PanelForm;
            }
        }
        else if (parent->declName().contains("book"))
            return gen_BookPage;
        else if (parent->isGen(gen_Project))
            return gen_PanelForm;
    }
    else if (getGenName == gen_sizeritem && parent && parent->isGen(gen_wxGridBagSizer))
    {
        return gen_gbsizeritem;
    }
    else if (object_name.contains("Panel") && parent && parent->isGen(gen_Project))
    {
        return gen_PanelForm;
    }
    else if (getGenName == gen_separator && parent &&
             (parent->isGen(gen_wxToolBar) || parent->isGen(gen_ToolBar) || parent->isGen(gen_wxAuiToolBar)))
    {
        return gen_toolSeparator;
    }
    else if (getGenName == gen_tool && parent && parent->isGen(gen_wxAuiToolBar))
    {
        getGenName = gen_auitool;
    }

    return getGenName;
}

// Call this AFTER the node has been hooked up to it's parent to prevent duplicate var_names.
void ImportXML::ProcessAttributes(const pugi::xml_node& xml_obj, Node* new_node)
{
    for (auto& iter: xml_obj.attributes())
    {
        if (iter.name() == "name")
        {
            if (new_node->isForm())
            {
                if (auto prop = new_node->getPropPtr(prop_class_name); prop)
                {
                    prop->set_value(iter.value());
                }
            }
            else if (iter.as_string().starts_with("wxID_"))
            {
                auto prop = new_node->getPropPtr(prop_id);
                if (prop)
                {
                    prop->set_value(iter.value());
                }
                else if (prop = new_node->getPropPtr(prop_var_name); prop)
                {
                    prop->set_value(iter.value());
                }
            }

            else
            {
                // In a ImportXML file, name is the ID and variable is the var_name
                if (!xml_obj.attribute("variable").empty())
                {
                    if (auto prop = new_node->getPropPtr(prop_id); prop)
                    {
                        prop->set_value(iter.value());
                    }
                    continue;
                }

                if (auto prop = new_node->getPropPtr(prop_var_name); prop)
                {
                    tt_string org_name(iter.value());
                    auto new_name = new_node->getUniqueName(org_name);
                    prop->set_value(new_name);
                }
            }
        }
        else if (iter.name() == "variable")
        {
            if (auto prop = new_node->getPropPtr(prop_var_name); prop)
            {
                tt_string org_name(iter.value());
                auto new_name = new_node->getUniqueName(org_name);
                prop->set_value(new_name);
            }
        }
        else if (iter.name() == "subclass")
        {
            new_node->set_value(prop_derived_class, iter.value());
        }
    }
}

void ImportXML::ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent)
{
    for (auto& iter: xml_obj.children())
    {
        if (iter.name() == "object")
        {
            continue;
        }

        auto wxue_prop = MapPropName(iter.name());

        if (wxue_prop == prop_unknown)
        {
            // Give inherited classes a chance to process unknown properties.
            if (!HandleUnknownProperty(iter, node, parent))
            {
                ProcessUnknownProperty(iter, node, parent);
            }
            continue;
        }

        if (HandleNormalProperty(iter, node, parent, wxue_prop))
        {
            continue;
        }

        // Start by processing names that wxUiEditor might use but that need special processing when importing.

        if (wxue_prop == prop_bitmap)
        {
            ProcessBitmap(iter, node);
            continue;
        }
        else if (wxue_prop == prop_inactive_bitmap)
        {
            ProcessBitmap(iter, node, prop_inactive_bitmap);
            continue;
        }
        else if (wxue_prop == prop_contents)
        {
            ProcessContent(iter, node);
            continue;
        }
        else if (wxue_prop == prop_value)
        {
            auto escaped = ConvertEscapeSlashes(iter.text().as_string());
            if (auto prop = node->getPropPtr(prop_value); prop)
            {
                prop->set_value(escaped);
            }
            continue;
        }
        else if (wxue_prop == prop_label)
        {
            tt_string label = ConvertEscapeSlashes(iter.text().as_string());
            label.Replace("_", "&");
            auto pos = label.find("\\t");
            if (tt::is_found(pos))
            {
                label[pos] = 0;
                node->set_value(prop_shortcut, label.subview(pos + 2));
            }
            if (auto prop = node->getPropPtr(prop_label); prop)
            {
                prop->set_value(label);
            }
            continue;
        }
        else if (wxue_prop == prop_extra_accels)
        {
            tt_string accel_list;
            for (auto& accel: iter.children())
            {
                if (accel_list.size())
                    accel_list << " ";
                accel_list << '"' << accel.text().as_string() << '"';
            }
            node->set_value(prop_extra_accels, accel_list);
            continue;
        }

        // Now process names that are identical.

        NodeProperty* prop = node->getPropPtr(wxue_prop);
        if (prop)
        {
            prop->set_value(iter.text().as_string());
            if (prop->getPropDeclaration()->declName().contains("colour") ||
                prop->getPropDeclaration()->declName().contains("color"))
            {
                // Convert old style into #RRGGBB
                prop->set_value(prop->as_color().GetAsString(wxC2S_HTML_SYNTAX));
            }
            continue;
        }
        else if (node->isGen(gen_BookPage) && wxue_prop == prop_style)
        {
            prop = node->getPropPtr(prop_window_style);
            if (prop)
            {
                prop->set_value(iter.text().as_string());
                continue;
            }
        }

        // Finally, process names that are unique to XRC/ImportXML
    }
}

namespace xrc_import
{
    enum
    {
        xrc_border,
        xrc_cellpos,
        xrc_cellspan,
        xrc_centered,
        xrc_checkable,
        xrc_creating_code,
        xrc_depth,
        xrc_dropdown,
        xrc_enabled,
        xrc_exstyle,
        xrc_flag,
        xrc_focused,
        xrc_handler,
        xrc_option,
        xrc_orient,
        xrc_radio,
        xrc_selected,
        xrc_selection,
        xrc_size,
        xrc_subclass,
        xrc_tabs,
        xrc_toggle,

    };

    // clang-format off
    std::map<std::string_view, int, std::less<>> unknown_properties = {

        { "border", xrc_border },
        { "cellpos", xrc_cellpos },
        { "cellspan", xrc_cellspan },
        { "centered", xrc_centered },
        { "checkable", xrc_checkable },
        { "creating_code", xrc_creating_code },
        { "depth", xrc_depth },
        { "dropdown", xrc_dropdown },
        { "enabled", xrc_enabled },
        { "exstyle", xrc_exstyle },
        { "flag", xrc_flag },
        { "focused", xrc_focused },
        { "handler", xrc_handler },
        { "option", xrc_option },
        { "orient", xrc_orient },
        { "radio", xrc_radio },
        { "selected", xrc_selected },
        { "selection", xrc_selection },
        { "size", xrc_size },
        { "subclass", xrc_subclass },
        { "tabs", xrc_tabs },
        { "toggle", xrc_toggle },

    };
    // clang-format on

};  // namespace xrc_import

void ImportXML::ProcessUnknownProperty(const pugi::xml_node& xml_obj, Node* node, Node* parent)
{
    // Mapping the strings to an enum is purely for readability -- it's a lot easier to find
    // the unknown property in a switch statement than it is to find it in a long list of
    // strings comparisons.

    if (auto result = unknown_properties.find(xml_obj.name()); result != unknown_properties.end())
    {
        switch (result->second)
        {
            case xrc_border:
                node->set_value(prop_border_size, xml_obj.text().as_string());
                return;

            case xrc_cellpos:
                if (tt_string_vector mstr(xml_obj.text().as_string(), ','); mstr.size())
                {
                    if (mstr[0].size())
                        node->set_value(prop_column, mstr[0]);
                    if (mstr.size() > 1 && mstr[1].size())
                        node->set_value(prop_row, mstr[1]);
                }
                return;

            case xrc_cellspan:
                if (tt_string_vector mstr(xml_obj.text().as_string(), ','); mstr.size())
                {
                    if (mstr[0].size() && tt::atoi(mstr[0]) > 0)
                        node->set_value(prop_rowspan, mstr[0]);
                    if (mstr.size() > 1 && mstr[1].size() && tt::atoi(mstr[1]) > 0)
                        node->set_value(prop_colspan, mstr[1]);
                }
                return;

            case xrc_centered:
                if (node->isGen(gen_wxDialog) || node->isGen(gen_wxFrame))
                {
                    if (!xml_obj.text().as_bool())
                        node->set_value(prop_center, "no");
                    return;  // default is centered, so we don't need to set it
                }
                break;

            case xrc_checkable:
                node->set_value(prop_kind, "wxITEM_CHECK");
                return;

            case xrc_creating_code:
                // TODO: [KeyWorks - 12-09-2021] This consists of macros that allow the user
                // to override one or more macros with their own parameter.
                return;

            case xrc_depth:
                // depth is used by wxTreeCtrl to indicate the depth of the item. wxUE should
                // be able to calculate this, so it doesn't use the property.
                return;

            case xrc_dropdown:
                if (node->isGen(gen_tool_dropdown))
                {
                    if (auto child_node = xml_obj.child("object"); child_node)
                    {
                        // XRC will have a wxMenu as the child of the dropdown object, but what we
                        // want is the wxMenuItem that is the child of the wxMenu.
                        for (auto& menu_item: child_node)
                        {
                            CreateXrcNode(menu_item, node);
                        }
                    }
                    else
                    {
                        MSG_INFO(tt_string() << "Unrecognized property: " << xml_obj.name() << " for " << node->declName());
                    }
                    return;
                }
                break;

            case xrc_enabled:
                if (!xml_obj.text().as_bool())
                    node->set_value(prop_disabled, true);
                return;

            case xrc_exstyle:
                if (node->isGen(gen_wxDialog))
                {
                    node->set_value(prop_extra_style, xml_obj.text().as_string());
                    return;
                }
                break;

            case xrc_flag:
                if (node->isGen(gen_sizeritem) || node->isGen(gen_gbsizeritem))
                {
                    HandleSizerItemProperty(xml_obj, node, parent);
                }
                else if (!node->isGen(gen_spacer))
                {
                    // spacer's don't use alignment or border styles
                    MSG_INFO(tt_string() << xml_obj.name() << " not supported for " << node->declName());
                }
                return;

            case xrc_focused:
                if (node->isGen(gen_wxTreeCtrl))
                {
                    return;
                }
                break;

            case xrc_handler:
                ProcessHandler(xml_obj, node);
                return;

            case xrc_option:
                if (node->hasProp(prop_proportion))
                {
                    node->set_value(prop_proportion, xml_obj.text().as_string());
                }
                else
                {
                    MSG_INFO(tt_string() << "\"option\" specified for node that doesn't have prop_proportion: "
                                         << node->declName());
                }
                return;

            case xrc_orient:
                if (node->hasProp(prop_orientation))
                {
                    node->set_value(prop_orientation, xml_obj.text().as_string());
                    return;
                }
                break;

            case xrc_radio:
                node->set_value(prop_kind, "wxITEM_RADIO");
                return;

            case xrc_selected:
                if (node->isGen(gen_oldbookpage) || node->isGen(gen_BookPage))
                {
                    node->set_value(prop_select, xml_obj.text().as_bool());
                    return;
                }
                // else if (auto* prop = node->getPropPtr(prop_checked); prop)
                else if (node->hasProp(prop_checked))
                {
                    node->set_value(prop_checked, xml_obj.text().as_bool());
                    return;
                }
                break;

            case xrc_selection:
                if (node->isGen(gen_wxChoice))
                {
                    node->set_value(prop_selection_int, xml_obj.text().as_int());
                    return;
                }
                break;

            // REVIEW: [Randalphwa - 10-03-2022] There is a prop_size, so how do we get here?
            case xrc_size:
                if (node->isGen(gen_spacer))
                {
                    if (tt_string_vector mstr(xml_obj.text().as_string(), ','); mstr.size())
                    {
                        if (mstr[0].size())
                            node->set_value(prop_width, mstr[0]);
                        if (mstr.size() > 1 && mstr[1].size())
                            node->set_value(prop_height, mstr[1]);
                    }
                    return;
                }
                break;

            case xrc_subclass:
                if (auto value = xml_obj.text().as_sview(); value.size())
                {
                    if (value.contains(";"))
                    {
                        // wxFormBuilder breaks this into three fields: class, header, forward_declare. Or at least it is
                        // supposed to. In version 3.10, it doesn't properly handle an empty class name, so the header file
                        // can appear first.
                        tt_string_vector parts(value, ';', tt::TRIM::both);
                        if (parts.size() > 0)
                        {
                            if (parts[0].contains(".h"))
                            {
                                node->set_value(prop_derived_header, parts[0]);
                            }
                            else if (parts.size() > 1)
                            {
                                node->set_value(prop_derived_class, parts[0]);
                                if (parts[1].size())
                                    node->set_value(prop_derived_header, parts[1]);
                            }
                        }
                    }
                    else
                    {
                        node->set_value(prop_derived_class, value);
                    }
                }
                return;

            case xrc_tabs:
                ProcessNotebookTabs(xml_obj, node);
                return;

            case xrc_toggle:
                node->set_value(prop_kind, "wxITEM_CHECK");
                return;

            default:
                break;
        }
    }

    MSG_INFO(tt_string() << "Unrecognized property: " << xml_obj.name() << " for " << node->declName());
}

void ImportXML::ProcessContent(const pugi::xml_node& xml_obj, Node* node)
{
    tt_string choices;
    for (auto& iter: xml_obj.children())
    {
        if (iter.name() == "item")
        {
            auto child = iter.child_as_cstr();
            child.Replace("\"", "\\\"", true);
            if (choices.size())
                choices << " ";
            choices << '\"' << child << '\"';
        }
    }

    if (choices.size())
        node->set_value(prop_contents, choices);
}

void ImportXML::ProcessNotebookTabs(const pugi::xml_node& xml_obj, Node* /* node */)
{
    m_notebook_tabs.clear();
    for (auto& iter: xml_obj.children())
    {
        if (iter.name() == "tab")
        {
            if (!iter.attribute("window").empty())
            {
                m_notebook_tabs[iter.attribute("window").as_std_str()] = iter.child_as_cstr();
            }
        }
    }
}

void ImportXML::ProcessBitmap(const pugi::xml_node& xml_obj, Node* node, GenEnum::PropName node_prop)
{
    if (!xml_obj.attribute("stock_id").empty())
    {
        tt_string bitmap("Art; ");
        bitmap << xml_obj.attribute("stock_id").value() << "|";
        if (!xml_obj.attribute("stock_client").empty())
            bitmap << xml_obj.attribute("stock_client").value();
        else
            bitmap << "wxART_OTHER";
        bitmap << ";[-1,-1]";

        if (auto prop = node->getPropPtr(node_prop); prop)
        {
            prop->set_value(bitmap);
        }
    }
    else
    {
        auto file = xml_obj.child_as_cstr();
        if (file.contains(".xpm", tt::CASE::either))
        {
            tt_string bitmap("XPM; ");
            bitmap << file;
            bitmap << ";[-1,-1]";

            if (auto prop = node->getPropPtr(prop_bitmap); prop)
            {
                prop->set_value(bitmap);
                if (node->isGen(gen_wxButton))
                    node->set_value(prop_markup, true);
            }
        }
        else
        {
            tt_string bitmap("Embed;");

            // wxGlade doubles the backslash after the drive letter on Windows, and that causes the conversion to a relative
            // path to be incorrect
            file.Replace(":\\\\", ":\\");

            tt_string relative(file.make_wxString());
            relative.make_relative(tt_string::GetCwd());
            relative.backslashestoforward();
            bitmap << relative;
            bitmap << ";[-1,-1]";

            if (auto prop = node->getPropPtr(prop_bitmap); prop)
            {
                prop->set_value(bitmap);
                if (node->isGen(gen_wxButton))
                    node->set_value(prop_markup, true);
            }
        }
    }
}

void ImportXML::ProcessHandler(const pugi::xml_node& xml_obj, Node* node)
{
    if (xml_obj.attribute("function").empty() || xml_obj.attribute("entry").empty())
        return;

    tt_string event_name("wx");
    event_name << xml_obj.attribute("entry").value();
    auto event = node->getEvent(event_name);
    if (event)
    {
        event->set_value(xml_obj.attribute("function").value());
        return;
    }
}

NodeSharedPtr ImportXML::CreateXrcNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem)
{
    auto object_name = xml_obj.attribute("class").as_cstr();
    if (object_name.empty())
        return NodeSharedPtr();

    bool isBitmapButton = (object_name == "wxBitmapButton");
    bool is_generic_version = false;
    auto getGenName = ConvertToGenName(object_name, parent);
    if (getGenName == gen_unknown)
    {
        if (object_name.ends_with("bookpage"))
        {
            getGenName = gen_BookPage;
        }
        else if (object_name == "wxGenericAnimationCtrl")
        {
            is_generic_version = true;
            getGenName = gen_wxAnimationCtrl;
        }
        else
        {
            MSG_INFO(tt_string() << "Unrecognized object: " << object_name);
            return NodeSharedPtr();
        }
    }

    if (getGenName == gen_wxCheckBox)
    {
        for (auto& iter: xml_obj.children())
        {
            if (iter.value() == "style")
            {
                if (iter.text().as_sview().contains("wxCHK_3STATE"))
                    getGenName = gen_Check3State;
                break;
            }
        }
    }

    if (getGenName == gen_tool)
    {
        if (xml_obj.find_node(
                [](const pugi::xml_node& node)
                {
                    return tt::is_sameas(node.name(), "dropdown", tt::CASE::either);
                }))
        {
            getGenName = gen_tool_dropdown;
        }
    }

    if (getGenName == gen_wxMenuBar && parent->isGen(gen_Project))
        // switch to the form version
        getGenName = gen_MenuBar;
    else if (getGenName == gen_wxToolBar && parent->isGen(gen_Project))
        // switch to the form version
        getGenName = gen_ToolBar;

    auto new_node = NodeCreation.createNode(getGenName, parent);
    if (new_node && is_generic_version)
    {
        new_node->set_value(prop_use_generic, true);
    }
    while (!new_node)
    {
        if (sizeritem && sizeritem->isGen(gen_oldbookpage))
        {
            if (auto page = NodeCreation.createNode(gen_PageCtrl, parent); page)
            {
                if (sizeritem->hasValue(prop_label))
                {
                    page->set_value(prop_label, sizeritem->as_string(prop_label));
                }
                parent->adoptChild(page);
                return CreateXrcNode(xml_obj, page.get(), sizeritem);
            }
        }
        else if (parent && parent->isGen(gen_wxPanel))
        {
            auto sizer = NodeCreation.createNode(gen_VerticalBoxSizer, parent);
            if (sizer)
            {
                new_node = NodeCreation.createNode(getGenName, sizer.get());
                if (new_node)
                {
                    parent->adoptChild(sizer);
                    parent = sizer.get();
                    continue;
                }
            }
        }

        tt_string msg("Unable to create ");
        msg << object_name;
        if (parent)
        {
            // We can't use the class name because that won't necessarily be the wxWidgets class name. E.g., PanelForm might
            // be the class name, but what we want to display to the user is wxPanel. GetHelpText() will give us something
            // that makes sense to the user.

            auto name = parent->getGenerator()->GetHelpText(parent);
            if (name.size() && name != "wxWidgets")
            {
#if defined(_DEBUG)
                // Currently, Debug builds also include the filename that gets passed to the browser if Help is requested.
                // That's not useful in a message box, so we remove it.

                name.erase_from('(');
#endif  // _DEBUG
                msg << " as a child of " << name;
            }
        }
        m_errors.emplace(msg);
        return {};
    }

    if (isBitmapButton)
    {
        new_node->set_value(prop_label, "");
    }

    if (parent)
    {
        if (auto prop = new_node->getPropPtr(prop_var_name); prop)
        {
            auto original = prop->as_string();
            auto new_name = parent->getUniqueName(prop->as_string());
            if (new_name.size() && new_name != prop->as_string())
                prop->set_value(new_name);
        }
    }

    if (new_node->isGen(gen_wxStdDialogButtonSizer))
    {
        if (parent)
        {
            parent->adoptChild(new_node);
        }
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());

        for (auto& button: xml_obj.children())
        {
            for (auto& btn_id: button.children())
            {
                auto id = btn_id.attribute("name").as_string();
                if (id == "wxID_OK")
                    new_node->getPropPtr(prop_OK)->set_value("1");
                else if (id == "wxID_YES")
                    new_node->getPropPtr(prop_Yes)->set_value("1");
                else if (id == "wxID_SAVE")
                    new_node->getPropPtr(prop_Save)->set_value("1");
                else if (id == "wxID_APPLY")
                    new_node->getPropPtr(prop_Apply)->set_value("1");
                else if (id == "wxID_NO")
                    new_node->getPropPtr(prop_No)->set_value("1");
                else if (id == "wxID_CANCEL")
                    new_node->getPropPtr(prop_Cancel)->set_value("1");
                else if (id == "wxID_CLOSE")
                    new_node->getPropPtr(prop_Close)->set_value("1");
                else if (id == "wxID_HELP")
                    new_node->getPropPtr(prop_Help)->set_value("1");
                else if (id == "wxID_CONTEXT_HELP")
                    new_node->getPropPtr(prop_ContextHelp)->set_value("1");
            }
        }

        new_node->getPropPtr(prop_alignment)->set_value("wxALIGN_RIGHT");
        return new_node;
    }

    auto child = xml_obj.child("object");
    if (NodeCreation.isOldHostType(new_node->declName()))
    {
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get(), parent);
        new_node = CreateXrcNode(child, parent, new_node.get());
        // ASSERT(new_node);
        if (!new_node)
            return NodeSharedPtr();
        if (new_node->isGen(gen_wxStdDialogButtonSizer))
            new_node->getPropPtr(prop_static_line)->set_value(false);
        child = child.next_sibling("object");
    }
    else if (sizeritem)
    {
        for (auto& iter: sizeritem->getPropsVector())
        {
            auto prop = new_node->addNodeProperty(iter.getPropDeclaration());
            prop->set_value(iter.as_string());
        }
        if (parent)
        {
            parent->adoptChild(new_node);
        }
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }
    else if (parent)
    {
        parent->adoptChild(new_node);

        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }

    // At this point, all properties have been processed.

    if (new_node->isGen(gen_wxGridSizer) || new_node->isGen(gen_wxFlexGridSizer))
    {
        if (new_node->as_int(prop_rows) > 0 && new_node->as_int(prop_cols) > 0)
            new_node->set_value(prop_rows, 0);
    }

    // Various designers allow the users to create settings that will generate an assert if compiled on a debug version of
    // wxWidgets. We fix some of the more common invalid settings here.

    if (new_node->hasValue(prop_flags) && new_node->as_string(prop_flags).contains("wxEXPAND"))
    {
        if (new_node->hasValue(prop_alignment))
        {
            // wxWidgets will ignore all alignment flags if wxEXPAND is set.
            new_node->set_value(prop_alignment, "");
        }
    }

    if (parent && parent->isSizer())
    {
        if (parent->as_string(prop_orientation).contains("wxHORIZONTAL"))
        {
            auto currentValue = new_node->as_string(prop_alignment);
            if (currentValue.size() && (currentValue.contains("wxALIGN_LEFT") || currentValue.contains("wxALIGN_RIGHT") ||
                                        currentValue.contains("wxALIGN_CENTER_HORIZONTAL")))
            {
                auto fixed = ClearMultiplePropFlags("wxALIGN_LEFT|wxALIGN_RIGHT|wxALIGN_CENTER_HORIZONTAL", currentValue);
                new_node->set_value(prop_alignment, fixed);
            }
        }
        else if (parent->as_string(prop_orientation).contains("wxVERTICAL"))
        {
            auto currentValue = new_node->as_string(prop_alignment);
            if (currentValue.size() && (currentValue.contains("wxALIGN_TOP") || currentValue.contains("wxALIGN_BOTTOM") ||
                                        currentValue.contains("wxALIGN_CENTER_VERTICAL")))
            {
                auto fixed = ClearMultiplePropFlags("wxALIGN_TOP|wxALIGN_BOTTOM|wxALIGN_CENTER_VERTICAL", currentValue);
                new_node->set_value(prop_alignment, fixed);
            }
        }
    }

    // XRC creates a bookpage with a few properties (label, style, etc.) and then it creates
    // a wxPanel object. We need to ignore the panel, and just process it's children. When we
    // create XRC content, the variable name and style attribute are duplicated in the
    // wxPanel -- but we should confirm that the bookpage information is always set.

    if (getGenName == gen_BookPage)
        child = child.child("object");

    while (child)
    {
        CreateXrcNode(child, new_node.get());
        child = child.next_sibling("object");
    }

    return new_node;
}

GenEnum::PropName ImportXML::MapPropName(std::string_view name) const
{
    if (name.size())
    {
        if (auto prop = FindProp(name); prop != prop_unknown)
        {
            return prop;
        }

        if (auto result = map_import_prop_names.find(name); result != map_import_prop_names.end())
        {
            return result->second;
        }
    }
    return prop_unknown;
}

GenEnum::GenName ImportXML::MapClassName(std::string_view name) const
{
    if (name.size())
    {
        if (auto result = rmap_GenNames.find(name); result != rmap_GenNames.end())
        {
            return result->second;
        }
        if (auto result = import_GenNames.find(name); result != import_GenNames.end())
        {
            return result->second;
        }
    }
    return gen_unknown;
}

tt_string_view ImportXML::GetCorrectEventName(tt_string_view name)
{
    if (auto result = map_old_events.find(name); result != map_old_events.end())
    {
        return result->second;
    }
    else
    {
        return name;
    }
}
