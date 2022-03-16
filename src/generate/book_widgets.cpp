//////////////////////////////////////////////////////////////////////////
// Purpose:   Book component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>     // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/choicebk.h>     // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/event.h>        // Event classes
#include <wx/listbook.h>     // wxListbook: wxListView and wxNotebook combination
#include <wx/notebook.h>     // wxNotebook interface
#include <wx/simplebook.h>   // wxBookCtrlBase-derived class without any controller.
#include <wx/toolbook.h>     // wxToolbook: wxToolBar and wxNotebook combination
#include <wx/treebook.h>     // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "../mockup/mockup_content.h"  // MockupContent -- Mockup of a form's contents

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "mainapp.h"     // App -- Main application class
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "book_widgets.h"

// Walks up the parent tree until it finds a Book and returns whether or not the book is
// supposed to display images. This function will handle wxTreeBook with any depth of sub
// pages.
static bool isBookDisplayImages(Node* node);

// This will walk through all of a book's pages to see if any of them have an image. This
// will handle wxTreebook with an unlimited number of nested sub pages.
static bool isBookHasImage(Node* node);

static void AddBookImageList(Node* node_book, wxObject* widget);
static void BookCtorAddImagelist(ttlib::cstr& code, Node* node);
static void AddTreebookSubImages(Node* node, wxImageList* img_list);
static void AddTreebookSubImages(Node* node, wxBookCtrlBase::Images& bundle_list);
static void AddTreebookImageCode(ttlib::cstr& code, Node* node, size_t& image_index);
static int GetTreebookImageIndex(Node* node);

//////////////////////////////////////////  BookPageGenerator  //////////////////////////////////////////

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
            for (auto& child: grandparent->GetChildNodePtrs())
            {
                if (child->HasValue(prop_bitmap))
                    ++idx_image;
                for (auto& grand_child: child->GetChildNodePtrs())
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
            for (auto& child: node_parent->GetChildNodePtrs())
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
                    for (auto& grand_child: child->GetChildNodePtrs())
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
            }
            if (!node->prop_as_bool(prop_select))
                code << ", false";
            code << ", " << idx_image;
        }

        code << ");";
    }
    return code;
}

//////////////////////////////////////////  PageCtrlGenerator  //////////////////////////////////////////

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
            for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child, ++idx_image)
            {
                if (node_parent->GetChild(idx_child) == node)
                {
                    if (idx_image < 0)
                        idx_image = 0;
                    break;
                }
            }
            book->AddPage(wxStaticCast(widget, wxWindow), node->prop_as_wxString(prop_label), false, idx_image);
        }
        else if (node->HasValue(prop_bitmap) && node_parent->prop_as_bool(prop_display_images))
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

            book->AddPage(wxStaticCast(widget, wxWindow), node->prop_as_wxString(prop_label), false, idx_image);
        }
        else
        {
            book->AddPage(wxStaticCast(widget, wxWindow), node->prop_as_wxString(prop_label));
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

                aui_book->AddPage(wxStaticCast(widget, wxWindow), node->prop_as_wxString(prop_label), false, idx_image);
            }
            else
            {
                aui_book->AddPage(wxStaticCast(widget, wxWindow), node->prop_as_wxString(prop_label));
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

    return widget;
}

std::optional<ttlib::cstr> PageCtrlGenerator::GenConstruction(Node* node)
{
    if (!node->GetChildCount())
        return {};

    auto child_node = node->GetChild(0);

    auto child_generator = child_node->GetGenerator();
    ASSERT(child_generator);

    auto result = child_generator->GenConstruction(child_node);
    if (!result)
    {
        return {};
    }

    ttlib::cstr code;
    code << result.value() << '\n';
    code << GetParentName(node) << "->AddPage(" << child_node->get_node_name() << ", ";
    code << GenerateQuotedString(node, prop_label);

    // Default is false, so only add parameter if it is true.
    if (node->prop_as_bool(prop_select))
        code << ", true";

    if (node->HasValue(prop_bitmap) &&
        (node->GetParent()->prop_as_bool(prop_display_images) || node->isParent(gen_wxToolbook)))
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

        if (!node->prop_as_bool(prop_select))
            code << ", false";
        code << ", " << idx_image;
    }

    code << ");";

    return code;
}

//////////////////////////////////////////  NotebookGenerator  //////////////////////////////////////////

wxObject* NotebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &NotebookGenerator::OnPageChanged, this);

    return widget;
}

void NotebookGenerator::OnPageChanged(wxNotebookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxNotebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> NotebookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxNotebook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    BookCtorAddImagelist(code, node);

    return code;
}

std::optional<ttlib::cstr> NotebookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool NotebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/notebook.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

//////////////////////////////////////////  AuiNotebookGenerator  //////////////////////////////////////////

