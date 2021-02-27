/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropDefinition and PropertyInfo classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "prop_types.h"  // Type -- Property types

struct PropDefinition
{
    ttlib::cstr m_name;
    ttlib::cstr m_def_value;
    ttlib::cstr m_help;
    Type m_type;
};

class PropertyInfo : public PropDefinition
{
public:
    PropertyInfo(ttlib::cview name, Type type, ttlib::cview def_value, ttlib::cview help, ttlib::cview customEditor,
                 const std::vector<PropDefinition>& children)
    {
        m_name = name;
        m_type = type;
        m_def_value = def_value;
        m_help = help;
        m_children = children;
        m_customEditor = customEditor;
    }

    const std::vector<PropDefinition>* GetChildren() const noexcept { return &m_children; }

    const ttlib::cstr& GetName() const noexcept { return m_name; }
    const ttlib::cstr& GetDefaultValue() const noexcept { return m_def_value; }
    const ttlib::cstr& GetDescription() const noexcept { return m_help; }
    const ttlib::cstr& GetCustomEditor() const noexcept { return m_customEditor; }

    Type GetType() const noexcept { return m_type; }

    // These get used to setup wxPGProperty, so both key and value need to be a wxString
    struct Options
    {
        wxString name;
        wxString help;
    };

    std::vector<Options>& GetOptions() { return m_options; }

private:
    ttlib::cstr m_customEditor;  // an optional custom editor for the property grid

    // This gets used to setup wxPGProperty, so both key and value need to be a wxString
    std::vector<Options> m_options;

    std::vector<PropDefinition> m_children;  // Only used for parent properties
};
