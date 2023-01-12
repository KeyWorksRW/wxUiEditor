/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/frame.h>              // wxFrame class interface
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_base.h"    // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "node_prop.h"   // NodeProperty -- NodeProperty class
#include "pugixml.hpp"   // xml_object_range
#include "utils.h"       // Utility functions that work with properties
#include "write_code.h"  // WriteCode -- Write code to Scintilla or file

#include "gen_frame.h"

bool FrameFormGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.Str((prop_class_name)).Str("::").Str(prop_class_name);
        code += "(wxWindow* parent, wxWindowID id, const wxString& title";
        code.Comma().Str("const wxPoint& pos").Comma().Str("const wxSize& size");
        code.Comma().Str("long style").Comma().Str("const wxString& name)");
        code.Str(" : wxFrame()").Eol() += "{";
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.Frame):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").Add(prop_id);
        code.Indent(3);
        code.Comma().Str("title=").QuotedString(prop_title).Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Add("size=").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() + 4);
        code.Add("style=").Style().Comma();
        size_t name_len =
            code.HasValue(prop_window_name) ? code.node()->as_string(prop_window_name).size() : sizeof("wx.DialogNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.HasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code.Str("wx.FrameNameStr");
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.Frame.__init__(self)";
    }

    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool FrameFormGenerator::SettingsCode(Code& code)
{
    if (code.is_cpp())
    {
        if (auto icon_code = GenerateIconCode(code.node()->as_string(prop_icon)); icon_code.size())
        {
            code += icon_code;
            code.Eol();
        }
    }
    else
    {
        // TODO: [Randalphwa - 12-31-2022] Add Python code for setting icon
    }

    if (code.is_cpp())
    {
        code.Eol(eol_if_needed).FormFunction("if (!Create(").Str("parent, id, title, pos, size, style, name))");
        code.Eol().Tab().Str("return;");
    }
    else
    {
        code.Eol(eol_if_needed).Str("if not self.Create(parent, id, title, pos, size, style, name):");
        code.Eol().Tab().Str("return");
    }

    GenFormSettings(code);

    return true;
}

bool FrameFormGenerator::AfterChildrenCode(Code& code)
{
    auto& center = code.node()->as_string(prop_center);
    if (center.size() && !center.is_sameas("no"))
    {
        code.Eol().FormFunction("Centre(").Add(center).EndFunction();
    }

    return true;
}

bool FrameFormGenerator::HeaderCode(Code& code)
{
    auto* node = code.node();

    code.NodeName().Str("(wxWindow* parent, wxWindowID id = ").Str(prop_id);
    code.Comma().Str("const wxString& title = ");
    auto& title = node->prop_as_string(prop_title);
    if (code.HasValue(prop_title))
    {
        code.QuotedString(title);
    }
    else
    {
        code.Str("wxEmptyString");
    }
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
        code.Comma().Str("const wxString &name = wxFrameNameStr");
    }

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool FrameFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_derived_class))
    {
        code.Str((prop_derived_class));
    }
    else
    {
        code += code.node()->DeclName();
    }

    return true;
}

int FrameFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    object.append_attribute("class").set_value("wxFrame");
    object.append_attribute("name").set_value(node->prop_as_string(prop_class_name));

    if (node->HasValue(prop_title))
    {
        object.append_child("title").text().set(node->prop_as_string(prop_title));
    }
    if (node->HasValue(prop_center))
    {
        if (node->prop_as_string(prop_center) == "wxVERTICAL" || node->prop_as_string(prop_center) == "wxHORIZONTAL" ||
            node->prop_as_string(prop_center) == "wxBOTH")
        {
            object.append_child("centered").text().set(1);
        }
        else
        {
            object.append_child("centered").text().set(0);
        }
    }
    if (node->HasValue(prop_icon))
    {
        ttlib::multistr parts(node->prop_as_string(prop_icon), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            ttlib::multistr art_parts(parts[IndexArtID], '|');
            auto icon = object.append_child("icon");
            icon.append_attribute("stock_id").set_value(art_parts[0]);
            icon.append_attribute("stock_client").set_value(art_parts[1]);
        }
        else
        {
            // REVIEW: [KeyWorks - 05-13-2022] As of wxWidgets 3.1.6, SVG files do not work here
            object.append_child("icon").text().set(parts[IndexImage]);
        }
    }

    GenXrcWindowSettings(node, object);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, object);

        if (node->prop_as_string(prop_center) == "wxVERTICAL")
        {
            object.append_child(pugi::node_comment)
                .set_value((" For centering, you cannot set only one direction in the XRC file (set wxBOTH instead)."));
        }

        if (node->prop_as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            object.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }

        if (node->prop_as_bool(prop_persist))
        {
            object.append_child(pugi::node_comment).set_value(" persist is not supported in the XRC file. ");
        }
    }

    return xrc_updated;
}

void FrameFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxFrameXmlHandler");
    if (node->HasValue(prop_icon))
    {
        handlers.emplace("wxIconXmlHandler");
        handlers.emplace("wxBitmapXmlHandler");
    }
}

bool FrameFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/frame.h>", set_src, set_hdr);

    return true;
}

bool FrameFormGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_extra_style))
    {
        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        ttString newValue = property->ValueToString(variant);
        if (newValue.IsEmpty())
            return true;

        if (newValue.contains("wxFRAME_EX_CONTEXTHELP"))
        {
            auto& style = node->prop_as_string(prop_style);
            if (style.contains("wxDEFAULT_FRAME_STYLE") || style.contains("wxMINIMIZE_BOX") ||
                style.contains("wxMINIMIZE_BOX"))
            {
                event->SetValidationFailureMessage(
                    "You can't add a context help button if there is a minimize or maximize button "
                    "(wxDEFAULT_FRAME_STYLE contains these).");
                event->Veto();
                return false;
            }
        }
    }

    return true;
}
