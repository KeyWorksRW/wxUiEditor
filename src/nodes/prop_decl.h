/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropChildDeclaration and PropDeclaration classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

// class PropDeclaration : public PropChildDeclaration
class PropDeclaration
{
public:
    PropDeclaration(GenEnum::PropName prop_name, GenEnum::PropType prop_type, std::string_view def_value,
                    std::string_view help)
    {
        m_name_enum = prop_name;
        m_prop_type = prop_type;
        m_def_value = def_value;
        m_help = help;
    }

    // Returns a char pointer to the name. Use get_name() if you want the enum value.
    tt_string_view declName() const noexcept { return GenEnum::map_PropNames[m_name_enum]; }

    const tt_string& getDefaultValue() const noexcept { return m_def_value; }
    const tt_string& getDescription() const noexcept { return m_help; }

    PropName get_name() const noexcept { return m_name_enum; }
    PropType get_type() const noexcept { return m_prop_type; }

    bool isType(PropType type) const noexcept { return (type == m_prop_type); }
    bool isProp(PropName name) const noexcept { return (name == m_name_enum); }

    // These get used to setup wxPGProperty, so both key and value need to be a wxString
    struct Options
    {
        tt_string name;
        tt_string help;
    };

    std::vector<Options>& getOptions() { return m_options; }

private:
    tt_string m_def_value;
    tt_string m_help;

    GenEnum::PropType m_prop_type;
    GenEnum::PropName m_name_enum;  // enumeration value for the name

    // This gets used to setup wxPGProperty, so both key and value need to be a wxString
    std::vector<Options> m_options;
};
