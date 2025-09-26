/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropChildDeclaration and PropDeclaration classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"  // Enumerations for generators
#include <string_view>
using namespace GenEnum;

// class PropDeclaration : public PropChildDeclaration
class PropDeclaration
{
public:
    // These structs are more cumbersome to use, but prevent accidentally switching the order of the
    // parameters when creating a PropDeclaration object.
    struct DefaultValue
    {
        std::string_view value;
    };
    struct HelpText
    {
        std::string_view value;
    };

    PropDeclaration(GenEnum::PropName prop_name, GenEnum::PropType prop_type,
                    DefaultValue def_value, HelpText help) :
        m_def_value(def_value.value), m_help(help.value), m_prop_type(prop_type),
        m_name_enum(prop_name)
    {
    }

    // Returns a std::string_view to the name. Use get_name() if you want the enum value.
    [[nodiscard]] auto get_DeclName() const -> std::string_view
    {
        ASSERT(map_PropNames.contains(m_name_enum));
        return map_PropNames.at(m_name_enum);
    }

    [[nodiscard]] auto getDefaultValue() const noexcept -> const std::string_view&
    {
        return m_def_value;
    }
    [[nodiscard]] auto getDescription() const noexcept -> const std::string_view& { return m_help; }

    [[nodiscard]] auto get_name() const noexcept -> PropName { return m_name_enum; }
    [[nodiscard]] auto get_type() const noexcept -> PropType { return m_prop_type; }

    [[nodiscard]] auto is_Type(PropType type) const noexcept -> bool { return m_prop_type == type; }
    [[nodiscard]] auto isProp(PropName name) const noexcept -> bool
    {
        return (name == m_name_enum);
    }

    struct Options
    {
        std::string_view name;
        std::string_view help;
    };

    auto getOptions() -> std::vector<Options>& { return m_options; }

private:
    std::string_view m_def_value;
    std::string_view m_help;

    GenEnum::PropType m_prop_type;
    GenEnum::PropName m_name_enum;  // enumeration value for the name

    std::vector<Options> m_options;
};
