/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDialog generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dialog.h>  // wxDialogBase class

#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"       // Common component functions
#include "gen_lang_common.h"  // Common mulit-language functions
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // WriteCode -- Write code to Scintilla or file

#include "gen_dialog.h"

// This is only used for Mockup Preview and XrcCompare -- it is not used by the Mockup panel
wxObject* DialogFormGenerator::CreateMockup(Node* node, wxObject* parent)
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

bool DialogFormGenerator::GenConstruction(Node* node, BaseCodeGenerator* code_gen)
{
    auto src_code = code_gen->GetSrcWriter();

    ttlib::cstr code;

    code << "bool " << node->prop_as_string(prop_class_name)
         << "::Create(wxWindow* parent, wxWindowID id, const wxString& title,\n\t\tconst wxPoint& pos, const wxSize& size, "
            "long "
            "style, const wxString &name)\n{\n\tif (!wxDialog::Create(parent, id, title, pos, size, style, "
            "name))\n\t\treturn "
            "false;\n\n";

    src_code->writeLine(code, indent::none);
    src_code->Indent();
    code.clear();

    if (node->HasValue(prop_extra_style))
    {
        code << "SetExtraStyle(GetExtraStyle() | " << node->prop_as_string(prop_extra_style) << ");";
        src_code->writeLine(code);
        code.clear();
    }

    if (node->HasValue(prop_icon))
    {
        code_gen->GenerateHandlers();
        auto icon_code = GenerateIconCode(node->prop_as_string(prop_icon));
        src_code->writeLine(icon_code, indent::auto_keep_whitespace);
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

std::optional<ttlib::cstr> DialogFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> DialogFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* dlg;
        if (node->IsForm())
        {
            dlg = node;
            ASSERT_MSG(dlg->GetChildCount(), "Trying to generate code for a dialog with no children.")
            if (!dlg->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(dlg->GetChild(0)->IsSizer(), "Expected first child of a dialog to be a sizer.");
            if (dlg->GetChild(0)->IsSizer())
                node = dlg->GetChild(0);
        }
        else
        {
            dlg = node->get_form();
        }

        auto min_size = dlg->prop_as_wxSize(prop_minimum_size);
        auto max_size = dlg->prop_as_wxSize(prop_maximum_size);
        auto size = dlg->prop_as_wxSize(prop_size);

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

        auto& center = dlg->prop_as_string(prop_center);
        if (center.size() && !center.is_sameas("no"))
        {
            code << "\n\tCentre(" << center << ");";
        }

        return code;
    }
    else if (cmd == code_header)
    {
        ttlib::cstr code;
        code.reserve(256);

        code << node->get_node_name() << "() {}\n";
        code << node->get_node_name() << "(wxWindow *parent, ";
        code << "wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
        code << "const wxString& title = " << GenerateQuotedString(node, prop_title) << ",\n\t";
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
            code << "wxDEFAULT_DIALOG_STYLE";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxDialogNameStr";

        code << ")\n{\n\tCreate(parent, id, title, pos, size, style, name);\n}\n";

        code << "\nbool Create(wxWindow *parent, ";
        code << "wxWindowID id = " << node->prop_as_string(prop_id) << ", ";
        code << "const wxString& title = " << GenerateQuotedString(node, prop_title) << ",\n\t";
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
            code << "wxDEFAULT_DIALOG_STYLE";

        code << ", const wxString &name = ";
        if (node->HasValue(prop_window_name))
            code << GenerateQuotedString(node, prop_window_name);
        else
            code << "wxDialogNameStr";

        code << ");\n\n";

        return code;
    }

    else
    {
        return GenFormCode(cmd, node);
    }
}

std::optional<ttlib::cstr> DialogFormGenerator::GenPythonAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* dlg;
        if (node->IsForm())
        {
            dlg = node;
            ASSERT_MSG(dlg->GetChildCount(), "Trying to generate code for a dialog with no children.")
            if (!dlg->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(dlg->GetChild(0)->IsSizer(), "Expected first child of a dialog to be a sizer.");
            if (dlg->GetChild(0)->IsSizer())
                node = dlg->GetChild(0);
        }
        else
        {
            dlg = node->get_form();
        }

        auto min_size = dlg->prop_as_wxSize(prop_minimum_size);
        auto max_size = dlg->prop_as_wxSize(prop_maximum_size);
        auto size = dlg->prop_as_wxSize(prop_size);

        if (min_size == wxDefaultSize && max_size == wxDefaultSize)
        {
            code << "\tself.SetSizerAndFit(" << node->get_node_name(GEN_LANG_PYTHON) << ");";
        }
        else
        {
            code << "\tself.SetSizer(" << node->get_node_name(GEN_LANG_PYTHON) << ");";
            if (min_size != wxDefaultSize)
            {
                code << "\n\tself.SetMinSize(wx.Size(" << min_size.GetWidth() << ", " << min_size.GetHeight() << "));";
            }
            if (max_size != wxDefaultSize)
            {
                code << "\n\tself.SetMaxSize(wx.Size(" << max_size.GetWidth() << ", " << max_size.GetHeight() << "));";
            }
            code << "\n\tself.Fit();";
        }

        if (size != wxDefaultSize)
        {
            code << "\n\tself.SetSize(wx.Size(" << size.GetWidth() << ", " << size.GetHeight() << "));";
        }

        auto& center = dlg->prop_as_string(prop_center);
        if (center.size() && !center.is_sameas("no"))
        {
            code << "\n\tself.Centre(" << GetWidgetName(GEN_LANG_PYTHON, center) << ")";
        }

        return code;
    }
    else
    {
        return {};
    }
}

