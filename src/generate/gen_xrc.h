/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class WriteCode;

class BaseXrcGenerator
{
public:
    BaseXrcGenerator();

    void SetSrcWriteCode(WriteCode* cw) { m_source = cw; }

private:
    WriteCode* m_source;
};
