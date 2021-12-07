/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/bitmap.h>  // wxBitmap class interface

#include "font_prop.h"     // FontProperty class
#include "node_classes.h"  // Forward defintions of Node classes
#include "prop_decl.h"     // PropDeclaration -- PropChildDeclaration and PropDeclaration classes

class wxAnimation;

class NodeProperty
{
public:
    NodeProperty(PropDeclaration* declaration, Node* node);

    void set_value(int integer) { m_value = ttlib::itoa(integer); };
    void set_value(double val);
    void set_value(const wxColour& colour);
    void set_value(const wxString& str);
    void set_value(const wxPoint& point);
    void set_value(const wxSize& size);
    void set_value(const char* val) { m_value.assign(val); }
    void set_value(std::string_view val) { m_value.assign(val); }
    void set_value(ttlib::cstr val) { m_value.assign(val); }
    void set_value(std::string val) { m_value.assign(val); }

    // Returns a non-const reference allowing you to modify the value
    ttlib::cstr& get_value() { return m_value; }

    int as_int() const;
    bool as_bool() const { return (as_int() != 0); };
    double as_float() const;

    const auto& as_string() const { return m_value; }
    auto as_cview() const { return ttlib::cview(m_value.c_str(), m_value.length()); }

    wxAnimation as_animation() const;
    wxBitmap as_bitmap() const;
    wxColour as_color() const;
    wxFont as_font() const;
    FontProperty as_font_prop() const;
    wxPoint as_point() const;
    wxSize as_size() const;
    auto as_wxString() const { return m_value.wx_str(); }
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
    bool HasValue() const;

    const PropDeclaration* GetPropDeclaration() const { return m_declaration; }

    Node* GetNode() { return m_node; }

    // Returns a char pointer to the name. Use get_name() if you want the enum value.
    ttlib::cview DeclName() const noexcept { return m_declaration->DeclName(); }

    bool isProp(PropName name) const noexcept { return m_declaration->isProp(name); }
    bool isType(PropType type) const noexcept { return m_declaration->isType(type); }

    PropType type() const noexcept { return m_declaration->get_type(); }
    const char* name_str() const noexcept { return m_declaration->name_str(); }
    PropName get_name() const noexcept { return m_declaration->get_name(); }

    PropDeclaration* GetPropDeclaration() { return m_declaration; }

    // Currently only called in debug builds, but available for release builds should we need it
    size_t GetPropSize() const { return sizeof(*this) + (m_value.size() + 1); }

private:
    PropDeclaration* m_declaration;
    Node* m_node;  // node this property is a child of
    ttlib::cstr m_value;
};
