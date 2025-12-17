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

#include "../code_compare_base.h"  // CodeCompareBase -- Class for comparing generated code
#include "diff_viewer.h"

class CodeCompare : public CodeCompareBase
{
public:
    CodeCompare() = default;  // If you use this constructor, you must call Create(parent)
    CodeCompare(wxWindow* parent) { Create(parent); }  // NOLINT (cppcheck-suppress)
    ~CodeCompare();

    CodeCompare(const CodeCompare&) = delete;
    CodeCompare(CodeCompare&&) = delete;
    auto operator=(const CodeCompare&) -> CodeCompare& = delete;
    auto operator=(CodeCompare&&) -> CodeCompare& = delete;

    // Static method for non-UI code comparison (used by verify_codegen)
    [[nodiscard]] static auto CollectFileDiffsForLanguage(GenLang language)
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
    auto CollectFileDiffs() -> std::vector<FileDiff>;
    auto GetGeneratedCodeRange(const ttwx::ViewVector& content) -> std::pair<size_t, size_t>;

    std::vector<std::string> m_class_list;
    GenLang m_current_language = GEN_LANG_CPLUSPLUS;
};
