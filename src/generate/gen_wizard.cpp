/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWizard generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/wizard.h>  // wxWizard class: a GUI control presenting the user with a

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "node.h"           // Node class
#include "project_class.h"  // Project class
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "../mockup/mockup_wizard.h"  // WizardPageSimple
#include "../panels/navpopupmenu.h"   // NavPopupMenu -- Context-menu for Navigation Panel

#include "gen_wizard.h"

wxObject* WizardFormGenerator::CreateMockup(Node* /* node */, wxObject* /* parent */)
{
    FAIL_MSG("Do not call CreateMockup() for wxWizard -- you must use the MockupWizard class instead!");
    return nullptr;
}

bool WizardFormGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.Str((prop_class_name)).Str("::").Str(prop_class_name);
        code += "(wxWindow* parent, wxWindowID id, const wxString& title";
        code.Comma().Str("const wxPoint& pos").Comma().Str("long style)");
        code.Str(" : wxWizard()").Eol() += "{";
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.adv.Wizard):\n");
        code.Eol().Tab().Add("def __init__(self, parent, id=").Add(prop_id);
        code.Indent(3);
        code.Comma().Str("title=").QuotedString(prop_title).Comma().Add("pos=").Pos(prop_pos);
        // wxWizard does not use a size parameter
        code.CheckLineLength(sizeof(", style=") + code.node()->as_string(prop_style).size() + 4);
        code.Comma().Add("style=").Style().Str("):");
        code.Unindent();
        code.Eol() += "wx.adv.Wizard.__init__(self)";

        code.ResetIndent();
    }

    return true;
}

bool WizardFormGenerator::SettingsCode(Code& code)
{
    if (code.HasValue(prop_extra_style))
    {
        code.Eol(eol_if_needed).FormFunction("SetExtraStyle(").FormFunction("GetExtraStyle() | ").Add(prop_extra_style);
        code.EndFunction();
    }

    if (!code.is_value(prop_border, 5))
    {
        code.Eol(eol_if_needed).FormFunction("SetBorder(").Str(prop_border).EndFunction();
    }

    if (code.IntValue(prop_bmp_placement))
    {
        code.Eol(eol_if_needed).FormFunction("SetBitmapPlacement(").Str(prop_bmp_placement).EndFunction();
        if (code.IntValue(prop_bmp_min_width) > 0)
        {
            code.Eol().FormFunction("SetBitmapMinWidth(").Str(prop_bmp_min_width).EndFunction();
        }
        if (code.HasValue(prop_bmp_background_colour))
        {
            code.Eol().FormFunction("SetBitmapBackgroundColour(").ColourCode(prop_bmp_background_colour).EndFunction();
        }
    }

    if (code.HasValue(prop_bitmap))
    {
        auto is_bitmaps_list = BitmapList(code, prop_bitmap);
        if (code.is_cpp() && wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            code.Eol() += "#if wxCHECK_VERSION(3, 1, 6)\n\t";
        }
        if (code.is_cpp())
            code.Eol(eol_if_needed).Str("Create(parent, id, title").Comma();
        else
            code.Eol(eol_if_needed).Str("self.Create(parent, id, title").Comma();
        if (is_bitmaps_list)
        {
            if (code.is_cpp())
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
            else
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
        }
        else
        {
            if (code.is_cpp())
            {
                ttlib::cstr bundle_code;
                GenerateBundleCode(code.node()->as_string(prop_bitmap), bundle_code);
                code.CheckLineLength(bundle_code.size());
                code += bundle_code;
            }
            else
            {
                PythonBundleCode(code, prop_bitmap);
            }
        }
        if (code.is_cpp())
        {
            code.Comma().Str("pos").Comma().Str("style").EndFunction();
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code.Eol() += "#else\n\t";
                code << "wxBitmap(" << GenerateBitmapCode(code.node()->as_string(prop_bitmap)) << ")";
                code.Comma().Str("pos").Comma().Str("style").EndFunction();
                ;
                code.Eol() += "#endif";
            }
        }
        else
        {
            code.Comma().Str("pos").Comma().Str("style").EndFunction();
        }
    }
    else
    {
        code.Eol(eol_if_needed).Str("Create(parent, id, title, wxNullBitmap, pos, style").EndFunction();
    }

    return true;
}

