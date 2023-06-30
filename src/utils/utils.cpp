/////////////////////////////////////////////////////////////////////////////
// Purpose:   Utility functions that work with properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <charconv>
#include <set>

#include <wx/gdicmn.h>   // Common GDI classes, types and declarations
#include <wx/mstream.h>  // Memory stream classes

#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "utils.h"         // Utility functions that work with properties

tt_string DoubleToStr(double val)
{
    tt_string result;

    std::array<char, 20> str;
    if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), val); ec == std::errc())
    {
        result.assign(str.data(), ptr);
    }
    return result;
}

tt_string ClearPropFlag(tt_string_view flag, tt_string_view currentValue)
{
    tt_string result;
    if (flag.empty() || currentValue.empty())
    {
        result = currentValue;
        return result;
    }

    tt_view_vector mstr(currentValue, '|');
    for (auto& iter: mstr)
    {
        if (iter != flag)
        {
            if (result.size())
                result << '|';
            result << iter;
        }
    }
    return result;
}

tt_string ClearMultiplePropFlags(tt_string_view flags, tt_string_view currentValue)
{
    tt_string result;
    if (flags.empty() || currentValue.empty())
    {
        result = currentValue;
        return result;
    }

    tt_string_vector mflags(flags, '|');

    tt_string_vector mstr(currentValue, '|');
    for (auto& iter: mstr)
    {
        bool isFlagged = false;
        for (auto& itFlags: mflags)
        {
            if (iter == itFlags)
            {
                isFlagged = true;
                break;
            }
        }

        if (!isFlagged)
        {
            if (result.size())
                result << '|';
            result << iter;
        }
    }
    return result;
}

tt_string SetPropFlag(tt_string_view flag, tt_string_view currentValue)
{
    tt_string result(currentValue);
    if (flag.empty())
    {
        return result;
    }

    tt_view_vector mstr(currentValue, '|');
    for (auto& iter: mstr)
    {
        if (iter.is_sameas(flag))
        {
            return result;  // flag has already been added
        }
    }
    if (result.size())
        result << '|';
    result << flag;
    return result;
}

bool isPropFlagSet(tt_string_view flag, tt_string_view currentValue)
{
    if (flag.empty() || currentValue.empty())
    {
        return false;
    }

    tt_view_vector mstr(currentValue, '|');
    for (auto& iter: mstr)
    {
        if (iter.is_sameas(flag))
        {
            return true;  // flag has already been added
        }
    }
    return false;
}

wxSystemColour ConvertToSystemColour(tt_string_view value)
{
    // clang-format off

    #define IS_SYSCOLOUR(name) if (value == #name) return name;
    #define ELSE_IS_SYSCOLOUR(name) else if (value == #name) return name;

    IS_SYSCOLOUR(wxSYS_COLOUR_SCROLLBAR)

    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_BACKGROUND)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_ACTIVECAPTION)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_INACTIVECAPTION)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_MENU)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_WINDOW)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_WINDOWFRAME)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_MENUTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_CAPTIONTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_ACTIVEBORDER)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_INACTIVEBORDER)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_APPWORKSPACE)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_HIGHLIGHT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_HIGHLIGHTTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_BTNFACE)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_BTNSHADOW)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_GRAYTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_BTNTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_INACTIVECAPTIONTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_BTNHIGHLIGHT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_3DDKSHADOW)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_3DLIGHT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_INFOTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_INFOBK)

    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_LISTBOX)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_HOTLIGHT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_GRADIENTACTIVECAPTION)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_GRADIENTINACTIVECAPTION)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_MENUHILIGHT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_MENUBAR)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_LISTBOXTEXT)
    ELSE_IS_SYSCOLOUR(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT)

    return wxSYS_COLOUR_BTNFACE;
    // clang-format on
}

wxColour ConvertToColour(tt_string_view value)
{
    // check for system colour
    if (value.starts_with("wx"))
    {
        return wxSystemSettings::GetColour(ConvertToSystemColour(value));
    }
    else
    {
        tt_view_vector mstr(value, ',');
        unsigned long rgb = 0;
        if (mstr.size() > 2)
        {
            auto blue = mstr[2].atoi();
            if (blue < 0 || blue > 255)
                blue = 0;
            rgb |= (blue << 16);
        }
        if (mstr.size() > 1)
        {
            auto green = mstr[1].atoi();
            if (green < 0 || green > 255)
                green = 0;
            rgb |= (green << 8);
        }
        if (mstr.size() > 0)
        {
            auto red = mstr[0].atoi();
            if (red < 0 || red > 255)
                red = 0;
            rgb |= red;
        }
        wxColour clr(rgb);
        return clr;
    }
}

const char* ConvertFontFamilyToString(wxFontFamily family)
{
    const char* result;

    switch (family)
    {
        case wxFONTFAMILY_DECORATIVE:
            result = "wxFONTFAMILY_DECORATIVE";
            break;
        case wxFONTFAMILY_ROMAN:
            result = "wxFONTFAMILY_ROMAN";
            break;
        case wxFONTFAMILY_SCRIPT:
            result = "wxFONTFAMILY_SCRIPT";
            break;
        case wxFONTFAMILY_SWISS:
            result = "wxFONTFAMILY_SWISS";
            break;
        case wxFONTFAMILY_MODERN:
            result = "wxFONTFAMILY_MODERN";
            break;
        case wxFONTFAMILY_TELETYPE:
            result = "wxFONTFAMILY_TELETYPE";
            break;
        default:
            result = "wxFONTFAMILY_DEFAULT";
            break;
    }

    return result;
}

