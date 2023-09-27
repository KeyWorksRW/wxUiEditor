/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/manager.h>  // wxPropertyGridManager
#include <wx/textctrl.h>          // wxTextAttr and wxTextCtrlBase class - the interface of wxTextCtrl

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // WriteCode -- Write code to Scintilla or file

#include "gen_text_ctrl.h"

wxObject* TextCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_value),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetMaxLength(node->as_int(prop_maxlength));

    if (node->hasValue(prop_auto_complete))
    {
        auto array = node->as_wxArrayString(prop_auto_complete);
        widget->AutoComplete(array);
    }

    if (node->hasValue(prop_hint))
        widget->SetHint(node->as_wxString(prop_hint));

    if (node->as_string(prop_spellcheck).contains("enabled"))
    {
        if (node->as_string(prop_spellcheck).contains("grammar"))
            widget->EnableProofCheck(wxTextProofOptions::Default().GrammarCheck());
        else
            widget->EnableProofCheck(wxTextProofOptions::Default());
    }

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
        if (prop->hasValue() && !node->as_string(prop_style).contains("wxTE_RICH2"))
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->ShowMessage("When used on Windows, spell checking requires the style to contain wxTE_RICH2.",
                                     wxICON_INFORMATION);
            }
        }
        else
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->Dismiss();
            }
        }
    }
    else if (prop->isProp(prop_style))
    {
        if (node->hasValue(prop_spellcheck) && !node->as_string(prop_style).contains("wxTE_RICH2"))
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->ShowMessage("When used on Windows, spell checking requires the style to contain wxTE_RICH2.",
                                     wxICON_INFORMATION);
            }
        }
        else
        {
            if (auto infobar = wxGetFrame().GetPropInfoBar(); infobar)
            {
                infobar->Dismiss();
            }
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
    code.PosSizeFlags(true);

    return true;
}

bool TextCtrlGenerator::SettingsCode(Code& code)
{
    if (code.hasValue(prop_hint))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
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
                code.Eol().Tab().NodeName().Function("SetMaxLength(").as_string(prop_maxlength).EndFunction().Eol();
                code.GetCode() += "#endif";
            }
            else
            {
                code.Add("if wx.Platform != \'__WXGTK__\':");
                code.Eol().Tab().NodeName().Function("SetMaxLength(").as_string(prop_maxlength).EndFunction().Eol();
            }
        }
        else
        {
            code.NodeName().Function("SetMaxLength(").as_string(prop_maxlength).EndFunction().Eol();
        }
    }

    if (code.hasValue(prop_auto_complete))
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
            code.Tab() << code.node()->getNodeName() << "->AutoComplete(tmp_array);";
            code.Eol() << "}";
            code.EnableAutoLineBreak(true);
        }

        // TODO: [Randalphwa - 12-02-2022] Add Python code
    }

    if (code.PropContains(prop_spellcheck, "enabled"))
    {
        if (code.is_cpp())
        {
            if (Project.as_string(prop_wxWidgets_version) == "3.1")
            {
                code.Eol(eol_if_needed) << "#if wxCHECK_VERSION(3, 1, 6)";
                code.Eol().Tab().NodeName() << "->EnableProofCheck(wxTextProofOptions::Default()";
                if (code.PropContains(prop_spellcheck, "grammar"))
                    code << ".GrammarCheck()";
                code << ");";
                code.Eol() << "#endif";
            }
            else
            {
                code.Eol(eol_if_needed).NodeName() << "->EnableProofCheck(wxTextProofOptions::Default()";
                if (code.PropContains(prop_spellcheck, "grammar"))
                    code << ".GrammarCheck()";
                code.EndFunction();
            }
        }
        else if (code.is_python())
        {
            code.Eol(eol_if_needed).Add("# wxPython 4.2.0 does not support wxTextProofOptions").Eol();
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

void TextCtrlGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop)
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

bool TextCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/textctrl.h>", set_src, set_hdr);

    // Only insert validator header files if the validator is being used (which requires a
    // variable name).
    if (node->hasValue(prop_validator_variable))
    {
        if (auto val_type = node->getValidatorType(); val_type.size())
        {
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
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxTextCtrl");

    ADD_ITEM_PROP(prop_maxlength, "maxlength")
    ADD_ITEM_PROP(prop_value, "value")

    GenXrcStylePosSize(node, item);

    if (node->hasValue(prop_value))
    {
        item.append_child("value").text().set(node->as_string(prop_value));
    }
    if (node->hasValue(prop_hint))
    {
        item.append_child("hint").text().set(node->as_string(prop_hint));
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->hasValue(prop_auto_complete))
        {
            item.append_child(pugi::node_comment).set_value(" auto complete cannot be be set in the XRC file. ");
        }
        if (node->hasValue(prop_spellcheck))
        {
            item.append_child(pugi::node_comment).set_value(" spell check cannot be be set in the XRC file. ");
        }
        GenXrcComments(node, item);
    }

    return result;
}

void TextCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxActivityIndicatorXmlHandler");
}

