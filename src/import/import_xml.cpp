/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>

#include <frozen/map.h>

#include <format>

#include "import_xml.h"

#include "base_generator.h"      // BaseGenerator -- Base Generator class
#include "gen_enums.h"           // Enumerations for generators
#include "mainframe.h"           // Main window frame
#include "node.h"                // Node class
#include "node_creator.h"        // NodeCreator class
#include "ttwx.h"                // ttwx helpers for character and numeric conversions
#include "ttwx_string_vector.h"  // ttwx::StringVector class
#include "ttwx_view_vector.h"    // ttwx_view_vector class
#include "utils.h"               // Utility functions that work with properties

using namespace GenEnum;

namespace xrc_import
{
    constexpr auto map_import_prop_names = frozen::make_map<std::string_view, GenEnum::PropName>({
        { "accel", prop_shortcut },
        { "art-provider", prop_art_provider },
        { "bg", prop_background_colour },
        { "background", prop_background_colour },
        { "bitmap-bg", prop_bmp_background_colour },
        { "bitmap-minwidth", prop_bmp_min_width },
        { "bitmap-placement", prop_bmp_placement },
        { "bitmapposition", prop_position },
        // BUGBUG: [Randalphwa - 06-17-2022] should bitmapsize be prop_bitmapsize?
        { "bitmapsize", prop_image_size },
        { "choices", prop_contents },
        { "class", prop_class_name },
        { "clicked", prop_checked },
        { "content", prop_contents },
        { "defaultdirectory", prop_initial_folder },
        { "defaultfilename", prop_initial_filename },
        { "dimension", prop_majorDimension },
        { "effectduration", prop_duration },
        { "empty_cellsize", prop_empty_cell_size },
        { "enabled", prop_auto_start },
        { "extra-accels", prop_extra_accels },
        { "fg", prop_foreground_colour },
        { "foreground", prop_foreground_colour },
        { "flexibledirection", prop_flexible_direction },
        { "focused", prop_focus },
        { "gradient-end", prop_end_colour },
        { "gradient-start", prop_start_colour },
        { "growable_rows", prop_growablerows },
        { "growable_cols", prop_growablecols },
        { "gravity", prop_sashgravity },
        { "hideeffect", prop_hide_effect },
        { "hover", prop_current },
        { "htmlcode", prop_html_content },
        { "inactive-bitmap", prop_inactive_bitmap },
        { "include_file", prop_subclass_header },
        { "linesize", prop_line_size },
        { "longhelp", prop_statusbar },  // Used by toolbar tools
        { "maxsize", prop_maximum_size },
        { "minpanesize", prop_min_pane_size },
        { "minsize", prop_min_size },
        { "nonflexiblegrowmode", prop_non_flexible_grow_mode },
        { "oneshot", prop_one_shot },
        { "orient", prop_orientation },
        { "pagesize", prop_page_size },
        { "period", prop_interval },
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

std::optional<pugi::xml_document> ImportXML::LoadDocFile(const std::string& file)
{
    pugi::xml_document doc;

    if (auto result = doc.load_file_string(file); !result)
    {
        std::string msg =
            std::format(std::locale(""), "Parsing error: {}\n Line: {}, Column: {}, Offset: {:L}\n",
                        result.description(), result.line, result.column, result.offset);
        wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error", wxOK | wxICON_ERROR)
            .ShowModal();
        return {};
    }

    m_importProjectFile = file;

    return doc;
}

auto ImportXML::HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent)
    -> void
{
    auto flag_value = xml_prop.text().as_sview();
    std::string border_value = "";

    auto append_with_pipe = [](std::string& value, std::string_view val) -> void
    {
        if (!value.empty())
        {
            value += '|';
        }
        value += val;
    };

    if (flag_value.contains("wxALL"))
    {
        border_value = "wxALL";
    }
    else
    {
        if (flag_value.contains("wxLEFT"))
        {
            append_with_pipe(border_value, "wxLEFT");
        }
        if (flag_value.contains("wxRIGHT"))
        {
            append_with_pipe(border_value, "wxRIGHT");
        }
        if (flag_value.contains("wxTOP"))
        {
            append_with_pipe(border_value, "wxTOP");
        }
        if (flag_value.contains("wxBOTTOM"))
        {
            append_with_pipe(border_value, "wxBOTTOM");
        }
    }

    // Always set this even if it is empty
    node->set_value(prop_borders, border_value);

    bool is_VerticalSizer = false;
    bool is_HorizontalSizer = false;

    if (parent && parent->is_Sizer())
    {
        if (parent->as_string(prop_orientation).contains("wxVERTICAL"))
        {
            is_VerticalSizer = true;
        }
        if (parent->as_string(prop_orientation).contains("wxHORIZONTAL"))
        {
            is_HorizontalSizer = true;
        }
    }

    std::string align_value = "";
    if (flag_value.contains("wxALIGN_LEFT") && !is_HorizontalSizer)
    {
        align_value = "wxALIGN_LEFT";
    }
    if (flag_value.contains("wxALIGN_TOP") && !is_VerticalSizer)
    {
        append_with_pipe(align_value, "wxALIGN_TOP");
    }
    if (flag_value.contains("wxALIGN_RIGHT") && !is_HorizontalSizer)
    {
        append_with_pipe(align_value, "wxALIGN_RIGHT");
    }
    if (flag_value.contains("wxALIGN_BOTTOM") && !is_VerticalSizer)
    {
        append_with_pipe(align_value, "wxALIGN_BOTTOM");
    }

    if (flag_value.contains("wxALIGN_CENTER") || flag_value.contains("wxALIGN_CENTRE"))
    {
        if (flag_value.contains("wxALIGN_CENTER_VERTICAL") ||
            flag_value.contains("wxALIGN_CENTRE_VERTICAL"))
        {
            append_with_pipe(align_value, "wxALIGN_CENTER_VERTICAL");
        }
        else if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") ||
                 flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
        {
            append_with_pipe(align_value, "wxALIGN_CENTER_HORIZONTAL");
        }
        if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") ||
            flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
        {
            append_with_pipe(align_value, "wxALIGN_CENTER_HORIZONTAL");
        }

        // Because we use find(), all we know is that a CENTER flag was used, but not which
        // one. If we get here and no CENTER flag has been added, then assume that
        // "wxALIGN_CENTER" or "wxALIGN_CENTRE" was specified.

        if (align_value.find("CENTER") == std::string::npos)
        {
            append_with_pipe(align_value, "wxALIGN_CENTER");
        }
    }

    if (!align_value.empty())
    {
        node->set_value(prop_alignment, align_value);
    }

    std::string flags_value = "";
    if (flag_value.contains("wxEXPAND") || flag_value.contains("wxGROW"))
    {
        // You can't use wxEXPAND with any alignment flags
        node->set_value(prop_alignment, "");
        flags_value = "wxEXPAND";
    }
    if (flag_value.contains("wxSHAPED"))
    {
        append_with_pipe(flags_value, "wxSHAPED");
    }
    if (flag_value.contains("wxFIXED_MINSIZE"))
    {
        append_with_pipe(flags_value, "wxFIXED_MINSIZE");
    }
    if (flag_value.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
    {
        append_with_pipe(flags_value, "wxRESERVE_SPACE_EVEN_IF_HIDDEN");
    }
    if (flag_value.contains("wxTILE"))
    {
        append_with_pipe(flags_value, "wxSHAPED|wxFIXED_MINSIZE");
    }

    if (!flags_value.empty())
    {
        node->set_value(prop_flags, flags_value);
    }
}

auto ImportXML::ProcessStyle(pugi::xml_node& xml_prop, Node* node, NodeProperty* prop) -> void
{
    if (node->is_Gen(gen_wxListBox) || node->is_Gen(gen_wxCheckListBox))
    {
        // A list box selection type can only be single, multiple, or extended, so wxUiEditor
        // stores this setting in a type property so that the user can only choose one.

        wxString style(xml_prop.text().as_view());
        if (style.Contains("wxLB_SINGLE"))
        {
            node->set_value(prop_type, "wxLB_SINGLE");
            if (style.Contains("wxLB_SINGLE|"))
            {
                style.Replace("wxLB_SINGLE|", "");
            }
            else
            {
                style.Replace("wxLB_SINGLE", "");
            }
        }
        else if (style.Contains("wxLB_MULTIPLE"))
        {
            node->set_value(prop_type, "wxLB_MULTIPLE");
            if (style.Contains("wxLB_MULTIPLE|"))
            {
                style.Replace("wxLB_MULTIPLE|", "");
            }
            else
            {
                style.Replace("wxLB_MULTIPLE", "");
            }
        }
        else if (style.Contains("wxLB_EXTENDED"))
        {
            node->set_value(prop_type, "wxLB_EXTENDED");
            if (style.Contains("wxLB_EXTENDED|"))
            {
                style.Replace("wxLB_EXTENDED|", "");
            }
            else
            {
                style.Replace("wxLB_EXTENDED", "");
            }
        }
        prop->set_value(style);
    }
    else if (node->is_Gen(gen_wxRadioBox))
    {
        wxString style(xml_prop.text().as_view());
        // It's a bug to specifiy both styles, we fix that here
        if (style.Contains("wxRA_SPECIFY_ROWS") && style.Contains("wxRA_SPECIFY_COLS"))
        {
            prop->set_value("wxRA_SPECIFY_ROWS");
        }
        else
        {
            prop->set_value(style);
        }
    }
    else if (node->is_Gen(gen_wxGauge))
    {
        // A list box selection type can only be single, multiple, or extended, so wxUiEditor
        // stores this setting in a type property so that the user can only choose one.

        wxString style(xml_prop.text().as_view());
        if (style.Contains("wxGA_VERTICAL"))
        {
            auto* prop_type = node->get_PropPtr(prop_orientation);
            prop_type->set_value("wxGA_VERTICAL");
            if (style.Contains("wxGA_VERTICAL|"))
            {
                style.Replace("wxGA_VERTICAL|", "");
            }
            else
            {
                style.Replace("wxGA_VERTICAL", "");
            }

            // wxFormBuilder allows the user to specify both styles
            if (style.Contains("wxGA_HORIZONTAL|"))
            {
                style.Replace("wxGA_HORIZONTAL|", "");
            }
            else
            {
                style.Replace("wxGA_HORIZONTAL", "");
            }
        }
        else if (style.Contains("wxGA_HORIZONTAL"))
        {
            auto* prop_type = node->get_PropPtr(prop_orientation);
            prop_type->set_value("wxGA_HORIZONTAL");

            if (style.Contains("wxGA_HORIZONTAL|"))
            {
                style.Replace("wxGA_HORIZONTAL|", "");
            }
            else
            {
                style.Replace("wxGA_HORIZONTAL", "");
            }
        }
        prop->set_value(style);
    }
    else if (node->is_Gen(gen_wxSlider))
    {
        // A list box selection type can only be single, multiple, or extended, so wxUiEditor
        // stores this setting in a type property so that the user can only choose one.

        wxString style(xml_prop.text().as_view());
        if (style.Contains("wxSL_HORIZONTAL"))
        {
            auto* prop_type = node->get_PropPtr(prop_orientation);
            prop_type->set_value("wxSL_HORIZONTAL");
            if (style.Contains("wxSL_HORIZONTAL|"))
            {
                style.Replace("wxSL_HORIZONTAL|", "");
            }
            else
            {
                style.Replace("wxSL_HORIZONTAL", "");
            }
        }
        else if (style.Contains("wxSL_VERTICAL"))
        {
            auto* prop_type = node->get_PropPtr(prop_orientation);
            prop_type->set_value("wxSL_VERTICAL");
            if (style.Contains("wxSL_VERTICAL|"))
            {
                style.Replace("wxSL_VERTICAL|", "");
            }
            else
            {
                style.Replace("wxSL_VERTICAL", "");
            }
        }
        prop->set_value(style);
    }
    else if (node->is_Gen(gen_wxFontPickerCtrl))
    {
        tt_string style(xml_prop.text().as_view());
        if (style.contains("wxFNTP_DEFAULT_STYLE"))
        {
            node->set_value(prop_style, "wxFNTP_FONTDESC_AS_LABEL|wxFNTP_USEFONT_FOR_LABEL");
        }
    }
    else if (node->is_Gen(gen_wxListView))
    {
        std::string style = "";
        ttwx::ViewVector mstr(xml_prop.text().as_view(), '|');
        for (auto& iter: mstr)
        {
            if (iter.starts_with("wxLC_ICON") || iter.starts_with("wxLC_SMALL_ICON") ||
                iter.starts_with("wxLC_LIST") || iter.starts_with("wxLC_REPORT"))
            {
                node->set_value(prop_mode, iter);
            }
            else
            {
                if (!style.empty())
                {
                    style += '|';
                }
                style += iter;
            }
        }
        if (!style.empty())
        {
            prop->set_value(style);
        }
    }
    else if (node->is_Gen(gen_wxToolBar))
    {
        wxString style(xml_prop.text().as_view());
        style.Replace("wxAUI_TB_DEFAULT_STYLE", "wxTB_HORIZONTAL");
        style.Replace("wxAUI_TB_HORZ_LAYOUT", "wxTB_HORZ_LAYOUT");
        style.Replace("wxAUI_TB_TEXT", "wxTB_TEXT");
        style.Replace("wxAUI_TB_VERTICAL", "wxTB_VERTICAL");
        style.Replace("wxAUI_TB_NO_TOOLTIPS", "wxTB_NO_TOOLTIPS");
        style.Replace("wxAUI_TB_NO_TOOLTIPS", "wxTB_NO_TOOLTIPS");
        if (!style.empty())
        {
            prop->set_value(style);
        }
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

[[nodiscard]] auto ImportXML::ConvertToGenName(const std::string& object_name, Node* parent)
    -> GenEnum::GenName
{
    auto get_GenName = MapClassName(object_name);

    if (get_GenName == gen_wxPanel)
    {
        if (!parent)
        {
            auto* owner = wxGetFrame().getSelectedNode();
            while (owner->get_GenType() == type_sizer)
            {
                owner = owner->get_Parent();
            }
            if (owner->get_DeclName().find("book") != std::string_view::npos)
            {
                return gen_BookPage;
            }
            return gen_PanelForm;
        }

        if (ttwx::is_found(parent->get_DeclName().find("book")))
        {
            return gen_BookPage;
        }

        if (parent->is_Gen(gen_Project))
        {
            return gen_PanelForm;
        }
    }
    else if (get_GenName == gen_sizeritem && parent && parent->is_Gen(gen_wxGridBagSizer))
    {
        return gen_gbsizeritem;
    }
    else if (object_name.find("Panel") != std::string::npos && parent &&
             parent->is_Gen(gen_Project))
    {
        return gen_PanelForm;
    }
    else if (get_GenName == gen_separator && parent &&
             (parent->is_Gen(gen_wxToolBar) || parent->is_Gen(gen_ToolBar) ||
              parent->is_Gen(gen_wxAuiToolBar)))
    {
        return gen_toolSeparator;
    }
    else if (get_GenName == gen_tool && parent && parent->is_Gen(gen_wxAuiToolBar))
    {
        get_GenName = gen_auitool;
    }

    return get_GenName;
}

// Call this AFTER the node has been hooked up to it's parent to prevent duplicate var_names.
auto ImportXML::ProcessAttributes(const pugi::xml_node& xml_obj, Node* new_node) -> void
{
    for (auto& iter: xml_obj.attributes())
    {
        if (iter.name() == "name")
        {
            if (new_node->is_Form())
            {
                if (auto* prop = new_node->get_PropPtr(prop_class_name); prop)
                {
                    if (prop->as_string().empty())
                    {
                        prop->set_value(iter.value());
                    }
                }
            }
            else if (iter.as_view().starts_with("wxID_"))
            {
                auto* prop = new_node->get_PropPtr(prop_id);
                if (prop)
                {
                    prop->set_value(iter.value());
                }
                else if (prop = new_node->get_PropPtr(prop_var_name); prop)
                {
                    prop->set_value(iter.value());
                }
            }

            else
            {
                // In a ImportXML file, name is the ID and variable is the var_name
                if (!xml_obj.attribute("variable").empty())
                {
                    if (auto* prop = new_node->get_PropPtr(prop_id); prop)
                    {
                        prop->set_value(iter.value());
                    }
                    continue;
                }

                if (auto* prop = new_node->get_PropPtr(prop_var_name); prop)
                {
                    tt_string org_name(iter.value());
                    auto new_name = new_node->get_UniqueName(org_name);
                    prop->set_value(new_name);
                }
            }
        }
        else if (iter.name() == "variable")
        {
            if (auto* prop = new_node->get_PropPtr(prop_var_name); prop)
            {
                tt_string org_name(iter.value());
                auto new_name = new_node->get_UniqueName(org_name);
                prop->set_value(new_name);
            }
        }
        else if (iter.name() == "subclass")
        {
            new_node->set_value(prop_subclass, iter.value());
        }
    }
}

auto ImportXML::ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent) -> void
{
    for (auto& iter: xml_obj.children())
    {
        if (iter.name() == "object")
        {
            if (node->is_Gen(gen_wxListView))
            {
                if (auto class_name = iter.attribute("class").value(); class_name.size())
                {
                    if (auto col_node = iter.child("text"); col_node)
                    {
                        auto col_name = col_node.text().as_view();
                        std::string cur_col_names = node->as_string(prop_column_labels);
                        if (cur_col_names.size())
                        {
                            cur_col_names += ";";
                        }
                        cur_col_names += col_name;
                        node->set_value(prop_column_labels, cur_col_names);
                    }
                }
            }
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

        // Start by processing names that wxUiEditor might use but that need special processing
        // when importing.

        switch (wxue_prop)
        {
            case prop_bitmap:
                ProcessBitmap(iter, node);
                continue;

            case prop_inactive_bitmap:
                ProcessBitmap(iter, node, prop_inactive_bitmap);
                continue;

            case prop_contents:
                ProcessContent(iter, node);
                continue;

            case prop_value:
                {
                    auto escaped = ConvertEscapeSlashes(iter.text().as_view());
                    if (auto* prop = node->get_PropPtr(prop_value); prop)
                    {
                        prop->set_value(escaped);
                    }
                    continue;
                }

            case prop_label:
                {
                    std::string label = ConvertEscapeSlashes(iter.text().as_view());
                    std::ranges::replace(label, '_', '&');
                    auto pos = label.find("\\t");
                    if (ttwx::is_found(pos))
                    {
                        label[pos] = 0;
                        node->set_value(prop_shortcut, std::string_view(label).substr(pos + 2));
                    }
                    if (auto* prop = node->get_PropPtr(prop_label); prop)
                    {
                        prop->set_value(label);
                    }
                    continue;
                }

            case prop_extra_accels:
                {
                    std::string accel_list;
                    for (const auto& accel: iter.children())
                    {
                        if (accel_list.size())
                        {
                            accel_list += " ";
                        }
                        accel_list += '"';
                        accel_list += accel.text().as_view();
                        accel_list += '"';
                    }
                    node->set_value(prop_extra_accels, accel_list);
                    continue;
                }

            case prop_font:
                ProcessFont(iter, node);
                continue;

            default:
                break;
        }

        // Now process names that are identical.

        NodeProperty* prop = node->get_PropPtr(wxue_prop);
        if (prop)
        {
            if (wxue_prop == prop_style)
            {
                ProcessStyle(iter, node, prop);
            }
            else
            {
                prop->set_value(iter.text().as_view());
                if (prop->get_PropDeclaration()->get_DeclName().find("colour") !=
                        std::string_view::npos ||
                    prop->get_PropDeclaration()->get_DeclName().find("color") !=
                        std::string_view::npos)
                {
                    // Convert old style into #RRGGBB
                    prop->set_value(prop->as_color().GetAsString(wxC2S_HTML_SYNTAX));
                }
            }
            continue;
        }

        if (node->is_Gen(gen_BookPage) && wxue_prop == prop_style)
        {
            prop = node->get_PropPtr(prop_window_style);
            if (prop)
            {
                prop->set_value(iter.text().as_view());
                continue;
            }
        }

        // Finally, process names that are unique to XRC/ImportXML
    }
}

namespace xrc_import
{
    enum : std::uint8_t
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

auto ImportXML::ProcessUnknownProperty(const pugi::xml_node& xml_obj, Node* node, Node* parent)
    -> void
{
    // Mapping the strings to an enum is purely for readability -- it's a lot easier to find
    // the unknown property in a switch statement than it is to find it in a long list of
    // strings comparisons.

    if (auto result = unknown_properties.find(xml_obj.name()); result != unknown_properties.end())
    {
        switch (result->second)
        {
            case xrc_border:
                node->set_value(prop_border_size, xml_obj.text().as_view());
                return;

            case xrc_cellpos:
                if (ttwx::StringVector mstr { xml_obj.text().as_view(), ',' }; mstr.size())
                {
                    if (mstr[0].size())
                    {
                        node->set_value(prop_column, mstr[0]);
                    }
                    if (mstr.size() > 1 && mstr[1].size())
                    {
                        node->set_value(prop_row, mstr[1]);
                    }
                }
                return;

            case xrc_cellspan:
                if (ttwx::StringVector mstr { xml_obj.text().as_view(), ',' }; mstr.size())
                {
                    if (mstr[0].size() && ttwx::atoi(mstr[0]) > 0)
                    {
                        node->set_value(prop_rowspan, mstr[0]);
                    }
                    if (mstr.size() > 1 && mstr[1].size() && ttwx::atoi(mstr[1]) > 0)
                    {
                        node->set_value(prop_colspan, mstr[1]);
                    }
                }
                return;

            case xrc_centered:
                if (node->is_Gen(gen_wxDialog) || node->is_Type(type_frame_form))
                {
                    if (!xml_obj.text().as_bool())
                    {
                        node->set_value(prop_center, "no");
                    }
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
                if (node->is_Gen(gen_tool_dropdown))
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
                    else if (wxGetApp().isTestingMenuEnabled())
                    {
                        if (parent && parent->get_Form())
                        {
                            std::string msg =
                                std::filesystem::path(m_importProjectFile).filename().string();
                            msg += ": Unrecognized property: ";
                            msg += xml_obj.name();
                            msg += " for ";
                            msg += node->get_DeclName();
                            msg += " in ";
                            msg += parent->get_Form()->as_string(prop_class_name);
                            MSG_INFO(msg);
                        }
                        else
                        {
                            std::string msg =
                                std::filesystem::path(m_importProjectFile).filename().string();
                            msg += ": Unrecognized property: ";
                            msg += xml_obj.name();
                            msg += " for ";
                            msg += node->get_DeclName();
                            MSG_INFO(msg);
                        }
                    }
                    return;
                }
                break;

            case xrc_enabled:
                if (!xml_obj.text().as_bool())
                {
                    node->set_value(prop_disabled, true);
                }
                return;

            case xrc_exstyle:
                if (node->is_Gen(gen_wxDialog))
                {
                    node->set_value(prop_extra_style, xml_obj.text().as_view());
                    return;
                }
                break;

            case xrc_flag:
                if (node->is_Gen(gen_sizeritem) || node->is_Gen(gen_gbsizeritem))
                {
                    HandleSizerItemProperty(xml_obj, node, parent);
                }
                else if (!node->is_Gen(gen_spacer) && wxGetApp().isTestingMenuEnabled())
                {
                    // spacer's don't use alignment or border styles
                    if (parent && parent->get_Form())
                    {
                        std::string msg =
                            std::filesystem::path(m_importProjectFile).filename().string();
                        msg += ": ";
                        msg += xml_obj.name();
                        msg += " not supported for ";
                        msg += node->get_DeclName();
                        msg += " in ";
                        msg += parent->get_Form()->as_string(prop_class_name);
                        MSG_INFO(msg);
                    }
                    else
                    {
                        std::string msg =
                            std::filesystem::path(m_importProjectFile).filename().string();
                        msg += ": ";
                        msg += xml_obj.name();
                        msg += " not supported for ";
                        msg += node->get_DeclName();
                        MSG_INFO(msg);
                    }
                }
                return;

            case xrc_focused:
                if (node->is_Gen(gen_wxTreeCtrl))
                {
                    return;
                }
                break;

            case xrc_handler:
                ProcessHandler(xml_obj, node);
                return;

            case xrc_option:
                if (node->HasProp(prop_proportion))
                {
                    node->set_value(prop_proportion, xml_obj.text().as_view());
                }
                else if (wxGetApp().isTestingMenuEnabled())
                {
                    if (parent && parent->get_Form())
                    {
                        // wxSmith does this, so ignore it
                        if (!node->is_Gen(gen_gbsizeritem))
                        {
                            std::string msg =
                                std::filesystem::path(m_importProjectFile).filename().string();
                            msg += ": \"option\" specified for node that does not have "
                                   "prop_proportion: ";
                            msg += node->get_DeclName();
                            msg += " in ";
                            msg += parent->get_Form()->as_string(prop_class_name);
                            MSG_INFO(msg);
                        }
                    }
                    else
                    {
                        std::string msg =
                            std::filesystem::path(m_importProjectFile).filename().string();
                        msg +=
                            ": \"option\" specified for node that does not have prop_proportion: ";
                        msg += node->get_DeclName();
                        MSG_INFO(msg);
                    }
                }
                return;

            case xrc_orient:
                if (node->HasProp(prop_orientation))
                {
                    node->set_value(prop_orientation, xml_obj.text().as_view());
                    return;
                }
                break;

            case xrc_radio:
                node->set_value(prop_kind, "wxITEM_RADIO");
                return;

            case xrc_selected:
                if (node->is_Gen(gen_oldbookpage) || node->is_Gen(gen_BookPage))
                {
                    node->set_value(prop_select, xml_obj.text().as_bool());
                    return;
                }
                // else if (auto* prop = node->get_PropPtr(prop_checked); prop)
                else if (node->HasProp(prop_checked))
                {
                    node->set_value(prop_checked, xml_obj.text().as_bool());
                    return;
                }
                break;

            case xrc_selection:
                if (node->is_Gen(gen_wxChoice))
                {
                    node->set_value(prop_selection_int, xml_obj.text().as_int());
                    return;
                }
                break;

            // REVIEW: [Randalphwa - 10-03-2022] There is a prop_size, so how do we get here?
            case xrc_size:
                if (node->is_Gen(gen_spacer))
                {
                    if (ttwx::StringVector mstr { xml_obj.text().as_view(), ',' }; mstr.size())
                    {
                        if (mstr[0].size())
                        {
                            node->set_value(prop_width, mstr[0]);
                        }
                        if (mstr.size() > 1 && mstr[1].size())
                        {
                            node->set_value(prop_height, mstr[1]);
                        }
                    }
                    return;
                }
                break;

            case xrc_subclass:
                if (auto value = xml_obj.text().as_sview(); value.size())
                {
                    if (value.contains(";"))
                    {
                        // wxFormBuilder breaks this into three fields: class, header,
                        // forward_declare. Or at least it is supposed to. In version 3.10, it
                        // doesn't properly handle an empty class name, so the header file can
                        // appear first.
                        ttwx::StringVector parts { value, ';', ttwx::TRIM::both };
                        if (parts.size() > 0)
                        {
                            if (parts[0].find(".h") != std::string::npos)
                            {
                                node->set_value(prop_subclass_header, parts[0]);
                            }
                            else if (parts.size() > 1)
                            {
                                node->set_value(prop_subclass, parts[0]);
                                if (parts[1].size())
                                {
                                    node->set_value(prop_subclass_header, parts[1]);
                                }
                            }
                        }
                    }
                    else
                    {
                        node->set_value(prop_subclass, value);
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

    if (wxGetApp().isTestingMenuEnabled())
    {
        if (parent && parent->get_Form())
        {
            std::string msg = m_importProjectFile;
            msg += ": Unrecognized property: ";
            msg += xml_obj.name();
            msg += " for ";
            msg += node->get_DeclName();
            msg += " in ";
            msg += parent->get_Form()->as_string(prop_class_name);
            MSG_INFO(msg);
        }
        else
        {
            std::string msg = m_importProjectFile;
            msg += ": Unrecognized property: ";
            msg += xml_obj.name();
            msg += " for ";
            msg += node->get_DeclName();
            MSG_INFO(msg);
        }
    }
}

auto ImportXML::ProcessContent(const pugi::xml_node& xml_obj, Node* node) -> void
{
    std::string choices = "";
    for (const auto& iter: xml_obj.children())
    {
        if (iter.name() == "item" || iter.name() == "choice")
        {
            auto child = iter.child_as_cstr();
            child.Replace("\"", "\\\"", true);
            if (choices.size())
            {
                choices += " ";
            }
            choices += '"';
            choices += child.ToStdString();
            choices += '"';
        }
    }

    if (choices.size())
    {
        node->set_value(prop_contents, choices);
    }
}

auto ImportXML::ProcessNotebookTabs(const pugi::xml_node& xml_obj, Node* /* node */) -> void
{
    m_notebook_tabs.clear();
    for (const auto& iter: xml_obj.children())
    {
        if (iter.name() == "tab")
        {
            if (!iter.attribute("window").empty())
            {
                m_notebook_tabs[iter.attribute("window").as_str()] = iter.child_as_cstr();
            }
        }
    }
}

auto ImportXML::ProcessBitmap(const pugi::xml_node& xml_obj, Node* node,
                              GenEnum::PropName node_prop) -> void
{
    if (!xml_obj.attribute("stock_id").empty())
    {
        std::string bitmap("Art; ");
        bitmap += xml_obj.attribute("stock_id").value();
        bitmap += "|";
        if (!xml_obj.attribute("stock_client").empty())
        {
            bitmap += xml_obj.attribute("stock_client").value();
        }
        else
        {
            bitmap += "wxART_OTHER";
        }
        bitmap += ";[-1,-1]";

        if (auto* prop = node->get_PropPtr(node_prop); prop)
        {
            prop->set_value(bitmap);
        }
    }
    else
    {
        wxString file = xml_obj.child_as_cstr();
        if (file.StartsWith("code:"))
        {
            // This is a wxGlade bitmap
            // TODO: [Randalphwa - 10-12-2023] wxGlade bitmaps are not yet supported
        }

        if (file.Lower().Find(".xpm") != wxNOT_FOUND)
        {
            std::string bitmap("XPM; ");
            bitmap += file.ToStdString();
            bitmap += ";[-1,-1]";

            if (auto* prop = node->get_PropPtr(prop_bitmap); prop)
            {
                prop->set_value(bitmap);
                if (node->is_Gen(gen_wxButton))
                {
                    node->set_value(prop_markup, true);
                }
            }
        }
        else
        {
            std::string bitmap("Embed;");

            // wxGlade doubles the backslash after the drive letter on Windows, and that causes
            // the conversion to a relative path to be incorrect
            file.Replace(":\\\\\\\\", ":\\\\");

            wxFileName fn(file);
            fn.MakeRelativeTo(wxString::FromUTF8(wxGetCwd()));
            wxString relative = fn.GetFullPath();
            ttwx::back_slashesto_forward(relative);
            bitmap += relative.ToStdString();
            bitmap += ";[-1,-1]";

            if (auto* prop = node->get_PropPtr(prop_bitmap); prop)
            {
                prop->set_value(bitmap);
                if (node->is_Gen(gen_wxButton))
                {
                    node->set_value(prop_markup, true);
                }
            }
        }
    }
}

auto ImportXML::ProcessHandler(const pugi::xml_node& xml_obj, Node* node) -> void
{
    if (xml_obj.attribute("function").empty() || xml_obj.attribute("entry").empty())
    {
        return;
    }

    std::string event_name("wx");
    event_name += xml_obj.attribute("entry").value();
    auto* event = node->get_Event(event_name);
    if (event)
    {
        event->set_value(xml_obj.attribute("function").value());
        return;
    }
}

[[nodiscard]] auto ImportXML::CreateXrcNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem)
    -> NodeSharedPtr
{
    auto object_name = xml_obj.attribute("class").as_cstr();
    if (object_name.empty())
    {
        return NodeSharedPtr();
    }

    bool isBitmapButton = (object_name == "wxBitmapButton");
    bool is_generic_version = false;
    auto get_GenName = ConvertToGenName(object_name.ToStdString(), parent);
    if (get_GenName == gen_unknown)
    {
        if (object_name.ends_with("bookpage") || object_name == "propertysheetpage")
        {
            get_GenName = gen_BookPage;
        }
        else if (object_name == "wxGenericAnimationCtrl")
        {
            is_generic_version = true;
            get_GenName = gen_wxAnimationCtrl;
        }
        else if (object_name == "button" && (parent->is_Gen(gen_wxRibbonButtonBar)))
        {
            get_GenName = gen_ribbonButton;
        }
        else if (object_name == "item" && (parent->is_Gen(gen_wxRibbonGallery)))
        {
            get_GenName = gen_ribbonGalleryItem;
        }
        else if (object_name == "listcol" && parent->is_Gen(gen_wxListView))
        {
            return NodeSharedPtr();
        }
        else
        {
            if (wxGetApp().isTestingMenuEnabled())
            {
                if (parent)
                {
                    auto* form = parent->get_Form();
                    if (form && form->HasValue(prop_class_name))
                    {
                        std::string msg =
                            std::filesystem::path(m_importProjectFile).filename().string();
                        msg += ": Unrecognized object: ";
                        msg += object_name;
                        msg += " in ";
                        msg += map_GenNames.at(parent->get_GenName());
                        msg += " (";
                        msg += form->as_string(prop_class_name);
                        msg += ")";
                        MSG_INFO(msg);
                    }
                    else
                    {
                        std::string msg =
                            std::filesystem::path(m_importProjectFile).filename().string();
                        msg += ": Unrecognized object: ";
                        msg += object_name;
                        msg += " in ";
                        msg += map_GenNames.at(parent->get_GenName());
                        MSG_INFO(msg);
                    }
                }
                else
                {
                    std::string msg =
                        std::filesystem::path(m_importProjectFile).filename().string();
                    msg += ": Unrecognized object: ";
                    msg += object_name;
                    MSG_INFO(msg);
                }
            }
            return {};
        }
    }
    else if (get_GenName == gen_wxCheckBox)
    {
        for (auto& iter: xml_obj.children())
        {
            if (iter.value() == "style")
            {
                if (iter.text().as_sview().contains("wxCHK_3STATE"))
                {
                    get_GenName = gen_Check3State;
                }
                break;
            }
        }
    }

    else if (get_GenName == gen_tool)
    {
        if (xml_obj.find_node(
                [](const pugi::xml_node& node)
                {
                    return tt::is_sameas(node.name(), "dropdown", tt::CASE::either);
                }))
        {
            get_GenName = gen_tool_dropdown;
        }
    }

    else if (get_GenName == gen_wxMenuBar && parent->is_Gen(gen_Project))
    {
        // switch to the form version
        get_GenName = gen_MenuBar;
    }
    else if (get_GenName == gen_wxToolBar && parent->is_Gen(gen_Project))
    {
        // switch to the form version
        get_GenName = gen_ToolBar;
    }

    auto new_node = NodeCreation.CreateNode(get_GenName, parent).first;
    if (new_node && is_generic_version)
    {
        new_node->set_value(prop_use_generic, true);
    }
    while (!new_node)
    {
        if (sizeritem && sizeritem->is_Gen(gen_oldbookpage))
        {
            if (auto page = NodeCreation.CreateNode(gen_PageCtrl, parent).first; page)
            {
                if (sizeritem->HasValue(prop_label))
                {
                    page->set_value(prop_label, sizeritem->as_string(prop_label));
                }
                parent->AdoptChild(page);
                return CreateXrcNode(xml_obj, page.get(), sizeritem);
            }
        }
        else if (parent && (parent->is_Gen(gen_wxPanel) || parent->is_Gen(gen_PanelForm) ||
                            parent->is_Gen(gen_wxDialog)))
        {
            auto sizer = NodeCreation.CreateNode(gen_VerticalBoxSizer, parent).first;
            if (sizer)
            {
                new_node = NodeCreation.CreateNode(get_GenName, sizer.get()).first;
                if (new_node)
                {
                    parent->AdoptChild(sizer);
                    parent = sizer.get();
                    continue;
                }
            }
        }
        // wxSmith uses wxMenu as a child of a wxMenu. In wxUiEditor, we use gen_submenu in order to
        // visually distinguish it as a parent.
        else if (parent && get_GenName == gen_wxMenu &&
                 (parent->is_Gen(gen_wxMenu) || parent->is_Gen(gen_submenu)))
        {
            new_node = NodeCreation.CreateNode(gen_submenu, parent).first;
            if (new_node)
            {
                continue;
            }
        }

        std::string msg("Unable to create ");
        msg += object_name.ToStdString();
        if (parent)
        {
            // We can't use the class name because that won't necessarily be the wxWidgets
            // class name. E.g., PanelForm might be the class name, but what we want to display
            // to the user is wxPanel. GetHelpText() will give us something that makes sense to
            // the user.

            auto name = parent->get_Generator()->GetHelpText(parent);
            if (name.size() && name != "wxWidgets")
            {
#if defined(_DEBUG)
                // Currently, Debug builds also include the filename that gets passed to the
                // browser if Help is requested. That's not useful in a message box, so we
                // remove it.

                name.erase_from('(');
#endif  // _DEBUG
                msg += " as a child of ";
                msg += name;
            }
        }
        m_errors.emplace(msg);
        return {};
    }

    if (isBitmapButton)
    {
        new_node->set_value(prop_label, "");
    }

    if (new_node->is_Form())
    {
        if (auto class_name = xml_obj.attribute("name").as_view(); class_name.size())
        {
            new_node->set_value(prop_class_name, class_name);
        }
    }

    if (parent)
    {
        if (auto* prop = new_node->get_PropPtr(prop_var_name); prop)
        {
            auto original = prop->as_string();
            auto new_name = parent->get_UniqueName(prop->as_string());
            if (new_name.size() && new_name != prop->as_string())
            {
                prop->set_value(new_name);
            }
        }
    }

    if (new_node->is_Gen(gen_wxStdDialogButtonSizer))
    {
        if (parent)
        {
            parent->AdoptChild(new_node);
        }
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());

        for (auto& button: xml_obj.children())
        {
            for (auto& btn_id: button.children())
            {
                auto btn_id_value = btn_id.attribute("name").as_view();
                if (btn_id_value == "wxID_OK")
                {
                    new_node->get_PropPtr(prop_OK)->set_value("1");
                }
                else if (btn_id_value == "wxID_YES")
                {
                    new_node->get_PropPtr(prop_Yes)->set_value("1");
                }
                else if (btn_id_value == "wxID_SAVE")
                {
                    new_node->get_PropPtr(prop_Save)->set_value("1");
                }
                else if (btn_id_value == "wxID_APPLY")
                {
                    new_node->get_PropPtr(prop_Apply)->set_value("1");
                }
                else if (btn_id_value == "wxID_NO")
                {
                    new_node->get_PropPtr(prop_No)->set_value("1");
                }
                else if (btn_id_value == "wxID_CANCEL")
                {
                    new_node->get_PropPtr(prop_Cancel)->set_value("1");
                }
                else if (btn_id_value == "wxID_CLOSE")
                {
                    new_node->get_PropPtr(prop_Close)->set_value("1");
                }
                else if (btn_id_value == "wxID_HELP")
                {
                    new_node->get_PropPtr(prop_Help)->set_value("1");
                }
                else if (btn_id_value == "wxID_CONTEXT_HELP")
                {
                    new_node->get_PropPtr(prop_ContextHelp)->set_value("1");
                }
            }
        }

        new_node->get_PropPtr(prop_alignment)->set_value("wxALIGN_RIGHT");
        return new_node;
    }

    auto child = xml_obj.child("object");
    if (NodeCreation.is_OldHostType(new_node->get_DeclName()))
    {
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get(), parent);
        new_node = CreateXrcNode(child, parent, new_node.get());
        // ASSERT(new_node);
        if (!new_node)
        {
            return NodeSharedPtr();
        }
        if (new_node->is_Gen(gen_wxStdDialogButtonSizer))
        {
            new_node->get_PropPtr(prop_static_line)->set_value(false);
        }
        child = child.next_sibling("object");
    }
    else if (sizeritem)
    {
        for (auto& iter: sizeritem->get_PropsVector())
        {
            auto* prop = new_node->AddNodeProperty(iter.get_PropDeclaration());
            prop->set_value(iter.as_string());
        }
        if (parent)
        {
            parent->AdoptChild(new_node);
        }
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }
    else if (parent)
    {
        parent->AdoptChild(new_node);

        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }

    // At this point, all properties have been processed.

    if (new_node->is_Gen(gen_wxGridSizer) || new_node->is_Gen(gen_wxFlexGridSizer))
    {
        if (new_node->as_int(prop_rows) > 0 && new_node->as_int(prop_cols) > 0)
        {
            new_node->set_value(prop_rows, 0);
        }
    }

    // Various designers allow the users to create settings that will generate an assert if
    // compiled on a debug version of wxWidgets. We fix some of the more common invalid
    // settings here.

    if (new_node->HasValue(prop_flags) && new_node->as_string(prop_flags).contains("wxEXPAND"))
    {
        if (new_node->HasValue(prop_alignment))
        {
            // wxWidgets will ignore all alignment flags if wxEXPAND is set.
            new_node->set_value(prop_alignment, "");
        }
    }

    if (parent && parent->is_Sizer())
    {
        if (parent->as_string(prop_orientation).contains("wxHORIZONTAL"))
        {
            auto currentValue = new_node->as_string(prop_alignment);
            if (currentValue.size() &&
                (currentValue.contains("wxALIGN_LEFT") || currentValue.contains("wxALIGN_RIGHT") ||
                 currentValue.contains("wxALIGN_CENTER_HORIZONTAL")))
            {
                auto fixed = ClearMultiplePropFlags(
                    "wxALIGN_LEFT|wxALIGN_RIGHT|wxALIGN_CENTER_HORIZONTAL", currentValue);
                new_node->set_value(prop_alignment, fixed);
            }
        }
        else if (parent->as_string(prop_orientation).contains("wxVERTICAL"))
        {
            auto currentValue = new_node->as_string(prop_alignment);
            if (currentValue.size() &&
                (currentValue.contains("wxALIGN_TOP") || currentValue.contains("wxALIGN_BOTTOM") ||
                 currentValue.contains("wxALIGN_CENTER_VERTICAL")))
            {
                auto fixed = ClearMultiplePropFlags(
                    "wxALIGN_TOP|wxALIGN_BOTTOM|wxALIGN_CENTER_VERTICAL", currentValue);
                new_node->set_value(prop_alignment, fixed);
            }
        }
    }

    // XRC creates a bookpage with a few properties (label, style, etc.) and then it creates
    // a wxPanel object. We need to ignore the panel, and just process it's children. When we
    // create XRC content, the variable name and style attribute are duplicated in the
    // wxPanel -- but we should confirm that the bookpage information is always set.

    if (get_GenName == gen_BookPage)
    {
        child = child.child("object");
    }

    while (child)
    {
        CreateXrcNode(child, new_node.get());
        child = child.next_sibling("object");
    }

    return new_node;
}

auto ImportXML::MapPropName(std::string_view name) const -> GenEnum::PropName
{
    if (name.size())
    {
        if (auto prop = FindProp(name); prop != prop_unknown)
        {
            return prop;
        }

        if (const auto* result = map_import_prop_names.find(name);
            result != map_import_prop_names.end())
        {
            return result->second;
        }
    }
    return prop_unknown;
}

auto ImportXML::MapClassName(std::string_view name) const -> GenEnum::GenName
{
    if (!name.empty())
    {
        if (auto result = rmap_GenNames.find(name); result != rmap_GenNames.end())
        {
            return result->second;
        }
        if (const auto* result = import_GenNames.find(name); result != import_GenNames.end())
        {
            return result->second;
        }
    }
    return gen_unknown;
}

auto ImportXML::GetCorrectEventName(std::string_view name) -> std::string_view
{
    if (const auto* result = map_old_events.find(name); result != map_old_events.end())
    {
        return result->second;
    }
    return name;
}

auto ImportXML::ProcessFont(const pugi::xml_node& xml_obj, Node* node) -> void
{
    FontProperty font_info;
    if (auto size_child = xml_obj.child("size"); size_child)
    {
        font_info.PointSize(size_child.text().as_double());
    }
    if (auto family_child = xml_obj.child("family");
        family_child && family_child.text().as_view() != "default")
    {
        FontFamilyPairs family_pair;
        font_info.Family(family_pair.GetValue(family_child.text().as_view()));
    }
    if (auto style_child = xml_obj.child("style");
        style_child && style_child.text().as_view() != "normal")
    {
        FontStylePairs style_pair;
        font_info.Style(style_pair.GetValue(style_child.text().as_view()));
    }
    if (auto weight_child = xml_obj.child("weight");
        weight_child && weight_child.text().as_view() != "normal")
    {
        FontWeightPairs weight_pair;
        font_info.Weight(weight_pair.GetValue(weight_child.text().as_view()));
    }
    if (auto underline_child = xml_obj.child("underline"); underline_child)
    {
        font_info.Underlined(underline_child.text().as_bool());
    }
    if (auto face_child = xml_obj.child("face"); face_child)
    {
        font_info.FaceName(face_child.text().as_cstr().make_wxString());
    }

    node->set_value(prop_font, font_info.as_string());
}
