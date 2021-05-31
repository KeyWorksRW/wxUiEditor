/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node.h"  // Node class

// Same as the Windows RECT structure -- this version declared to provide a cross-platform
// version
struct RC_RECT
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

class rcCtrl
{
public:
    rcCtrl();

    auto GetNode() { return m_node; }

    void ParseEditCtrl(ttlib::cview line);
    void ParseGroupBox(ttlib::cview line);
    void ParsePushButton(ttlib::cview line);
    void ParseStaticCtrl(ttlib::cview line);

    auto GetLeft() const { return m_rc.left; }
    auto GetTop() const { return m_rc.top; }

protected:
    void AppendStyle(GenEnum::PropName prop_name, ttlib::cview style);

    void ParseCommonStyles(ttlib::cview line);
    void GetDimensions(ttlib::cview line);

    ttlib::cview StepOverQuote(ttlib::cview line, ttlib::cstr& str);
    ttlib::cview StepOverComma(ttlib::cview line, ttlib::cstr& str);

private:
    NodeSharedPtr m_node;

    RC_RECT m_rc { 0, 0, 0, 0 };

    int m_minHeight;
    int m_minWidth;
};
