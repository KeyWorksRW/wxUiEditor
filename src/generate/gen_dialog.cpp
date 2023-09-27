/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDialog generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dialog.h>  // wxDialogBase class

#include "code.h"           // Code -- Helper class for generating code
#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // Common component functions
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_dialog.h"

// This is only used for Mockup Preview and XrcCompare -- it is not used by the Mockup panel
wxObject* DialogFormGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                              DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->hasValue(prop_extra_style))
    {
        int ex_style = 0;
        // Can't use multiview because getConstantAsInt() searches an unordered_map which requires a std::string to pass to
        // it
        tt_string_vector mstr(node->as_string(prop_extra_style), '|');
        for (auto& iter: mstr)
        {
            // Friendly names will have already been converted, so normal lookup works fine.
            ex_style |= NodeCreation.getConstantAsInt(iter);
        }

        widget->SetExtraStyle(widget->GetExtraStyle() | ex_style);
    }

    return widget;
}

bool DialogFormGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.Str("bool ").as_string(prop_class_name);
        code +=
            "::Create(wxWindow* parent, wxWindowID id, const wxString& title,\n\tconst wxPoint& pos, const wxSize& size, "
            "long style, const wxString &name)";
        code.OpenBrace();

        if (code.hasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed).FormFunction("SetExtraStyle(GetExtraStyle() | ").Add(prop_extra_style);
            code.EndFunction();
        }

        code.Eol(eol_if_needed) += "if (!wxDialog::Create(parent, id, title, pos, size, style, name))";
        code.Eol().Tab() += "return false;\n";
    }
    else if (code.is_python())
    {
        // The Python version creates an empty wx.Dialog and generates the Create() method in
        // SettingsCode(). From the user's perspective, it looks like one-step creation, but
        // it's actually two steps.
        code.Add("class ").NodeName().Str("(wx.Dialog):");
        code.Eol().Tab().Add("def __init__(self, parent, id=").as_string(prop_id);
        code.Indent(3);
        code.Comma().Str("title=").QuotedString(prop_title).Comma().Add("pos=").Pos(prop_pos);
        code.Comma().Str("size=").WxSize(prop_size).Comma();
        code.CheckLineLength(sizeof("style=") + code.node()->as_string(prop_style).size() + 4);
        code.Add("style=").Style().Comma();
        size_t name_len =
            code.hasValue(prop_window_name) ? code.node()->as_string(prop_window_name).size() : sizeof("wx.DialogNameStr");
        code.CheckLineLength(sizeof("name=") + name_len + 4);
        code.Str("name=");
        if (code.hasValue(prop_window_name))
            code.QuotedString(prop_window_name);
        else
            code.Str("wx.DialogNameStr");
        code.Str("):");
        code.Unindent();
        code.Eol() += "wx.Dialog.__init__(self)";
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::Dialog");
        code.Eol().Tab().Add("def initialize(parent");
        // Indent any wrapped lines
        code.Indent(3);
        code.Str(", id = ");
        if (code.hasValue(prop_id))
        {
            code.Add(prop_id);
        }
        else
        {
            code.Add("Wx::ID_ANY");
        }
        code.Comma().Str("title = ").QuotedString(prop_title);
        // We have to break these out in order to add the variable assignment (pos=, size=, etc.)
        code.Comma().CheckLineLength(sizeof("pos = Wx::DEFAULT_POSITION")).Str("pos = ").Pos(prop_pos);
        code.Comma().CheckLineLength(sizeof("size = Wx::DEFAULT_SIZE")).Str("size = ").WxSize(prop_size);
        code.Comma().CheckLineLength(sizeof("style = Wx::DEFAULT_DIALOG_STYLE")).Str("style = ").Style();
        if (code.hasValue(prop_window_name))
        {
            code.Comma().CheckLineLength(sizeof("name = ") + code.as_string(prop_window_name).size() + 2);
            code.Str("name = ").QuotedString(prop_window_name);
        }

        code.EndFunction();
        code.Unindent();
        if (auto indent_pos = code.GetCode().find("parent"); tt::is_found(indent_pos))
        {
            indent_pos -= code.GetCode().find("\n");
            std::string spaces(indent_pos, ' ');
            code.GetCode().Replace("\t\t\t\t", spaces, true);
        }
    }
    else
    {
        code.AddComment("Unknown language");
    }
    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool DialogFormGenerator::SettingsCode(Code& code)
{
    if (code.is_python())
    {
        if (code.hasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed).FormFunction("SetExtraStyle(GetExtraStyle() | ").Add(prop_extra_style);
            code.EndFunction();
        }

        code.Eol(eol_if_needed) += "if not self.Create(parent, id, title, pos, size, style, name):";
        code.Eol().Tab().Str("return");
    }
    else if (code.is_ruby())
    {
        code.Eol(eol_if_needed).Str("super(parent, id, title, pos, size, style)\n");

        if (code.hasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed).FormFunction("SetExtraStyle(");
            code.Function("GetExtraStyle").Str(" | ").Add(prop_extra_style);
            code.EndFunction();
        }
    }
    code.Eol(eol_if_needed).GenFontColourSettings();

    return true;
}

