/////////////////////////////////////////////////////////////////////////////
// Purpose:   Toolbar component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/toolbar.h>

#include "bitmaps.h"     // Map of bitmaps accessed by name
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class

#include "toolbar_widgets.h"

//////////////////////////////////////////  ToolBarFormGenerator  //////////////////////////////////////////

wxObject* ToolBarFormGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxToolBar(
        wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
        node->prop_as_int(txtStyle) | node->prop_as_int("window_style") | wxTB_NOALIGN | wxTB_NODIVIDER | wxNO_BORDER);

    if (node->HasValue("bitmapsize"))
        widget->SetToolBitmapSize(node->prop_as_wxSize("bitmapsize"));
    if (node->HasValue("margins"))
    {
        wxSize margins(node->prop_as_wxSize("margins"));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue("packing"))
        widget->SetToolPacking(node->prop_as_int("packing"));
    if (node->HasValue("separation"))
        widget->SetToolSeparation(node->prop_as_int("separation"));

    widget->Bind(wxEVT_TOOL, &ToolBarFormGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void ToolBarFormGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto toolbar = wxStaticCast(wxobject, wxToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }

    auto node = GetMockup()->GetNode(wxobject);
    auto count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        auto child = GetMockup()->GetChild(wxobject, i);
        if ("tool" == childObj->GetClassName())
        {
            auto bmp = childObj->prop_as_wxBitmap("bitmap");
            if (!bmp.IsOk())
                bmp = GetXPMImage("default");

            toolbar->AddTool(wxID_ANY, childObj->GetPropertyAsString(txtLabel), bmp, wxNullBitmap,
                             (wxItemKind) childObj->prop_as_int("kind"), childObj->GetPropertyAsString("help"),
                             wxEmptyString, child);
        }
        else if ("toolSeparator" == childObj->GetClassName())
        {
            toolbar->AddSeparator();
        }
        else
        {
            wxControl* control = wxDynamicCast(child, wxControl);
            if (NULL != control)
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

    code << node->prop_as_string(txtClassName) << "::" << node->prop_as_string(txtClassName);
    code << "(wxWindow* parent, wxWindowID id, ";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string("window_name").size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxToolBar(parent, id, pos, size, style";
    if (node->prop_as_string("window_name").size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> ToolBarFormGenerator::GenCode(const std::string& cmd, Node* node)
{
    return GenFormCode(cmd, node, "wxToolBar");
}

std::optional<ttlib::cstr> ToolBarFormGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    auto code = GenFormSettings(node);
    if (node->HasValue("bitmapsize"))
    {
        if (code.size())
            code << '\n';
        code << "SetToolBitmapSize(" << node->prop_as_string("bitmapsize") << ");";
    }

    if (node->prop_as_int("separation") != 5)
    {
        if (code.size())
            code << '\n';
        code << "SetToolSeparation(" << node->prop_as_string("separation") << ");";
    }

    if (node->HasValue("margins"))
    {
        if (code.size())
            code << '\n';
        code << "SetMargins(" << node->prop_as_string("margins") << ");";
    }

    if (node->prop_as_int("packing") != 1)
    {
        if (code.size())
            code << '\n';
        code << "SetToolPacking(" << node->prop_as_string("packing") << ");";
    }

    return code;
}

std::optional<ttlib::cstr> ToolBarFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    auto code = GenEventCode(event, class_name);
    // Since this is the base class, we don't want to use the pointer that GenEventCode() would normally create
    code.Replace(ttlib::cstr() << event->GetNode()->prop_as_string(txtVarName) << "->", "");
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

//////////////////////////////////////////  ToolBarGenerator  //////////////////////////////////////////

wxObject* ToolBarGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxToolBar(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                      node->prop_as_int(txtStyle) | node->prop_as_int("window_style") | wxTB_NODIVIDER | wxNO_BORDER);

    if (node->HasValue("bitmapsize"))
        widget->SetToolBitmapSize(node->prop_as_wxSize("bitmapsize"));
    if (node->HasValue("margins"))
    {
        wxSize margins(node->prop_as_wxSize("margins"));
        widget->SetMargins(margins.GetWidth(), margins.GetHeight());
    }
    if (node->HasValue("packing"))
        widget->SetToolPacking(node->prop_as_int("packing"));
    if (node->HasValue("separation"))
        widget->SetToolSeparation(node->prop_as_int("separation"));

    widget->Bind(wxEVT_TOOL, &ToolBarGenerator::OnTool, this);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void ToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto toolbar = wxStaticCast(wxobject, wxToolBar);
    ASSERT(toolbar);
    if (!toolbar)
    {
        return;
    }

    auto node = GetMockup()->GetNode(wxobject);
    auto count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        auto childObj = node->GetChild(i);
        auto child = GetMockup()->GetChild(wxobject, i);
        if ("tool" == childObj->GetClassName())
        {
            auto bmp = childObj->prop_as_wxBitmap("bitmap");
            if (!bmp.IsOk())
                bmp = GetXPMImage("default");

            toolbar->AddTool(wxID_ANY, childObj->GetPropertyAsString(txtLabel), bmp, wxNullBitmap,
                             (wxItemKind) childObj->prop_as_int("kind"), childObj->GetPropertyAsString("help"),
                             wxEmptyString, child);
        }
        else if ("toolSeparator" == childObj->GetClassName())
        {
            toolbar->AddSeparator();
        }
        else
        {
            wxControl* control = wxDynamicCast(child, wxControl);
            if (NULL != control)
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
        code << "auto ";
    code << node->prop_as_string(txtVarName);
    if (node->FindParentForm()->GetClassName() == "wxFrame")
    {
        code << " = CreateToolBar(";

        auto& id = node->prop_as_string("id");
        auto& window_name = node->prop_as_string("window_name");
        auto& style = node->prop_as_string(txtStyle);
        auto& win_style = node->prop_as_string("window_style");

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
        code << " = new wxToolBar(" << GetParentName(node) << ", " << node->prop_as_string("id");
        GeneratePosSizeFlags(node, code, false, "", "wxTB_HORIZONTAL");
    }

    return code;
}

std::optional<ttlib::cstr> ToolBarGenerator::GenCode(const std::string& cmd, Node* node)
{
    ttlib::cstr code;
    if (cmd == "after_addchild")
    {
        code << '\t' << node->get_node_name() << "->Realize();";
    }
    else
    {
        return {};
    }

    return code;
}

std::optional<ttlib::cstr> ToolBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    auto code = GenFormSettings(node);
    if (node->HasValue("bitmapsize"))
    {
        if (code.size())
            code << '\n';
        code << "SetToolBitmapSize(" << node->prop_as_string("bitmapsize") << ");";
    }

    if (node->prop_as_int("separation") != 5)
    {
        if (code.size())
            code << '\n';
        code << "SetToolSeparation(" << node->prop_as_string("separation") << ");";
    }

    if (node->HasValue("margins"))
    {
        if (code.size())
            code << '\n';
        code << "SetMargins(" << node->prop_as_string("margins") << ");";
    }

    if (node->prop_as_int("packing") != 1)
    {
        if (code.size())
            code << '\n';
        code << "SetToolPacking(" << node->prop_as_string("packing") << ");";
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

//////////////////////////////////////////  ToolGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> ToolGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << '\t';

    if (node->prop_as_string("id") == "wxID_ANY" && node->GetInUseEventCount())
    {
        if (node->IsLocal())
            code << "auto ";
        code << node->get_node_name() << " = ";
    }

    // If the user doesn't want access, then we have no use for the return value.
    if (node->IsLocal())
    {
        if (node->GetParent()->GetClassName().is_sameas("wxToolBar"))
            code << node->get_parent_name() << "->AddTool(" << node->prop_as_string("id") << ", ";
        else
            code << "AddTool(" << node->prop_as_string("id") << ", ";
    }
    else
    {
        if (node->GetParent()->GetClassName().is_sameas("wxToolBar"))
            code << node->get_node_name() << " = " << node->get_parent_name() << "->AddTool(" << node->prop_as_string("id")
                 << ", ";
        else
            code << node->get_node_name() << " = AddTool(" << node->prop_as_string("id") << ", ";
    }

    auto& label = node->prop_as_string(txtLabel);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    code << ", " << GenerateBitmapCode(node->prop_as_string("bitmap"));

    if (!node->HasValue(txtTooltip) && !node->HasValue("statusbar"))
    {
        if (node->prop_as_string("kind") != "wxITEM_NORMAL")
        {
            code << ", wxEmptyString, " << node->prop_as_string("kind");
        }

        code << ");";
        return code;
    }

    if (node->HasValue(txtTooltip) && !node->HasValue("statusbar"))
    {
        code << ",\n\t\t\t" << GenerateQuotedString(node->prop_as_string(txtTooltip));
        if (node->prop_as_string("kind") != "wxITEM_NORMAL")
        {
            code << ", " << node->prop_as_string("kind");
        }
    }

    else if (node->HasValue("statusbar"))
    {
        code << ", wxNullBitmap, ";
        code << node->prop_as_string("kind") << ", \n\t\t\t";

        if (node->HasValue(txtTooltip))
        {
            code << GenerateQuotedString(node->prop_as_string(txtTooltip));
        }
        else
        {
            code << "wxEmptyString";
        }

        code << ", " << GenerateQuotedString(node->prop_as_string("statusbar"));
    }

    code << ");";

    return code;
}

std::optional<ttlib::cstr> ToolGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

//////////////////////////////////////////  ToolGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> ToolSeparatorGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->GetParent()->GetClassName().is_sameas("wxToolBar"))

        code << node->get_parent_name() << "->AddSeparator();";
    else
        code << "AddSeparator();";

    return code;
}
