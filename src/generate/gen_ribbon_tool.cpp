/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/toolbar.h>    // Ribbon-style tool bar

#include "bitmaps.h"     // Contains various images handling functions
#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "image_gen.h"   // Functions for generating embedded images
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_ribbon_tool.h"

wxObject* RibbonToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonToolBar((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size));
    if (node->as_int(prop_min_rows) != 1 || node->as_string(prop_max_rows) != "-1")
    {
        auto min_rows = node->as_int(prop_min_rows);
        auto max_rows = node->as_int(prop_max_rows);
        if (max_rows < min_rows)
            max_rows = min_rows;
        widget->SetRows(min_rows, max_rows);
    }

    return widget;
}

void RibbonToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonToolBar);

    for (const auto& child: node->getChildNodePtrs())
    {
        if (child->isGen(gen_ribbonSeparator))
        {
            btn_bar->AddSeparator();
        }
        else
        {
            auto bundle = child->as_wxBitmapBundle(prop_bitmap);
            wxBitmap bmp;
            if (bundle.IsOk())
                bmp = bundle.GetBitmapFor(wxGetMainFrame()->getWindow());
            else
                bmp = GetInternalImage("default");

            btn_bar->AddTool(wxID_ANY, bmp, child->as_wxString(prop_help), (wxRibbonButtonKind) child->as_int(prop_kind));
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
    auto min_rows = code.node()->as_int(prop_min_rows);
    auto max_rows = code.node()->as_int(prop_max_rows);
    if (min_rows != 1 || max_rows != -1)
    {
        if (max_rows < min_rows)
            max_rows = min_rows;
        code.NodeName().Function("SetRows(").itoa(min_rows, max_rows).EndFunction();
    }

    return true;
}

bool RibbonToolBarGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */,
                                         std::set<std::string>& set_hdr, GenLang /* language */)
{
    // Normally we'd use the access property to determin if the header should be in the source or header file. However,
    // the two events used by this component are also in this header file and the tools themselves are fairly useless
    // without processing the events, so we just add the header file to the header generated file.

    set_hdr.insert("#include <wx/ribbon/toolbar.h>");

    return true;
}

int RibbonToolBarGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */, size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
}

std::optional<tt_string> RibbonToolBarGenerator::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_XRC:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << " XRC currently does not support wxRibbonToolBar ";
                return msg;
            }
        default:
            return {};
    }
}

//////////////////////////////////////////  RibbonToolGenerator  //////////////////////////////////////////

bool RibbonToolGenerator::ConstructionCode(Code& code)
{
    code.ParentName().Function("AddTool(").as_string(prop_id);
    code.Comma();

    tt_string_vector parts(code.node()->as_string(prop_bitmap), BMP_PROP_SEPARATOR, tt::TRIM::both);
    GenerateBundleParameter(code, parts, true);

    code.Comma().CheckLineLength(sizeof("wxEmptyString")).QuotedString(prop_help).Comma().Add(prop_kind).EndFunction();

    return true;
}

int RibbonToolGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */, size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
}