bool DialogFormGenerator::AfterChildrenCode(Code& code)
{
    Node* form = code.node();
    Node* child_node = form;
    ASSERT_MSG(form->getChildCount(), "Trying to generate code for a dialog with no children.")
    if (!form->getChildCount())
        return {};  // empty dialog, so nothing to do
    ASSERT_MSG(form->getChild(0)->isSizer(), "Expected first child of a dialog to be a sizer.");
    if (form->getChild(0)->isSizer())
    {
        // If the first child is not a sizer, then child_node will still point to the dialog
        // node, which means the SetSizer...(child_node) calls below will generate invalid
        // code.
        child_node = form->getChild(0);
    }

    const auto min_size = form->as_wxSize(prop_minimum_size);
    const auto max_size = form->as_wxSize(prop_maximum_size);

    if (min_size == wxDefaultSize && max_size == wxDefaultSize)
    {
        code.FormFunction("SetSizerAndFit(").NodeName(child_node).EndFunction();
    }
    else
    {
        code.FormFunction("SetSizer(").NodeName(child_node).EndFunction();
        if (min_size != wxDefaultSize)
        {
            code.Eol().FormFunction("SetMinSize(").WxSize(prop_minimum_size).EndFunction();
        }
        if (max_size != wxDefaultSize)
        {
            code.Eol().FormFunction("SetMaxSize(").WxSize(prop_maximum_size).EndFunction();
        }
        // EndFunction() will remove the opening paren if Ruby code
        code.Eol().FormFunction("Fit(").EndFunction();
    }

    bool is_focus_set = false;
    auto SetChildFocus = [&](Node* child, auto&& SetChildFocus) -> void
    {
        if (child->hasProp(prop_focus))
        {
            if (child->as_bool(prop_focus))
            {
                code.Eol().NodeName(child).Function("SetFocus(").EndFunction();
                is_focus_set = true;
                return;
            }
        }
        else if (child->getChildCount())
        {
            for (auto& iter: child->getChildNodePtrs())
            {
                SetChildFocus(iter.get(), SetChildFocus);
                if (is_focus_set)
                    return;
            }
        }
    };

    for (auto& iter: form->getChildNodePtrs())
    {
        SetChildFocus(iter.get(), SetChildFocus);
        if (is_focus_set)
        {
            code.Eol();
            break;
        }
    }

    auto& center = form->as_string(prop_center);
    if (center.size() && !center.is_sameas("no"))
    {
        code.Eol().FormFunction("Centre(").Add(center).EndFunction();
    }

    return true;
}

