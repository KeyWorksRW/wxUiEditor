/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonBar -- form and regular
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar

#include "bitmaps.h"     // Contains various images handling functions
#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "image_gen.h"   // Functions for generating embedded images
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_ribbon_button.h"

wxObject* RibbonButtonBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxRibbonButtonBar((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size), 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonButtonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    auto btn_bar = wxStaticCast(wxobject, wxRibbonButtonBar);

    for (const auto& child: node->getChildNodePtrs())
    {
        auto bundle = child->as_wxBitmapBundle(prop_bitmap);
        wxBitmap bmp;
        if (bundle.IsOk())
            bmp = bundle.GetBitmapFor(wxGetMainFrame()->getWindow());
        else
            bmp = GetInternalImage("default");

        btn_bar->AddButton(wxID_ANY, child->as_wxString(prop_label), bmp, child->as_wxString(prop_help),
                           (wxRibbonButtonKind) child->as_int(prop_kind));
    }
}

bool RibbonButtonBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ParentName().Comma().as_string(prop_id).PosSizeFlags();

    return true;
}

bool RibbonButtonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                           GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/buttonbar.h>", set_src, set_hdr);

    return true;
}

int RibbonButtonBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "wxRibbonButtonBar");

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  RibbonButtonGenerator  //////////////////////////////////////////

bool RibbonButtonGenerator::ConstructionCode(Code& code)
{
    code.ParentName().Function("AddButton(").as_string(prop_id).Comma().QuotedString(prop_label);
    code.Comma();

    tt_string_vector parts(code.node()->as_string(prop_bitmap), BMP_PROP_SEPARATOR, tt::TRIM::both);
    GenerateBundleParameter(code, parts, true);

    code.Comma().QuotedString(prop_help).Comma().Add(prop_kind).EndFunction();

    return true;
}

int RibbonButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "button");
    ADD_ITEM_PROP(prop_label, "label")

    if (!node->hasValue(prop_bitmap))
    {
        auto bmp = item.append_child("bitmap");
        bmp.append_attribute("stock_id").set_value("wxART_QUESTION");
        bmp.append_attribute("stock_client").set_value("wxART_TOOLBAR");
    }

    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}
