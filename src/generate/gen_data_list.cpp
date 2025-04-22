/////////////////////////////////////////////////////////////////////////////
// Purpose:   Data List generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>     // Base header for wxPanel
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/stattext.h>  // wxStaticText base header

// Blank line added because wx/stattext.h must be included first
#include <wx/generic/stattextg.h>  // wxGenericStaticText header

#include "gen_data_list.h"

#include "data_handler.h"   // DataHandler class
#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"      // MainFrame -- Main window frame
#include "mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window
#include "node.h"           // Node class
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // Write code to Scintilla or file

// using namespace wxue_data;

//////////////////////////////////////////  ImagesGenerator  //////////////////////////////////////////

// horizontal spacing between the cells in the grid sizer
constexpr int horizontal_spacing = 5;
constexpr int number_of_columns = 2;

wxObject* DataGenerator::CreateMockup(Node* node, wxObject* wxobject)
{
    ProjectData.Initialize();
    auto* parent = wxStaticCast(wxobject, wxWindow);
    // sizer type needs to match "else if (form->isGen(gen_Data))" section of mockup_content.cpp
    auto* flex_grid_sizer =
        new wxFlexGridSizer(number_of_columns, static_cast<int>(node->getChildCount()), horizontal_spacing);

    auto cur_sel_node = wxGetFrame().getSelectedNode();
    if (cur_sel_node->isGen(gen_data_folder))
        node = cur_sel_node;
    for (auto& iter: node->getChildNodePtrs())
    {
        auto* var_name = new wxStaticText(parent, wxID_ANY, iter->as_string(prop_var_name));
        flex_grid_sizer->Add(var_name, wxSizerFlags().Border(wxALL));

        // wxGenericStaticText used so that at some point we can make the text red if the file
        // cannot be found.
        auto* file_name = new wxGenericStaticText(parent, wxID_ANY, iter->as_string(prop_data_file));
        flex_grid_sizer->Add(file_name, wxSizerFlags().Border(wxALL));
    }

    return flex_grid_sizer;
}

int DataGenerator::GetRequiredVersion(Node* /* node */)
{
    // return minRequiredVer + 5;  // 1.2.1 release
    return minRequiredVer + 4;  // 1.2.0 release
}
