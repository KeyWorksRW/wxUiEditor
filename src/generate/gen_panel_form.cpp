/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPanel Form generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>  // Base header for wxPanel

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "node_creator.h"   // Class used to create nodes
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_panel_form.h"

wxObject* PanelFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                              DlgSize(node, prop_size), GetStyleInt(node));
    if (!node->hasValue(prop_extra_style))
    {
        int ex_style = 0;
        // Can't use multiview because getConstantAsInt() searches an unordered_map which
        // requires a std::string to pass to it
        tt_string_vector mstr(node->as_string(prop_extra_style), '|');
        for (auto& iter: mstr)
        {
            // Friendly names will have already been converted, so normal lookup works fine.
            ex_style |= NodeCreation.getConstantAsInt(iter);
        }

        widget->SetExtraStyle(widget->GetExtraStyle() | ex_style);
    }

    if (node->isPropValue(prop_variant, "small"))
        widget->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    else if (node->isPropValue(prop_variant, "mini"))
        widget->SetWindowVariant(wxWINDOW_VARIANT_MINI);
    else if (node->isPropValue(prop_variant, "large"))
        widget->SetWindowVariant(wxWINDOW_VARIANT_LARGE);

    return widget;
}

bool PanelFormGenerator::ConstructionCode(Code& code)
{
    // Note: Form construction is called before any indentation is set
    if (code.is_cpp())
    {
        code.Str("bool ").as_string(prop_class_name);
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
        code.Comma().Add("pos=").Pos(prop_pos, code::force_scaling);
        code.Comma().Add("size=").WxSize(prop_size, code::force_scaling);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() +
                                     4);
        code.Add("style=").Style().Comma();
        size_t name_len = code.hasValue(prop_window_name) ?
                              code.node()->as_string(prop_window_name).size() :
                              sizeof("wx.DialogNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.hasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code.Str("wx.PanelNameStr");
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.Panel.__init__(self)";
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::Panel").Eol();
        code.AddPublicRubyMembers();
        code.Eol(eol_if_needed).Tab().Add("def initialize(parent");
        // Indent any wrapped lines
        code.Indent(3);
        code.Str(", id=");
        if (code.hasValue(prop_id))
        {
            code.Add(prop_id);
        }
        else
        {
            code.Add("Wx::ID_ANY");
        }
        // We have to break these out in order to add the variable assignment (pos=, size=, etc.)
        code.Comma()
            .CheckLineLength(sizeof("pos = Wx::DEFAULT_POSITION"))
            .Str("pos = ")
            .Pos(prop_pos);
        code.Comma()
            .CheckLineLength(sizeof("size = Wx::DEFAULT_SIZE"))
            .Str("size = ")
            .WxSize(prop_size);
        code.Comma()
            .CheckLineLength(sizeof("style = Wx::DEFAULT_DIALOG_STYLE"))
            .Str("style = ")
            .Style();
        if (code.hasValue(prop_window_name))
        {
            code.Comma().CheckLineLength(sizeof("name = ") +
                                         code.as_string(prop_window_name).size() + 2);
            code.Str("name = ").QuotedString(prop_window_name);
        }

        code.EndFunction();
        code.Unindent();
        if (auto indent_pos = code.GetCode().find("parent"); tt::is_found(indent_pos))
        {
            indent_pos -= code.GetCode().find("\n");
            std::string spaces(indent_pos, ' ');
            code.GetCode().Replace("\t\t\t\t", spaces, true);
        }
    }
    else if (code.is_perl())
    {
        code.Str("sub new {");
        code.Indent();
        code.Eol().Str("my( $class, $parent, $id, $pos, $size, $style, $name ) = @_;");
        code.Eol() += "$parent = undef unless defined $parent;";
        code.Eol().Str("$id = ").as_string(prop_id).Str(" unless defined $id;");
        code.Eol().Str("$pos = ").Pos().Str(" unless defined $pos;");
        code.Eol().Str("$size = ").WxSize(prop_size).Str(" unless defined $size;");
        code.Eol().Str("$style = ").Style().Str(" unless defined $style;");
        code.Eol().Str("$name = ");
        if (code.hasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code += "\"panel\"";
        code.Str(" unless defined $name;");

        code.Eol().Str(
            "my $self = $class->SUPER::new( $parent, $id, $pos, $size, $style, $name );");
    }
    else
    {
        code.AddComment("Unknown language", true);
    }

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool PanelFormGenerator::SettingsCode(Code& code)
{
    if (!code.node()->isPropValue(prop_variant, "normal"))
    {
        code.Eol(eol_if_empty).FormFunction("SetWindowVariant(");
        if (code.node()->isPropValue(prop_variant, "small"))
            code.Add("wxWINDOW_VARIANT_SMALL");
        else if (code.node()->isPropValue(prop_variant, "mini"))
            code.Add("wxWINDOW_VARIANT_MINI");
        else
            code.Add("wxWINDOW_VARIANT_LARGE");

        code.EndFunction();
    }

    if (code.is_cpp())
    {
        code.Eol(eol_if_needed) += "if (!";
        if (code.node()->hasValue(prop_subclass))
            code.as_string(prop_subclass);
        else
            code += "wxPanel";
        code += "::Create(parent, id, pos, size, style, name))";
        code.Eol().OpenBrace().Str("return false;").CloseBrace().Eol(eol_always);
    }
    else if (code.is_python())
    {
        code.Eol(eol_if_needed).Str("if not self.Create(parent, id, pos, size, style, name):");
        code.Eol().Tab().Str("return");
    }
    else if (code.is_ruby())
    {
        code.Eol(eol_if_needed).Str("super(parent, id, pos, size, style, name)");
        // REVIEW: [Randalphwa - 07-17-2023] The following doesn't work with an error that
        // Wx::Panel.create doesn't exist. code.Eol(eol_if_needed).Str("return false unless
        // Wx::Panel.create(parent, id, pos, size, style, name)");
    }
    else
    {
        return false;
    }

    return true;
}

bool PanelFormGenerator::AfterChildrenCode(Code& code)
{
    Node* form;
    auto* node = code.node();
    Node* form_sizer = nullptr;
    if (node->isForm())
    {
        form = node;
        ASSERT_MSG(form->getChildCount(), "Trying to generate code for a wxform with no children.")
        if (!form->getChildCount())
            return true;  // empty dialog, so nothing to do
        ASSERT_MSG(form->getChild(0)->isSizer() || form->isGen(gen_PanelForm),
                   "Expected first child of a wxform to be a sizer.");
        if (form->getChild(0)->isSizer())
            form_sizer = form->getChild(0);
    }
    else
    {
        form = node->getForm();
    }

    const auto min_size = form->as_wxSize(prop_minimum_size);
    const auto max_size = form->as_wxSize(prop_maximum_size);
    const auto size = form->as_wxSize(prop_size);

    if (!form_sizer)
    {
        if (min_size != wxDefaultSize)
        {
            code.Eol()
                .FormFunction("SetMinSize(")
                .WxSize(prop_minimum_size, code::force_scaling)
                .EndFunction();
        }
        if (max_size != wxDefaultSize)
        {
            code.Eol()
                .FormFunction("SetMaxSize(")
                .WxSize(prop_maximum_size, code::force_scaling)
                .EndFunction();
        }

        // Note that without a sizer, we cannot calculate a default dimension
        if (size != wxDefaultSize)
        {
            code.Eol()
                .FormFunction("SetSize(")
                .WxSize(prop_size, code::force_scaling)
                .EndFunction();
        }

        return true;
    }

    if (min_size == wxDefaultSize && max_size == wxDefaultSize && size == wxDefaultSize)
    {
        code.FormFunction("SetSizerAndFit(").NodeName(form_sizer).EndFunction();
    }
    else
    {
        if (min_size != wxDefaultSize)
        {
            code.Eol()
                .FormFunction("SetMinSize(")
                .WxSize(prop_minimum_size, code::force_scaling)
                .EndFunction();
        }
        if (max_size != wxDefaultSize)
        {
            code.Eol()
                .FormFunction("SetMaxSize(")
                .WxSize(prop_maximum_size, code::force_scaling)
                .EndFunction();
        }

        if (form->as_wxSize(prop_size) != wxDefaultSize)
        {
            // If both size.x and size.y are set to non-default values, then set the window size and
            // call Layout() to layout the child controls to match the new size. However, if one
            // dimension has a default value, then we need to call Fit() first to calculate what
            // that size should be, then call SetSize() and Layout().

            code.FormFunction("SetSizer(").NodeName(form_sizer).EndFunction();
            if (size.x == wxDefaultCoord || size.y == wxDefaultCoord)
            {
                Code code_temp = code;
                code_temp.clear();
                code_temp.FormFunction("SetSize(")
                    .WxSize(prop_size, code::force_scaling)
                    .EndFunction();
                auto comment_column = code_temp.size() + 2;

                code_temp = code;
                code_temp.clear();
                code_temp.FormFunction("Fit(").EndFunction();
                while (code_temp.size() < comment_column)
                    code_temp += ' ';
                code.Eol().Str(code_temp);
                if (size.x == wxDefaultCoord)
                    code += "// calculate width";
                else
                    code += "// calculate height";

                code.Eol()
                    .FormFunction("SetSize(")
                    .WxSize(prop_size, code::force_scaling)
                    .EndFunction();
                code.Str("  // set specified and calculated size dimensions");

                code_temp = code;
                code_temp.clear();
                code_temp.FormFunction("Layout(").EndFunction();
                while (code_temp.size() < comment_column)
                    code_temp += ' ';
                code.Eol().Str(code_temp).Str("// change layout to match SetSize() value");
            }
            else
            {
                code.Eol()
                    .FormFunction("SetSize(")
                    .WxSize(prop_size, code::force_scaling)
                    .EndFunction();
                code.Eol().FormFunction("Layout(").EndFunction();
            }
        }
        else
        {
            code.FormFunction("SetSizerAndFit(").NodeName(form_sizer).EndFunction();
        }
    }

    // REVIEW: [Randalphwa - 08-08-2024] gen_dialog checks for a child that has focus and sets it
    // here. Should we do the same for wxPanel?

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
        code.Pos(prop_pos, no_dpi_scaling);

    code.Comma().Str("const wxSize& size = ");

    auto size = node->as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dpi_scaling);

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
        code.Pos(prop_pos, no_dpi_scaling);

    code.Comma().Str("const wxSize& size = ");

    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dpi_scaling);

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
    if (node->hasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxPanelNameStr");

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool PanelFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.hasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
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
    if (node->getParent() && node->getParent()->isSizer())
        result = BaseGenerator::xrc_sizer_item_created;
    auto item = InitializeXrcObject(node, object);

    item.append_attribute("class").set_value("wxPanel");
    object.append_attribute("name").set_value(node->as_string(prop_class_name));

    if (!node->isPropValue(prop_variant, "normal"))
    {
        ADD_ITEM_PROP(prop_variant, "variant")
    }
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

bool PanelFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);

    return true;
}

bool PanelFormGenerator::GetImports(Node* node, std::set<std::string>& set_imports,
                                    GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use base qw[Wx::Panel];");
        set_imports.emplace("use Wx qw[:panel];");
        set_imports.emplace("use Wx qw[:misc];");  // for wxDefaultPosition and wxDefaultSize

        if (auto qw_events = GatherPerlNodeEvents(node); qw_events.size())
        {
            set_imports.emplace(qw_events);
        }

        return true;
    }

    return false;
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