bool WizardFormGenerator::AfterChildrenCode(Code& code)
{
    if (auto panes = GetChildPanes(code.node()); panes.size())
    {
        if (panes.size() > 1)
        {
            code.Eol(eol_if_needed).Str(panes[0]->prop_as_string(prop_var_name)).Function("Chain(");
            code.Str(panes[1]->as_string(prop_var_name)) += ")";
            for (size_t pos = 1; pos + 1 < panes.size(); ++pos)
            {
                code.Str(".Chain(").Str(panes[pos + 1]->as_string(prop_var_name)) += ")";
            }
            if (code.is_cpp())
                code += ";";
        }
        code.Eol(eol_if_needed).FormFunction("GetPageAreaSizer()").Function("Add(");
        code.Str(panes[0]->as_string(prop_var_name)).EndFunction();
    }

    if (auto& center = code.node()->as_string(prop_center); center.size() && !center.is_sameas("no"))
    {
        code.Eol(eol_if_needed).FormFunction("Center(").Add(center).EndFunction();
    }

    return true;
}

bool WizardFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_derived_class))
    {
        code.Str((prop_derived_class));
    }
    else
    {
        code += "wxWizard";
    }
    return true;
}
bool WizardFormGenerator::HeaderCode(Code& code)
{
    auto* node = code.node();

    code.Str(prop_class_name).Str("(wxWindow* parent, wxWindowID id = ").Str(prop_id);
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
    code.EndFunction();
    code.Eol().Eol() +=
        "bool Run() { return RunWizard((wxWizardPage*) GetPageAreaSizer()->GetItem((size_t) 0)->GetWindow()); }";
    code.Eol().Eol();

    return true;
}

bool WizardFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/wizard.h>", set_src, set_hdr);

    // This is needed for the Run() command
    set_hdr.insert("#include <wx/sizer.h>");
    return true;
}

std::vector<Node*> WizardFormGenerator::GetChildPanes(Node* parent)
{
    std::vector<Node*> panes;

    for (const auto& child: parent->GetChildNodePtrs())
    {
        if (child->isGen(gen_wxWizardPageSimple))
        {
            panes.emplace_back(child.get());
        }
    }

    return panes;
}

std::optional<ttlib::cstr> WizardFormGenerator::GetHint(NodeProperty* prop)
{
    if (prop->isProp(prop_title) && !prop->GetNode()->HasValue(prop_title))
    {
        return (ttlib::cstr() << "Title bar text");
    }
    else
    {
        return {};
    }
}