std::optional<ttlib::cstr> DialogFormGenerator::GenLuaAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* dlg;
        if (node->IsForm())
        {
            dlg = node;
            ASSERT_MSG(dlg->GetChildCount(), "Trying to generate code for a dialog with no children.")
            if (!dlg->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(dlg->GetChild(0)->IsSizer(), "Expected first child of a dialog to be a sizer.");
            if (dlg->GetChild(0)->IsSizer())
                node = dlg->GetChild(0);
        }
        else
        {
            dlg = node->get_form();
        }

        auto min_size = dlg->prop_as_wxSize(prop_minimum_size);
        auto max_size = dlg->prop_as_wxSize(prop_maximum_size);
        auto size = dlg->prop_as_wxSize(prop_size);

        if (min_size == wxDefaultSize && max_size == wxDefaultSize)
        {
            code << "\t" << dlg->get_node_name(GEN_LANG_LUA) << ":SetSizerAndFit(" << node->get_node_name() << ");";
        }
        else
        {
            code << "\t" << dlg->get_node_name(GEN_LANG_LUA) << ":SetSizer(" << node->get_node_name() << ");";
            if (min_size != wxDefaultSize)
            {
                code << "\n\t" << dlg->get_node_name(GEN_LANG_LUA) << ":SetMinSize(wx.wxSize(" << min_size.GetWidth() << ", " << min_size.GetHeight() << "));";
            }
            if (max_size != wxDefaultSize)
            {
                code << "\n\t" << dlg->get_node_name(GEN_LANG_LUA) << ":SetMaxSize(wx.wxSize(" << max_size.GetWidth() << ", " << max_size.GetHeight() << "));";
            }
            code << "\n\t" << dlg->get_node_name(GEN_LANG_LUA) << ":Fit();";
        }

        if (size != wxDefaultSize)
        {
            code << "\n\t" << dlg->get_node_name(GEN_LANG_LUA) << ":SetSize(wx.wxSize(" << size.GetWidth() << ", " << size.GetHeight() << "));";
        }

        auto& center = dlg->prop_as_string(prop_center);
        if (center.size() && !center.is_sameas("no"))
        {
            code << "\n\t" << dlg->get_node_name(GEN_LANG_LUA) << ":Centre(" << GetWidgetName(GEN_LANG_LUA, center) << ");";
        }

        return code;
    }
    else
    {
        return {};
    }
}

