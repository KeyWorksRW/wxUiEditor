/////////////////////////////////////////////////////////////////////////////
// Purpose:   Scintilla document class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_doc_scintilla.h"

#include "code.h"  // Code -- Helper class for generating code

inline constexpr auto txt_ScintillaDocViewBlock =
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

bool %class%::isModified() const
{
    auto text_ctrl = GetTextCtrl();
    return wxDocument::isModified() || (text_ctrl && text_ctrl->isModified());
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

auto ScintillaDocGenerator::ConstructionCode(Code& code) -> bool
{
    if (code.is_cpp())
    {
        tt_string_vector lines;
        lines.ReadString(txt_ScintillaDocViewBlock);
        tt_string class_name = code.node()->get_Parent()->as_string(prop_class_name);
        for (auto& line: lines)
        {
            line.Replace("%class%", class_name, true);
            code.Str(line).Eol();
        }
    }

    return true;
}

auto ScintillaDocGenerator::GetIncludes(Node* [[maybe_unused]] node, std::set<std::string>& set_src,
                                        std::set<std::string>& [[maybe_unused]] set_hdr,
                                        GenLang [[maybe_unused]] language) -> bool
{
    set_src.insert("#include <wx/docmdi.h>");
    set_src.insert("#include <wx/docview.h>");
    set_src.insert("#include <wx/textctrl.h>");

    return true;
}

auto ScintillaDocGenerator::BaseClassNameCode(Code& code) -> bool
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += "wxDocument";
    }

    return true;
}
