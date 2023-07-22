/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropChildDeclaration and PropDeclaration classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

struct PropChildDeclaration
{
    tt_string m_def_value;
    tt_string m_help;

    // BUGBUG: [KeyWorks - 04-09-2021] NodeCreator::parseProperties does not initialize the following for parent properties

    const char* name_str() const noexcept { return m_name_str; }
    PropName get_name() const noexcept { return m_name_enum; }
    PropType get_type() const noexcept { return m_prop_type; }

    bool isType(PropType type) const noexcept { return (type == m_prop_type); }
    bool isProp(PropName name) const noexcept { return (name == m_name_enum); }

    GenEnum::PropType m_prop_type;
    GenEnum::PropName m_name_enum;  // enumeration value for the name
    const char* m_name_str;         // const char* pointer to the name
};

class PropDeclaration : public PropChildDeclaration
{
public:
    PropDeclaration(GenEnum::PropName prop_name, GenEnum::PropType prop_type, std::string_view def_value,
                    std::string_view help, std::string_view customEditor)
    {
        m_def_value = def_value;
        m_help = help;
        m_customEditor = customEditor;

        m_prop_type = prop_type;
        m_name_enum = prop_name;
        m_name_str = GenEnum::map_PropNames[m_name_enum];
    }

    // Returns a char pointer to the name. Use get_name() if you want the enum value.
    tt_string_view declName() const noexcept { return m_name_str; }

    const tt_string& getDefaultValue() const noexcept { return m_def_value; }
    const tt_string& getDescription() const noexcept { return m_help; }
    const tt_string& getCustomEditor() const noexcept { return m_customEditor; }

    // These get used to setup wxPGProperty, so both key and value need to be a wxString
    struct Options
    {
        tt_string name;
        tt_string help;
    };

    std::vector<Options>& getOptions() { return m_options; }

private:
    tt_string m_customEditor;  // an optional custom editor for the property grid

    // This gets used to setup wxPGProperty, so both key and value need to be a wxString
    std::vector<Options> m_options;
};
