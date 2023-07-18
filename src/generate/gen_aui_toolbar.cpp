/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibar.h>        // wxaui: wx advanced user interface - docking window manager
#include <wx/aui/framemanager.h>  // wxaui: wx advanced user interface - docking window manager
#include <wx/toolbar.h>           // wxToolBar interface declaration

#include "bitmaps.h"          // Map of bitmaps accessed by name
#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

#include "gen_aui_toolbar.h"

wxObject* AuiToolBarFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxAuiToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node) | wxTB_NODIVIDER);

    if (node->HasValue(prop_margins))
    {
        wxSize margins(node->as_wxSize(prop_margins));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue(prop_packing))
        widget->SetToolPacking(node->as_int(prop_packing));
    if (node->HasValue(prop_separation))
        widget->SetToolSeparation(node->as_int(prop_separation));

    widget->Bind(wxEVT_TOOL, &AuiToolBarFormGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void AuiToolBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool is_preview)
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
            auto bmp = childObj->as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            added_tool = toolbar->AddTool(wxID_ANY, childObj->as_wxString(prop_label), bmp, wxNullBitmap,
                                          (wxItemKind) childObj->as_int(prop_kind), childObj->as_wxString(prop_help),
                                          wxEmptyString, nullptr);
            if (childObj->value(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")
            {
                auto cur_state = GetBitlistInt(childObj.get(), prop_initial_state);
                added_tool->SetState(cur_state);
            }
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

        if (added_tool && childObj->as_bool(prop_disabled))
        {
            toolbar->EnableTool(added_tool->GetId(), false);
        }

        ++idx_child;
    }
    toolbar->Realize();
}

bool AuiToolBarFormGenerator::ConstructionCode(Code& code)
{
    // Note: Form construction is called before any indentation is set
    if (code.is_cpp())
    {
        code.Str((prop_class_name)).Str("::").Str(prop_class_name);
        code += "(wxWindow* parent, wxWindowID id";
        code.Comma().Str("const wxPoint& pos").Comma().Str("const wxSize& size");
        code.Comma().Str("long style").Comma().Str("const wxString& name)");
        code.Str(" : wxToolBar(parent, id, pos, size, style, name)").Eol() += "{";
    }
    else if (code.is_python())
    {
        code.Add("class ").NodeName().Add("(wx.ToolBar):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").as_string(prop_id);
        code.Indent(3);
        code.Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() + 4);
        code.Add("style=").Style().Comma();
        size_t name_len =
            code.HasValue(prop_window_name) ? code.node()->as_string(prop_window_name).size() : sizeof("wx.ToolBarNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.HasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code.Str("wx.ToolBarNameStr");
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.ToolBar.__init__(self, parent, id, pos, size, style, name)";
    }
    else
    {
        return false;
    }

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool AuiToolBarFormGenerator::AfterChildrenCode(Code& code)
{
    code.NodeName().Function("Realize(").EndFunction();

    return true;
}

bool AuiToolBarFormGenerator::HeaderCode(Code& code)
{
    auto* node = code.node();

    code.NodeName().Str("(wxWindow* parent, wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxPoint& pos = ");

    auto position = node->as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
        code.Str("wxDefaultPosition");
    else
        code.Pos(prop_pos, no_dlg_units);

    code.Comma().Str("const wxSize& size = ");

    auto size = node->as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dlg_units);

    auto& style = node->as_string(prop_style);
    auto& win_style = node->as_string(prop_window_style);
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

    if (node->as_string(prop_window_name).size())
    {
        code.Comma().Str("const wxString &name = ").QuotedString(prop_window_name);
    }
    else
    {
        code.Comma().Str("const wxString &name = wxPanelNameStr");
    }

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool AuiToolBarFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_derived_class))
    {
        code.Str((prop_derived_class));
    }
    else
    {
        code += "wxAuiToolBar";
    }

    return true;
}

void AuiToolBarFormGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    BaseGenerator::GenEvent(code, event, class_name);

    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.Replace(tt_string() << event->GetNode()->as_string(prop_var_name) << "->", "");
}

bool AuiToolBarFormGenerator::SettingsCode(Code& code)
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

    return true;
}

bool AuiToolBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/aui/auibar.h>", set_src, set_hdr);

    return true;
}

