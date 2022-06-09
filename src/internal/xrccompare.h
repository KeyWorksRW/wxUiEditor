/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++/XRC UI Comparison dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "xrccompare_base.h"

class Node;

class XrcCompare : public XrcCompareBase
{
public:
    XrcCompare();  // If you use this constructor, you must call Create(parent)
    ~XrcCompare();

    bool DoCreate(wxWindow *parent, Node* node_form);

protected:
    bool InitXrc(Node* form_node);

private:
    wxString m_res_name;
};
