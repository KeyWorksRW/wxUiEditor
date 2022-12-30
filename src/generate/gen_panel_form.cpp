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
        // Can't use multiview because GetConstantAsInt() searches an unordered_map which requires a std::string to pass to
        // it
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

bool PanelFormGenerator::GenConstruction(Node* node, BaseCodeGenerator* code_gen)
{
    auto src_code = code_gen->GetSrcWriter();

    ttlib::cstr code;
    code << "bool " << node->prop_as_string(prop_class_name) << "::Create";

    code << "(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString "
            "&name)\n{";
    code << "\n\tif (!wxPanel::Create(parent, id, pos, size, style, name))\n\t\treturn false;\n\n";

    src_code->writeLine(code, indent::none);
    code.clear();
    src_code->Indent();

    if (node->HasValue(prop_extra_style))
    {
        code << "SetExtraStyle(GetExtraStyle() | " << node->prop_as_string(prop_extra_style) << ");";
        src_code->writeLine(code);
        code.clear();
    }

    code << GenFontColourSettings(node);
    if (code.size())
    {
        src_code->writeLine(code);
        code.clear();
    }

    src_code->Unindent();
    src_code->writeLine();

    return true;
}

bool PanelFormGenerator::GenPythonForm(Code& code)
{
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
        // Note: this code is called before any indentation is set
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

    code.GenFontColourSettings();
    code.ResetIndent();

    return true;
}

std::optional<ttlib::cstr> PanelFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_header)
    {
        ttlib::cstr code;

        code << node->get_node_name() << "() {}\n";

        code << node->get_node_name() << "(wxWindow* parent, wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
        code << "const wxPoint& pos = ";

        auto position = node->prop_as_wxPoint(prop_pos);
        if (position == wxDefaultPosition)
            code << "wxDefaultPosition, ";
        else
            code << "wxPoint(" << position.x << ", " << position.y << "), ";

        code << "const wxSize& size = ";

        auto size = node->prop_as_wxSize(prop_size);
        if (size == wxDefaultSize)
            code << "wxDefaultSize";
        else
            code << "wxSize(" << size.x << ", " << size.y << ")";

        code << ",\n\tlong style = ";
        if (node->HasValue(prop_style))
            code << node->prop_as_string(prop_style);
        else
            code << "wxTAB_TRAVERSAL";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxPanelNameStr";

        code << ")\n{\n\tCreate(parent, id, pos, size, style, name);\n}\n";

        code << "\nbool Create(wxWindow *parent, ";
        code << "wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
        code << "const wxPoint& pos = ";

        if (position == wxDefaultPosition)
            code << "wxDefaultPosition, ";
        else
            code << "wxPoint(" << position.x << ", " << position.y << "), ";

        code << "const wxSize& size = ";

        if (size == wxDefaultSize)
            code << "wxDefaultSize";
        else
            code << "wxSize(" << size.x << ", " << size.y << ")";

        code << ",\n\tlong style = ";
        if (node->HasValue(prop_style))
            code << node->prop_as_string(prop_style);
        else
            code << "wxTAB_TRAVERSAL";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxPanelNameStr";

        code << ");\n\n";

        return code;
    }
    else if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* panel;
        if (node->IsForm())
        {
            panel = node;
            ASSERT_MSG(panel->GetChildCount(), "Trying to generate code for a wxPanel with no children.")
            if (!panel->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(panel->GetChild(0)->IsSizer(), "Expected first child of a wxPanel to be a sizer.");
            if (panel->GetChild(0)->IsSizer())
                node = panel->GetChild(0);
        }
        else
        {
            panel = node->get_form();
        }

        auto min_size = panel->prop_as_wxSize(prop_minimum_size);
        auto max_size = panel->prop_as_wxSize(prop_maximum_size);
        auto size = panel->prop_as_wxSize(prop_size);

        if (min_size == wxDefaultSize && max_size == wxDefaultSize)
        {
            code << "\tSetSizerAndFit(" << node->get_node_name() << ");";
        }
        else
        {
            code << "\tSetSizer(" << node->get_node_name() << ");";
            if (min_size != wxDefaultSize)
            {
                code << "\n\tSetMinSize(wxSize(" << min_size.GetWidth() << ", " << min_size.GetHeight() << "));";
            }
            if (max_size != wxDefaultSize)
            {
                code << "\n\tSetMaxSize(wxSize(" << max_size.GetWidth() << ", " << max_size.GetHeight() << "));";
            }
            code << "\n\tFit();";
        }

        if (size != wxDefaultSize)
        {
            code << "\n\tSetSize(wxSize(" << size.GetWidth() << ", " << size.GetHeight() << "));";
        }

        return code;
    }
    else if (cmd == code_base_class)
    {
        ttlib::cstr code;
        code << "wxPanel";
        return code;
    }

    return {};
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
