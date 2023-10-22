//////////////////////////////////////////////////////////////////////////
// Purpose:   wxBannerWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bannerwindow.h>  // wxBannerWindow class declaration
#include <wx/stattext.h>      // wxStaticText base header

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // WriteCode -- Write code to Scintilla or file

#include "gen_banner_window.h"

wxObject* BannerWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    if (Project.getCodePreference() == GEN_LANG_RUBY)
    {
        auto* widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, "wxBannerWindow not available in wxRuby3",
                                        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_RAISED);
        widget->Wrap(DlgPoint(parent, 150));
        return widget;
    }
    auto widget = new wxBannerWindow(wxStaticCast(parent, wxWindow),
                                     (wxDirection) NodeCreation.getConstantAsInt(node->as_string(prop_direction)));

    if (node->hasValue(prop_bitmap))
    {
        widget->SetBitmap(node->as_wxBitmapBundle(prop_bitmap));
    }

    else if (node->hasValue(prop_start_colour) && node->hasValue(prop_end_colour))
    {
        widget->SetGradient(node->as_wxColour(prop_start_colour), node->as_wxColour(prop_end_colour));
    }

    if (node->hasValue(prop_title) || node->hasValue(prop_message))
    {
        widget->SetText(node->as_wxString(prop_title), node->as_wxString(prop_message));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool BannerWindowGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    if (code.is_cpp())
        code.ValidParentName().Comma().as_string(prop_direction);
    else if (code.is_python() || code.is_ruby())
    {
        code.ValidParentName().Comma().Add(prop_id).Comma().Add(prop_direction);
    }
    else
    {
        code.Str("# unknown language") << "wxBannerWindow";
    }

    code.PosSizeFlags(true);

    return true;
}

bool BannerWindowGenerator::SettingsCode(Code& code)
{
    if (code.hasValue(prop_bitmap))
    {
        if (code.is_cpp())
        {
            tt_string tmp;
            GenBtnBimapCode(code.node(), tmp, true);
            code += tmp;
        }
        else
        {
            PythonBtnBimapCode(code, true);
        }
    }
    else if (code.hasValue(prop_start_colour) && code.hasValue(prop_end_colour))
    {
        code.NodeName().Function("SetGradient(");

        auto colour = code.node()->as_wxColour(prop_start_colour);
        code.Object("wxColour").QuotedString(colour) += ')';
        code.Comma().CheckLineLength();
        colour = code.node()->as_wxColour(prop_end_colour);
        code.Object("wxColour").QuotedString(colour) += ')';

        code.EndFunction();
    }

    if (code.hasValue(prop_title) || code.hasValue(prop_message))
    {
        code.Eol(eol_if_empty);
        code.NodeName().Function("SetText(").QuotedString(prop_title);
        code.Comma().QuotedString(prop_message).EndFunction();
    }

    return true;
}

bool BannerWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/bannerwindow.h>", set_src, set_hdr);
    return true;
}

bool BannerWindowGenerator::GetPythonImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("import wx.adv");
    return true;
}

int BannerWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxBannerWindow");

    ADD_ITEM_PROP(prop_message, "message")
    ADD_ITEM_PROP(prop_title, "title")
    ADD_ITEM_PROP(prop_direction, "direction")

    if (node->hasValue(prop_start_colour) && !node->hasValue(prop_bitmap))
    {
        item.append_child("gradient-start")
            .text()
            .set(node->as_wxColour(prop_start_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }
    if (node->hasValue(prop_end_colour) && !node->hasValue(prop_bitmap))
    {
        item.append_child("gradient-end")
            .text()
            .set(node->as_wxColour(prop_end_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }

    GenXrcStylePosSize(node, item);

    GenXrcBitmap(node, item, xrc_flags);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void BannerWindowGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxBannerWindowXmlHandler");
    if (node->hasValue(prop_bitmap))
    {
        handlers.emplace("wxBitmapXmlHandler");
    }
}
