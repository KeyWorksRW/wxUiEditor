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

#include "hash_map.h"  // Find std::string_view key in std::unordered_map

#include "node_decl.h"   // NodeDeclaration class
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count
#include "prop_decl.h"   // PropChildDeclaration and PropDeclaration classes

#include "node_classes.h"  // Forward defintions of Node classes

#include "pugixml.hpp"

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

class NodeCategory;

using NodeDeclarationArray = std::array<NodeDeclaration*, gen_name_array_size>;

// Contains definitions of all components
class NodeCreator
{
private:
    NodeCreator() {}

public:
    NodeCreator(NodeCreator const&) = delete;
    void operator=(NodeCreator const&) = delete;

    static NodeCreator& getInstance()
    {
        static NodeCreator instance;
        return instance;
    }

public:
    void Initialize();

    // Only creates the node if the parent allows it as a child
    NodeSharedPtr createNode(GenName name, Node* parent);

    // Only creates the node if the parent allows it as a child
    NodeSharedPtr createNode(tt_string_view name, Node* parent);

    // Creates an orphaned node.
    NodeSharedPtr NewNode(GenEnum::GenName getGenName) { return NewNode(m_a_declarations[getGenName]); }

    // Creates an orphaned node.
    NodeSharedPtr NewNode(NodeDeclaration* node_info);

    // If you have the class enum value, this is the preferred way to get the Declaration
    // pointer.
    NodeDeclaration* get_declaration(GenEnum::GenName getGenName) { return m_a_declarations[getGenName]; }

    NodeDeclaration* getNodeDeclaration(tt_string_view class_name);

    // This returns the integer value of most wx constants used in various components
    int GetConstantAsInt(const std::string& name, int defValue = 0) const;

    NodeSharedPtr createNode(pugi::xml_node& node, Node* parent = nullptr, bool check_for_duplicates = false,
                             bool allow_ui = true);

    // Only use this with .wxui projects -- it will fail on a .fbp project
    NodeSharedPtr CreateProjectNode(pugi::xml_node* xml_obj, bool allow_ui = true);

    // Makes a copy, including the entire child heirarchy. The copy does not have a parent.
    NodeSharedPtr MakeCopy(Node* node, Node* parent = nullptr);
    NodeSharedPtr MakeCopy(NodeSharedPtr node) { return MakeCopy(node.get()); };

    void InitGenerators();

    bool IsOldHostType(tt_string_view old_type) const { return m_setOldHostTypes.contains(old_type); }

    const NodeDeclarationArray& GetNodeDeclarationArray() const { return m_a_declarations; }

    size_t CountChildrenWithSameType(Node* parent, GenType type);

protected:
    // This must
    void ParseGeneratorFile(const char* file);
    void ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* obj_info, NodeCategory& category);

    NodeType* getNodeType(GenEnum::GenType type_name) { return &m_a_node_types[static_cast<size_t>(type_name)]; }

    void AddAllConstants();

private:
    std::array<NodeDeclaration*, gen_name_array_size> m_a_declarations;
    std::array<NodeType, gen_type_array_size> m_a_node_types;

    std::unordered_set<std::string, str_view_hash, std::equal_to<>> m_setOldHostTypes;

    std::unordered_map<std::string, int> m_map_constants;

    // Contains the nodes that m_interfaces maps to -- valid only during Initialize()
    pugi::xml_document* m_pdoc_interface { nullptr };

    // Contains a map to every interface class -- valid only during Initialize()
    std::map<std::string, pugi::xml_node, std::less<>> m_interfaces;
};

extern NodeCreator& NodeCreation;

// Map of friendly name to wxWidgets constant string
extern std::unordered_map<std::string, const char*> g_friend_constant;
