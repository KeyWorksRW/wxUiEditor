/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGenericDirCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dirctrl.h>  // Directory control base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_dir_ctrl.h"

//////////////////////////////////////////  GenericDirCtrlGenerator  //////////////////////////////////////////

wxObject* GenericDirCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxGenericDirCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_defaultfolder),
                                       DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node),
                                       node->as_wxString(prop_filter), node->as_int(prop_defaultfilter));

    widget->ShowHidden(node->as_bool(prop_show_hidden));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool GenericDirCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id).Comma();
    if (code.hasValue(prop_defaultfolder))
        code.QuotedString(prop_defaultfolder);
    else
        code.Add("wxDirDialogDefaultFolderStr");

    if (!code.hasValue(prop_filter) && code.IntValue(prop_defaultfilter) == 0 && !code.hasValue(prop_window_name))
    {
        code.PosSizeFlags(code::allow_scaling, false, "wxDIRCTRL_DEFAULT_STYLE");
    }
    else
    {
        code.Comma().Pos().Comma().WxSize().Comma().Style();
        code.Comma().QuotedString(prop_filter).Comma().Add(prop_defaultfilter);
        if (code.hasValue(prop_window_name))
        {
            code.Comma().QuotedString(prop_window_name);
        }
        code.EndFunction();
    }

    return true;
}

bool GenericDirCtrlGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_show_hidden))
    {
        code.NodeName().Function("ShowHidden(").True().EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
        {
            code.NodeName().Function("SetFocus(").EndFunction();
        }
    }

    return true;
}

bool GenericDirCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                          GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/dirctrl.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_gdctl.cpp
// ../../../wxWidgets/src/xrc/xh_gdctl.cpp

int GenericDirCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxGenericDirCtrl");

    ADD_ITEM_PROP(prop_defaultfolder, "defaultfolder")
    ADD_ITEM_PROP(prop_filter, "filter")
    ADD_ITEM_PROP(prop_defaultfilter, "defaultfilter")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void GenericDirCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxGenericDirCtrlXmlHandler");
}
