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

wxObject* BookPageGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(txt_pos), node->prop_as_wxSize(txt_size),
                    node->prop_as_int(txt_style) | node->prop_as_int(txt_window_style));

    auto node_parent = node->GetParent();
    auto book = wxDynamicCast(parent, wxBookCtrlBase);
    if (book)
    {
        if (node_parent->GetClassName() == "wxToolbook")
        {
            int idx_image = 0;
            for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child, ++idx_image)
            {
                if (node_parent->GetChild(idx_child) == node)
                    break;
            }
            book->AddPage(widget, node->prop_as_wxString(txt_label), false, idx_image);
        }
        else if (node->HasValue("bitmap") && node_parent->prop_as_bool("display_images"))
        {
            int idx_image = 0;
            for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child)
            {
                if (node_parent->GetChild(idx_child) == node)
                    break;
                if (node_parent->GetChild(idx_child)->HasValue("bitmap") || node_parent->GetClassName() == "wxToolbook")
                    ++idx_image;
            }

            book->AddPage(widget, node->prop_as_wxString(txt_label), false, idx_image);
        }
        else
        {
            book->AddPage(widget, node->prop_as_wxString(txt_label));
        }

        auto cur_selection = book->GetSelection();
        if (node->prop_as_bool("select"))
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
    code << GetParentName(node) << ", " << node->prop_as_string("id");

    GeneratePosSizeFlags(node, code);

    code << '\n';
    code << GetParentName(node) << "->AddPage(" << node->get_node_name() << ", ";
    if (node->HasValue(txt_label))
        code << GenerateQuotedString(node->prop_as_string(txt_label));
    else
        code << "wxEmptyString";

    // Default is false, so only add parameter if it is true.
    if (node->prop_as_bool("select"))
        code << ", true";

    if (node->HasValue("bitmap") &&
        (node->GetParent()->prop_as_bool("display_images") || node->GetParent()->GetClassName() == "wxToolbook"))
    {
        auto node_parent = node->GetParent();
        int idx_image = 0;
        for (size_t idx_child = 0; idx_child < node_parent->GetChildCount(); ++idx_child)
        {
            if (node_parent->GetChild(idx_child) == node)
                break;
            if (node_parent->GetChild(idx_child)->HasValue("bitmap"))
                ++idx_image;
        }

        if (!node->prop_as_bool("select"))
            code << ", false";
        code << ", " << idx_image;
    }

    code << ");";

    return code;
}

//////////////////////////////////////////  NotebookGenerator  //////////////////////////////////////////

wxObject* NotebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

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
    code << GetParentName(node) << ", " << node->prop_as_string("id");

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
    auto size = node->prop_as_wxSize("bitmapsize");
    if (size.x != -1 || size.y != -1)
    {
        InsertGeneratorInclude(node, "#include <wx/imaglist.h>", set_src, set_hdr);
        InsertGeneratorInclude(node, "#include <wx/image.h>", set_src, set_hdr);
    }

    if (node->prop_has_value("persist_name"))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

//////////////////////////////////////////  ChoicebookGenerator  //////////////////////////////////////////

wxObject* ChoicebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxChoicebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                         node->prop_as_wxSize("size"), node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

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
    code << GetParentName(node) << ", " << node->prop_as_string("id");

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
    if (node->prop_has_value("persist_name"))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }
    return true;
}

//////////////////////////////////////////  ListbookGenerator  //////////////////////////////////////////

wxObject* ListbookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxListbook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

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
    code << GetParentName(node) << ", " << node->prop_as_string("id");

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
    auto size = node->prop_as_wxSize("bitmapsize");
    if (size.x != -1 || size.y != -1)
    {
        InsertGeneratorInclude(node, "#include <wx/imaglist.h>", set_src, set_hdr);
        InsertGeneratorInclude(node, "#include <wx/image.h>", set_src, set_hdr);
    }

    if (node->prop_has_value("persist_name"))
    {
        set_src.insert("#include <wx/persist/bookctrl.h>");
    }

    return true;
}

//////////////////////////////////////////  ToolbookGenerator  //////////////////////////////////////////

wxObject* ToolbookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxToolbook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    // A toolbook always has images, so we can't use AddBookImageList

    auto size = node->prop_as_wxSize("bitmapsize");
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
        if (node->GetChild(idx_child)->HasValue("bitmap"))
        {
            auto img = wxGetApp().GetImage(node->GetChild(idx_child)->prop_as_string("bitmap"));
            ASSERT(img.IsOk());
            img_list->Add(img.Scale(size.x, size.y));
        }
        else
        {
            auto img = GetXPMImage("unknown");
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
    code << GetParentName(node) << ", " << node->prop_as_string("id");

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

wxObject* TreebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxTreebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

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
    code << GetParentName(node) << ", " << node->prop_as_string("id");

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
    if (node->prop_has_value("persist_name"))
    {
        set_src.insert("#include <wx/persist/treebook.h>");
    }

    return true;
}

//////////////////////////////////////////  SimplebookGenerator  //////////////////////////////////////////

wxObject* SimplebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxSimplebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                                   node->prop_as_wxSize("size"), node->prop_as_int("window_style"));

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
    code << GetParentName(node) << ", " << node->prop_as_string("id");

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
    if (node->prop_as_bool("display_images"))
    {
        bool has_bitmaps = false;
        for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
        {
            if (node->GetChild(idx_child)->HasValue("bitmap"))
            {
                has_bitmaps = true;
                break;
            }
        }

        if (has_bitmaps)
        {
            auto size = node->prop_as_wxSize("bitmapsize");
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
                if (node->GetChild(idx_child)->HasValue("bitmap"))
                {
                    auto img = wxGetApp().GetImage(node->GetChild(idx_child)->prop_as_string("bitmap"));
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
    if (node->prop_as_bool("display_images") || node->GetClassName() == "wxToolbook")
    {
        bool has_bitmaps = false;
        for (size_t idx_child = 0; idx_child < node->GetChildCount(); ++idx_child)
        {
            if (node->GetChild(idx_child)->HasValue("bitmap"))
            {
                has_bitmaps = true;
                break;
            }
        }

        if (has_bitmaps)
        {
            code.insert(0, "    ");
            auto size = node->prop_as_wxSize("bitmapsize");
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

                if (node->GetChild(idx_child)->HasValue("bitmap"))
                {
                    code << "\n        auto img_" << idx_child << " = ";
                    code << GenerateBitmapCode(node->GetChild(idx_child)->prop_as_string("bitmap")) << ";";
                    code << "\n        img_list->Add(img_" << idx_child;
                    if (node->GetChild(idx_child)->prop_as_string("bitmap").is_sameprefix("Art;"))
                        code << ".ConvertToImage()";
                    code << ".Scale(" << size.x << ", " << size.y << "));";
                }
            }
            code << "\n        " << node->get_node_name() << "->AssignImageList(img_list);";
            code << "\n    }";
        }
    }
}
