/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSplitterWindow view class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_view_splitter.h"

#include "code.h"             // Code -- Helper class for generating code
#include "project_handler.h"  // ProjectHandler class

inline constexpr auto txt_SplitterViewBlock =
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

auto SplitterViewGenerator::ConstructionCode(Code& code) -> bool
{
    if (code.is_cpp())
    {
        tt_string_vector lines;
        lines.ReadString(txt_SplitterViewBlock);
        tt_string class_name = code.node()->as_string(prop_class_name);
        for (auto& line: lines)
        {
            line.Replace("%class%", class_name, true);
            code.Str(line).Eol();
        }
    }

    return true;
}

auto SplitterViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                        std::set<std::string>& [[maybe_unused]] set_hdr,
                                        GenLang [[maybe_unused]] language) -> bool
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
            tt_string hdr_file = iter->as_string(prop_base_file);
            if (hdr_file.size())
            {
                hdr_file += Project.as_string(prop_header_ext);
                set_src.insert(tt_string("#include ") << '"' << hdr_file << '"');
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

inline constexpr const auto txt_SplitterViewHdrBlock =
    R"===(
    %class%() : wxView(), m_splitter(nullptr) {}

    bool OnCreate(wxDocument* doc, long flags) override;
    void OnDraw(wxDC* dc) override;
    bool OnClose(bool deleteWindow = true) override;

    wxTextCtrl* GetText() const { return m_text; }

protected:
    void OnCopy(wxCommandEvent& /* event unused */) { m_text->Copy(); }
    void OnPaste(wxCommandEvent& /* event unused */) { m_text->Paste(); }
    void OnSelectAll(wxCommandEvent& /* event unused */) { m_text->SelectAll(); }

private:
    wxDECLARE_DYNAMIC_CLASS(%class%);
};
)===";

auto SplitterViewGenerator::HeaderCode(Code& code) -> bool
{
    tt_string_vector lines;
    lines.ReadString(txt_SplitterViewHdrBlock);
    tt_string class_name = code.node()->as_string(prop_class_name);
    for (auto& line: lines)
    {
        line.Replace("%class%", class_name, true);
        code.Str(line).Eol();
    }

    return true;
}

auto SplitterViewGenerator::BaseClassNameCode(Code& code) -> bool
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

auto SplitterViewGenerator::CollectMemberVariables(Node* /* node unused */,
                                                   std::set<std::string>& code_lines) -> void
{
    code_lines.insert("wxSplitterWindow* m_splitter;");
}
