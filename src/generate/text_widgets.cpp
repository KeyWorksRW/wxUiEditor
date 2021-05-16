/////////////////////////////////////////////////////////////////////////////
// Purpose:   Text component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>                  // Event classes
#include <wx/html/htmlwin.h>           // wxHtmlWindow class for parsing & displaying HTML
#include <wx/richtext/richtextctrl.h>  // A rich edit control
#include <wx/stattext.h>               // wxStaticText base header
#include <wx/stc/stc.h>                // A wxWidgets implementation of Scintilla.

#include "text_widgets.h"

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

wxObject* StaticTextGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, node->prop_as_wxPoint(prop_pos),
                                   node->prop_as_wxSize(prop_size),
                                   node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    if (node->prop_as_bool(prop_markup))
        widget->SetLabelMarkup(node->prop_as_wxString(prop_label));
    else
        widget->SetLabel(node->prop_as_wxString(prop_label));

    if (node->prop_as_int(prop_wrap) > 0)
        widget->Wrap(node->prop_as_int(prop_wrap));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool StaticTextGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_wrap) || prop->isProp(prop_label) || prop->isProp(prop_markup))
    {
        // If the text was wrapped previously, then it already has \n characters inserted in it, so we need to restore
        // it to it's original state before wrapping again.

        auto ctrl = wxStaticCast(widget, wxStaticText);
        if (node->prop_as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->prop_as_wxString(prop_label));
        else
            ctrl->SetLabel(node->prop_as_wxString(prop_label));

        if (node->prop_as_int(prop_wrap) > 0)
            ctrl->Wrap(node->prop_as_int(prop_wrap));

        return true;
    }

    return false;
}

std::optional<ttlib::cstr> StaticTextGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxStaticText(";

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    // If the label is going to be set via SetLabelMarkup(), then there is no reason to initialize it here and then
    // replace it on the next line of code (which will be the call to SetLabelMarkUp())
    if (node->prop_as_bool(prop_markup))
    {
        code << "wxEmptyString";
    }
    else
    {
        auto& label = node->prop_as_string(prop_label);
        if (label.size())
        {
            code << GenerateQuotedString(label);
        }
        else
        {
            code << "wxEmptyString";
        }
    }

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> StaticTextGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> StaticTextGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;
    if (node->prop_as_bool(prop_markup))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetLabelMarkup(" << GenerateQuotedString(node->prop_as_string(prop_label))
             << ");";
    }

    // Note that wrap MUST be called after the text is set, otherwise it will be ignored.
    if (node->prop_as_int(prop_wrap) > 0)
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->Wrap(" << node->prop_as_string(prop_wrap) << ");";
    }
    return code;
}

bool StaticTextGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/stattext.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  TextCtrlGenerator  //////////////////////////////////////////

wxObject* TextCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                 node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size),
                                 node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    widget->SetMaxLength(node->prop_as_int(prop_maxlength));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool TextCtrlGenerator::OnPropertyChange(wxObject* widget, Node* /* node */, NodeProperty* prop)
{
    if (prop->isProp(prop_value))
    {
        wxStaticCast(widget, wxTextCtrl)->SetValue(prop->GetValue().wx_str());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> TextCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxTextCtrl(";

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->prop_as_string(prop_value).size())
        code << GenerateQuotedString(node->prop_as_string(prop_value));
    else
        code << "wxEmptyString";

    if (node->prop_as_string(prop_window_name).empty())
        GeneratePosSizeFlags(node, code);
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, since GenSettings will create it, but we have to supply something before the window
        // name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> TextCtrlGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;
    if (node->prop_as_bool(prop_maxlength))
    {
        if (code.size())
            code << '\n';
        if (node->prop_as_string(prop_style).contains("wxTE_MULTILINE"))
        {
            code << "#if !defined(__WXGTK__))\n\t";
            code << node->get_node_name() << "->SetMaxLength(" << node->prop_as_string(prop_maxlength) << ");\n";
            code << "#endif";
            auto_indent = false;
        }
        else
        {
            code << node->get_node_name() << "->SetMaxLength(" << node->prop_as_string(prop_maxlength) << ");";
        }
    }
    return code;
}

std::optional<ttlib::cstr> TextCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool TextCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/textctrl.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
    {
        if (node->prop_as_string(prop_validator_type) == "wxGenericValidator")
            InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
        else
            InsertGeneratorInclude(node, "#include <wx/valtext.h>", set_src, set_hdr);
    }

    return true;
}

//////////////////////////////////////////  RichTextCtrlGenerator  //////////////////////////////////////////

