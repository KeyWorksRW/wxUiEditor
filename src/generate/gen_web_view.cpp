/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWebView generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stattext.h>  // wxStaticText base header
#include <wx/webview.h>   // Common interface and events for web view component

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties

#include "gen_web_view.h"

wxObject* WebViewGenerator::CreateMockup(Node* node, wxObject* parent)
{
    if (Project.getCodePreference() == GEN_LANG_RUBY || Project.getCodePreference() == GEN_LANG_XRC)
    {
        tt_string msg = "wxWebView not available in ";
        if (Project.getCodePreference() == GEN_LANG_RUBY)
            msg += "wxRuby3";
        else
            msg += "XRC";
        auto* widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, msg.make_wxString(), wxDefaultPosition,
                                        wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_RAISED);
        widget->Wrap(DlgPoint(150));
        return widget;
    }
#if defined(WIN32)
    auto widget =
        wxWebView::New(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_url), DlgPoint(node, prop_pos),
                       DlgSize(node, prop_size), wxWebViewBackendDefault, GetStyleInt(node));
#else
    auto* widget =
        new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, "wxWebView mockup currently only available for Windows",
                         wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_RAISED);
    widget->Wrap(DlgPoint(150));
#endif

    return widget;
}

bool WebViewGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().Str(" = ").Add("wxWebView").ClassMethod("New(");
    code.ValidParentName().Comma().Add(prop_id).Comma().QuotedString(prop_url);

    auto params_needed = code.WhatParamsNeeded();
    if (params_needed == nothing_needed)
    {
        code.EndFunction();
        return true;
    }
    code.Comma().Pos().Comma().WxSize();
    if (params_needed & style_needed || params_needed & window_name_needed)
    {
        code.Comma().Add("wxWebViewBackendDefault").Comma().Style();
        if (params_needed & window_name_needed)
            code.Comma().QuotedString(prop_window_name);
    }
    code.EndFunction();
    return true;
}

void WebViewGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    if (code.is_python())
    {
        BaseGenerator::GenEvent(code, event, class_name);
        return;
    }
    if ((event->get_name() == "wxEVT_WEBVIEW_FULL_SCREEN_CHANGED" ||
         event->get_name() == "wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED") &&
        Project.is_wxWidgets31())
    {
        code.Add("\n#if wxCHECK_VERSION(3, 1, 5)\n");
        BaseGenerator::GenEvent(code, event, class_name);
        code.Add("\n#endif");
        return;
    }
}

bool WebViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                   GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/webview.h>", set_src, set_hdr);
    return true;
}

std::optional<tt_string> WebViewGenerator::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_RUBY:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxRuby currently does not support Wx::WebView";
                return msg;
            }
        default:
            return {};
    }
}
