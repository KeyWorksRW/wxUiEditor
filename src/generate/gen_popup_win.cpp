/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPopupWindow/wxPopupTransientWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-04-2026]

#include <wx/popupwin.h>  // wxBitmapComboBox base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_popup_win.h"

static void PopupFfiConstructionCodeFortran(Code& code)
{
    // module NodeName_mod
    //     use kwx_fortran
    //     implicit none
    //     type(wx_popup_window_t) :: self
    //         ...or type(wx_popup_transient_window_t) :: self
    // contains
    // subroutine create(parent)
    //     type(c_ptr), intent(in) :: parent

    const std::string_view decl_name = code.node()->get_DeclName();
    const std::string_view fortran_type =
        decl_name.contains("Transient") ? "wx_popup_transient_window_t" : "wx_popup_window_t";

    code.Str("module ").NodeName().Str("_mod").Eol();
    code.Tab().Str("use kwx_fortran").Eol();
    code.Tab().Str("implicit none").Eol();
    code.Tab().Str("type(").Str(fortran_type).Str(") :: self").Eol();
    code.Eol();
    code.Str("contains").Eol();
    code.Eol();
    code.Str("subroutine create(parent)").Eol();
    code.Tab().Str("type(c_ptr), intent(in) :: parent");
}

static void PopupFfiConstructionCodeGo(Code& code)
{
    // type NodeName struct {
    //     popup *wx.PopupWindow
    //         ...or popup *wx.PopupTransientWindow
    // }
    //
    // func NewNodeName(parent wx.Pointer) *NodeName {
    //     self := &NodeName{}

    const std::string_view decl_name = code.node()->get_DeclName();
    const std::string_view go_type =
        decl_name.contains("Transient") ? "*wx.PopupTransientWindow" : "*wx.PopupWindow";

    code.Str("type ").NodeName().Str(" struct {").Eol();
    code.Tab().Str("popup ").Str(go_type).Eol();
    code.Str("}").Eol();
    code.Eol();
    code.Str("func New").NodeName().Str("(parent wx.Pointer) *").NodeName().Str(" {").Eol();
    code.Tab().Str("self := &").NodeName().Str("{}");
}

static void PopupFfiConstructionCodeJulia(Code& code)
{
    // mutable struct NodeName
    //     frame::Ptr{Cvoid}
    //
    //     function NodeName(parent=nothing)
    //         self = new()

    code.Str("mutable struct ").NodeName().Eol();
    code.Indent();
    code.Tab().Str("frame::Ptr{Cvoid}").Eol();
    code.Eol();
    code.Tab().Str("function ").NodeName().Str("(parent=nothing)").Eol();
    code.Indent();
    code.Tab().Str("self = new()");
}

static void PopupFfiConstructionCodeLuaJIT(Code& code)
{
    // local NodeName = {}
    // NodeName.__index = NodeName
    //
    // function NodeName:new(parent)
    //     local self = setmetatable({}, NodeName)

    code.Str("local ").NodeName().Str(" = {}").Eol();
    code.NodeName().Str(".__index = ").NodeName().Eol();
    code.Eol();
    code.Str("function ").NodeName().Str(":new(parent)").Eol();
    code.Tab().Str("local self = setmetatable({}, ").NodeName().Str(")");
}

static void PopupFfiConstructionCodeTypeScript(Code& code)
{
    // TypeScript popup window creation inside the constructor body.
    // Popup windows take (parent, style) -- no title, pos, or size.

    const LanguageTraits& traits = code.get_traits();
    const std::string_view decl_name = code.node()->get_DeclName();
    const std::string_view ts_class =
        decl_name.contains("Transient") ? "wxPopupTransientWindow" : "wxPopupWindow";

    code.Tab()
        .Str(traits.self_reference)
        .Str(".frame = ")
        .Str(ts_class)
        .Str(".Create(")
        .Eol()
        .Tab(2);
    code.Str("parent").Comma().Eol().Tab(2);

    // Style: border combined with any optional style flags.
    code.Add(prop_border);
    if (code.HasValue(prop_style))
    {
        code.Str(" | ").Add(prop_style);
    }
    code.Str(")!");
    code.Str(traits.stmt_end);
}

