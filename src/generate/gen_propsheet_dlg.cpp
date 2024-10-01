/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPropertySheetDialog generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bookctrl.h>  // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/choicebk.h>  // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/dialog.h>    // wxDialogBase class
#include <wx/listbook.h>  // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/toolbook.h>  // wxToolbook: wxToolBar and wxNotebook combination
#include <wx/treebook.h>  // wxTreebook: wxNotebook-like control presenting pages in a tree

#include "code.h"           // Code -- Helper class for generating code
#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // Common component functions
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_propsheet_dlg.h"

// This is only used for Mockup Preview and XrcCompare -- it is not used by the Mockup panel
wxObject* PropSheetDlgGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxWindow* widget = nullptr;
    const auto& book_type = node->as_string(prop_book_type);
    if (book_type == "wxPROPSHEET_CHOICEBOOK")
    {
        widget = new wxChoicebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                                  DlgSize(node, prop_size), GetStyleInt(node));
    }

    else if (book_type == "wxPROPSHEET_LISTBOOK")
    {
        widget = new wxListbook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                                GetStyleInt(node));
    }
    else if (book_type == "wxPROPSHEET_TREEBOOK")
    {
        widget = new wxTreebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                                GetStyleInt(node));
    }
    else if (book_type == "wxPROPSHEET_TOOLBOOK")
    {
        widget = new wxToolbook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                                GetStyleInt(node));
    }
    else  // default to wxPROPSHEET_NOTEBOOK
    {
        widget = new wxNotebook(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                                GetStyleInt(node));
    }

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

    if (node->isPropValue(prop_variant, "small"))
        widget->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    else if (node->isPropValue(prop_variant, "mini"))
        widget->SetWindowVariant(wxWINDOW_VARIANT_MINI);
    else if (node->isPropValue(prop_variant, "large"))
        widget->SetWindowVariant(wxWINDOW_VARIANT_LARGE);

    return widget;
}

bool PropSheetDlgGenerator::ConstructionCode(Code& code)
{
    auto* node = code.node();  // for convenience
    if (code.is_cpp())
    {
        code.Str("bool ").as_string(prop_class_name);
        code += "::Create(wxWindow* parent, wxWindowID id, const wxString& title,\n\tconst "
                "wxPoint& pos, const wxSize& size, long style, const wxString& name)\n";
        code.OpenBrace();

        if (code.hasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed).FormFunction("SetExtraStyle(GetExtraStyle() | ").Add(prop_extra_style);
            code.EndFunction();
        }

        code.Eol(eol_if_needed).FormFunction("SetSheetStyle(").Add(prop_book_type).EndFunction().Eol();
        if (node->as_int(prop_inner_border) >= 0)
        {
            code.FormFunction("SetSheetInnerBorder(").Add(prop_inner_border).EndFunction().Eol();
        }
        if (node->as_int(prop_outer_border) >= 0)
        {
            code.FormFunction("SetSheetOuterBorder(").Add(prop_outer_border).EndFunction().Eol();
        }

        code.Eol(eol_if_needed) += "if (!";
        if (code.node()->hasValue(prop_subclass))
            code.as_string(prop_subclass);
        else
            code += "wxPropertySheetDialog";
        code += "::Create(parent, id, title, pos, size, style, name))";
        code.Eol().Tab() += "return false;\n";

        code.Eol().Str("CreateButtons(").Add(prop_buttons).EndFunction();
    }
    else if (code.is_python())
    {
        code.Add("class ").NodeName().Str("(wx.adv.PropertySheetDialog):");
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
        code.Eol().Str("wx.adv.PropertySheetDialog.__init__(self)");
        auto book_type = node->as_string(prop_book_type);
        book_type.erase(0, 2);  // Remove the "wx" from the front
        code.Eol().FormFunction("SetSheetStyle(wx.adv.").Str(book_type).EndFunction().Eol();
        if (node->as_int(prop_inner_border) >= 0)
        {
            code.FormFunction("SetSheetInnerBorder(").Add(prop_inner_border).EndFunction().Eol();
        }
        if (node->as_int(prop_outer_border) >= 0)
        {
            code.FormFunction("SetSheetOuterBorder(").Add(prop_outer_border).EndFunction().Eol();
        }

        code.Eol().Str("if not self.Create(parent, id, title, pos, size, style, name):").Eol().Tab().Str("return");
        code.Eol().FormFunction("CreateButtons(").Add(prop_buttons).EndFunction().Eol();
    }
    else if (code.is_ruby())
    {
        code.Add("class ").NodeName().Add(" < Wx::PropertySheetDialog");
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

        // Try to line up the parameters with the "parent" parameter
        if (auto indent_pos = code.GetCode().find("parent"); tt::is_found(indent_pos))
        {
            indent_pos -= code.GetCode().find("\n");
            std::string spaces(indent_pos, ' ');
            code.GetCode().Replace("\t\t\t\t", spaces, true);
        }

        code.Eol().Str("super()");

        code.Eol().FormFunction("SetSheetStyle(").Add(prop_book_type).EndFunction().Eol();
        if (node->as_int(prop_inner_border) >= 0)
        {
            code.FormFunction("SetSheetInnerBorder(").Add(prop_inner_border).EndFunction().Eol();
        }
        if (node->as_int(prop_outer_border) >= 0)
        {
            code.FormFunction("SetSheetOuterBorder(").Add(prop_outer_border).EndFunction().Eol();
        }

        code.Eol().Str("create(parent, id, title, pos, size, style, name)").Eol();
        code.Eol().FormFunction("CreateButtons(").Add(prop_buttons).EndFunction().Eol();
    }
    else
    {
        code.AddComment("Unknown language", true);
    }
    code.ResetIndent();
    code.ResetBraces();  // In C++, caller must close the final brace after all construction

    return true;
}

