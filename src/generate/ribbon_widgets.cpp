/////////////////////////////////////////////////////////////////////////////
// Purpose:   Ribbon component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/gallery.h>    // Ribbon control which displays a gallery of items to choose from
#include <wx/ribbon/toolbar.h>    // Ribbon-style tool bar

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "ribbon_widgets.h"

//////////////////////////////////////////  RibbonBarGenerator  //////////////////////////////////////////

wxObject* RibbonBarGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxRibbonBar(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                        node->prop_as_int(prop_style) | node->prop_as_int("window_style"));

    if (node->prop_as_string("theme") == "Default")
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    else if (node->prop_as_string("theme") == "Generic")
        widget->SetArtProvider(new wxRibbonAUIArtProvider);
    else if (node->prop_as_string("theme") == "MSW")
        widget->SetArtProvider(new wxRibbonMSWArtProvider);

    widget->Bind(wxEVT_RIBBONBAR_PAGE_CHANGED, &RibbonBarGenerator::OnPageChanged, this);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    auto book = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (book && event.GetSelection() != wxNOT_FOUND)
        GetMockup()->SelectNode(book->GetPage(event.GetSelection()));
    event.Skip();
}

std::optional<ttlib::cstr> RibbonBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonBar(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");

    GeneratePosSizeFlags(node, code, false, "wxRIBBON_BAR_DEFAULT_STYLE", "wxRIBBON_BAR_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> RibbonBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    auto& theme = node->prop_as_string("theme");
    if (theme.is_sameas("Default"))
        code << node->get_node_name() << "->SetArtProvider(new wxRibbonDefaultArtProvider);";
    else if (theme.is_sameas("Generic"))
        code << node->get_node_name() << "->SetArtProvider(new wxRibbonAUIArtProvider);";
    else if (theme.is_sameas("MSW"))
        code << node->get_node_name() << "->SetArtProvider(new wxRibbonMSWArtProvider);";

    return code;
}

std::optional<ttlib::cstr> RibbonBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RibbonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/art.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/bar.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/control.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  RibbonPageGenerator  //////////////////////////////////////////

wxObject* RibbonPageGenerator::Create(Node* node, wxObject* parent)
{
    auto bmp = node->HasValue("bitmap") ? node->prop_as_wxBitmap("bitmap") : wxNullBitmap;
    auto widget = new wxRibbonPage((wxRibbonBar*) parent, wxID_ANY, node->prop_as_wxString(prop_label), bmp, 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> RibbonPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonPage(";
    code << node->get_parent_name() << ", " << node->prop_as_string("id");
    code << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
        code << GenerateQuotedString(label);
    else
        code << "wxEmptyString";

    if (node->prop_as_string("bitmap").size())
    {
        code << ", " << GenerateBitmapCode(node->prop_as_string("bitmap"));
    }
    code << ");";

    return code;
}

std::optional<ttlib::cstr> RibbonPageGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool("select"))
        code << node->get_parent_name() << "->SetActivePage(" << node->get_node_name() << ");";

    return code;
}

std::optional<ttlib::cstr> RibbonPageGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RibbonPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/page.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  RibbonPanelGenerator  //////////////////////////////////////////

wxObject* RibbonPanelGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxRibbonPanel((wxRibbonPage*) parent, wxID_ANY, node->prop_as_wxString(prop_label),
                          node->prop_as_wxBitmap("bitmap"), node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                          node->prop_as_int(prop_style) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> RibbonPanelGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonPanel(";
    code << node->get_parent_name() << ", " << node->prop_as_string("id");
    code << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
        code << GenerateQuotedString(label);
    else
        code << "wxEmptyString";

    if (node->prop_as_string("bitmap").size())
    {
        if (label.size())
        {
            code << ",\n\n    ";
        }
        else
        {
            code << ", ";
        }

        code << GenerateBitmapCode(node->prop_as_string("bitmap"));
    }
    else
        code << ", wxNullBitmap";

    GeneratePosSizeFlags(node, code, false, "wxRIBBON_PANEL_DEFAULT_STYLE", "wxRIBBON_PANEL_DEFAULT_STYLE");

    code.Replace(", wxNullBitmap);", ");");
    code.Replace(", wxNullBitmap,", ",\n        wxNullBitmap,");

    return code;
}

std::optional<ttlib::cstr> RibbonPanelGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RibbonPanelGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/panel.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  RibbonButtonBarGenerator  //////////////////////////////////////////

wxObject* RibbonButtonBarGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonButtonBar((wxRibbonPanel*) parent, wxID_ANY, node->prop_as_wxPoint("pos"),
                                        node->prop_as_wxSize("size"), 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonButtonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonButtonBar);

    auto node = GetMockup()->GetNode(wxobject);
    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);

        auto bmp = childObj->prop_as_wxBitmap("bitmap");
        if (!bmp.IsOk())
            bmp = GetXPMImage("default");

        btn_bar->AddButton(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, childObj->GetPropertyAsString("help"),
                           (wxRibbonButtonKind) childObj->prop_as_int("kind"));
    }
}

std::optional<ttlib::cstr> RibbonButtonBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonButtonBar(";
    code << node->get_parent_name() << ", " << node->prop_as_string("id");

    GeneratePosSizeFlags(node, code, false);

    return code;
}

