/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements NodeProperty, storing user-configured property values for Node
// instances. Each property holds m_declaration (PropDeclaration* for metadata), m_node (parent
// Node*), m_value (tt_string storage), and m_constant (cached friendly-name-to-wx-constant
// conversion). The class provides type-safe setters (set_value overloads for
// int/wxColour/wxPoint/etc.) and getters (as_int/as_bool/as_color/as_font/etc.) with lazy parsing.
// Complex properties use custom structs (NODEPROP_STATUSBAR_FIELD, NODEPROP_CHECKLIST_ITEM, etc.)
// with serialization methods (convert_statusbar_fields, as_checklist_items) handling pipe-separated
// storage format. The as_constant method converts friendly names ("Default") to wxWidgets constants
// ("wxID_ANY"), caching in m_constant. HasValue checks non-default/non-empty states. Property
// access delegates to m_declaration for type info (get_name, get_type, isProp) enabling generic
// property grid and code generation logic.

#pragma once

#include "font_prop.h"  // FontProperty class
#include "prop_decl.h"  // PropDeclaration -- PropChildDeclaration and PropDeclaration classes

class wxAnimation;
class Node;

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
    // assumes embedded, svg only if .svg file extension, and svg will default to 16x16
    tt_string bitmap;
};

class NodeProperty
{
public:
    NodeProperty(PropDeclaration* declaration, Node* node);

    auto set_value(int integer) -> void { m_value = tt::itoa(integer); };
    auto set_value(double val) -> void;
    auto set_value(const wxColour& colour) -> void;
    auto set_value(const wxString& str) -> void;
    auto set_value(const wxPoint& point) -> void;
    auto set_value(const wxSize& size) -> void;
    auto set_value(const char* val) -> void { m_value.assign(val); }
    auto set_value(std::string_view val) -> void { m_value.assign(val); }
    auto set_value(const tt_string& val) -> void { m_value.assign(val); }
    auto set_value(const std::string& val) -> void { m_value.assign(val); }

    tt_string convert_statusbar_fields(std::vector<NODEPROP_STATUSBAR_FIELD>& fields) const;
    tt_string convert_checklist_items(std::vector<NODEPROP_CHECKLIST_ITEM>& fields) const;
    tt_string convert_radiobox_items(std::vector<NODEPROP_RADIOBOX_ITEM>& fields) const;
    tt_string convert_bmp_combo_items(std::vector<NODEPROP_BMP_COMBO_ITEM>& fields) const;

    auto set_value(std::vector<NODEPROP_STATUSBAR_FIELD>& fields) -> void
    {
        m_value = convert_statusbar_fields(fields);
    }
    auto set_value(std::vector<NODEPROP_CHECKLIST_ITEM>& items) -> void
    {
        m_value = convert_checklist_items(items);
    }
    auto set_value(std::vector<NODEPROP_RADIOBOX_ITEM>& items) -> void
    {
        m_value = convert_radiobox_items(items);
    }
    auto set_value(std::vector<NODEPROP_BMP_COMBO_ITEM>& items) -> void
    {
        m_value = convert_bmp_combo_items(items);
    }

    // By returnung a vector instead of the string, the way the property string gets formatted is
    // entirely up to NodeProperty. This allows, for example, to use '|' as a separator instead of
    // ',' when there is text which contains a comma.

    std::vector<NODEPROP_STATUSBAR_FIELD> as_statusbar_fields() const;
    std::vector<NODEPROP_CHECKLIST_ITEM> as_checklist_items() const;
    std::vector<NODEPROP_RADIOBOX_ITEM> as_radiobox_items() const;
    std::vector<NODEPROP_BMP_COMBO_ITEM> as_bmp_combo_items() const;

    // Returns a non-const reference allowing you to modify the value. Do *NOT* use this for
    // the vector functions, as the formatting of the property string is entirely up to
    // NodeProperty.
    tt_string& get_value() { return m_value; }

    // Returns string containing the property ID without any assignment if it is a custom id.
    tt_string get_PropId() const;

    // Returns a string containing the ID without any assignment if it is a custom id.
    static tt_string get_PropId(const tt_string& complete_id);

    const tt_string& value() const { return m_value; }

    [[nodiscard]] auto as_int() const -> int;
    [[nodiscard]] auto as_bool() const -> bool { return (as_int() != 0); };
    [[nodiscard]] auto as_float() const -> double;

