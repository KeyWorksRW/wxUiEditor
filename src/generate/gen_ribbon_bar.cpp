/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonButtonBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar

#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_ribbon_bar.h"

wxObject* RibbonBarFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                  DlgSize(node, prop_size), GetStyleInt(node));

    if (node->as_string(prop_theme) == "Default")
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    else if (node->as_string(prop_theme) == "Generic")
        widget->SetArtProvider(new wxRibbonAUIArtProvider);
    else if (node->as_string(prop_theme) == "MSW")
        widget->SetArtProvider(new wxRibbonMSWArtProvider);

    widget->Bind(wxEVT_RIBBONBAR_PAGE_CHANGED, &RibbonBarFormGenerator::OnPageChanged, this);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */,
                                           bool /* is_preview */)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarFormGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    auto bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (bar)
        // BUGBUG: [Randalphwa - 06-12-2022] Don't use getMockup() if is_preview is true!
        getMockup()->SelectNode(event.GetPage());
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
        code.Indent(3);
        code.Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() + 4);
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
    auto* node = code.node();

    code.NodeName().Str("(wxWindow* parent, wxWindowID id = ").as_string(prop_id);
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

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool RibbonBarFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.hasValue(prop_subclass))
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
    auto& theme = code.node()->as_string(prop_theme);
    if (theme.is_sameas("Default"))
    {
        code.FormFunction("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonDefaultArtProvider");
        code.AddIfPython("()");
    }
    else if (theme.is_sameas("Generic"))
    {
        code.FormFunction("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonAUIArtProvider");
        code.AddIfPython("()");
    }
    else if (theme.is_sameas("MSW"))
    {
        code.FormFunction("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonMSWArtProvider");
        code.AddIfPython("()");
    }
    code.EndFunction();

    return true;
}

void RibbonBarFormGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    BaseGenerator::GenEvent(code, event, class_name);

    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.Replace(tt_string() << event->getNode()->as_string(prop_var_name) << "->", "");
}

bool RibbonBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                         GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/art.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/bar.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/control.h>", set_src, set_hdr);

    return true;
}

bool RibbonBarFormGenerator::GetImports(Node*, std::set<std::string>& set_imports, GenLang language)
{
    if (language == GEN_LANG_RUBY)
    {
        set_imports.insert("require 'wx/rbn'");
        return true;
    }
    else
    {
    }
    return false;
}

//////////////////////////////////////////  RibbonBarGenerator  //////////////////////////////////////////

wxObject* RibbonBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                  DlgSize(node, prop_size), GetStyleInt(node));

    if (node->as_string(prop_theme) == "Default")
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    else if (node->as_string(prop_theme) == "Generic")
        widget->SetArtProvider(new wxRibbonAUIArtProvider);
    else if (node->as_string(prop_theme) == "MSW")
        widget->SetArtProvider(new wxRibbonMSWArtProvider);

    widget->Bind(wxEVT_RIBBONBAR_PAGE_CHANGED, &RibbonBarGenerator::OnPageChanged, this);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */, bool /* is_preview */)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    auto bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (bar)
        // BUGBUG: [Randalphwa - 06-12-2022] Don't use getMockup() if is_preview is true!
        getMockup()->SelectNode(event.GetPage());
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
    auto& theme = code.node()->as_string(prop_theme);
    if (theme.is_sameas("Default"))
    {
        code.Eol().NodeName().Function("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonDefaultArtProvider");
        code.AddIfRuby(".new").AddIfPython("()");
    }
    else if (theme.is_sameas("Generic"))
    {
        code.Eol().NodeName().Function("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonAUIArtProvider");
        code.AddIfRuby(".new").AddIfPython("()");
    }
    else if (theme.is_sameas("MSW"))
    {
        code.Eol().NodeName().Function("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonMSWArtProvider");
        code.AddIfRuby(".new").AddIfPython("()");
    }
    code.EndFunction();

    return true;
}

bool RibbonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                     GenLang /* language */)
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
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonBar");

    tt_string art(node->as_string(prop_theme));
    if (art == "Generic")
        art = "aui";
    else if (art == "MSW")
        art = "msw";
    else
        art = "default";

    item.append_child("art-provider").text().set(art);

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

bool RibbonBarGenerator::GetImports(Node*, std::set<std::string>& set_imports, GenLang language)
{
    if (language == GEN_LANG_RUBY)
    {
        set_imports.insert("require 'wx/rbn'");
        return true;
    }
    else
    {
    }
    return false;
}
