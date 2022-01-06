/////////////////////////////////////////////////////////////////////////////
// Purpose:   Inherited class code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_inherit.h"

std::optional<ttlib::cstr> GenGetSetCode(Node* node)
{
    auto& get_name = node->prop_as_string(prop_get_function);
    auto& set_name = node->prop_as_string(prop_set_function);
    if (get_name.empty() && set_name.empty())
        return {};

    if (auto& var_name = node->prop_as_string(prop_validator_variable); var_name.size())
    {
        auto& val_data_type = node->prop_as_string(prop_validator_data_type);
        if (val_data_type.empty())
            return {};
        ttlib::cstr code;
        if (val_data_type == "wxString" || val_data_type == "wxFileName" || val_data_type == "wxArrayInt")
        {
            if (get_name.size())
                code << "\tconst " << val_data_type << "& " << get_name << "() const { return " << var_name << "; }";
            if (set_name.size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << set_name << "(const " << val_data_type << "& value) { " << var_name << " = value; }";
            }
            if (code.empty())
                return {};
            return code;
        }
        else
        {
            if (get_name.size())
                code << '\t' << val_data_type << ' ' << get_name << "() const { return " << var_name << "; }";
            if (set_name.size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << set_name << "(" << val_data_type << " value) { " << var_name << " = value; }";
            }
            if (code.empty())
                return {};
            return code;
        }
    }

    return {};
}

std::optional<ttlib::cstr> GenInheritSettings(Node* node)
{
    if (auto& var_name = node->prop_as_string(prop_validator_variable); var_name.size())
    {
        auto& val_data_type = node->prop_as_string(prop_validator_data_type);
        if (val_data_type.empty())
            return {};

        ttlib::cstr code;
        auto& validator_type = node->prop_as_string(prop_validator_type);
        if (validator_type.is_sameas("wxTextValidator"))
        {
            code << node->get_node_name() << "->SetValidator(wxTextValidator(" << node->prop_as_string(prop_validator_style)
                 << ", &" << var_name << "));";
        }
        else
        {
            if (node->isGen(gen_StaticCheckboxBoxSizer))
                code << node->prop_as_string(prop_checkbox_var_name);
            else if (node->isGen(gen_StaticRadioBtnBoxSizer))
                code << node->prop_as_string(prop_radiobtn_var_name);
            else
                code << node->get_node_name();

            if (node->isGen(gen_wxRearrangeCtrl))
                code << "->GetList()";
            code << "->SetValidator(wxGenericValidator(&" << var_name << "));";
        }

        return code;
    }

    return {};
}

static void AddPropIfUsed(PropName prop_name, ttlib::cview func_name, Node* node, ttlib::cstr& code)
{
    if (prop_name == prop_background_colour)
    {
        auto& color = node->prop_as_string(prop_background_colour);
        if (color.size())
        {
            if (code.size())
                code << "\n\t";
            if (!node->IsForm())
                code << node->get_node_name() << "->";
            code << "SetBackgroundColour(";
            if (color.contains("wx"))
                code << "wxSystemSettings::GetColour(" << color << "));";
            else
            {
                wxColour colour = ConvertToColour(color);
                code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
            }
        }
    }
    else if (prop_name == prop_foreground_colour)
    {
        auto& color = node->prop_as_string(prop_foreground_colour);
        if (color.size())
        {
            if (code.size())
                code << "\n\t";
            if (!node->IsForm())
                code << node->get_node_name() << "->";
            code << "SetForegroundColour(";
            if (color.contains("wx"))
                code << "wxSystemSettings::GetColour(" << color << "));";
            else
            {
                wxColour colour = ConvertToColour(color);
                code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
            }
        }
    }
    else if (node->prop_as_string(prop_name).size())
    {
        if (code.size())
            code << "\n\t";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << func_name << node->prop_as_string(prop_name) << ");";
    }
}

