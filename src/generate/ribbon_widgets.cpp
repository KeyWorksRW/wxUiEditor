/////////////////////////////////////////////////////////////////////////////
// Purpose:   Ribbon component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/gallery.h>    // Ribbon control which displays a gallery of items to choose from
#include <wx/ribbon/toolbar.h>    // Ribbon-style tool bar

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "code.h"        // Code -- Helper class for generating code
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

void RibbonBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */,
                                           bool /* is_preview */)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarFormGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    auto bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (bar)
        // BUGBUG: [Randalphwa - 06-12-2022] Don't use GetMockup() if is_preview is true!
        GetMockup()->SelectNode(event.GetPage());
    event.Skip();
}

bool RibbonBarFormGenerator::ConstructionCode(Code& code)
{
    // Note: Form construction is called before any indentation is set
    if (code.is_cpp())
    {
        code.Str((prop_class_name)).Str("::").Str(prop_class_name);
        code += "(wxWindow* parent, wxWindowID id";
        code.Comma().Str("const wxPoint& pos").Comma().Str("const wxSize& size");
        code.Comma().Str("long style)");
        code.Str(" : wxRibbonBar(parent, id, pos, size, style)").Eol() += "{";
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.RibbonBar):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").Add(prop_id);
        code.Indent(3);
        code.Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() + 4);
        code.Add("style=").Style();
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.RibbonBar.__init__(self, parent, id, pos, size, style)";
    }

    code.ResetIndent();

    return true;
}

bool RibbonBarFormGenerator::AfterChildrenCode(Code& code)
{
    code.FormFunction("Realize(").EndFunction();

    return true;
}

bool RibbonBarFormGenerator::HeaderCode(Code& code)
{
    auto* node = code.node();

    code.NodeName().Str("(wxWindow* parent, wxWindowID id = ").Str(prop_id);
    code.Comma().Str("const wxPoint& pos = ");

    auto position = node->as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
        code.Str("wxDefaultPosition");
    else
        code.Pos(prop_pos, no_dlg_units);

    code.Comma().Str("const wxSize& size = ");

    auto size = node->prop_as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dlg_units);

    auto& style = node->prop_as_string(prop_style);
    auto& win_style = node->prop_as_string(prop_window_style);
    if (style.empty() && win_style.empty())
        code.Comma().Str("long style = 0");
    else
    {
        code.Comma();
        code.CheckLineLength(style.size() + win_style.size() + sizeof("long style = "));
        code.Str("long style = ");
        if (style.size())
        {
            code.CheckLineLength(style.size() + win_style.size());
            code += style;
            if (win_style.size())
            {
                code << '|' << win_style;
            }
        }
        else if (win_style.size())
        {
            code.Str(win_style);
        }
    }

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool RibbonBarFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_derived_class))
    {
        code.Str((prop_derived_class));
    }
    else
    {
        code += "wxRibbonBar";
    }

    return true;
}

bool RibbonBarFormGenerator::SettingsCode(Code& code)
{
    auto& theme = code.node()->as_string(prop_theme);
    if (theme.is_sameas("Default"))
    {
        code.FormFunction("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonDefaultArtProvider");
    }
    else if (theme.is_sameas("Generic"))
    {
        code.FormFunction("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonAUIArtProvider");
    }
    else if (theme.is_sameas("MSW"))
    {
        code.FormFunction("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonMSWArtProvider");
    }
    code.EndFunction();

    return true;
}

std::optional<ttlib::sview> RibbonBarFormGenerator::GenEvents(Code& code, NodeEvent* event, const std::string& class_name)
{
    BaseGenerator::GenEvents(code, event, class_name);

    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.m_code.Replace(ttlib::cstr() << event->GetNode()->as_string(prop_var_name) << "->", "");
    return code.m_code;
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

void RibbonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */, bool /* is_preview */)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonBar);
    btn_bar->Realize();
}

void RibbonBarGenerator::OnPageChanged(wxRibbonBarEvent& event)
{
    auto bar = wxDynamicCast(event.GetEventObject(), wxRibbonBar);
    if (bar)
        // BUGBUG: [Randalphwa - 06-12-2022] Don't use GetMockup() if is_preview is true!
        GetMockup()->SelectNode(event.GetPage());
    event.Skip();
}

bool RibbonBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ValidParentName().Comma().Add(prop_id);
    code.PosSizeFlags(false, "wxRIBBON_BAR_DEFAULT_STYLE");

    return true;
}

bool RibbonBarGenerator::SettingsCode(Code& code)
{
    auto& theme = code.node()->as_string(prop_theme);
    if (theme.is_sameas("Default"))
    {
        code.Eol().NodeName().Function("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonDefaultArtProvider");
    }
    else if (theme.is_sameas("Generic"))
    {
        code.Eol().NodeName().Function("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonAUIArtProvider");
    }
    else if (theme.is_sameas("MSW"))
    {
        code.Eol().NodeName().Function("SetArtProvider(").Str(code.is_cpp() ? "new " : "").Add("wxRibbonMSWArtProvider");
    }
    code.EndFunction();

    return true;
}

bool RibbonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/art.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/bar.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/ribbon/control.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_ribbon.cpp
// ../../../wxWidgets/src/xrc/xh_ribbon.cpp
// See Handle_bar()

int RibbonBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonBar");

    ttlib::cstr art(node->value(prop_theme));
    if (art == "Generic")
        art = "aui";
    else if (art == "MSW")
        art = "msw";
    else
        art = "default";

    item.append_child("art-provider").text().set(art);

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void RibbonBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxRibbonXmlHandler");
}

