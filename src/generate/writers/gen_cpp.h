/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <thread>

#include "gen_base.h"  // BaseCodeGenerator

class CppCodeGenerator : public BaseCodeGenerator
{
public:
    CppCodeGenerator(Node* form_node);

    // All language generators must implement this method.
    void GenerateClass(GenLang language = GEN_LANG_CPLUSPLUS,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL) override;

    // Returns result::fail, result::exists, result::created, or result::ignored
    auto GenerateDerivedClass(Node* project, Node* form_node,
                              PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL) -> int override;

protected:
    void GenerateCppClassHeader(bool class_namespace = false);
    void GenerateCppClassConstructor();

    // Called from GenerateClass() to generate #include statements in both source and header
    // files
    void GenerateClassIncludes(Code& code, PANEL_PAGE panel_type, std::thread* thrd_get_events);

    // Write code to m_source that will load any image handlers needed by the form's class
    void GenerateCppHandlers();

    // Recursive function for generating all get/set validator functions in the header file
    void GenCppValidatorFunctions(Node* node);

    // Convert one or more project/folder/form namespaces and store them in the names vector, and
    // update the indent to tell us how much to un-indent the code inside the namespace after
    // GenerateCppClassHeader()
    void GenHdrNameSpace(tt_string& namespace_prop, tt_string_vector& names, size_t& indent);
    // Generate any headers and functions needed for images in m_source
    void GenCppImageFunctions();
    // Writes the #include files to m_header
    void GenInitHeaderFile(std::set<std::string>& hdr_includes);

    // Generates an enum of all use-defined ids
    void GenCppEnumIds(Node* class_node);

    // Called from GenerateCppClassConstructor if node is a gen_Data
    void GenerateDataClassConstructor(PANEL_PAGE panel_type);

    // This function simply generates unhandled event handlers in a multi-string comment.
    void GenUnhandledEvents(EventVector& events);

    void GenHdrEvents();

    // Handles both source and header code generation
    void GenerateDataForm();

    void CollectValidatorVariables(Node* node, std::set<std::string>& code_lines);

    void CollectIncludes(Node* form, std::set<std::string>& set_src,
                         std::set<std::string>& set_hdr);

    // Recursive function for generating all include files needed by any nodes in the form
    void GatherGeneratorIncludes(Node* node, std::set<std::string>& set_src,
                                 std::set<std::string>& set_hdr);

    // Generates all the code lines for validator_variables initialized in the header file
    void GenCppValVarsBase(const NodeDeclaration* declaration, Node* node,
                           std::set<std::string>& code_lines);

    void CollectMemberVariables(Node* node, Permission perm, std::set<std::string>& code_lines);

    // Generate extern statements after the header definition for embedded images not defined
    // in the gen_Images node.
    void WriteImagePostHeader();  // declared in image_gen.cpp

    // Generate extern references to images used in the current form that are defined in the
    // gen_Images node.
    //
    // This will call code.clear() before writing any code.
    void WriteImagePreConstruction(Code& code);  // declared in image_gen.cpp

    void WritePropHdrCode(Node* node, GenEnum::PropName prop);

private:
    // ============================================================================
    // IMPLEMENTATION DETAILS BELOW - Not part of the public interface
    // ============================================================================
    // The following private helper methods are implementation details of the
    // CppCodeGenerator class. They are declared here only to satisfy the compiler
    // requirements for class member function definitions. External code should
    // not depend on or use these methods.
    // ============================================================================

    // Helper methods for WritePropHdrCode
    static auto IsAccessSpecifier(const tt_string& code) -> bool;
    static auto ShouldIndentAfter(const tt_string& code) -> bool;

    // Helper methods for GatherGeneratorIncludes
    static void ProcessFontProperty(const NodeProperty& prop, bool isAddToSrc,
                                    std::set<std::string>& set_src, std::set<std::string>& set_hdr);
    static void ProcessColourProperty(bool isAddToSrc, std::set<std::string>& set_src,
                                      std::set<std::string>& set_hdr);
    void ProcessImageProperty(const NodeProperty& prop, bool isAddToSrc,
                              std::set<std::string>& set_src, std::set<std::string>& set_hdr);

