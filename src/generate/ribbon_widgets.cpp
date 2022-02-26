/////////////////////////////////////////////////////////////////////////////
// Purpose:   Ribbon component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/gallery.h>    // Ribbon control which displays a gallery of items to choose from
#include <wx/ribbon/toolbar.h>    // Ribbon-style tool bar

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "ribbon_widgets.h"

//////////////////////////////////////////  RibbonBarFormGenerator  //////////////////////////////////////////

wxObject* RibbonBarFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                  DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->prop_as_string(prop_theme) == "Default")
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    else if (node->prop_as_string(prop_theme) == "Generic")
        widget->SetArtProvider(new wxRibbonAUIArtProvider);
    else if (node->prop_as_string(prop_theme) == "MSW")
        widget->SetArtProvider(new wxRibbonMSWArtProvider);

    widget->Bind(wxEVT_RIBBONBAR_PAGE_CHANGED, &RibbonBarFormGenerator::OnPageChanged, this);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarFormGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    auto bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (bar)
        GetMockup()->SelectNode(event.GetPage());
    event.Skip();
}

std::optional<ttlib::cstr> RibbonBarFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id, ";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style) :";

    code << "\n\twxRibbonBar(parent, id, pos, size, style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> RibbonBarFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_base_class)
    {
        ttlib::cstr code;
        code << "wxRibbonBar";
        return code;
    }

    return GenFormCode(cmd, node);
}

std::optional<ttlib::cstr> RibbonBarFormGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    auto& theme = node->prop_as_string(prop_theme);
    if (theme.is_sameas("Default"))
        code << "SetArtProvider(new wxRibbonDefaultArtProvider);";
    else if (theme.is_sameas("Generic"))
        code << "SetArtProvider(new wxRibbonAUIArtProvider);";
    else if (theme.is_sameas("MSW"))
        code << "SetArtProvider(new wxRibbonMSWArtProvider);";

    return code;
}

std::optional<ttlib::cstr> RibbonBarFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    auto code = GenEventCode(event, class_name);
    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.Replace(ttlib::cstr() << event->GetNode()->prop_as_string(prop_var_name) << "->", "");
    return code;
}

bool RibbonBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/art.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/bar.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/control.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  RibbonBarGenerator  //////////////////////////////////////////

wxObject* RibbonBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                  DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->prop_as_string(prop_theme) == "Default")
        widget->SetArtProvider(new wxRibbonDefaultArtProvider);
    else if (node->prop_as_string(prop_theme) == "Generic")
        widget->SetArtProvider(new wxRibbonAUIArtProvider);
    else if (node->prop_as_string(prop_theme) == "MSW")
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
    auto bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (bar)
        GetMockup()->SelectNode(event.GetPage());
    event.Skip();
}

std::optional<ttlib::cstr> RibbonBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonBar(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code, false, "wxRIBBON_BAR_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> RibbonBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    auto& theme = node->prop_as_string(prop_theme);
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

wxObject* RibbonPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto bmp = node->HasValue(prop_bitmap) ? node->prop_as_wxBitmap(prop_bitmap) : wxNullBitmap;
    // REVIEW: [KeyWorks - 02-25-2022] This is still a bitmap rather then a bundle as of 2/25/22
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
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);
    code << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
        code << GenerateQuotedString(label);
    else
        code << "wxEmptyString";

    if (node->prop_as_string(prop_bitmap).size())
    {
        code << ", " << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    }
    code << ");";

    return code;
}

std::optional<ttlib::cstr> RibbonPageGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_select))
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

