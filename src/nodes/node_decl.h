/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeDeclaration class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-15-2026]

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
#include "wxue_namespace/wxue_string.h"  // wxue::string, wxue::string_view

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
    NodeDeclaration(wxue::string_view class_name, NodeType* type);

    NodeDeclaration(const NodeDeclaration&) = delete;
    NodeDeclaration& operator=(const NodeDeclaration&) = delete;
    NodeDeclaration(NodeDeclaration&&) = delete;
    NodeDeclaration& operator=(NodeDeclaration&&) = delete;

    // This will delete m_generator which was created by NodeCreator::InitGenerators()
    ~NodeDeclaration();

    NodeCategory& GetCategory() { return m_category; }

    [[nodiscard]] size_t get_PropertyCount() const { return m_properties.size(); }
    [[nodiscard]] size_t get_EventCount() const { return m_events.size(); }

    [[nodiscard]] PropDeclaration* get_PropDeclaration(size_t idx) const;

    [[nodiscard]] const NodeEventInfo* get_EventInfo(wxue::string_view name) const;
    [[nodiscard]] const NodeEventInfo* get_EventInfo(size_t idx) const;

    auto GetPropInfoMap() -> auto& { return m_properties; }
    auto GetEventInfoMap() -> auto& { return m_events; }

    [[nodiscard]] NodeType* get_NodeType() const { return m_type; }
    [[nodiscard]] GenName get_GenName() const noexcept { return m_gen_name; }
    [[nodiscard]] GenType get_GenType() const noexcept { return m_gen_type; }

    [[nodiscard]] bool is_Type(GenType type) const noexcept { return (type == m_gen_type); }
    [[nodiscard]] bool is_Gen(GenName name) const noexcept { return (name == m_gen_name); }

    [[nodiscard]] std::string_view get_DeclName() const noexcept { return m_name; }

    size_t AddBaseClass(NodeDeclaration* base)
    {
        m_base.emplace_back(base);
        return m_base.size() - 1;
    }

    [[nodiscard]] bool isSubclassOf(GenName class_name) const;

    [[nodiscard]] NodeDeclaration* GetBaseClass(size_t idx, bool inherited = true) const;

    void GetBaseClasses(std::vector<NodeDeclaration*>& classes, bool inherited = true) const;

    [[nodiscard]] size_t GetBaseClassCount(bool inherited = true) const;
    [[nodiscard]] bool hasBaseClasses() const { return !m_base.empty(); }

    void SetBundleFunction(std::function<wxBitmapBundle(int width, int height)> func)
    {
        m_bundle_function = std::move(func);
    }
    void SetImage(wxImage image) { m_image = std::move(image); }
    [[nodiscard]] wxImage GetImage() const;
    [[nodiscard]] wxBitmapBundle GetBitmapBundle(int width, int height) const;

    void SetGenerator(BaseGenerator* generator) { m_generator = generator; }
    [[nodiscard]] BaseGenerator* get_Generator() const { return m_generator; }

    void ParseEvents(pugi::xml_node& elem_obj, NodeCategory& category);

    const std::string& GetGeneratorFlags() const { return m_internal_flags; }
    void SetGeneratorFlags(std::string_view flags) { m_internal_flags = flags; }

    [[nodiscard]] ptrdiff_t get_AllowableChildren(GenType child_gen_type) const;

    void SetOverRideDefValue(GenEnum::PropName prop_name, std::string_view new_value)
    {
        m_override_def_values[prop_name] = new_value;
    }
    std::optional<wxue::string> GetOverRideDefValue(GenEnum::PropName prop_name) const;

    void HideProperty(GenEnum::PropName prop_name) { m_hide_properties.emplace(prop_name); }
    bool IsPropHidden(GenEnum::PropName prop_name) const
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