    // Helper methods for GenUnhandledEvents
    [[nodiscard]] auto CollectUserEventHandlers(std::unordered_set<std::string>& code_lines)
        -> bool;
    [[nodiscard]] auto
        CheckIfAllEventsImplemented(const EventVector& events,
                                    const std::unordered_set<std::string>& code_lines) -> bool;
    static void GenerateEventFunctionBody(Code& code, NodeEvent* event);

    // Helper methods for GenerateCppClassConstructor
    void GenerateConstructionPreamble(Code& code, BaseGenerator* generator);
    void GenerateChildrenAndEvents(Code& code, BaseGenerator* generator);
    void GenerateConstructorClosing(Code& code, BaseGenerator* generator);

    // Helper methods for GenerateClassIncludes
    void CollectBaseIncludes(std::set<std::string>& src_includes,
                             std::set<std::string>& hdr_includes, std::thread* thrd_get_events);
    static void ProcessOrderDependentIncludes(std::set<std::string>& src_includes,
                                              std::vector<std::string>& ordered_includes);
    void WriteSourceIncludes(const std::set<std::string>& src_includes,
                             const std::vector<std::string>& ordered_includes,
                             const std::string& file);

    // Helper methods for GenerateClass - reduce function complexity
    void InitializeGenerationState();
    void StartThreadedCollections(std::set<std::string>& img_include_set);
    void ProcessEmbeddedImagesAndIncludes(const std::set<std::string>& img_include_set);
    void DetermineNamespace(tt_string& namespace_prop);
    void FinalizeNamespace(const tt_string_vector& names, size_t indent, Code& code);

    // Helper methods for GenHdrEvents
    [[nodiscard]] static auto ShouldSkipEvent(const tt_string& event_code) -> bool;
    [[nodiscard]] static auto HasContextMenuHandler(NodeEvent* event) -> bool;
    void ProcessSingleEvent(NodeEvent* event, std::set<tt_string>& code_lines);
    void BuildEventHandlerDeclaration(tt_string& code, const tt_string& event_code,
                                      const tt_string& event_class) const;
    void WriteEventHandlerHeader() const;
    void ProcessConditionalEvents(Code& code);

    // Helper methods for GenInitHeaderFile
    static void ProcessOrderDependentHeaderIncludes(std::set<std::string>& hdr_includes,
                                                    std::vector<std::string>& ordered_includes);
    void WriteWxWidgetsHeaders(const std::set<std::string>& hdr_includes);
    static auto ExtractNamespaces(std::set<std::string>& hdr_includes) -> std::vector<std::string>;
    void WriteNonWxHeaders(const std::set<std::string>& hdr_includes);
    void WritePreambleAndCustomIncludes();
    void WriteNamespaceDeclarations(const std::vector<std::string>& namespaces);

    // Helper methods for GenerateCppClassHeader
    void WriteClassDeclaration(Code& code, BaseGenerator* generator);
    void WritePublicSection(Code& code, BaseGenerator* generator);
    void WriteProtectedAndPrivateSections(Code& code, BaseGenerator* generator);

    // Helper methods for WritePublicSection
    void WritePublicMemberVariables(Code& code);
    void WriteConstValues(Code& code);
    void WriteGeneratorHeaderCode(Code& code, BaseGenerator* generator);
    void WritePublicClassMethods();

    // Helper methods for WriteConstValues
    static void WriteFormIdConst(Code& code, Node* node);
    static void WriteFormStyleConst(Code& code, Node* node);
    static void WriteFormPosConst(Code& code, Node* node);
    static void WriteFormSizeConst(Code& code, Node* node);
    static void WriteFormTitleConst(Code& code, Node* node);

    // Helper methods for WriteProtectedAndPrivateSections
    void WriteProtectedClassMethods();
    void WriteValidatorVariables(Code& code, std::set<std::string>& code_lines);
    void WriteProtectedMemberVariables(Code& code, BaseGenerator* generator,
                                       std::set<std::string>& code_lines);

