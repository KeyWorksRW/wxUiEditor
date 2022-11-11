/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibar.h>  // wxaui: wx advanced user interface - docking window manager
#include <wx/toolbar.h>     // wxToolBar interface declaration

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_aui_toolbar.h"

wxObject* AuiToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxAuiToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
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

    widget->Bind(wxEVT_TOOL, &AuiToolBarGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void AuiToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool is_preview)
{
    auto toolbar = wxStaticCast(wxobject, wxAuiToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }

    size_t idx_child = 0;
    for (auto& childObj: node->GetChildNodePtrs())
    {
        wxAuiToolBarItem* added_tool = nullptr;
        if (childObj->isGen(gen_auitool))
        {
            auto bmp = childObj->prop_as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            added_tool = toolbar->AddTool(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, wxNullBitmap,
                                          (wxItemKind) childObj->prop_as_int(prop_kind),
                                          childObj->prop_as_wxString(prop_help), wxEmptyString, nullptr);
        }
        else if (childObj->isGen(gen_auitool_label))
        {
            toolbar->AddLabel(wxID_ANY, childObj->as_wxString(prop_label), childObj->as_int(prop_width));
        }
        else if (childObj->isGen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else if (childObj->isGen(gen_auitool_spacer))
        {
            toolbar->AddSpacer(childObj->as_int(prop_width));
        }
        else if (childObj->isGen(gen_auitool_stretchable))
        {
            toolbar->AddStretchSpacer(childObj->as_int(prop_proportion));
        }
        else
        {
            const wxObject* child;
            if (!is_preview)
                child = GetMockup()->GetChild(wxobject, idx_child);
            else
                child = node->GetChild(idx_child)->GetMockupObject();

            if (auto* control = wxDynamicCast(child, wxControl); control)
            {
                added_tool = toolbar->AddControl(control);
            }
        }

        if (added_tool && childObj->prop_as_bool(prop_disabled))
        {
            toolbar->EnableTool(added_tool->GetId(), false);
        }

        ++idx_child;
    }
    toolbar->Realize();
}

std::optional<ttlib::cstr> AuiToolBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->prop_as_string(prop_var_name);

    code << " = new wxAuiToolBar(" << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxAUI_TB_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> AuiToolBarGenerator::GenAfterChildren(Node* node)
{
    ttlib::cstr code;
    code << '\t' << node->get_node_name() << "->Realize();";

    return code;
}

std::optional<ttlib::cstr> AuiToolBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
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

std::optional<ttlib::cstr> AuiToolBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool AuiToolBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/aui/auibar.h>", set_src, set_hdr);

    return true;
}

// REVIEW: [KeyWorks - 05-13-2021] Unlike wxToolBar, you can't store a pointer in a wxAuiToolBar item -- the closest
// equivalent is m_userdata, but that's a long so it won't work for storing a pointer.
void AuiToolBarGenerator::OnTool(wxCommandEvent& WXUNUSED(event))
{
#if 0
    auto toolbar = wxDynamicCast(event.GetEventObject(), wxAuiToolBar);

    auto wxobject = toolbar->GetToolClientData(event.GetToolId());
    if (wxobject)
    {
        wxGetFrame().GetMockup()->SelectNode(wxobject);
    }
#endif
}

// ../../wxSnapShot/src/xrc/xh_auitoolb.cpp
// ../../../wxWidgets/src/xrc/xh_auitoolb.cpp

int AuiToolBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxAuiToolBar");

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

void AuiToolBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxAuiToolBarXmlHandler");
}

//////////////////////////////////////////  AuiToolGenerator  //////////////////////////////////////////

// The code generation between gen_tool and gen_auitool is idenical, but they *MUST* be separate classes because the
// interface specifies different events for each (e.g., wxEVT_AUITOOLBAR_BEGIN_DRAG)

std::optional<ttlib::cstr> AuiToolGenerator::GenConstruction(Node* node)
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

std::optional<ttlib::cstr> AuiToolGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int AuiToolGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolLabelGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> AuiToolLabelGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->isParent(gen_wxAuiToolBar))
    {
        code << node->get_parent_name() << "->AddLabel(";
    }
    else
    {
        code << "AddLabel(";
    }

    code << node->value(prop_id) << ", " << GenerateQuotedString(node->value(prop_label)) << ", " << node->value(prop_width)
         << ");";

    return code;
}

std::optional<ttlib::cstr> AuiToolLabelGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int AuiToolLabelGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "label");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolSpacerGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> AuiToolSpacerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->isParent(gen_wxAuiToolBar))
    {
        code << node->get_parent_name() << "->AddSpacer(";
    }
    else
    {
        code << "AddSpacer(";
    }
    code << node->get_parent_name() << "->FromDIP(" << node->value(prop_width) << "));";

    return code;
}

int AuiToolSpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "space");
    item.append_child("width").text().set(node->prop_as_string(prop_width));

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolStretchSpacerGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> AuiToolStretchSpacerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->isParent(gen_wxAuiToolBar))
    {
        code << node->get_parent_name() << "->AddStretchSpacer(";
    }
    else
    {
        code << "AddStretchSpacer(";
    }
    code << node->value(prop_proportion) << ");";

    return code;
}

int AuiToolStretchSpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "space");
    item.append_child("proportion").text().set(node->prop_as_string(prop_proportion));

    return BaseGenerator::xrc_updated;
}
