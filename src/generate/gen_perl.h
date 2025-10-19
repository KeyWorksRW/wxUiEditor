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
                       PANEL_PAGE panel_type = NOT_PANEL) override;

protected:
    // This will collect all potential use statements, sort and separate the, and write them
    // to m_source.
    void WriteUsageStatements();
    void ParseNodesForUsage(Node* node);

    void GenerateImagesForm();

    // This function simply generates unhandled event handlers in a multi-string comment.
    void GenUnhandledEvents(EventVector& events);

    void CheckMimeBase64Requirement(Code& code);

    void InitializeUsageStatements();

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