std::optional<ttlib::cstr> RibbonButtonBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RibbonButtonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/buttonbar.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  RibbonButtonGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> RibbonButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->get_parent_name() << "->AddButton(" << node->prop_as_string("id") << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
        code << GenerateQuotedString(label);
    else
        code << "wxEmptyString";

    if (node->prop_as_string("bitmap").size())
        code << ", " << GenerateBitmapCode(node->prop_as_string("bitmap"));
    else
        code << ", wxNullBitmap";

    code << ", ";
    auto& help = node->prop_as_string("help");
    if (help.size())
        code << GenerateQuotedString(help);
    else
        code << "wxEmptyString";

    code << ", " << node->prop_as_string("kind") << ");";

    return code;
}

std::optional<ttlib::cstr> RibbonButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

//////////////////////////////////////////  RibbonToolBarGenerator  //////////////////////////////////////////

wxObject* RibbonToolBarGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxRibbonToolBar((wxRibbonPanel*) parent, wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"));
    if (node->prop_as_int("min_rows") != 1 || node->prop_as_string("max_rows") != "-1")
    {
        auto min_rows = node->prop_as_int("min_rows");
        auto max_rows = node->prop_as_int("max_rows");
        if (max_rows < min_rows)
            max_rows = min_rows;
        widget->SetRows(min_rows, max_rows);
    }

    return widget;
}

void RibbonToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto btn_bar = wxDynamicCast(wxobject, wxRibbonToolBar);

    auto node = GetMockup()->GetNode(wxobject);
    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);

        auto bmp = childObj->prop_as_wxBitmap("bitmap");
        if (!bmp.IsOk())
            bmp = GetXPMImage("default");
        btn_bar->AddTool(wxID_ANY, bmp, childObj->GetPropertyAsString("help"),
                         (wxRibbonButtonKind) childObj->prop_as_int("kind"));
    }
    btn_bar->Realize();
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonToolBar(";
    code << node->get_parent_name() << ", " << node->prop_as_string("id");

    GeneratePosSizeFlags(node, code, false);

    return code;
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    auto min_rows = node->prop_as_int("min_rows");
    auto max_rows = node->prop_as_int("max_rows");
    if (min_rows != 1 || max_rows != -1)
    {
        if (max_rows < min_rows)
            max_rows = min_rows;
        code << node->get_node_name() << "->SetRows(" << ttlib::cstr().Format("%d, %d", min_rows, max_rows) << ");";
    }
    else
    {
        return {};
    }

    return code;
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RibbonToolBarGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */,
                                         std::set<std::string>& set_hdr)
{
    // Normally we'd use the access property to determin if the header should be in the source or header file. However,
    // the two events used by this component are also in this header file and the tools themselves are fairly useless
    // without processing the events, so we just add the header file to the header generated file.

    set_hdr.insert("#include <wx/ribbon/toolbar.h>");

    return true;
}

//////////////////////////////////////////  RibbonToolGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> RibbonToolGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->get_parent_name() << "->AddTool(";
    if (node->prop_as_string("id").size())
        code << node->prop_as_string("id");
    else
        code << "wxID_ANY";
    code << ", ";

    if (node->prop_as_string("bitmap").size())
        code << GenerateBitmapCode(node->prop_as_string("bitmap"));
    else
        code << "wxNullBitmap";

    code << ", ";
    auto& help = node->prop_as_string("help");
    if (help.size())
        code << GenerateQuotedString(help);
    else
        code << "wxEmptyString";

    code << ", " << node->prop_as_string("kind") << ");";

    return code;
}

std::optional<ttlib::cstr> RibbonToolGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

//////////////////////////////////////////  RibbonGalleryGenerator  //////////////////////////////////////////

wxObject* RibbonGalleryGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonGallery((wxRibbonPanel*) parent, wxID_ANY, node->prop_as_wxPoint("pos"),
                                      node->prop_as_wxSize("size"), 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonGalleryGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto gallery = wxStaticCast(wxobject, wxRibbonGallery);

    auto node = GetMockup()->GetNode(wxobject);
    size_t count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->GetClassName() == "ribbonGalleryItem")
        {
            auto bmp = childObj->prop_as_wxBitmap("bitmap");
            if (!bmp.IsOk())
                bmp = GetXPMImage("default");

            gallery->Append(bmp, wxID_ANY);
        }
    }
}

std::optional<ttlib::cstr> RibbonGalleryGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonGallery(";
    code << node->get_parent_name() << ", " << node->prop_as_string("id");

    GeneratePosSizeFlags(node, code, false);

    return code;
}

std::optional<ttlib::cstr> RibbonGalleryGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RibbonGalleryGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/gallery.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  RibbonGalleryItemGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> RibbonGalleryItemGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->get_parent_name() << "->Append(";

    if (node->prop_as_string("bitmap").size())
        code << GenerateBitmapCode(node->prop_as_string("bitmap"));
    else
        code << "wxNullBitmap";

    code << ", wxID_ANY);";

    return code;
}

std::optional<ttlib::cstr> RibbonGalleryItemGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}