wxObject* RichTextCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRichTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString,
                                     node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size),
                                     node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> RichTextCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRichTextCtrl(";

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    code << "wxEmptyString";

    if (node->prop_as_string(prop_window_name).empty())
        GeneratePosSizeFlags(node, code);
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, but we have to supply something before the window name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> RichTextCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RichTextCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/richtext/richtextctrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  HtmlWindowGenerator  //////////////////////////////////////////

wxObject* HtmlWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxHtmlWindow(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                   node->prop_as_wxSize(prop_size),
                                   node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    widget->SetPage("<b>wxHtmlWindow</b><br/><br/>This is a dummy page.</body></html>");

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> HtmlWindowGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxHtmlWindow(";

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    code.Replace(", wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO)", ")");

    return code;
}

std::optional<ttlib::cstr> HtmlWindowGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool HtmlWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/html/htmlwin.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  StyledTextGenerator  //////////////////////////////////////////

wxObject* StyledTextGenerator::CreateMockup(Node* node, wxObject* parent)
{
    // REVIEW: [KeyWorks - 12-10-2020] This is the original code which needs to be replaced as part of issue #512

    auto scintilla = new wxStyledTextCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                          node->prop_as_wxSize(prop_size), node->prop_as_int(prop_window_style),
                                          node->prop_as_wxString(prop_var_name));

    if (node->prop_as_int(prop_line_numbers) != 0)
    {
        scintilla->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        scintilla->SetMarginWidth(0, scintilla->TextWidth(wxSTC_STYLE_LINENUMBER, "_99999"));
    }
    else
    {
        scintilla->SetMarginWidth(0, 0);
    }

    if (node->prop_as_int(prop_folding) != 0)
    {
        scintilla->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
        scintilla->SetMarginMask(1, wxSTC_MASK_FOLDERS);
        scintilla->SetMarginWidth(1, 16);
        scintilla->SetMarginSensitive(1, true);

        scintilla->SetProperty("fold", "1");
        scintilla->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
    }
    else
    {
        scintilla->SetMarginWidth(1, 0);
    }
    scintilla->SetIndentationGuides(node->prop_as_int(prop_indentation_guides));
    scintilla->SetUseTabs((node->prop_as_int(prop_use_tabs)));
    scintilla->SetTabWidth(node->prop_as_int(prop_tab_width));
    scintilla->SetTabIndents((node->prop_as_int(prop_tab_indents)));
    scintilla->SetBackSpaceUnIndents((node->prop_as_int(prop_backspace_unindents)));
    scintilla->SetIndent(node->prop_as_int(prop_tab_width));
    scintilla->SetViewEOL((node->prop_as_int(prop_view_eol)));
    scintilla->SetViewWhiteSpace(node->prop_as_int(prop_view_whitespace));

    if (node->HasValue(prop_font))
    {
        scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, node->prop_as_font(prop_font));
    }

#if 0

    // REVIEW: [KeyWorks - 12-10-2020] All of the Marker codes below were originally generated in the code generation constructor

    scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS);
    scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, wxColour("BLACK"));
    scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, wxColour("WHITE"));
    scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS);
    scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, wxColour("BLACK"));
    scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, wxColour("WHITE"));
    scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
    scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS);
    scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, wxColour("BLACK"));
    scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, wxColour("WHITE"));
    scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS);
    scintilla->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour("BLACK"));
    scintilla->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour("WHITE"));
    scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
    scintilla->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);

    scintilla->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    scintilla->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));

    // REVIEW: [KeyWorks - 12-10-2020] Everything below here could not actually be generated by the user -- which meant the Mockup windows
    // would NOT look the same as it would when the user compiled the class we created for them. The purpose of the Mockup window is to
    // resemble as closely as possible what will be shown when the user's program is compiled and run -- I can't think of any valid reason
    // to make all the settings below which will make the window appear quite different.

    scintilla->SetLexer(wxSTC_LEX_CPP);
    scintilla->SetKeyWords(0, "asm auto bool break case catch char class const const_cast \
							   continue default delete do double dynamic_cast else enum explicit \
							   export extern false float for friend goto if inline int long \
							   mutable namespace new operator private protected public register \
							   reinterpret_cast return short signed sizeof static static_cast \
							   struct switch template this throw true try typedef typeid \
							   typename union unsigned using virtual void volatile wchar_t \
							   while");

    if (!node->HasValue(prop_font))
    {
        scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    }

    scintilla->StyleClearAll();
    scintilla->StyleSetBold(wxSTC_C_WORD, true);
    scintilla->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
    scintilla->StyleSetForeground(wxSTC_C_STRING, *wxRED);
    scintilla->StyleSetForeground(wxSTC_C_STRINGEOL, *wxRED);
    scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
    scintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 128, 0));
    scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
    scintilla->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 128, 0));
    scintilla->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour(0, 128, 0));
    scintilla->StyleSetForeground(wxSTC_C_NUMBER, *wxBLUE);

    scintilla->SetMarginWidth(2, 0);
    scintilla->SetCaretWidth(2);

    scintilla->SetText("/** Sample Class to Display wxScintilla */\n"
                       "class ScintillaSampleCode\n"
                       "{\n"
                       "private:\n"
                       "\tint m_privateMember;\n\n"
                       "public:\n\n"
                       "\t// Sample Member Function\n"
                       "\tint SampleFunction( int sample = 0 )\n"
                       "\t{\n"
                       "\t\treturn sample;\n"
                       "\t}\n"
                       "};\n");