void GenerateWindowSettings(Node* node, ttlib::cstr& code)
{
    AddPropIfUsed(prop_window_extra_style, "SetExtraStyle(", node, code);
    if (node->IsForm())
    {
        AddPropIfUsed(prop_font, "SetFont(", node, code);
        AddPropIfUsed(prop_foreground_colour, "SetForegroundColour(", node, code);
        AddPropIfUsed(prop_background_colour, "SetBackgroundColour(", node, code);
    }
    else
    {
        code << GenFontColourSettings(node);
    }

    // Note that \t is added after the \n in case the caller needs to keep indents

    if (node->prop_as_bool(prop_disabled))
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "Enable(false);";
    }

    if (node->prop_as_bool(prop_hidden))
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "Hide();";
    }

    bool allow_minmax { true };
    bool is_smart_size { false };

    // REVIEW: [KeyWorks - 05-20-2021] Because of issue #242, this was shut off for all forms.

    // The following needs to match GenFormSettings() in gen_common.cpp. If these conditions are met, then GenFormSettings()
    // will generate SetSizeHints(min, max) so there is no reason to generate SetMinSize()/SetMaxSize()
    if (node->IsForm() && !node->isGen(gen_PanelForm) && !node->isGen(gen_wxToolBar))
        allow_minmax = false;

    auto size = node->prop_as_wxSize(prop_smart_size);
    if (size != wxDefaultSize)
    {
        is_smart_size = true;  // Set to true means prop_size and prop_minimum_size will be ignored

        // REVIEW: [KeyWorks - 12-07-2021] Do we need to block if allow_minmax is false?

        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetInitialSize(";
        if (node->prop_as_string(prop_smart_size).contains("d", tt::CASE::either))
            code << "ConvertPixelsToDialog(";

        if (size.IsFullySpecified())
            code << "\n\t";
        code << "wxSize(";

        if (size.x != -1)
            code << size.x << " > GetBestSize().x ? " << size.x << " : -1, ";
        else
            code << "-1, ";
        if (size.y != -1)
            code << size.y << " > GetBestSize().y ? " << size.y << " : -1";
        else
            code << "-1";
        if (node->prop_as_string(prop_smart_size).contains("d", tt::CASE::either))
            code << ')';  // close the ConvertPixelsToDialog function call
        code << "));";
    }

    if (!is_smart_size && allow_minmax)
    {
        size = node->prop_as_wxSize(prop_minimum_size);
        if (size != wxDefaultSize)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->";
            code << "SetMinSize(";
            if (node->prop_as_string(prop_minimum_size).contains("d", tt::CASE::either))
                code << "ConvertPixelsToDialog(";

            code << "wxSize(" << size.x << ", " << size.y;
            if (node->prop_as_string(prop_minimum_size).contains("d", tt::CASE::either))
                code << ')';  // close the ConvertPixelsToDialog function call
            code << "));";
        }
    }

    size = node->prop_as_wxSize(prop_maximum_size);
    if (size != wxDefaultSize)
    {
        if (allow_minmax)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->";
            code << "SetMaxSize(";
            if (node->prop_as_string(prop_minimum_size).contains("d", tt::CASE::either))
                code << "ConvertPixelsToDialog(";

            code << "wxSize(" << size.x << ", " << size.y;
            if (node->prop_as_string(prop_minimum_size).contains("d", tt::CASE::either))
                code << ')';  // close the ConvertPixelsToDialog function call
            code << "));";
        }
    }

    if (!node->IsForm() && !node->isPropValue(prop_variant, "normal"))
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetWindowVariant(";

        if (node->isPropValue(prop_variant, "small"))
            code << "wxWINDOW_VARIANT_SMALL);";
        else if (node->isPropValue(prop_variant, "mini"))
            code << "wxWINDOW_VARIANT_MINI);";
        else
            code << "wxWINDOW_VARIANT_LARGE);";
    }

    if (node->prop_as_string(prop_tooltip).size())
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "SetToolTip(" << GenerateQuotedString(node->prop_as_string(prop_tooltip)) << ");";
    }

    if (node->prop_as_string(prop_context_help).size())
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "SetHelpText(" << GenerateQuotedString(node->prop_as_string(prop_context_help)) << ");";
    }
}