std::optional<ttlib::cstr> DialogFormGenerator::GenPhpAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* dlg;
        if (node->IsForm())
        {
            dlg = node;
            ASSERT_MSG(dlg->GetChildCount(), "Trying to generate code for a dialog with no children.")
            if (!dlg->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(dlg->GetChild(0)->IsSizer(), "Expected first child of a dialog to be a sizer.");
            if (dlg->GetChild(0)->IsSizer())
                node = dlg->GetChild(0);
        }
        else
        {
            dlg = node->get_form();
        }

        auto min_size = dlg->prop_as_wxSize(prop_minimum_size);
        auto max_size = dlg->prop_as_wxSize(prop_maximum_size);
        auto size = dlg->prop_as_wxSize(prop_size);

        if (min_size == wxDefaultSize && max_size == wxDefaultSize)
        {
            code << "\t\tSetSizerAndFit($" << node->get_node_name() << ");";
        }
        else
        {
            code << "\t\tSetSizer($" << node->get_node_name() << ");";
            if (min_size != wxDefaultSize)
            {
                code << "\n\t\tSetMinSize(new wxSize(" << min_size.GetWidth() << ", " << min_size.GetHeight() << "));";
            }
            if (max_size != wxDefaultSize)
            {
                code << "\n\t\tSetMaxSize(new wxSize(" << max_size.GetWidth() << ", " << max_size.GetHeight() << "));";
            }
            code << "\n\t\tFit();";
        }

        if (size != wxDefaultSize)
        {
            code << "\n\t\tSetSize(new wxSize(" << size.GetWidth() << ", " << size.GetHeight() << "));";
        }

        auto& center = dlg->prop_as_string(prop_center);
        if (center.size() && !center.is_sameas("no"))
        {
            code << "\n\t\tCentre(" << center << ");";
        }

        return code;
    }
    else
    {
        return {};
    }
}

bool DialogFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    return true;
}

std::optional<ttlib::cstr> DialogFormGenerator::GenPythonConstruction(Node* node)
{
    ttlib::cstr code;

    ttlib::cstr dlg_name(node->get_node_name());
    if (dlg_name.size() > 4 && dlg_name.ends_with("Base") && node->get_form()->prop_as_bool(prop_use_derived_class))
    {
        dlg_name.resize(dlg_name.size() - 4);
    }

    code << "class " << dlg_name << "(wx.Dialog):\n";
    code << "\tdef __init__(self, parent):\n\t\twx.Dialog.__init__(self, parent, id=wx.ID_ANY,\n\t\t\ttitle=";
    if (node->HasValue(prop_title))
        code << GeneratePythonQuotedString(node, prop_title) << ",";
    else
        code << "wx.EmptyString,";

    code << "\n\t\t\tpos=";
    auto position = node->prop_as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
        code << "wx.DefaultPosition, ";
    else
        code << "wx.Point(" << position.x << ", " << position.y << "), ";

    code << "size=";
    auto size = node->prop_as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code << "wx.DefaultSize";
    else
        code << "wx.Size(" << size.x << ", " << size.y << ")";

    code << ",\n\t\t\tstyle=";
    if (node->HasValue(prop_style) && !node->prop_as_string(prop_style).is_sameas("wxDEFAULT_DIALOG_STYLE"))
        code << GetWidgetName(GEN_LANG_PYTHON, node->prop_as_string(prop_style));
    else
        code << "wx.DEFAULT_DIALOG_STYLE";
    code << ")";

    return code;
}

std::optional<ttlib::cstr> DialogFormGenerator::GenLuaConstruction(Node* node)
{
    ttlib::cstr code;

    ttlib::cstr dlg_name(node->get_node_name());
    if (dlg_name.size() > 4 && dlg_name.ends_with("Base") && node->get_form()->prop_as_bool(prop_use_derived_class))
    {
        dlg_name.resize(dlg_name.size() - 4);
    }

    code << dlg_name << " = wx.wxDialog(wx.NULL, wx.wxID_ANY, ";
    if (node->HasValue(prop_title))
        code << GenerateLuaQuotedString(node, prop_title) << ",\n\t\t";
    else
        code << "wx.wxEmptyString,\n\t\t";

    auto position = node->prop_as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
        code << "wx.wxDefaultPosition, ";
    else
        code << "wx.wxPoint(" << position.x << ", " << position.y << "), ";

    auto size = node->prop_as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code << "wx.wxDefaultSize,";
    else
        code << "wx.wxSize(" << size.x << ", " << size.y << "),";

    if (node->HasValue(prop_style) && !node->prop_as_string(prop_style).is_sameas("wxDEFAULT_DIALOG_STYLE"))
        code << GetWidgetName(GEN_LANG_LUA, node->prop_as_string(prop_style));
    else
        code << "wx.wxDEFAULT_DIALOG_STYLE";
    code << ")";

    return code;
}

