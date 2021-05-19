//////////////////////////////////////////////////////////////////////////
// Purpose:   Book component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/bookctrl.h>    // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/choicebk.h>    // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/event.h>       // Event classes
#include <wx/listbook.h>    // wxListbook: wxListView and wxNotebook combination
#include <wx/notebook.h>    // wxNotebook interface
#include <wx/simplebook.h>  // wxBookCtrlBase-derived class without any controller.
#include <wx/toolbook.h>    // wxToolbook: wxToolBar and wxNotebook combination
#include <wx/treebook.h>    // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "mainapp.h"     // App -- Main application class
#include "node.h"        // Node class

#include "book_widgets.h"

// These dimensions match the default size in containers.xml -- if you change them here, then you must also change every
// "image_size" property in containers.xml. Doing so will break any project that has these values as the default, so you will
// also need to do a project version increase and convert down-level projects. Bottom line: don't change these values!

constexpr const int DEF_TAB_IMG_WIDTH = 16;
constexpr const int DEF_TAB_IMG_HEIGHT = 16;

static void AddBookImageList(Node* node, wxObject* widget);
static void BookCtorAddImagelist(ttlib::cstr& code, Node* node);

//////////////////////////////////////////  BookPageGenerator  //////////////////////////////////////////

wxObject* BookPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                    node->prop_as_wxSize(prop_size), node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    auto node_parent = node->GetParent();
    auto book = wxDynamicCast(parent, wxBookCtrlBase);
    if (book)
    {
        if (node_parent->isGen(gen_wxToolbook))
        {
            int idx_image = 0;
            for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child, ++idx_image)
            {
                if (node_parent->GetChild(idx_child) == node)
                    break;
            }
            book->AddPage(widget, node->prop_as_wxString(prop_label), false, idx_image);
        }
        else if (node->HasValue(prop_bitmap) && node_parent->prop_as_bool(prop_display_images))
        {
            int idx_image = 0;
            for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child)
            {
                if (node_parent->GetChild(idx_child) == node)
                    break;
                if (node_parent->GetChild(idx_child)->HasValue(prop_bitmap) || node_parent->isGen(gen_wxToolbook))
                    ++idx_image;
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
    code << node->get_node_name() << " = new wxPanel(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    code << '\n';
    code << GetParentName(node) << "->AddPage(" << node->get_node_name() << ", ";
    code << GenerateQuotedString(node, prop_label);

    // Default is false, so only add parameter if it is true.
    if (node->prop_as_bool(prop_select))
        code << ", true";

    if (node->HasValue(prop_bitmap) &&
        (node->GetParent()->prop_as_bool(prop_display_images) || node->isParent(gen_wxToolbook)))
    {
        auto node_parent = node->GetParent();
        int idx_image = 0;
        for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child)
        {
            if (node_parent->GetChild(idx_child) == node)
                break;
            if (node_parent->GetChild(idx_child)->HasValue(prop_bitmap))
                ++idx_image;
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
    auto widget = new wxNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                 node->prop_as_wxSize(prop_size),
                                 node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

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
    auto size = node->prop_as_wxSize(prop_bitmapsize);
    if (size.x != -1 || size.y != -1)
    {
        InsertGeneratorInclude(node, "#include <wx/imaglist.h>", set_src, set_hdr);
        InsertGeneratorInclude(node, "#include <wx/image.h>", set_src, set_hdr);
    }

    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

//////////////////////////////////////////  ChoicebookGenerator  //////////////////////////////////////////

wxObject* ChoicebookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxChoicebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                   node->prop_as_wxSize(prop_size),
                                   node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

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
    auto widget = new wxListbook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                 node->prop_as_wxSize(prop_size),
                                 node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

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
    auto size = node->prop_as_wxSize(prop_bitmapsize);
    if (size != wxDefaultSize)
    {
        InsertGeneratorInclude(node, "#include <wx/imaglist.h>", set_src, set_hdr);
        InsertGeneratorInclude(node, "#include <wx/image.h>", set_src, set_hdr);
    }

    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

//////////////////////////////////////////  ToolbookGenerator  //////////////////////////////////////////

wxObject* ToolbookGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxToolbook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                 node->prop_as_wxSize(prop_size),
                                 node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    // A toolbook always has images, so we can't use AddBookImageList

    auto size = node->prop_as_wxSize(prop_bitmapsize);
    if (size.GetWidth() == -1)
    {
        size.SetWidth(DEF_TAB_IMG_WIDTH);
    }
    if (size.GetHeight() == -1)
    {
        size.SetHeight(DEF_TAB_IMG_HEIGHT);
    }

    auto img_list = new wxImageList(size.x, size.y);

    for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
    {
        if (node->GetChild(idx_child)->HasValue(prop_bitmap))
        {
            auto img = wxGetApp().GetImage(node->GetChild(idx_child)->prop_as_string(prop_bitmap));
            ASSERT(img.IsOk());
            img_list->Add(img.Scale(size.x, size.y));
        }
        else
        {
            auto img = GetInternalImage("unknown");
            ASSERT(img.IsOk());
            img_list->Add(img.Scale(size.x, size.y));
        }
    }

    auto book = wxStaticCast(widget, wxBookCtrlBase);
    book->AssignImageList(img_list);

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
    auto widget = new wxTreebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                 node->prop_as_wxSize(prop_size),
                                 node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

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
    auto widget = new wxSimplebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                   node->prop_as_wxSize(prop_size), node->prop_as_int(prop_window_style));

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

static void AddBookImageList(Node* node, wxObject* widget)
{
    if (node->prop_as_bool(prop_display_images))
    {
        bool has_bitmaps = false;
        for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
        {
            if (node->GetChild(idx_child)->HasValue(prop_bitmap))
            {
                has_bitmaps = true;
                break;
            }
        }

        if (has_bitmaps)
        {
            auto size = node->prop_as_wxSize(prop_bitmapsize);
            if (size.x == -1)
            {
                size.x = DEF_TAB_IMG_WIDTH;
            }
            if (size.y == -1)
            {
                size.y = DEF_TAB_IMG_HEIGHT;
            }

            auto img_list = new wxImageList(size.x, size.y);

            for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
            {
                if (node->GetChild(idx_child)->HasValue(prop_bitmap))
                {
                    auto img = wxGetApp().GetImage(node->GetChild(idx_child)->prop_as_string(prop_bitmap));
                    ASSERT(img.IsOk());
                    img_list->Add(img.Scale(size.x, size.y));
                }
            }

            auto book = wxStaticCast(widget, wxBookCtrlBase);
            book->AssignImageList(img_list);
        }
    }
}

static void BookCtorAddImagelist(ttlib::cstr& code, Node* node)
{
    if (node->prop_as_bool(prop_display_images) || node->isGen(gen_wxToolbook))
    {
        bool has_bitmaps = false;
        for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
        {
            if (node->GetChild(idx_child)->HasValue(prop_bitmap))
            {
                has_bitmaps = true;
                break;
            }
        }

        if (has_bitmaps)
        {
            code.insert(0, "    ");
            auto size = node->prop_as_wxSize(prop_bitmapsize);
            if (size.x == -1)
            {
                size.x = DEF_TAB_IMG_WIDTH;
            }
            if (size.y == -1)
            {
                size.y = DEF_TAB_IMG_HEIGHT;
            }

            // Enclose the code in braces to allow using "img_list" and "bmp" as variable names, as well as making the code
            // more readable.

            code << "\n    {";
            code << "\n        auto img_list = new wxImageList(";
            code << size.x << ", " << size.y << ");";

            for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
            {
                // Note: when we generate the code, we could look at the actual image and determine whether it's already the
                // correct size and only scale it if needed. However, that requires the user to know to regenerate the code
                // any time the image is changed to ensure it has the correct dimensions.

                if (node->GetChild(idx_child)->HasValue(prop_bitmap))
                {
                    code << "\n        auto img_" << idx_child << " = ";
                    code << GenerateBitmapCode(node->GetChild(idx_child)->prop_as_string(prop_bitmap)) << ";";
                    code << "\n        img_list->Add(img_" << idx_child;
                    if (node->GetChild(idx_child)->prop_as_string(prop_bitmap).is_sameprefix("Art;"))
                        code << ".ConvertToImage()";
                    code << ".Scale(" << size.x << ", " << size.y << "));";
                }
            }
            code << "\n        " << node->get_node_name() << "->AssignImageList(img_list);";
            code << "\n    }";
        }
    }
}
