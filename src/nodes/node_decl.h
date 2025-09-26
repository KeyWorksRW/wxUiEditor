/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeDeclaration class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/bmpbndl.h>  // Declaration of wxBitmapBundle class.
#include <wx/image.h>    // wxImage class

#include <map>
#include <optional>
#include <set>
#include <utility>

#include "category.h"    // NodeCategory -- Node property categories
#include "gen_enums.h"   // Enumerations for generators
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count

class BaseGenerator;
class NodeDeclaration;
class NodeEventInfo;
class PropDeclaration;

using namespace GenEnum;

using DeclPropMap = std::map<std::string, PropDeclaration*>;
using DeclEventMap = std::map<std::string, NodeEventInfo*, std::less<>>;

namespace pugi
{
    class xml_node;
}

class NodeDeclaration
{
public:
    NodeDeclaration(tt_string_view class_name, NodeType* type);

    NodeDeclaration(const NodeDeclaration&) = delete;
    auto operator=(const NodeDeclaration&) -> NodeDeclaration& = delete;
    NodeDeclaration(NodeDeclaration&&) = delete;
    auto operator=(NodeDeclaration&&) -> NodeDeclaration& = delete;

    // This will delete m_generator which was created by NodeCreator::InitGenerators()
    ~NodeDeclaration();

    auto GetCategory() -> NodeCategory& { return m_category; }

    [[nodiscard]] auto get_PropertyCount() const -> size_t { return m_properties.size(); }
    [[nodiscard]] auto get_EventCount() const -> size_t { return m_events.size(); }

    [[nodiscard]] auto get_PropDeclaration(size_t idx) const -> PropDeclaration*;

    [[nodiscard]] auto get_EventInfo(tt_string_view name) const -> const NodeEventInfo*;
    [[nodiscard]] auto get_EventInfo(size_t idx) const -> const NodeEventInfo*;

    auto GetPropInfoMap() -> auto& { return m_properties; }
    auto GetEventInfoMap() -> auto& { return m_events; }

    [[nodiscard]] auto get_NodeType() const -> NodeType* { return m_type; }
    [[nodiscard]] auto get_GenName() const noexcept -> GenName { return m_gen_name; }
    [[nodiscard]] auto get_GenType() const noexcept -> GenType { return m_gen_type; }

    [[nodiscard]] auto is_Type(GenType type) const noexcept -> bool { return (type == m_gen_type); }
    [[nodiscard]] auto is_Gen(GenName name) const noexcept -> bool { return (name == m_gen_name); }

    [[nodiscard]] auto get_DeclName() const noexcept -> std::string_view { return m_name; }

    auto AddBaseClass(NodeDeclaration* base) -> size_t
    {
        m_base.emplace_back(base);
        return m_base.size() - 1;
    }

    [[nodiscard]] auto isSubclassOf(GenName class_name) const -> bool;

    [[nodiscard]] auto GetBaseClass(size_t idx, bool inherited = true) const -> NodeDeclaration*;

    void GetBaseClasses(std::vector<NodeDeclaration*>& classes, bool inherited = true) const;

    [[nodiscard]] auto GetBaseClassCount(bool inherited = true) const -> size_t;
    [[nodiscard]] auto hasBaseClasses() const -> bool { return !m_base.empty(); }

    void SetBundleFunction(std::function<wxBitmapBundle(int width, int height)> func)
    {
        m_bundle_function = std::move(func);
    }
    void SetImage(wxImage image) { m_image = std::move(image); }
    [[nodiscard]] auto GetImage() const -> wxImage;
    [[nodiscard]] auto GetBitmapBundle(int width, int height) const -> wxBitmapBundle;

    void SetGenerator(BaseGenerator* generator) { m_generator = generator; }
    [[nodiscard]] auto get_Generator() const -> BaseGenerator* { return m_generator; }

    void ParseEvents(pugi::xml_node& elem_obj, NodeCategory& category);

    auto GetGeneratorFlags() -> const std::string& { return m_internal_flags; }
    void SetGeneratorFlags(std::string_view flags) { m_internal_flags = flags; }

    [[nodiscard]] auto get_AllowableChildren(GenType child_gen_type) const -> ptrdiff_t;

    void SetOverRideDefValue(GenEnum::PropName prop_name, std::string_view new_value)
    {
        m_override_def_values[prop_name] = new_value;
    }
    auto GetOverRideDefValue(GenEnum::PropName prop_name) -> std::optional<tt_string>;

    void HideProperty(GenEnum::PropName prop_name) { m_hide_properties.emplace(prop_name); }
    auto IsPropHidden(GenEnum::PropName prop_name) -> bool
    {
        return (m_hide_properties.contains(prop_name));
    }

private:
    std::string m_internal_flags;

    wxImage m_image;  // The node's image, primarily used in the navigation pane

    // If there is an svg file for this declaration, then this function can be used to
    // retrieve it at whatever size is needed.
    std::function<wxBitmapBundle(int width, int height)> m_bundle_function { nullptr };

    NodeType* m_type;

    NodeCategory m_category;

    std::map<std::string, PropDeclaration*> m_properties;
    std::map<std::string, NodeEventInfo*, std::less<>> m_events;

    std::map<GenEnum::PropName, std::string> m_override_def_values;
    std::set<GenEnum::PropName> m_hide_properties;

    std::vector<NodeDeclaration*> m_base;  // base classes

    // Created by NodeCreator::InitGenerators(), destroyed by ~NodeDeclaration()
    BaseGenerator* m_generator { nullptr };

    GenName m_gen_name;
    GenType m_gen_type;
    std::string_view m_name;  // this points into GenEnum::map_GenNames
};
