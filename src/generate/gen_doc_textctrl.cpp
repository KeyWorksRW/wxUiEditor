/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl document class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_doc_textctrl.h"

#include "code.h"  // Code -- Helper class for generating code

inline constexpr const auto txt_TextCtrlViewBlock =
    R"===(wxIMPLEMENT_DYNAMIC_CLASS(%class%, wxDocument);

bool %class%::OnCreate(const wxString& path, long flags)
{
    if (!wxDocument::OnCreate(path, flags))
        return false;

    GetTextCtrl()->Bind(wxEVT_TEXT, &%class%::OnTextChange, this);

    return true;
}

// Since text windows have their own method for saving to/loading from files, we override
// DoSaveDocument/DoOpenDocument instead of Save/LoadObject

bool %class%::DoOpenDocument(const wxString& filename)
{
    if (!GetTextCtrl()->LoadFile(filename))
        return false;

    Modify(false);

    return true;
}

bool %class%::DoSaveDocument(const wxString& filename)
{
    auto result = GetTextCtrl()->SaveFile(filename);
    if (auto view = GetFirstView(); view)
    {
        wxStaticCast(view, TextEditView)->GetFrame()->SetTitle(wxFileName(filename).Filename());
    }
    return result;
}

bool %class%::IsModified() const
{
    auto text_ctrl = GetTextCtrl();
    return wxDocument::IsModified() || (text_ctrl && text_ctrl->IsModified());
}

void %class%::Modify(bool modified)
{
    wxDocument::Modify(modified);

    if (auto text_ctrl = GetTextCtrl(); text_ctrl && !modified)
    {
        // This doesn't save the text, it just resets the modified flag.
        text_ctrl->DiscardEdits();
    }
}

wxTextCtrl* %class%::GetTextCtrl() const
{
    auto view = GetFirstView();
    return view ? wxStaticCast(view, TextEditView)->GetText() : nullptr;
}

void %class%::OnTextChange(wxCommandEvent& event)
{
    Modify(true);

    event.Skip();
}
)===";

bool TextDocumentGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        tt_string_vector lines;
        lines.ReadString(txt_TextCtrlViewBlock);
        tt_string class_name = code.node()->GetParent()->value(prop_class_name);
        for (auto& line: lines)
        {
            line.Replace("%class%", class_name, true);
            code.Str(line).Eol();
        }
    }

    return true;
}

bool TextDocumentGenerator::GetIncludes(Node* /* node */, std::set<std::string>& set_src,
                                        std::set<std::string>& /* set_hdr */)
{
    set_src.insert("#include <wx/docmdi.h");
    set_src.insert("#include <wx/docview.h");
    set_src.insert("#include <wx/textctrl.h");

    return true;
}
