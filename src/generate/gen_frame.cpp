/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/frame.h>              // wxFrame class interface
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include <frozen/set.h>

#include "gen_base.h"    // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "node_prop.h"   // NodeProperty -- NodeProperty class
#include "pugixml.hpp"   // xml_object_range
#include "utils.h"       // Utility functions that work with properties
#include "write_code.h"  // WriteCode -- Write code to Scintilla or file

#include "gen_frame.h"

bool FrameFormGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.Str("bool ").as_string(prop_class_name);
        code +=
            "::Create(wxWindow* parent, wxWindowID id, const wxString& title,\n\tconst wxPoint& pos, const wxSize& size, "
            "long style, const wxString &name)";
        code.OpenBrace();

        if (code.hasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed).FormFunction("SetExtraStyle(GetExtraStyle() | ").Add(prop_extra_style);
            code.EndFunction();
        }
    }
    else if (code.is_python())
    {
        code.Add("class ").NodeName().Add("(wx.Frame):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").as_string(prop_id);
        code.Indent(3);
        code.Comma().Str("title=").QuotedString(prop_title).Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size, code::no_scaling);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() + 4);
        code.Add("style=").Style().Comma();
        size_t name_len =
            code.hasValue(prop_window_name) ? code.node()->as_string(prop_window_name).size() : sizeof("wx.FrameNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.hasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code.Str("wx.FrameNameStr");
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.Frame.__init__(self)";
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::Frame");
        code.Eol().Tab().Add("def initialize(parent");
        // Indent any wrapped lines
        code.Indent(3);
        code.Str(", id = ");
        if (code.hasValue(prop_id))
        {
            code.Add(prop_id);
        }
        else
        {
            code.Add("Wx::ID_ANY");
        }
        code.Comma().Str("title = ").QuotedString(prop_title);
        // We have to break these out in order to add the variable assignment (pos=, size=, etc.)
        code.Comma().CheckLineLength(sizeof("pos = Wx::DEFAULT_POSITION")).Str("pos = ").Pos(prop_pos);
        code.Comma().CheckLineLength(sizeof("size = Wx::DEFAULT_SIZE")).Str("size = ").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style = Wx::DEFAULT_FRAME_STYLE")).Str("style = ").Style();
        if (code.hasValue(prop_window_name))
        {
            code.Comma().CheckLineLength(sizeof("name = ") + code.as_string(prop_window_name).size() + 2);
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
        code += "sub new {";
        code.Indent();
        code.Eol() += "my ($class, $parent, $id, $title, $pos, $size, $style, $name) = @_;";
        code.Eol() += "$parent = undef unless defined $parent;";
        code.Eol().Str("$id = ").as_string(prop_id).Str(" unless defined $id;");
        code.Eol().Str("$title = ").QuotedString(prop_title).Str(" unless defined $title;");
        code.Eol().Str("$pos = ").Pos().Str(" unless defined $pos;");
        code.Eol().Str("$size = ").WxSize(prop_size).Str(" unless defined $size;");
        code.Eol().Str("$style = ").Style().Str(" unless defined $style;");

        code.Eol().Str("$name = ");
        if (code.hasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code += "\"frame\"";
        code.Str(" unless defined $name;");

        code.Eol().Eol() += "my $self = $class->SUPER::new($parent, $id, $title, $pos, $size, $style, $name);";
    }
    else if (code.is_rust())
    {
        code.Str("#[derive(Clone)]").Eol().Str("struct ").NodeName();
        code.OpenBrace();
        code.Str("base: wx::WeakRef<wx::Frame>").Eol();
        code.CloseBrace().Eol();
        code.Str("impl ").NodeName();
        code.OpenBrace();
        code.Str("fn new(");
        code.Str(
            "parent: &wx::Window, id: i32, title: &str, pos: wx::Point, size: wx::Size, style: i32, name: &str) -> Self");
        code.OpenBrace();
        code.Str("let frame = wx::Frame::builder(parent, id, title, pos, size, style, name).build();").Eol();
        return true;
    }
#if GENERATE_NEW_LANG_CODE
    else if (code.is_lua())
    {
        code.Eol().NodeName().Str(" = {}\n");
        code.Eol().Str("function ").NodeName().Str(":create(parent, id, title, pos, size, style, name)");
        code.Indent();
        code.Eol().Str("parent = parent or wx.NULL");
        code.Eol().Str("id = id or ").as_string(prop_id);
        code.Eol().Str("title = title or ").QuotedString(prop_title);
        code.Eol().Str("pos = pos or ").Pos(prop_pos);
        code.Eol().Str("size = size or ").WxSize(prop_size);
        code.Eol().Str("style = style or ").Style();
        code.Eol().Str("name = name or ");
        if (code.hasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code += "\"frame\"";
        code.Eol().Eol().Str("this = wx.wxFrame(parent, id, title, pos, size, style, name)");
    }
#endif

    else
    {
        code.AddComment("Unknown language", true);
    }

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool FrameFormGenerator::SettingsCode(Code& code)
{
#if GENERATE_NEW_LANG_CODE
    if (code.is_lua())
    {
        code.ResetIndent();
        code.ResetBraces();
    }
#endif
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
        if (code.node()->hasValue(prop_subclass))
            code.as_string(prop_subclass);
        else
            code += "wxFrame";
        code += "::Create(";
        if (code.node()->hasValue(prop_subclass_params))
        {
            code += code.node()->as_string(prop_subclass_params);
            code.RightTrim();
            if (code.back() != ',')
                code.Comma();
            else
                code += ' ';
        }
        code += "parent, id, title, pos, size, style, name))";
        code.Eol().Tab() += "return false;\n";
    }
    else if (code.is_python())
    {
        code.Eol(eol_if_needed).Str("if not self.Create(parent, id, title, pos, size, style, name):");
        code.Eol().Tab().Str("return\n");
    }
    else if (code.is_ruby())
    {
        code.Eol(eol_if_needed).Str("super(parent, id, title, pos, size, style)\n");
    }
#if GENERATE_NEW_LANG_CODE
    else if (code.is_lua())
    {
        // Lua doesn't check the result of creating the window
    }
#endif
    else
    {
        return false;
    }

    if (isScalingEnabled(code.node(), prop_pos, code.get_language()) ||
        isScalingEnabled(code.node(), prop_size, code.get_language()))
    {
        code.Eol(eol_if_needed).BeginConditional().Str("pos != ").AddConstant("wxDefaultPosition");
        code.AddConditionalOr().Str("size != ").AddConstant("wxDefaultSize");
        code.EndConditional().OpenBrace(true);
        code.FormFunction("SetSize(");
        code.FormFunction("FromDIP(pos).x").Comma().FormFunction("FromDIP(pos).y").Comma().Eol();
        code.FormFunction("FromDIP(size).x").Comma().FormFunction("FromDIP(size).y").Comma();
        code.Add("wxSIZE_USE_EXISTING").EndFunction();
        code.CloseBrace(true);
    }

    Node* frame = code.node();
    const auto min_size = frame->as_wxSize(prop_minimum_size);
    const auto max_size = frame->as_wxSize(prop_maximum_size);
    if (min_size != wxDefaultSize)
    {
        code.Eol().FormFunction("SetMinSize(").WxSize(prop_minimum_size, code::force_scaling).EndFunction();
    }
    if (max_size != wxDefaultSize)
    {
        code.Eol().FormFunction("SetMaxSize(").WxSize(prop_maximum_size, code::force_scaling).EndFunction();
    }

    if (code.hasValue(prop_window_extra_style))
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

bool FrameFormGenerator::AfterChildrenCode(Code& code)
{
    Node* form = code.node();
    if (form->getChildCount())
    {
        bool is_focus_set = false;
        auto SetChildFocus = [&](Node* child, auto&& SetChildFocus) -> void
        {
            if (child->hasProp(prop_focus))
            {
                if (child->as_bool(prop_focus))
                {
                    code.NodeName(child).Function("SetFocus(").EndFunction();
                    is_focus_set = true;
                    return;
                }
            }
            else if (child->getChildCount())
            {
                for (auto& iter: child->getChildNodePtrs())
                {
                    SetChildFocus(iter.get(), SetChildFocus);
                    if (is_focus_set)
                        return;
                }
            }
        };

        for (auto& iter: form->getChildNodePtrs())
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

    if (code.is_rust())
    {
        code.Eol(eol_if_needed).NodeName();
        code.OpenBrace().Str("base: frame.to_weak_ref()").CloseBrace();
    }

    return true;
}

bool FrameFormGenerator::HeaderCode(Code& code)
{
    auto* node = code.node();
    code.NodeName() += "() {}";
    code.Eol().NodeName().Str("(wxWindow* parent, wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxString& title = ");
    auto& title = node->as_string(prop_title);
    if (code.hasValue(prop_title))
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

    code.Str(")").Eol().OpenBrace().Str("Create(parent, id, title, pos, size, style, name);").CloseBrace();

    code.Eol().Str("bool Create(wxWindow *parent");
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
    if (node->hasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxFrameNameStr");

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool FrameFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.hasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += code.node()->declName();
    }

    return true;
}

int FrameFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // We use item so that the macros in base_generator.h work, and the code looks the same as other
    // widget XRC generatorsl
    auto item = object;

    GenXrcObjectAttributes(node, item, "wxFrame");
    if (!node->isPropValue(prop_variant, "normal"))
    {
        ADD_ITEM_PROP(prop_variant, "variant")
    }
    ADD_ITEM_PROP(prop_title, "title")

    if (node->hasValue(prop_center))
    {
        if (node->as_string(prop_center) == "wxVERTICAL" || node->as_string(prop_center) == "wxHORIZONTAL" ||
            node->as_string(prop_center) == "wxBOTH")
        {
            object.append_child("centered").text().set(1);
        }
        else
        {
            object.append_child("centered").text().set(0);
        }
    }
    if (node->hasValue(prop_icon))
    {
        tt_string_vector parts(node->as_string(prop_icon), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            tt_string_vector art_parts(parts[IndexArtID], '|');
            auto icon = object.append_child("icon");
            icon.append_attribute("stock_id").set_value(art_parts[0]);
            icon.append_attribute("stock_client").set_value(art_parts[1]);
        }
        else
        {
            // REVIEW: [KeyWorks - 05-13-2022] As of wxWidgets 3.1.6, SVG files do not work here
            object.append_child("icon").text().set(parts[IndexImage]);
        }
    }

    GenXrcWindowSettings(node, object);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, object);

        if (node->as_string(prop_center) == "wxVERTICAL")
        {
            object.append_child(pugi::node_comment)
                .set_value((" For centering, you cannot set only one direction in the XRC file (set wxBOTH instead)."));
        }

        if (node->as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            object.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }

        if (node->as_bool(prop_persist))
        {
            object.append_child(pugi::node_comment).set_value(" persist is not supported in the XRC file. ");
        }
    }

    return xrc_updated;
}

void FrameFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxFrameXmlHandler");
    if (node->hasValue(prop_icon))
    {
        handlers.emplace("wxIconXmlHandler");
        handlers.emplace("wxBitmapXmlHandler");
    }
}

