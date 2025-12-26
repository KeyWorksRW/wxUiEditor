/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStyledTextCtrl view class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_view_scintilla.h"

#include "code.h"                // Code -- Helper class for generating code
#include "project_handler.h"     // ProjectHandler class
#include "ttwx_string_vector.h"  // ttwx::StringVector class
#include "utils.h"               // Miscellaneous utility functions

inline constexpr auto txt_ScintillaViewBlock =
    R"===(wxIMPLEMENT_DYNAMIC_CLASS(%class%, wxView);

%class%::%class%() : wxView(), m_text(NULL) {}

bool %class%::OnCreate(wxDocument* doc, long flags)
{
    if (!wxView::OnCreate(doc, flags))
        return false;

    Bind(
        wxEVT_MENU, [this](wxCommandEvent&) { m_text->Copy(); }, wxID_COPY);
    Bind(
        wxEVT_MENU, [this](wxCommandEvent&) { m_text->Paste(); }, wxID_PASTE);
    Bind(
        wxEVT_MENU, [this](wxCommandEvent&) { m_text->SelectAll(); }, wxID_SELECTALL);

    m_frame = wxGetApp().CreateChildFrame(this);
    m_text = new wxTextCtrl(m_frame, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    m_frame->SetTitle(wxFileName(doc->GetFilename()).Filename());
    m_frame->Show();

    return true;
}

void %class%::OnDraw(wxDC* /* property unused */dc))
{
    // nothing to do here, wxTextCtrl draws itself
}

bool TextE%class%ditView::OnClose(bool delete_window)
{
    if (!wxView::OnClose(delete_window))
        return false;

    Activate(false);

    if (delete_window)
    {
        m_frame->Destroy();
        SetFrame(nullptr);
        m_frame = nullptr;
    }
    return true;
}
)===";

auto ScintillaViewGenerator::ConstructionCode(Code& code) -> bool
{
    if (code.is_cpp())
    {
        ttwx::StringVector lines;
        lines.ReadString(std::string_view(txt_ScintillaViewBlock));
        auto class_name = code.node()->as_view(prop_class_name);
        for (const auto& wxline: lines)
        {
            std::string line = wxline.ToStdString();
            utils::replace_in_line(line, "%class%", class_name, true);
            code.Str(line).Eol();
        }
    }

    return true;
}

auto ScintillaViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                         [[maybe_unused]] std::set<std::string>& set_hdr,
                                         [[maybe_unused]] GenLang language) -> bool
{
    set_src.insert("#include <wx/docmdi.h>");
    set_src.insert("#include <wx/docview.h>");
    set_src.insert("#include <wx/textctrl.h>");

    auto* parent = node->get_Parent();
    for (const auto& iter: parent->get_ChildNodePtrs())
    {
        if (iter.get() == node)
        {
            continue;
        }
        if (iter->as_string(prop_class_name) == node->as_string(prop_mdi_doc_name))
        {
            wxString hdr_file = iter->as_string(prop_base_file).make_wxString();
            if (!hdr_file.empty())
            {
                hdr_file += Project.as_string(prop_header_ext).make_wxString();
                set_src.insert(std::string("#include \"") + hdr_file.ToStdString() + "\"");
            }
            else
            {
                set_src.insert("// Either the Document class cannot be found, or it doesn't "
                               "specify a base filename.");
            }

            break;
        }
    }

    return true;
}

inline constexpr const auto txt_ScintillaViewHdrBlock =
    R"===(
    %class%() : wxView(), m_text(nullptr) {}

    bool OnCreate(wxDocument* doc, long flags) override;
    void OnDraw(wxDC* dc) override;
    bool OnClose(bool deleteWindow = true) override;

    wxStyledTextCtrl* GetText() const { return m_text; }

protected:
    void OnCopy(wxCommandEvent& /* event unused */) { m_text->Copy(); }
    void OnPaste(wxCommandEvent& /* event unused */) { m_text->Paste(); }
    void OnSelectAll(wxCommandEvent& /* event unused */) { m_text->SelectAll(); }

private:
    wxDECLARE_DYNAMIC_CLASS(%class%);
};
)===";

auto ScintillaViewGenerator::HeaderCode(Code& code) -> bool
{
    ttwx::StringVector lines;
    lines.ReadString(std::string_view(txt_ScintillaViewHdrBlock));
    auto class_name = code.node()->as_view(prop_class_name);
    for (const auto& wxline: lines)
    {
        std::string line = wxline.ToStdString();
        utils::replace_in_line(line, "%class%", class_name, true);
        code.Str(line).Eol();
    }

    return true;
}

auto ScintillaViewGenerator::BaseClassNameCode(Code& code) -> bool
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += "wxView";
    }

    return true;
}

auto ScintillaViewGenerator::CollectMemberVariables(Node* /* node unused */,
                                                    std::set<std::string>& code_lines) -> void
{
    code_lines.insert("wxStyledTextCtrl* m_text;");
}
