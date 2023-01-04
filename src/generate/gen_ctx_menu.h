/////////////////////////////////////////////////////////////////////////////
// Purpose:   CtxMenuGenerator -- generates function and includes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

#include "node_classes.h"  // Forward defintions of Node classes

class CtxMenuGenerator : public BaseGenerator
{
public:
    bool AdditionalCode(Code&, GenEnum::GenCodeType /* cmd */) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

protected:
    void CollectEventHandlers(Node* node, std::vector<NodeEvent*>& events);

private:
    std::vector<NodeEvent*> m_CtxMenuEvents;
};
