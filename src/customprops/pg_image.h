/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/property.h>  // wxPGProperty and related support classes

#include "img_props.h"  // ImageProperties -- Handles property grid image properties

class NodeProperty;

class PropertyGrid_Image : public wxPGProperty
{
public:
    wxDECLARE_ABSTRACT_CLASS(PropertyGrid_Image);

    PropertyGrid_Image(const wxString& label, NodeProperty* prop);

    wxVariant ChildChanged(wxVariant& thisValue, int childIndex,
                           wxVariant& childValue) const override;
    auto RefreshChildren() -> void override;

    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrl; }

    [[nodiscard]] auto GetImageProperties() -> ImageProperties& { return m_img_props; }

protected:
    auto SetAutoComplete() -> void;

private:
    ImageProperties m_img_props;

    tt_string m_old_type;
    tt_string m_old_image;

    bool m_isEmbeddedImage { false };
    bool m_isAutoCompleteSet { false };
};
