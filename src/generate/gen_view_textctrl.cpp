/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl view class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_view_textctrl.h"

#include "code.h"             // Code -- Helper class for generating code
#include "project_handler.h"  // ProjectHandler class

inline constexpr const auto txt_TextCtrlViewBlock =
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

void %class%::OnDraw(wxDC* WXUNUSED(dc))
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

bool TextViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& /* set_hdr */)
{
    set_src.insert("#include <wx/docmdi.h");
    set_src.insert("#include <wx/docview.h");
    set_src.insert("#include <wx/textctrl.h");

    auto parent = node->getParent();
    for (auto& iter: parent->getChildNodePtrs())
    {
        if (iter.get() == node)
            continue;
        if (iter->as_string(prop_class_name) == node->as_string(prop_mdi_doc_name))
        {
            tt_string hdr_file = iter->as_string(prop_base_file);
            if (hdr_file.size())
            {
                hdr_file += Project.as_string(prop_header_ext);
                set_src.insert(tt_string().Format("#include %ks", hdr_file.c_str()));
            }
            else
            {
                set_src.insert("// Either the Document class cannot be found, or it doesn't specify a base filename.");
            }

            break;
        }
    }

    return true;
}
