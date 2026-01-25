/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code Generation Comparison
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/listbox.h>
#include <wx/radiobut.h>

#include "code_compare_base.h"  // CodeCompareBase -- Class for comparing generated code
#include "diff_viewer.h"        // DiffViewer -- Dialog for displaying file differences

class CodeCompare : public CodeCompareBase
{
public:
    CodeCompare() = default;  // If you use this constructor, you must call Create(parent)
    CodeCompare(wxWindow* parent) { std::ignore = Create(parent); }
    ~CodeCompare() = default;

    CodeCompare(const CodeCompare&) = delete;
    CodeCompare(CodeCompare&&) = delete;
    auto operator=(const CodeCompare&) -> CodeCompare& = delete;
    auto operator=(CodeCompare&&) -> CodeCompare& = delete;

    // Static method for non-UI code comparison (used by verify_codegen)
    // If form_node is provided, only compare that form; otherwise compare entire project
    [[nodiscard]] static auto CollectFileDiffsForLanguage(GenLang language,
                                                          Node* form_node = nullptr)
        -> std::vector<FileDiff>;

protected:
    // Event handlers

    void OnCPlusPlus(wxCommandEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnPerl(wxCommandEvent& event) override;
    void OnPython(wxCommandEvent& event) override;
    void OnRuby(wxCommandEvent& event) override;
    void OnDiff(wxCommandEvent& event) override;
    void OnXRC(wxCommandEvent& event) override;

    void OnRadioButton(GenLang language);

private:
    std::vector<FileDiff> m_file_diffs;  // Diffs collected by OnRadioButton
    GenLang m_current_language = GEN_LANG_CPLUSPLUS;
};
