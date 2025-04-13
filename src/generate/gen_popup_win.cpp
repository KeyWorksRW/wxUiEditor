/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPopupWindow/wxPopupTransientWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/popupwin.h>  // wxBitmapComboBox base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_popup_win.h"

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
        if (code.hasValue(prop_style))
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
        if (code.hasValue(prop_style))
        {
            code.Str(" | ").Add(prop_style);
        }
        code.EndFunction();
        code.ResetIndent();
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
        if (code.hasValue(prop_style))
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
        if (code.hasValue(prop_style))
        {
            code.Str(" | ").Add(prop_style);
        }
        code.EndFunction();
        code.ResetIndent();
    }
    else
    {
        code.AddComment("Unknown language", true);
    }
    return true;
}

bool PopupWinGenerator::BaseClassNameCode(Code& code)
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

bool PopupTransientWinGenerator::BaseClassNameCode(Code& code)
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

int PopupWinGenerator::GenXrcObject(Node*, pugi::xml_node& object, size_t xrc_flags)
{
    if (xrc_flags & xrc::add_comments)
    {
        object.append_child(pugi::node_comment).set_value(" wxPopupWindow is not supported by XRC. ");
    }
    return BaseGenerator::xrc_form_not_supported;
}

int PopupTransientWinGenerator::GenXrcObject(Node*, pugi::xml_node& object, size_t xrc_flags)
{
    if (xrc_flags & xrc::add_comments)
    {
        object.append_child(pugi::node_comment).set_value(" wxPopupTransientWindow is not supported by XRC. ");
    }
    return BaseGenerator::xrc_form_not_supported;
}

/////////////////////////////// PopupWinBaseGenerator //////////////////////////////////////////////

bool PopupWinBaseGenerator::SettingsCode(Code& code)
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
    ASSERT_MSG(form->isGen(gen_wxPopupWindow) || form->getChildCount(),
               "Trying to generate code for wxPopup with no children.")
    if (!form->getChildCount())
        return {};  // empty popup window, so nothing to do
    ASSERT_MSG(form->getChild(0)->isSizer(), "Expected first child of wxPopup to be a sizer.");
    if (form->getChild(0)->isSizer())
    {
        // If the first child is not a sizer, then child_node will still point to the dialog
        // node, which means the SetSizer...(child_node) calls below will generate invalid
        // code.
        child_node = form->getChild(0);
    }

    const auto min_size = form->as_wxSize(prop_minimum_size);
    const auto max_size = form->as_wxSize(prop_maximum_size);

    if (min_size == wxDefaultSize && max_size == wxDefaultSize && form->as_wxSize(prop_size) == wxDefaultSize)
    {
        // If is_scaling_enabled == false, then neither pos or size have high dpi scaling enabled
        code.FormFunction("SetSizerAndFit(").NodeName(child_node).EndFunction();
    }
    else
    {
        if (min_size != wxDefaultSize)
        {
            code.Eol().FormFunction("SetMinSize(").WxSize(prop_minimum_size, code::allow_scaling).EndFunction();
        }
        if (max_size != wxDefaultSize)
        {
            code.Eol().FormFunction("SetMaxSize(").WxSize(prop_maximum_size, code::allow_scaling).EndFunction();
        }

        // The default will be size == wxDefaultSize, in which case all we need to do is call
        // SetSizerAndFit(child_node)
        if (code.node()->as_wxSize(prop_size) == wxDefaultSize)
        {
            code.FormFunction("SetSizerAndFit(").NodeName(child_node).EndFunction().CloseBrace(true, false);
        }
        else
        {
            code.FormFunction("SetSizer(").NodeName(child_node).EndFunction();
            code.Eol().OpenBrace();

            const auto size = form->as_wxSize(prop_size);

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

            // If size != wxDefaultSize, it's more complicated because either the width or the height might still
            // be set to wxDefaultCoord. In that case, we need to call Fit() to calculate the missing dimension

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

    bool is_focus_set = false;
    auto SetChildFocus = [&](Node* child, auto&& SetChildFocus) -> void
    {
        if (child->hasProp(prop_focus))
        {
            if (child->as_bool(prop_focus))
            {
                code.Eol().NodeName(child).Function("SetFocus(").EndFunction();
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

    return true;
}

bool PopupWinBaseGenerator::HeaderCode(Code& code)
{
    code.NodeName().Str("(wxWindow* parent, int style = ").as_string(prop_border);
    if (code.hasValue(prop_style))
    {
        code.Str(" | ").Add(prop_style);
    }
    code.EndFunction();

    return true;
}

bool PopupWinBaseGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                        GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/popupwin.h>", set_src, set_hdr);
    return true;
}

std::pair<bool, tt_string> PopupWinBaseGenerator::isLanguageVersionSupported(GenLang language)
{
    if (language == GEN_LANG_NONE || (language & (GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON | GEN_LANG_RUBY)))
        return { true, {} };
    // TODO: [Randalphwa - 10-01-2024] At some point, other languages may have versions that support these,
    // in which case call Project.getLangVersion()

    return { false, tt_string() << "wxPopupWindow and wxPopupTransientWindow are not supported by "
                                << GenLangToString(language) };
}