std::optional<ttlib::cstr> DialogFormGenerator::GenPhpConstruction(Node* node)
{
    ttlib::cstr code;

    ttlib::cstr dlg_name(node->get_node_name());
    if (dlg_name.size() > 4 && dlg_name.ends_with("Base") && node->get_form()->prop_as_bool(prop_use_derived_class))
    {
        dlg_name.resize(dlg_name.size() - 4);
    }

    code << "class " << dlg_name << " extends wxDialog\n{";
    code << "\n\tpublic function __construct($parent=null)\n\t{";
    code << "\n\t\tparent::__construct($parent, wxID_ANY, ";
    if (node->HasValue(prop_title))
        code << GeneratePhpQuotedString(node, prop_title) << ",\n\t\t\t";
    else
        code << "wxEmptyString,\n\t\t\t";

    auto position = node->prop_as_wxPoint(prop_pos);
    if (position == wxDefaultPosition)
        code << "new wxPoint(-1, -1), ";
    else
        code << "new wxPoint(" << position.x << ", " << position.y << "), ";

    auto size = node->prop_as_wxSize(prop_size);
    if (size == wxDefaultSize)
        code << "new wxSize(-1, -1), ";
    else
        code << "new wxSize(" << size.x << ", " << size.y << "), ";

    if (node->HasValue(prop_style) && !node->prop_as_string(prop_style).is_sameas("wxDEFAULT_DIALOG_STYLE"))
        code << node->prop_as_string(prop_style);
    else
        code << "wxDEFAULT_DIALOG_STYLE";
    code << ");";

    return code;
}

int DialogFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // We use item so that the macros in base_generator.h work, and the code looks the same
    // as other widget XRC generatorsl
    auto item = object;
    GenXrcObjectAttributes(node, item, "wxDialog");

    ADD_ITEM_PROP(prop_title, "title")

    if (node->HasValue(prop_style))
    {
        if ((xrc_flags & xrc::add_comments) && node->prop_as_string(prop_style).contains("wxWANTS_CHARS"))
        {
            item.append_child(pugi::node_comment)
                .set_value("The wxWANTS_CHARS style will be ignored when the XRC is loaded.");
        }
        if (!node->HasValue(prop_extra_style))
        {
            item.append_child("style").text().set(node->prop_as_string(prop_style));
        }
        else
        {
            ttlib::cstr all_styles = node->prop_as_string(prop_style);
            all_styles << '|' << node->prop_as_string(prop_extra_style);
            item.append_child("style").text().set(all_styles);
        }
    }

    if (node->HasValue(prop_pos))
        item.append_child("pos").text().set(node->prop_as_string(prop_pos));
    if (node->HasValue(prop_size))
        item.append_child("size").text().set(node->prop_as_string(prop_size));

    if (node->HasValue(prop_center))
    {
        if (node->prop_as_string(prop_center).is_sameas("wxVERTICAL") ||
            node->prop_as_string(prop_center).is_sameas("wxHORIZONTAL"))
        {
            if (xrc_flags & xrc::add_comments)
            {
                item.append_child(pugi::node_comment)
                    .set_value((ttlib::cstr(node->prop_as_string(prop_center)) << " cannot be be set in the XRC file."));
            }
            item.append_child("centered").text().set(1);
        }
        else
        {
            item.append_child("centered").text().set(node->prop_as_string(prop_center).is_sameas("no") ? 0 : 1);
        }
    }

    if (node->HasValue(prop_icon))
    {
        ttlib::multistr parts(node->prop_as_string(prop_icon), ';', tt::TRIM::both);
        ASSERT(parts.size() > 1)
        if (parts[IndexType].is_sameas("Art"))
        {
            ttlib::multistr art_parts(parts[IndexArtID], '|');
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
        if (node->prop_as_bool(prop_persist))
            item.append_child(pugi::node_comment).set_value(" persist is not supported in the XRC file. ");

        GenXrcComments(node, item);
    }

    return xrc_updated;
}

void DialogFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxDialogXmlHandler");
    if (node->HasValue(prop_icon))
    {
        handlers.emplace("wxIconXmlHandler");
        handlers.emplace("wxBitmapXmlHandler");
    }
}
