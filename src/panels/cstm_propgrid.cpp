/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxPropertyGrid class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/msgdlg.h>

#include "cstm_propgrid.h"

#include "mainframe.h"       // Main window frame
#include "propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events

// Most of this code is copied from wxWidgets/src/propgrid/propgrid.cpp, updated to Modern C++ coding practices. The call to
// WasFailureHandled is specific to wxUiEditor, so remove if you use this code in another project. Also note that because the
// message text has been converted to proper English grammar, it will not be automatically translated by wxWidgets in
// non-English languages.

bool CustomPropertyGrid::DoOnValidationFailure(wxPGProperty* property, wxVariant& WXUNUSED(invalidValue))
{
    auto vfb = m_validationInfo.GetFailureBehavior();

    if (vfb & wxPG_VFB_BEEP)
        ::wxBell();

    if ((vfb & wxPG_VFB_MARK_CELL) && !property->HasFlag(wxPG_PROP_INVALID_VALUE))
    {
        auto vfbFg = *wxWHITE;
        auto vfbBg = *wxRED;

        for (unsigned int column = 0; column < m_pState->GetColumnCount(); ++column)
        {
            wxPGCell& cell = property->GetCell(column);
            cell.SetFgCol(vfbFg);
            cell.SetBgCol(vfbBg);
        }

        if (property == GetSelection())
        {
            SetInternalFlag(wxPG_FL_CELL_OVERRIDES_SEL);

            auto editor = GetEditorControl();
            if (editor)
            {
                editor->SetForegroundColour(vfbFg);
                editor->SetBackgroundColour(vfbBg);
            }
        }

        DrawItemAndChildren(property);
    }

    if (wxGetFrame().GetPropPanel()->WasFailureHandled())
    {
        return (vfb & wxPG_VFB_STAY_IN_PROPERTY) ? false : true;
    }

    if (vfb & wxPG_VFB_SHOW_MESSAGEBOX)
    {
        auto msg = m_validationInfo.GetFailureMessage();

        if (msg.empty())
            msg =
                _("You have entered an invalid value. Either change the value, or press ESC to restore the original value.");

        /* TRANSLATORS: Caption of message box displaying any property error */
        ::wxMessageBox(msg, _("Property Error"));
    }

    return (vfb & wxPG_VFB_STAY_IN_PROPERTY) ? false : true;
}
