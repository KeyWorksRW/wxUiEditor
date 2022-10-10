/////////////////////////////////////////////////////////////////////////////
// Purpose:   Event handlers for PreviewSettings dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "../wxui/preview_settings.h"

#include "mainframe.h"  // MainFrame -- Main application window
#include "node.h"       // Node class

void PreviewSettings::OnInit(wxInitDialogEvent& event)
{
    auto* cur_selection = GetMainFrame()->GetSelectedNode();

    if (!cur_selection->isGen(gen_wxDialog) && !cur_selection->isGen(gen_PanelForm))
    {
        if (is_type_both())
        {
            set_type_both(false);
            set_type_cpp(true);
        }
        m_radio_XRC_CPP->Hide();
        Refresh();
    }

    event.Skip();
}
