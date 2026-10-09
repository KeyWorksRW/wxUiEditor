///////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for choosing and generating specific language file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/valgen.h>

#include "generate_dlg_base.h"

class GenerateDlg : public GenerateDlgBase
{
public:
    GenerateDlg() {}  // If you use this constructor, you must call Create(parent)
    GenerateDlg(wxWindow* parent);

    bool is_gen_base() const { return m_gen_base_code; }
    bool is_gen_inherited() const { return m_gen_inherited_code; }
    bool is_gen_fortran() const { return m_gen_fortran_code; }
    bool is_gen_go() const { return m_gen_go_code; }
    bool is_gen_julia() const { return m_gen_julia_code; }
    bool is_gen_luajit() const { return m_gen_luajit_code; }
    bool is_gen_python() const { return m_gen_python_code; }
    bool is_gen_ruby() const { return m_gen_ruby_code; }
    bool is_gen_typescript() const { return m_gen_typescript_code; }
    bool is_gen_xrc() const { return m_gen_xrc_code; }

protected:
    // Event handlers

    void OnInit(wxInitDialogEvent& event) override;

private:
    wxCheckBox* m_checkBaseCode { nullptr };
    wxCheckBox* m_checkDerived { nullptr };
    wxCheckBox* m_checkFortran { nullptr };
    wxCheckBox* m_checkGo { nullptr };
    wxCheckBox* m_checkJulia { nullptr };
    wxCheckBox* m_checkLuaJIT { nullptr };
    wxCheckBox* m_checkPython { nullptr };
    wxCheckBox* m_checkRuby { nullptr };
    wxCheckBox* m_checkTypeScript { nullptr };
    wxCheckBox* m_checkXRC { nullptr };

    bool m_gen_base_code { false };
    bool m_gen_inherited_code { false };
    bool m_gen_fortran_code { false };
    bool m_gen_go_code { false };
    bool m_gen_julia_code { false };
    bool m_gen_luajit_code { false };
    bool m_gen_python_code { false };
    bool m_gen_ruby_code { false };
    bool m_gen_typescript_code { false };
    bool m_gen_xrc_code { false };
};
