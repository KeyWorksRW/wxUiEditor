/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>

#include "node_classes.h"  // Forward defintions of Node classes

#include "../panels/base_panel.h"  // BasePanel -- Base class for all code generation panels
#include "gen_enums.h"             // Enumerations for generators
#include "gen_xrc.h"               // BaseXrcGenerator -- Generate XRC file

class ProjectSettings;
class NodeCreator;
class WriteCode;
class wxWindow;

struct EmbeddedImage;

using EventVector = std::vector<NodeEvent*>;

namespace pugi
{
    class xml_node;
}

enum PANEL_PAGE : size_t
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
        needs_writing = 3,
    };
}  // namespace result

// This determines the longest line when generating embedded images. Do *not* use constexpr for this -- at some point we may
// want to allow the user to set maximum line length of all generated code, and if so, this will need to reflect the user's
// preference.

constexpr int max_image_line_length { 125 };

int WriteCMakeFile(Node* parent_node, std::vector<ttlib::cstr>& results);  // See gen_cmake.cpp

// If NeedsGenerateCheck is true, this will not write any files, but will return true if at
// least one file needs to be generated. If pClassList is non-null, it will contain the base
// class name of every form that needs updating.
//
// gen_codefiles.cpp
bool GenerateCodeFiles(wxWindow* parent, std::vector<ttlib::cstr>* pClassList = nullptr);

// gen_python.cpp
bool GeneratePythonFiles(wxWindow* parent, bool NeedsGenerateCheck = false, std::vector<ttlib::cstr>* pClassList = nullptr);

// gen_lua.cpp
bool GenerateLuaFiles(wxWindow* parent, bool NeedsGenerateCheck = false, std::vector<ttlib::cstr>* pClassList = nullptr);

// gen_php.cpp
bool GeneratePhpFiles(wxWindow* parent, bool NeedsGenerateCheck = false, std::vector<ttlib::cstr>* pClassList = nullptr);

#if defined(INTERNAL_TESTING)
void GenerateTmpFiles(const std::vector<ttlib::cstr>& ClassList, pugi::xml_node root);
#endif

class BaseCodeGenerator
{
public:
    BaseCodeGenerator(int language);

    void SetHdrWriteCode(WriteCode* cw) { m_header = cw; }
    void SetSrcWriteCode(WriteCode* cw) { m_source = cw; }

    void GenerateBaseClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GeneratePythonClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GenerateLuaClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GeneratePhpClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);

    // GenerateDerivedClass() is in gen_derived.cpp

    // Returns result::fail, result::exists, result::created, or result::ignored
    int GenerateDerivedClass(Node* project, Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);

    // code for this is in gen_xrc.cpp
    void GenerateXrcClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);

    void PreviewXrcClass(Node* form_node);

    auto GetHeaderWriter() { return m_header; }
    auto GetSrcWriter() { return m_source; }

    // Write code to m_source that will load any handlers needed by the form's class
    void GenerateHandlers();

    PANEL_PAGE GetPanelType() { return m_panel_type; }

protected:
    void WritePropSourceCode(Node* node, GenEnum::PropName prop);
    void WritePropHdrCode(Node* node, GenEnum::PropName prop);
    void GenCtxConstruction(Node* node);
    void AddPersistCode(Node* node);
    enum Permission
    {
        Protected,
        Public
    };

    // This method is in images_form.cpp, and handles both source and header code generation
    void GenerateImagesForm();

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

    // Determine if Header or Animation functions need to be generated, and whether the
    // wx/artprov.h is needed
    void ParseImageProperties(Node* class_node);

    // Generate node construction code
    void GenConstruction(Node* node);
    void GenPythonConstruction(Node* node);
    void GenLuaConstruction(Node* node);
    void GenPhpConstruction(Node* node);

    // This allows generators to create calls to a widget after it has been created.
    void GenSettings(Node* node);

    // Write everything in the set and then clear it
    void WriteSetLines(WriteCode* pDest, std::set<std::string>& code_lines);

    // Called after base class is fully constructed
    void GenContextMenuHandler(Node* form_node, Node* node_ctx_menu);

private:
    WriteCode* m_header;
    WriteCode* m_source;

    ttlib::cstr m_baseFullPath;
    ttlib::cstr m_header_ext { ".h" };

    EventVector m_CtxMenuEvents;

    std::vector<const EmbeddedImage*> m_embedded_images;
    std::set<wxBitmapType> m_type_generated;

    Node* m_form_node { nullptr };
    Node* m_ImagesForm { nullptr };
    Project* m_project { nullptr };

    PANEL_PAGE m_panel_type { NOT_PANEL };

    int m_language { GEN_LANG_CPLUSPLUS };

    bool m_is_derived_class { true };

    // These are also initialized whenever GenerateBaseClass() is called
    bool m_NeedArtProviderHeader { false };  // Set when Art type is used
    bool m_NeedHeaderFunction { false };     // Set when Header type is used
    bool m_NeedAnimationFunction { false };  // Set when an Animation image is used
    bool m_NeedSVGFunction { false };        // Set when SVG image type is used
    bool m_NeedImageFunction { false };      // Set when Embed type is used
};
