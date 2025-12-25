/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxHtmlWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/html/htmlwin.h>  // wxHtmlWindow class for parsing & displaying HTML

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_html_window.h"

auto HtmlWindowGenerator::CreateMockup(Node* node, wxObject* parent) -> wxObject*
{
    auto* widget =
        new wxHtmlWindow(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                         DlgSize(node, prop_size), GetStyleInt(node));

    if (node->as_int(prop_html_borders) >= 0)
    {
        widget->SetBorders(
            wxStaticCast(parent, wxWindow)->FromDIP(node->as_int(prop_html_borders)));
    }

    if (node->HasValue(prop_html_content))
    {
        widget->SetPage(node->as_wxString(prop_html_content));
    }
    else if (node->HasValue(prop_html_url))
    {
        // Just display a placeholder message instead of loading the actual page since we don't want
        // to spend a long time downloading the URL.
        wxString page_content;
        page_content << "Contents of<br>    " << node->as_string(prop_html_url)
                     << "<br>will be displayed here.";
        widget->SetPage(page_content);
    }
    else
    {
        widget->SetPage("<b>wxHtmlWindow</b><br/><br/>This is a dummy page.</body></html>");
    }

#if defined(_DEBUG)
    auto size = node->as_wxSize(prop_size);
    (void) size;
    auto* node_decl = node->get_NodeDeclaration();
    (void) node_decl;
    auto name = node_decl->get_DeclName();
    (void) name;
#endif  // _DEBUG

    if (node->as_wxSize(prop_size) == wxDefaultSize &&
        node->as_wxSize(GenEnum::prop_minimum_size) == wxDefaultSize)
    {
        widget->SetMinSize(wxSize(160, 60));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

auto HtmlWindowGenerator::ConstructionCode(Code& code) -> bool
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, false, "wxHW_SCROLLBAR_AUTO");

    // If the last parameter is wxID_ANY, then remove it. This is the default value, so it's
    // not needed.
    code.Replace(", wxID_ANY)", ")");

    return true;
}

auto HtmlWindowGenerator::SettingsCode(Code& code) -> bool
{
    if (code.IntValue(prop_html_borders) >= 0)
    {
        code.Eol(eol_if_needed).NodeName().Function("SetBorders(").BorderSize(prop_html_borders);
        code.EndFunction();
    }

    auto has_explicit_size = (code.node()->as_wxSize(prop_size) != wxDefaultSize ||
                              code.node()->as_wxSize(GenEnum::prop_minimum_size) != wxDefaultSize);

    if (code.HasValue(prop_html_content))
    {
        code.Eol(eol_if_needed)
            .NodeName()
            .Function("SetPage(")
            .QuotedString(prop_html_content)
            .EndFunction();
        if (has_explicit_size)
        {
            return true;
        }
    }
    else if (code.HasValue(prop_html_url))
    {
        code.Eol(eol_if_needed)
            .NodeName()
            .Function("LoadPage(")
            .QuotedString(prop_html_url)
            .EndFunction();
        if (has_explicit_size)
        {
            return true;
        }
    }

    if (!has_explicit_size)
    {
        code.Eol(eol_if_needed)
            .AddComment(
                "Neither size nor minimum_size properties set; setting a temporary minimum size",
                true);
        code.Eol(eol_if_needed)
            .NodeName()
            .Function("SetMinSize(")
            .Add("wxSize(160, 60)")
            .EndFunction();
    }

    return true;
}

auto HtmlWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags) -> int
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxHtmlWindow");

    if (node->as_int(prop_html_borders) >= 0)
    {
        ADD_ITEM_PROP(prop_html_borders, "borders");
    }
    ADD_ITEM_PROP(prop_html_url, "url");
    ADD_ITEM_PROP(prop_html_content, "htmlcode")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);
    if (node->as_wxSize(prop_size) == wxDefaultSize &&
        node->as_wxSize(GenEnum::prop_minimum_size) == wxDefaultSize)
    {
        item.append_child(pugi::node_comment)
            .set_value(
                " Neither size nor minimum_size properties set; setting a temporary minimum size ");
        item.append_child("minsize").text().set("160,60");
    }

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void HtmlWindowGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxHtmlWindowXmlHandler");
}

auto HtmlWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                      std::set<std::string>& set_hdr, GenLang /* language */)
    -> bool
{
    InsertGeneratorInclude(node, "#include <wx/html/htmlwin.h>", set_src, set_hdr);
    if (node->HasValue(prop_html_url))
    {
        InsertGeneratorInclude(node, "#include <wx/filesys.h>", set_src, set_hdr);
    }
    return true;
}

auto HtmlWindowGenerator::GetImports(Node* /*unused*/, std::set<std::string>& set_imports,
                                     GenLang language) -> bool
{
    if (language == GEN_LANG_RUBY)
    {
        set_imports.insert("require 'wx/html'");
        return true;
    }
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use base qw[Wx::Html];");
    }
    return false;
}
