/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropDefinition and PropertyInfo classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"  // Enumerations for generators

struct PropDefinition
{
    ttlib::cstr m_def_value;
    ttlib::cstr m_help;

    // TODO: these are obsolete and need to be removed
    ttlib::cstr m_name;

    // BUGBUG: [KeyWorks - 04-09-2021] NodeCreator::ParseProperties does not initialize the following for parent properties

    GenEnum::PropType m_prop_type;
    GenEnum::PropName m_enum_name;
    const char* m_prop_name;
};

class PropertyInfo : public PropDefinition
{
public:
    PropertyInfo(GenEnum::PropName prop_name, GenEnum::PropType prop_type, ttlib::cview def_value,
                 ttlib::cview help, ttlib::cview customEditor, const std::vector<PropDefinition>& children)
    {
        m_def_value = def_value;
        m_help = help;
        m_children = children;
        m_customEditor = customEditor;

        m_prop_type = prop_type;
        m_enum_name = prop_name;
        m_prop_name = GenEnum::map_PropNames[m_enum_name];

        // TODO: This is only here until all callers stop using GetName()
        m_name = m_prop_name;
    }

    const std::vector<PropDefinition>* GetChildren() const noexcept { return &m_children; }

    const char* name_as_string() const noexcept { return m_prop_name; }
    GenEnum::PropName name() const noexcept { return m_enum_name; }
    GenEnum::PropType type() const noexcept { return m_prop_type; }

    const ttlib::cstr& GetName() const noexcept { return m_name; }

    const ttlib::cstr& GetDefaultValue() const noexcept { return m_def_value; }
    const ttlib::cstr& GetDescription() const noexcept { return m_help; }
    const ttlib::cstr& GetCustomEditor() const noexcept { return m_customEditor; }

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