wxObject* AuiNotebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxAuiNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                    DlgSize(parent, node, prop_size), GetStyleInt(node));
    if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiGenericTabArt"))
        widget->SetArtProvider(new wxAuiGenericTabArt());
    else if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiSimpleTabArt"))
        widget->SetArtProvider(new wxAuiSimpleTabArt());

    if (node->prop_as_int(prop_tab_height) > 0)
        widget->SetTabCtrlHeight(node->prop_as_int(prop_tab_height));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &AuiNotebookGenerator::OnPageChanged, this);

    return widget;
}

void AuiNotebookGenerator::OnPageChanged(wxNotebookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxNotebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> AuiNotebookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxAuiNotebook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    BookCtorAddImagelist(code, node);

    if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiGenericTabArt"))
    {
        code << "\n\t" << node->get_node_name() << "->SetArtProvider(new wxAuiGenericTabArt());";
    }
    else if (node->prop_as_string(prop_art_provider).is_sameas("wxAuiSimpleTabArt"))
    {
        code << "\n\t" << node->get_node_name() << "->SetArtProvider(new wxAuiSimpleTabArt());";
    }

    return code;
}

std::optional<ttlib::cstr> AuiNotebookGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    if (node->prop_as_int(prop_tab_height) > 0)
    {
        ttlib::cstr code;
        code << node->get_node_name() << "->SetTabCtrlHeight(" << node->prop_as_string(prop_tab_height) << ");";
        return code;
    }
    return {};
}

std::optional<ttlib::cstr> AuiNotebookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool AuiNotebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/aui/auibook.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

//////////////////////////////////////////  ChoicebookGenerator  //////////////////////////////////////////

wxObject* ChoicebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxChoicebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_CHOICEBOOK_PAGE_CHANGED, &ChoicebookGenerator::OnPageChanged, this);

    return widget;
}

void ChoicebookGenerator::OnPageChanged(wxBookCtrlEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxChoicebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> ChoicebookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxChoicebook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> ChoicebookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ChoicebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/choicebk.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }
    return true;
}

//////////////////////////////////////////  ListbookGenerator  //////////////////////////////////////////

wxObject* ListbookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    // Note the currently, wxListbook does not have a "style" property since the only thing that can be set is the
    // label (tab) position
    auto widget = new wxListbook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_LISTBOOK_PAGE_CHANGED, &ListbookGenerator::OnPageChanged, this);

    return widget;
}

void ListbookGenerator::OnPageChanged(wxListbookEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxListbook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> ListbookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxListbook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    BookCtorAddImagelist(code, node);

    return code;
}

std::optional<ttlib::cstr> ListbookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ListbookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/listbook.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

//////////////////////////////////////////  ToolbookGenerator  //////////////////////////////////////////

wxObject* ToolbookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxToolbook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    wxBookCtrlBase::Images bundle_list;
    for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
    {
        if (node->GetChild(idx_child)->HasValue(prop_bitmap))
        {
            bundle_list.push_back(node->GetChild(idx_child)->prop_as_wxBitmapBundle(prop_bitmap));
        }
    }
    auto book = wxStaticCast(widget, wxBookCtrlBase);
    book->SetImages(bundle_list);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_TOOLBOOK_PAGE_CHANGED, &ToolbookGenerator::OnPageChanged, this);

    return widget;
}

void ToolbookGenerator::OnPageChanged(wxBookCtrlEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxToolbook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> ToolbookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxToolbook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    BookCtorAddImagelist(code, node);

    return code;
}

std::optional<ttlib::cstr> ToolbookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ToolbookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/toolbook.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  TreebookGenerator  //////////////////////////////////////////

wxObject* TreebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTreebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    AddBookImageList(node, widget);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_TREEBOOK_PAGE_CHANGED, &TreebookGenerator::OnPageChanged, this);

    return widget;
}

void TreebookGenerator::OnPageChanged(wxBookCtrlEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxTreebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> TreebookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxTreebook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);
    BookCtorAddImagelist(code, node);

    return code;
}

std::optional<ttlib::cstr> TreebookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool TreebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/treebook.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/treebook.h>");
    }

    return true;
}

//////////////////////////////////////////  SimplebookGenerator  //////////////////////////////////////////

wxObject* SimplebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSimplebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetEffects((wxShowEffect) node->prop_as_mockup(prop_show_effect, "info_"),
                       (wxShowEffect) node->prop_as_mockup(prop_hide_effect, "info_"));
    if (node->HasValue(prop_duration))
    {
        widget->SetEffectTimeout(node->prop_as_int(prop_duration));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_BOOKCTRL_PAGE_CHANGED, &SimplebookGenerator::OnPageChanged, this);

    return widget;
}

