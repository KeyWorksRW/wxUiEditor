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
        new wxCollapsiblePane(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                              DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (GetMockup()->IsShowingHidden())
        widget->Collapse(false);
    else
        widget->Collapse(node->prop_as_bool(prop_collapsed));

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

std::optional<ttlib::cstr> CollapsiblePaneGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    GeneratePosSizeFlags(node, code, true, "wxCP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> CollapsiblePaneGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_collapsed))
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->Collapse();";
    }
    else
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->Expand();";
    }

    return code;
}

std::optional<ttlib::cstr> CollapsiblePaneGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool CollapsiblePaneGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/collpane.h>", set_src, set_hdr);
    return true;
}
