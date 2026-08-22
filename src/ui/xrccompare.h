/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++/XRC UI Comparison dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-22-2026]

#pragma once

#include "../wxui/xrccompare_base.h"

// Requires xrccompare_base.h which forward-declares Node, but declare it explicitly so a
// future refactor of the generated base header cannot silently break this one.
class Node;

class XrcCompare : public XrcCompareBase
{
public:
    // If you use this constructor, you must call DoCreate(parent, form_node, ...) -- the dialog
    // is not operational until DoCreate has built all of its children.
    //
    // There is deliberately no parent-taking constructor: XrcCompareBase already provides
    // Create(), and DoCreate() alone performs the full initialization while avoiding a second
    // (failing) Create() on an already-created dialog.
    XrcCompare();

    ~XrcCompare() override;

    // Call this instead of Create() -- it will directly call Create() and do a bunch of
    // other initialization related to turning it into a comparison dialog.
    //
    // true for compare_import will compare against a currently imported XRC file.
    //
    // Returns false if the dialog could not be built or the XRC could not be loaded -- the
    // caller must handle that failure.
    [[nodiscard]] bool DoCreate(wxWindow* parent, Node* form_node, bool compare_import = false);

protected:
    // Requires Create()/DoCreate() to have succeeded; form_node must be non-null.
    bool InitXrc(Node* form_node);
    bool InitImport(Node* form_node);

private:
    XrcCompare(const XrcCompare&) = delete;
    XrcCompare& operator=(const XrcCompare&) = delete;

    wxString m_res_name;
    wxString m_import_res_name;
    bool m_created { false };
    bool m_compare_import { false };
};
