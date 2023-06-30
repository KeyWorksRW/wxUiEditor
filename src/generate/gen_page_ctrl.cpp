//////////////////////////////////////////////////////////////////////////
// Purpose:   Page control generator
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

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_page_ctrl.h"

wxObject* PageCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    if (!node->GetChildCount())
        return nullptr;

    auto child_generator = node->GetChild(0)->GetGenerator();
    ASSERT(child_generator);
    auto widget = child_generator->CreateMockup(node->GetChild(0), parent);
    ASSERT(widget);

    auto node_parent = node->GetParent();

    if (auto book = wxDynamicCast(parent, wxBookCtrlBase); book)
    {
        if (node_parent->isGen(gen_wxToolbook))
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
                else
                {
                    ++idx_image;
                }
            }
            book->AddPage(wxStaticCast(widget, wxWindow), node->as_wxString(prop_label), false, idx_image);
        }
        else if (node->HasValue(prop_bitmap) && node_parent->as_bool(prop_display_images))
        {
            int idx_image = -1;
            for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child)
            {
                if (node_parent->GetChild(idx_child) == node)
                {
                    if (idx_image < 0)
                        idx_image = 0;
                    break;
                }
                if (node_parent->GetChild(idx_child)->HasValue(prop_bitmap) || node_parent->isGen(gen_wxToolbook))
                {
                    if (idx_image < 0)
                        idx_image = 0;

                    ++idx_image;
                }
            }

            book->AddPage(wxStaticCast(widget, wxWindow), node->as_wxString(prop_label), false, idx_image);
        }
        else
        {
            book->AddPage(wxStaticCast(widget, wxWindow), node->as_wxString(prop_label));
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
                int idx_image = 0;
                for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child)
                {
                    if (node_parent->GetChild(idx_child) == node)
                    {
                        if (idx_image < 0)
                            idx_image = 0;

                        break;
                    }
                    if (node_parent->GetChild(idx_child)->HasValue(prop_bitmap))
                    {
                        if (idx_image < 0)
                            idx_image = 0;
                        ++idx_image;
                    }
                }

                aui_book->AddPage(wxStaticCast(widget, wxWindow), node->as_wxString(prop_label), false, idx_image);
            }
            else
            {
                aui_book->AddPage(wxStaticCast(widget, wxWindow), node->as_wxString(prop_label));
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

    return widget;
}

bool PageCtrlGenerator::ConstructionCode(Code& code)
{
    Node* node = code.node();
    if (!node->GetChildCount())
        return false;

    if (auto child_node = code.node()->GetChild(0); child_node)
    {
        if (auto child_generator = child_node->GetGenerator(); child_generator)
        {
            Code gen_code(child_node, code.m_language);
            if (child_generator->ConstructionCode(gen_code))
            {
                code += gen_code.GetCode();
                code.Eol(eol_if_needed).ValidParentName().Function("AddPage(");
                code.Str(child_node->get_node_name()).Comma().QuotedString(prop_label);

                // Default is false, so only add parameter if it is true.
                if (code.IsTrue(prop_select))
                    code.Comma().AddTrue();

                if (node->HasValue(prop_bitmap) &&
                    (node->GetParent()->as_bool(prop_display_images) || node->isParent(gen_wxToolbook)))
                {
                    auto node_parent = node->GetParent();
                    int idx_image = -1;
                    for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child)
                    {
                        if (node_parent->GetChild(idx_child) == node)
                        {
                            if (idx_image < 0)
                                idx_image = 0;

                            break;
                        }
                        if (node_parent->GetChild(idx_child)->HasValue(prop_bitmap))
                        {
                            if (idx_image < 0)
                                idx_image = 0;

                            ++idx_image;
                        }
                    }

                    // If it is true, then the parameter has already been added
                    if (!node->as_bool(prop_select))
                        code.Comma().AddFalse();
                    code.Comma().itoa(idx_image);
                }
                code.EndFunction();
            }
        }
    }

    return true;
}
