//////////////////////////////////////////////////////////////////////////
// Purpose:   Tree page generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-07-2026]

#include <wx/bookctrl.h>  // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/treebook.h>  // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "mockup_content.h"  // MockupContent -- Mockup of a form's contents
#include "mockup_parent.h"   // MockupParent -- Top-Level MockUp Parent window
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_tree_page.h"

// A TreePage is always created as a child of the wxTreebook window, regardless of how deeply the
// pages are nested in our node tree.
static Node* Get_OwnerTreebook(Node* node)
{
    for (Node* parent = node->get_Parent(); parent; parent = parent->get_Parent())
    {
        if (parent->is_Gen(gen_wxTreebook))
        {
            return parent;
        }
    }
    return nullptr;
}

// Helper for adding a page as a sub-page of a wxTreebook
// Called by: CreateMockup
bool TreePageGenerator::AddTreebookSubPage(Node* node, wxPanel* widget, Node* /* node_parent */)
{
    Node* treebook = Get_OwnerTreebook(node);
    ASSERT(treebook);
    if (!treebook)
    {
        return false;
    }

    wxObject* parent_obj = getMockup()->GetMockupContent()->Get_wxObject(treebook);
    ASSERT(parent_obj);
    if (!parent_obj)
    {
        return false;
    }

    wxTreebook* tree = wxDynamicCast(parent_obj, wxTreebook);
    if (!tree)
    {
        FAIL_MSG("AddTreebookSubPage: parent_obj is not a wxTreebook");
        return false;
    }

    if (node->HasValue(prop_bitmap) && isBookDisplayImages(node))
    {
        tree->AddSubPage(widget, node->as_wxString(prop_label), false, GetTreebookImageIndex(node));
    }
    else
    {
        tree->AddSubPage(widget, node->as_wxString(prop_label), false, -1);
    }

    return true;
}

wxObject* TreePageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    (std::ignore) = parent;  // the real wx parent is always the owning wxTreebook

    Node* treebook = Get_OwnerTreebook(node);
    ASSERT(treebook);
    if (!treebook)
    {
        return nullptr;
    }

    const wxObject* treebook_window = getMockup()->GetMockupContent()->Get_wxObject(treebook);
    ASSERT(treebook_window);
    if (!treebook_window)
    {
        return nullptr;
    }

    wxPanel* widget =
        new wxPanel(wxStaticCast(treebook_window, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                    DlgSize(node, prop_size), GetStyleInt(node));

    if (node->get_Parent()->is_Type(type_treepage))
    {
        if (!AddTreebookSubPage(node, widget, node->get_Parent()))
        {
            delete widget;
            return nullptr;
        }
    }
    else
    {
        wxBookCtrlBase* book = wxDynamicCast(treebook_window, wxBookCtrlBase);
        ASSERT(book);
        if (!book)
        {
            delete widget;
            return nullptr;
        }

        if (node->HasValue(prop_bitmap) && isBookDisplayImages(node))
        {
            book->AddPage(widget, node->as_wxString(prop_label), false,
                          GetTreebookImageIndex(node));
        }
        else
        {
            book->AddPage(widget, node->as_wxString(prop_label), node->as_bool(prop_select));
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool TreePageGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();

    Node* node = code.node();
    const bool is_display_images = isBookDisplayImages(node);

    if (node->get_Parent()->is_Type(type_treepage))
    {
        Node* treebook = Get_OwnerTreebook(node);
        ASSERT(treebook);
        if (!treebook)
        {
            return false;
        }

        code.NodeName(treebook).Comma().as_string(prop_id);
        code.PosSizeFlags();

        // If the last parameter is wxID_ANY, then remove it. This is the default value, so it's
        // not needed.
        std::ignore = code.Replace(", wxID_ANY)", ")");

        code.Eol()
            .NodeName(treebook)
            .Function("AddSubPage(")
            .NodeName()
            .Comma()
            .QuotedString(prop_label);

        // Default is false, so only add parameter if it is true.
        if (code.IsTrue(prop_select))
        {
            code.Comma().True();
        }

        if (node->HasValue(prop_bitmap) && is_display_images)
        {
            const int idx_image = GetTreebookImageIndex(node);
            if (!node->as_bool(prop_select))
            {
                code.Comma().False();
            }
            code.Comma() << idx_image;
        }
        code.EndFunction();
    }
    else
    {
        code.ValidParentName().Comma().as_string(prop_id);
        code.PosSizeFlags();

        code.Eol().ParentName().Function("AddPage(").NodeName().Comma().QuotedString(prop_label);

        // Default is false, so only add parameter if it is true.
        if (code.IsTrue(prop_select))
        {
            code.Comma().True();
        }

        if (node->HasValue(prop_bitmap) && is_display_images)
        {
            const int idx_image = GetTreebookImageIndex(node);
            if (!node->as_bool(prop_select))
            {
                code.Comma().False();
            }
            code.Comma() << idx_image;
        }
        code.EndFunction();
    }

    return true;
}

bool TreePageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    return true;
}

// ../../build/_deps/wxwidgets-src/src/xrc/xh_bookctrlbase.cpp

int TreePageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "treebookpage");
    GenXrcBitmap(node, item, xrc_flags);

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_select, "selected")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    pugi::xml_node panel = item.append_child("object");
    panel.append_attribute("class").set_value("wxPanel");
    panel.append_attribute("name").set_value(node->as_string(prop_var_name));
    panel.append_child("style").text().set("wxTAB_TRAVERSAL");

    return BaseGenerator::xrc_sizer_item_created;
}

void TreePageGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxBookCtrlXmlHandlerBase");
}
