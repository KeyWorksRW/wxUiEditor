/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <charconv>
#include <cstdlib>

#include <wx/animate.h>                // wxAnimation and wxAnimationCtrl
#include <wx/propgrid/propgriddefs.h>  // wxPropertyGrid miscellaneous definitions

#include "node_prop.h"

#include "font_prop.h"                          // FontProperty -- FontProperty class
#include "image_handler.h"                      // ImageHandler class
#include "mainapp.h"                            // App -- Main application class
#include "node.h"                               // Node -- Node class
#include "node_creator.h"                       // NodeCreator class
#include "project_handler.h"                    // ProjectHandler singleton class
#include "utils.h"                              // Utility functions that work with properties
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector
#include "wxue_namespace/wxue_view_vector.h"    // wxue::ViewVector

using namespace GenEnum;

NodeProperty::NodeProperty(PropDeclaration* info, Node* node) : m_declaration(info), m_node(node) {}

// The advantage of placing the one-line calls to PropDeclaration (m_declaration) here is that it
// reduces the header-file dependency for other modules that need NodeProperty, and it allows for
// changes to PropDeclaration that don't require recompiling every module that included prop_decl.h.

auto NodeProperty::isDefaultValue() const -> bool
{
    return m_value.is_sameas(m_declaration->getDefaultValue());
}

auto NodeProperty::as_int() const -> int
{
    switch (type())
    {
        case type_editoption:
        case type_option:
        case type_id:
            return NodeCreation.get_ConstantAsInt(m_value, 0);

        case type_bitlist:
            {
                int result = 0;
                wxue::StringVector mstr(m_value, '|', wxue::TRIM::both);
                for (const auto& iter: mstr)
                {
                    result |= NodeCreation.get_ConstantAsInt(iter);
                }
                return result;
            }

        default:
            return m_value.atoi();  // this will return 0 if the m_value is an empty string
    }
}

auto NodeProperty::as_id() const -> int
{
    return NodeCreation.get_ConstantAsInt(m_value, wxID_ANY);
}

// Static class function
auto NodeProperty::get_PropId(const wxue::string& complete_id) -> wxue::string
{
    wxue::string id;
    if (auto pos = complete_id.find('='); pos != wxue::npos)
    {
        while (pos > 0 && wxue::is_whitespace(complete_id[pos - 1]))
        {
            --pos;
        }
        id = complete_id.substr(0, pos);
    }
    else
    {
        id = complete_id;
    }
    return id;
}

auto NodeProperty::get_PropId() const -> wxue::string
{
    return get_PropId(m_value);
}

auto NodeProperty::as_mockup(std::string_view prefix) const -> int
{
    switch (type())
    {
        case type_editoption:
        case type_option:
        case type_id:
            if (m_value.starts_with("wx"))
            {
                return NodeCreation.get_ConstantAsInt(m_value, 0);
            }
            if (prefix.size())
            {
                wxue::string name;
                name << prefix << m_value;
                if (auto result = g_friend_constant.find(name); result != g_friend_constant.end())
                {
                    return NodeCreation.get_ConstantAsInt(result->second, 0);
                }
            }
            if (auto result = g_friend_constant.find(m_value); result != g_friend_constant.end())
            {
                return NodeCreation.get_ConstantAsInt(result->second, 0);
            }
            return 0;

        case type_bitlist:
            {
                wxue::StringVector mstr(m_value, '|', wxue::TRIM::both);
                int value = 0;
                for (auto& iter: mstr)
                {
                    if (iter.starts_with("wx"))
                    {
                        value |= NodeCreation.get_ConstantAsInt(iter);
                    }
                    else
                    {
                        if (prefix.size())
                        {
                            iter.insert(0, prefix);
                        }
                        if (auto result = g_friend_constant.find(iter);
                            result != g_friend_constant.end())
                        {
                            value |= NodeCreation.get_ConstantAsInt(result->second);
                        }
                    }
                }
                return value;
            }

        default:
            return m_value.atoi();  // this will return 0 if the m_value is an empty string
    }
}

