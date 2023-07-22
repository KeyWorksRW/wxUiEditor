/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonPage and wxRibbonPanel generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar

#include "bitmaps.h"     // Contains various images handling functions
#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "image_gen.h"   // Functions for generating embedded images
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_ribbon_page.h"

//////////////////////////////////////////  RibbonPageGenerator  //////////////////////////////////////////

wxObject* RibbonPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto bmp = node->hasValue(prop_bitmap) ? node->as_wxBitmap(prop_bitmap) : wxNullBitmap;
    // REVIEW: This is still a bitmap rather then a bundle as of the 3.1.6 release
    auto widget = new wxRibbonPage((wxRibbonBar*) parent, wxID_ANY, node->as_wxString(prop_label), bmp, 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RibbonPageGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ParentName().Comma().as_string(prop_id);
    code.Comma().QuotedString(prop_label);
    if (code.hasValue(prop_bitmap))
    {
        code.Comma();
        GenerateSingleBitmapCode(code, code.node()->as_string(prop_bitmap));
    }
    code.EndFunction();

    return true;
}

bool RibbonPageGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_select))
    {
        code.ParentName().Function("SetActivePage(").NodeName().EndFunction();
    }

    return true;
}

bool RibbonPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/page.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp
// See Handle_page()

int RibbonPageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonPage");

    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags, "icon");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

//////////////////////////////////////////  RibbonPanelGenerator  //////////////////////////////////////////

wxObject* RibbonPanelGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxRibbonPanel((wxRibbonPage*) parent, wxID_ANY, node->as_wxString(prop_label), node->as_wxBitmap(prop_bitmap),
                          DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RibbonPanelGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
    if (code.hasValue(prop_bitmap))
    {
        code.Comma();
        GenerateSingleBitmapCode(code, code.node()->as_string(prop_bitmap));
    }
    code.EndFunction();

    return true;
}

bool RibbonPanelGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/panel.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp
// See Handle_panel()

int RibbonPanelGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonPanel");

    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags, "icon");

    // Up through wxWidgets 3.1.7, no styles are accepted
    // GenXrcStylePosSize(node, item);
    GenXrcPreStylePosSize(node, item, {});

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}
