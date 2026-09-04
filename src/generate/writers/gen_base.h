/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <mutex>
#include <set>

#include "../panels/base_panel.h"        // BasePanel -- Base class for all code generation panels
#include "gen_enums.h"                   // Enumerations for generators
#include "gen_results.h"                 // Code generation file writing functions
#include "wxue_namespace/wxue_string.h"  // wxue::string, wxue::string_view
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

class Code;
class Node;
class NodeCreator;
class NodeEvent;
class ProjectSettings;
class WriteCode;
class wxWindow;
class wxProgressDialog;

class EmbeddedImage;

#include "language_traits.h"  // LanguageTraits, LanguageStrategy

// The maps of platform-specific code (m_map_conditional_events, m_map_public_members,
// m_map_protected) are keyed by this composite string: the platform spec, optionally followed
// by '\x1f' and the user condition. SplitCondKey() reverses it.
inline wxue::string MakeCondKey(std::string_view platforms, std::string_view conditional)
{
    if (conditional.empty())
    {
        return wxue::string(platforms);
    }
    return wxue::string(platforms) << '\x1f' << conditional;
}

inline void SplitCondKey(std::string_view key, wxue::string& platforms, wxue::string& conditional)
{
    if (auto pos = key.find('\x1f'); pos != std::string_view::npos)
    {
        platforms = wxue::string(key.substr(0, pos));
        conditional = wxue::string(key.substr(pos + 1));
    }
    else
    {
        platforms = key;
        conditional.clear();
    }
}

// The NodeEvent class is used to store event information specific to what the user has
// requested (node containing the event, name of the event handler) along with a pointer to
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

    constexpr int progress_image_step = 25;
}  // namespace result

// flag == 1 for test only, flag == 2 if temp filename in updated_files
int WriteCMakeFile(Node* parent_node, GenResults& results, int flag = 0);  // See gen_cmake.cpp

class BaseCodeGenerator
{
public:
    BaseCodeGenerator(const BaseCodeGenerator&) = delete;
    BaseCodeGenerator& operator=(const BaseCodeGenerator&) = delete;
    BaseCodeGenerator(BaseCodeGenerator&&) = delete;
    BaseCodeGenerator& operator=(BaseCodeGenerator&&) = delete;

    BaseCodeGenerator(GenLang language, Node* form_node);
    virtual ~BaseCodeGenerator() = default;

    void SetHdrWriteCode(WriteCode* code_to_write) { m_header = code_to_write; }
    void SetSrcWriteCode(WriteCode* code_to_write) { m_source = code_to_write; }

    // All language generators must implement this method.
    // If progress is provided, GenerateClass can call progress->Update() for long operations.
    virtual void GenerateClass(GenLang language, PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL,
                               wxProgressDialog* progress = nullptr) = 0;

    // CppCodeGenerator is the only derived class that implements this method.
    virtual int GenerateDerivedClass(Node* /* form_node */,
                                     PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL)
    {
        m_panel_type = panel_type;
        return result::fail;
    }

    auto GetHeaderWriter() { return m_header; }
    auto GetSrcWriter() { return m_source; }

    PANEL_PAGE GetPanelType() { return m_panel_type; }

    // Returns the language strategy for the current generation language.
    // May be nullptr if the language doesn't have a strategy yet.
    [[nodiscard]] LanguageStrategy* get_strategy() const { return m_strategy.get(); }

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

    void WritePropSourceCode(Node* node, GenEnum::PropName prop);
    void AddPersistCode(Node* node);
    enum class Permission : std::uint8_t
    {
        Protected,
        Public
    };

    // This method is in gen_images.cpp, and handles both source and header code generation
    void GenerateImagesForm(wxProgressDialog* progress = nullptr);

    [[nodiscard]] static wxue::string GetDeclaration(Node* node);

    void CollectEventHandlers(Node* node, EventVector& events);

    // m_language and m_form_node must be set first. This will add to m_embedded_images
    void CollectImageHeaders(Node* node, std::set<std::string>& embedset);

    void GenSrcEventBinding(Node* class_node, EventVector& events);

    // Determine if Header or Animation functions need to be generated, and whether the
    // wx/artprov.h is needed.
    //
    // Requires m_ImagesForm to be set before calling
    void ParseImageProperties(Node* class_node);

    // implemented in gen_construction.cpp
    void GenConstruction(Node* node);