auto NodeProperty::as_constant(std::string_view prefix) -> const wxue::string&
{
    switch (type())
    {
        case type_editoption:
        case type_option:
        case type_id:
            if (m_value.starts_with("wx"))
            {
                return m_value;
            }
            if (prefix.size())
            {
                m_constant.clear();
                m_constant << prefix << m_value;
                if (auto result = g_friend_constant.find(m_constant);
                    result != g_friend_constant.end())
                {
                    m_constant = result->second;
                }
                else
                {
                    m_constant.clear();
                }
            }
            else
            {
                if (auto result = g_friend_constant.find(m_value);
                    result != g_friend_constant.end())
                {
                    m_constant = result->second;
                }
                else
                {
                    m_constant.clear();
                }
            }
            return m_constant;

        case type_bitlist:
            {
                wxue::StringVector mstr(m_value, '|', wxue::TRIM::both);
                m_constant.clear();
                for (auto& iter: mstr)
                {
                    if (iter.starts_with("wx"))
                    {
                        if (m_constant.size())
                        {
                            m_constant << '|';
                        }
                        m_constant << iter;
                    }
                    else
                    {
                        if (prefix.size())
                        {
                            iter.insert(0, prefix);
                        }
                        if (auto result = g_friend_constant.find(iter);
                            result != g_friend_constant.end())
                        {
                            if (m_constant.size())
                            {
                                m_constant << " | ";
                            }
                            m_constant << result->second;
                        }
                    }
                }
                return m_constant;
            }

        default:
            return m_value;  // this will return 0 if the m_value is an empty string
    }
}

auto NodeProperty::as_point() const -> wxPoint
{
    wxPoint result { -1, -1 };
    if (m_value.size())
    {
        wxue::ViewVector tokens(m_value, ',');
        if (tokens.size())
        {
            if (tokens[0].size())
                result.x = tokens[0].atoi();

            if (tokens.size() > 1 && tokens[1].size())
                result.y = tokens[1].atoi();
        }
    }
    return result;
}

auto NodeProperty::as_size() const -> wxSize
{
    wxSize result { -1, -1 };
    if (m_value.size())
    {
        wxue::ViewVector tokens(m_value, ',');
        if (tokens.size())
        {
            if (tokens[0].size())
            {
                result.x = tokens[0].atoi();
            }

            if (tokens.size() > 1 && tokens[1].size())
            {
                result.y = tokens[1].atoi();
            }
        }
    }
    return result;
}

// Defined in ../custom_ctrls/kw_color_picker.cpp
extern const std::map<std::string, std::string, std::less<>> kw_css_colors;

// Note that this is not only used to handle older wxUiEditor projects, but also some of the
// imported projects such as wxFormBuilder.
auto NodeProperty::as_color() const -> wxColour
{
    if (m_value.empty())
    {
        return wxNullColour;
    }
    // check for system colour
    if (m_value.starts_with("wx"))
    {
        return wxSystemSettings::GetColour(ConvertToSystemColour(m_value));
    }
    if (m_value.starts_with('#') || m_value.starts_with("RGB") || m_value.starts_with("rgb"))
    {
        return wxColour(m_value);
    }
    if (wxue::is_alpha(m_value[0]))
    {
        if (auto result = kw_css_colors.find(m_value); result != kw_css_colors.end())
        {
            return wxColour(result->second);
        }
        MSG_ERROR(wxue::string("Unknown CSS color: ") << m_value);
        return wxNullColour;
    }
    wxue::ViewVector mstr(m_value, ',');
    unsigned long rgb = 0;
    size_t shift_value = 0;
    for (const auto& color: mstr)
    {
        auto value = color.atoi();
        if (value < 0 || value > 255)  // ensure value is in range
        {
            value = 0;
        }
        rgb |= (value << shift_value);
        shift_value += 8;
        if (shift_value > 24)  // limited to 4 values (RGBA)
        {
            break;
        }
    }

    return wxColour(rgb);
}

auto NodeProperty::as_font() const -> wxFont
{
    return FontProperty(m_value.subview()).GetFont();
}

auto NodeProperty::as_font_prop() const -> FontProperty
{
    FontProperty font_prop(m_value.subview());
    return font_prop;
}

auto NodeProperty::as_bitmap() const -> wxBitmap
{
    auto image = ProjectImages.GetImage(m_value);
    if (!image.IsOk())
    {
        image = ProjectImages.GetImage(m_value);
        if (!image.IsOk())
        {
            return wxNullBitmap;
        }
    }

    return image;
}

auto NodeProperty::as_bitmap_bundle() const -> wxBitmapBundle
{
    auto bundle = ProjectImages.GetBitmapBundle(m_value);
    if (!bundle.IsOk())
    {
        return wxNullBitmap;
    }
    return bundle;
}

auto NodeProperty::as_animation(wxAnimation* p_animate) const -> void
{
    ProjectImages.GetPropertyAnimation(m_value, p_animate);
}

auto NodeProperty::as_escape_text() const -> wxue::string
{
    wxue::string result;

    for (auto ch: m_value)
    {
        switch (ch)
        {
            case '\n':
                result += "\\n";
                break;

            case '\t':
                result += "\\t";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\\':
                result += "\\\\";
                break;

            default:
                result += ch;
                break;
        }
    }

    return result;
}

