/////////////////////////////////////////////////////////////////////////////
// Purpose:   ListCtrl component class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/event.h>     // Event classes
#include <wx/listctrl.h>  // wxSearchCtrlBase class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "listctrl_widgets.h"

//////////////////////////////////////////  ListViewGenerator  //////////////////////////////////////////

wxObject* ListViewGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxListView(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

#if 0
// REVIEW: [KeyWorks - 12-13-2020] This is the original code, but we should be able to do much better by making it possible for the user
// to set up column headers, etc.

        // Refilling
        int i, j;
        wxString buf;
        if ((lc->GetWindowStyle() & wxLC_REPORT) != 0)
        {
            for (i = 0; i < 4; i++)
            {
                buf.Printf("Label %d", i);
                lc->InsertColumn(i, buf, wxLIST_FORMAT_LEFT, 80);
            }
        }

        for (j = 0; j < 10; j++)
        {
            long temp;
            buf.Printf("Cell (0,%d)", j);
            temp = lc->InsertItem(j, buf);
            if ((lc->GetWindowStyle() & wxLC_REPORT) != 0)
            {
                for (i = 1; i < 4; i++)
                {
                    buf.Printf("Cell (%d,%d)", i, j);
                    lc->SetItem(temp, i, buf);
                }
            }
        }
#endif

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ListViewGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    // Note that the default style is not specified, so that it will always be generated. That makes the generated code
    // easier to understand since you know exactly which type of list view is being created instead of having to know what
    // the default it.
    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> ListViewGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ListViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/listctrl.h>", set_src, set_hdr);
    return true;
}
