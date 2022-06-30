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

bool WizardFormGenerator::GenConstruction(Node* node, BaseCodeGenerator* code_gen)
{
    auto src_code = code_gen->GetSrcWriter();

    ttlib::cstr code;

    // By calling the default wxWizard() constructor, we don't need for the caller to pass in wxNullBitmap which will be
    // ignored if the bitmap property for the wizard has been set. Calling Create() instead gives us the opportunity to
    // first load the image.

    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id, const wxString& title,";
    code << "\n\t\tconst wxPoint& pos, long style) : wxWizard()";
    code << "\n{";

    if (node->HasValue(prop_extra_style))
        code << "\n\tSetExtraStyle(" << node->prop_as_string(prop_extra_style) << ");";
    if (node->prop_as_int(prop_border) != 5)
        code << "\n\tSetBorder(" << node->prop_as_string(prop_border) << ");";
    if (node->prop_as_int(prop_bmp_placement))
    {
        code << "\n\tSetBitmapPlacement(" << node->prop_as_string(prop_bmp_placement) << ");";
        if (node->prop_as_int(prop_bmp_min_width) > 0)
            code << "\n\tSetMinimumBitmapWidth(" << node->prop_as_string(prop_bmp_min_width) << ");";
        if (node->HasValue(prop_bmp_background_colour))
            code << "\n\tSetBitmapBackgroundColour(" << GenerateColourCode(node, prop_bmp_background_colour) << ");";
    }

    src_code->writeLine(code, indent::none);
    code.clear();
    src_code->writeLine();
    src_code->Indent();
    code_gen->GenerateHandlers();
    src_code->Unindent();

    if (node->HasValue(prop_bitmap))
    {
        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
        if (is_code_block)
        {
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)\n";
            }
            code << '\t' << bundle_code;
            code << "\n\tCreate(parent, id, title, wxBitmapBundle::FromBitmaps(bitmaps)";
            code << ", pos, style);";
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else";
                code << "\n\tCreate(parent, id, title, ";
                code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
                code << ", pos, style);";
                code << "\n#endif";
            }
            code << "\n\t}";
        }
        else
        {
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)";
            }
            code << "\n\tCreate(parent, id, title, ";
            code << bundle_code;
            code << ", pos, style);";
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else";
                code << "\n\tCreate(parent, id, title, ";
                code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
                code << ", pos, style);";
                code << "\n#endif";
            }
        }
    }
    else
    {
        code << "\n\tCreate(parent, id, title, wxNullBitmap, pos, style);";
    }

    src_code->writeLine(code, indent::none);

    return true;
}

std::optional<ttlib::cstr> WizardFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_header)
    {
        // This is the code to add to the header file
        code << node->prop_as_string(prop_class_name)
             << "(wxWindow* parent, wxWindowID id = " << node->prop_as_string(prop_id);
        code << ",\n\tconst wxString& title = ";
        auto& title = node->prop_as_string(prop_title);
        if (title.size())
        {
            code << GenerateQuotedString(title) << ",\n\t";
        }
        else
        {
            code << "wxEmptyString,\n    ";
        }

        code << "const wxPoint& pos = ";
        auto point = node->prop_as_wxPoint(prop_pos);
        if (point.x != -1 || point.y != -1)
            code << "wxPoint(" << point.x << ", " << point.y << ")";
        else
            code << "wxDefaultPosition";

        code << ",\n\tlong style = ";
        auto& style = node->prop_as_string(prop_style);
        auto& win_style = node->prop_as_string(prop_window_style);
        if (style.empty() && win_style.empty())
            code << "0";
        else
        {
            if (style.size())
            {
                code << style;
                if (win_style.size())
                {
                    code << '|' << win_style;
                }
            }
            else if (win_style.size())
            {
                code << win_style;
            }
        }

        code << ");\n\n";

        code << "bool Run() { return RunWizard((wxWizardPage*) GetPageAreaSizer()->GetItem((size_t) 0)->GetWindow()); "
                "}\n\n";
    }

    else if (cmd == code_base_class)
    {
        if (node->HasValue(prop_derived_class))
        {
            code << node->prop_as_string(prop_derived_class);
        }
        else
        {
            code << "wxWizard";
        }
    }
    else if (cmd == code_after_children)
    {
        auto panes = GetChildPanes(node);
        if (panes.size())
        {
            if (panes.size() > 1)
            {
                code << "\t" << panes[0]->prop_as_string(prop_var_name) << "->Chain("
                     << panes[1]->prop_as_string(prop_var_name) << ")";
                for (size_t pos = 1; pos + 1 < panes.size(); ++pos)
                {
                    code << ".Chain(" << panes[pos + 1]->prop_as_string(prop_var_name) << ")";
                }
                code << ";\n";
            }
            code << "    GetPageAreaSizer()->Add(" << panes[0]->prop_as_string(prop_var_name) << ");\n";
        }

        if (auto& center = node->prop_as_string(prop_center); center.size() && !center.is_sameas("no"))
        {
            code << "    Center(" << center << ");";
        }
    }
    else
    {
        return {};
    }

    return code;
}

std::optional<ttlib::cstr> WizardFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
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
    GenXrcBitmap(node, item);

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

std::optional<ttlib::cstr> WizardPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (!node->HasValue(prop_bitmap))
    {
        if (node->IsLocal())
            code << "auto ";
        code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this);";
    }
    else
    {
        if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
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
                code << "auto ";
            code << node->prop_as_string(prop_var_name)
                 << " = new wxWizardPageSimple(this, nullptr, nullptr, wxBitmapBundle::FromBitmaps(bitmaps));";
            code << "\n\t}";
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n\t";
                if (node->IsLocal())
                    code << "auto ";
                code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this, nullptr, nullptr, ";
                code << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";
                code << "\n#endif";
            }
        }
        else
        {
            if (node->IsLocal())
                code << "auto ";
            code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this, nullptr, nullptr, ";
            code << bundle_code << ");";
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                if (node->IsLocal())
                    code << "auto ";
                code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this, nullptr, nullptr, ";
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
    GenXrcBitmap(node, item);
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