auto NodeProperty::as_vector() const -> std::vector<wxue::string>
{
    std::vector<wxue::string> array;
    if (m_value.empty())
    {
        return array;
    }
    wxue::string parse;
    std::string_view value = m_value;
    auto pos = parse.ExtractSubString(value);
    array.emplace_back(parse);

    for (value = wxue::stepover(std::string_view(value.data() + pos)); value.size();
         value = wxue::stepover(std::string_view(value.data() + pos)))
    {
        pos = parse.ExtractSubString(value);
        array.emplace_back(parse);
    }

    return array;
}

auto NodeProperty::as_ArrayString(char separator) const -> std::vector<wxue::string>
{
    if (separator == 0)
    {
        wxue::StringVector result;

        if (m_value.size())
        {
            if (type() == type_stringlist_semi)
            {
                result.SetString(std::string_view(m_value), ";", wxue::TRIM::both);
            }
            else if (type() == type_stringlist_escapes || m_value[0] == '"')
            {
                auto view = m_value.view_substr(0, '"', '"');
                while (view.size() > 0)
                {
                    result.emplace_back(view);
                    view = wxue::stepover(std::string_view(view.data() + view.size()));
                    view = view.view_substr(0, '"', '"');
                }
            }
        }

        return result;
    }
    else
    {
        wxue::StringVector result(std::string_view(m_value), separator, wxue::TRIM::both);
        return result;
    }
}

auto NodeProperty::as_wxArrayString() const -> wxArrayString
{
    wxArrayString result;

    if (m_value.size())
    {
        if (m_value[0] == '"' &&
            !(type() == type_stringlist_semi && Project.get_OriginalProjectVersion() >= 18))
        {
            wxue::string_view view = m_value.view_substr(0, '"', '"');
            while (view.size() > 0)
            {
                result.Add(view.wx());
                view = wxue::stepover(std::string_view(view.data() + view.size()));
                view = wxue::string_view(view).view_substr(0, '"', '"');
            }
        }
        else
        {
            wxue::ViewVector array;
            array.SetString(std::string_view(m_value), ";", wxue::TRIM::both);
            for (const auto& str: array)
            {
                result.Add(str.wx());
            }
        }
    }

    return result;
}

auto NodeProperty::as_float() const -> double
{
    return std::atof(m_value.c_str());
}

auto NodeProperty::set_value(double value) -> void
{
    std::array<char, 20> str;
    if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), value);
        ec == std::errc())
    {
        m_value.assign(str.data(), ptr);
    }
    else
    {
        m_value.clear();
    }
}

auto NodeProperty::set_value(const wxColour& value) -> void
{
    m_value.clear();
    m_value << value.Red() << ',' << value.Green() << ',' << value.Blue();
}

auto NodeProperty::set_value(const wxPoint& value) -> void
{
    m_value.clear();
    m_value << value.x << ',' << value.y;
}

auto NodeProperty::set_value(const wxSize& value) -> void
{
    m_value.clear();
    m_value << value.x << ',' << value.y;
}

auto NodeProperty::set_value(const wxString& value) -> void
{
    m_value.clear();
    m_value << value.utf8_string();
}

// All but one of the std::vector properties contain text which could have commas in it, so we need
// to use a '|' character as the separator.

auto NodeProperty::convert_statusbar_fields(std::vector<NODEPROP_STATUSBAR_FIELD>& fields) const
    -> wxue::string
{
    wxue::string result;
    for (const auto& field: fields)
    {
        if (result.size())
            result << ';';
        result << field.style << '|' << field.width;
    }
    return result;
}

auto NodeProperty::convert_checklist_items(std::vector<NODEPROP_CHECKLIST_ITEM>& items) const
    -> wxue::string
{
    wxue::string result;
    for (auto& item: items)
    {
        if (result.size())
            result << ';';
        result << item.label;
        if (item.checked.size() || item.checked.atoi() != 0)
            result << '|' << item.checked;
    }
    return result;
}

auto NodeProperty::convert_radiobox_items(std::vector<NODEPROP_RADIOBOX_ITEM>& items) const
    -> wxue::string
{
    wxue::string result;
    for (auto& item: items)
    {
        if (result.size())
            result << ';';
        result << item.label;
        if (item.enabled.atoi() != 1 || item.show.atoi() != 1 || item.tooltip.size() ||
            item.helptext.size())
            result << '|' << item.enabled << '|' << item.show << '|' << item.tooltip << '|'
                   << item.helptext;
    }
    return result;
}

