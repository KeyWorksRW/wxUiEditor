/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWebView generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/webview.h>  // Common interface and events for web view component

#include "code.h"           // Code -- Helper class for generating code
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

std::optional<ttlib::sview> WebViewGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().Str(" = ").Add("wxWebView").ClassMethod("New(");
    code.ValidParentName().Comma().Add(prop_id).Comma().QuotedString(prop_url);

    auto params_needed = code.WhatParamsNeeded();
    if (params_needed == nothing_needed)
    {
        code.EndFunction();
        return code.m_code;
    }
    code.Comma().Pos().Comma().WxSize();
    if (params_needed & style_needed || params_needed & window_name_needed)
    {
        code.Comma().Add("wxWebViewBackendDefault").Comma().Style();
        if (params_needed & window_name_needed)
            code.Comma().QuotedString(prop_window_name);
    }
    code.EndFunction();
    return code.m_code;
}

std::optional<ttlib::sview> WebViewGenerator::GenEvents(Code& code, NodeEvent* event, const std::string& class_name)
{
    if (code.is_python())
        return BaseGenerator::GenEvents(code, event, class_name);
    if ((event->get_name() == "wxEVT_WEBVIEW_FULL_SCREEN_CHANGED" ||
         event->get_name() == "wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED") &&
        wxGetProject().value(prop_wxWidgets_version) == "3.1")
    {
        code.Add("\n#if wxCHECK_VERSION(3, 1, 5)\n");
        BaseGenerator::GenEvents(code, event, class_name);
        code.Add("\n#endif");
        return code.m_code;
    }
    else
    {
        return BaseGenerator::GenEvents(code, event, class_name);
    }
}

bool WebViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/webview.h>", set_src, set_hdr);
    return true;
}
