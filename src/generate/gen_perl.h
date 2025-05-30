/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate wxPerl code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_base.h"  // BaseCodeGenerator

class PerlCodeGenerator : public BaseCodeGenerator
{
public:
    PerlCodeGenerator(Node* form_node);

    // All language generators must implement this method.
    void GenerateClass(PANEL_PAGE panel_type = NOT_PANEL) override;

protected:
    // This will collect all potential use statements, sort and separate the, and write them
    // to m_source.
    void WriteUsageStatements();

    void GenerateImagesForm();

    // This function simply generates unhandled event handlers in a multi-string comment.
    void GenUnhandledEvents(EventVector& events);
};
