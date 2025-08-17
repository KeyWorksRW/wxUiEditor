/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu bar classes for an MDI frame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class MdiFrameMenuBar : public BaseGenerator
{
public:
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

private:
    // Node* m_node_menubar;
};

class MdiDocumentMenuBar : public BaseGenerator
{
public:
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

private:
    // Node* m_node_menubar;
};