// REVIEW: [KeyWorks - 05-13-2021] Unlike wxToolBar, you can't store a pointer in a wxAuiToolBar item -- the closest
// equivalent is m_userdata, but that's a long so it won't work for storing a pointer.
void AuiToolBarFormGenerator::OnTool(wxCommandEvent& WXUNUSED(event))
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

int AuiToolBarFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
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

void AuiToolBarFormGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxAuiToolBarXmlHandler");
}

bool AuiToolBarFormGenerator::GetRubyImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("require 'wx/aui'");
    return true;
}

//////////////////////////////////////////  AuiToolBarGenerator  //////////////////////////////////////////

wxObject* AuiToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxAuiToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node) | wxTB_NODIVIDER);

    if (node->HasValue(prop_margins))
    {
        wxSize margins(node->as_wxSize(prop_margins));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue(prop_packing))
        widget->SetToolPacking(node->as_int(prop_packing));
    if (node->HasValue(prop_separation))
        widget->SetToolSeparation(node->as_int(prop_separation));

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
            auto bmp = childObj->as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            added_tool = toolbar->AddTool(wxID_ANY, childObj->as_wxString(prop_label), bmp, wxNullBitmap,
                                          (wxItemKind) childObj->as_int(prop_kind), childObj->as_wxString(prop_help),
                                          wxEmptyString, nullptr);
            if (childObj->value(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")
            {
                auto cur_state = GetBitlistInt(childObj.get(), prop_initial_state);
                added_tool->SetState(cur_state);
            }
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

        if (added_tool && childObj->as_bool(prop_disabled))
        {
            toolbar->EnableTool(added_tool->GetId(), false);
        }

        ++idx_child;
    }
    toolbar->Realize();
}

bool AuiToolBarGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(false, "wxAUI_TB_DEFAULT_STYLE");

    return true;
}

bool AuiToolBarGenerator::AfterChildrenCode(Code& code)
{
    code.NodeName().Function("Realize(").EndFunction();

    return true;
}

bool AuiToolBarGenerator::SettingsCode(Code& code)
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

    return true;
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

bool AuiToolBarGenerator::GetRubyImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("require 'wx/aui'");
    return true;
}

//////////////////////////////////////////  AuiToolGenerator  //////////////////////////////////////////

bool AuiToolGenerator::ConstructionCode(Code& code)
{
    if (code.HasValue(prop_bitmap))
    {
        auto is_bitmaps_list = BitmapList(code, prop_bitmap);
        GenToolCode(code, is_bitmaps_list);
        if (is_bitmaps_list && code.is_cpp())
        {
            if (Project.value(prop_wxWidgets_version) == "3.1")
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

    if (code.node()->value(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")
    {
        code.Eol().NodeName().Function("SetState(").Str(prop_initial_state).EndFunction();
    }

    return true;
}

int AuiToolGenerator::GetRequiredVersion(Node* node)
{
    if (node->HasProp(prop_initial_state) && node->value(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")
    {
        return std::max(minRequiredVer + 2, BaseGenerator::GetRequiredVersion(node));
    }
    else if (node->as_bool(prop_disabled))
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }
    return BaseGenerator::GetRequiredVersion(node);
}

int AuiToolGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

bool AuiToolGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& /* set_hdr */)
{
    if (node->value(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")
    {
        set_src.insert("#include <wx/aui/framemanager.h>");
        return true;
    }
    else
    {
        return false;
    }
}

//////////////////////////////////////////  AuiToolLabelGenerator  //////////////////////////////////////////

bool AuiToolLabelGenerator::ConstructionCode(Code& code)
{
    if (code.node()->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddLabel(");
    }
    else
    {
        code.FormFunction("AddLabel(");
    }
    code.as_string(prop_id).Comma().QuotedString(prop_label);
    if (code.IntValue(prop_width) >= 0)
        code.Comma().Str(prop_width);
    code.EndFunction();

    return true;
}

int AuiToolLabelGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "label");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolSpacerGenerator  //////////////////////////////////////////

bool AuiToolSpacerGenerator::ConstructionCode(Code& code)
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

    return true;
}

int AuiToolSpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "space");
    item.append_child("width").text().set(node->as_string(prop_width));

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  AuiToolStretchSpacerGenerator  //////////////////////////////////////////

bool AuiToolStretchSpacerGenerator::ConstructionCode(Code& code)
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

    return true;
}

int AuiToolStretchSpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "space");
    item.append_child("proportion").text().set(node->as_string(prop_proportion));

    return BaseGenerator::xrc_updated;
}
