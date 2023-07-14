/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPanel Form generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>  // Base header for wxPanel

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_panel_form.h"

wxObject* PanelFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                              DlgSize(parent, node, prop_size), GetStyleInt(node));
    if (!node->HasValue(prop_extra_style))
    {
        int ex_style = 0;
        // Can't use multiview because GetConstantAsInt() searches an unordered_map which
        // requires a std::string to pass to it
        tt_string_vector mstr(node->value(prop_extra_style), '|');
        for (auto& iter: mstr)
        {
            // Friendly names will have already been converted, so normal lookup works fine.
            ex_style |= NodeCreation.GetConstantAsInt(iter);
        }

        widget->SetExtraStyle(widget->GetExtraStyle() | ex_style);
    }

    return widget;
}

bool PanelFormGenerator::ConstructionCode(Code& code)
{
    // Note: Form construction is called before any indentation is set
    if (code.is_cpp())
    {
        code.Str("bool ").Str((prop_class_name));
        code += "::Create(wxWindow* parent, wxWindowID id";
        code.Comma().Str("const wxPoint& pos").Comma().Str("const wxSize& size");
        code.Comma().Str("long style").Comma().Str("const wxString& name)");
        code.OpenBrace();
    }
    else if (code.is_python())
    {
        code.Add("class ").NodeName().Add("(wx.Panel):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").as_string(prop_id);
        code.Indent(3);
        code.Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() + 4);
        code.Add("style=").Style().Comma();
        size_t name_len =
            code.HasValue(prop_window_name) ? code.node()->as_string(prop_window_name).size() : sizeof("wx.DialogNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.HasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code.Str("wx.PanelNameStr");
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.Panel.__init__(self)";
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::Panel");
        code.Eol().Tab().Add("def initialize(parent, ");
        // Indent any wrapped lines
        code.Indent();
        if (code.HasValue(prop_id))
            code.Add(prop_id);
        else
            code.Add("Wx::ID_ANY");
        code.PosSizeFlags();
        code.EndFunction();
        code.Unindent();
    }
    else
    {
        code.AddComment("Unknown language");
    }

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool PanelFormGenerator::SettingsCode(Code& code)
{
    if (code.is_cpp())
    {
        code.Eol(eol_if_needed).FormFunction("if (!wxPanel::Create(").Str("parent, id, pos, size, style, name))");
        code.Eol().Tab().Str("return false;\n");
    }
    else if (code.is_python())
    {
        code.Eol(eol_if_needed).Str("if not self.Create(parent, id, pos, size, style, name):");
        code.Eol().Tab().Str("return");
    }
    else if (code.is_ruby())
    {
        code.Eol(eol_if_needed).Str("if !create(parent, id, pos, size, style, name)");
        code.Eol().Tab().Str("return false");
    }
    else
    {
        return false;
    }

    return true;
}

bool PanelFormGenerator::AfterChildrenCode(Code& code)
{
    Node* panel;
    auto* node = code.node();
    if (node->IsForm())
    {
        panel = node;
        ASSERT_MSG(panel->GetChildCount(), "Trying to generate code for a wxPanel with no children.")
        if (!panel->GetChildCount())
            return true;  // empty dialog, so nothing to do
        ASSERT_MSG(panel->GetChild(0)->IsSizer(), "Expected first child of a wxPanel to be a sizer.");
        if (panel->GetChild(0)->IsSizer())
            node = panel->GetChild(0);
    }
    else
    {
        panel = node->get_form();
    }

    const auto min_size = panel->as_wxSize(prop_minimum_size);
    const auto max_size = panel->as_wxSize(prop_maximum_size);
    const auto size = panel->as_wxSize(prop_size);

    if (min_size == wxDefaultSize && max_size == wxDefaultSize)
    {
        if (code.is_ruby())
            code.FormFunction("set_sizer_and_fit(").NodeName(node).EndFunction();
        else
            code.FormFunction("SetSizerAndFit(").NodeName(node).EndFunction();
    }
    else
    {
        if (code.is_ruby())
        {
            code.FormFunction("set_sizer(").NodeName(node).EndFunction();
            code.Eol().FormFunction("fit(").EndFunction();
        }
        else
        {
            code.FormFunction("SetSizer(").NodeName(node).EndFunction();
            code.Eol().FormFunction("Fit(").EndFunction();
        }
    }

    if (size != wxDefaultSize)
    {
        code.Eol().FormFunction("SetSize(").WxSize(prop_size).EndFunction();
    }

    return true;
}

bool PanelFormGenerator::HeaderCode(Code& code)
{
    auto* node = code.node();

    code.NodeName() += "() {}";
    code.Eol().NodeName().Str("(wxWindow* parent, wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxPoint& pos = ");

    auto position = node->as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
        code.Str("wxDefaultPosition");
    else
        code.Pos(prop_pos, no_dlg_units);

    code.Comma().Str("const wxSize& size = ");

    auto size = node->as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dlg_units);

    auto& style = node->as_string(prop_style);
    auto& win_style = node->as_string(prop_window_style);
    if (style.empty() && win_style.empty())
        code.Comma().Str("long style = 0");
    else
    {
        code.Comma();
        code.CheckLineLength(style.size() + win_style.size() + sizeof("long style = "));
        code.Str("long style = ");
        if (style.size())
        {
            code.CheckLineLength(style.size() + win_style.size());
            code += style;
            if (win_style.size())
            {
                code << '|' << win_style;
            }
        }
        else if (win_style.size())
        {
            code.Str(win_style);
        }
    }

    if (node->as_string(prop_window_name).size())
    {
        code.Comma().Str("const wxString &name = ").QuotedString(prop_window_name);
    }
    else
    {
        code.Comma().Str("const wxString &name = wxPanelNameStr");
    }

    code.Str(")").Eol().OpenBrace().Str("Create(parent, id, pos, size, style, name);").CloseBrace();

    code.Eol().Str("bool Create(wxWindow *parent");
    code.Comma().Str("wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxPoint& pos = ");

    if (position == wxDefaultPosition)
        code.Str("wxDefaultPosition");
    else
        code.Pos(prop_pos, no_dlg_units);

    code.Comma().Str("const wxSize& size = ");

    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dlg_units);

    if (style.empty() && win_style.empty())
        code.Comma().Str("long style = 0");
    else
    {
        code.Comma();
        code.CheckLineLength(style.size() + win_style.size() + sizeof("long style = "));
        code.Str("long style = ");
        if (style.size())
        {
            code.CheckLineLength(style.size() + win_style.size());
            code += style;
            if (win_style.size())
            {
                code << '|' << win_style;
            }
        }
        else if (win_style.size())
        {
            code.Str(win_style);
        }
    }

    code.Comma().Str("const wxString &name = ");
    if (node->HasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxPanelNameStr");

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool PanelFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_derived_class))
    {
        code.Str((prop_derived_class));
    }
    else
    {
        code += "wxPanel";
    }

    return true;
}

int PanelFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = BaseGenerator::xrc_updated;
    if (node->GetParent() && node->GetParent()->IsSizer())
        result = BaseGenerator::xrc_sizer_item_created;
    auto item = InitializeXrcObject(node, object);

    item.append_attribute("class").set_value("wxPanel");
    object.append_attribute("name").set_value(node->as_string(prop_class_name));

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void PanelFormGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxPanelXmlHandler");
}

bool PanelFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);

    return true;
}

tt_string PanelFormGenerator::GetPythonHelpText(Node* /* node */)
{
    tt_string help_text("wx.Panel");
    return help_text;
}

tt_string PanelFormGenerator::GetRubyHelpText(Node* /* node */)
{
    tt_string help_text("Wx::Panel");
    return help_text;
}

tt_string PanelFormGenerator::GetPythonURL(Node* /* node */)
{
    tt_string help_text("wx.Panel.html");
    return help_text;
}

tt_string PanelFormGenerator::GetRubyURL(Node* /* node */)
{
    tt_string help_text("Wx/Panel.html");
    return help_text;
}
