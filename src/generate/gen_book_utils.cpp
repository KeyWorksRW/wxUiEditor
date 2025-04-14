//////////////////////////////////////////////////////////////////////////
// Purpose:   Common Book utilities
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bookctrl.h>  // wxBookCtrlBase: common base class for wxList/Tree/Notebook

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // Common component functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "ui_images.h"        // Generated images header

#include "gen_book_utils.h"

bool isBookDisplayImages(Node* node)
{
    if (!node->isGen(gen_BookPage))
        return node->as_bool(prop_display_images);
    for (auto node_parent = node->getParent(); node_parent; node_parent = node_parent->getParent())
    {
        if (!node_parent->isGen(gen_BookPage))
            return node_parent->as_bool(prop_display_images);
    }
    return false;
}

bool isBookHasImage(Node* node)
{
    bool is_book = !node->isGen(gen_BookPage);

    for (const auto& child_node: node->getChildNodePtrs())
    {
        if (child_node->isGen(gen_BookPage))
        {
            if (child_node->hasValue(prop_bitmap))
                return true;
            if (is_book && !node->isGen(gen_wxTreebook))
                continue;

            for (const auto& grand_child: child_node->getChildNodePtrs())
            {
                if (grand_child->isGen(gen_BookPage))
                {
                    auto result = isBookHasImage(grand_child.get());
                    if (result)
                        return true;
                }
            }
        }
    }
    return false;
}

void AddBookImageList(Node* node_book, wxObject* widget)
{
    if (isBookDisplayImages(node_book) && isBookHasImage(node_book))
    {
        wxBookCtrlBase::Images bundle_list;
        for (const auto& child_node: node_book->getChildNodePtrs())
        {
            if (child_node->hasValue(prop_bitmap))
            {
                auto bundle = child_node->as_wxBitmapBundle(prop_bitmap);
                if (!bundle.IsOk())
                {
                    bundle = wxue_img::bundle_unknown_svg(24, 24);
                }
                bundle_list.push_back(bundle);
            }

            if (node_book->isGen(gen_wxTreebook))
            {
                AddTreebookSubImages(child_node.get(), bundle_list);
            }
        }
        auto book = wxStaticCast(widget, wxBookCtrlBase);
        book->SetImages(bundle_list);
    }
}

void BookCtorAddImagelist(Code& code)
{
    if ((code.IsTrue(prop_display_images) || code.IsGen(gen_wxToolbook)) && isBookHasImage(code.node()))
    {
        code.OpenBrace();
        code.Eol(eol_if_needed);
        if (code.is_cpp())
        {
            code.Eol(eol_if_needed) << "wxWithImages::Images bundle_list;";
            for (const auto& child_node: code.node()->getChildNodePtrs())
            {
                if (child_node->hasValue(prop_bitmap))
                {
                    tt_string bundle_code;
                    if (GenerateBundleCode(child_node->as_string(prop_bitmap), bundle_code))
                    {
                        code.Eol() << bundle_code;
                        code.Eol() << "\tbundle_list.push_back(wxBitmapBundle::FromBitmaps(bitmaps));";
                        // Close the brace that was opened by GenerateBundleCode()
                        code.Eol() << "}";
                    }
                    else
                    {
                        // If GenerateBundleCode() returns false, then only a bitmap was generated
                        code.Eol().Str("bundle_list.push_back(").Str(bundle_code).EndFunction();
                    }
                }
            }
        }
        else if (code.is_python() || code.is_ruby())
        {
            code.Eol().Str("bundle_list = [");
            code.Indent();
            for (const auto& child_node: code.node()->getChildNodePtrs())
            {
                if (child_node->hasValue(prop_bitmap))
                {
                    Code bundle_code(child_node.get(), code.get_language());
                    bundle_code.Bundle(prop_bitmap);
                    code.Eol() << bundle_code;
                    code.Str(",");
                }
            }

            if (code.back() == ',')
            {
                // There may have been a line break, so remove that too
                code.pop_back();
                while (code.back() == '\t')
                    code.pop_back();
                if (code.back() == '\n')
                    code.pop_back();
            }
            code.Unindent();
            code.Eol(eol_if_needed).Str("]");
        }
        else
        {
            FAIL_MSG("Unknown language");
        }

        code.Eol().NodeName().Function("SetImages(bundle_list").EndFunction();

        code.CloseBrace();
    }
}

void AddTreebookSubImages(Node* node, wxBookCtrlBase::Images& bundle_list)
{
    for (const auto& child_node: node->getChildNodePtrs())
    {
        if (child_node->isGen(gen_BookPage))
        {
            if (child_node->hasValue(prop_bitmap))
            {
                bundle_list.push_back(child_node->as_wxBitmapBundle(prop_bitmap));
            }
            AddTreebookSubImages(child_node.get(), bundle_list);
        }
    }
}

void AddTreebookImageCode(tt_string& code, Node* child_node, size_t& image_index)
{
    for (const auto& grand_child: child_node->getChildNodePtrs())
    {
        if (grand_child->isGen(gen_BookPage) && grand_child->hasValue(prop_bitmap))
        {
            code << "\n\tauto img_" << image_index << " = ";
            code << GenerateBitmapCode(grand_child->as_string(prop_bitmap)) << ";";
            code << "\n\timg_list->Add(img_" << image_index;
            if (grand_child->as_string(prop_bitmap).starts_with("Art;"))
                code << ".ConvertToImage()";
            code << ");";
            ++image_index;
            AddTreebookImageCode(code, grand_child.get(), image_index);
        }
    }
}

int GetTreebookImageIndex(Node* node)
{
    int idx_image = 0;

    auto treebook = node->getParent();
    while (treebook->isGen(gen_BookPage))
    {
        treebook = treebook->getParent();
    }

    for (const auto& child_node: treebook->getChildNodePtrs())
    {
        if (child_node.get() == node)
            return idx_image;
        if (child_node->hasValue(prop_bitmap))
            ++idx_image;
        for (const auto& grand_child: child_node->getChildNodePtrs())
        {
            if (grand_child->isGen(gen_BookPage))
            {
                if (grand_child.get() == node)
                {
                    return idx_image;
                }
                if (grand_child->hasValue(prop_bitmap))
                    ++idx_image;
            }
        }
    }

    return idx_image;
}
