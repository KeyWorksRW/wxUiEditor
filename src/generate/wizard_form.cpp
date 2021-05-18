/////////////////////////////////////////////////////////////////////////////
// Purpose:   Wizard form class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/wizard.h>  // wxWizard class: a GUI control presenting the user with a

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "../mockup/mockup_wizard.h"  // WizardPageSimple

#include "wizard_form.h"

wxObject* WizardFormGenerator::CreateMockup(Node* /* node */, wxObject* /* parent */)
{
    FAIL_MSG("Do not call CreateMockup() for wxWizard -- you must use the MockupWizard class instead!");
    return nullptr;
}

std::optional<ttlib::cstr> WizardFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // By calling the default wxWizard() constructor, we don't need for the caller to pass in wxNullBitmap which will be
    // ignored if a the bitmap property for the wizard has been set. Calling Create() instead gives us the opportunity to
    // first load the image from a header file.

    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id, const wxString& title,";
    code << "\n        const wxPoint& pos, long style) : wxWizard()";
    code << "\n{";

    code << "\n    Create(parent, id, title, ";
    if (node->HasValue(prop_bitmap))
        code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    else
        code << "wxNullBitmap";
    code << ", pos, style);";

    return code;
}

std::optional<ttlib::cstr> WizardFormGenerator::GenCode(const std::string& cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == "ctor_declare")
    {
        // This is the code to add to the header file
        code << node->prop_as_string(prop_class_name)
             << "(wxWindow* parent, wxWindowID id = " << node->prop_as_string(prop_id);
        code << ",\n    const wxString& title = ";
        auto& title = node->prop_as_string(prop_title);
        if (title.size())
        {
            code << GenerateQuotedString(title) << ",\n    ";
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

        code << ",\n    long style = ";
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

    else if (cmd == "base")
    {
        code << "public ";
        if (node->HasValue(prop_base_class_name))
        {
            code << node->prop_as_string(prop_base_class_name);
        }
        else
        {
            code << "wxWizard";
        }
    }
    else if (cmd == "after_addchild")
    {
        auto panes = GetChildPanes(node);
        if (panes.size())
        {
            if (panes.size() > 1)
            {
                code << "    " << panes[0]->prop_as_string(prop_var_name) << "->Chain("
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

std::optional<ttlib::cstr> WizardFormGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    // Wizard and Dialog settings are identical

    auto comp = g_NodeCreator.GetNodeDeclaration("wxDialog")->GetGenerator();
    auto result = comp->GenSettings(node, auto_indent);
    return result.value();
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

    for (size_t pos = 0; pos < parent->GetChildCount(); ++pos)
    {
        auto child = parent->GetChild(pos);
        if (child->isGen(gen_wxWizardPageSimple))
        {
            panes.emplace_back(child);
        }
    }

    return panes;
}

//////////////////////////////////////////  WizardPageGenerator  //////////////////////////////////////////

wxObject* WizardPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    return new MockupWizardPage(node, parent);
}

std::optional<ttlib::cstr> WizardPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->IsLocal())
        code << "auto ";
    code << node->prop_as_string(prop_var_name) << " = new wxWizardPageSimple(this";

    if (node->HasValue(prop_bitmap))
    {
        code << ", nullptr, nullptr, ";

        if (node->HasValue(prop_bitmap))
            code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
        else
            code << "wxNullBitmap";
    }

    code << ");";

    return code;
}