//////////////////////////////////////////  RibbonPageGenerator  //////////////////////////////////////////

wxObject* RibbonPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto bmp = node->HasValue(prop_bitmap) ? node->prop_as_wxBitmap(prop_bitmap) : wxNullBitmap;
    // REVIEW: This is still a bitmap rather then a bundle as of the 3.1.6 release
    auto widget = new wxRibbonPage((wxRibbonBar*) parent, wxID_ANY, node->prop_as_wxString(prop_label), bmp, 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RibbonPageGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ParentName().Comma().Add(prop_id);
    code.Comma().QuotedString(prop_label);
    if (code.HasValue(prop_bitmap))
    {
        code.Comma();
        GenerateSingleBitmapCode(code, code.node()->as_string(prop_bitmap));
    }
    code.EndFunction();

    return true;
}

bool RibbonPageGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_select))
    {
        code.ParentName().Function("SetActivePage(").NodeName().EndFunction();
    }

    return true;
}

bool RibbonPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/page.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp
// See Handle_page()

int RibbonPageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonPage");

    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags, "icon");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
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
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
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

bool RibbonPanelGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/panel.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp
// See Handle_panel()

int RibbonPanelGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonPanel");

    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags, "icon");

    // Up through wxWidgets 3.1.7, no styles are accepted
    // GenXrcStylePosSize(node, item);
    GenXrcPreStylePosSize(node, item, {});

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

//////////////////////////////////////////  RibbonButtonBarGenerator  //////////////////////////////////////////

wxObject* RibbonButtonBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonButtonBar((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(parent, node, prop_pos),
                                        DlgSize(parent, node, prop_size), 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonButtonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonButtonBar);

    for (const auto& child: node->GetChildNodePtrs())
    {
        auto bmp = child->prop_as_wxBitmap(prop_bitmap);
        if (!bmp.IsOk())
            bmp = GetInternalImage("default");

        // REVIEW: This is still a bitmap rather then a bundle as of the 3.1.6 release
        btn_bar->AddButton(wxID_ANY, child->prop_as_wxString(prop_label), bmp, child->prop_as_wxString(prop_help),
                           (wxRibbonButtonKind) child->prop_as_int(prop_kind));
    }
}

std::optional<ttlib::cstr> RibbonButtonBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << " = new wxRibbonButtonBar(";
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code, false);

    return code;
}

bool RibbonButtonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/buttonbar.h>", set_src, set_hdr);

    return true;
}

int RibbonButtonBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "wxRibbonButtonBar");

    return BaseGenerator::xrc_updated;
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

int RibbonButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "button");

    if (!node->HasValue(prop_bitmap))
    {
        auto bmp = item.append_child("bitmap");
        bmp.append_attribute("stock_id").set_value("wxART_QUESTION");
        bmp.append_attribute("stock_client").set_value("wxART_TOOLBAR");
    }

    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
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

void RibbonToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    auto btn_bar = wxDynamicCast(wxobject, wxRibbonToolBar);

    for (const auto& child: node->GetChildNodePtrs())
    {
        if (child->isGen(gen_ribbonSeparator))
        {
            btn_bar->AddSeparator();
        }
        else
        {
            auto bmp = child->prop_as_wxBitmap(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");
            // REVIEW: This is still a bitmap rather then a bundle as of the 3.1.6 release
            btn_bar->AddTool(wxID_ANY, bmp, child->prop_as_wxString(prop_help),
                             (wxRibbonButtonKind) child->prop_as_int(prop_kind));
        }
    }
    btn_bar->Realize();
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
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

bool RibbonToolBarGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */,
                                         std::set<std::string>& set_hdr)
{
    // Normally we'd use the access property to determin if the header should be in the source or header file. However,
    // the two events used by this component are also in this header file and the tools themselves are fairly useless
    // without processing the events, so we just add the header file to the header generated file.

    set_hdr.insert("#include <wx/ribbon/toolbar.h>");

    return true;
}

int RibbonToolBarGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */, size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
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

int RibbonToolGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */, size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
}

//////////////////////////////////////////  RibbonGalleryGenerator  //////////////////////////////////////////

wxObject* RibbonGalleryGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonGallery((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size), 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonGalleryGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    auto gallery = wxStaticCast(wxobject, wxRibbonGallery);

    for (const auto& child: node->GetChildNodePtrs())
    {
        if (child->isGen(gen_ribbonGalleryItem))
        {
            auto bmp = child->prop_as_wxBitmap(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            // REVIEW: This is still a bitmap rather then a bundle as of the 3.1.6 release
            gallery->Append(bmp, wxID_ANY);
        }
    }
}

std::optional<ttlib::cstr> RibbonGalleryGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << " = new wxRibbonGallery(";
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code, false);

    return code;
}

bool RibbonGalleryGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/gallery.h>", set_src, set_hdr);

    return true;
}

int RibbonGalleryGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "wxRibbonGallery");

    return BaseGenerator::xrc_updated;
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

int RibbonGalleryItemGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "item");

    if (!node->HasValue(prop_bitmap))
    {
        auto bmp = item.append_child("bitmap");
        bmp.append_attribute("stock_id").set_value("wxART_QUESTION");
        bmp.append_attribute("stock_client").set_value("wxART_TOOLBAR");
    }

    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}