tt_string ConvertEscapeSlashes(tt_string_view str)
{
    tt_string result;

    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        auto ch = str[pos];
        if (ch == '\\')
        {
            // REVIEW: [KeyWorks - 06-07-2020] Like the original wxString version, this will not save a backslash if it is at
            // the end of a line. Is that intentional or just a bug?
            if (pos < str.length() - 1)
            {
                auto nextChar = str[pos + 1];

                switch (nextChar)
                {
                    case 'n':
                        result += '\n';
                        ++pos;
                        break;

                    case 't':
                        result += '\t';
                        ++pos;
                        break;

                    case 'r':
                        result += '\r';
                        ++pos;
                        break;

                    case '\\':
                        result += '\\';
                        ++pos;
                        break;
                }
            }
        }
        else
            result += ch;
    }

    return result;
}

wxPoint DlgPoint(wxObject* parent, Node* node, GenEnum::PropName prop)
{
    if (node->as_string(prop).contains("d", tt::CASE::either))
    {
        return wxStaticCast(parent, wxWindow)->ConvertDialogToPixels(node->prop_as_wxPoint(prop));
    }
    else
    {
        return node->prop_as_wxPoint(prop);
    }
}

wxSize DlgSize(wxObject* parent, Node* node, GenEnum::PropName prop)
{
    if (node->as_string(prop).contains("d", tt::CASE::either))
    {
        return wxStaticCast(parent, wxWindow)->ConvertDialogToPixels(node->prop_as_wxSize(prop));
    }
    else
    {
        return node->prop_as_wxSize(prop);
    }
}

wxSize GetSizeInfo(tt_string_view description)
{
    wxSize size;

    tt_view_vector size_description;
    if (description.contains(";"))
        size_description.SetString(description, ';', tt::TRIM::left);
    else
        size_description.SetString(description, ',');

    ASSERT(size_description.size())
    ASSERT(size_description[0].size())

    if (size_description.size())
    {
        size_t start = size_description[0].front() == '[' ? 1 : 0;
        size.x = size_description[0].atoi(start);
        if (size_description.size() > 1)
            size.y = size_description[1].atoi();
        else
            size.y = 0;
    }
    else
    {
        size.x = 16;
        size.y = 16;
    }

    return size;
}

// Any mime type in the following list will NOT be converted to PNG.

// clang-format off
inline constexpr const char* lst_no_png_conversion[] = {

    "image/x-ani",
    "image/x-cur",
    "image/gif",
    "image/x-ico",
    "image/jpeg",

};
// clang-format on

bool isConvertibleMime(const tt_string& suffix)
{
    for (auto& iter: lst_no_png_conversion)
    {
        if (suffix.is_sameas(iter))
            return false;
    }
    return true;
}

extern const char* g_u8_cpp_keywords;  // defined in ../panels/base_panel.cpp
std::set<std::string> g_set_cpp_keywords;

bool isValidVarName(const std::string& str)
{
    // variable names must start with an alphabetic character or underscore character
    if (!((str[0] >= 'a' && str[0] <= 'z') || (str[0] >= 'A' && str[0] <= 'Z') || str[0] == '_'))
        return false;

    for (auto iter: str)
    {
        if (!((iter >= 'a' && iter <= 'z') || (iter >= 'A' && iter <= 'Z') || (iter >= '0' && iter <= '9') || iter == '_'))
            return false;
    }

    // Ensure that the variable name is not a C++ keyword

    // The set is only initialized the first time this function is called.
    if (g_set_cpp_keywords.empty())
    {
        tt_string_vector keywords(g_u8_cpp_keywords, ' ');
        for (auto& iter: keywords)
        {
            g_set_cpp_keywords.emplace(iter);
        }
    }

    if (g_set_cpp_keywords.contains(str))
        return false;

    return true;
}

tt_string CreateBaseFilename(Node* form_node, const tt_string& class_name)
{
    tt_string filename;
    if (class_name.size())
    {
        filename = class_name;
    }
    else
    {
        filename = form_node->value(prop_class_name);
    }

    if (filename.ends_with("Base"))
    {
        filename.erase(filename.size() - (sizeof("Base") - 1));
        filename += "_base";
    }

    filename.MakeLower();

    return filename;
}

tt_string CreateDerivedFilename(Node* form_node, const tt_string& class_name)
{
    tt_string filename;
    if (class_name.size())
    {
        filename = class_name;
    }
    else
    {
        filename = form_node->value(prop_derived_class_name);
    }

    if (filename.ends_with("Derived"))
    {
        filename.erase(filename.size() - (sizeof("Derived") - 1));
        filename += "_derived";
    }
    else if (!form_node->value(prop_base_file).ends_with("_base"))
    {
        filename += "_derived";
    }
    filename.MakeLower();

    return filename;
}
