/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code generation panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

#include <wx/panel.h>  // Base header for wxPanel

class CodeDisplay;
class CustomEvent;
class MainFrame;

class wxAuiNotebook;
class wxFindDialogEvent;
class wxStyledTextCtrl;

enum class PANEL_PAGE : std::uint8_t
{
    NOT_PANEL,
    SOURCE_PANEL,
    HDR_INFO_PANEL,
    DERIVED_SRC_PANEL,
    DERIVED_HDR_PANEL,
};

class BasePanel : public wxPanel
{
public:
    BasePanel(wxWindow* parent, MainFrame* frame, GenLang GenerateDerivedCode);
    ~BasePanel() override;

    BasePanel(const BasePanel&) = delete;
    BasePanel& operator=(const BasePanel&) = delete;
    BasePanel(BasePanel&&) = delete;
    BasePanel& operator=(BasePanel&&) = delete;

    void GenerateBaseClass();
    auto GetSelectedText() -> wxString;

    auto GetPanelPage() const -> PANEL_PAGE;

    void OnFind(wxFindDialogEvent& event);

    void OnNodeSelected(CustomEvent& event);

    void SetColor(int style, const wxColour& color);
    void SetCodeFont(const wxFont& font);

protected:
private:
    CodeDisplay* m_source_panel;
    CodeDisplay* m_hdr_info_panel;  // Header, inherit, info panel
    CodeDisplay* m_derived_src_panel { nullptr };
    CodeDisplay* m_derived_hdr_panel { nullptr };
    wxAuiNotebook* m_notebook;
    Node* m_cur_form { nullptr };

    GenLang m_panel_type;
};
