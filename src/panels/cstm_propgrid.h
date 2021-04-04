/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxPropertyGrid class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/propgrid.h>

class CustomPropertyGrid : public wxPropertyGrid
{
public:
    CustomPropertyGrid() : wxPropertyGrid() {}

protected:
    bool DoOnValidationFailure(wxPGProperty* property, wxVariant& invalidValue) override;

private:
};
