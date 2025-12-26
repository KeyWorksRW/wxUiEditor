/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl view class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_view_textctrl.h"

#include "code.h"                // Code -- Helper class for generating code
#include "project_handler.h"     // ProjectHandler class
#include "ttwx_string_vector.h"  // StringVector -- ttwx::StringVector class
#include "utils.h"               // Miscellaneous utility functions

inline constexpr auto txt_TextCtrlViewBlock =
    R"===(wxIMPLEMENT_DYNAMIC_CLASS(%class%, wxView);

%class%::%class%() : wxView(), m_text(NULL) {}

bool %class%::OnCreate(wxDocument* doc, long flags)
{
    if (!wxView::OnCreate(doc, flags))
        return false;

    m_frame = wxGetApp().CreateChildFrame(this);
    wxASSERT(frame == GetFrame());
    m_text = new wxTextCtrl(m_frame, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            GetTextCtrl()->Copy();
        },
        wxID_COPY);
    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            GetTextCtrl()->Paste();
        },
        wxID_PASTE);
    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            GetTextCtrl()->SelectAll();
        },
        wxID_SELECTALL);

    m_frame->Show();

    return true;
}

bool %class%::OnClose(bool delete_window)
{
    if (!wxView::OnClose(delete_window))
        return false;

    Activate(false);

    if (delete_window)
    {
        GetFrame()->Destroy();
        SetFrame(nullptr);
    }
    return true;
}
)===";

auto TextViewGenerator::ConstructionCode(Code& code) -> bool
{
    if (code.is_cpp())
    {
        ttwx::StringVector lines;
        lines.ReadString(std::string_view(txt_TextCtrlViewBlock));
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

auto TextViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang language) -> bool
{
    if (language == GEN_LANG_CPLUSPLUS)
    {
        set_src.insert("#include <wx/docmdi.h>");
        set_hdr.insert("#include <wx/docview.h>");
        set_hdr.insert("#include <wx/textctrl.h>");

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

    return false;
}

inline constexpr const auto txt_TextCtrlViewHdrBlock =
    R"===(
    %class%() : m_text(nullptr) {}

    bool OnCreate(wxDocument* doc, long flags) override;
    bool OnClose(bool deleteWindow = true) override;

    // nothing to do here, wxTextCtrl draws itself
    void OnDraw(wxDC* /* dc unused */) override {};

    wxTextCtrl* GetTextCtrl() const { return m_text; }

private:
    wxDECLARE_DYNAMIC_CLASS(%class%);
)===";

auto TextViewGenerator::HeaderCode(Code& code) -> bool
{
    ttwx::StringVector lines;
    lines.ReadString(std::string_view(txt_TextCtrlViewHdrBlock));
    auto class_name = code.node()->as_view(prop_class_name);
    for (const auto& wxline: lines)
    {
        std::string line = wxline.ToStdString();
        utils::replace_in_line(line, "%class%", class_name, true);
        code.Str(line).Eol();
    }

    return true;
}

auto TextViewGenerator::BaseClassNameCode(Code& code) -> bool
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

auto TextViewGenerator::CollectMemberVariables(Node* /* node unused */,
                                               std::set<std::string>& code_lines) -> void
{
    code_lines.insert("wxTextCtrl* m_text;  // NOLINT (protected member variable)");
}
