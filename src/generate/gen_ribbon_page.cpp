/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonPage and wxRibbonPanel generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bmpbndl.h>           // wxBitmapBundle
#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar

#include "code.h"                               // Code -- Helper class for generating code
#include "gen_common.h"                         // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"                      // Common XRC generating functions
#include "image_gen.h"                          // Functions for generating embedded images
#include "node.h"                               // Node class
#include "project_handler.h"                    // ProjectHandler class
#include "utils.h"                              // Utility functions that work with properties
#include "version.h"                            // Version numbers and other constants
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "gen_ribbon_page.h"

////////////////////////////////////  RibbonPageGenerator ///////////////////////////////////////

wxObject* RibbonPageGenerator::CreateMockup(Node* node, wxObject* parent)
{
    const wxBitmapBundle bundle =
        node->HasValue(prop_bitmap) ? node->as_wxBitmapBundle(prop_bitmap) : wxBitmapBundle();
    wxRibbonPage* widget = new wxRibbonPage(wxStaticCast(parent, wxRibbonBar), wxID_ANY,
                                            node->as_wxString(prop_label), bundle, 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RibbonPageGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.Comma().QuotedString(prop_label);
    if (code.HasValue(prop_bitmap))
    {
        code.Comma();

        const wxue::StringVector parts(code.node()->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                       wxue::TRIM::both);
        if (code.is_cpp() &&
            Project.get_LangVersion(GEN_LANG_CPLUSPLUS) >= CPP_WIDGETS_VERSION_3_3_0)
        {
            code.GenerateBundleParameter(parts, false);
        }
        else
        {
            code.GenerateBundleParameter(parts, true);
        }
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

bool RibbonPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                      std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/page.h>", set_src, set_hdr);

    return true;
}

// TODO: [Randalphwa - 04-04-2026] wrong path to xrc file
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp
// See Handle_page()

int RibbonPageGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    const int result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                        BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

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

////////////////////////////////////  RibbonPanelGenerator ///////////////////////////////////////

wxObject* RibbonPanelGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxRibbonPanel* widget =
        new wxRibbonPanel(wxStaticCast(parent, wxRibbonPage), wxID_ANY,
                          node->as_wxString(prop_label), node->as_wxBitmap(prop_bitmap),
                          DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RibbonPanelGenerator::ConstructionCode(Code& code)
{
    code.AddAuto()
        .NodeName()
        .CreateClass()
        .ParentName()
        .Comma()
        .as_string(prop_id)
        .Comma()
        .QuotedString(prop_label);
    if (code.HasValue(prop_bitmap))
    {
        code.Comma();

        const wxue::StringVector parts(code.node()->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                       wxue::TRIM::both);
        if (code.is_cpp() &&
            Project.get_LangVersion(GEN_LANG_CPLUSPLUS) >= CPP_WIDGETS_VERSION_3_3_0)
        {
            code.GenerateBundleParameter(parts, false);
        }
        else
        {
            code.GenerateBundleParameter(parts, true);
        }
    }
    code.EndFunction();

    return true;
}

bool RibbonPanelGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                       std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/panel.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp
// See Handle_panel()

int RibbonPanelGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    const int result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                        BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxRibbonPanel");

    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags, "icon");

    GenXrcPreStylePosSize(node, item, {});

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}
