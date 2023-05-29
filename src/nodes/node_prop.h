/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "image_handler.h"  // ProjectImage class

#include "font_prop.h"     // FontProperty class
#include "node_classes.h"  // Forward defintions of Node classes
#include "prop_decl.h"     // PropDeclaration -- PropChildDeclaration and PropDeclaration classes

class wxAnimation;

struct NODEPROP_STATUSBAR_FIELD
{
    tt_string style;
    tt_string width;
};

struct NODEPROP_CHECKLIST_ITEM
{
    tt_string label;
    tt_string checked;  // 1 is true, 0 or empty is false
};

struct NODEPROP_RADIOBOX_ITEM
{
    tt_string label;
    tt_string enabled;  // 1 is enabled, empty or 0 is disabled
    tt_string show;     // 1 is shown, empty or 0 is hidden
    tt_string tooltip;
    tt_string helptext;
};

struct NODEPROP_BMP_COMBO_ITEM
{
    tt_string label;
    tt_string bitmap;  // assumes embedded, svg only if .svg file extension, and svg will default to 16x16
};

class NodeProperty
{
public:
    NodeProperty(PropDeclaration* declaration, Node* node);

    void set_value(int integer) { m_value = tt::itoa(integer); };
    void set_value(double val);
    void set_value(const wxColour& colour);
    void set_value(const wxString& str);
    void set_value(const wxPoint& point);
    void set_value(const wxSize& size);
    void set_value(const char* val) { m_value.assign(val); }
    void set_value(std::string_view val) { m_value.assign(val); }
    void set_value(tt_string val) { m_value.assign(val); }
    void set_value(std::string val) { m_value.assign(val); }

    tt_string convert_statusbar_fields(std::vector<NODEPROP_STATUSBAR_FIELD>& fields) const;
    tt_string convert_checklist_items(std::vector<NODEPROP_CHECKLIST_ITEM>& fields) const;
    tt_string convert_radiobox_items(std::vector<NODEPROP_RADIOBOX_ITEM>& fields) const;
    tt_string convert_bmp_combo_items(std::vector<NODEPROP_BMP_COMBO_ITEM>& fields) const;

    void set_value(std::vector<NODEPROP_STATUSBAR_FIELD>& fields) { m_value = convert_statusbar_fields(fields); }
    void set_value(std::vector<NODEPROP_CHECKLIST_ITEM>& items) { m_value = convert_checklist_items(items); }
    void set_value(std::vector<NODEPROP_RADIOBOX_ITEM>& items) { m_value = convert_radiobox_items(items); }
    void set_value(std::vector<NODEPROP_BMP_COMBO_ITEM>& items) { m_value = convert_bmp_combo_items(items); }

    // By returnung a vector instead of the string, the way the property string gets formatted is entirely up to
    // NodeProperty. This allows, for example, to use '|' as a separator instead of ',' when there is text which contains a
    // comma.

    std::vector<NODEPROP_STATUSBAR_FIELD> as_statusbar_fields() const;
    std::vector<NODEPROP_CHECKLIST_ITEM> as_checklist_items() const;
    std::vector<NODEPROP_RADIOBOX_ITEM> as_radiobox_items() const;
    std::vector<NODEPROP_BMP_COMBO_ITEM> as_bmp_combo_items() const;

    // Returns a non-const reference allowing you to modify the value. Do *NOT* use this for
    // the vector functions, as the formatting of the property string is entirely up to
    // NodeProperty.
    tt_string& get_value() { return m_value; }

    const tt_string& value() const { return m_value; }

    int as_int() const;
    bool as_bool() const { return (as_int() != 0); };
    double as_float() const;

    // Looks up wx constant, returns it's numerical value.
    //
    // Returns wxID_ANY if constant is not found
    int as_id() const;

    // tt_string_view as_sview() const { return m_value; }

    // Use with caution! This allows you to modify the property string directly.
    auto as_raw_ptr() { return &m_value; }

    wxAnimation as_animation() const;
    wxBitmap as_bitmap() const;
    wxColour as_color() const;
    wxFont as_font() const;
    FontProperty as_font_prop() const;
    wxPoint as_point() const;
    wxSize as_size() const;
    wxArrayString as_wxArrayString() const;

    // On Windows this will first convert to UTF-16 unless wxUSE_UNICODE_UTF8 is set.
    wxString as_wxString() const { return m_value.wx_str(); }

    wxBitmapBundle as_bitmap_bundle() const;
    const ImageBundle* as_image_bundle() const;

    const tt_string& as_string() const { return m_value; }

    // Converts friendly name to wxWidgets constant
    const tt_string& as_constant(std::string_view prefix);

    // Converts friendly name to wxWidgets constant, and then returns the integer value of
    // that constant.
    int as_mockup(std::string_view prefix) const;

    auto as_vector() const -> std::vector<tt_string>;

    // This first doubles the backslash in escaped characters (\\n, \\t, \\r, and \\)
    tt_string as_escape_text() const;

    operator bool() const { return (as_int() != 0); }
    operator int() const { return as_int(); }
    operator const char*() const { return m_value.c_str(); }

    operator wxBitmap() const { return as_bitmap(); }
    operator wxColour() const { return as_color(); }
    operator wxFont() const { return as_font(); }
    operator wxPoint() const { return as_point(); }
    operator wxSize() const { return as_size(); }

    bool IsDefaultValue() const;
    const tt_string& GetDefaultValue() const noexcept { return m_declaration->GetDefaultValue(); }

    // Returns false if the property is empty. For size, point, and Bitmap properties,
    // returns false if the default value is used.
    bool HasValue() const;

    const PropDeclaration* GetPropDeclaration() const { return m_declaration; }

    Node* GetNode() { return m_node; }

    // Returns a char pointer to the name. Use get_name() if you want the enum value.
    tt_string_view DeclName() const noexcept { return m_declaration->DeclName(); }

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
    tt_string m_value;
    tt_string m_constant;  // filled in by as_constant() if m_value is a friendly name
};