bool FrameFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                     GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/frame.h>", set_src, set_hdr);

    return true;
}

bool FrameFormGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
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

// defined in code.cpp
extern constexpr auto set_perl_constants = frozen::make_set<std::string_view>;

bool FrameFormGenerator::GetImports(Node* node, std::set<std::string>& set_imports, GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        tt_string constants;

        auto set_constants = [&]()
        {
            if (constants.size())
            {
                // remove the leading space
                constants.erase(0, 1);
                constants.insert(0, "use Wx qw(");
                constants += ");";
                set_imports.emplace(constants);
                constants.clear();
            }
        };

        if (node->as_string(prop_style) == "wxDEFAULT_FRAME_STYLE")
            constants += " wxDEFAULT_FRAME_STYLE";
        if (node->as_string(prop_style).contains("wxCAPTION"))
            constants += " wxCAPTION";
        if (node->as_string(prop_style).contains("wxCLOSE_BOX"))
            constants += " wxCLOSE_BOX";
        if (node->as_string(prop_style).contains("wxFRAME_TOOL_WINDOW"))
            constants += " wxFRAME_TOOL_WINDOW";
        if (node->as_string(prop_style).contains("wxFRAME_NO_TASKBAR"))
            constants += " wxFRAME_NO_TASKBAR";
        if (node->as_string(prop_style).contains("wxFRAME_FLOAT_ON_PARENT"))
            constants += " wxFRAME_FLOAT_ON_PARENT";
        if (node->as_string(prop_style).contains("wxFRAME_SHAPED"))
            constants += " wxFRAME_SHAPED";
        if (node->as_string(prop_style).contains("wxICONIZE"))
            constants += " wxICONIZE";
        if (node->as_string(prop_style).contains("wxMINIMIZE_BOX"))
            constants += " wxMINIMIZE_BOX";
        if (node->as_string(prop_style).contains("wxMAXIMIZE_BOX"))
            constants += " wxMAXIMIZE_BOX";
        if (node->as_string(prop_style).contains("wxRESIZE_BORDER"))
            constants += " wxRESIZE_BORDER";
        if (node->as_string(prop_style).contains("wxSYSTEM_MENU"))
            constants += " wxSYSTEM_MENU";
        if (node->as_string(prop_style).contains("wxCLIP_CHILDREN"))
            constants += " wxCLIP_CHILDREN";
        if (node->as_string(prop_style).contains("wxSTAY_ON_TOP"))
            constants += " wxSTAY_ON_TOP";
        if (node->as_string(prop_extra_style).contains("wxFRAME_EX_CONTEXTHELP"))
            constants += " wxFRAME_EX_CONTEXTHELP";
        if (node->as_string(prop_extra_style).contains("wxFRAME_EX_METAL"))
            constants += " wxFRAME_EX_METAL";
        set_constants();

        if (node->as_string(prop_center) != "no")
            constants += " wxBOTH wxHORIZONTAL wxVERTICAL";
        set_constants();

        set_imports.emplace("use base qw(Wx::Frame);");
        return true;
    }

    return false;
}
