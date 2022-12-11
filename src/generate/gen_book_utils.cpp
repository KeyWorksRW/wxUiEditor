//////////////////////////////////////////////////////////////////////////
// Purpose:   Common Book utilities
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bookctrl.h>  // wxBookCtrlBase: common base class for wxList/Tree/Notebook

#include "gen_common.h"  // Common component functions
#include "node.h"        // Node class

#include "gen_book_utils.h"

bool isBookDisplayImages(Node* node)
{
    if (!node->isGen(gen_BookPage))
        return node->prop_as_bool(prop_display_images);
    for (auto node_parent = node->GetParent(); node_parent; node_parent = node_parent->GetParent())
    {
        if (!node_parent->isGen(gen_BookPage))
            return node_parent->prop_as_bool(prop_display_images);
    }
    return false;
}

bool isBookHasImage(Node* node)
{
    bool is_book = !node->isGen(gen_BookPage);

    for (const auto& child_node: node->GetChildNodePtrs())
    {
        if (child_node->isGen(gen_BookPage))
        {
            if (child_node->HasValue(prop_bitmap))
                return true;
            if (is_book && !node->isGen(gen_wxTreebook))
                continue;

            for (const auto& grand_child: child_node->GetChildNodePtrs())
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
#if 1
        wxBookCtrlBase::Images bundle_list;
        for (const auto& child_node: node_book->GetChildNodePtrs())
        {
            if (child_node->HasValue(prop_bitmap))
            {
                bundle_list.push_back(child_node->prop_as_wxBitmapBundle(prop_bitmap));
            }

            if (node_book->isGen(gen_wxTreebook))
            {
                AddTreebookSubImages(child_node.get(), bundle_list);
            }
        }
        auto book = wxStaticCast(widget, wxBookCtrlBase);
        book->SetImages(bundle_list);
#else
        // Don't remove this section -- we need to use it to compare with our code generation for pre-3.1.6 code

        wxImageList* img_list = nullptr;

        for (const auto& child_node: node_book->GetChildNodePtrs())
        {
            if (child_node->HasValue(prop_bitmap))
            {
                auto img = wxGetApp().GetImage(child_node->prop_as_string(prop_bitmap));
                ASSERT(img.IsOk());
                if (!img_list)
                {
                    img_list = new wxImageList(img.GetWidth(), img.GetHeight());
                    img_list->Add(img);
                }
                else
                {
                    auto size = img_list->GetSize();
                    // If the image is already the desired size, then Scale() will return immediately without doing anything
                    img_list->Add(img.Scale(size.x, size.y));
                }
            }

            if (node_book->isGen(gen_wxTreebook) && img_list)
            {
                AddTreebookSubImages(child_node.get(), img_list);
            }
        }

        auto book = wxStaticCast(widget, wxBookCtrlBase);
        book->AssignImageList(img_list);
#endif
    }
}

void BookCtorAddImagelist(ttlib::cstr& code, Node* node)
{
    if ((node->prop_as_bool(prop_display_images) || node->isGen(gen_wxToolbook)) && isBookHasImage(node))
    {
        code.insert(0, "\t");

        // Enclose the code in braces to allow using "img_list" and "bmp" as variable names, as well as making the
        // code more readable.

        code << "\n\t{";
        if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            code << "\n#if wxCHECK_VERSION(3, 1, 6)";
        }

        code << "\n\t\twxBookCtrlBase::Images bundle_list;";
        for (const auto& child_node: node->GetChildNodePtrs())
        {
            if (child_node->HasValue(prop_bitmap))
            {
                ttlib::cstr bundle_code;
                if (GenerateBundleCode(child_node->prop_as_string(prop_bitmap), bundle_code))
                {
                    code << "\n\t" << bundle_code;
                    code << "\n\t\t\tbundle_list.push_back(wxBitmapBundle::FromBitmaps(bitmaps));";
                    code << "\n\t\t}";
                }
                else
                {
                    code << "\n\t\tbundle_list.push_back(";
                    code << bundle_code;
                    code << ");";
                }
            }
        }
        code << "\n\t" << node->get_node_name() << "->SetImages(bundle_list);";

        if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            code << "\n\n#else  // older version of wxWidgets that don't support bitmap bundles\n";

            code << "\n\t\tauto img_list = new wxImageList;";

            size_t image_index = 0;
            for (const auto& child_node: node->GetChildNodePtrs())
            {
                // Note: when we generate the code, we could look at the actual image and determine whether it's already
                // the correct size and only scale it if needed. However, that requires the user to know to regenerate
                // the code any time the image is changed to ensure it has the correct dimensions.

                if (child_node->HasValue(prop_bitmap))
                {
                    code << "\n\t\tauto img_" << image_index << " = ";
                    code << GenerateBitmapCode(child_node->prop_as_string(prop_bitmap)) << ";";
                    code << "\n\t\timg_list->Add(img_" << image_index;
                    if (child_node->prop_as_string(prop_bitmap).starts_with("Art;"))
                        code << ".ConvertToImage()";
                    code << ");";
                    ++image_index;
                }
                if (node->isGen(gen_wxTreebook))
                {
                    // This is a recursive function that will handle unlimited nesting
                    AddTreebookImageCode(code, child_node.get(), image_index);
                }
            }
            code << "\n\t" << node->get_node_name() << "->AssignImageList(img_list);";

            code << "\n#endif  // wxCHECK_VERSION(3, 1, 6)";
        }
        code << "\n\t}";
    }
}

void AddTreebookSubImages(Node* node, wxBookCtrlBase::Images& bundle_list)
{
    for (const auto& child_node: node->GetChildNodePtrs())
    {
        if (child_node->isGen(gen_BookPage))
        {
            if (child_node->HasValue(prop_bitmap))
            {
                bundle_list.push_back(child_node->prop_as_wxBitmapBundle(prop_bitmap));
            }
            AddTreebookSubImages(child_node.get(), bundle_list);
        }
    }
}

void AddTreebookImageCode(ttlib::cstr& code, Node* child_node, size_t& image_index)
{
    for (const auto& grand_child: child_node->GetChildNodePtrs())
    {
        if (grand_child->isGen(gen_BookPage) && grand_child->HasValue(prop_bitmap))
        {
            code << "\n\t\tauto img_" << image_index << " = ";
            code << GenerateBitmapCode(grand_child->prop_as_string(prop_bitmap)) << ";";
            code << "\n\t\timg_list->Add(img_" << image_index;
            if (grand_child->prop_as_string(prop_bitmap).starts_with("Art;"))
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

    auto treebook = node->GetParent();
    while (treebook->isGen(gen_BookPage))
    {
        treebook = treebook->GetParent();
    }

    for (const auto& child_node: treebook->GetChildNodePtrs())
    {
        if (child_node.get() == node)
            return idx_image;
        if (child_node->HasValue(prop_bitmap))
            ++idx_image;
        for (const auto& grand_child: child_node->GetChildNodePtrs())
        {
            if (grand_child->isGen(gen_BookPage))
            {
                if (grand_child.get() == node)
                {
                    return idx_image;
                }
                if (grand_child->HasValue(prop_bitmap))
                    ++idx_image;
            }
        }
    }

    return idx_image;
}
