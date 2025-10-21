/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDocParent common generator functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_frame_common.h"

#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "utils.h"       // Utility functions that work with properties

using namespace FrameCommon;

bool FrameCommon::ConstructionCode(Code& code, int frame_type)
{
    if (code.is_cpp())
    {
        code.Str("bool ").as_string(prop_class_name).Str("::Create(");

        // Note that with the exception of wxAui, all the MDI type windows require a wxFrame as the
        // parent, not a wxWindow.
        if (frame_type == frame_sdi_doc || frame_type == frame_mdi_doc)
        {
            code.Str("wxDocManager* manager, wxFrame* parent").Comma();
        }
        else if (frame_type == frame_sdi_child || frame_type == frame_mdi_child)
        {
            code.Str("wxDocManager* manager, wxView* view, wxFrame* parent").Comma();
        }
        else
        {
            code.Str("wxWindow* parent").Comma();
        }
        code += "wxWindowID id, const wxString& title,\n\tconst wxPoint& pos, const wxSize& size, "
                "long style, const wxString &name)";
        code.OpenBrace();

        if (code.HasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed)
                .FormFunction("SetExtraStyle(GetExtraStyle() | ")
                .Add(prop_extra_style);
            code.EndFunction();
        }
    }
    else if (code.is_perl())
    {
        code += "sub new {";
        code.Indent();
        code.Eol().Str("my ($class");
        if (frame_type == frame_sdi_doc || frame_type == frame_mdi_doc)
        {
            code.Str("$manager").Comma();
        }
        else if (frame_type == frame_sdi_child || frame_type == frame_mdi_child)
        {
            code.Str("$manager, $view").Comma();
        }
        code.Comma().Str("$parent, $id, $title, $pos, $size, $style, $name) = @_;");
        code.Eol() += "$parent = undef unless defined $parent;";
        code.Eol().Str("$id = ").as_string(prop_id).Str(" unless defined $id;");
        code.Eol().Str("$title = ").QuotedString(prop_title).Str(" unless defined $title;");
        code.Eol().Str("$pos = ").Pos().Str(" unless defined $pos;");
        code.Eol().Str("$size = ").WxSize(prop_size).Str(" unless defined $size;");
        code.Eol().Str("$style = ").Style().Str(" unless defined $style;");

        code.Eol().Str("$name = ");
        if (code.HasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code += "\"frame\"";
        code.Str(" unless defined $name;");
    }
    else if (code.is_python())
    {
        // https://docs.wxpython.org/wx.lib.docview.DocMDIParentFrame.html
        // https://docs.wxpython.org/wx.lib.docview.DocParentFrame.html
        code.Add("class ").NodeName();
        if (frame_type == frame_aui)
            code.Str("wx.aui.AuiMDIParentFrame):\n");
        else
            code.Str("(wx.Frame):\n");
        code.Eol().Tab().Add("def __init__(self, ");
        if (frame_type == frame_sdi_doc || frame_type == frame_mdi_doc)
        {
            code.Str("manager").Comma();
        }
        else if (frame_type == frame_sdi_child || frame_type == frame_mdi_child)
        {
            code.Str("manager, view").Comma();
        }
        code.Str("parent, id=").as_string(prop_id);
        code.Indent(3);
        code.Comma().Str("title=").QuotedString(prop_title).Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size, code::no_scaling);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() +
                                     4);
        code.Add("style=").Style().Comma();
        size_t name_len = code.HasValue(prop_window_name) ?
                              code.node()->as_string(prop_window_name).size() :
                              sizeof("wx.FrameNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.HasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code.Str("wx.FrameNameStr");
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.Frame.__init__(self)";
    }
    else if (code.is_ruby())
    {
        if (frame_type == frame_sdi_doc)
        {
            code.AddComment("wxDocParentFrame is not currently supported in wxRuby3. Generating a "
                            "wxFrame instead.",
                            true);
        }
        else if (frame_type == frame_mdi_doc)
        {
            code.AddComment("wxDocMDIParentFrame is not currently supported in wxRuby3. Generating "
                            "a wxFrame instead.",
                            true);
        }
        code.Add("class ").NodeName();
        if (frame_type == frame_aui)
            code.Str(" < Wx::AUI::AuiMDIParentFrame").Eol();
        else
            code.Str(" < Wx::Frame").Eol();
        code.AddPublicRubyMembers();
        code.Eol(eol_if_needed).Tab().Str("def initialize(parent");
        // Indent any wrapped lines
        code.Indent(3);
        code.Str(", id = ");
        if (code.HasValue(prop_id))
        {
            code.Add(prop_id);
        }
        else
        {
            code.Add("Wx::ID_ANY");
        }
        code.Comma().Str("title = ").QuotedString(prop_title);
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
            .CheckLineLength(sizeof("style = Wx::DEFAULT_FRAME_STYLE"))
            .Str("style = ")
            .Style();
        if (code.HasValue(prop_window_name))
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
    else
    {
        code.AddComment("Unknown language", true);
    }

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool FrameCommon::SettingsCode(Code& code, int frame_type)
{
    if (!code.node()->is_PropValue(prop_variant, "normal"))
    {
        code.Eol(eol_if_empty).FormFunction("SetWindowVariant(");
        if (code.node()->is_PropValue(prop_variant, "small"))
            code.Add("wxWINDOW_VARIANT_SMALL");
        else if (code.node()->is_PropValue(prop_variant, "mini"))
            code.Add("wxWINDOW_VARIANT_MINI");
        else
            code.Add("wxWINDOW_VARIANT_LARGE");

        code.EndFunction();
    }

    if (code.is_cpp())
    {
        if (auto icon_code = GenerateIconCode(code.node()->as_string(prop_icon)); icon_code.size())
        {
            code += icon_code;
            code.Eol();
        }
    }
    else
    {
        // TODO: [Randalphwa - 12-31-2022] Add Python and Ruby code for setting icon
    }

    if (isScalingEnabled(code.node(), prop_pos) || isScalingEnabled(code.node(), prop_size))
    {
        code.AddComment("Don't scale pos and size until after the window has been created.");
    }

    if (code.is_cpp())
    {
        code.Eol(eol_if_needed) += "if (!";
        if (code.node()->HasValue(prop_subclass))
            code.as_string(prop_subclass);
        else
            code.Class(code.node()->get_DeclName());
        code += "::Create(";
        if (code.node()->HasValue(prop_subclass_params))
        {
            code += code.node()->as_string(prop_subclass_params);
            code.RightTrim();
            if (code.back() != ',')
                code.Comma();
            else
                code += ' ';
        }
        if (frame_type == frame_sdi_doc || frame_type == frame_mdi_doc)
        {
            code.Str("manager").Comma();
        }
        else if (frame_type == frame_sdi_child || frame_type == frame_mdi_child)
        {
            code.Str("manager, view").Comma();
        }
        code += "parent, id, title, pos, size, style, name))";
        code.Eol().OpenBrace().Str("return false;").CloseBrace().Eol(eol_always);
    }
    else if (code.is_python())
    {
        code.Eol(eol_if_needed)
            .Str("if not self.Create(parent, id, title, pos, size, style, name):");
        code.Eol().Tab().Str("return\n");
    }
    else if (code.is_ruby())
    {
        code.Eol(eol_if_needed).Str("super(parent, id, title, pos, size, style)\n");
    }
    else if (code.is_perl())
    {
        code.Eol(eol_if_needed) +=
            "my $self = $class->SUPER::new($parent, $id, $title, $pos, $size, $style, $name);";
    }
    else
    {
        return false;
    }

    if (isScalingEnabled(code.node(), prop_pos, code.get_language()) ||
        isScalingEnabled(code.node(), prop_size, code.get_language()))
    {
        code.Eol(eol_if_needed).BeginConditional();
        if (code.is_perl())
        {
            code.Str("$pos != ").AddConstant("wxDefaultPosition").AddConditionalOr();
            code.Str("$size != ").AddConstant("wxDefaultSize").EndConditional().OpenBrace(true);
            code.Str("my $dip_pos = $self->FromDIP->new($pos);").Eol();
            code.Str("my $dip_size = $self->FromDIP->new($size);").Eol();
            code.Str("$self->SetSize($dip_pos->x, $dip_pos->y, $dip_size->x, $dip_size->y,").Eol();
            code.Tab().Str("wxSIZE_USE_EXISTING);").Eol();
        }
        else
        {
            code.Str("pos != ").AddConstant("wxDefaultPosition").AddConditionalOr();
            code.Str("size != ").AddConstant("wxDefaultSize").EndConditional().OpenBrace(true);
            code.FormFunction("SetSize(");
            code.FormFunction("FromDIP(pos).x")
                .Comma()
                .FormFunction("FromDIP(pos).y")
                .Comma()
                .Eol();
            code.FormFunction("FromDIP(size).x").Comma().FormFunction("FromDIP(size).y").Comma();
            code.Add("wxSIZE_USE_EXISTING").EndFunction();
        }
        code.CloseBrace(true);
    }

    Node* frame = code.node();
    const auto min_size = frame->as_wxSize(prop_minimum_size);
    const auto max_size = frame->as_wxSize(prop_maximum_size);
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

    if (code.HasValue(prop_window_extra_style))
    {
        code.Eol(eol_if_needed).FormFunction("SetExtraStyle(").FormFunction("GetExtraStyle");
        if (!code.is_ruby())
        {
            // In Ruby, don't add () to the end of a function call if there are no parameters.
            code.Str("()");
        }
        code.Str(" | ").Add(prop_window_extra_style).EndFunction();
    }

    return true;
}

bool FrameCommon::AfterChildrenCode(Code& code, int /* frame_type */)
{
    Node* form = code.node();
    if (form->get_ChildCount())
    {
        bool is_focus_set = false;
        auto SetChildFocus = [&](Node* child, auto&& SetChildFocus) -> void
        {
            if (child->HasProp(prop_focus))
            {
                if (child->as_bool(prop_focus))
                {
                    code.NodeName(child).Function("SetFocus(").EndFunction();
                    is_focus_set = true;
                    return;
                }
            }
            else if (child->get_ChildCount())
            {
                for (auto& iter: child->get_ChildNodePtrs())
                {
                    SetChildFocus(iter.get(), SetChildFocus);
                    if (is_focus_set)
                        return;
                }
            }
        };

        for (auto& iter: form->get_ChildNodePtrs())
        {
            SetChildFocus(iter.get(), SetChildFocus);
            if (is_focus_set)
            {
                code.Eol();
                break;
            }
        }
    }

    auto& center = code.node()->as_string(prop_center);
    if (center.size() && !center.is_sameas("no"))
    {
        code.Eol(eol_if_needed).FormFunction("Centre(").AddConstant(center).EndFunction();
    }

    return true;
}

bool FrameCommon::HeaderCode(Code& code, int frame_type)
{
    auto* node = code.node();
    code.NodeName() += "() = default;";
    code.Eol().NodeName().Str("(");
    if (frame_type == frame_sdi_doc || frame_type == frame_mdi_doc)
    {
        // Since manager has to be supplied, parent can default to nullptr
        code.Str("wxDocManager* manager, wxFrame* parent = nullptr");
    }
    else if (frame_type == frame_sdi_child || frame_type == frame_mdi_child)
    {
        // Since manager has to be supplied, parent can default to nullptr
        code.Str("wxDocManager* manager, wxView* view, wxFrame* parent");
    }
    else if (frame_type == frame_aui_child)
    {
        // Since manager has to be supplied, parent can default to nullptr
        code.Str("wxAuiMDIParentFrame* manager");
    }
    else
    {
        // If this is the first parameter, then we can't use nullptr as the default because then
        // wxFrame() could be the empty ctor, or the one that calls Create().
        code.Str("wxWindow* parent");
    }
    code.Comma().Str("wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxString& title = ");
    auto& title = node->as_string(prop_title);
    if (code.HasValue(prop_title))
    {
        code.QuotedString(title);
    }
    else
    {
        code.Str("wxEmptyString");
    }
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
        code.Comma().Str("const wxString &name = wxFrameNameStr");
    }

    code.Str(")").Eol().OpenBrace().Str("Create(");
    if (frame_type == frame_sdi_doc || frame_type == frame_mdi_doc)
    {
        code.Str("manager").Comma();
    }
    else if (frame_type == frame_sdi_child || frame_type == frame_mdi_child)
    {
        code.Str("manager, view").Comma();
    }
    code.Str("parent, id, title, pos, size, style, name);").CloseBrace();

    code.Eol().Str("bool Create(");
    if (frame_type == frame_sdi_doc || frame_type == frame_mdi_doc)
    {
        code.Str("wxDocManager* manager, wxFrame* parent = nullptr");
    }
    else if (frame_type == frame_sdi_child || frame_type == frame_mdi_doc)
    {
        code.Str("wxDocManager* manager, wxView* view, wxFrame* parent");
    }
    else
    {
        code.Str("wxWindow* parent");
    }
    code.Comma().Str("wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxString& title = ").QuotedString(prop_title);
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
    if (node->HasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxFrameNameStr");

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool FrameCommon::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += code.node()->get_DeclName();
    }

    return true;
}

bool FrameCommon::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_extra_style))
    {
        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
            return true;

        if (newValue.contains("wxFRAME_EX_CONTEXTHELP"))
        {
            auto& style = node->as_string(prop_style);
            if (style.contains("wxDEFAULT_FRAME_STYLE") || style.contains("wxMINIMIZE_BOX") ||
                style.contains("wxMINIMIZE_BOX"))
            {
                event->SetValidationFailureMessage(
                    "You can't add a context help button if there is a minimize or maximize button "
                    "(wxDEFAULT_FRAME_STYLE contains these).");
                event->Veto();
                return false;
            }
        }
    }

    return true;
}

bool FrameCommon::GetImports(Node* node, std::set<std::string>& set_imports, GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use base qw[Wx::Frame];");
        set_imports.emplace("use Wx qw[:frame];");
        set_imports.emplace("use Wx qw[:misc];");  // for wxDefaultPosition and wxDefaultSize

        if (auto qw_events = GatherPerlNodeEvents(node); qw_events.size())
        {
            set_imports.emplace(qw_events);
        }

        return true;
    }

    return false;
}
