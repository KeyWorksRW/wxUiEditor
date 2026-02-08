/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate wxPerl code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_base.h"  // BaseCodeGenerator

#include <set>

class PerlCodeGenerator : public BaseCodeGenerator
{
public:
    PerlCodeGenerator(Node* form_node);

    // All language generators must implement this method.
    void GenerateClass(GenLang language = GEN_LANG_PERL,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL,
                       wxProgressDialog* progress = nullptr) override;

protected:
    // Helper methods to break down GenerateClass complexity
    [[nodiscard]] auto InitializeThreads(std::set<std::string>& img_include_set)
        -> std::tuple<std::thread, std::thread, std::thread>;
    auto WriteSourceHeader() -> void;
    auto WriteIDConstants() -> void;
    auto WriteSampleFrameApp(Code& code) -> void;
    auto GenerateConstructionCode(Code& code) -> void;
    auto GenerateEventHandlers(Code& code, std::thread& thrd_get_events) -> void;
    auto WriteHelperFunctions() -> void;
    auto WriteEmbeddedImages(Code& code) -> void;

    // This will collect all potential use statements, sort and separate the, and write them
    // to m_source.
    void WriteUsageStatements();
    void ParseNodesForUsage(Node* node);

    void GenerateImagesForm(wxProgressDialog* progress = nullptr);

    // This function simply generates unhandled event handlers in a multi-string comment.
    void GenUnhandledEvents(EventVector& events);

    // Helper methods for GenUnhandledEvents
    [[nodiscard]] auto CollectExistingEventHandlers(std::unordered_set<std::string>& code_lines)
        -> bool;
    static auto GenerateEventHandlerComment(bool found_user_handlers, Code& code) -> void;
    static auto GenerateEventHandlerBody(NodeEvent* event, Code& undefined_handlers) -> void;
    auto WriteEventHandlers(Code& code, Code& undefined_handlers) -> void;

    void CheckMimeBase64Requirement(Code& code);

    // Helper methods for CheckMimeBase64Requirement
    auto ProcessImageFromImagesForm(Code& code, bool& images_file_imported, bool& svg_import_libs,
                                    const EmbeddedImage* iter) -> void;
    auto ProcessExternalImage(const EmbeddedImage* iter, bool svg_import_libs) -> void;

    void InitializeUsageStatements();

    // Helper methods for ParseNodesForUsage
    auto ProcessNodeProperties(Node* node) -> void;
    auto ProcessNodeImports(Node* node) -> void;

private:
    bool m_base64_requirement_written { false };
    bool m_stringio_requirement_written { false };

    // This won't be needed until wxPerl supports SVG files.
    // bool M_zlib_requirement_written { false };

    std::set<std::string> m_art_ids;
    std::set<std::string> m_use_classes;
    std::set<std::string> m_use_constants;
    std::set<std::string> m_use_expands;  // for use Wx qw[:...];
    std::set<std::string> m_use_packages;
};
