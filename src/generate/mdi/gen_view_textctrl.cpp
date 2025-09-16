/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl view class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "gen_view_textctrl.h"

#include "code.h"             // Code -- Helper class for generating code
#include "project_handler.h"  // ProjectHandler class

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

bool TextViewGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        tt_string_vector lines;
        lines.ReadString(txt_TextCtrlViewBlock);
        tt_string class_name = code.node()->as_string(prop_class_name);
        for (auto& line: lines)
        {
            line.Replace("%class%", class_name, true);
            code.Str(line).Eol();
        }
    }

    return true;
}

bool TextViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang language)
{
    if (language == GEN_LANG_CPLUSPLUS)
    {
        set_src.insert("#include <wx/docmdi.h");
        set_hdr.insert("#include <wx/docview.h");
        set_hdr.insert("#include <wx/textctrl.h");

        auto parent = node->get_Parent();
        for (auto& iter: parent->get_ChildNodePtrs())
        {
            if (iter.get() == node)
                continue;
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

    return false;
}

inline constexpr const auto txt_TextCtrlViewHdrBlock =
    R"===(
#pragma once

// This view uses a standard wxTextCtrl to show its contents
class %class% : public wxView
{
public:
    %class%() : wxView(), m_text(nullptr) {}

    virtual bool OnCreate(wxDocument* doc, long flags) override;
    virtual bool OnClose(bool deleteWindow = true) override;

    // nothing to do here, wxTextCtrl draws itself
    virtual void OnDraw(wxDC* /* dc unused */) override {};

    wxTextCtrl* GetTextCtrl() const { return m_text; }

private:
    wxTextCtrl* m_text;

    wxDECLARE_DYNAMIC_CLASS(%class%);
};
)===";

bool TextViewGenerator::HeaderCode(Code& code)
{
    tt_string_vector lines;
    lines.ReadString(txt_TextCtrlViewHdrBlock);
    tt_string class_name = code.node()->as_string(prop_class_name);
    for (auto& line: lines)
    {
        line.Replace("%class%", class_name, true);
        code.Str(line).Eol();
    }

    return true;
}