#else
    scintilla->SetText("Sample text...");
#endif

    scintilla->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return scintilla;
}

std::optional<ttlib::cstr> StyledTextGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxStyledTextCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> StyledTextGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    // There are potentially a LOT of settings, so we put them all in a bracket pair to make them easier to identifiy

    auto_indent = false;
    code << "\t{";

    if (node->prop_as_bool(prop_folding))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetProperty(\"fold\", \"1\");";

        // TOD: [KeyWorks - 12-10-2020] All of these need to be user-settable options
        code << "\n\t\t" << node->get_node_name() << "->SetMarginType(1, wxSTC_MARGIN_SYMBOL);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginMask(1, wxSTC_MASK_FOLDERS);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginWidth(1, 16);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginSensitive(1, true);";
        code << "\n\t\t" << node->get_node_name()
             << "->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);";
    }

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_line_numbers))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetMarginWidth(0, 0);";
    }
    else
    {
        code << "\n\t\t" << node->get_node_name() << "->SetMarginType(0, wxSTC_MARGIN_NUMBER);";
        code << "\n\t\t" << node->get_node_name() << "->SetMarginWidth(0, " << node->get_node_name()
             << "->TextWidth(wxSTC_STYLE_LINENUMBER, \"_99999\"));";
    }

    // BUGBUG: [KeyWorks - 12-10-2020] Indentation is not a boolean -- there are 4 possible values
    if (!node->prop_as_bool(prop_indentation_guides))
        code << "\n\t\t" << node->get_node_name() << "->SetIndentationGuides(0);";

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_use_tabs))
    {
        code << "\n\t\t" << node->get_node_name() << "->SetUseTabs(false);";

        if (node->prop_as_int(prop_tab_width) != 8)
            code << "\n\t\t" << node->get_node_name() << "->SetTabWidth(" << node->prop_as_string(prop_tab_width) << ");";
    }

    // Default is true, so only set if false
    if (!node->prop_as_bool(prop_tab_indents))
        code << "\n\t\t" << node->get_node_name() << "->SetTabIndents(false);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_backspace_unindents))
        code << "\n\t\t" << node->get_node_name() << "->SetBackSpaceUnIndents(true);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_view_eol))
        code << "\n\t\t" << node->get_node_name() << "->SetViewEOL(true);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_view_whitespace))
        code << "\n\t\t" << node->get_node_name() << "->SetViewWhiteSpace(true);";

    // Default is false, so only set if true
    if (node->prop_as_bool(prop_read_only))
        code << "\n\t\t" << node->get_node_name() << "->SetReadOnly(true);";

    ttlib::cstr win_settings;

    // BUGBUG: [KeyWorks - 12-19-2020] This will also be generated in gen_base if we don't special-case it
    GenerateWindowSettings(node, win_settings);
    if (win_settings.size())
    {
        // Need to increase indentation of all lines by one tab
        win_settings.Replace("\n\t", "\n\t\t");
        code << "\n\t\t" << win_settings;
    }

    code << "\n\t}";

    if (code.is_sameas("\t{\n\t}"))
        code.clear();  // means there were no settings

    return code;
}

std::optional<ttlib::cstr> StyledTextGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StyledTextGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/stc/stc.h>", set_src, set_hdr);
    return true;
}

// REVIEW: [KeyWorks - 12-10-2020] This was in the original code, but it's not hooked up yet.
void StyledTextGenerator::OnMarginClick(wxStyledTextEvent& event)
{
    if (auto scintilla = wxStaticCast(event.GetEventObject(), wxStyledTextCtrl); scintilla)
    {
        if (event.GetMargin() == 1)
        {
            int lineClick = scintilla->LineFromPosition(event.GetPosition());
            int levelClick = scintilla->GetFoldLevel(lineClick);
            if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
            {
                scintilla->ToggleFold(lineClick);
            }
        }
    }
    event.Skip();
}
