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
    NodeCreator() = default;

public:
    NodeCreator(NodeCreator const&) = delete;
    void operator=(NodeCreator const&) = delete;
    NodeCreator(NodeCreator&&) = delete;
    void operator=(NodeCreator&&) = delete;
    ~NodeCreator() = default;

    static auto get_Instance() -> NodeCreator&
    {
        static NodeCreator instance;
        return instance;
    }

    void Initialize();

    // Only creates the node if the parent allows it as a child. Returns the node and an
    // error code.
    //
    // If verify_language_support is true, then the node will only be created if the
    // preferred language supports it (unless the user agrees to create it anyway)
    auto CreateNode(GenName name, Node* parent, bool verify_language_support = false)
        -> std::pair<NodeSharedPtr, int>;

    // Only creates the node if the parent allows it as a child. Returns the node and a
    // Node:: error code (see enum in node.h).
    //
    // If verify_language_support is true, then the node will only be created if the
    // preferred language supports it (unless the user agrees to create it anyway)
    auto CreateNode(tt_string_view name, Node* parent, bool verify_language_support = false)
        -> std::pair<NodeSharedPtr, int>;

    NodeSharedPtr CreateNodeFromXml(pugi::xml_node& node, Node* parent = nullptr,
                                    bool check_for_duplicates = false, bool allow_ui = true);

    // Only use this with .wxui projects -- it will fail on a .fbp project
    auto CreateProjectNode(pugi::xml_node* xml_obj, bool allow_ui = true) -> NodeSharedPtr;

    // Creates an orphaned node.
    auto NewNode(NodeDeclaration* node_info) -> NodeSharedPtr;

    // Creates an orphaned node.
    auto NewNode(GenEnum::GenName get_GenName) -> NodeSharedPtr
    {
        return NewNode(m_a_declarations.at(get_GenName));
    }

    // If you have the class enum value, this is the preferred way to get the Declaration
    // pointer.
    auto get_declaration(GenEnum::GenName get_GenName) -> NodeDeclaration*
    {
        return m_a_declarations.at(static_cast<size_t>(get_GenName));
    }

    NodeDeclaration* get_NodeDeclaration(tt_string_view class_name);

    [[nodiscard]] auto get_NodeDeclarationArray() const -> const NodeDeclarationArray&
    {
        return m_a_declarations;
    }

    // This returns the integer value of most wx constants used in various components
    // [[nodiscard]] auto get_ConstantAsInt(const std::string& name, int defValue = 0) const -> int;
    [[nodiscard]] auto get_ConstantAsInt(std::string_view name, int defValue = 0) const -> int;

    // Makes a copy, including the entire child heirarchy. The copy does not have a parent.
    NodeSharedPtr MakeCopy(Node* node, Node* parent = nullptr);

    // Makes a copy, including the entire child heirarchy. The copy does not have a parent.
    auto MakeCopy(const NodeSharedPtr& node) -> NodeSharedPtr { return MakeCopy(node.get()); };

    void InitGenerators();

    [[nodiscard]] auto is_OldHostType(tt_string_view old_type) const -> bool
    {
        return m_setOldHostTypes.contains(old_type);
    }

    // Returns valid parent if there is one, which may be different from the parent passed to
    // the function.
    //
    // Returns nullptr if no parent can be found that allows this child type (which might
    // mean that parent already has the maximum number of children allowed).
    auto is_ValidCreateParent(GenName name, Node* parent, bool use_recursion = true) const -> Node*;

    auto CountChildrenWithSameType(Node* parent, GenType type) const -> size_t;

protected:
    // This must
    void ParseGeneratorFile(const char* xml_data);
    void ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* obj_info,
                         NodeCategory& category);

    auto get_NodeType(GenEnum::GenType type_name) -> NodeType*
    {
        return &m_a_node_types.at(static_cast<size_t>(type_name));
    }

    void AddAllConstants();

private:
    std::array<NodeDeclaration*, gen_name_array_size> m_a_declarations {};
    std::array<NodeType, gen_type_array_size> m_a_node_types;

    std::unordered_set<std::string, str_view_hash, std::equal_to<>> m_setOldHostTypes;

    std::unordered_map<std::string, int> m_map_constants;

    // Contains the nodes that m_interfaces maps to -- valid only during Initialize()
    pugi::xml_document* m_pdoc_interface { nullptr };

    // Contains a map to every interface class -- valid only during Initialize()
    std::map<std::string, pugi::xml_node, std::less<>> m_interfaces;
};

extern NodeCreator& NodeCreation;  // NOLINT (global variable) // cppcheck-suppress globalVariable

// Map of friendly name to wxWidgets constant string
extern const std::unordered_map<std::string_view, std::string_view> g_friend_constant;