bool PropSheetDlgGenerator::SettingsCode(Code& code)
{
    if (code.is_python())
    {
        if (code.hasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed).FormFunction("SetExtraStyle(GetExtraStyle() | ").Add(prop_extra_style);
            code.EndFunction();
        }
    }
    else if (code.is_ruby())
    {
        if (code.hasValue(prop_extra_style))
        {
            code.Eol(eol_if_needed).FormFunction("SetExtraStyle(");
            code.Function("GetExtraStyle").Str(" | ").Add(prop_extra_style);
            code.EndFunction();
        }
    }
    code.Eol(eol_if_needed).GenFontColourSettings();

    // Note: variant must be set *after* any font is set, or it will be ignored because a new font
    // was set after the variant modified the original font.
    if (!code.node()->isPropValue(prop_variant, "normal"))
    {
        // code.Eol(eol_if_empty).FormFunction("SetWindowVariant(");
        code.Eol(eol_if_empty).FormFunction("GetBookCtrl()").Function("SetWindowVariant(");
        if (code.node()->isPropValue(prop_variant, "small"))
            code.Add("wxWINDOW_VARIANT_SMALL");
        else if (code.node()->isPropValue(prop_variant, "mini"))
            code.Add("wxWINDOW_VARIANT_MINI");
        else
            code.Add("wxWINDOW_VARIANT_LARGE");

        code.EndFunction();
    }

    return true;
}

bool PropSheetDlgGenerator::AfterChildrenCode(Code& code)
{
    code.FormFunction("LayoutDialog(").Add(prop_center).EndFunction();

    return true;
}

bool PropSheetDlgGenerator::HeaderCode(Code& code)
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
        code.Pos(prop_pos, no_dpi_scaling);

    code.Comma().Str("const wxSize& size = ");

    auto size = node->as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dpi_scaling);

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
        code.Pos(prop_pos, no_dpi_scaling);

    code.Comma().Str("const wxSize& size = ");

    if (size == wxDefaultSize)
        code.Str("wxDefaultSize");
    else
        code.WxSize(prop_size, no_dpi_scaling);

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

bool PropSheetDlgGenerator::BaseClassNameCode(Code& code)
{
    if (code.hasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += "wxPropertySheetDialog";
    }

    return true;
}

bool PropSheetDlgGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                        GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/propdlg.h>", set_src, set_hdr);
    set_src.insert("#include <wx/bookctrl.h>");

    return true;
}

int PropSheetDlgGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // We use item so that the macros in base_generator.h work, and the code looks the same
    // as other widget XRC generatorsl
    auto item = object;
    GenXrcObjectAttributes(node, item, "wxPropertySheetDialog");

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

void PropSheetDlgGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxPropertySheetDialogXmlHandler");
    if (node->hasValue(prop_icon))
    {
        handlers.emplace("wxIconXmlHandler");
        handlers.emplace("wxBitmapXmlHandler");
    }
}

void PropSheetDlgGenerator::GenEvent(Code& code, NodeEvent* event, const std::string& class_name)
{
    BaseGenerator::GenEvent(code, event, class_name);
}
