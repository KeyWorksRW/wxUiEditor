/////////////////////////////////////////////////////////////////////////////
// Purpose:   Grid component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/event.h>             // Event classes
#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "grid_widgets.h"

//////////////////////////////////////////  PropertyGridManagerGenerator  //////////////////////////////////////////

wxObject* PropertyGridManagerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPropertyGridManager(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                            DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_extra_style))
    {
        widget->SetExtraStyle(node->prop_as_int(prop_extra_style));
    }

    // BUGBUG: [KeyWorks - 04-11-2021] There is no "show_header" property
    // widget->ShowHeader(node->prop_as_int(prop_show_header) != 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void PropertyGridManagerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node,
                                                 bool /* is_preview */)
{
    auto pgm = wxStaticCast(wxobject, wxPropertyGridManager);

    for (const auto& child: node->GetChildNodePtrs())
    {
        if (child->isGen(gen_propGridPage))
        {
            wxPropertyGridPage* page =
                pgm->AddPage(child->prop_as_wxString(prop_label), child->prop_as_wxBitmapBundle(prop_bitmap));

            for (const auto& inner_child: child->GetChildNodePtrs())
            {
                if (inner_child->isGen(gen_propGridItem))
                {
                    if (inner_child->prop_as_string(prop_type) == "Category")
                    {
                        page->Append(new wxPropertyCategory(inner_child->prop_as_wxString(prop_label),
                                                            inner_child->prop_as_wxString(prop_label)));
                    }
                    else
                    {
                        wxPGProperty* prop = wxDynamicCast(
                            wxCreateDynamicObject("wx" + (inner_child->prop_as_string(prop_type)) + "Property"),
                            wxPGProperty);
                        if (prop)
                        {
                            prop->SetLabel(inner_child->prop_as_wxString(prop_label));
                            prop->SetName(inner_child->prop_as_wxString(prop_label));
                            page->Append(prop);

                            if (inner_child->HasValue(prop_help))
                            {
                                page->SetPropertyHelpString(prop, inner_child->prop_as_wxString(prop_help));
                            }
                        }
                    }
                }
            }
        }
    }

    if (node->GetChildCount())
    {
        pgm->SelectPage(0);
    }

    pgm->Update();
}

std::optional<ttlib::cstr> PropertyGridManagerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxPGMAN_DEFAULT_STYLE");

    code.Replace(", wxID_ANY);", ");");

    if (node->HasValue(prop_extra_style))
        code << "\n\t" << node->get_node_name() << "->SetExtraStyle(" << node->prop_as_string(prop_extra_style) << ");";

    return code;
}

bool PropertyGridManagerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/propgrid/propgrid.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/propgrid/manager.h>", set_src, set_hdr);

    if (node->prop_as_bool(prop_include_advanced))
        InsertGeneratorInclude(node, "#include <wx/propgrid/advprops.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  PropertyGridPageGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> PropertyGridPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->HasValue(prop_bitmap))
    {
        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
        if (is_code_block)
        {
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)\n";
            }
            // GenerateBundleCode assumes an indent within an indent
            bundle_code.Replace("\t\t\t", "\t\t", true);
            code << '\t' << bundle_code;
            code << "\t\t";
            if (node->IsLocal())
                code << "auto* ";
            code << node->get_node_name() << " = " << node->get_parent_name() << "->AddPage(";
            code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
                 << "wxBitmapBundle::FromBitmaps(bitmaps));";
            code << "\n\t}";
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n\t";
                if (node->IsLocal())
                    code << "auto* ";
                code << node->get_node_name() << " = " << node->get_parent_name() << "->AddPage(";
                code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
                     << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";
                code << "\n#endif";
            }
        }
        else
        {
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)\n";
            }
            if (node->IsLocal())
                code << "auto* ";
            code << node->get_node_name() << " = " << node->get_parent_name() << "->AddPage(";
            code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", " << bundle_code << ");";
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                if (node->IsLocal())
                    code << "auto* ";
                code << node->get_node_name() << " = " << node->get_parent_name() << "->AddPage(";
                code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
                     << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";
                code << "\n#endif";
            }
        }
    }
    else
    {
        if (node->IsLocal())
            code << "auto* ";
        code << node->get_node_name() << " = " << node->get_parent_name() << "->AddPage(";
        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ");";
    }

    return code;
}
