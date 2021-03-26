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
#include <wx/treebook.h>    // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "book_widgets.h"

//////////////////////////////////////////  BookPageGenerator  //////////////////////////////////////////

wxObject* BookPageGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(txt_pos), node->prop_as_wxSize(txt_size),
                    node->prop_as_int(txt_style) | node->prop_as_int(txt_window_style));
    auto book = wxDynamicCast(parent, wxBookCtrlBase);
    if (book)
    {
        auto cur_selection = book->GetSelection();
        book->AddPage(widget, node->prop_as_string(txt_label));
        if (node->prop_as_bool("select"))
        {
            book->SetSelection(book->GetPageCount() - 1);
        }
        else if (cur_selection >= 0)
        {
            book->SetSelection(cur_selection);
        }

        // TODO: [KeyWorks - 11-22-2020] If the page has a bitmap and the parent set bitmap size, then we need to add the
        // image to the parent's image list
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
    code << ");";

    return code;
}

//////////////////////////////////////////  NotebookGenerator  //////////////////////////////////////////

wxObject* NotebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    // TODO: [KeyWorks - 11-22-2020] If a bitmap size is specified, then we need to create an imagelist

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

    return true;
}

//////////////////////////////////////////  ChoicebookGenerator  //////////////////////////////////////////

wxObject* ChoicebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxChoicebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                         node->prop_as_wxSize("size"), node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    // TODO: [KeyWorks - 11-22-2020] If a bitmap size is specified, then we need to create an imagelist -- see issue #518

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &ChoicebookGenerator::OnPageChanged, this);

    return widget;
}

void ChoicebookGenerator::OnPageChanged(wxChoicebookEvent& event)
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

    return true;
}

//////////////////////////////////////////  ListbookGenerator  //////////////////////////////////////////

wxObject* ListbookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxListbook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    // TODO: [KeyWorks - 11-22-2020] If a bitmap size is specified, then we need to create an imagelist -- see issue #518

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &ListbookGenerator::OnPageChanged, this);

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

    return true;
}

//////////////////////////////////////////  TreebookGenerator  //////////////////////////////////////////

wxObject* TreebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxTreebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    // TODO: [KeyWorks - 11-22-2020] If a bitmap size is specified, then we need to create an imagelist

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &TreebookGenerator::OnPageChanged, this);

    return widget;
}

void TreebookGenerator::OnPageChanged(wxTreebookEvent& event)
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

    return code;
}

std::optional<ttlib::cstr> TreebookGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool TreebookGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/treebook.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  SimplebookGenerator  //////////////////////////////////////////

wxObject* SimplebookGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxSimplebook(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                                   node->prop_as_wxSize("size"), node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &SimplebookGenerator::OnPageChanged, this);

    return widget;
}

void SimplebookGenerator::OnPageChanged(wxBookCtrlBaseEvent& event)
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