wxObject* RibbonPanelGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonPanel((wxRibbonPage*) parent, wxID_ANY, node->prop_as_wxString(prop_label),
                                    node->prop_as_wxBitmap(prop_bitmap), DlgPoint(parent, node, prop_pos),
                                    DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> RibbonPanelGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonPanel(";
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);
    code << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
        code << GenerateQuotedString(label);
    else
        code << "wxEmptyString";

    if (node->prop_as_string(prop_bitmap).size())
    {
        if (label.size())
        {
            code << ",\n\t";
        }
        else
        {
            code << ", ";
        }

        code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    }
    else
        code << ", wxNullBitmap";

    GeneratePosSizeFlags(node, code, false, "wxRIBBON_PANEL_DEFAULT_STYLE");

    code.Replace(", wxNullBitmap);", ");");
    code.Replace(", wxNullBitmap,", ",\n\t\twxNullBitmap,");

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

wxObject* RibbonButtonBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonButtonBar((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(parent, node, prop_pos),
                                        DlgSize(parent, node, prop_size), 0);

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

        auto bmp = childObj->prop_as_wxBitmap(prop_bitmap);
        if (!bmp.IsOk())
            bmp = GetInternalImage("default");

        btn_bar->AddButton(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, childObj->prop_as_wxString(prop_help),
                           (wxRibbonButtonKind) childObj->prop_as_int(prop_kind));
    }
}

std::optional<ttlib::cstr> RibbonButtonBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonButtonBar(";
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);

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

    code << node->get_parent_name() << "->AddButton(" << node->prop_as_string(prop_id) << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
        code << GenerateQuotedString(label);
    else
        code << "wxEmptyString";

    if (node->prop_as_string(prop_bitmap).size())
        code << ", " << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    else
        code << ", wxNullBitmap";

    code << ", ";
    auto& help = node->prop_as_string(prop_help);
    if (help.size())
        code << GenerateQuotedString(help);
    else
        code << "wxEmptyString";

    code << ", " << node->prop_as_string(prop_kind) << ");";

    return code;
}

std::optional<ttlib::cstr> RibbonButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

//////////////////////////////////////////  RibbonToolBarGenerator  //////////////////////////////////////////

wxObject* RibbonToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonToolBar((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size));
    if (node->prop_as_int(prop_min_rows) != 1 || node->prop_as_string(prop_max_rows) != "-1")
    {
        auto min_rows = node->prop_as_int(prop_min_rows);
        auto max_rows = node->prop_as_int(prop_max_rows);
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
    for (size_t idx = 0; idx < count; ++idx)
    {
        auto childObj = node->GetChild(idx);

        if (childObj->isGen(gen_ribbonSeparator))
        {
            btn_bar->AddSeparator();
        }
        else
        {
            auto bmp = childObj->prop_as_wxBitmap(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");
            btn_bar->AddTool(wxID_ANY, bmp, childObj->prop_as_wxString(prop_help),
                             (wxRibbonButtonKind) childObj->prop_as_int(prop_kind));
        }
    }
    btn_bar->Realize();
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRibbonToolBar(";
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code, false);

    return code;
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    auto min_rows = node->prop_as_int(prop_min_rows);
    auto max_rows = node->prop_as_int(prop_max_rows);
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
    if (node->prop_as_string(prop_id).size())
        code << node->prop_as_string(prop_id);
    else
        code << "wxID_ANY";
    code << ", ";

    if (node->prop_as_string(prop_bitmap).size())
        code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    else
        code << "wxNullBitmap";

    code << ", ";
    auto& help = node->prop_as_string(prop_help);
    if (help.size())
        code << GenerateQuotedString(help);
    else
        code << "wxEmptyString";

    code << ", " << node->prop_as_string(prop_kind) << ");";

    return code;
}

std::optional<ttlib::cstr> RibbonToolGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

//////////////////////////////////////////  RibbonGalleryGenerator  //////////////////////////////////////////

wxObject* RibbonGalleryGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonGallery((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size), 0);

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
        if (childObj->isGen(gen_ribbonGalleryItem))
        {
            auto bmp = childObj->prop_as_wxBitmap(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

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
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);

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

    if (node->prop_as_string(prop_bitmap).size())
        code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    else
        code << "wxNullBitmap";

    code << ", wxID_ANY);";

    return code;
}

std::optional<ttlib::cstr> RibbonGalleryItemGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}
