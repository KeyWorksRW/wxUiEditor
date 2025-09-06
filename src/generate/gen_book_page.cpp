//////////////////////////////////////////////////////////////////////////
// Purpose:   Book page generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>     // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/treebook.h>     // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "gen_book_utils.h"  // Common Book utilities
#include "gen_common.h"      // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"   // Common XRC generating functions
#include "mockup_content.h"  // MockupContent -- Mockup of a form's contents
#include "mockup_parent.h"   // MockupParent -- Top-level MockUp Parent window
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_book_page.h"

wxObject* BookPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxPanel* widget;
    auto node_parent = node->get_Parent();

    if (node->get_Parent()->is_Gen(gen_BookPage))
    {
        auto grandparent = node_parent->get_Parent();
        ASSERT(grandparent);
        ASSERT(grandparent && grandparent->is_Gen(gen_wxTreebook));

        auto grand_window = getMockup()->GetMockupContent()->Get_wxObject(grandparent);
        widget = new wxPanel(wxStaticCast(grand_window, wxWindow), wxID_ANY,
                             DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));
    }
    else
    {
        widget = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                             DlgSize(node, prop_size), GetStyleInt(node));
    }

    if (node_parent->is_Gen(gen_BookPage))
    {
        auto grandparent = node_parent->get_Parent();
        ASSERT(grandparent);
        ASSERT(grandparent->is_Gen(gen_wxTreebook));

        parent = getMockup()->GetMockupContent()->Get_wxObject(grandparent);
        ASSERT(parent);
        auto tree = wxDynamicCast(parent, wxTreebook);
        ASSERT(tree);

        // To find an image previously added to the treebook's image list, we need to iterate
        // through treebooks's pages and sub-pages until we find the matching node.

        if (node->HasValue(prop_bitmap) && isBookDisplayImages(node))
        {
            int idx_image = 0;
            bool is_image_found { false };
            for (const auto& child: grandparent->get_ChildNodePtrs())
            {
                if (child->HasValue(prop_bitmap))
                    ++idx_image;
                for (const auto& grand_child: child->get_ChildNodePtrs())
                {
                    if (grand_child.get() == node)
                    {
                        is_image_found = true;
                        break;
                    }

                    // The parent bookpage can contain regular widgets along with child BookPages
                    if (grand_child->is_Gen(gen_BookPage) && grand_child->HasValue(prop_bitmap))
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
        if (node->HasValue(prop_bitmap) &&
            (node_parent->as_bool(prop_display_images) || node_parent->is_Gen(gen_wxToolbook)))
        {
            int idx_image = -1;
            bool is_image_found { false };
            for (const auto& child: node_parent->get_ChildNodePtrs())
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
                if (child->get_Parent()->is_Gen(gen_wxTreebook))
                {
                    for (const auto& grand_child: child->get_ChildNodePtrs())
                    {
                        if (grand_child.get() == node)
                        {
                            is_image_found = true;
                            break;
                        }
                        if (grand_child->is_Gen(gen_BookPage) && grand_child->HasValue(prop_bitmap))
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
            if (node->HasValue(prop_bitmap) && node_parent->as_bool(prop_display_images))
            {
                int idx_image = -1;
                for (const auto& child: node_parent->get_ChildNodePtrs())
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
    if (code.is_perl() && code.node()->get_Parent()->is_Gen(gen_wxSimplebook))
    {
        code += "# Can't add books to wxSimplebook which is not supported in Perl";
        return true;
    }

    code.AddAuto().NodeName().CreateClass();

    Node* node = code.node();
    if (node->get_Parent()->is_Gen(gen_BookPage))
    {
        bool is_display_images = isBookDisplayImages(node);
        auto treebook = node->get_Parent()->get_Parent();
        while (treebook->is_Gen(gen_BookPage))
        {
            treebook = treebook->get_Parent();
        }

        code.NodeName(treebook).Comma().as_string(prop_id);
        code.PosSizeFlags();

        // If the last parameter is wxID_ANY, then remove it. This is the default value, so it's
        // not needed.
        code.Replace(", wxID_ANY)", ")");

        code.Eol()
            .NodeName(treebook)
            .Function("AddSubPage(")
            .NodeName()
            .Comma()
            .QuotedString(prop_label);

        // Default is false, so only add parameter if it is true.
        if (code.IsTrue(prop_select))
            code.Comma().True();

        if (node->HasValue(prop_bitmap) && is_display_images)
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
        if (node->get_Parent()->is_Gen(gen_wxPropertySheetDialog))
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
        code.PosSizeFlags();
        if (node->get_Parent()->is_Gen(gen_wxPropertySheetDialog))
        {
            // Break out the close parenthesis so that the Ruby code can remove the () entirely.
            if (code.is_ruby())
                code.Eol().Str("get_book_ctrl");
            else
                code.Eol().FormFunction("GetBookCtrl()");
            code.Function("AddPage(").NodeName().Comma().QuotedString(prop_label);
        }
        else
            code.Eol()
                .ParentName()
                .Function("AddPage(")
                .NodeName()
                .Comma()
                .QuotedString(prop_label);

        // Default is false, so only add parameter if it is true.
        if (code.IsTrue(prop_select))
            code.Comma().True();

        if (node->HasValue(prop_bitmap) && (node->get_Parent()->as_bool(prop_display_images) ||
                                            node->get_Parent()->is_Gen(gen_wxToolbook)))
        {
            auto node_parent = node->get_Parent();
            int idx_image = -1;
            if (node_parent->is_Gen(gen_wxTreebook))
            {
                idx_image = GetTreebookImageIndex(node);
            }
            else
            {
                for (const auto& child: node_parent->get_ChildNodePtrs())
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
            if (!node->as_bool(prop_select))
                code.Comma().False();
            code.Comma() << idx_image;
        }
        code.EndFunction();
    }

    return true;
}

bool BookPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_bookctrlbase.cpp
// ../../../wxWidgets/src/xrc/xh_bookctrlbase.cpp

int BookPageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);

    tt_string page_type;
    if (node->get_Parent()->is_Gen(gen_wxNotebook) || node->get_Parent()->is_Gen(gen_wxAuiNotebook))
        page_type = "notebookpage";
    else if (node->get_Parent()->is_Gen(gen_wxChoicebook))
        page_type = "choicebookpage";
    else if (node->get_Parent()->is_Gen(gen_wxListbook))
        page_type = "listbookpage";
    else if (node->get_Parent()->is_Gen(gen_wxSimplebook))
        page_type = "simplebookpage";
    else if (node->get_Parent()->is_Gen(gen_wxToolbook))
        page_type = "toolbookpage";
    else if (node->get_Parent()->is_Gen(gen_wxTreebook))
        page_type = "treebookpage";
    else if (node->get_Parent()->is_Gen(gen_BookPage))
        page_type = "treebookpage";
    else if (node->get_Parent()->is_Gen(gen_wxPropertySheetDialog))
        page_type = "propertysheetpage";
    else
        FAIL_MSG(
            "BookPageGenerator needs to know what to call the pages to pass to the XRC handler.")

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

bool BookPageGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                   GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use Wx qw[:panel];");
    }
    return false;
}
