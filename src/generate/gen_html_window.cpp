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

    if (node->prop_as_int(prop_html_borders) >= 0)
        widget->SetBorders(wxStaticCast(parent, wxWindow)->FromDIP(node->prop_as_int(prop_html_borders)));

    if (node->HasValue(prop_html_content))
    {
        widget->SetPage(node->prop_as_wxString(prop_html_content));
    }

#if 0
    // These work, but can take a LONG time to actually load and display if the html file is large.
    // Note that the XRC preview does display the URL so the user can still preview it.
    else if (node->HasValue(prop_html_url))
    {
        wxBusyInfo wait(wxBusyInfoFlags()
                            .Parent(wxStaticCast(parent, wxWindow))
                            .Title(ttString("Parsing ") << node->prop_as_wxString(prop_html_url))
                            .Text("This could take awhile..."));
        widget->LoadPage(node->prop_as_wxString(prop_html_url));
    }
#else
    else if (node->HasValue(prop_html_url))
    {
        widget->SetPage(ttlib::cstr("Contents of<br>    ")
                        << node->prop_as_string(prop_html_url) << "<br>will be displayed here.");
    }
#endif
    else
    {
        widget->SetPage("<b>wxHtmlWindow</b><br/><br/>This is a dummy page.</body></html>");
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> HtmlWindowGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    code.Replace(", wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO)", ")");

    return code;
}

std::optional<ttlib::cstr> HtmlWindowGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_int(prop_html_borders) >= 0)
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBorders(this->FromDIP(, " << node->prop_as_int(prop_html_borders) << "));\n";
    }

    if (node->HasValue(prop_html_content))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetPage(" << GenerateQuotedString(node->prop_as_string(prop_html_content))
             << ");\n";
    }
    else if (node->HasValue(prop_html_url))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->LoadPage(" << GenerateQuotedString(node->prop_as_string(prop_html_url))
             << ");\n";
    }

    if (code.size())
        return code;
    else

        return {};
}

std::optional<ttlib::cstr> HtmlWindowGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int HtmlWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxHtmlWindow");

    if (node->prop_as_int(prop_html_borders) >= 0)
        ADD_ITEM_PROP(prop_html_borders, "borders")
    ADD_ITEM_PROP(prop_html_url, "url")
    ADD_ITEM_PROP(prop_html_content, "htmlcode")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
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
