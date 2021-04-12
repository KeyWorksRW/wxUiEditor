/////////////////////////////////////////////////////////////////////////////
// Purpose:   Inherited class code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_inherit.h"

std::optional<ttlib::cstr> GenGetSetCode(Node* node)
{
    auto get_name = node->get_value_ptr("get_function");
    auto set_name = node->get_value_ptr("set_function");
    if (!get_name && !set_name)
        return {};

    auto var_name = node->get_value_ptr("validator_variable");
    if (var_name && var_name->size())
    {
        auto val_data_type = node->get_value_ptr("validator_data_type");
        if (val_data_type->empty())
            return {};
        ttlib::cstr code;
        if (*val_data_type == "wxString" || *val_data_type == "wxFileName" || *val_data_type == "wxArrayInt")
        {
            if (get_name && get_name->size())
                code << "\tconst " << *val_data_type << "& " << *get_name << "() const { return " << *var_name << "; }";
            if (set_name && set_name->size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << *set_name << "(const " << *val_data_type << "& value) { " << *var_name << " = value; }";
            }
            if (code.empty())
                return {};
            return code;
        }
        else
        {
            if (get_name && get_name->size())
                code << '\t' << *val_data_type << ' ' << *get_name << "() const { return " << *var_name << "; }";
            if (set_name && set_name->size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << *set_name << "(" << *val_data_type << " value) { " << *var_name << " = value; }";
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
    if (auto var_name = node->get_value_ptr("validator_variable"); var_name && var_name->size())
    {
        auto val_data_type = node->get_value_ptr("validator_data_type");
        if (val_data_type->empty())
            return {};

        ttlib::cstr code;
        auto validator_type = node->get_value_ptr("validator_type");
        if (validator_type && validator_type->is_sameas("wxTextValidator"))
        {
            code << node->get_node_name() << "->SetValidator(wxTextValidator(" << node->prop_as_string(prop_validator_style)
                 << ", &" << *var_name << "));";
        }
        else
        {
            code << node->get_node_name() << "->SetValidator(wxGenericValidator(&" << *var_name << "));";
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
                code << "\n    ";
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
                code << "\n    ";
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
            code << "\n    ";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << func_name << node->prop_as_string(prop_name) << ");";
    }
}

void GenerateWindowSettings(Node* node, ttlib::cstr& code)
{
    AddPropIfUsed(prop_window_extra_style, "SetExtraStyle(", node, code);
    AddPropIfUsed(prop_font, "SetFont(", node, code);
    AddPropIfUsed(prop_foreground_colour, "SetForegroundColour(", node, code);
    AddPropIfUsed(prop_background_colour, "SetBackgroundColour(", node, code);

    // Note that \t is added after the \n in case the caller needs to keep indents

    if (node->prop_as_bool(prop_disabled))
    {
        if (code.size())
            code << "\n    ";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "Enable(false);";
    }

    if (node->prop_as_bool(prop_hidden))
    {
        if (code.size())
            code << "\n    ";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "Hide();";
    }

    bool allow_minmax = true;

    // The following needs to match GenFormSettings() in gen_common.cpp. If these conditions are met, then GenFormSettings()
    // will generate SetSizeHints(min, max) so there is no reason to generate SetMinSize()/SetMaxSize()
    if (node->IsForm() && !node->isGen(gen_PanelForm) && !node->isGen(gen_wxToolBar))
        allow_minmax = false;

    auto size = node->prop_as_wxPoint(prop_minimum_size);
    if (size.x != -1 || size.y != -1)
    {
        if (allow_minmax)
        {
            if (code.size())
                code << "\n    ";
            code << node->get_node_name() << "->";
            code << "SetMinSize(wxSize(" << size.x << ", " << size.y << "));";
        }
    }

    size = node->prop_as_wxPoint(prop_maximum_size);
    if (size.x != -1 || size.y != -1)
    {
        if (allow_minmax)
        {
            if (code.size())
                code << "\n    ";
            code << node->get_node_name() << "->";
            code << "SetMaxSize(wxSize(" << size.x << ", " << size.y << "));";
        }
    }

    if (node->prop_as_string(prop_tooltip).size())
    {
        if (code.size())
            code << "\n    ";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "SetToolTip(" << GenerateQuotedString(node->prop_as_string(prop_tooltip)) << ");";
    }

    if (node->prop_as_string(prop_context_help).size())
    {
        if (code.size())
            code << "\n    ";
        if (!node->IsForm())
            code << node->get_node_name() << "->";
        code << "SetHelpText(" << GenerateQuotedString(node->prop_as_string(prop_context_help)) << ");";
    }
}
