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

std::optional<ttlib::sview> AuiToolBarGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.GetParentName().Comma().Add(prop_id);
    code.PosSizeFlags(false, "wxAUI_TB_DEFAULT_STYLE");

    return code.m_code;
}

std::optional<ttlib::sview> AuiToolBarGenerator::CommonAfterChildren(Code& code)
{
    code.NodeName().Function("Realize(").EndFunction();

    return code.m_code;
}

std::optional<ttlib::sview> AuiToolBarGenerator::CommonSettings(Code& code)
{
    GenFormSettings(code);

    if (!code.is_value(prop_separation, 5))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetToolSeparation(").Str(prop_separation).EndFunction();
    }

    if (code.HasValue(prop_margins))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetMargins(").Str(prop_margins).EndFunction();
    }

    if (!code.is_value(prop_packing, 1))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetToolPacking(").Str(prop_packing).EndFunction();
    }

    return code.m_code;
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

std::optional<ttlib::sview> AuiToolGenerator::CommonConstruction(Code& code)
{
    if (code.HasValue(prop_bitmap))
    {
        auto is_bitmaps_list = BitmapList(code, prop_bitmap);
        GenToolCode(code, is_bitmaps_list);
        if (is_bitmaps_list && code.is_cpp())
        {
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code.CloseBrace();
                code.Add("#else").Eol();
                GenToolCode(code, false);
                code.Eol().Add("#endif").Eol();
            }
            else
            {
                code.Eol() += "}\n";
            }
        }
    }
    else
    {
        GenToolCode(code, false);
    }

    return code.m_code;
}

int AuiToolGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolLabelGenerator  //////////////////////////////////////////

std::optional<ttlib::sview> AuiToolLabelGenerator::CommonConstruction(Code& code)
{
    if (code.node()->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddLabel(");
    }
    else
    {
        code.FormFunction("AddLabel(");
    }
    code.Add(prop_id).Comma().QuotedString(prop_label);
    if (code.IntValue(prop_width) >= 0)
        code.Comma().Str(prop_width);
    code.EndFunction();

    return code.m_code;
}

int AuiToolLabelGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "label");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolSpacerGenerator  //////////////////////////////////////////

std::optional<ttlib::sview> AuiToolSpacerGenerator::CommonConstruction(Code& code)
{
    if (code.node()->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddSpacer(");
    }
    else
    {
        code.FormFunction("AddSpacer(");
    }
    code.ParentName().Function("FromDIP(").Str(prop_width).Str(")").EndFunction();

    return code.m_code;
}

int AuiToolSpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "space");
    item.append_child("width").text().set(node->prop_as_string(prop_width));

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolStretchSpacerGenerator  //////////////////////////////////////////

std::optional<ttlib::sview> AuiToolStretchSpacerGenerator::CommonConstruction(Code& code)
{
    if (code.node()->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddStretchSpacer(");
    }
    else
    {
        code.FormFunction("AddStretchSpacer(");
    }

    if (code.IntValue(prop_proportion) != 1)
    {
        code.Str(prop_proportion);
    }
    code.EndFunction();

    return code.m_code;
}

int AuiToolStretchSpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "space");
    item.append_child("proportion").text().set(node->prop_as_string(prop_proportion));

    return BaseGenerator::xrc_updated;
}
