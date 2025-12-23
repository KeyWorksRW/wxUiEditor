/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate wxPython code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_base.h"  // BaseCodeGenerator

class PythonCodeGenerator : public BaseCodeGenerator
{
public:
    PythonCodeGenerator(Node* form_node);

    // All language generators must implement this method.
    void GenerateClass(GenLang language = GEN_LANG_PYTHON,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL) override;

protected:
    // Helper methods to break down GenerateClass complexity
    auto InitializeThreads(std::set<std::string>& img_include_set)
        -> std::tuple<std::thread, std::thread>;
    auto WriteSourceHeader() -> void;
    auto WriteImports(std::set<std::string>& imports) -> void;
    auto WriteImportList() -> void;
    auto WriteIDConstants() -> void;
    auto WriteInheritedClass() -> void;
    auto WriteInsertCode() -> void;
    auto GenerateConstructionCode(Code& code) -> void;
    auto GenerateEventHandlers(Code& code, std::thread& thrd_get_events) -> void;
    auto WriteWizardComment(Code& code) -> void;

    // Helper methods for GenUnhandledEvents
    auto CollectExistingEventHandlers(std::unordered_set<std::string>& code_lines) -> bool;
    static auto GenerateEventHandlerComment(bool found_user_handlers, Code& code) -> void;
    static auto GenerateEventHandlerBody(NodeEvent* event, Code& code) -> void;
    static auto CheckIfAllEventsImplemented(EventVector& events,
                                            const std::unordered_set<std::string>& code_lines,
                                            bool found_user_handlers) -> bool;
    auto GenerateUndefinedHandlers(EventVector& events, std::unordered_set<std::string>& code_lines,
                                   Code& undefined_handlers) -> void;
    auto WriteEventHandlers(Code& code, Code& undefined_handlers, bool found_user_handlers,
                            bool is_all_events_implemented) -> void;

    // This function simply generates unhandled event handlers in a multi-string comment.
    void GenUnhandledEvents(EventVector& events);

    void GenerateImagesForm();

    void WriteImageImportStatements(Code& code);

    // Helper methods for WriteImageImportStatements
    auto WriteImagesFormImport(Code& code, bool& images_file_imported, bool& svg_import_libs)
        -> void;
    auto WriteExternalImageImports(Code& code) -> bool;
    auto WriteEmbeddedImageImports(Code& code, bool blank_line_seen) -> void;
};
