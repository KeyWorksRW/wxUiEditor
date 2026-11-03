/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPropertyGridManager and wxPropertyGridPage generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/event.h>             // Event classes
#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties
#include "utils_prop_grid.h"  // PropertyGrid utilities

#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "../mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

#include "gen_prop_grid_mgr.h"

////////////////////////////////  PropertyGridManagerGenerator ///////////////////////////////////

wxObject* PropertyGridManagerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget = new wxPropertyGridManager(wxStaticCast(parent, wxWindow), wxID_ANY,
                                             DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                                             GetStyleInt(node));

    if (node->HasValue(prop_extra_style))
    {
        widget->SetExtraStyle(node->as_int(prop_extra_style));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_PG_PAGE_CHANGED, &PropertyGridManagerGenerator::OnPageChanged, this);

    return widget;
}

void PropertyGridManagerGenerator::OnPageChanged(wxPropertyGridEvent& event)
{
    if (auto* pgm = wxDynamicCast(event.GetEventObject(), wxPropertyGridManager); pgm)
    {
        if (auto* cur_page = pgm->GetCurrentPage(); cur_page)
        {
            auto page_index = cur_page->GetIndex();
            if (auto* parent = getMockup()->getNode(event.GetEventObject()); parent)
            {
                for (size_t idx_page = 0; idx_page < parent->get_ChildCount(); ++idx_page)
                {
                    if ((to_int) idx_page == page_index)
                    {
                        wxGetFrame().SelectNode(parent->get_ChildNodePtrs()[idx_page].get());
                        break;
                    }
                }
            }
        }
    }
    event.Skip();
}

void PropertyGridManagerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */,
                                                 Node* node, bool /* is_preview */)
{
    auto* pgm = wxStaticCast(wxobject, wxPropertyGridManager);
    for (auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_propGridPage))
        {
            auto* page =
                pgm->AddPage(child->as_wxString(prop_label), child->as_wxBitmapBundle(prop_bitmap));

            AfterCreationAddItems(page, child.get());
        }
    }

    if (node->get_ChildCount())
    {
        pgm->SelectPage(0);
    }

    if (node->as_bool(prop_show_header))
    {
        pgm->ShowHeader();
    }
    pgm->Update();
}

bool PropertyGridManagerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, false, "wxPGMAN_DEFAULT_STYLE");

    if (code.HasValue(prop_extra_style))
    {
        code.Eol().NodeName().Function("SetExtraStyle(").Add(prop_extra_style).EndFunction();
    }

    return true;
}

bool PropertyGridManagerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                               std::set<std::string>& set_hdr,
                                               GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/propgrid/propgrid.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/propgrid/manager.h>", set_src, set_hdr);

    if (CheckAdvancePropertyInclude(node))
    {
        InsertGeneratorInclude(node, "#include <wx/propgrid/advprops.h>", set_src, set_hdr);
    }

    return true;
}
bool PropertyGridManagerGenerator::AfterChildrenCode(Code& code)
{
    if (code.IsTrue(prop_show_header))
    {
        code.NodeName().Function("ShowHeader(").True().EndFunction();
        return true;
    }
    return false;
}

bool PropertyGridManagerGenerator::GetImports([[maybe_unused]] Node* node,
                                              std::set<std::string>& set_imports, GenLang language)
{
    if (language == GenLang::ruby)
    {
        set_imports.insert("require 'wx/pg'");
        return true;
    }
    return false;
}

int PropertyGridManagerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    const int result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                        BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxPropertyGridManager");
    GenXrcPreStylePosSize(node, item, GetXrcPropertyGridStyle(node, "wxPGMAN_DEFAULT_STYLE"));
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (!child->is_Gen(gen_propGridPage))
        {
            continue;
        }

        pugi::xml_node page = item.append_child("page");
        if (child->HasValue(prop_label))
        {
            page.append_child("label").text().set(child->as_string(prop_label));
        }
        GenXrcPropertyGridChildren(child.get(), page);
    }

    return result;
}

void PropertyGridManagerGenerator::RequiredHandlers([[maybe_unused]] Node* node,
                                                    std::set<std::string>& handlers)
{
    handlers.emplace("wxPropertyGridXmlHandler");
}

/////////////////////////////////  PropertyGridPageGenerator ////////////////////////////////////

bool PropertyGridPageGenerator::ConstructionCode(Code& code)
{
    if (code.HasValue(prop_bitmap))
    {
        auto is_bitmaps_list = BitmapList(code, prop_bitmap);
        code.AddAuto()
            .NodeName()
            .Str(" = ")
            .ParentName()
            .Function("AddPage(")
            .QuotedString(prop_label);
        code.Comma();
        if (is_bitmaps_list)
        {
            if (code.is_cpp())
            {
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
            }
            else
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
        }
        else
        {
            if (code.is_cpp())
            {
                wxue::string bundle_code;
                GenerateBundleCode(code.node()->as_string(prop_bitmap), bundle_code);
                code.CheckLineLength(bundle_code.size());
                code += bundle_code;
            }
            else
            {
                code.Bundle(prop_bitmap);
            }
        }
        code.EndFunction();
    }
    else
    {
        code.AddAuto()
            .NodeName()
            .Str(" = ")
            .ParentName()
            .Function("AddPage(")
            .QuotedString(prop_label)
            .EndFunction();
    }

    return true;
}
