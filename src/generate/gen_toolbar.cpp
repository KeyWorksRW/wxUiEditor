/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/toolbar.h>  // wxToolBar interface declaration

#include "bitmaps.h"          // Map of bitmaps accessed by name
#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

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
        wxToolBarToolBase* added_tool = nullptr;
        if (childObj->isGen(gen_tool))
        {
            auto bmp = childObj->prop_as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            added_tool = toolbar->AddTool(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, wxNullBitmap,
                                          (wxItemKind) childObj->prop_as_int(prop_kind),
                                          childObj->prop_as_wxString(prop_help), wxEmptyString, nullptr);
        }
        else if (childObj->isGen(gen_tool_dropdown))
        {
            auto bmp = childObj->prop_as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            added_tool = toolbar->AddTool(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, wxNullBitmap,
                                          wxITEM_DROPDOWN, childObj->prop_as_wxString(prop_help), wxEmptyString, nullptr);
        }
        else if (childObj->isGen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else if (childObj->isGen(gen_toolStretchable))
        {
            toolbar->AddStretchableSpace();
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
                added_tool = toolbar->AddControl(control);
            }
        }

        if (added_tool && childObj->prop_as_bool(prop_disabled))
        {
            toolbar->EnableTool(added_tool->GetId(), false);
        }
    }
    toolbar->Realize();
}

bool ToolBarFormGenerator::ConstructionCode(Code& code)
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
    else
    {
        code.Add("class ").NodeName().Add("(wx.ToolBar):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").Add(prop_id);
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

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool ToolBarFormGenerator::SettingsCode(Code& code)
{
    GenFormSettings(code);

    if (code.IsTrue(prop_disabled))
        code.Eol(eol_if_needed).FormFunction("Disable()").EndFunction();

    if (code.IsTrue(prop_hidden))
        code.Eol(eol_if_needed).FormFunction("Hide()").EndFunction();

    if (!code.is_value(prop_separation, 5))
    {
        code.Eol(eol_if_needed).FormFunction("SetToolSeparation(").Add(prop_separation).EndFunction();
    }

    if (code.HasValue(prop_margins))
    {
        code.FormFunction("SetMargins(").Add(prop_margins).EndFunction();
    }

    if (!code.is_value(prop_packing, 1))
    {
        code.FormFunction("SetToolPacking(").Add(prop_packing).EndFunction();
    }

    return true;
}

bool ToolBarFormGenerator::AfterChildrenCode(Code& code)
{
    code.FormFunction("Realize(").EndFunction();

    return true;
}

bool ToolBarFormGenerator::HeaderCode(Code& code)
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

    if (node->prop_as_string(prop_window_name).size())
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

bool ToolBarFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_derived_class))
    {
        code.Str((prop_derived_class));
    }
    else
    {
        code += "wxToolBar";
    }

    return true;
}

void ToolBarFormGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    BaseGenerator::GenEvent(code, event, class_name);

    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.m_code.Replace(ttlib::cstr() << event->GetNode()->as_string(prop_var_name) << "->", "");
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
        else if (childObj->isGen(gen_tool_dropdown))
        {
            auto bmp = childObj->prop_as_wxBitmapBundle(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");

            toolbar->AddTool(wxID_ANY, childObj->prop_as_wxString(prop_label), bmp, wxNullBitmap, wxITEM_DROPDOWN,
                             childObj->prop_as_wxString(prop_help), wxEmptyString, nullptr);
        }
        else if (childObj->isGen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else if (childObj->isGen(gen_toolStretchable))
        {
            toolbar->AddStretchableSpace();
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

bool ToolBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    if (code.node()->isParent(gen_wxFrame))
    {
        code.Str(" = ").FormFunction("CreateToolBar(");
        Node* node = code.node();
        auto& id = node->as_string(prop_id);
        auto& window_name = node->as_string(prop_window_name);
        auto& style = node->as_string(prop_style);
        auto& win_style = node->as_string(prop_window_style);

        if (window_name.size())
        {
            code.Style();
            code.Comma().as_string(prop_id);
            code.Comma().QuotedString(prop_window_name);
        }
        else if (!id.is_sameas("wxID_ANY"))
        {
            code.Style();
            code.Comma().as_string(prop_id);
        }
        else if (!style.is_sameas("wxTB_HORIZONTAL") || win_style.size())
        {
            code.Style();
        }
        code.EndFunction();
    }
    else
    {
        code.CreateClass().ValidParentName().Comma().as_string(prop_id);
        code.PosSizeFlags();
    }

    return true;
}

bool ToolBarGenerator::SettingsCode(Code& code)
{
    if (code.node()->as_int(prop_separation) != 5)
    {
        code.Eol().NodeName().Function("SetToolSeparation(").Str(prop_separation).EndFunction();
    }

    if (code.HasValue(prop_margins))
    {
        code.Eol().NodeName().Function("SetMargins(").Str(prop_margins).EndFunction();
    }

    if (code.node()->as_int(prop_packing) != 1)
    {
        code.Eol().NodeName().Function("SetToolPacking(").Str(prop_packing).EndFunction();
    }

    return true;
}

bool ToolBarGenerator::AfterChildrenCode(Code& code)
{
    code.NodeName().Function("Realize(").EndFunction();

    return true;
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

bool ToolGenerator::ConstructionCode(Code& code)
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

    if (code.IsTrue(prop_disabled))
    {
        code.Eol().NodeName().Function("Enable(") << (code.is_cpp() ? "false" : "False");
        code.EndFunction();
    }

    return true;
}

int ToolGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  ToolDropDownGenerator  //////////////////////////////////////////

bool ToolDropDownGenerator::ConstructionCode(Code& code)
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

    return true;
}

int ToolDropDownGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    if (node->GetChildCount())
    {
        object = object.append_child("dropdown");
        object = object.append_child("object");
        object.append_attribute("class").set_value("wxMenu");

        for (const auto& child: node->GetChildNodePtrs())
        {
            auto child_object = object.append_child("object");
            auto child_generator = child->GetNodeDeclaration()->GetGenerator();
            if (child_generator->GenXrcObject(child.get(), child_object, xrc_flags) == BaseGenerator::xrc_not_supported)
            {
                object.remove_child(child_object);
            }

            // A submenu can have children
            if (child->GetChildCount())
            {
                for (const auto& grandchild: child->GetChildNodePtrs())
                {
                    auto grandchild_object = child_object.append_child("object");
                    auto grandchild_generator = grandchild->GetNodeDeclaration()->GetGenerator();
                    if (grandchild_generator->GenXrcObject(grandchild.get(), grandchild_object, xrc_flags) ==
                        BaseGenerator::xrc_not_supported)
                    {
                        child_object.remove_child(grandchild_object);
                    }
                    // A submenu menu item can also be a submenu with great grandchildren.
                    if (grandchild->GetChildCount())
                    {
                        for (const auto& great_grandchild: grandchild->GetChildNodePtrs())
                        {
                            auto great_grandchild_object = grandchild_object.append_child("object");
                            auto great_grandchild_generator = grandchild->GetNodeDeclaration()->GetGenerator();
                            if (great_grandchild_generator->GenXrcObject(great_grandchild.get(), great_grandchild_object,
                                                                         xrc_flags) == BaseGenerator::xrc_not_supported)
                            {
                                grandchild_object.remove_child(grandchild_object);
                            }
                            // It's possible to have even more levels of submenus, but we'll stop here.
                        }
                    }
                }
            }
        }
    }

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  ToolSeparatorGenerator  //////////////////////////////////////////

bool ToolSeparatorGenerator::ConstructionCode(Code& code)
{
    auto* node = code.node();
    if (node->isParent(gen_wxToolBar) || node->isParent(gen_wxRibbonToolBar) || node->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddSeparator(").EndFunction();
    }
    else
    {
        code.FormFunction("AddSeparator(").EndFunction();
    }

    return true;
}

int ToolSeparatorGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& object, size_t /* xrc_flags */)
{
    object.append_attribute("class").set_value("separator");

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  ToolStretchableGenerator  //////////////////////////////////////////

bool ToolStretchableGenerator::ConstructionCode(Code& code)
{
    auto* node = code.node();
    if (node->isParent(gen_wxToolBar))
    {
        code.ParentName().Function("AddStretchableSpace(").EndFunction();
    }
    else if (node->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddStretchSpacer(");
        if (code.IntValue(prop_proportion) != 1)
        {
            code.Str(prop_proportion);
        }
        code.EndFunction();
    }
    else
    {
        code.FormFunction("AddStretchableSpace(").EndFunction();
    }

    return true;
}

int ToolStretchableGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& object, size_t /* xrc_flags */)
{
    object.append_attribute("class").set_value("space");

    return BaseGenerator::xrc_updated;
}
