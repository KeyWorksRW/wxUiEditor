/////////////////////////////////////////////////////////////////////////////
// Purpose:   Compare code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dbg_code_diff_base.h"

class DbgCodeDiff : public DbgCodeDiffBase
{
public:
    DbgCodeDiff(wxWindow* parent = nullptr);
    ~DbgCodeDiff();

protected:
    void OnWinMerge(wxCommandEvent& event) override;
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;

private:
    std::vector<ttlib::cstr> m_class_list;
};