void SimplebookGenerator::OnPageChanged(wxBookCtrlEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxSimplebook);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> SimplebookGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSimplebook(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> SimplebookGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    if (node->prop_as_string(prop_show_effect) != "no effects" || node->prop_as_string(prop_hide_effect) != "no effects")
    {
        ttlib::cstr code;
        code << '\t' << node->get_node_name() << "->SetEffects(" << node->prop_as_constant(prop_show_effect, "info_") << ", "
             << node->prop_as_constant(prop_hide_effect, "info_") << ");";

        if (node->prop_as_int(prop_duration))
        {
            code << "\n\t" << node->get_node_name() << "->SetEffectTimeout(" << node->prop_as_string(prop_duration) << ");";
        }

        return code;
    }
    else
    {
        return {};
    }
}

std::optional<ttlib::cstr> SimplebookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool SimplebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/Simplebk.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  Book utility functions  //////////////////////////////////////////

static void AddBookImageList(Node* node_book, wxObject* widget)
{
    if (isBookDisplayImages(node_book) && isBookHasImage(node_book))
    {
#if 1
        wxBookCtrlBase::Images bundle_list;
        for (auto& child_node: node_book->GetChildNodePtrs())
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

        for (auto& child_node: node_book->GetChildNodePtrs())
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

static void BookCtorAddImagelist(ttlib::cstr& code, Node* node)
{
    if ((node->prop_as_bool(prop_display_images) || node->isGen(gen_wxToolbook)) && isBookHasImage(node))
    {
        code.insert(0, "\t");

        // Enclose the code in braces to allow using "img_list" and "bmp" as variable names, as well as making the
        // code more readable.

        code << "\n\t{";
        code << "\n\t\tauto img_list = new wxImageList;";

        size_t image_index = 0;
        for (auto& child_node: node->GetChildNodePtrs())
        {
            // Note: when we generate the code, we could look at the actual image and determine whether it's already
            // the correct size and only scale it if needed. However, that requires the user to know to regenerate
            // the code any time the image is changed to ensure it has the correct dimensions.

            if (child_node->HasValue(prop_bitmap))
            {
                code << "\n\t\tauto img_" << image_index << " = ";
                code << GenerateBitmapCode(child_node->prop_as_string(prop_bitmap)) << ";";
                code << "\n\t\timg_list->Add(img_" << image_index;
                if (child_node->prop_as_string(prop_bitmap).is_sameprefix("Art;"))
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
        code << "\n\t\t" << node->get_node_name() << "->AssignImageList(img_list);";
        code << "\n\t}";
    }
}

static bool isBookDisplayImages(Node* node)
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

static bool isBookHasImage(Node* node)
{
    bool is_book = !node->isGen(gen_BookPage);

    for (auto& child_node: node->GetChildNodePtrs())
    {
        if (child_node->isGen(gen_BookPage))
        {
            if (child_node->HasValue(prop_bitmap))
                return true;
            if (is_book && !node->isGen(gen_wxTreebook))
                continue;

            for (auto& grand_child: child_node->GetChildNodePtrs())
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

static void AddTreebookSubImages(Node* node, wxImageList* img_list)
{
    if (!img_list)
        return;

    for (auto& child_node: node->GetChildNodePtrs())
    {
        if (child_node->isGen(gen_BookPage))
        {
            if (child_node->HasValue(prop_bitmap))
            {
                auto img = wxGetApp().GetImage(child_node->prop_as_string(prop_bitmap));
                ASSERT(img.IsOk());

                auto size = img_list->GetSize();
                // If the image is already the desired size, then Scale() will return immediately without doing anything
                img_list->Add(img.Scale(size.x, size.y));
            }
            AddTreebookSubImages(child_node.get(), img_list);
        }
    }
}

static void AddTreebookSubImages(Node* node, wxBookCtrlBase::Images& bundle_list)
{
    for (auto& child_node: node->GetChildNodePtrs())
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

static void AddTreebookImageCode(ttlib::cstr& code, Node* child_node, size_t& image_index)
{
    for (auto& grand_child: child_node->GetChildNodePtrs())
    {
        if (grand_child->isGen(gen_BookPage) && grand_child->HasValue(prop_bitmap))
        {
            code << "\n\t\tauto img_" << image_index << " = ";
            code << GenerateBitmapCode(grand_child->prop_as_string(prop_bitmap)) << ";";
            code << "\n\t\timg_list->Add(img_" << image_index;
            if (grand_child->prop_as_string(prop_bitmap).is_sameprefix("Art;"))
                code << ".ConvertToImage()";
            code << ");";
            ++image_index;
            AddTreebookImageCode(code, grand_child.get(), image_index);
        }
    }
}

static int GetTreebookImageIndex(Node* node)
{
    int idx_image = 0;

    auto treebook = node->GetParent();
    while (treebook->isGen(gen_BookPage))
    {
        treebook = treebook->GetParent();
    }

    for (auto& child_node: treebook->GetChildNodePtrs())
    {
        if (child_node.get() == node)
            return idx_image;
        if (child_node->HasValue(prop_bitmap))
            ++idx_image;
        for (auto& grand_child: child_node->GetChildNodePtrs())
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
