/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "font_prop.h"     // FontProperty class
#include "node_classes.h"  // Forward defintions of Node classes
#include "prop_info.h"     // PropertyInfo -- PropDefinition and PropertyInfo classes

class NodeProperty
{
public:
    NodeProperty(PropertyInfo* info, Node* node);

    void set_value(int integer) { m_value = ttlib::itoa(integer); };
    void set_value(double val);
    void set_value(const wxColour& colour);
    void set_value(const wxString& str);
    void set_value(const wxPoint& point);
    void set_value(const wxSize& size);
    void set_value(const char* val) { m_value.assign(val); }
    void set_value(ttlib::cview val) { m_value.assign(val); }
    void set_value(ttlib::cstr val) { m_value.assign(val); }

    // Returns a non-const reference allowing you to modify the value
    ttlib::cstr& get_value() { return m_value; }

    int as_int() const;
    bool as_bool() const { return (as_int() != 0); };
    double as_float() const;
    const auto& as_string() const { return m_value; }

    wxBitmap as_bitmap() const;
    wxColour as_color() const;
    wxFont as_font() const;
    FontProperty as_font_prop() const;
    wxPoint as_point() const;
    wxSize as_size() const;
    wxString as_wxString() const { return m_value.wx_str(); }
    ttlib::cview as_cview() const { return m_value.subview(); }
    wxArrayString as_wxArrayString() const;

    auto as_vector() const -> std::vector<ttlib::cstr>;

    // This first doubles the backslash in escaped characters (\\n, \\t, \\r, and \\)
    ttlib::cstr as_escape_text() const;

    operator bool() const { return (as_int() != 0); }
    operator int() const { return as_int(); }
    operator const char*() const { return m_value.c_str(); }

    operator wxBitmap() const { return as_bitmap(); }
    operator wxColour() const { return as_color(); }
    operator wxFont() const { return as_font(); }
    operator wxPoint() const { return as_point(); }
    operator wxSize() const { return as_size(); }

    bool IsDefaultValue() const;

    // Returns false if the property is empty. For size, point, and Bitmap properties,
    // returns false if the default value is used.
    bool HasValue();

    void splitParentProperty(std::map<ttlib::cstr, ttlib::cstr>& children) const;
    void SplitParentProperty(std::map<wxString, wxString>* children) const;

    const PropertyInfo* GetPropertyInfo() const { return m_info; }

    NodeEnums::PropType type() const noexcept { return m_info->type(); }

    ttlib::cstr getChildFromParent(const ttlib::cstr& childName) const;

    Node* GetNode() { return m_node; }
    const ttlib::cstr& GetPropName() const;

    const char* prop_name_as_string() const noexcept { return m_info->name_as_string(); }
    NodeEnums::Prop prop_name() const noexcept { return m_info->name(); }

    PropertyInfo* GetPropertyInfo() { return m_info; }

    auto& GetValue() { return m_value; }

private:
    PropertyInfo* m_info;
    Node* m_node;  // node this property is a child of
    ttlib::cstr m_value;
};
