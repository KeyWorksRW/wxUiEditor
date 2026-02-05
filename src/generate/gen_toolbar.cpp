/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/toolbar.h>  // wxToolBar interface declaration

#include "gen_toolbar.h"

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mainframe.h"      // MainFrame -- Main window frame
#include "mockup_parent.h"  // Top-level MockUp Parent window
#include "node.h"           // Node class
#include "node_creator.h"   // Class used to create nodes
#include "ui_images.h"      // Generated list of images
#include "utils.h"          // Utility functions that work with properties

wxObject* ToolBarFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget =
        new wxToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                      DlgSize(node, prop_size), GetStyleInt(node) | wxTB_NOALIGN | wxTB_NODIVIDER);

    if (node->HasValue(prop_margins))
    {
        wxSize margins(node->as_wxSize(prop_margins));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue(prop_packing))
    {
        widget->SetToolPacking(node->as_int(prop_packing));
    }
    if (node->HasValue(prop_separation))
    {
        widget->SetToolSeparation(node->as_int(prop_separation));
    }

    widget->Bind(wxEVT_TOOL, &ToolBarFormGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void ToolBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node,
                                         bool is_preview)
{
    auto* toolbar = wxStaticCast(wxobject, wxToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }

    auto count = node->get_ChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto* childObj = node->get_Child(i);
        wxToolBarToolBase* added_tool = nullptr;
        if (childObj->is_Gen(gen_tool))
        {
            auto bundle = childObj->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                bundle = wxue_img::bundle_unknown_svg(16, 16);
            }

            added_tool = toolbar->AddTool(wxID_ANY, childObj->as_wxString(prop_label), bundle,
                                          wxNullBitmap, (wxItemKind) childObj->as_int(prop_kind),
                                          childObj->as_wxString(prop_help), wxEmptyString, nullptr);
        }
        else if (childObj->is_Gen(gen_tool_dropdown))
        {
            auto bundle = childObj->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                bundle = wxue_img::bundle_unknown_svg(16, 16);
            }

            added_tool = toolbar->AddTool(wxID_ANY, childObj->as_wxString(prop_label), bundle,
                                          wxNullBitmap, wxITEM_DROPDOWN,
                                          childObj->as_wxString(prop_help), wxEmptyString, nullptr);
        }
        else if (childObj->is_Gen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else if (childObj->is_Gen(gen_toolStretchable))
        {
            toolbar->AddStretchableSpace();
        }
        else
        {
            const wxObject* child = nullptr;
            if (!is_preview)
            {
                child = getMockup()->get_Child(wxobject, i);
            }
            else
            {
                child = node->get_Child(i)->get_MockupObject();
            }

            if (auto* control = wxDynamicCast(child, wxControl); control)
            {
                added_tool = toolbar->AddControl(control);
            }
        }

        if (added_tool && childObj->as_bool(prop_disabled))
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
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code += "(wxWindow* parent, wxWindowID id";
        code.Comma().Str("const wxPoint& pos").Comma().Str("const wxSize& size");
        code.Comma().Str("long style").Comma().Str("const wxString& name)");
        code.Str(" : wxToolBar(parent, id, pos, size, style, name)").Eol() += "{";
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.ToolBar):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").as_string(prop_id);
        code.Indent(3);
        code.Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() +
                                     4);
        code.Add("style=").Style().Comma();
        size_t name_len = code.HasValue(prop_window_name) ?
                              code.node()->as_string(prop_window_name).size() :
                              sizeof("wx.ToolBarNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.HasValue(prop_window_name))
        {
            code.QuotedString(prop_window_name);
        }
        else
        {
            code.Str("wx.ToolBarNameStr");
        }
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
    {
        code.Eol(eol_if_needed).FormFunction("Disable(").EndFunction();
    }

    if (code.IsTrue(prop_hidden))
    {
        code.Eol(eol_if_needed).FormFunction("Hide(").EndFunction();
    }

    if (!code.is_PropValue(prop_separation, 5))
    {
        code.Eol(eol_if_needed)
            .FormFunction("SetToolSeparation(")
            .Add(prop_separation)
            .EndFunction();
    }

    if (code.HasValue(prop_margins))
    {
        code.FormFunction("SetMargins(").Add(prop_margins).EndFunction();
    }

    if (!code.is_PropValue(prop_packing, 1))
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

    code.NodeName().Str("(wxWindow* parent, wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxPoint& pos = ");

    auto position = node->as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
    {
        code.Str("wxDefaultPosition");
    }
    else
    {
        code.Pos(prop_pos, no_dpi_scaling);
    }

    code.Comma().Str("const wxSize& size = ");

    auto size = node->as_wxSize(prop_size);
    if (size == wxDefaultSize)
    {
        code.Str("wxDefaultSize");
    }
    else
    {
        code.WxSize(prop_size, no_dpi_scaling);
    }

    const auto& style = node->as_string(prop_style);
    const auto& win_style = node->as_string(prop_window_style);
    if (style.empty() && win_style.empty())
    {
        code.Comma().Str("long style = 0");
    }
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

bool ToolBarFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
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

    // Since this is the base class, we don't want to use the pointer that GenEventCode() would
    // normally create
    code.Replace((wxString() << event->getNode()->as_string(prop_var_name) << "->").ToStdString(),
                 "");
}

