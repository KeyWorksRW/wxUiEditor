/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for animations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/dir.h>                // wxDir is a class for enumerating the files in a directory
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "pg_animation.h"

#include "node.h"  // Node -- Node class

#include "anim_string_prop.h"  // wxSingleChoiceDialogAdapter

wxIMPLEMENT_ABSTRACT_CLASS(PropertyGrid_Animation, wxPGProperty);

PropertyGrid_Animation::PropertyGrid_Animation(const wxString& label, NodeProperty* prop) : wxPGProperty(label, wxPG_LABEL)
{
    m_img_props.node_property = prop;
    m_value = prop->as_wxString();
    if (prop->HasValue())
    {
        m_img_props.InitValues(prop->as_string().c_str());
    }

    AddPrivateChild(new AnimStringProperty("header_file", m_img_props));
    AddPrivateChild(new wxStringProperty("convert_from", wxPG_LABEL, m_img_props.convert.wx_str()));
}

void PropertyGrid_Animation::RefreshChildren()
{
    wxString value = m_value;
    if (value.size())
    {
        m_img_props.InitValues(value.utf8_str().data());

        Item(0)->SetLabel("header_file");
        Item(1)->SetLabel("convert_from");
        Item(1)->SetHelpString(
            "Specifies the original animation file that should be converted to an animation header file.");
    }

    Item(0)->SetValue(m_img_props.image.wx_str());
    Item(1)->SetValue(m_img_props.convert.wx_str());
}

wxVariant PropertyGrid_Animation::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue) const
{
    wxString value = thisValue;

    AnimationProperties img_props;
    if (value.size())
    {
        img_props.InitValues(value.utf8_str().data());
    }

    switch (childIndex)
    {
        case 0:
            {
                ttlib::cstr results;
                results << childValue.GetString().wx_str();
                auto pos = results.find_first_of('|');
                if (ttlib::is_found(pos))
                {
                    img_props.convert = results.subview(pos + 1);
                    results.erase(pos);
                }
                img_props.image = results;
            }
            break;

        case 1:
            img_props.convert = childValue.GetString().utf8_str().data();
            break;
    }

    value = img_props.CombineValues().wx_str();
    return value;
}
