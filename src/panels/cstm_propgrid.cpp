/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxPropertyGrid class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <wx/msgdlg.h>

#include "cstm_propgrid.h"

#include "mainframe.h"       // Main window frame
#include "propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events

// The primary reason for this is to fix the broken English used for the default error message in the wxWidgets code.

// Most of this code originates from wxWidgets/src/propgrid/propgrid.cpp, updated to Modern C++ coding practices. Note that
// because the message text has been converted to proper English grammar, it will not be automatically translated by
// wxWidgets in non-English languages.

bool CustomPropertyGrid::DoOnValidationFailure(wxPGProperty* property, wxVariant& WXUNUSED(invalidValue))
{
    auto validation_behaviour = static_cast<int>(m_validationInfo.GetFailureBehavior());

    if (validation_behaviour & static_cast<int>(wxPGVFBFlags::Beep))
        ::wxBell();

    if ((validation_behaviour & static_cast<int>(wxPGVFBFlags::MarkCell)) &&
        !property->HasFlag(wxPGPropertyFlags::InvalidValue))
    {
        auto foreground_colour = *wxWHITE;
        auto background_colour = *wxRED;

        for (unsigned int column = 0; column < m_pState->GetColumnCount(); ++column)
        {
            auto& cell = property->GetCell(column);
            cell.SetFgCol(foreground_colour);
            cell.SetBgCol(background_colour);
        }

        if (property == GetSelection())
        {
            SetInternalFlag(wxPG_FL_CELL_OVERRIDES_SEL);

            if (auto editor = GetEditorControl(); editor)
            {
                editor->SetForegroundColour(foreground_colour);
                editor->SetBackgroundColour(background_colour);
            }
        }

        DrawItemAndChildren(property);
    }

    // Note that since this is already a customized wxPropertyGrid, we don't call DoShowPropertyError() if
    // wxPG_VFB_SHOW_MESSAGE is set.

    if (validation_behaviour & static_cast<int>(wxPGVFBFlags::ShowMessageBox) ||
        validation_behaviour & static_cast<int>(wxPGVFBFlags::ShowMessage) ||
        validation_behaviour & static_cast<int>(wxPGVFBFlags::ShowMessageOnStatusBar))
    {
        auto msg = m_validationInfo.GetFailureMessage();
        if (msg.empty())
        {
            msg =
                _("You have entered an invalid value. Either change the value, or press ESC to restore the original value.");
        }

        if (validation_behaviour & static_cast<int>(wxPGVFBFlags::ShowMessageOnStatusBar))
        {
            if (!wxPGGlobalVars->m_offline)
            {
                if (auto pStatusBar = GetStatusBar(); pStatusBar)
                {
                    pStatusBar->SetStatusText(msg);
                }
            }
        }

        if (validation_behaviour & static_cast<int>(wxPGVFBFlags::ShowMessageBox))
        {
            // Displaying the message box can cause a focus change which will result in idle processing sending the
            // validation event again. Preserving the focus window avoids validating twice.
            auto focus_window = wxWindow::FindFocus();

            /* TRANSLATORS: Caption of message box displaying any property error */
            ::wxMessageBox(msg, _("Property Error"), wxOK, focus_window);

            if (focus_window)
            {
                focus_window->SetFocus();
            }
        }
    }

    return (validation_behaviour & static_cast<int>(wxPGVFBFlags::StayInProperty)) ? false : true;
}
