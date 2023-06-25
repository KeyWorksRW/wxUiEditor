/////////////////////////////////////////////////////////////////////////////
// Purpose:   Event handlers for PreviewSettings dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "../tools/preview_settings.h"

#include "mainframe.h"  // MainFrame -- Main application window
#include "node.h"       // Node class

void PreviewSettings::OnInit(wxInitDialogEvent& event)
{
    auto* form_node = GetMainFrame()->GetSelectedNode();
    if (!form_node->IsForm())
    {
        if (form_node->isGen(gen_Project) && form_node->GetChildCount())
        {
            form_node = form_node->GetChild(0);
        }
        else
        {
            form_node = form_node->get_form();
        }
    }

    if (!form_node->isGen(gen_wxDialog) && !form_node->isGen(gen_PanelForm))
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