bool PopupWinGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code += "(wxWindow* parent, int style) : wxPopupWindow(parent, style)\n{";
    }
    else if (code.is_python())
    {
        code.Add("class ").NodeName().Add("(wx.PopupWindow):\n");
        code.Tab().Add("def __init__(self, parent):").Eol().Tab(2);
        code << "wx.PopupWindow.__init__(self, parent, flags=";
        code.Add(prop_border);
        if (code.HasValue(prop_style))
        {
            code.Str(" | ").Add(prop_style);
        }
        code.EndFunction();
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::PopupWindow").Eol();
        code.AddPublicRubyMembers();
        code.Eol(eol_if_needed).Tab().Add("def initialize(parent, flags = ");
        // Indent any wrapped lines
        code.Indent(3);
        code.Add(prop_border);
        if (code.HasValue(prop_style))
        {
            code.Str(" | ").Add(prop_style);
        }
        code.EndFunction();
        code.ResetIndent();
    }
    else if (code.is_ffi())
    {
        switch (code.get_language())
        {
            case GenLang::fortran:
                PopupFfiConstructionCodeFortran(code);
                break;
            case GenLang::go:
                PopupFfiConstructionCodeGo(code);
                break;
            case GenLang::julia:
                PopupFfiConstructionCodeJulia(code);
                break;
            case GenLang::luajit:
                PopupFfiConstructionCodeLuaJIT(code);
                break;
            case GenLang::typescript:
                PopupFfiConstructionCodeTypeScript(code);
                break;
            default:
                code.AddComment("Unsupported FFI language", true);
                break;
        }
        code.ResetIndent();
        code.ResetBraces();
    }
    else
    {
        code.AddComment("Unknown language", true);
    }
    return true;
}

bool PopupTransientWinGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code += "(wxWindow* parent, int style) : wxPopupTransientWindow(parent, style)\n{";
    }
    else if (code.is_python())
    {
        code.Add("class ").NodeName().Add("(wx.PopupTransientWindow):\n");
        code.Tab().Add("def __init__(self, parent):").Eol().Tab(2);
        code << "wx.PopupTransientWindow.__init__(self, parent, flags=";
        code.Add(prop_border);
        if (code.HasValue(prop_style))
        {
            code.Str(" | ").Add(prop_style);
        }
        code.EndFunction();
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::PopupTransientWindow").Eol();
        code.AddPublicRubyMembers();
        code.Eol(eol_if_needed).Tab().Add("def initialize(parent, flags = ");
        // Indent any wrapped lines
        code.Indent(3);
        code.Add(prop_border);
        if (code.HasValue(prop_style))
        {
            code.Str(" | ").Add(prop_style);
        }
        code.EndFunction();
        code.ResetIndent();
    }
    else if (code.is_ffi())
    {
        switch (code.get_language())
        {
            case GenLang::fortran:
                PopupFfiConstructionCodeFortran(code);
                break;
            case GenLang::go:
                PopupFfiConstructionCodeGo(code);
                break;
            case GenLang::julia:
                PopupFfiConstructionCodeJulia(code);
                break;
            case GenLang::luajit:
                PopupFfiConstructionCodeLuaJIT(code);
                break;
            case GenLang::typescript:
                PopupFfiConstructionCodeTypeScript(code);
                break;
            default:
                code.AddComment("Unsupported FFI language", true);
                break;
        }
        code.ResetIndent();
        code.ResetBraces();
    }
    else
    {
        code.AddComment("Unknown language", true);
    }
    return true;
}

bool PopupWinGenerator::BaseClassNameCode(Code& code)
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

bool PopupTransientWinGenerator::BaseClassNameCode(Code& code)
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

int PopupWinGenerator::GenXrcObject([[maybe_unused]] Node* node, pugi::xml_node& object,
                                    size_t xrc_flags)
{
    if (xrc_flags & xrc::add_comments)
    {
        object.append_child(pugi::node_comment)
            .set_value(" wxPopupWindow is not supported by XRC. ");
    }
    return BaseGenerator::xrc_form_not_supported;
}

int PopupTransientWinGenerator::GenXrcObject([[maybe_unused]] Node* node, pugi::xml_node& object,
                                             size_t xrc_flags)
{
    if (xrc_flags & xrc::add_comments)
    {
        object.append_child(pugi::node_comment)
            .set_value(" wxPopupTransientWindow is not supported by XRC. ");
    }
    return BaseGenerator::xrc_form_not_supported;
}

/////////////////////////////// PopupWinBaseGenerator //////////////////////////////////////////////

bool PopupWinBaseGenerator::SettingsCode(Code& code)
{
    if (!code.node()->is_PropValue(prop_variant, "normal"))
    {
        code.Eol(eol_if_empty).FormFunction("SetWindowVariant(");
        if (code.node()->is_PropValue(prop_variant, "small"))
        {
            code.Add("wxWINDOW_VARIANT_SMALL");
        }
        else if (code.node()->is_PropValue(prop_variant, "mini"))
        {
            code.Add("wxWINDOW_VARIANT_MINI");
        }
        else
        {
            code.Add("wxWINDOW_VARIANT_LARGE");
        }

        code.EndFunction();
    }

    if (code.is_ruby())
    {
        code.Eol(eol_if_needed).Str("super(parent, flags)\n");
    }

    code.GenFontColourSettings();

    return true;
}

