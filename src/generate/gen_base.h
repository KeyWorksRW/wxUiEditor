/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>

#include "node_classes.h"  // Forward defintions of Node classes

class ProjectSettings;
class NodeCreator;
class WriteCode;
class wxWindow;

using EventVector = std::vector<NodeEvent*>;

enum PANEL_TYPE : size_t
{
    NOT_PANEL,
    CPP_PANEL,
    HDR_PANEL,
};

namespace result
{
    // These enums are returned to indicate the result of generating a file
    enum
    {
        fail = -1,
        exists = 0,
        created = 1,
        ignored = 2,
    };
}  // namespace result

// If NeedsGenerateCheck is true, this will not write any files, but will return true if at
// least one file needs to be generated.
bool GenerateCodeFiles(wxWindow* parent, bool NeedsGenerateCheck = false);

class BaseCodeGenerator
{
public:
    BaseCodeGenerator();

    void SetHdrWriteCode(WriteCode* cw) { m_header = cw; }
    void SetSrcWriteCode(WriteCode* cw) { m_source = cw; }

    void GenerateBaseClass(Node* project, Node* form_node, PANEL_TYPE panel_type = NOT_PANEL);

    // GenerateDerivedClass() is in gen_derived.cpp

    // Returns result::fail, result::exists, result::created, or result::ignored
    int GenerateDerivedClass(Node* project, Node* form_node, PANEL_TYPE panel_type = NOT_PANEL);

protected:
    void GenCtxConstruction(Node* node);
    void AddPersistCode(Node* node);
    enum Permission
    {
        Protected,
        Public
    };

    ttlib::cstr GetDeclaration(Node* node);

    void CollectEventHandlers(Node* node, EventVector& events);
    void CollectIDs(Node* node, std::set<std::string>& set_ids);
    void CollectImageHeaders(Node* node, std::set<std::string>& embedset);
    void CollectIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr);
    void CollectMemberVariables(Node* node, Permission perm, std::set<std::string>& code_lines);
    void CollectValidatorVariables(Node* node, std::set<std::string>& code_lines);

    void GenerateClassHeader(Node* form_node, const EventVector& events);
    void GenerateClassConstructor(Node* form_node, const EventVector& events);

    void GenSrcEventBinding(Node* class_node, const EventVector& events);
    void GenHdrEvents(const EventVector& events);

    // Generates all the code lines for validator_variables initialized in the header file
    void GenValVarsBase(const NodeDeclaration* info, Node* node, std::set<std::string>& code_lines);

    // Recursive function for generating all get/set validator functions in the header file
    void GenValidatorFunctions(Node* node);

    // Recursive function for generating all include files needed by any nodes in the form
    void GatherGeneratorIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr);

    // Generates an enum of all use-defined ids
    void GenEnumIds(Node* class_node);

    // Returns true if there is at least one image data Header file
    bool FindImageHeader(Node* class_node);

    // Returns true if there is at least one animation data Header file
    bool FindAnimationHeader(Node* class_node);

    // Generate node construction code
    void GenConstruction(Node* node);

    // This allows generators to create calls to a widget after it has been created.
    void GenSettings(Node* node);

    void CheckForArtProvider(Node* node);

    // Write everything in the set and then clear it
    void WriteSetLines(WriteCode* pDest, std::set<std::string>& code_lines);

    // Called after base class is fully constructed
    void GenContextMenuHandler(Node* form_node, Node* node_ctx_menu);

private:
    WriteCode* m_header;
    WriteCode* m_source;

    ttlib::cstr m_baseFullPath;
    EventVector m_CtxMenuEvents;

    PANEL_TYPE m_panel_type { NOT_PANEL };
    bool m_artProvider;
};
