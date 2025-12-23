/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements BaseCodeGenerator, an abstract base class for generating
// complete source and header files in C++, Python, Perl, and Ruby. Derived classes
// (CppCodeGenerator, PythonCodeGenerator, etc.) must implement GenerateClass() which orchestrates
// code generation through WriteCode objects (m_header and m_source). The class manages event
// collection (m_events, m_ctx_menu_events, m_map_conditional_events), embedded image tracking
// (m_embedded_images), platform- conditional code sections (m_map_public_members, m_map_protected),
// and ID sets (m_set_enum_ids, m_set_const_ids). Generation flow: CollectEventHandlers →
// GenConstruction → GenSettings → GenAfterChildren, with helper methods (BeginBrace, EndBrace,
// GenSrcEventBinding) adapting output to m_language. The class coordinates with Code class for
// language-specific syntax and tracks generation state (m_NeedArtProviderHeader, m_NeedSVGFunction)
// to optimize includes and helper function generation.

#pragma once

#include <mutex>
#include <set>

#include "../panels/base_panel.h"  // BasePanel -- Base class for all code generation panels
#include "gen_enums.h"             // Enumerations for generators
#include "gen_results.h"           // Code generation file writing functions
#include "tt_string_vector.h"      // tt_string_vector -- Read/Write line-oriented strings/files

class Code;
class Node;
class NodeCreator;
class NodeEvent;
class ProjectSettings;
class WriteCode;
class wxWindow;

class EmbeddedImage;

// The NodeEvent class is used to store event information specific to what the user has
// requsted (node containing the event, name of the event handler) along with a pointer to
// the fixed event information.
using EventVector = std::vector<NodeEvent*>;

namespace result
{
    // These enums are returned to indicate the result of generating a file
    enum : std::int8_t
    {
        fail = -1,
        exists = 0,
        created = 1,
        ignored = 2,
        needs_writing = 3,
    };
}  // namespace result

// flag == 1 for test only, flag == 2 if temp filename in updated_files
auto WriteCMakeFile(Node* parent_node, GenResults& results, int flag = 0)
    -> int;  // See gen_cmake.cpp

class BaseCodeGenerator
{
public:
    BaseCodeGenerator(const BaseCodeGenerator&) = delete;
    auto operator=(const BaseCodeGenerator&) -> BaseCodeGenerator& = delete;
    BaseCodeGenerator(BaseCodeGenerator&&) = delete;
    auto operator=(BaseCodeGenerator&&) -> BaseCodeGenerator& = delete;

    BaseCodeGenerator(GenLang language, Node* form_node);
    virtual ~BaseCodeGenerator() = default;

    void SetHdrWriteCode(WriteCode* code_to_write) { m_header = code_to_write; }
    void SetSrcWriteCode(WriteCode* code_to_write) { m_source = code_to_write; }

    // All language generators must implement this method.
    virtual void GenerateClass(GenLang language, PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL) = 0;

    // CppCodeGenerator is the only derived class that implements this method.
    virtual auto GenerateDerivedClass(Node* /* project */, Node* /* form_node */,
                                      PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL) -> int
    {
        m_panel_type = panel_type;
        return result::fail;
    }

    auto GetHeaderWriter() { return m_header; }
    auto GetSrcWriter() { return m_source; }

    auto GetPanelType() -> PANEL_PAGE { return m_panel_type; }

    static void CollectIDs(Node* node, std::set<std::string>& set_enum_ids,
                           std::set<std::string>& set_const_ids);

    // Retrieve a list of any warnings the generators have created
    auto getWarnings() { return m_warnings; }

protected:
    // Generate code for embedded images not defined in the gen_Images
    // node.
    //
    // This will call code.clear() before writing any code.
    void WriteImageConstruction(Code& code);  // declared in image_gen.cpp

    auto WritePropSourceCode(Node* node, GenEnum::PropName prop) -> void;
    auto AddPersistCode(Node* node) -> void;
    enum class Permission : std::uint8_t
    {
        Protected,
        Public
    };

    // This method is in gen_images.cpp, and handles both source and header code generation
    void GenerateImagesForm();

    [[nodiscard]] static auto GetDeclaration(Node* node) -> tt_string;

    auto CollectEventHandlers(Node* node, EventVector& events) -> void;

    // m_language and m_form_node must be set first. This will add to m_embedded_images
    auto CollectImageHeaders(Node* node, std::set<std::string>& embedset) -> void;

    void GenSrcEventBinding(Node* class_node, EventVector& events);

    // Determine if Header or Animation functions need to be generated, and whether the
    // wx/artprov.h is needed.
    //
    // Requires m_ImagesForm to be set before calling
    auto ParseImageProperties(Node* class_node) -> void;

    // implemented in gen_construction.cpp
    void GenConstruction(Node* node);

    // This allows generators to create calls to a widget after it has been created.
    void GenSettings(Node* node, bool within_brace = false);

    // Write everything in the set and then clear it
    static auto WriteSetLines(WriteCode* write_code, std::set<std::string>& set_lines) -> void;

    // Called after base class is fully constructed
    auto GenContextMenuHandler(Node* node_ctx_menu) -> void;

