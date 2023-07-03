/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxHtmlWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/html/htmlwin.h>  // wxHtmlWindow class for parsing & displaying HTML

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_html_window.h"

wxObject* HtmlWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxHtmlWindow(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->as_int(prop_html_borders) >= 0)
        widget->SetBorders(wxStaticCast(parent, wxWindow)->FromDIP(node->as_int(prop_html_borders)));

    if (node->HasValue(prop_html_content))
    {
        widget->SetPage(node->as_wxString(prop_html_content));
    }

#if 0
    // These work, but can take a LONG time to actually load and display if the html file is large.
    // Note that the XRC preview does display the URL so the user can still preview it.
    else if (node->HasValue(prop_html_url))
    {
        wxBusyInfo wait(wxBusyInfoFlags()
                            .Parent(wxStaticCast(parent, wxWindow))
                            .Title(wxString("Parsing ") << node->as_wxString(prop_html_url))
                            .Text("This could take awhile..."));
        widget->LoadPage(node->as_wxString(prop_html_url));
    }
#else
    else if (node->HasValue(prop_html_url))
    {
        widget->SetPage(tt_string("Contents of<br>    ") << node->as_string(prop_html_url) << "<br>will be displayed here.");
    }
#endif
    else
    {
        widget->SetPage("<b>wxHtmlWindow</b><br/><br/>This is a dummy page.</body></html>");
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool HtmlWindowGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(true, "wxHW_SCROLLBAR_AUTO");

    // If the last parameter is wxID_ANY, then remove it. This is the default value, so it's
    // not needed.
    code.Replace(", wxID_ANY)", ")");

    return true;
}

bool HtmlWindowGenerator::SettingsCode(Code& code)
{
    if (code.IntValue(prop_html_borders) >= 0)
    {
        code.Eol(eol_if_needed).NodeName().Function("SetBorders(");
        code += (code.is_cpp() ? "this->FromDIP(, " : "self.FromDIP(, ");
        code.Str(prop_html_borders).Str(")").EndFunction();
    }

    if (code.HasValue(prop_html_content))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetPage(").QuotedString(prop_html_content).EndFunction();
    }
    else if (code.HasValue(prop_html_url))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetPage(").QuotedString(prop_html_url).EndFunction();
    }

    return true;
}

int HtmlWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxHtmlWindow");

    if (node->as_int(prop_html_borders) >= 0)
        ADD_ITEM_PROP(prop_html_borders, "borders")
    ADD_ITEM_PROP(prop_html_url, "url")
    ADD_ITEM_PROP(prop_html_content, "htmlcode")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

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

bool HtmlWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/html/htmlwin.h>", set_src, set_hdr);
    if (node->HasValue(prop_html_url))
    {
        InsertGeneratorInclude(node, "#include <wx/filesys.h>", set_src, set_hdr);
    }
    return true;
}
