/////////////////////////////////////////////////////////////////////////////
// Purpose:   CtxMenuGenerator -- generates function and includes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class Node;

class CtxMenuGenerator : public BaseGenerator
{
public:
    bool AfterChildrenCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

protected:
    void CollectCtxMenuEventHandlers(Node* node, std::vector<NodeEvent*>& events);

private:
    std::vector<NodeEvent*> m_CtxMenuEvents;
};
