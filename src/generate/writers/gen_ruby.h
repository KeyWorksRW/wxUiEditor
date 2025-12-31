/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate wxRuby code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <tuple>

#include "gen_base.h"  // BaseCodeGenerator

class RubyCodeGenerator : public BaseCodeGenerator
{
public:
    RubyCodeGenerator(Node* form_node);

    // All language generators must implement this method.
    void GenerateClass(GenLang language = GEN_LANG_RUBY,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL,
                       wxProgressDialog* progress = nullptr) override;

protected:
    // This function simply generates unhandled event handlers in a multi-string comment.
    void GenUnhandledEvents(EventVector& events);

    void GenerateImagesForm(wxProgressDialog* progress = nullptr);

    void WriteImageRequireStatements(Code& code);

private:
    auto InitializeThreads(std::set<std::string>& img_include_set)
        -> std::tuple<std::thread, std::thread, std::thread>;
    auto WriteSourceHeader() -> void;
    auto WriteImports(std::set<std::string>& imports) -> void;
    auto WriteRelativeRequires(const std::vector<Node*>& forms) -> void;
    auto WriteIDConstants() -> void;
    auto WriteInheritedClass() -> void;
    auto GenerateConstructionCode(Code& code) -> void;
    auto GenerateEventHandlers([[maybe_unused]] Code& code, std::thread& thrd_get_events) -> void;
    auto WriteHelperFunctions() -> void;
    auto WriteEmbeddedImages(Code& code) -> void;
    auto WriteRuboCopFooter() -> void;

    // Helper methods for WriteImageRequireStatements
    struct ImageFromImagesParameters
    {
        const EmbeddedImage* iter;
        bool* images_file_imported;
        bool* svg_import_libs;
        Code* code;
    };

    auto WriteSVGRequirements() -> void;
    auto WriteImagesFileImport(Code& code, Node* form) -> void;
    auto ProcessImageFromImagesForm(const ImageFromImagesParameters& params) -> void;
    auto ProcessExternalImage(const EmbeddedImage* iter, bool svg_import_libs) -> void;

    // Helper methods for GenUnhandledEvents
    auto CollectExistingEventHandlers(std::unordered_set<std::string>& code_lines) -> bool;
    static auto GenerateEventHandlerComment(bool found_user_handlers, Code& code) -> void;
    static auto GenerateEventHandlerBody(NodeEvent* event, Code& undefined_handlers) -> void;
    auto WriteEventHandlers(Code& code, Code& undefined_handlers) -> void;

    bool m_base64_requirement_written { false };
    bool m_stringio_requirement_written { false };
    bool m_zlib_requirement_written { false };
};