bool DialogFormGenerator::HeaderCode(Code& code)
{
    auto* node = code.node();
    code.NodeName() += "() {}";
    code.Eol().NodeName() += "(wxWindow *parent";
    code.Comma().Str("wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxString& title = ").QuotedString(prop_title);
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

    code.Comma().Eol().Tab().Str("long style = ");
    if (node->hasValue(prop_style))
        code.as_string(prop_style);
    else
        code.Str("wxDEFAULT_DIALOG_STYLE");

    code.Comma().Str("const wxString &name = ");
    if (node->hasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxDialogNameStr");

    code.Str(")").Eol().OpenBrace().Str("Create(parent, id, title, pos, size, style, name);").CloseBrace();

    code.Eol().Str("bool Create(wxWindow *parent");
    code.Comma().Str("wxWindowID id = ").as_string(prop_id);
    code.Comma().Str("const wxString& title = ").QuotedString(prop_title);
    code.Comma().Str("const wxPoint& pos = ");

    if (position == wxDefaultPosition)
        code.Str("wxDefaultPosition");
    else
        code.Pos(prop_pos, no_dlg_units);

    code.Comma().Str("const wxSize& size = ");

    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dlg_units);

    code.Comma().Eol().Tab().Str("long style = ");
    if (node->hasValue(prop_style))
        code.Style();
    else
        code.Str("wxDEFAULT_DIALOG_STYLE");

    code.Comma().Str("const wxString &name = ");
    if (node->hasValue(prop_window_name))
        code.QuotedString(prop_window_name);
    else
        code.Str("wxDialogNameStr");

    // Extra eols at end to force space before "Protected:" section
    code.EndFunction().Eol().Eol();

    return true;
}

bool DialogFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.hasValue(prop_derived_class))
    {
        code.as_string(prop_derived_class);
    }
    else
    {
        code += "wxDialog";
    }

    return true;
}

bool DialogFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    return true;
}

int DialogFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // We use item so that the macros in base_generator.h work, and the code looks the same
    // as other widget XRC generatorsl
    auto item = object;
    GenXrcObjectAttributes(node, item, "wxDialog");

    ADD_ITEM_PROP(prop_title, "title")

    if (node->hasValue(prop_style))
    {
        if ((xrc_flags & xrc::add_comments) && node->as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            item.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }
        if (!node->hasValue(prop_extra_style))
        {
            item.append_child("style").text().set(node->as_string(prop_style));
        }
        else
        {
            tt_string all_styles = node->as_string(prop_style);
            all_styles << '|' << node->as_string(prop_extra_style);
            item.append_child("style").text().set(all_styles);
        }
    }

    if (node->hasValue(prop_pos))
        item.append_child("pos").text().set(node->as_string(prop_pos));
    if (node->hasValue(prop_size))
        item.append_child("size").text().set(node->as_string(prop_size));

    if (node->hasValue(prop_center))
    {
        if (node->as_string(prop_center).is_sameas("wxVERTICAL") || node->as_string(prop_center).is_sameas("wxHORIZONTAL"))
        {
            if (xrc_flags & xrc::add_comments)
            {
                item.append_child(pugi::node_comment)
                    .set_value((tt_string(node->as_string(prop_center)) << " cannot be be set in the XRC file."));
            }
            item.append_child("centered").text().set(1);
        }
        else
        {
            item.append_child("centered").text().set(node->as_string(prop_center).is_sameas("no") ? 0 : 1);
        }
    }

    if (node->hasValue(prop_icon))
    {
        tt_string_vector parts(node->as_string(prop_icon), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            tt_string_vector art_parts(parts[IndexArtID], '|');
            auto icon = item.append_child("icon");
            icon.append_attribute("stock_id").set_value(art_parts[0]);
            icon.append_attribute("stock_client").set_value(art_parts[1]);
        }
        else
        {
            // REVIEW: [KeyWorks - 05-13-2022] As of wxWidgets 3.1.6, SVG files do not work here
            item.append_child("icon").text().set(parts[IndexImage]);
        }
    }

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_bool(prop_persist))
            item.append_child(pugi::node_comment).set_value(" persist is not supported in the XRC file. ");

        GenXrcComments(node, item);
    }

    return xrc_updated;
}

void DialogFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxDialogXmlHandler");
    if (node->hasValue(prop_icon))
    {
        handlers.emplace("wxIconXmlHandler");
        handlers.emplace("wxBitmapXmlHandler");
    }
}
