/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/manager.h>  // wxPropertyGridManager
#include <wx/textctrl.h>  // wxTextAttr and wxTextCtrlBase class - the interface of wxTextCtrl

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_text_ctrl.h"

wxObject* TextCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_value),
                       DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));

#if defined(__WXGTK__)
    if (widget->IsSingleLine())
#endif
        widget->SetMaxLength(node->as_int(prop_maxlength));

    if (node->HasValue(prop_auto_complete))
    {
        auto array = node->as_wxArrayString(prop_auto_complete);
        widget->AutoComplete(array);
    }

    if (node->HasValue(prop_hint))
        widget->SetHint(node->as_wxString(prop_hint));

#if wxUSE_SPELLCHECK
    if (node->as_string(prop_spellcheck).contains("enabled"))
    {
        if (node->as_string(prop_spellcheck).contains("grammar"))
            widget->EnableProofCheck(wxTextProofOptions::Default().GrammarCheck());
        else
            widget->EnableProofCheck(wxTextProofOptions::Default());
    }
#endif

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool TextCtrlGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_value))
    {
        wxStaticCast(widget, wxTextCtrl)->SetValue(prop->as_wxString());
        return true;
    }
#if defined(_WIN32)
    else if (prop->isProp(prop_spellcheck))
    {
        if (prop->HasValue() && !node->as_string(prop_style).contains("wxTE_RICH2"))
        {
            wxGetFrame().ShowInfoBarMsg(
                "When used on Windows, spell checking requires the style to contain wxTE_RICH2.",
                wxICON_INFORMATION);
        }
        else
        {
            wxGetFrame().DismissInfoBar();
        }
    }
    else if (prop->isProp(prop_style))
    {
        if (node->HasValue(prop_spellcheck) && !node->as_string(prop_style).contains("wxTE_RICH2"))
        {
            wxGetFrame().ShowInfoBarMsg(
                "When used on Windows, spell checking requires the style to contain wxTE_RICH2.",
                wxICON_INFORMATION);
        }
        else
        {
            wxGetFrame().DismissInfoBar();
        }
    }
#endif  // _WIN32

    return false;
}

bool TextCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().CheckLineLength();
    code.QuotedString(prop_value);
    code.PosSizeFlags(code::allow_scaling, true);

    return true;
}

bool TextCtrlGenerator::SettingsCode(Code& code)
{
    if (code.HasValue(prop_hint))
    {
        if (code.is_perl())
        {
            code.Eol(eol_if_needed).Str("# wxPerl does not support SetHint()");
        }
        else
        {
            code.Eol(eol_if_needed)
                .NodeName()
                .Function("SetHint(")
                .QuotedString(prop_hint)
                .EndFunction();
        }
    }

    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->get_Form();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->is_Gen(gen_wxDialog) && !form->is_Type(type_frame_form))
        {
            code.Eol(eol_if_needed).NodeName().Function("SetFocus(").EndFunction();
        }
    }

    if (code.IsTrue(prop_maxlength))
    {
        code.Eol(eol_if_needed);
        if (code.PropContains(prop_style, "wxTE_MULTILINE"))
        {
            if (code.is_cpp())
            {
                code << "#if !defined(__WXGTK__)";
                code.Eol()
                    .Tab()
                    .NodeName()
                    .Function("SetMaxLength(")
                    .as_string(prop_maxlength)
                    .EndFunction()
                    .Eol();
                code.GetCode() += "#endif";
            }
            else if (code.is_perl())
            {
                code.Str("if (Wx::wxVERSION_STRING() !~ /GTK/)").OpenBrace();
                code.Eol(eol_if_needed)
                    .NodeName()
                    .Function("SetMaxLength(")
                    .as_string(prop_maxlength)
                    .EndFunction()
                    .Eol();
                code.CloseBrace();
            }
            else if (code.is_python())
            {
                code.Add("if wx.Platform != \'__WXGTK__\':");
                code.Eol()
                    .Tab()
                    .NodeName()
                    .Function("SetMaxLength(")
                    .as_string(prop_maxlength)
                    .EndFunction()
                    .Eol();
            }
            else if (code.is_ruby())
            {
                code.Add("if Wx::PLATFORM == 'WXGTK'");
                code.Eol()
                    .Tab()
                    .NodeName()
                    .Function("SetMaxLength(")
                    .as_string(prop_maxlength)
                    .EndFunction()
                    .Eol();
                code.Str("end");
            }
        }
        else
        {
            code.NodeName().Function("SetMaxLength(").as_string(prop_maxlength).EndFunction().Eol();
        }
    }

    if (code.HasValue(prop_auto_complete))
    {
        if (code.is_cpp())
        {
            code.EnableAutoLineBreak(false);
            code.Eol(eol_if_needed).Add("{").Eol().Tab().Add("wxArrayString tmp_array;").Eol();
            auto array = code.node()->as_ArrayString(prop_auto_complete);
            for (auto& iter: array)
            {
                code.Tab().Add("tmp_array.Add(").QuotedString(iter) << ");";
                code.Eol();
            }
            code.Tab() << code.node()->get_NodeName() << "->AutoComplete(tmp_array);";
            code.Eol() << "}";
            code.EnableAutoLineBreak(true);
        }

        // TODO: [Randalphwa - 12-02-2022] Add Python code
    }

    if (code.PropContains(prop_spellcheck, "enabled"))
    {
        if (code.is_cpp())
        {
            code.Eol(eol_if_needed).NodeName()
                << "->EnableProofCheck(wxTextProofOptions::Default()";
            if (code.PropContains(prop_spellcheck, "grammar"))
                code << ".GrammarCheck()";
            code.EndFunction();
        }
        else if (code.is_perl())
        {
            code.Eol(eol_if_needed).Str("# wxPerl does not support wxTextProofOptions").Eol();
        }
        else if (code.is_python())
        {
            code.Eol(eol_if_needed)
                .Str("# wxPython 4.2.0 does not support wxTextProofOptions")
                .Eol();
        }
        else if (code.is_ruby())
        {
            // REVIEW: [Randalphwa - 08-05-2023] The code is correct, but spell-checking does
            // not work as of wxRuby3 rc3
            code.Eol(eol_if_needed).NodeName().Function("EnableProofCheck(");
            code.Add("wxTextProofOptions").ClassMethod("Default");
            if (code.PropContains(prop_spellcheck, "grammar"))
                code.Function("GrammarCheck");
            code << ')';
        }
        else
        {
            code.Eol(eol_if_needed).Str("# unknown language in TextCtrlGenerator::SettingsCode");
        }
    }

    return true;
}

void TextCtrlGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid,
                                          NodeProperty* changed_prop)
{
    if (changed_prop->isProp(prop_spellcheck))
    {
        if (auto pg_parent = prop_grid->GetProperty("spellcheck"); pg_parent)
        {
            for (unsigned int idx = 0; idx < pg_parent->GetChildCount(); ++idx)
            {
                if (auto pg_setting = pg_parent->Item(idx); pg_setting)
                {
                    auto label = pg_setting->GetLabel();
                    if (label == "grammar")
                    {
                        pg_setting->Enable(changed_prop->as_string().contains("enabled"));
                    }
                }
            }
        }
    }
    else
    {
        BaseGenerator::ChangeEnableState(prop_grid, changed_prop);
    }
}

bool TextCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/textctrl.h>", set_src, set_hdr);

    // Only insert validator header files if the validator is being used (which requires a
    // variable name).
    if (node->HasValue(prop_validator_variable))
    {
        if (auto val_type = node->get_ValidatorType(); val_type.size())
        {
            if (node->as_string(prop_validator_data_type) == "wxFileName")
                set_hdr.insert("#include <wx/filename.h>");
            if (val_type == "wxGenericValidator")
                set_src.insert("#include <wx/valgen.h>");
            else if (val_type == "wxTextValidator")
                set_src.insert("#include <wx/valtext.h>");
            else if (val_type == "wxIntegerValidator" || val_type == "wxFloatingPointValidator")
                set_src.insert("#include <wx/valnum.h>");
        }
    }
    return true;
}

int TextCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxTextCtrl");

    ADD_ITEM_PROP(prop_maxlength, "maxlength")
    ADD_ITEM_PROP(prop_value, "value")

    GenXrcStylePosSize(node, item);

    if (node->HasValue(prop_value))
    {
        item.append_child("value").text().set(node->as_string(prop_value));
    }
    if (node->HasValue(prop_hint))
    {
        item.append_child("hint").text().set(node->as_string(prop_hint));
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->HasValue(prop_auto_complete))
        {
            item.append_child(pugi::node_comment)
                .set_value(" auto complete cannot be be set in the XRC file. ");
        }
        if (node->HasValue(prop_spellcheck))
        {
            item.append_child(pugi::node_comment)
                .set_value(" spell check cannot be be set in the XRC file. ");
        }
        GenXrcComments(node, item);
    }

    return result;
}

void TextCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxActivityIndicatorXmlHandler");
}

bool TextCtrlGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                   GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use Wx qw[:textctrl];");
    }

    return false;
}
