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
    void GenerateClass(PANEL_PAGE panel_type = NOT_PANEL) override;

    // Returns result::fail, result::exists, result::created, or result::ignored
    int GenerateDerivedClass(Node* project, Node* form_node,
                             PANEL_PAGE panel_type = NOT_PANEL) override;

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
    void GenCppValVarsBase(const NodeDeclaration* info, Node* node,
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
};