bool WizardFormGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* node)
{
    menu->Append(NavPopupMenu::MenuADD_WIZARD_PAGE, "Add Page");
    menu->Bind(
        wxEVT_MENU,
        [=](wxCommandEvent&)
        {
            node->CreateToolNode(gen_wxWizardPageSimple);
        },
        NavPopupMenu::MenuADD_WIZARD_PAGE);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp

int WizardFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // We use item so that the macros in base_generator.h work, and the code looks the same
    // as other widget XRC generatorsl
    auto item = object;
    GenXrcObjectAttributes(node, item, "wxWizard");

    ADD_ITEM_PROP(prop_title, "title")
    GenXrcBitmap(node, item, xrc_flags);

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

    if (node->HasValue(prop_border) && node->as_int(prop_border) > 0)
        item.append_child("border").text().set(node->prop_as_string(prop_border));

    if (node->HasValue(prop_bmp_placement))
    {
        item.append_child("bitmap-placement").text().set(node->prop_as_string(prop_bmp_placement));
        if (node->as_int(prop_bmp_min_width) > 0)
            item.append_child("bitmap-minwidth").text().set(node->prop_as_string(prop_bmp_min_width));
        if (node->HasValue(prop_bmp_background_colour))
            item.append_child("bitmap-bg")
                .text()
                .set(node->prop_as_wxColour(prop_bmp_background_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }

    if (xrc_flags & xrc::add_comments)
    {
        if (node->prop_as_bool(prop_persist))
            item.append_child(pugi::node_comment).set_value(" persist is not supported in the XRC file. ");

        GenXrcComments(node, item);
    }

    return xrc_updated;
}

void WizardFormGenerator::RequiredHandlers(Node* node, std::set<std::string>& handlers)
{
    handlers.emplace("wxWizardXmlHandler");
    if (node->HasValue(prop_bitmap))
    {
        handlers.emplace("wxBitmapXmlHandler");
    }
}

//////////////////////////////////////////  WizardPageGenerator  //////////////////////////////////////////

wxObject* WizardPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    return new MockupWizardPage(node, parent);
}

bool WizardPageGenerator::ConstructionCode(Code& code)
{
    if (!code.HasValue(prop_bitmap))
    {
        code.AddAuto().Str(prop_var_name).CreateClass().Str(code.is_cpp() ? "this" : "self").EndFunction();
    }
    else
    {
        auto is_bitmaps_list = BitmapList(code, prop_bitmap);
        if (code.is_cpp() && wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            code.Eol() += "#if wxCHECK_VERSION(3, 1, 6)\n\t";
        }
        code.AddAuto().Str(prop_var_name).CreateClass().Str(code.is_cpp() ? "this" : "self");
        if (code.is_cpp())
        {
            code.Comma().Str("nullptr, nullptr").Comma();
        }
        else
        {
            code.Comma().Str("None, None").Comma();
        }
        if (is_bitmaps_list)
        {
            if (code.is_cpp())
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
            else
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
        }
        else
        {
            if (code.is_cpp())
            {
                ttlib::cstr bundle_code;
                GenerateBundleCode(code.node()->as_string(prop_bitmap), bundle_code);
                code.CheckLineLength(bundle_code.size());
                code += bundle_code;
            }
            else
            {
                PythonBundleCode(code, prop_bitmap);
            }
        }
        code.EndFunction();
    }

    return true;
}

std::optional<ttlib::cstr> WizardPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (!node->HasValue(prop_bitmap))
    {
        if (node->IsLocal())
            code << "auto* ";
        code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this);";
    }
    else
    {
        if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            code << "#if wxCHECK_VERSION(3, 1, 6)\n";
        }
        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
        if (is_code_block)
        {
            // GenerateBundleCode assumes an indent within an indent
            bundle_code.Replace("\t\t\t", "\t\t", true);

            code << '\t' << bundle_code << "\t\t";
            if (node->IsLocal())
                code << "auto* ";
            code << node->prop_as_string(prop_var_name)
                 << " = new wxWizardPageSimple(this, nullptr, nullptr, wxBitmapBundle::FromBitmaps(bitmaps));";
            code << "\n\t}";
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n\t";
                if (node->IsLocal())
                    code << "auto* ";
                code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this, nullptr, nullptr, ";
                code << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";
                code << "\n#endif";
            }
        }
        else
        {
            if (node->IsLocal())
                code << "auto* ";
            code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this, nullptr, nullptr, ";
            code << bundle_code << ");";
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                if (node->IsLocal())
                    code << "auto* ";
                code << node->value(prop_var_name) << " = new wxWizardPageSimple(this, nullptr, nullptr, ";
                code << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";
                code << "\n#endif";
            }
        }
    }

    return code;
}

bool WizardPageGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* node)
{
    menu->Append(NavPopupMenu::MenuADD_WIZARD_PAGE, "Add Page");
    menu->Bind(
        wxEVT_MENU,
        [=](wxCommandEvent&)
        {
            node->CreateToolNode(gen_wxWizardPageSimple);
        },
        NavPopupMenu::MenuADD_WIZARD_PAGE);

    if (node->GetChildCount() && node->GetChild(0)->IsSizer())
    {
        menu->MenuAddChildSizerCommands(node->GetChild(0));
    }

    return true;
}

int WizardPageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxWizardPageSimple");
    GenXrcBitmap(node, item, xrc_flags);
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void WizardPageGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxWizardXmlHandler");
}
