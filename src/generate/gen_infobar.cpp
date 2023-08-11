//////////////////////////////////////////////////////////////////////////
// Purpose:   wxInfoBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>   // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/stattext.h>  // wxStaticText base header
#include <wx/timer.h>     // wxTimer, wxStopWatch and global time-related functions

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties

#include "gen_infobar.h"

wxObject* InfoBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    if (Project.getCodePreference() == GEN_LANG_RUBY)
    {
        auto* widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, "wxInfoBar not available in wxRuby3",
                                        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxBORDER_RAISED);
        widget->Wrap(DlgPoint(parent, 150));
        return widget;
    }
    m_infobar = new wxInfoBar(wxStaticCast(parent, wxWindow));

    // Show the message before effects are added in case the show_effect has a delay (which would delay the display of
    // the dialog)

    m_infobar->ShowMessage("Message ...", wxICON_INFORMATION);

    m_infobar->SetShowHideEffects((wxShowEffect) node->as_mockup(prop_show_effect, "info_"),
                                  (wxShowEffect) node->as_mockup(prop_hide_effect, "info_"));
    m_infobar->SetEffectDuration(node->as_int(prop_duration));

    m_infobar->Bind(wxEVT_BUTTON, &InfoBarGenerator::OnButton, this);
    m_infobar->Bind(wxEVT_TIMER, &InfoBarGenerator::OnTimer, this);
    m_infobar->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return m_infobar;
}

bool InfoBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName();
    if (code.node()->as_string(prop_id) != "wxID_ANY")
        code.Comma().as_string(prop_id);
    code.EndFunction();

    return true;
}

bool InfoBarGenerator::SettingsCode(Code& code)
{
    code.NodeName().Function("SetShowHideEffects(");
    code.Add(code.node()->as_constant(prop_show_effect, "info_"));
    code.Comma().Add(code.node()->as_constant(prop_hide_effect, "info_"));
    code.EndFunction();

    code.Eol().NodeName().Function("SetEffectDuration(").as_string(prop_duration).EndFunction();

    return true;
}

void InfoBarGenerator::OnButton(wxCommandEvent& event)
{
    m_timer.SetOwner(m_infobar);
    m_timer.StartOnce(m_infobar->GetEffectDuration() + 1000);

    event.Skip();
}

void InfoBarGenerator::OnTimer(wxTimerEvent& /* event */)
{
    m_infobar->ShowMessage("Message ...");
}

bool InfoBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/infobar.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_infobar.cpp
// ../../../wxWidgets/src/xrc/xh_infobar.cpp

int InfoBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxInfoBar");

    if (node->hasValue(prop_show_effect))
    {
        item.append_child("showeffect").text().set(node->as_constant(prop_show_effect, "info_"));
    }
    if (node->hasValue(prop_hide_effect))
    {
        item.append_child("hideeffect").text().set(node->as_constant(prop_hide_effect, "info_"));
    }

    ADD_ITEM_PROP(prop_duration, "effectduration")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void InfoBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxInfoBarXmlHandler");
}
