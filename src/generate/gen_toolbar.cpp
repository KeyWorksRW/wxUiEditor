/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/toolbar.h>  // wxToolBar interface declaration

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_toolbar.h"

wxObject* ToolBarFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                      DlgSize(parent, node, prop_size), GetStyleInt(node) | wxTB_NOALIGN | wxTB_NODIVIDER | wxNO_BORDER);

    if (node->HasValue(prop_margins))
    {
        wxSize margins(node->prop_as_wxSize(prop_margins));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue(prop_packing))
        widget->SetToolPacking(node->prop_as_int(prop_packing));
    if (node->HasValue(prop_separation))
        widget->SetToolSeparation(node->prop_as_int(prop_separation));

    widget->Bind(wxEVT_TOOL, &ToolBarFormGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void ToolBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool is_preview)
{
    auto toolbar = wxStaticCast(wxobject, wxToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }

    auto count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->isGen(gen_tool))
        {
            auto bmp = childObj->prop_as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            toolbar->AddTool(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, wxNullBitmap,
                             (wxItemKind) childObj->prop_as_int(prop_kind), childObj->prop_as_wxString(prop_help),
                             wxEmptyString, nullptr);
        }
        else if (childObj->isGen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else
        {
            const wxObject* child;
            if (!is_preview)
                child = GetMockup()->GetChild(wxobject, i);
            else
                child = node->GetChild(i)->GetMockupObject();

            if (auto control = wxDynamicCast(child, wxControl); control)
            {
                toolbar->AddControl(control);
            }
        }
    }
    toolbar->Realize();
}

std::optional<ttlib::cstr> ToolBarFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id, ";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxToolBar(parent, id, pos, size, style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> ToolBarFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_base_class)
    {
        ttlib::cstr code;
        code << "wxToolBar";
        return code;
    }

    return GenFormCode(cmd, node);
}

std::optional<ttlib::cstr> ToolBarFormGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    auto code = GenFormSettings(node);

    if (node->prop_as_int(prop_separation) != 5)
    {
        if (code.size())
            code << '\n';
        code << "SetToolSeparation(" << node->prop_as_string(prop_separation) << ");";
    }

    if (node->HasValue(prop_margins))
    {
        if (code.size())
            code << '\n';
        code << "SetMargins(" << node->prop_as_string(prop_margins) << ");";
    }

    if (node->prop_as_int(prop_packing) != 1)
    {
        if (code.size())
            code << '\n';
        code << "SetToolPacking(" << node->prop_as_string(prop_packing) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> ToolBarFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    auto code = GenEventCode(event, class_name);
    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.Replace(ttlib::cstr() << event->GetNode()->prop_as_string(prop_var_name) << "->", "");
    return code;
}

bool ToolBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/toolbar.h>", set_src, set_hdr);

    return true;
}

void ToolBarFormGenerator::OnTool(wxCommandEvent& event)
{
    auto toolbar = wxDynamicCast(event.GetEventObject(), wxToolBar);
    auto wxobject = toolbar->GetToolClientData(event.GetId());
    if (wxobject)
    {
        wxGetFrame().GetMockup()->SelectNode(wxobject);
    }
}

// ../../wxSnapShot/src/xrc/xh_toolb.cpp
// ../../../wxWidgets/src/xrc/xh_toolb.cpp

int ToolBarFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxToolBar");

    if (node->as_int(prop_packing) >= 0)
        ADD_ITEM_PROP(prop_packing, "packing")
    if (node->as_int(prop_separation) >= 0)
        ADD_ITEM_PROP(prop_separation, "separation")
    ADD_ITEM_PROP(prop_margins, "margins")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ToolBarFormGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxToolBarXmlHandler");
}

//////////////////////////////////////////  ToolBarGenerator  //////////////////////////////////////////

wxObject* ToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                DlgSize(parent, node, prop_size), GetStyleInt(node) | wxTB_NODIVIDER | wxNO_BORDER);

    if (node->HasValue(prop_margins))
    {
        wxSize margins(node->prop_as_wxSize(prop_margins));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue(prop_packing))
        widget->SetToolPacking(node->prop_as_int(prop_packing));
    if (node->HasValue(prop_separation))
        widget->SetToolSeparation(node->prop_as_int(prop_separation));

    widget->Bind(wxEVT_TOOL, &ToolBarGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void ToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool is_preview)
{
    auto toolbar = wxStaticCast(wxobject, wxToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }

    auto count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        if (childObj->isGen(gen_tool))
        {
            auto bmp = childObj->prop_as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            toolbar->AddTool(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, wxNullBitmap,
                             (wxItemKind) childObj->prop_as_int(prop_kind), childObj->prop_as_wxString(prop_help),
                             wxEmptyString, nullptr);
        }
        else if (childObj->isGen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else
        {
            const wxObject* child;
            if (!is_preview)
                child = GetMockup()->GetChild(wxobject, i);
            else
                child = node->GetChild(i)->GetMockupObject();

            if (auto control = wxDynamicCast(child, wxControl); control)
            {
                toolbar->AddControl(control);
            }
        }
    }
    toolbar->Realize();
}