    // This allows generators to create calls to a widget after it has been created.
    void GenSettings(Node* node, bool within_brace = false);

    // Write everything in the set and then clear it
    static void WriteSetLines(WriteCode* write_code, std::set<std::string>& set_lines);

    // Called after base class is fully constructed
    void GenContextMenuHandler(Node* node_ctx_menu);

    // Call this to set m_ImagesForm
    void SetImagesForm();

    void BeginPlatformCode(Code& code, const wxue::string& platforms,
                           std::string_view conditional = {});
    void EndPlatformCode();
    bool GenAfterChildren(Node* node, bool need_closing_brace);

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

    wxue::string m_baseFullPath;
    wxue::string m_header_ext { ".h" };

    std::vector<NodeEvent*> m_ctx_menu_events;
    std::vector<NodeEvent*> m_events;

    // Maps platform string to vector of NodeEvent pointers
    std::map<wxue::string, std::vector<NodeEvent*>> m_map_conditional_events;

    // Maps platform string to set of public: member declarations
    std::map<wxue::string, std::set<wxue::string>> m_map_public_members;

    // Maps platform string to set of protected: member declarations
    std::map<wxue::string, std::set<wxue::string>> m_map_protected;

    std::vector<const EmbeddedImage*> m_embedded_images;
    std::set<wxBitmapType> m_type_generated;
    std::set<std::string> m_set_enum_ids;
    std::set<std::string> m_set_const_ids;

    // Warnings to be displayed to the user when generating code to a file
    std::set<wxue::string> m_warnings;

    Node* m_form_node { nullptr };
    Node* m_ImagesForm { nullptr };
    wxue::string m_include_images_statement;

    PANEL_PAGE m_panel_type { PANEL_PAGE::NOT_PANEL };

    GenLang m_language { GenLang::cplusplus };
    std::unique_ptr<LanguageStrategy> m_strategy;

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
    void AddConditionalEvent(std::string_view platform, NodeEvent* event);

    // Adds event to context menu events or regular events based on parent type
    void AddEventToProperContainer(Node* node, NodeEvent* event, EventVector& events);

    // Processes a single event, determining where it should be stored
    void ProcessEventHandler(Node* node, NodeEvent* event, EventVector& events);

    // Helper methods for CollectImageHeaders - reduce function complexity
    // Processes embedded or SVG images from a bundle
    void ProcessEmbeddedImages(const std::vector<wxue::string>& filenames);

    // Processes header or XPM images from a bundle
    void ProcessHeaderImages(Node* node, const std::vector<wxue::string>& filenames,
                             std::set<std::string>& embedset);

    // Processes animation embed data
    void ProcessAnimationEmbed(std::string_view value);

    // Processes animation header or XPM data
    void ProcessAnimationHeaders(std::string_view value, Node* node,
                                 std::set<std::string>& embedset);

    // Helper to check if embedded image already exists in collection
    [[nodiscard]] bool IsEmbeddedImageInCollection(const EmbeddedImage* embed);

    // Helper methods for ParseImageProperties - reduce function complexity
    // Processes icon property for form nodes
    void ProcessFormIcon(Node* node);

    // Processes embed type images/animations from child node
    void ProcessChildEmbedType(const wxue::StringVector& parts, bool is_animation);

    // Processes SVG type images/animations from child node
    void ProcessChildSVGType(const wxue::StringVector& parts, bool is_animation);

    // Processes header or XPM type images/animations from child node
    void ProcessChildHeaderType(const wxue::StringVector& parts, bool is_animation);

    // Helper methods for GetDeclaration - reduce function complexity
    // Processes wx class declarations (wxStdDialogButtonSizer, wxStaticBitmap, etc)
    static void ProcessWxClassDeclaration(const wxue::string& class_name, Node* node,
                                          wxue::string& code);

    // Processes special custom class declarations
    static void ProcessCustomClassDeclaration(Node* node, wxue::string& code);

    // Processes tool class declarations based on parent type
    static void ProcessToolDeclaration(Node* node, wxue::string& code);

    // Processes StaticCheckboxBoxSizer or StaticRadioBtnBoxSizer declarations
    static void ProcessStaticBoxSizerDeclaration(const wxue::string& class_name, Node* node,
                                                 wxue::string& code);

    // Writes lambda event code lines with proper indentation tracking
    void WriteLambdaEventLines(Code& code);

    std::mutex m_embedded_images_mutex;  // Protects m_embedded_images from concurrent access
};
