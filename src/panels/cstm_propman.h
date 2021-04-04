/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxPropertyGrid class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// In order to use a custom wxPropertyGrid, you have to create a class derived from wxPropertyGridManager and override the
// CreatePropertyGrid method.

#pragma once

#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "cstm_propgrid.h"  // CustomPropertyGrid -- Derived wxPropertyGrid class

class CustomPropertyManager : public wxPropertyGridManager
{
public:
    CustomPropertyManager() : wxPropertyGridManager() {}

protected:
    wxPropertyGrid* CreatePropertyGrid() const { return new CustomPropertyGrid(); };
};
