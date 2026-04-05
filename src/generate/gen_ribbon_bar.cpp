/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonBar -- form and regular
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar

#include "assertion_dlg.h"
#include "code.h"                        // Code -- Helper class for generating code
#include "gen_common.h"                  // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"               // Common XRC generating functions
#include "mockup_parent.h"               // Top-level MockUp Parent window
#include "node.h"                        // Node class
#include "utils.h"                       // Utility functions that work with properties
#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "gen_ribbon_bar.h"

// Ribbon theme property values (match prop_theme strings from .wxui project files)
static constexpr std::string_view THEME_RIBBON_DEFAULT { "Default" };
static constexpr std::string_view THEME_RIBBON_GENERIC { "Generic" };  // AUI art provider
static constexpr std::string_view THEME_RIBBON_MSW { "MSW" };          // MSW art provider

wxObject* RibbonBarFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget =
        new wxRibbonBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                        DlgSize(node, prop_size), GetStyleInt(node));

    const wxue::string& theme = node->as_string(prop_theme);
    if (theme.is_sameas(THEME_RIBBON_DEFAULT))
    {
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    }
    else if (theme.is_sameas(THEME_RIBBON_GENERIC))
    {
        widget->SetArtProvider(new wxRibbonAUIArtProvider);
    }
    else if (theme.is_sameas(THEME_RIBBON_MSW))
    {
        widget->SetArtProvider(new wxRibbonMSWArtProvider);
    }
    else
    {
        FAIL_MSG("Unknown ribbon theme: " + theme);
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    }
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/,
                                           Node* /* node */, bool /* is_preview */)
{
    wxRibbonBar* btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarFormGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    const wxRibbonBar* ribbon_bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (ribbon_bar)
    {
        MockupParent* mockup_parent = getMockup();
        ASSERT_MSG(mockup_parent,
                   "RibbonBarGenerator::OnPageChanged() -- getMockup() returned nullptr");
        if (mockup_parent)
        {
            mockup_parent->SelectNode(event.GetPage());
        }
    }
    event.Skip();
}

bool RibbonBarFormGenerator::ConstructionCode(Code& code)
{
    // Note: Form construction is called before any indentation is set
    if (code.is_cpp())
    {
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code += "(wxWindow* parent, wxWindowID id";
        code.Comma().Str("const wxPoint& pos").Comma().Str("const wxSize& size");
        code.Comma().Str("long style)");
        code.Str(" : wxRibbonBar(parent, id, pos, size, style)").Eol() += "{";
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.RibbonBar):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").as_string(prop_id);
        // TODO: [Randalphwa - 04-04-2026] Figure out why we are indenting 3 in all languages except
        // C++. If this is valid, change 3 to a named constant so anyone reading the code will
        // understand why we are indenting 3.
        code.Indent(3);
        code.Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() +
                                     4);
        code.Add("style=").Style();
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.RibbonBar.__init__(self, parent, id, pos, size, style)";
    }

    code.ResetIndent();

    return true;
}

bool RibbonBarFormGenerator::AfterChildrenCode(Code& code)
{
    code.FormFunction("Realize(").EndFunction();

    return true;
}

bool RibbonBarFormGenerator::HeaderCode(Code& code)
{
    const Node* node = code.node();

    code.NodeName().Str("(wxWindow* parent, wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxPoint& pos = ");

    const wxPoint position = node->as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
    {
        code.Str("wxDefaultPosition");
    }
    else
    {
        code.Pos(prop_pos, no_dpi_scaling);
    }

    code.Comma().Str("const wxSize& size = ");

    const wxSize size = node->as_wxSize(prop_size);
    if (size == wxDefaultSize)
    {
        code.Str("wxDefaultSize");
    }
    else
    {
        code.WxSize(prop_size, no_dpi_scaling);
    }

    const wxue::string& style = node->as_string(prop_style);
    const wxue::string& win_style = node->as_string(prop_window_style);
    if (style.empty() && win_style.empty())
    {
        code.Comma().Str("long style = 0");
    }
    else
    {
        code.Comma();
        code.CheckLineLength(style.size() + win_style.size() + sizeof("long style = "));
        code.Str("long style = ");
        if (!style.empty())
        {
            code.CheckLineLength(style.size() + win_style.size());
            code += style;
            if (!win_style.empty())
            {
                code << '|' << win_style;
            }
        }
        else if (!win_style.empty())
        {
            code.Str(win_style);
        }
    }

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool RibbonBarFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += "wxRibbonBar";
    }

    return true;
}

