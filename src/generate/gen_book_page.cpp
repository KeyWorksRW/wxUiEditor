//////////////////////////////////////////////////////////////////////////
// Purpose:   Book page generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
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
    auto node_parent = node->GetParent();

    if (node->GetParent()->isGen(gen_BookPage))
    {
        auto grandparent = node_parent->GetParent();
        ASSERT(grandparent);
        ASSERT(grandparent->isGen(gen_wxTreebook));

        auto grand_window = GetMockup()->GetMockupContent()->Get_wxObject(grandparent);
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
        auto grandparent = node_parent->GetParent();
        ASSERT(grandparent);
        ASSERT(grandparent->isGen(gen_wxTreebook));

        parent = GetMockup()->GetMockupContent()->Get_wxObject(grandparent);
        ASSERT(parent);
        auto tree = wxDynamicCast(parent, wxTreebook);
        ASSERT(tree);

        // To find an image previously added to the treebook's image list, we need to iterate through treebooks's pages and
        // sub-pages until we find the matching node.

        if (node->HasValue(prop_bitmap) && isBookDisplayImages(node))
        {
            int idx_image = 0;
            bool is_image_found { false };
            for (const auto& child: grandparent->GetChildNodePtrs())
            {
                if (child->HasValue(prop_bitmap))
                    ++idx_image;
                for (const auto& grand_child: child->GetChildNodePtrs())
                {
                    if (grand_child.get() == node)
                    {
                        is_image_found = true;
                        break;
                    }

                    // The parent bookpage can contain regular widgets along with child BookPages
                    if (grand_child->isGen(gen_BookPage) && grand_child->HasValue(prop_bitmap))
                        ++idx_image;
                }
                if (is_image_found)
                    break;
            }
            tree->AddSubPage(widget, node->prop_as_wxString(prop_label), false, idx_image);
        }
        else
        {
            tree->AddSubPage(widget, node->prop_as_wxString(prop_label), false, -1);
        }
    }
    else if (auto book = wxDynamicCast(parent, wxBookCtrlBase); book)
    {
        if (node->HasValue(prop_bitmap) &&
            (node_parent->prop_as_bool(prop_display_images) || node_parent->isGen(gen_wxToolbook)))
        {
            int idx_image = -1;
            bool is_image_found { false };
            for (const auto& child: node_parent->GetChildNodePtrs())
            {
                if (child.get() == node)
                {
                    if (idx_image < 0)
                        idx_image = 0;
                    break;
                }
                if (child->HasValue(prop_bitmap))
                {
                    if (idx_image < 0)
                        idx_image = 0;
                    ++idx_image;
                }
                if (child->GetParent()->isGen(gen_wxTreebook))
                {
                    for (const auto& grand_child: child->GetChildNodePtrs())
                    {
                        if (grand_child.get() == node)
                        {
                            is_image_found = true;
                            break;
                        }
                        if (grand_child->isGen(gen_BookPage) && grand_child->HasValue(prop_bitmap))
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

            book->AddPage(widget, node->prop_as_wxString(prop_label), false, idx_image);
        }
        else
        {
            book->AddPage(widget, node->prop_as_wxString(prop_label));
        }

        auto cur_selection = book->GetSelection();
        if (node->prop_as_bool(prop_select))
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
            if (node->HasValue(prop_bitmap) && node_parent->prop_as_bool(prop_display_images))
            {
                int idx_image = -1;
                for (const auto& child: node_parent->GetChildNodePtrs())
                {
                    if (child.get() == node)
                    {
                        if (idx_image < 0)
                            idx_image = 0;
                        break;
                    }
                    if (child->HasValue(prop_bitmap))
                    {
                        if (idx_image < 0)
                            idx_image = 0;
                        ++idx_image;
                    }
                }

                aui_book->AddPage(widget, node->prop_as_wxString(prop_label), false, idx_image);
            }
            else
            {
                aui_book->AddPage(widget, node->prop_as_wxString(prop_label));
            }

            auto cur_selection = aui_book->GetSelection();
            if (node->prop_as_bool(prop_select))
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

bool BookPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    return true;
}

std::optional<ttlib::cstr> BookPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);

    if (node->GetParent()->isGen(gen_BookPage))
    {
        bool is_display_images = isBookDisplayImages(node);
        auto treebook = node->GetParent()->GetParent();
        while (treebook->isGen(gen_BookPage))
        {
            treebook = treebook->GetParent();
        }

        code << treebook->get_node_name() << ", " << node->prop_as_string(prop_id);

        GeneratePosSizeFlags(node, code);

        code << '\n';
        code << treebook->get_node_name() << "->AddSubPage(" << node->get_node_name() << ", ";
        code << GenerateQuotedString(node, prop_label);

        // Default is false, so only add parameter if it is true.
        if (node->prop_as_bool(prop_select))
            code << ", true";

        if (node->HasValue(prop_bitmap) && is_display_images)
        {
            int idx_image = GetTreebookImageIndex(node);
            if (!node->prop_as_bool(prop_select))
                code << ", false";
            code << ", " << idx_image;
        }
        code << ");";
    }
    else
    {
        code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

        GeneratePosSizeFlags(node, code);

        code << '\n';
        code << GetParentName(node) << "->AddPage(" << node->get_node_name() << ", ";
        code << GenerateQuotedString(node, prop_label);

        // Default is false, so only add parameter if it is true.
        if (node->prop_as_bool(prop_select))
            code << ", true";

        if (node->HasValue(prop_bitmap) &&
            (node->GetParent()->prop_as_bool(prop_display_images) || node->GetParent()->isGen(gen_wxToolbook)))
        {
            auto node_parent = node->GetParent();
            int idx_image = -1;
            if (node_parent->isGen(gen_wxTreebook))
                idx_image = GetTreebookImageIndex(node);
            else
            {
                for (const auto& child: node_parent->GetChildNodePtrs())
                {
                    if (child.get() == node)
                    {
                        if (idx_image < 0)
                            idx_image = 0;
                        break;
                    }
                    if (child->HasValue(prop_bitmap))
                    {
                        if (idx_image < 0)
                            idx_image = 0;

                        ++idx_image;
                    }
                }
            }
            if (!node->prop_as_bool(prop_select))
                code << ", false";
            code << ", " << idx_image;
        }

        code << ");";
    }
    return code;
}