auto NodeProperty::convert_bmp_combo_items(std::vector<NODEPROP_BMP_COMBO_ITEM>& items) const
    -> wxue::string
{
    wxue::string result;
    for (auto& item: items)
    {
        if (result.size())
            result << ';';
        result << item.label;
        if (item.bitmap.size())
            result << '|' << item.bitmap;
    }
    return result;
}

std::vector<NODEPROP_STATUSBAR_FIELD> NodeProperty::as_statusbar_fields() const
{
    std::vector<NODEPROP_STATUSBAR_FIELD> result;

    // "1" is the default value, indicating nothing has been set.
    if (m_value == "1")
    {
        NODEPROP_STATUSBAR_FIELD field;
        field.style = "wxSB_NORMAL";
        field.width = "-1";
        result.emplace_back(field);
        return result;
    }

    wxue::StringVector fields(m_value, ';');
    for (auto& field: fields)
    {
        wxue::StringVector parts(field, '|');
        auto& item = result.emplace_back();
        if (parts.size() == 2)
        {
            item.style = parts[0];
            item.width = parts[1];
        }
        else if (parts.size() > 0)
        {
            item.style = parts[0];
            item.width = "-1";
        }
        else
        {
            item.style = "wxSB_NORMAL";
            item.width = "-1";
        }
    }

    return result;
}

std::vector<NODEPROP_CHECKLIST_ITEM> NodeProperty::as_checklist_items() const
{
    std::vector<NODEPROP_CHECKLIST_ITEM> result;

    if (m_value.size() && m_value[0] == '"' && wxGetApp().get_ProjectVersion() <= minRequiredVer)
    {
        auto array = as_ArrayString();
        for (const auto& iter: array)
        {
            NODEPROP_CHECKLIST_ITEM item;
            item.label = iter;
            result.emplace_back(item);
        }
        return result;
    }

    wxue::StringVector fields(m_value, ';');
    for (auto& field: fields)
    {
        NODEPROP_CHECKLIST_ITEM item;
        wxue::StringVector parts(field, '|');
        if (parts.size())
        {
            item.label = parts[0];
            if (parts.size() > 1)
            {
                item.checked = parts[1];
            }
        }
        result.emplace_back(item);
    }

    return result;
}

std::vector<NODEPROP_BMP_COMBO_ITEM> NodeProperty::as_bmp_combo_items() const
{
    std::vector<NODEPROP_BMP_COMBO_ITEM> result;

    wxue::StringVector fields(m_value, ';');
    for (auto& field: fields)
    {
        NODEPROP_BMP_COMBO_ITEM item;
        wxue::StringVector parts(field, '|');
        if (parts.size())
        {
            item.label = parts[0];
            if (parts.size() > 1)
            {
                item.bitmap = parts[1];
            }
        }
        result.emplace_back(item);
    }

    return result;
}

std::vector<NODEPROP_RADIOBOX_ITEM> NodeProperty::as_radiobox_items() const
{
    std::vector<NODEPROP_RADIOBOX_ITEM> result;

    wxue::StringVector fields(m_value, ';');
    for (auto& field: fields)
    {
        wxue::StringVector parts(field, '|');
        NODEPROP_RADIOBOX_ITEM item;
        if (parts.size() > 0)
            item.label = parts[0];
        if (parts.size() > 1)
            item.enabled = parts[1];
        if (parts.size() > 2)
            item.show = parts[2];
        if (parts.size() > 3)
            item.tooltip = parts[3];
        if (parts.size() > 4)
            item.helptext = parts[4];
        result.emplace_back(item);
    }

    return result;
}

auto NodeProperty::HasValue() const -> bool
{
    if (m_value.empty())
    {
        return false;
    }

    switch (type())
    {
        case type_wxSize:
            return (as_size() != wxDefaultSize);

        case type_wxPoint:
            return (as_point() != wxDefaultPosition);

        case type_animation:
            if (auto semicolonIndex = m_value.find_first_of(";"); wxue::is_found(semicolonIndex))
            {
                return (semicolonIndex != 0);
            }
            return m_value.size();

        case type_image:
            if (auto semicolonIndex = m_value.find_first_of(";"); wxue::is_found(semicolonIndex))
            {
                return (semicolonIndex != 0 && semicolonIndex + 2 < m_value.size());
            }
            return m_value.size();

        case type_bitlist:
            if (isProp(prop_window_style))
            {
                return (as_int() != 0);
            }
            return true;

        case type_statbar_fields:
            return (m_value == "1" || m_value == "wxSB_NORMAL|-1") ? false : true;

        default:
            return true;
    }
}
