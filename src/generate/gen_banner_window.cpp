//////////////////////////////////////////////////////////////////////////
// Purpose:   wxBannerWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bannerwindow.h>  // wxBannerWindow class declaration

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
                                     (wxDirection) g_NodeCreator.GetConstantAsInt(node->prop_as_string(prop_direction)));

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

std::optional<ttlib::cstr> BannerWindowGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_direction) << ");";

    return code;
}

std::optional<ttlib::cstr> BannerWindowGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;
    if (node->HasValue(prop_bitmap))
    {
        if (GenBtnBimapCode(node, code, true))
        {
            auto_indent = indent::auto_keep_whitespace;
        }
    }
    else if (node->HasValue(prop_start_colour) && node->HasValue(prop_end_colour))
    {
        auto& start_colour = node->prop_as_string(prop_start_colour);
        code << node->get_node_name() << "->SetGradient(";
        if (start_colour.contains("wx"))
            code << "wxSystemSettings::GetColour(" << start_colour << ")";
        else
        {
            wxColour colour = ConvertToColour(start_colour);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
        }

        code << ",\n\t";

        auto& end_colour = node->prop_as_string(prop_end_colour);
        if (end_colour.contains("wx"))
            code << "wxSystemSettings::GetColour(" << end_colour << "));";
        else
        {
            wxColour colour = ConvertToColour(end_colour);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i));", colour.Red(), colour.Green(), colour.Blue());
        }
        auto_indent = indent::auto_keep_whitespace;
    }

    if (node->HasValue(prop_title) || node->HasValue(prop_message))
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetText(" << GenerateQuotedString(node->prop_as_string(prop_title)) << ",\n\t";
        code << GenerateQuotedString(node->prop_as_string(prop_message)) << ");";
        auto_indent = indent::auto_keep_whitespace;
    }

    return code;
}

bool BannerWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/bannerwindow.h>", set_src, set_hdr);
    return true;
}

int BannerWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
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

    GenXrcBitmap(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
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