bool ToolBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                       std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/toolbar.h>", set_src, set_hdr);

    return true;
}

void ToolBarFormGenerator::OnTool(wxCommandEvent& event)
{
    auto* toolbar = wxDynamicCast(event.GetEventObject(), wxToolBar);
    auto* wxobject = toolbar->GetToolClientData(event.GetId());
    if (wxobject)
    {
        wxGetFrame().getMockup()->SelectNode(wxobject);
    }
}

// ../../wxSnapShot/src/xrc/xh_toolb.cpp
// ../../../wxWidgets/src/xrc/xh_toolb.cpp

int ToolBarFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
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

bool ToolBarFormGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                      GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use Wx::Event qw(EVT_TOOL);");
        set_imports.emplace("use Wx qw[:toolbar];");

        return true;
    }
    return false;
}

//////////////////////////////////////////  ToolBarGenerator
/////////////////////////////////////////////

wxObject* ToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget =
        new wxToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                      DlgSize(node, prop_size), GetStyleInt(node) | wxTB_NODIVIDER | wxNO_BORDER);

    if (node->HasValue(prop_margins))
    {
        wxSize margins(node->as_wxSize(prop_margins));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue(prop_packing))
    {
        widget->SetToolPacking(node->as_int(prop_packing));
    }
    if (node->HasValue(prop_separation))
    {
        widget->SetToolSeparation(node->as_int(prop_separation));
    }

    widget->Bind(wxEVT_TOOL, &ToolBarGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void ToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node,
                                     bool is_preview)
{
    auto* toolbar = wxStaticCast(wxobject, wxToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }

    auto count = node->get_ChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto* childObj = node->get_Child(i);
        if (childObj->is_Gen(gen_tool))
        {
            auto bundle = childObj->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                bundle = wxue_img::bundle_unknown_svg(16, 16);
            }

            toolbar->AddTool(wxID_ANY, childObj->as_wxString(prop_label), bundle, wxNullBitmap,
                             (wxItemKind) childObj->as_int(prop_kind),
                             childObj->as_wxString(prop_help), wxEmptyString, nullptr);
        }
        else if (childObj->is_Gen(gen_tool_dropdown))
        {
            auto bundle = childObj->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                bundle = wxue_img::bundle_unknown_svg(16, 16);
            }

            toolbar->AddTool(wxID_ANY, childObj->as_wxString(prop_label), bundle, wxNullBitmap,
                             wxITEM_DROPDOWN, childObj->as_wxString(prop_help), wxEmptyString,
                             nullptr);
        }
        else if (childObj->is_Gen(gen_toolSeparator))
        {
            toolbar->AddSeparator();
        }
        else if (childObj->is_Gen(gen_toolStretchable))
        {
            toolbar->AddStretchableSpace();
        }
        else
        {
            const wxObject* child = nullptr;
            if (!is_preview)
            {
                child = getMockup()->get_Child(wxobject, i);
            }
            else
            {
                child = node->get_Child(i)->get_MockupObject();
            }

            if (auto* control = wxDynamicCast(child, wxControl); control)
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
    if (code.node()->is_Parent(gen_wxFrame))
    {
        code.Str(" = ").FormFunction("CreateToolBar(");
        Node* node = code.node();
        const auto& widget_id = node->as_string(prop_id);
        const auto& window_name = node->as_string(prop_window_name);
        const auto& style = node->as_string(prop_style);
        const auto& win_style = node->as_string(prop_window_style);

        if (window_name.size())
        {
            code.Style();
            code.Comma().as_string(prop_id);
            code.Comma().QuotedString(prop_window_name);
        }
        else if (!widget_id.is_sameas("wxID_ANY"))
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
        code.Eol()
            .NodeName()
            .Function("SetToolSeparation(")
            .as_string(prop_separation)
            .EndFunction();
    }

    if (code.HasValue(prop_margins))
    {
        code.Eol().NodeName().Function("SetMargins(").as_string(prop_margins).EndFunction();
    }

    if (code.node()->as_int(prop_packing) != 1)
    {
        code.Eol().NodeName().Function("SetToolPacking(").as_string(prop_packing).EndFunction();
    }

    return true;
}

bool ToolBarGenerator::AfterChildrenCode(Code& code)
{
    code.NodeName().Function("Realize(").EndFunction();

    return true;
}

bool ToolBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                   std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/toolbar.h>", set_src, set_hdr);

    return true;
}

void ToolBarGenerator::OnTool(wxCommandEvent& event)
{
    auto* toolbar = wxDynamicCast(event.GetEventObject(), wxToolBar);
    auto* wxobject = toolbar->GetToolClientData(event.GetId());
    if (wxobject)
    {
        wxGetFrame().getMockup()->SelectNode(wxobject);
    }
}

// ../../wxSnapShot/src/xrc/xh_toolb.cpp
// ../../../wxWidgets/src/xrc/xh_toolb.cpp

int ToolBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
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

bool ToolBarGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                  GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use Wx::Event qw(EVT_TOOL);");
        set_imports.emplace("use Wx qw[:toolbar];");
        return true;
    }

    return false;
}
