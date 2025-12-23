/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl document class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_doc_textctrl.h"

#include "code.h"  // Code -- Helper class for generating code

inline constexpr auto txt_TextCtrlDocBlock =
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
    return GetTextCtrl()->SaveFile(filename);
}

bool %class%::isModified() const
{
    auto* text_ctrl = GetTextCtrl();
    return wxDocument::isModified() || (text_ctrl && text_ctrl->isModified());
}

void %class%::Modify(bool modified)
{
    wxDocument::Modify(modified);

    if (auto* text_ctrl = GetTextCtrl(); text_ctrl && !modified)
    {
        // This doesn't save the text, it just resets the modified flag.
        text_ctrl->DiscardEdits();
    }
}

wxTextCtrl* %class%::GetTextCtrl() const
{
    auto* view = GetFirstView();
    return view ? wxStaticCast(view, TextEditView)->GetText() : nullptr;
}

void %class%::OnTextChange(wxCommandEvent& event)
{
    Modify(true);

    event.Skip();
}
)===";

auto TextDocGenerator::ConstructionCode(Code& code) -> bool
{
    if (code.is_cpp())
    {
        tt_string_vector lines;
        lines.ReadString(txt_TextCtrlDocBlock);
        tt_string class_name = code.node()->as_string(prop_class_name);
        for (auto& line: lines)
        {
            line.Replace("%class%", class_name, true);
            code.Str(line).Eol();
        }
        return true;
    }

    return false;
}

auto TextDocGenerator::GetIncludes(Node* [[maybe_unused]] node, std::set<std::string>& set_src,
                                   std::set<std::string>& [[maybe_unused]] set_hdr,
                                   GenLang language) -> bool
{
    if (language == GEN_LANG_CPLUSPLUS)
    {
        set_src.insert("#include <iostream");
        return true;
    }

    return false;
}

inline constexpr const auto txt_TextCtrlDocHdrBlock =
    R"===(
#pragma once

class %class% : public wxDocument
{
public:
    %class%() : wxDocument() { }
    %class%(const TextEditDocument&) = delete;
    %class% &operator=(const TextEditDocument&) = delete;

    virtual bool OnCreate(wxDocument* doc, long flags) override;

    virtual bool IsModified() const override;
    virtual void Modify(bool mod) override;
protected:
    virtual bool DoOpenDocument(const wxString& filename) override;
    virtual bool DoSaveDocument(const wxString& filename) override;
private:
    wxTextCtrl* GetTextCtrl() const;

    void OnTextChange(wxCommandEvent& event);

    wxDECLARE_DYNAMIC_CLASS(%class%);
};
)===";

auto TextDocGenerator::HeaderCode(Code& code) -> bool
{
    tt_string_vector lines;
    lines.ReadString(txt_TextCtrlDocHdrBlock);
    tt_string class_name = code.node()->as_string(prop_class_name);
    for (auto& line: lines)
    {
        line.Replace("%class%", class_name, true);
        code.Str(line).Eol();
    }

    return true;
}