void TextCtrlGenerator::AddPropsAndEvents(NodeDeclaration* declaration)
{
    DeclAddVarNameProps(declaration, "m_textCtrl");
    // DeclAddProp(declaration, prop_var_name, type_string, "", "m_textCtrl");
    DeclAddProp(declaration, prop_value, type_string_escapes, "Specifies the initial value of the text control.");
    DeclAddProp(declaration, prop_hint, type_string_escapes,
                "The maximum length of user-entered text. 0 means no limit. Note that in wxGTK "
                "this function may only be used with single line text controls.");
    DeclAddProp(declaration, prop_maxlength, type_string,
                "The maximum length of user-entered text. 0 means no limit. Note that in wxGTK this function may only be "
                "used with single line text controls.");
    DeclAddProp(declaration, prop_auto_complete, type_stringlist_semi,
                "If one or more strings are entered, they will be used to initialize autocomplete.");

    auto* prop_info = DeclAddProp(declaration, prop_spellcheck, type_bitlist);
    {
        DeclAddOption(
            prop_info, "enabled",
            "Currently this is supported in wxMSW (when running under Windows 8 or later), wxGTK when using GTK 3 and "
            "wxOSX. In addition, wxMSW requires that the text control has the wxTE_RICH2 style set, while wxOSX "
            "requires that the control has the wxTE_MULTILINE style.\n\nAvailable since 3.1.6");
        DeclAddOption(
            prop_info, "grammar",
            "Enables grammar checking in addition to spell checking. Currently this is supported in wxMSW (when "
            "running under Windows 8 or later), wxGTK when using GTK 3 and wxOSX. In addition, wxMSW requires that "
            "the text control has the wxTE_RICH2 style set, while wxOSX requires that the control has the "
            "wxTE_MULTILINE style.\n\nAvailable since 3.1.6");
    }

    prop_info = DeclAddProp(declaration, prop_style, type_bitlist);
    {
        DeclAddOption(
            prop_info, "wxTE_PROCESS_ENTER",
            "The control will generate the event wxEVT_TEXT_ENTER (otherwise pressing Enter key is either processed "
            "internally by the control or used for navigation between dialog controls).");
        DeclAddOption(
            prop_info, "wxTE_PROCESS_TAB",
            "The control will receive wxEVT_CHAR events for TAB pressed - normally, TAB is used for passing to the "
            "next control in a dialog instead. For the control created with this style, you can still use Ctrl-Enter "
            "to pass to the next control from the keyboard.");
        DeclAddOption(prop_info, "wxTE_MULTILINE", "The text control allows multiple lines.");
        DeclAddOption(prop_info, "wxTE_PASSWORD", "The text will be echoed as asterisks.");
        DeclAddOption(prop_info, "wxTE_READONLY", "The text will not be user-editable.");
        DeclAddOption(prop_info, "wxTE_RICH",
                      "Use rich text control under Windows. This allows having more than 64KB of text in the control. This "
                      "style is ignored under other platforms.");
        DeclAddOption(
            prop_info, "wxTE_RICH2",
            "Use rich text control version 2.0 or 3.0 under Windows. This style is ignored under other platforms.");
        DeclAddOption(prop_info, "wxTE_AUTO_URL",
                      "Highlight the URLs and generate the wxTextUrlEvents when mouse events occur over them. This style is "
                      "only supported for wxTE_RICH Win32 and multi-line wxGTK2 text controls.");
        DeclAddOption(
            prop_info, "wxTE_NOHIDESEL",
            "By default, the Windows text control doesn't show the selection when it doesn't have focus - use this "
            "style to force it to always show it. This style is ignored under other platforms.");
        DeclAddOption(
            prop_info, "wxTE_NO_VSCROLL",
            "For multiline controls only: a vertical scrollbar will never be created. This limits the amount of text "
            "which can be entered into the control to what can be displayed in it under MSW but not under GTK2. "
            "Currently not implemented for the other platforms.");
        DeclAddOption(prop_info, "wxTE_LEFT", "The text in the control will be left-justified (default).");
        DeclAddOption(prop_info, "wxTE_CENTER",
                      "The text in the control will be centered (currently Windows and wxGTK2 only).");
        DeclAddOption(prop_info, "wxTE_RIGHT",
                      "The text in the control will be right-justified (currently Windows and wxGTK2 only).");
        DeclAddOption(prop_info, "wxTE_DONTWRAP",
                      "Same as wxHSCROLL style: don't wrap at all, show horizontal scrollbar instead.");
        DeclAddOption(prop_info, "wxTE_CHARWRAP",
                      "Wrap the lines too long to be shown entirely at any position (wxUniv and wxGTK2 only).");
        DeclAddOption(prop_info, "wxTE_WORDWRAP",
                      "Wrap the lines too long to be shown entirely at word boundaries (wxUniv and wxGTK2 only).");
        DeclAddOption(
            prop_info, "wxTE_BESTWRAP",
            "Wrap the lines at word boundaries or at any other character if there are words longer than the window "
            "width (this is the default).");
    }

    DeclAddProp(declaration, prop_focus, type_bool,
                "When checked, this control will be set to receive keyboard input when the parent form is first created.",
                "0");

    // Add events
    DeclAddEvent(declaration, "wxEVT_TEXT", "wxCommandEvent",
                 "Generated when the text changes. Notice that this event will always be generated when the text controls "
                 "contents changes - whether this is due to user input or comes from the program itself (for example, if "
                 "SetValue() is called.)");
    DeclAddEvent(declaration, "wxEVT_TEXT_ENTER", "wxCommandEvent",
                 "Generated when enter is pressed in a text control (which must have wxTE_PROCESS_ENTER style for this "
                 "event to be generated).");
    DeclAddEvent(declaration, "wxEVT_TEXT_URL", "wxTextUrlEvent",
                 "Generated when the a mouse event occurred over an URL in the text control (Windows and wxGTK2 only)");
    DeclAddEvent(declaration, "wxEVT_TEXT_MAXLEN", "wxCommandEvent",
                 "Generated when the user tries to enter more text into the control than the limit set by SetMaxLength.");
}
