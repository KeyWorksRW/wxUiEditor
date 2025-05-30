/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_base.h"  // BaseCodeGenerator

class CppCodeGenerator : public BaseCodeGenerator
{
public:
    CppCodeGenerator(Node* form_node);

    // All language generators must implement this method.
    void GenerateClass(PANEL_PAGE panel_type = NOT_PANEL) override;

protected:
    void GenerateCppClassHeader();
    void GenerateCppClassConstructor();

    // Write code to m_source that will load any image handlers needed by the form's class
    void GenerateCppHandlers();

    // Recursive function for generating all get/set validator functions in the header file
    void GenCppValidatorFunctions(Node* node);

    void GenHdrNameSpace(tt_string& namespace_prop, tt_string_vector& names, size_t& indent);
    // Generate any headers and functions needed for images in m_source
    void GenCppImageFunctions();
    // Writes the #include files to m_header
    void GenInitHeaderFile(std::set<std::string>& hdr_includes);

    // Generates an enum of all use-defined ids
    void GenCppEnumIds(Node* class_node);

    // Called from GenerateCppClassConstructor if node is a gen_Data
    void GenerateDataClassConstructor(PANEL_PAGE panel_type);
};
