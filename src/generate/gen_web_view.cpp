/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWebView generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/webview.h>  // Common interface and events for web view component

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_web_view.h"

#if !wxUSE_WEBVIEW_WEBKIT && !wxUSE_WEBVIEW_WEBKIT2 && !wxUSE_WEBVIEW_IE
    #error "A wxWebView backend is required for the wxWebView Mockup"
#endif

wxObject* WebViewGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = wxWebView::New(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_string(prop_url).wx_str(),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), wxWebViewBackendDefault,
                                 GetStyleInt(node));

    return widget;
}

std::optional<ttlib::cstr> WebViewGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << " = wxWebView::New(";

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", "
         << GenerateQuotedString(node->prop_as_string(prop_url));

    bool isPosSet { false };
    auto pos = node->prop_as_wxPoint(prop_pos);
    if (pos.x != -1 || pos.y != -1)
    {
        code << ", wxPoint(" << pos.x << ", " << pos.y << ")";
        isPosSet = true;
    }

    auto size = node->prop_as_wxSize(prop_size);
    if (size.x != -1 || size.y != -1)
    {
        if (!isPosSet)
        {
            code << ", wxDefaultPosition";
            isPosSet = true;
        }
        code << ", wxSize(" << size.x << ", " << size.y << ")";
    }

    ttlib::cstr all_styles;
    GenStyle(node, all_styles);
    if (all_styles.is_sameas("0"))
        all_styles.clear();

    if (all_styles.size())
    {
        code << ", wxWebViewBackendDefault, " << all_styles;
    }

    code << ");";

    return code;
}

std::optional<ttlib::cstr> WebViewGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    if ((event->get_name() == "wxEVT_WEBVIEW_FULL_SCREEN_CHANGED" ||
         event->get_name() == "wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED") &&
        wxGetProject().value(prop_wxWidgets_version) == "3.1")
    {
        ttlib::cstr code("\n#if wxCHECK_VERSION(3, 1, 5)\n");
        code << GenEventCode(event, class_name);
        code << "\n#endif";
        return code;
    }
    else
    {
        return GenEventCode(event, class_name);
    }
}

bool WebViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/webview.h>", set_src, set_hdr);
    return true;
}
