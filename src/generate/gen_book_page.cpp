//////////////////////////////////////////////////////////////////////////
// Purpose:   Book page generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>     // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/treebook.h>     // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "../mockup/mockup_content.h"  // MockupContent -- Mockup of a form's contents

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_book_page.h"

wxObject* BookPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxPanel* widget;
    auto node_parent = node->getParent();

    if (node->getParent()->isGen(gen_BookPage))
    {
        auto grandparent = node_parent->getParent();
        ASSERT(grandparent);
        ASSERT(grandparent && grandparent->isGen(gen_wxTreebook));

        auto grand_window = getMockup()->GetMockupContent()->Get_wxObject(grandparent);
        widget = new wxPanel(wxStaticCast(grand_window, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node));
    }
    else
    {
        widget = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node));
    }

    if (node_parent->isGen(gen_BookPage))
    {
        auto grandparent = node_parent->getParent();
        ASSERT(grandparent);
        ASSERT(grandparent->isGen(gen_wxTreebook));

        parent = getMockup()->GetMockupContent()->Get_wxObject(grandparent);
        ASSERT(parent);
        auto tree = wxDynamicCast(parent, wxTreebook);
        ASSERT(tree);

        // To find an image previously added to the treebook's image list, we need to iterate through treebooks's pages and
        // sub-pages until we find the matching node.

        if (node->hasValue(prop_bitmap) && isBookDisplayImages(node))
        {
            int idx_image = 0;
            bool is_image_found { false };
            for (const auto& child: grandparent->getChildNodePtrs())
            {
                if (child->hasValue(prop_bitmap))
                    ++idx_image;
                for (const auto& grand_child: child->getChildNodePtrs())
                {
                    if (grand_child.get() == node)
                    {
                        is_image_found = true;
                        break;
                    }

                    // The parent bookpage can contain regular widgets along with child BookPages
                    if (grand_child->isGen(gen_BookPage) && grand_child->hasValue(prop_bitmap))
                        ++idx_image;
                }
                if (is_image_found)
                    break;
            }
            tree->AddSubPage(widget, node->as_wxString(prop_label), false, idx_image);
        }
        else
        {
            tree->AddSubPage(widget, node->as_wxString(prop_label), false, -1);
        }
    }
    else if (auto book = wxDynamicCast(parent, wxBookCtrlBase); book)
    {
        if (node->hasValue(prop_bitmap) && (node_parent->as_bool(prop_display_images) || node_parent->isGen(gen_wxToolbook)))
        {
            int idx_image = -1;
            bool is_image_found { false };
            for (const auto& child: node_parent->getChildNodePtrs())
            {
                if (child.get() == node)
                {
                    if (idx_image < 0)
                        idx_image = 0;
                    break;
                }
                if (child->hasValue(prop_bitmap))
                {
                    if (idx_image < 0)
                        idx_image = 0;
                    ++idx_image;
                }
                if (child->getParent()->isGen(gen_wxTreebook))
                {
                    for (const auto& grand_child: child->getChildNodePtrs())
                    {
                        if (grand_child.get() == node)
                        {
                            is_image_found = true;
                            break;
                        }
                        if (grand_child->isGen(gen_BookPage) && grand_child->hasValue(prop_bitmap))
                        {
                            if (idx_image < 0)
                                idx_image = 0;
                            ++idx_image;
                        }
                    }
                    if (is_image_found)
                        break;
                }
            }

            book->AddPage(widget, node->as_wxString(prop_label), false, idx_image);
        }
        else
        {
            book->AddPage(widget, node->as_wxString(prop_label));
        }

        auto cur_selection = book->GetSelection();
        if (node->as_bool(prop_select))
        {
            book->SetSelection(book->GetPageCount() - 1);
        }
        else if (cur_selection >= 0)
        {
            book->SetSelection(cur_selection);
        }
    }
    else
    {
        auto aui_book = wxDynamicCast(parent, wxAuiNotebook);
        if (aui_book)
        {
            if (node->hasValue(prop_bitmap) && node_parent->as_bool(prop_display_images))
            {
                int idx_image = -1;
                for (const auto& child: node_parent->getChildNodePtrs())
                {
                    if (child.get() == node)
                    {
                        if (idx_image < 0)
                            idx_image = 0;
                        break;
                    }
                    if (child->hasValue(prop_bitmap))
                    {
                        if (idx_image < 0)
                            idx_image = 0;
                        ++idx_image;
                    }
                }

                aui_book->AddPage(widget, node->as_wxString(prop_label), false, idx_image);
            }
            else
            {
                aui_book->AddPage(widget, node->as_wxString(prop_label));
            }

            auto cur_selection = aui_book->GetSelection();
            if (node->as_bool(prop_select))
            {
                aui_book->SetSelection(aui_book->GetPageCount() - 1);
            }
            else if (cur_selection >= 0)
            {
                aui_book->SetSelection(cur_selection);
            }
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool BookPageGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();

    Node* node = code.node();
    if (node->getParent()->isGen(gen_BookPage))
    {
        bool is_display_images = isBookDisplayImages(node);
        auto treebook = node->getParent()->getParent();
        while (treebook->isGen(gen_BookPage))
        {
            treebook = treebook->getParent();
        }

        code.NodeName(treebook).Comma().as_string(prop_id);
        code.PosSizeFlags();

        // If the last parameter is wxID_ANY, then remove it. This is the default value, so it's
        // not needed.
        code.Replace(", wxID_ANY)", ")");

        code.Eol().NodeName(treebook).Function("AddSubPage(").NodeName().Comma().QuotedString(prop_label);

        // Default is false, so only add parameter if it is true.
        if (code.IsTrue(prop_select))
            code.Comma().True();

        if (node->hasValue(prop_bitmap) && is_display_images)
        {
            int idx_image = GetTreebookImageIndex(node);
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
        if (node->getParent()->isGen(gen_wxPropertySheetDialog))
        {
            if (code.is_ruby())
            {
                // wxRuby will fail if the empty () is included.
                code.Str("get_book_ctrl");
            }
            else
            {
                code.FormFunction("GetBookCtrl()");
            }
        }
        else
        {
            code.ValidParentName();
        }
        code.Comma().as_string(prop_id);
        code.PosSizeFlags(false);
        if (node->getParent()->isGen(gen_wxPropertySheetDialog))
        {
            // Break out the close parenthesis so that the Ruby code can remove the () entirely.
            if (code.is_ruby())
                code.Eol().Str("get_book_ctrl");
            else
                code.Eol().FormFunction("GetBookCtrl()");
            code.Function("AddPage(").NodeName().Comma().QuotedString(prop_label);
        }
        else
            code.Eol().ParentName().Function("AddPage(").NodeName().Comma().QuotedString(prop_label);

        // Default is false, so only add parameter if it is true.
        if (code.IsTrue(prop_select))
            code.Comma().True();

        if (node->hasValue(prop_bitmap) &&
            (node->getParent()->as_bool(prop_display_images) || node->getParent()->isGen(gen_wxToolbook)))
        {
            auto node_parent = node->getParent();
            int idx_image = -1;
            if (node_parent->isGen(gen_wxTreebook))
            {
                idx_image = GetTreebookImageIndex(node);
            }
            else
            {
                for (const auto& child: node_parent->getChildNodePtrs())
                {
                    if (child.get() == node)
                    {
                        if (idx_image < 0)
                            idx_image = 0;
                        break;
                    }
                    if (child->hasValue(prop_bitmap))
                    {
                        if (idx_image < 0)
                            idx_image = 0;

                        ++idx_image;
                    }
                }
            }
            if (!node->as_bool(prop_select))
                code.Comma().False();
            code.Comma() << idx_image;
        }
        code.EndFunction();
    }

    return true;
}

bool BookPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                    int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_bookctrlbase.cpp
// ../../../wxWidgets/src/xrc/xh_bookctrlbase.cpp

int BookPageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
#if 0
    int depth = 0;
    if (node->getParent()->isGen(gen_BookPage))
    {
        auto treebook = node->getParent()->getParent();
        ++depth;
        while (!treebook->isGen(gen_wxTreebook))
        {
            if (treebook->isForm())
            {
                FAIL_MSG("Expected a wxTreeBook parent for nested Book Page")
                return BaseGenerator::xrc_not_supported;
            }
            ++depth;
            treebook = treebook->getParent();
        }
        // Now that we've found the depth, let's find the xml node of the treebook
        for(;;)
        {
            auto class_attr = object.attribute("class");
            if (class_attr.value() != "wxTreebook")
            {
                object = object.parent();
                ASSERT(!object.empty())
            }
            else
            {
                break;
            }
        }
    }
#endif

    auto item = InitializeXrcObject(node, object);

    tt_string page_type;
    if (node->getParent()->isGen(gen_wxNotebook) || node->getParent()->isGen(gen_wxAuiNotebook))
        page_type = "notebookpage";
    else if (node->getParent()->isGen(gen_wxChoicebook))
        page_type = "choicebookpage";
    else if (node->getParent()->isGen(gen_wxListbook))
        page_type = "listbookpage";
    else if (node->getParent()->isGen(gen_wxSimplebook))
        page_type = "simplebookpage";
    else if (node->getParent()->isGen(gen_wxToolbook))
        page_type = "toolbookpage";
    else if (node->getParent()->isGen(gen_wxTreebook))
        page_type = "treebookpage";
    else if (node->getParent()->isGen(gen_BookPage))
        page_type = "treebookpage";
    else
        FAIL_MSG("BookPageGenerator needs to know what to call the pages to pass to the XRC handler.")

    GenXrcObjectAttributes(node, item, page_type);
    // if (depth > 0)
    // item.append_child("depth").text().set(depth);
    GenXrcBitmap(node, item, xrc_flags);

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_select, "selected")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    auto panel = item.append_child("object");
    panel.append_attribute("class").set_value("wxPanel");
    panel.append_attribute("name").set_value(node->as_string(prop_var_name));
    panel.append_child("style").text().set("wxTAB_TRAVERSAL");

    return BaseGenerator::xrc_sizer_item_created;
}

void BookPageGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxBookCtrlXmlHandlerBase");
}
