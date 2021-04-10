/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <map>
#include <set>
#include <unordered_set>

#include "node_decl.h"   // NodeDeclaration class
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count
#include "prop_info.h"   // PropDefinition and PropertyInfo classes

#include "node_classes.h"  // Forward defintions of Node classes

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

class NodeCategory;

using NodeDeclarationArray = std::array<NodeDeclaration*, gen_name_array_size>;

namespace pugi
{
    class xml_document;
    class xml_node;
}  // namespace pugi

// Contains definitions of all components
class NodeCreator
{
public:
    NodeCreator() {};
    ~NodeCreator();

    void Initialize();

    NodeSharedPtr CreateNode(ttlib::cview class_name, Node* parent);
    NodeSharedPtr NewNode(NodeDeclaration* node_info);

    // If you have the class enum value, this is the preferred way to get the Declaration
    // pointer.
    NodeDeclaration* get_declaration(GenEnum::GenName class_enum)
    {
        return m_a_declarations[static_cast<size_t>(class_enum)];
    }

    NodeDeclaration* GetNodeDeclaration(ttlib::cview class_name);

    // This returns the integer value of most wx constants used in various components
    int GetConstantAsInt(const std::string& name, int defValue = 0);

    // Only use this with .wxui projects -- it will fail on a .fbp project
    NodeSharedPtr CreateNode(pugi::xml_node& node, Node* parent = nullptr);

    NodeSharedPtr MakeCopy(Node* node);
    NodeSharedPtr MakeCopy(NodeSharedPtr node) { return MakeCopy(node.get()); };

    void InitGenerators();

    bool IsOldHostType(ttlib::cview old_type)
    {
        return (m_setOldHostTypes.find(old_type.c_str()) != m_setOldHostTypes.end());
    }

    int_t GetAllowableChildren(Node* parent, ttlib::cview child_name, bool is_aui_parent = false) const;
    int_t GetAllowableChildren(Node* parent, GenEnum::GenType child_class_type, bool is_aui_parent = false) const;

    const NodeDeclarationArray& GetNodeDeclarationArray() const { return m_a_declarations; }

protected:
    void ParseGeneratorFile(ttlib::cview file);
    void ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* obj_info, NodeCategory& category);

    NodeType* GetNodeType(GenEnum::GenType type_name) { return &m_a_node_types[static_cast<size_t>(type_name)]; }

    size_t CountChildrenWithSameType(Node* parent, NodeType* type);

    void SetDefaultLayoutProperties(Node* node);

    void AddAllConstants();

private:
    std::array<NodeDeclaration*, gen_name_array_size> m_a_declarations;
    std::array<NodeType, gen_type_array_size> m_a_node_types;

    std::unordered_set<std::string> m_setOldHostTypes;

    std::unordered_map<std::string, int> m_map_constants;
};

extern NodeCreator g_NodeCreator;
