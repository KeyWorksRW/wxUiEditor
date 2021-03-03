/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <unordered_set>

#include "node_decl.h"   // NodeDeclaration class
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count
#include "prop_info.h"   // PropDefinition and PropertyInfo classes

#include "node_classes.h"  // Forward defintions of Node classes

class NodeCategory;

using NodeDeclarationMap =
    std::unordered_map<std::string,
                       std::shared_ptr<NodeDeclaration>>;  // std::map<std::string, std::shared_ptr<NodeDeclaration>>

namespace pugi
{
    class xml_document;
    class xml_node;
}  // namespace pugi

bool LoadInternalXmlDocFile(ttlib::cview file, pugi::xml_document& doc);

// Contains definitions of all components
class NodeCreator
{
public:
    NodeCreator() {};
    void Initialize();

    NodeSharedPtr CreateNode(ttlib::cview class_name, Node* parent);
    NodeSharedPtr NewNode(NodeDeclaration* node_info);

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

    const NodeDeclarationMap& GetNodeDeclarationMap() const { return m_node_declarations; }

protected:
    void InitCompTypes();
    void InitDeclarations();

    bool ParseGeneratorFile(ttlib::cview file);

    void ParseCompInfo(pugi::xml_node root);
    void SetupGroup(ttlib::cview file);
    void ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* obj_info, NodeCategory& category, std::set<Type>* types);

    // conversion routines
    Type ParsePropertyType(ttlib::cview str);

    NodeType* GetNodeType(ttlib::cview name);

    size_t CountChildrenWithSameType(Node* parent, NodeType* type);

    void SetDefaultLayoutProperties(Node* node);

    void AddAllConstants();

private:
    NodeDeclarationMap m_node_declarations;

    std::unordered_map<std::string, std::unique_ptr<NodeType>> m_component_types;
    std::map<std::string, Type> m_propTypes;

    std::unordered_set<std::string> m_setOldHostTypes;

    std::unordered_map<std::string, int> m_map_constants;
};

extern NodeCreator g_NodeCreator;
