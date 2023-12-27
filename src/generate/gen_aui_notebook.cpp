//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiNotebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
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
    if (node->as_string(prop_art_provider).is_sameas("wxAuiGenericTabArt"))
        widget->SetArtProvider(new wxAuiGenericTabArt());
    else if (node->as_string(prop_art_provider).is_sameas("wxAuiSimpleTabArt"))
        widget->SetArtProvider(new wxAuiSimpleTabArt());

    if (node->as_int(prop_tab_height) > 0)
        widget->SetTabCtrlHeight(node->as_int(prop_tab_height));

    AddBookImageList(node, widget);

    for (size_t idx = 0; idx < node->getChildCount(); ++idx)
    {
        auto child = node->getChild(idx);
        if (child->hasValue(prop_tooltip))
        {
            widget->SetPageToolTip(idx, child->as_string(prop_tooltip));
        }
    }

    if (node->hasValue(prop_selected_tab_font))
    {
        if (auto font = node->as_wxFont(prop_selected_tab_font); font.IsOk())
        {
            widget->SetSelectedFont(font);
        }
    }

    if (node->hasValue(prop_non_selected_tab_font))
    {
        if (auto font = node->as_wxFont(prop_non_selected_tab_font); font.IsOk())
        {
            widget->SetNormalFont(font);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &AuiNotebookGenerator::OnPageChanged, this);

    return widget;
}

void AuiNotebookGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    if (auto notebook = wxStaticCast(wxobject, wxAuiNotebook); notebook)
    {
        for (size_t idx = 0; idx < node->getChildCount(); ++idx)
        {
            auto child = node->getChild(idx);
            if (child->hasValue(prop_tooltip))
            {
                notebook->SetPageToolTip(idx, child->as_string(prop_tooltip));
            }
        }
    }
}

void AuiNotebookGenerator::OnPageChanged(wxNotebookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxNotebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        getMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

bool AuiNotebookGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).PosSizeFlags(false);
    BookCtorAddImagelist(code);

    if (code.IsEqualTo(prop_art_provider, "wxAuiGenericTabArt"))
    {
        if (code.is_cpp())
        {
            code.Eol().NodeName().Function("SetArtProvider(");
            code.Str("new wxAuiGenericTabArt()").EndFunction();
        }
        else if (code.is_python())
        {
            code.Eol() += "# wxPython does not support wxAuiGenericTabArt";
        }
        else if (code.is_ruby())
        {
            code.Eol() += "# wxRuby does not support wxAuiGenericTabArt";
        }
    }
    else if (code.IsEqualTo(prop_art_provider, "wxAuiSimpleTabArt"))
    {
        code.Eol().NodeName().Function("SetArtProvider(");
        code.CreateClass(false, "wxAuiSimpleTabArt", false).Str(")").EndFunction();
    }

    return true;
}

bool AuiNotebookGenerator::SettingsCode(Code& code)
{
    bool is_changed = false;
    // Note that currently there is no UI to set this -- it's simply here to handle importing from other designers.
    // See issue #936
#if 0
    if (code.IntValue(prop_tab_height) > 0)
    {
        code.Eol().NodeName().Function("SetTabCtrlHeight(").as_string(prop_tab_height).EndFunction();
        is_changed = true;
    }
#endif

    if (code.node()->hasValue(prop_selected_tab_font))
    {
        code.GenFont(prop_selected_tab_font, "SetSelectedFont(");
        is_changed = true;
    }
    if (code.node()->hasValue(prop_non_selected_tab_font))
    {
        code.GenFont(prop_non_selected_tab_font, "SetNormalFont(");
        is_changed = true;
    }

    return is_changed;
}

bool AuiNotebookGenerator::AfterChildrenCode(Code& code)
{
    bool is_tooltip_set = false;
    for (size_t idx = 0; idx < code.node()->getChildCount(); ++idx)
    {
        auto child = code.node()->getChild(idx);
        if (child->hasValue(prop_tooltip))
        {
            is_tooltip_set = true;
            code.Eol().NodeName().Function("SetPageToolTip(").itoa(idx).Comma();
            code.CheckLineLength(child->as_string(prop_tooltip).size() + 2)
                .QuotedString(child->as_string(prop_tooltip))
                .EndFunction();
        }
    }
    return is_tooltip_set;
}

bool AuiNotebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                       int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/aui/auibook.h>", set_src, set_hdr);
    if (node->hasValue(prop_persist_name))
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
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxAuiNotebook");

    if (node->as_string(prop_art_provider) == "wxAuiSimpleTabArt")
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

bool AuiNotebookGenerator::GetRubyImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("require 'wx/aui'");
    return true;
}
