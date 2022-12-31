/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPanel Form generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>  // Base header for wxPanel

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_panel_form.h"

wxObject* PanelFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                              DlgSize(parent, node, prop_size), GetStyleInt(node));
    if (node->HasValue(prop_extra_style))
    {
        int ex_style = 0;
        // Can't use multiview because GetConstantAsInt() searches an unordered_map which
        // requires a std::string to pass to it
        ttlib::multistr mstr(node->value(prop_extra_style), '|');
        for (auto& iter: mstr)
        {
            // Friendly names will have already been converted, so normal lookup works fine.
            ex_style |= g_NodeCreator.GetConstantAsInt(iter);
        }

        widget->SetExtraStyle(widget->GetExtraStyle() | ex_style);
    }

    return widget;
}

bool PanelFormGenerator::ConstructionCode(Code& code)
{
    // Note: Form construction is called before any indentation is set
    if (code.is_cpp())
    {
        code.Str("bool ").Str((prop_class_name)) += "::Create";
        code += "(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString "
                "&name)";
        code.OpenBrace();
        code += "if (!wxPanel::Create(parent, id, pos, size, style, name))";
        code.Eol().Tab() += "return false;\n";
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.Panel):\n");
        code.Tab().Add("def __init__(self, parent):").Eol().Tab(2);
        code << "wx.Panel.__init__(self, parent, id=";
        code.as_string(prop_id);

        code.Indent(3);

        code.Comma().Eol().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().Eol().Add("style=");
        if (code.HasValue(prop_style) && !code.is_value(prop_style, "wxTAB_TRAVERSAL"))
            code.Style();
        else
            code << "wx.wxTAB_TRAVERSAL";
        code << ")";

        code.Unindent();
    }

    if (code.HasValue(prop_extra_style))
    {
        code.FormFunction("SetExtraStyle(GetExtraStyle() | ").Add(prop_extra_style).Str(")").EndFunction();
    }

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool PanelFormGenerator::AfterChildrenCode(Code& code)
{
    Node* panel;
    auto* node = code.node();
    if (node->IsForm())
    {
        panel = node;
        ASSERT_MSG(panel->GetChildCount(), "Trying to generate code for a wxPanel with no children.")
        if (!panel->GetChildCount())
            return true;  // empty dialog, so nothing to do
        ASSERT_MSG(panel->GetChild(0)->IsSizer(), "Expected first child of a wxPanel to be a sizer.");
        if (panel->GetChild(0)->IsSizer())
            node = panel->GetChild(0);
    }
    else
    {
        panel = node->get_form();
    }

    const auto min_size = panel->prop_as_wxSize(prop_minimum_size);
    const auto max_size = panel->prop_as_wxSize(prop_maximum_size);
    const auto size = panel->prop_as_wxSize(prop_size);

    if (min_size == wxDefaultSize && max_size == wxDefaultSize)
    {
        code.FormFunction("SetSizerAndFit(").NodeName(node).EndFunction();
    }
    else
    {
        code.FormFunction("SetSizer(").NodeName(node).EndFunction();
        if (min_size != wxDefaultSize)
        {
            code.Eol().FormFunction("SetMinSize(").WxSize(prop_minimum_size).EndFunction();
        }
        if (max_size != wxDefaultSize)
        {
            code.Eol().FormFunction("SetMaxSize(").WxSize(prop_maximum_size).EndFunction();
        }
        code.Eol().FormFunction("Fit(").EndFunction();
    }

    if (size != wxDefaultSize)
    {
        code.Eol().FormFunction("SetSize(").WxSize(prop_size).EndFunction();
    }

    return true;
}

bool PanelFormGenerator::HeaderCode(Code& code)
{
    if (!code.is_cpp())
        return false;

    code.NodeName() += "() {}";
    code.Eol().Str("const wxPoint& pos = ").Pos(prop_pos).Comma();
    code.Str("const wxSize& size = ").WxSize(prop_size).Comma();
    code.Str("long style = ");
    if (code.HasValue(prop_style))
        code.Str(prop_style);
    else
        code += "wxTAB_TRAVERSAL";
    code.Comma().Str("const wxString &name = ");
    if (code.HasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxPanelNameStr");
    code += ")";
    code.OpenBrace().Str("Create(parent, id, pos, size, style, name);").CloseBrace();
    code.Eol() += "bool Create(wxWindow *parent, ";
    code.Str("wxWindowID id = ").Str(prop_id).Comma();
    code.Str("const wxPoint& pos = ").Pos(prop_pos).Comma();
    code.Str("const wxSize& size = ").WxSize(prop_size).Comma();
    code.Str("long style = ");
    if (code.HasValue(prop_style))
        code.Str(prop_style);
    else
        code.Str("wxTAB_TRAVERSAL");
    code.Comma().Str("const wxString &name = ");
    if (code.HasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxPanelNameStr");
    code += ");\n\n";
    return true;
}

bool PanelFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_derived_class))
    {
        code.Str((prop_derived_class));
    }
    else
    {
        code += "wxPanel";
    }

    return true;
}

int PanelFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = BaseGenerator::xrc_updated;
    if (node->GetParent() && node->GetParent()->IsSizer())
        result = BaseGenerator::xrc_sizer_item_created;
    auto item = InitializeXrcObject(node, object);

    item.append_attribute("class").set_value("wxPanel");
    object.append_attribute("name").set_value(node->prop_as_string(prop_class_name));

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void PanelFormGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxPanelXmlHandler");
}

bool PanelFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);

    return true;
}
