/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <set>

#include "../panels/base_panel.h"  // BasePanel -- Base class for all code generation panels
#include "gen_enums.h"             // Enumerations for generators
#include "gen_xrc.h"               // BaseXrcGenerator -- Generate XRC file

#include "node_classes.h"  // Forward defintions of Node classes

class Code;
class ProjectSettings;
class NodeCreator;
class WriteCode;
class wxWindow;

struct EmbeddedImage;

namespace pugi
{
    class xml_node;
}

// The NodeEvent class is used to store event information specific to what the user has
// requsted (node containing the event, name of the event handler) along with a pointer to
// the fixed event information.
using EventVector = std::vector<NodeEvent*>;

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

int WriteCMakeFile(Node* parent_node, std::vector<tt_string>& updated_files,
                   std::vector<tt_string>& results);  // See gen_cmake.cpp

class BaseCodeGenerator
{
public:
    BaseCodeGenerator(int language);

    void SetHdrWriteCode(WriteCode* cw) { m_header = cw; }
    void SetSrcWriteCode(WriteCode* cw) { m_source = cw; }

    void GenerateCppClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GeneratePythonClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GenerateRubyClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);

    // The following languages are experimental and may not work correctly

    void GenerateGoLangClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GenerateLuaClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GeneratePerlClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);
    void GenerateRustClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);

    // GenerateDerivedClass() is in gen_derived.cpp

    // Returns result::fail, result::exists, result::created, or result::ignored
    int GenerateDerivedClass(Node* project, Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);

    // code for this is in gen_xrc.cpp
    void GenerateXrcClass(Node* form_node, PANEL_PAGE panel_type = NOT_PANEL);

    void PreviewXrcClass(Node* form_node);

    auto GetHeaderWriter() { return m_header; }
    auto GetSrcWriter() { return m_source; }

    // Write code to m_source that will load any image handlers needed by the form's class
    void GenerateCppHandlers();

    PANEL_PAGE GetPanelType() { return m_panel_type; }

    bool is_cpp() const { return m_language == GEN_LANG_CPLUSPLUS; }

    static void CollectIDs(Node* node, std::set<std::string>& set_enum_ids, std::set<std::string>& set_const_ids);

    // Retrieve a list of any warnings the generators have created
    auto getWarnings() { return m_warnings; }

protected:
    // Generate extern references to images used in the current form that are defined in the
    // gen_Images node.
    //
    // This will call code.clear() before writing any code.
    void WriteImagePreConstruction(Code& code);  // declared in image_gen.cpp

    // Generate code for embedded images not defined in the gen_Images
    // node.
    //
    // This will call code.clear() before writing any code.
    void WriteImageConstruction(Code& code);  // declared in image_gen.cpp

    // Generate extern statements after the header definition for embedded images not defined
    // in the gen_Images node.
    void WriteImagePostHeader();  // declared in image_gen.cpp

    void WritePropSourceCode(Node* node, GenEnum::PropName prop);
    void WritePropHdrCode(Node* node, GenEnum::PropName prop);
    void AddPersistCode(Node* node);
    enum Permission
    {
        Protected,
        Public
    };

    // This method is in images_form.cpp, and handles both source and header code generation
    void GenerateImagesForm();

    // This method is in image_gen.cpp, and handles Python code generation
    void GeneratePythonImagesForm();  // declared in image_gen.cpp

    // This method is in image_gen.cpp, and handles Ruby code generation
    void GenerateRubyImagesForm();  // declared in image_gen.cpp

    tt_string GetDeclaration(Node* node);

    void CollectEventHandlers(Node* node, EventVector& events);
    void CollectImageHeaders(Node* node, std::set<std::string>& embedset);
    void CollectIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr);
    void CollectMemberVariables(Node* node, Permission perm, std::set<std::string>& code_lines);
    void CollectValidatorVariables(Node* node, std::set<std::string>& code_lines);

    void GenerateCppClassHeader(Node* form_node, EventVector& events);
    void GenerateCppClassConstructor(Node* form_node, EventVector& events);

    void GenSrcEventBinding(Node* class_node, EventVector& events);
    void GenHdrEvents(const EventVector& events);
    void GenPythonEventHandlers(EventVector& events);
    void GenRubyEventHandlers(EventVector& events);

    // Generates all the code lines for validator_variables initialized in the header file
    void GenCppValVarsBase(const NodeDeclaration* info, Node* node, std::set<std::string>& code_lines);

    // Recursive function for generating all get/set validator functions in the header file
    void GenCppValidatorFunctions(Node* node);

    // Recursive function for generating all include files needed by any nodes in the form
    void GatherGeneratorIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr);

    // Generates an enum of all use-defined ids
    void GenCppEnumIds(Node* class_node);

    // Determine if Header or Animation functions need to be generated, and whether the
    // wx/artprov.h is needed
    void ParseImageProperties(Node* class_node);

    // implemented in gen_construction.cpp
    void GenConstruction(Node* node);

    // This allows generators to create calls to a widget after it has been created.
    void GenSettings(Node* node);

    // Write everything in the set and then clear it
    void WriteSetLines(WriteCode* pDest, std::set<std::string>& code_lines);

    // Called after base class is fully constructed
    void GenContextMenuHandler(Node* node_ctx_menu);

protected:
    const char* LangPtr() const;
    void BeginPlatformCode(Node* node);
    void EndPlatformCode();
    bool GenAfterChildren(Node* node, bool need_closing_brace);

    // Call if GenAfterChildren() returns false and node's parent is a sizer
    void GenParentSizer(Node* node, bool need_closing_brace);

    // In C++ adds a line with "{" and indents. Other languages just indent.
    void BeginBrace();

    // In C++ unindents, then adds a line with "}". Other languages just unindent.
    void EndBrace();

private:
    WriteCode* m_header;
    WriteCode* m_source;

    tt_string m_baseFullPath;
    tt_string m_header_ext { ".h" };

    EventVector m_CtxMenuEvents;

    std::vector<const EmbeddedImage*> m_embedded_images;
    std::set<wxBitmapType> m_type_generated;
    std::set<std::string> m_set_enum_ids;
    std::set<std::string> m_set_const_ids;

    // Warnings to be displayed to the user when generating code to a file
    std::set<tt_string> m_warnings;

    Node* m_form_node { nullptr };
    Node* m_ImagesForm { nullptr };

    PANEL_PAGE m_panel_type { NOT_PANEL };

    int m_language { GEN_LANG_CPLUSPLUS };

    bool m_is_derived_class { true };

    // These are also initialized whenever GenerateBaseClass() is called
    bool m_NeedArtProviderHeader { false };  // Set when Art type is used
    bool m_NeedHeaderFunction { false };     // Set when Header type is used
    bool m_NeedAnimationFunction { false };  // Set when an Animation image is used
    bool m_NeedSVGFunction { false };        // Set when SVG image type is used
    bool m_NeedImageFunction { false };      // Set when Embed type is used
    bool m_TranslationUnit { true };         // Reflects form->as_bool(prop_generate_translation_unit)
};
