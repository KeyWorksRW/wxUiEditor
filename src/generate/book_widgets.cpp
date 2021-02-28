//////////////////////////////////////////////////////////////////////////
// Purpose:   Book component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/choicebk.h>    // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/event.h>       // Event classes
#include <wx/listbook.h>    // wxListbook: wxListView and wxNotebook combination
#include <wx/notebook.h>    // wxNotebook interface
#include <wx/simplebook.h>  // wxBookCtrlBase-derived class without any controller.

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "book_widgets.h"

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
