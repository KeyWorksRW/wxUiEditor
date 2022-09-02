//////////////////////////////////////////////////////////////////////////
// Purpose:   wxInfoBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/infobar.h>  // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/timer.h>    // wxTimer, wxStopWatch and global time-related functions

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_infobar.h"

wxObject* InfoBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    m_infobar = new wxInfoBar(wxStaticCast(parent, wxWindow));

    // Show the message before effects are added in case the show_effect has a delay (which would delay the display of
    // the dialog)

    m_infobar->ShowMessage("Message ...", wxICON_INFORMATION);

    m_infobar->SetShowHideEffects((wxShowEffect) node->prop_as_mockup(prop_show_effect, "info_"),
                                  (wxShowEffect) node->prop_as_mockup(prop_hide_effect, "info_"));
    m_infobar->SetEffectDuration(node->prop_as_int(prop_duration));

    m_infobar->Bind(wxEVT_BUTTON, &InfoBarGenerator::OnButton, this);
    m_infobar->Bind(wxEVT_TIMER, &InfoBarGenerator::OnTimer, this);
    m_infobar->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return m_infobar;
}

std::optional<ttlib::cstr> InfoBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node) << GetParentName(node) << ");";

    return code;
}

std::optional<ttlib::cstr> InfoBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    code << '\t' << node->get_node_name() << "->SetShowHideEffects(" << node->prop_as_constant(prop_show_effect, "info_")
         << ", " << node->prop_as_constant(prop_hide_effect, "info_") << ");";

    if (node->prop_as_int(prop_duration) != 500)
    {
        code << "\n\t" << node->get_node_name() << "->SetEffectDuration(" << node->prop_as_string(prop_duration) << ");";
    }

    return code;
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
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxInfoBar");

    if (node->HasValue(prop_show_effect))
    {
        item.append_child("showeffect").text().set(node->prop_as_constant(prop_show_effect, "info_"));
    }
    if (node->HasValue(prop_hide_effect))
    {
        item.append_child("hideeffect").text().set(node->prop_as_constant(prop_hide_effect, "info_"));
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
