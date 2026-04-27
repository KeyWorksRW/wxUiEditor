/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/toolbar.h>    // Ribbon-style tool bar

#include "bitmaps.h"                            // Contains various images handling functions
#include "code.h"                               // Code -- Helper class for generating code
#include "image_gen.h"                          // Functions for generating embedded images
#include "node.h"                               // Node class
#include "project_handler.h"                    // ProjectHandler class
#include "utils.h"                              // Utility functions that work with properties
#include "version.h"                            // Version numbers and other constants
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "gen_ribbon_tool.h"

wxObject* RibbonToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget = new wxRibbonToolBar(wxStaticCast(parent, wxRibbonPanel), wxID_ANY,
                                       DlgPoint(node, prop_pos), DlgSize(node, prop_size));
    const int min_rows = node->as_int(prop_min_rows);
    const int max_rows = node->as_int(prop_max_rows);
    if (min_rows != 1 || max_rows != -1)
    {
        const int clamped_max = std::max(max_rows, min_rows);
        widget->SetRows(min_rows, clamped_max);
    }

    return widget;
}

// Part of Mockup, called after children have been created.
void RibbonToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node,
                                           bool /* is_preview */)
{
    wxRibbonToolBar* btn_bar = wxStaticCast(wxobject, wxRibbonToolBar);

    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_ribbonSeparator))
        {
            std::ignore = btn_bar->AddSeparator();
        }
        else
        {
            wxBitmapBundle bundle = child->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                bundle = GetSvgImage("unknown");
            }

            btn_bar->AddTool(wxID_ANY, bundle, child->as_wxString(prop_help),
                             static_cast<wxRibbonButtonKind>(child->as_int(prop_kind)));
        }
    }
    btn_bar->Realize();
}

bool RibbonToolBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ParentName().Comma().as_string(prop_id).PosSizeFlags();

    return true;
}

bool RibbonToolBarGenerator::SettingsCode(Code& code)
{
    const int min_rows = code.node()->as_int(prop_min_rows);
    int max_rows = code.node()->as_int(prop_max_rows);
    if (min_rows != 1 || max_rows != -1)
    {
        max_rows = std::max(max_rows, min_rows);
        code.NodeName().Function("SetRows(").itoa(min_rows, max_rows).EndFunction();
    }

    return true;
}

bool RibbonToolBarGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */,
                                         std::set<std::string>& set_hdr, GenLang /* language */)
{
    // Normally we'd use the access property to determine if the header should be in the source or
    // header file. However, the two events used by this component are also in this header file and
    // the tools themselves are fairly useless without processing the events, so we just add the
    // header file to the header generated file.

    set_hdr.insert("#include <wx/ribbon/toolbar.h>");

    return true;
}

int RibbonToolBarGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */,
                                         size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
}

std::optional<wxue::string> RibbonToolBarGenerator::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_XRC:
            {
                wxue::string message;
                if (const Node* form = node->get_Form(); form && form->HasValue(prop_class_name))
                {
                    message << form->as_string(prop_class_name) << ": ";
                }
                message << "XRC currently does not support wxRibbonToolBar";
                return message;
            }
        default:
            return {};
    }
}

//////////////////////////////////////////  RibbonToolGenerator //////////////////////////////////

bool RibbonToolGenerator::ConstructionCode(Code& code)
{
    code.ParentName().Function("AddTool(").as_string(prop_id);
    code.Comma();

    const wxue::StringVector parts(code.node()->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                   wxue::TRIM::both);
    if (code.is_cpp() && Project.get_LangVersion(GEN_LANG_CPLUSPLUS) >= CPP_WIDGETS_VERSION_3_3_0)

    {
        code.GenerateBundleParameter(parts, false);
    }
    else
    {
        code.GenerateBundleParameter(parts, true);
    }

    code.Comma()
        .CheckLineLength(sizeof("wxEmptyString"))
        .QuotedString(prop_help)
        .Comma()
        .Add(prop_kind)
        .EndFunction();

    return true;
}

int RibbonToolGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */,
                                      size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
}