std::optional<ttlib::cstr> ToolBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->prop_as_string(prop_var_name);

    if (node->isParent(gen_wxFrame))
    {
        code << " = CreateToolBar(";

        auto& id = node->prop_as_string(prop_id);
        auto& window_name = node->prop_as_string(prop_window_name);
        auto& style = node->prop_as_string(prop_style);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (window_name.size())
        {
            GenStyle(node, code);
            code << ", " << id << ", " << window_name;
        }
        else if (!id.is_sameas("wxID_ANY"))
        {
            GenStyle(node, code);
            code << ", " << id;
        }
        else if (!style.is_sameas("wxTB_HORIZONTAL") || win_style.size())
        {
            GenStyle(node, code);
        }

        code << ");";
    }
    else
    {
        code << " = new wxToolBar(" << GetParentName(node) << ", " << node->prop_as_string(prop_id);
        GeneratePosSizeFlags(node, code, false, "wxTB_HORIZONTAL");
    }

    return code;
}

std::optional<ttlib::cstr> ToolBarGenerator::GenAfterChildren(Node* node)
{
    ttlib::cstr code;
    code << '\t' << node->get_node_name() << "->Realize();";

    return code;
}

std::optional<ttlib::cstr> ToolBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    auto code = GenFormSettings(node);

    if (node->prop_as_int(prop_separation) != 5)
    {
        if (code.size())
            code << '\n';
        code << "SetToolSeparation(" << node->prop_as_string(prop_separation) << ");";
    }

    if (node->HasValue(prop_margins))
    {
        if (code.size())
            code << '\n';
        code << "SetMargins(" << node->prop_as_string(prop_margins) << ");";
    }

    if (node->prop_as_int(prop_packing) != 1)
    {
        if (code.size())
            code << '\n';
        code << "SetToolPacking(" << node->prop_as_string(prop_packing) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> ToolBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ToolBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/toolbar.h>", set_src, set_hdr);

    return true;
}

void ToolBarGenerator::OnTool(wxCommandEvent& event)
{
    auto toolbar = wxDynamicCast(event.GetEventObject(), wxToolBar);
    auto wxobject = toolbar->GetToolClientData(event.GetId());
    if (wxobject)
    {
        wxGetFrame().GetMockup()->SelectNode(wxobject);
    }
}

// ../../wxSnapShot/src/xrc/xh_toolb.cpp
// ../../../wxWidgets/src/xrc/xh_toolb.cpp

int ToolBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxToolBar");

    if (node->as_int(prop_packing) >= 0)
        ADD_ITEM_PROP(prop_packing, "packing")
    if (node->as_int(prop_separation) >= 0)
        ADD_ITEM_PROP(prop_separation, "separation")
    ADD_ITEM_PROP(prop_margins, "margins")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ToolBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxToolBarXmlHandler");
}

//////////////////////////////////////////  ToolGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> ToolGenerator::GenConstruction(Node* node)
{
    if (node->HasValue(prop_bitmap))
    {
        ttlib::cstr code;
        if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            code << "#if wxCHECK_VERSION(3, 1, 6)\n";
        }

        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
        if (is_code_block)
        {
            // GenerateBundleCode assumes an indent within an indent
            bundle_code.Replace("\t\t\t", "\t\t", true);
            code << '\t' << bundle_code;
            code << '\t' << GenToolCode(node, "wxBitmapBundle::FromBitmaps(bitmaps)");
            code << "\n\t}";
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                code << GenToolCode(node, GenerateBitmapCode(node->prop_as_string(prop_bitmap)));
                code << "\n#endif";
            }
        }
        else
        {
            code << GenToolCode(node, bundle_code);
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                code << GenToolCode(node, GenerateBitmapCode(node->prop_as_string(prop_bitmap)));
                code << "\n#endif";
            }
        }
        return code;
    }
    else
    {
        return GenToolCode(node);
    }
}

std::optional<ttlib::cstr> ToolGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int ToolGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  ToolSeparatorGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> ToolSeparatorGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->isParent(gen_wxToolBar) || node->isParent(gen_wxRibbonToolBar))
    {
        code << node->get_parent_name() << "->AddSeparator();";
    }
    else
    {
        code << "AddSeparator();";
    }

    return code;
}

int ToolSeparatorGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& object, size_t /* xrc_flags */)
{
    object.append_attribute("class").set_value("separator");

    return BaseGenerator::xrc_updated;
}