    // Looks up wx constant, returns it's numerical value.
    //
    // Returns wxID_ANY if constant is not found
    [[nodiscard]] auto as_id() const -> int;

    // tt_string_view as_sview() const { return m_value; }

    // Use with caution! This allows you to modify the property string directly.
    auto as_raw_ptr() { return &m_value; }

    auto as_animation(wxAnimation* p_animation) const -> void;
    [[nodiscard]] auto as_bitmap() const -> wxBitmap;
    [[nodiscard]] auto as_color() const -> wxColour;
    [[nodiscard]] auto as_font() const -> wxFont;
    [[nodiscard]] auto as_font_prop() const -> FontProperty;
    [[nodiscard]] auto as_point() const -> wxPoint;
    [[nodiscard]] auto as_size() const -> wxSize;
    [[nodiscard]] auto as_wxArrayString() const -> wxArrayString;

    // Unless separator is set, assumes all values are within quotes,
    // or separated by semi-colons.
    //
    // Specify separator to use a specific character as the separator
    [[nodiscard]] auto as_ArrayString(char separator = 0) const -> std::vector<tt_string>;

    // On Windows this will first convert to UTF-16 unless wxUSE_UNICODE_UTF8 is set.
    [[nodiscard]] auto as_wxString() const -> wxString { return m_value.make_wxString(); }

    [[nodiscard]] auto as_bitmap_bundle() const -> wxBitmapBundle;

    [[nodiscard]] auto as_string() const -> const tt_string& { return m_value; }
    [[nodiscard]] auto as_view() const -> std::string_view { return m_value; }

    // Converts friendly name to wxWidgets constant
    auto as_constant(std::string_view prefix) -> const tt_string&;

    // Converts friendly name to wxWidgets constant, and then returns the integer value of
    // that constant.
    [[nodiscard]] auto as_mockup(std::string_view prefix) const -> int;

    [[nodiscard]] auto as_vector() const -> std::vector<tt_string>;

    // This first doubles the backslash in escaped characters (\\n, \\t, \\r, and \\)
    [[nodiscard]] auto as_escape_text() const -> tt_string;

    operator bool() const { return (as_int() != 0); }
    operator int() const { return as_int(); }
    operator const char*() const { return m_value.c_str(); }

    operator wxBitmap() const { return as_bitmap(); }
    operator wxColour() const { return as_color(); }
    operator wxFont() const { return as_font(); }
    operator wxPoint() const { return as_point(); }
    operator wxSize() const { return as_size(); }

    [[nodiscard]] auto isDefaultValue() const -> bool;
    [[nodiscard]] auto getDefaultValue() const noexcept -> std::string_view
    {
        return m_declaration->getDefaultValue();
    }

    // Returns false if the property is empty. For size and point properties, returns false
    // if the default value is used. For image properties, returns true if a filename is
    // specified. For window style, returns true if at least one option is specified.
    [[nodiscard]] auto HasValue() const -> bool;

    [[nodiscard]] auto get_PropDeclaration() const -> const PropDeclaration*
    {
        return m_declaration;
    }

    auto getNode() -> Node* { return m_node; }

    // Returns a char pointer to the name. Use get_name() if you want the enum value.
    [[nodiscard]] auto get_DeclName() const noexcept -> std::string_view
    {
        return m_declaration->get_DeclName();
    }

    [[nodiscard]] auto isProp(PropName name) const noexcept -> bool
    {
        return m_declaration->isProp(name);
    }
    [[nodiscard]] auto is_Type(PropType type) const noexcept -> bool
    {
        return m_declaration->is_Type(type);
    }

    [[nodiscard]] auto type() const noexcept -> PropType { return m_declaration->get_type(); }
    [[nodiscard]] auto get_name() const noexcept -> PropName { return m_declaration->get_name(); }

    auto get_PropDeclaration() -> PropDeclaration* { return m_declaration; }

    // Currently only called in debug builds, but available for release builds should we need it
    [[nodiscard]] auto get_PropSize() const -> size_t
    {
        return sizeof(*this) + (m_value.size() + 1);
    }

private:
    PropDeclaration* m_declaration;
    Node* m_node;  // node this property is a child of
    tt_string m_value;
    tt_string m_constant;  // filled in by as_constant() if m_value is a friendly name
};