bool PopupWinBaseGenerator::AfterChildrenCode(Code& code)
{
    Node* form = code.node();
    Node* child_node = form;
    ASSERT_MSG(form->get_ChildCount(), "Trying to generate code for wxPopup with no children.")
    if (!form->get_ChildCount())
    {
        return false;  // empty popup window, so nothing to do
    }
    ASSERT_MSG(form->get_Child(0)->is_Sizer(), "Expected first child of wxPopup to be a sizer.");
    if (form->get_Child(0)->is_Sizer())
    {
        // If the first child is not a sizer, then child_node will still point to the dialog
        // node, which means the SetSizer...(child_node) calls below will generate invalid
        // code.
        child_node = form->get_Child(0);
    }

    const wxSize min_size = form->as_wxSize(prop_minimum_size);
    const wxSize max_size = form->as_wxSize(prop_maximum_size);

    if (min_size == wxDefaultSize && max_size == wxDefaultSize &&
        form->as_wxSize(prop_size) == wxDefaultSize)
    {
        // If is_scaling_enabled == false, then neither pos or size have high dpi scaling enabled
        code.FormFunction("SetSizerAndFit(").NodeName(child_node).EndFunction();
    }
    else
    {
        if (min_size != wxDefaultSize)
        {
            code.Eol()
                .FormFunction("SetMinSize(")
                .WxSize(prop_minimum_size, code::allow_scaling)
                .EndFunction();
        }
        if (max_size != wxDefaultSize)
        {
            code.Eol()
                .FormFunction("SetMaxSize(")
                .WxSize(prop_maximum_size, code::allow_scaling)
                .EndFunction();
        }

        // The default will be size == wxDefaultSize, in which case all we need to do is call
        // SetSizerAndFit(child_node)
        if (code.node()->as_wxSize(prop_size) == wxDefaultSize)
        {
            code.FormFunction("SetSizerAndFit(")
                .NodeName(child_node)
                .EndFunction()
                .CloseBrace(true, false);
        }
        else
        {
            if (code.is_ffi())
            {
                // FFI layer handles size and scaling differently; use SetSizerAndFit.
                code.FormFunction("SetSizerAndFit(").NodeName(child_node).EndFunction();
            }
            else
            {
                code.FormFunction("SetSizer(").NodeName(child_node).EndFunction();
                code.Eol().OpenBrace();

                const wxSize size = form->as_wxSize(prop_size);

                if (code.is_cpp())
                {
                    code.Add("wxSize size = { ").itoa(size.x).Add(", ").itoa(size.y).Add(" };");
                }
                else if (code.is_python())
                {
                    code.Add("size = (").itoa(size.x).Add(", ").itoa(size.y).Add(")");
                }
                else if (code.is_ruby())
                {
                    code.Add("size = Wx::Size.new(").itoa(size.x).Add(", ").itoa(size.y).Add(")");
                }

                // If size != wxDefaultSize, it's more complicated because either the width or the
                // height might still be set to wxDefaultCoord. In that case, we need to call Fit()
                // to calculate the missing dimension

                code.Eol().BeginConditional().Str("size.x == ").Add("wxDefaultCoord");
                code.AddConditionalOr().Str("size.y == ").Add("wxDefaultCoord");
                code.EndConditional().OpenBrace(true);
                code.AddComment("Use the sizer to calculate the missing dimension");
                code.FormFunction("Fit(").EndFunction();
                code.CloseBrace(true);
                code.Eol().FormFunction("SetSize(").FormFunction("FromDIP(size)").EndFunction();
                code.Eol().FormFunction("Layout(").EndFunction();
                code.CloseBrace(false);
            }
        }
    }

    bool is_focus_set = false;
    auto SetChildFocus = [&](Node* child, auto&& SetChildFocus) -> void
    {
        if (child->HasProp(prop_focus))
        {
            if (child->as_bool(prop_focus))
            {
                code.Eol().NodeName(child).Function("SetFocus(").EndFunction();
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
                {
                    return;
                }
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

    return true;
}

bool PopupWinBaseGenerator::HeaderCode(Code& code)
{
    code.NodeName().Str("(wxWindow* parent, int style = ").as_string(prop_border);
    if (code.HasValue(prop_style))
    {
        code.Str(" | ").Add(prop_style);
    }
    code.EndFunction();

    return true;
}

bool PopupWinBaseGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                        std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/popupwin.h>", set_src, set_hdr);
    return true;
}

std::pair<bool, wxue::string> PopupWinBaseGenerator::isLanguageVersionSupported(GenLang language)
{
    if (language == GenLang::none ||
        (language & (GenLang::cplusplus | GenLang::python | GenLang::ruby | GenLang::fortran |
                     GenLang::go | GenLang::julia | GenLang::luajit | GenLang::typescript)))
    {
        return { true, {} };
    }
    // TODO: [Randalphwa - 10-01-2024] At some point, other languages may have versions that support
    // these, in which case call Project.get_LangVersion()

    return { false, wxue::string()
                        << "wxPopupWindow and wxPopupTransientWindow are not supported by "
                        << GenLangToString(language) };
}
