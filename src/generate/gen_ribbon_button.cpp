/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonBar -- form and regular
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar

#include "bitmaps.h"                            // Contains various images handling functions
#include "code.h"                               // Code -- Helper class for generating code
#include "gen_common.h"                         // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"                      // Common XRC generating functions
#include "image_gen.h"                          // Functions for generating embedded images
#include "node.h"                               // Node class
#include "project_handler.h"                    // ProjectHandler class
#include "utils.h"                              // Utility functions that work with properties
#include "version.h"                            // Version numbers and other constants
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "gen_ribbon_button.h"

wxObject* RibbonButtonBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget = new wxRibbonButtonBar(wxStaticCast(parent, wxRibbonPanel), wxID_ANY,
                                         DlgPoint(node, prop_pos), DlgSize(node, prop_size), 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void RibbonButtonBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node,
                                             bool /* is_preview */)
{
    wxRibbonButtonBar* btn_bar = wxStaticCast(wxobject, wxRibbonButtonBar);

    for (const auto& child: node->get_ChildNodePtrs())
    {
        wxBitmapBundle bundle = child->as_wxBitmapBundle(prop_bitmap);
        if (!bundle.IsOk())
        {
            bundle = GetSvgImage("unknown");
        }

        btn_bar->AddButton(wxID_ANY, child->as_wxString(prop_label), bundle,
                           child->as_wxString(prop_help),
                           static_cast<wxRibbonButtonKind>(child->as_int(prop_kind)));
    }
}

bool RibbonButtonBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ParentName().Comma().as_string(prop_id).PosSizeFlags();

    return true;
}

bool RibbonButtonBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                           std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/buttonbar.h>", set_src, set_hdr);

    return true;
}

int RibbonButtonBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object,
                                           size_t /* xrc_flags */)
{
    pugi::xml_node item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "wxRibbonButtonBar");

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  RibbonButtonGenerator ////////////////////////////////

bool RibbonButtonGenerator::ConstructionCode(Code& code)
{
    code.ParentName().Function("AddButton(").as_string(prop_id).Comma().QuotedString(prop_label);
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

    code.Comma().QuotedString(prop_help).Comma().Add(prop_kind).EndFunction();

    return true;
}

int RibbonButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    pugi::xml_node item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "button");
    ADD_ITEM_PROP(prop_label, "label")

    if (!node->HasValue(prop_bitmap))
    {
        pugi::xml_node bitmap_node = item.append_child("bitmap");
        bitmap_node.append_attribute("stock_id").set_value("wxART_QUESTION");
        bitmap_node.append_attribute("stock_client").set_value("wxART_TOOLBAR");
    }

    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}