    // Call this to set m_ImagesForm
    auto SetImagesForm() -> void;

    void BeginPlatformCode(Code& code, const tt_string& platforms);
    void EndPlatformCode();
    auto GenAfterChildren(Node* node, bool need_closing_brace) -> bool;

    // Call if GenAfterChildren() returns false and node's parent is a sizer
    void GenParentSizer(Node* node, bool need_closing_brace);

    // In C++ adds a line with "{" and indents. Other languages just indent.
    void BeginBrace();

    // In C++ unindents, then adds a line with "}". Other languages just unindent.
    void EndBrace();

    // NOLINTBEGIN
    // We expect derived classes to be able to access these variables, so they must remain
    // protected, not private.
protected:
    WriteCode* m_header;
    WriteCode* m_source;

    tt_string m_baseFullPath;
    tt_string m_header_ext { ".h" };

    std::vector<NodeEvent*> m_ctx_menu_events;
    std::vector<NodeEvent*> m_events;

    // Maps platorm string to vector of NodeEvent pointers
    std::map<tt_string, std::vector<NodeEvent*>> m_map_conditional_events;

    // Maps platorm string to set of public: member declarations
    std::map<tt_string, std::set<tt_string>> m_map_public_members;

    // Maps platorm string to set of protected: member declarations
    std::map<tt_string, std::set<tt_string>> m_map_protected;

    std::vector<const EmbeddedImage*> m_embedded_images;
    std::set<wxBitmapType> m_type_generated;
    std::set<std::string> m_set_enum_ids;
    std::set<std::string> m_set_const_ids;

    // Warnings to be displayed to the user when generating code to a file
    std::set<tt_string> m_warnings;

    Node* m_form_node { nullptr };
    Node* m_ImagesForm { nullptr };
    tt_string m_include_images_statement;

    PANEL_PAGE m_panel_type { PANEL_PAGE::NOT_PANEL };

    GenLang m_language { GEN_LANG_CPLUSPLUS };

    bool m_is_derived_class { true };

    // These are also initialized whenever GenerateBaseClass() is called
    bool m_NeedArtProviderHeader { false };  // Set when Art type is used
    bool m_NeedHeaderFunction { false };     // Set when Header type is used
    bool m_NeedAnimationFunction { false };  // Set when an Animation image is used
    bool m_NeedSVGFunction { false };        // Set when SVG image type is used
    bool m_NeedImageFunction { false };
    // NOLINTEND
private:
    // Helper methods for CollectEventHandlers - reduce function complexity
    // Adds event to conditional events map, checking for duplicates
    auto AddConditionalEvent(std::string_view platform, NodeEvent* event) -> void;

    // Adds event to context menu events or regular events based on parent type
    auto AddEventToProperContainer(Node* node, NodeEvent* event, EventVector& events) -> void;

    // Processes a single event, determining where it should be stored
    auto ProcessEventHandler(Node* node, NodeEvent* event, EventVector& events) -> void;

    // Helper methods for CollectImageHeaders - reduce function complexity
    // Processes embedded or SVG images from a bundle
    auto ProcessEmbeddedImages(const std::vector<tt_string>& filenames) -> void;

    // Processes header or XPM images from a bundle
    auto ProcessHeaderImages(Node* node, const std::vector<tt_string>& filenames,
                             std::set<std::string>& embedset) -> void;

    // Processes animation embed data
    auto ProcessAnimationEmbed(std::string_view value) -> void;

    // Processes animation header or XPM data
    auto ProcessAnimationHeaders(std::string_view value, Node* node,
                                 std::set<std::string>& embedset) -> void;

    // Helper to check if embedded image already exists in collection
    [[nodiscard]] auto IsEmbeddedImageInCollection(const EmbeddedImage* embed) -> bool;

    // Helper methods for ParseImageProperties - reduce function complexity
    // Processes icon property for form nodes
    auto ProcessFormIcon(Node* node) -> void;

    // Processes embed type images/animations from child node
    auto ProcessChildEmbedType(const tt_string_vector& parts, bool is_animation) -> void;

    // Processes SVG type images/animations from child node
    auto ProcessChildSVGType(const tt_string_vector& parts, bool is_animation) -> void;

    // Processes header or XPM type images/animations from child node
    auto ProcessChildHeaderType(const tt_string_vector& parts, bool is_animation) -> void;

    // Helper methods for GetDeclaration - reduce function complexity
    // Processes wx class declarations (wxStdDialogButtonSizer, wxStaticBitmap, etc)
    static auto ProcessWxClassDeclaration(const tt_string& class_name, Node* node, tt_string& code)
        -> void;

    // Processes special custom class declarations
    static auto ProcessCustomClassDeclaration(Node* node, tt_string& code) -> void;

    // Processes tool class declarations based on parent type
    static auto ProcessToolDeclaration(Node* node, tt_string& code) -> void;

    // Processes StaticCheckboxBoxSizer or StaticRadioBtnBoxSizer declarations
    static auto ProcessStaticBoxSizerDeclaration(const tt_string& class_name, Node* node,
                                                 tt_string& code) -> void;

    std::mutex m_embedded_images_mutex;  // Protects m_embedded_images from concurrent access
};