    // Helper methods for CollectMemberVariables
    static void AdjustGenericClassName(Node* node, tt_string& code);
    void InsertMemberVariable(Node* node, const tt_string& code, Permission perm);
    void InsertPlatformSpecificVariable(const tt_string& platform, const tt_string& code,
                                        Permission perm);
    static void InsertRegularMemberVariable(const tt_string& code,
                                            std::set<std::string>& code_lines);
    void ProcessProtectedMemberVariables(Node* node, std::set<std::string>& code_lines);
    void ProcessCheckboxRadioVariables(Node* node, std::set<std::string>& code_lines);
    void ProcessClassAccessProperty(Node* node, Permission perm, std::set<std::string>& code_lines);

    // Helper methods for GenCppValVarsBase
    static void AppendBoolInitializer(tt_string& code, Node* node);
    static void AppendNumericInitializer(tt_string& code, Node* node);
    static void AppendStringInitializer(tt_string& code, Node* node);
    void InsertValidatorVariable(Node* node, const tt_string& code,
                                 std::set<std::string>& code_lines);

    // Helper methods for GenerateDerivedClass
    static void GetFileExtensions(Node* project, tt_string& source_ext, tt_string& header_ext);
    [[nodiscard]] auto DetermineDerivedFilePath(Node* form, PANEL_PAGE panel_type,
                                                const tt_string& source_ext) -> tt_string;
    void DetermineBaseFilePath(Node* form, tt_string& baseFile);
    static void ProcessNamespace(Node* form, tt_string& namespace_using_name);
    void GenerateDerivedClassName(tt_string& derived_name);
    void GenerateDerivedHeader(const tt_string& derived_name, const tt_string& baseFile,
                               const tt_string& namespace_using_name, const tt_string& header_ext,
                               PANEL_PAGE panel_type);
    void GenerateDerivedSource(Node* project, const tt_string& derived_name,
                               const tt_string& baseFile, const tt_string& derived_file,
                               const tt_string& namespace_using_name, const tt_string& header_ext,
                               const tt_string& source_ext, PANEL_PAGE panel_type);
    void GenerateDerivedEventHandlers(const EventVector& events, const tt_string& derived_name,
                                      PANEL_PAGE panel_type);
    static auto IsCloseTypeButton(NodeEvent* event) -> bool;
    static auto ShouldSkipContextMenuEvent(NodeEvent* event) -> bool;
    void WriteEventHandlerDeclaration(const tt_string& event_code, const tt_string& event_class);
    void WriteEventHandlerImplementation(NodeEvent* event, const tt_string& derived_name,
                                         const tt_string& event_code, bool close_type_button);

    // Thread member variables for GenerateClass
    std::thread m_thrd_get_events;
    std::thread m_thrd_collect_img_headers;
    std::thread m_thrd_need_img_func;
};

class GenData
{
public:
    GenData(GenResults& results, std::vector<std::string>* pClassList) :
        m_pClassList(pClassList), m_results(&results)
    {
    }

    void AddUpdateFilename(tt_string& path) const
    {
        m_results->GetUpdatedFiles().emplace_back(path);
    };

    void AddResultMsg(std::string_view msg) const { m_results->GetMsgs().emplace_back(msg); };

    void UpdateFileCount() const { m_results->IncrementFileCount(); };

    void AddClassName(std::string_view class_name) const
    {
        if (m_pClassList)
        {
            m_pClassList->emplace_back(class_name);
        }
    };

    auto get_source_ext() const -> std::string_view { return m_source_ext; }
    void set_source_ext(std::string_view ext) { m_source_ext = ext; }

    auto get_header_ext() const -> std::string_view { return m_header_ext; }
    void set_header_ext(std::string_view ext) { m_header_ext = ext; }

    auto get_pClassList() const -> std::vector<std::string>* { return m_pClassList; }

private:
    std::string m_source_ext;
    std::string m_header_ext;
    std::vector<std::string>* m_pClassList { nullptr };
    GenResults* m_results { nullptr };
};

void GenCppForm(GenData& gen_data, Node* form);
