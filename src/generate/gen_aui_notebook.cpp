//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiNotebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>     // wxBookCtrlBase: common base class for wxList/Tree/Notebook

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_aui_notebook.h"

wxObject* AuiNotebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxAuiNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                    DlgSize(parent, node, prop_size), GetStyleInt(node));
    if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiGenericTabArt"))
        widget->SetArtProvider(new wxAuiGenericTabArt());
    else if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiSimpleTabArt"))
        widget->SetArtProvider(new wxAuiSimpleTabArt());

    if (node->prop_as_int(prop_tab_height) > 0)
        widget->SetTabCtrlHeight(node->prop_as_int(prop_tab_height));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &AuiNotebookGenerator::OnPageChanged, this);

    return widget;
}

void AuiNotebookGenerator::OnPageChanged(wxNotebookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxNotebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool AuiNotebookGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(false);

    // TODO: [Randalphwa - 12-21-2022] Add Python support
    if (code.is_cpp())
    {
        BookCtorAddImagelist(code.m_code, code.m_node);
    }

    // REVIEW: [Randalphwa - 12-21-2022] We use this ourselves in base_panel.cpp and it is in wx/aui/tabart.h
    // without any specific comments, however it is not documented. Should we support it? Currently there
    // is no setting for it in aui_xml.xml.

    if (code.IsEqualTo(prop_art_provider, "wxAuiGenericTabArt"))
    {
        if (code.is_cpp())
        {
            code.Eol().NodeName().Function("SetArtProvider(");
            code.Str(code.is_cpp() ? "new wxAuiGenericTabArt()" : "wx.aui.wxAuiGenericTabArt()").EndFunction();
        }
        else
        {
            code.Eol() += "# wxPython does not support wxAuiGenericTabArt";
        }
    }
    else if (code.IsEqualTo(prop_art_provider, "wxAuiSimpleTabArt"))
    {
        code.Eol().NodeName().Function("SetArtProvider(");
        code.Str(code.is_cpp() ? "new wxAuiSimpleTabArt()" : "wx.aui.AuiSimpleTabArt()").EndFunction();
    }

    return true;
}

bool AuiNotebookGenerator::SettingsCode(Code& code)
{
    if (code.IntValue(prop_tab_height) > 0)
    {
        code.Eol().NodeName().Function("SetTabCtrlHeight(").Str(prop_tab_height).EndFunction();
    }

    return true;
}

bool AuiNotebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/aui/auibook.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_aui.cpp
// ../../../wxWidgets/src/xrc/xh_aui.cpp
// wxAuiNotebook handler is near the end of this file.

int AuiNotebookGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxAuiNotebook");

    if (node->value(prop_art_provider) == "wxAuiSimpleTabArt")
        item.append_child("art-provider").text().set("simple");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void AuiNotebookGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxAuiXmlHandler");
}
