/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCollapsiblePane generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/collpane.h>  // wxCollapsiblePane

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "../mockup/mockup_content.h"  // MockupContent -- Mockup of a form's contents

#include "gen_collapsible.h"

wxObject* CollapsiblePaneGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxCollapsiblePane(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_label),
                              DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (GetMockup()->IsShowingHidden())
        widget->Collapse(false);
    else
        widget->Collapse(node->as_bool(prop_collapsed));

    widget->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &CollapsiblePaneGenerator::OnCollapse, this);

    return widget;
}

void CollapsiblePaneGenerator::OnCollapse(wxCollapsiblePaneEvent& event)
{
    if (auto wxobject = event.GetEventObject(); wxobject)
    {
        auto node = wxGetFrame().GetMockup()->GetNode(wxobject);

        if (wxGetFrame().GetSelectedNode() != node)
        {
            wxGetFrame().GetMockup()->SelectNode(wxobject);
        }
    }

    auto min_size = wxGetFrame().GetMockup()->GetMockupContent()->GetSizer()->GetMinSize();
    wxGetFrame().GetMockup()->GetMockupContent()->SetMinClientSize(min_size);
    wxGetFrame().GetMockup()->Layout();
}

bool CollapsiblePaneGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
    code.PosSizeFlags(true, "wxCP_DEFAULT_STYLE");

    return true;
}

bool CollapsiblePaneGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_collapsed))
    {
        code.Eol(eol_if_empty).NodeName().Function("Collapse(").EndFunction();
    }
    else
    {
        code.Eol(eol_if_empty).NodeName().Function("Expand(").EndFunction();
    }
    return true;
}

bool CollapsiblePaneGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/collpane.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_collpane.cpp
// ../../../wxWidgets/src/xrc/xh_collpane.cpp

int CollapsiblePaneGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxCollapsiblePane");

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_collapsed, "collapsed")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void CollapsiblePaneGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxCollapsiblePaneXmlHandler");
}
