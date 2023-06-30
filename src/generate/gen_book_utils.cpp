//////////////////////////////////////////////////////////////////////////
// Purpose:   Common Book utilities
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bookctrl.h>  // wxBookCtrlBase: common base class for wxList/Tree/Notebook

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // Common component functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "gen_book_utils.h"

bool isBookDisplayImages(Node* node)
{
    if (!node->isGen(gen_BookPage))
        return node->as_bool(prop_display_images);
    for (auto node_parent = node->GetParent(); node_parent; node_parent = node_parent->GetParent())
    {
        if (!node_parent->isGen(gen_BookPage))
            return node_parent->as_bool(prop_display_images);
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
                auto img = wxGetApp().GetImage(child_node->as_string(prop_bitmap));
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

void BookCtorAddImagelist(Code& code)
{
    if ((code.IsTrue(prop_display_images) || code.IsGen(gen_wxToolbook)) && isBookHasImage(code.node()))
    {
        code.OpenBrace();
        if (code.is_cpp() && Project.value(prop_wxWidgets_version) == "3.1")
        {
            if (code.back() == '\t')
            {
                // Remove the trailing tab so that writeline() will know not to indend the #if
                // statement.
                code.pop_back();
            }

            code << "#if wxCHECK_VERSION(3, 1, 6)";
        }
        code.Eol(eol_if_needed);
        if (code.is_cpp())
        {
            code.Eol(eol_if_needed) << "wxBookCtrlBase::Images bundle_list;";
            for (const auto& child_node: code.node()->GetChildNodePtrs())
            {
                if (child_node->HasValue(prop_bitmap))
                {
                    tt_string bundle_code;
                    if (GenerateBundleCode(child_node->as_string(prop_bitmap), bundle_code))
                    {
                        code.Eol() << bundle_code;
                        code.Eol() << "bundle_list.push_back(wxBitmapBundle::FromBitmaps(bitmaps));";
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
        else  // wxPython code
        {
            int bitmap_index = 0;
            for (const auto& child_node: code.node()->GetChildNodePtrs())
            {
                if (child_node->HasValue(prop_bitmap))
                {
                    Code bundle_code(child_node.get(), GEN_LANG_PYTHON);
                    PythonBundleCode(bundle_code, prop_bitmap);
                    code.Eol().Str("bundle_").itoa(++bitmap_index).Str(" = ") << bundle_code;
                }
            }
            code.Eol().Str("bundle_list = [");
            code.Indent();
            for (int index = 1; index <= bitmap_index; ++index)
            {
                if (index > 1)
                {
                    code.Str(",");
                }
                code.Eol();
                code.Str("bundle_").itoa(index);
            }
            code.Unindent();
            code.Eol().Str("]");
        }

        code.Eol().NodeName().Function("SetImages(bundle_list").EndFunction();

        if (code.is_cpp() && Project.value(prop_wxWidgets_version) == "3.1")
        {
            // Don't use Eol() here, because we want to insert the #if at the beginning of the line
            code << "\n\n#else  // older version of wxWidgets that don't support bitmap bundles\n";

            code.Eol() << "auto img_list = new wxImageList;";

            size_t image_index = 0;
            for (const auto& child_node: code.node()->GetChildNodePtrs())
            {
                // Note: when we generate the code, we could look at the actual image and determine whether it's already
                // the correct size and only scale it if needed. However, that requires the user to know to regenerate
                // the code any time the image is changed to ensure it has the correct dimensions.

                if (child_node->HasValue(prop_bitmap))
                {
                    code.Eol() << "auto img_" << image_index << " = ";
                    code << GenerateBitmapCode(child_node->as_string(prop_bitmap)) << ";";
                    code.Eol() << "img_list->Add(img_" << image_index;
                    if (child_node->as_string(prop_bitmap).starts_with("Art;"))
                        code << ".ConvertToImage()";
                    code.EndFunction();
                    ++image_index;
                }
                if (code.IsGen(gen_wxTreebook))
                {
                    // This is a recursive function that will handle unlimited nesting
                    AddTreebookImageCode(code, child_node.get(), image_index);
                }
            }
            code.Eol().NodeName().Function("AssignImageList(img_list").EndFunction();
            // Don't use Eol() here, because we want to insert the #if at the beginning of the line
            code << "\n#endif  // wxCHECK_VERSION(3, 1, 6)";
        }

        code.CloseBrace();
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

void AddTreebookImageCode(tt_string& code, Node* child_node, size_t& image_index)
{
    for (const auto& grand_child: child_node->GetChildNodePtrs())
    {
        if (grand_child->isGen(gen_BookPage) && grand_child->HasValue(prop_bitmap))
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
