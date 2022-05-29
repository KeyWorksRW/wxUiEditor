/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class WriteCode;
class Node;

// Generate a string containing the XRC of the starting node and all of it's children.
//
// Use project node to generate an XRC string of the entire project.
std::string GenerateXrcStr(Node* node_start, bool add_comments = false, bool is_preview = false);

class BaseXrcGenerator
{
public:
    BaseXrcGenerator();

    void SetSrcWriteCode(WriteCode* cw) { m_source = cw; }

private:
    WriteCode* m_source;
};
