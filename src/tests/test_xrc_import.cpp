/////////////////////////////////////////////////////////////////////////////
// Purpose:   XRC Import tests
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// XRC exporting can be verified simply by running the XRC Preview dialog. What this test
// does is create an XML document containing the XRC for the selected node, then imports that
// document letting the standard MSG_INFO messages to let us know if there were problems.

#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class

void MainFrame::OnTestXrcImport(wxCommandEvent& /* event */)
{
    ASSERT_MSG(m_selected_node, "No node selected");


}
