/////////////////////////////////////////////////////////////////////////////
// Purpose:   Wizard form class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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

    code << "\n\tCreate(parent, id, title, ";
    if (node->HasValue(prop_bitmap))
        code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    else
        code << "wxNullBitmap";
    code << ", pos, style);";

    return code;
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
