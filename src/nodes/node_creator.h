/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <map>
#include <unordered_set>
#include <utility>  // for std::pair

#include "hash_map.h"  // Find std::string_view key in std::unordered_map

#include "node_decl.h"   // NodeDeclaration class
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count

#include "pugixml.hpp"

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

class Node;
class NodeCategory;

using NodeDeclarationArray = std::array<NodeDeclaration*, gen_name_array_size>;
using NodeSharedPtr = std::shared_ptr<Node>;

// Contains definitions of all components
class NodeCreator
{
private:
    NodeCreator() {}

public:
    NodeCreator(NodeCreator const&) = delete;
    void operator=(NodeCreator const&) = delete;

    static NodeCreator& get_Instance()
    {
        static NodeCreator instance;
        return instance;
    }

public:
    void Initialize();

    // Only creates the node if the parent allows it as a child. Returns the node and an
    // error code.
    //
    // If verify_language_support is true, then the node will only be created if the
    // preferred language supports it (unless the user agrees to create it anyway)
    std::pair<NodeSharedPtr, int> CreateNode(GenName name, Node* parent,
                                             bool verify_language_support = false);

    // Only creates the node if the parent allows it as a child. Returns the node and a
    // Node:: error code (see enum in node.h).
    //
    // If verify_language_support is true, then the node will only be created if the
    // preferred language supports it (unless the user agrees to create it anyway)
    std::pair<NodeSharedPtr, int> CreateNode(tt_string_view name, Node* parent,
                                             bool verify_language_support = false);

    NodeSharedPtr CreateNodeFromXml(pugi::xml_node& node, Node* parent = nullptr,
                                    bool check_for_duplicates = false, bool allow_ui = true);

    // Only use this with .wxui projects -- it will fail on a .fbp project
    NodeSharedPtr CreateProjectNode(pugi::xml_node* xml_obj, bool allow_ui = true);

    // Creates an orphaned node.
    NodeSharedPtr NewNode(NodeDeclaration* node_info);

    // Creates an orphaned node.
    NodeSharedPtr NewNode(GenEnum::GenName get_GenName)
    {
        return NewNode(m_a_declarations[get_GenName]);
    }

    // If you have the class enum value, this is the preferred way to get the Declaration
    // pointer.
    NodeDeclaration* get_declaration(GenEnum::GenName get_GenName)
    {
        return m_a_declarations[get_GenName];
    }

    NodeDeclaration* get_NodeDeclaration(tt_string_view class_name);

    const NodeDeclarationArray& get_NodeDeclarationArray() const { return m_a_declarations; }

    // This returns the integer value of most wx constants used in various components
    int get_ConstantAsInt(const std::string& name, int defValue = 0) const;

    // Makes a copy, including the entire child heirarchy. The copy does not have a parent.
    NodeSharedPtr MakeCopy(Node* node, Node* parent = nullptr);

    // Makes a copy, including the entire child heirarchy. The copy does not have a parent.
    NodeSharedPtr MakeCopy(NodeSharedPtr node) { return MakeCopy(node.get()); };

    void InitGenerators();

    bool is_OldHostType(tt_string_view old_type) const
    {
        return m_setOldHostTypes.contains(old_type);
    }

    // Returns valid parent if there is one, which may be different from the parent passed to
    // the function.
    //
    // Returns nullptr if no parent can be found that allows this child type (which might
    // mean that parent already has the maximum number of children allowed).
    Node* is_ValidCreateParent(GenName name, Node* parent, bool use_recursion = true) const;

    size_t CountChildrenWithSameType(Node* parent, GenType type) const;

protected:
    // This must
    void ParseGeneratorFile(const char* file);
    void ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* obj_info,
                         NodeCategory& category);

    NodeType* get_NodeType(GenEnum::GenType type_name)
    {
        return &m_a_node_types[static_cast<size_t>(type_name)];
    }

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
