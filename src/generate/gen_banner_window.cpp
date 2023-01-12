//////////////////////////////////////////////////////////////////////////
// Purpose:   wxBannerWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bannerwindow.h>  // wxBannerWindow class declaration

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_banner_window.h"

wxObject* BannerWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxBannerWindow(wxStaticCast(parent, wxWindow),
                                     (wxDirection) NodeCreation.GetConstantAsInt(node->prop_as_string(prop_direction)));

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->prop_as_wxBitmapBundle(prop_bitmap));
    }

    else if (node->HasValue(prop_start_colour) && node->HasValue(prop_end_colour))
    {
        widget->SetGradient(node->prop_as_wxColour(prop_start_colour), node->prop_as_wxColour(prop_end_colour));
    }

    if (node->HasValue(prop_title) || node->HasValue(prop_message))
    {
        widget->SetText(node->prop_as_wxString(prop_title), node->prop_as_wxString(prop_message));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool BannerWindowGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    if (code.is_cpp())
        code.ValidParentName().Comma().as_string(prop_direction);
    else
    {
        // wxPython docs state that you only need the direction, but in 4.2, there's an error
        // if you don't inlude the id.
        code.ValidParentName().Comma().as_string(prop_id).Comma().as_string(prop_direction);
    }
    code.PosSizeFlags(true);

    return true;
}

bool BannerWindowGenerator::SettingsCode(Code& code)
{
    if (code.HasValue(prop_bitmap) && code.is_cpp())
    {
        if (code.is_cpp())
        {
            ttlib::cstr tmp;
            GenBtnBimapCode(code.node(), tmp, true);
            code += tmp;
        }
        else
        {
            PythonBtnBimapCode(code, true);
        }
    }
    else if (code.HasValue(prop_start_colour) && code.HasValue(prop_end_colour))
    {
        auto& start_colour = code.node()->as_string(prop_start_colour);
        code.NodeName().Function("SetGradient(");
        if (start_colour.contains("wx"))
        {
            code.Add("wxSystemSettings").ClassMethod("GetColour(").Add(start_colour) << ")";
        }
        else
        {
            wxColour colour = ConvertToColour(start_colour);
            ttlib::cstr clr_format;
            clr_format.Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
            code.CheckLineLength(clr_format.size());
            code.Add(clr_format);
        }

        code.Comma().CheckLineLength();

        auto& end_colour = code.node()->as_string(prop_end_colour);
        if (end_colour.contains("wx"))
        {
            code.CheckLineLength(sizeof("wxSystemSettings::GetColour(") + end_colour.size() + sizeof(")"));
            code.Add("wxSystemSettings").ClassMethod("GetColour(").Add(end_colour) << ")";
        }
        else
        {
            wxColour colour = ConvertToColour(end_colour);
            ttlib::cstr clr_format;
            clr_format.Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
            code.CheckLineLength(clr_format.size());
            code.Add(clr_format);
        }
        code.EndFunction();
    }

    if (code.HasValue(prop_title) || code.HasValue(prop_message))
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
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxBannerWindow");

    ADD_ITEM_PROP(prop_message, "message")
    ADD_ITEM_PROP(prop_title, "title")
    ADD_ITEM_PROP(prop_direction, "direction")

    if (node->HasValue(prop_start_colour) && !node->HasValue(prop_bitmap))
    {
        item.append_child("gradient-start")
            .text()
            .set(node->prop_as_wxColour(prop_start_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }
    if (node->HasValue(prop_end_colour) && !node->HasValue(prop_bitmap))
    {
        item.append_child("gradient-end")
            .text()
            .set(node->prop_as_wxColour(prop_end_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
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
    if (node->HasValue(prop_bitmap))
    {
        handlers.emplace("wxBitmapXmlHandler");
    }
}
