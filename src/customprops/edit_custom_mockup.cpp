/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Property editor for pop_custom_mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

#include "edit_custom_mockup.h"

#include "keywords.h"

EditCustomMockupProperty::EditCustomMockupProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

EditCustomMockupDialog::EditCustomMockupDialog(wxWindow* parent, NodeProperty* prop) :
    EditCustomMockupBase(parent)
{
    m_result = prop->as_wxString();
};

bool EditCustomMockupDialogAdapter::DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                                                 wxPGProperty* /* property unused */)
{
    EditCustomMockupDialog mockup_dialog(wxGetFrame().getWindow(), m_prop);
    if (mockup_dialog.ShowModal() == wxID_OK)
    {
        SetValue(mockup_dialog.GetResults());
        return true;
    }

    return false;
}