bool RibbonBarFormGenerator::SettingsCode(Code& code)
{
    const wxue::string& theme = code.node()->as_string(prop_theme);
    if (theme.is_sameas(THEME_RIBBON_DEFAULT))
    {
        code.FormFunction("SetArtProvider(")
            .Str(code.is_cpp() ? "new " : "")
            .Add("wxRibbonDefaultArtProvider");
        code.AddIfPython("()");
    }
    else if (theme.is_sameas(THEME_RIBBON_GENERIC))
    {
        code.FormFunction("SetArtProvider(")
            .Str(code.is_cpp() ? "new " : "")
            .Add("wxRibbonAUIArtProvider");
        code.AddIfPython("()");
    }
    else if (theme.is_sameas(THEME_RIBBON_MSW))
    {
        code.FormFunction("SetArtProvider(")
            .Str(code.is_cpp() ? "new " : "")
            .Add("wxRibbonMSWArtProvider");
        code.AddIfPython("()");
    }
    else
    {
        FAIL_MSG("Unknown ribbon theme: " + theme);
        return false;
    }
    code.EndFunction();

    return true;
}

void RibbonBarFormGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    BaseGenerator::GenEvent(code, event, class_name);

    // Since this is the base class, we don't want to use the pointer that GenEventCode() would
    // normally create
    code.Replace(wxue::string() << event->getNode()->as_string(prop_var_name) << "->", "");
}

bool RibbonBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                         std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/art.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/bar.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/control.h>", set_src, set_hdr);

    return true;
}

bool RibbonBarFormGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                        GenLang language)
{
    if (language == GEN_LANG_RUBY)
    {
        set_imports.insert("require 'wx/rbn'");
        return true;
    }
    return false;
}

//////////////////////////////////////////  RibbonBarGenerator
/////////////////////////////////////////////

wxObject* RibbonBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget =
        new wxRibbonBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                        DlgSize(node, prop_size), GetStyleInt(node));

    const wxue::string& theme = node->as_string(prop_theme);
    if (theme.is_sameas(THEME_RIBBON_DEFAULT))
    {
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    }
    else if (theme.is_sameas(THEME_RIBBON_GENERIC))
    {
        widget->SetArtProvider(new wxRibbonAUIArtProvider);
    }
    else if (theme.is_sameas(THEME_RIBBON_MSW))
    {
        widget->SetArtProvider(new wxRibbonMSWArtProvider);
    }
    else
    {
        FAIL_MSG(wxue::string("Unknown ribbon theme: ") << theme);
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    }

    widget->Bind(wxEVT_RIBBONBAR_PAGE_CHANGED, &RibbonBarGenerator::OnPageChanged, this);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */,
                                       bool /* is_preview */)
{
    wxRibbonBar* btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    const wxRibbonBar* ribbon_bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (ribbon_bar)
    {
        MockupParent* mockup_parent = getMockup();
        ASSERT_MSG(mockup_parent,
                   "RibbonBarGenerator::OnPageChanged() -- getMockup() returned nullptr");
        if (mockup_parent)
        {
            mockup_parent->SelectNode(event.GetPage());
        }
    }

    event.Skip();
}

bool RibbonBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, false, "wxRIBBON_BAR_DEFAULT_STYLE");

    return true;
}

bool RibbonBarGenerator::SettingsCode(Code& code)
{
    const wxue::string& theme = code.node()->as_string(prop_theme);
    if (theme.is_sameas(THEME_RIBBON_DEFAULT))
    {
        code.Eol()
            .NodeName()
            .Function("SetArtProvider(")
            .Str(code.is_cpp() ? "new " : "")
            .Add("wxRibbonDefaultArtProvider");
        code.AddIfRuby(".new").AddIfPython("()");
    }
    else if (theme.is_sameas(THEME_RIBBON_GENERIC))
    {
        code.Eol()
            .NodeName()
            .Function("SetArtProvider(")
            .Str(code.is_cpp() ? "new " : "")
            .Add("wxRibbonAUIArtProvider");
        code.AddIfRuby(".new").AddIfPython("()");
    }
    else if (theme.is_sameas(THEME_RIBBON_MSW))
    {
        code.Eol()
            .NodeName()
            .Function("SetArtProvider(")
            .Str(code.is_cpp() ? "new " : "")
            .Add("wxRibbonMSWArtProvider");
        code.AddIfRuby(".new").AddIfPython("()");
    }
    else
    {
        FAIL_MSG(wxue::string("Unknown ribbon theme: ") << theme);
        return false;
    }
    code.EndFunction();

    return true;
}

bool RibbonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/art.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/bar.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/control.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_ribbon.cpp
// ../../../wxWidgets/src/xrc/xh_ribbon.cpp
// See Handle_bar()

int RibbonBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    const int result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                        BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonBar");

    wxue::string art_provider(node->as_string(prop_theme));
    if (art_provider == THEME_RIBBON_GENERIC)
    {
        art_provider = "aui";
    }
    else if (art_provider == THEME_RIBBON_MSW)
    {
        art_provider = "msw";
    }
    else
    {
        art_provider = "default";
    }

    item.append_child("art-provider").text().set(art_provider);

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void RibbonBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxRibbonXmlHandler");
}

bool RibbonBarGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                    GenLang language)
{
    if (language == GEN_LANG_RUBY)
    {
        set_imports.insert("require 'wx/rbn'");
        return true;
    }
    return false;
}
