/////////////////////////////////////////////////////////////////////////////
// Purpose:   Utility functions that work with properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <charconv>

#include <wx/gdicmn.h>   // Common GDI classes, types and declarations
#include <wx/mstream.h>  // Memory stream classes

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "utils.h"         // Utility functions that work with properties

ttlib::cstr DoubleToStr(double val)
{
    ttlib::cstr result;

    std::array<char, 20> str;
    if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), val); ec == std::errc())
    {
        result.assign(str.data(), ptr);
    }
    return result;
}

ttlib::cstr ClearPropFlag(ttlib::cview flag, ttlib::cview currentValue)
{
    ttlib::cstr result;
    if (flag.empty() || currentValue.empty())
    {
        result = currentValue;
        return result;
    }

    ttlib::multiview mstr(currentValue, '|');
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

ttlib::cstr ClearMultiplePropFlags(ttlib::cview flags, ttlib::cview currentValue)
{
    ttlib::cstr result;
    if (flags.empty() || currentValue.empty())
    {
        result = currentValue;
        return result;
    }

    ttlib::multistr mflags(flags, '|');

    ttlib::multistr mstr(currentValue, '|');
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

ttlib::cstr SetPropFlag(ttlib::cview flag, ttlib::cview currentValue)
{
    ttlib::cstr result(currentValue);
    if (flag.empty())
    {
        return result;
    }

    ttlib::multiview mstr(currentValue, '|');
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

bool isPropFlagSet(ttlib::cview flag, ttlib::cview currentValue)
{
    if (flag.empty() || currentValue.empty())
    {
        return false;
    }

    ttlib::multiview mstr(currentValue, '|');
    for (auto& iter: mstr)
    {
        if (iter.is_sameas(flag))
        {
            return true;  // flag has already been added
        }
    }
    return false;
}

int ConvertBitlistToInt(ttlib::cview list)
{
    int result = 0;
    if (list.size())
    {
        ttlib::multistr mstr(list, '|');
        for (auto& iter: mstr)
        {
            result |= g_NodeCreator.GetConstantAsInt(iter);
        }
    }
    return result;
}

ttlib::cstr ConvertColourToString(const wxColour& colour)
{
    ttlib::cstr str;
    str << colour.Red() << ',' << colour.Green() << ',' << colour.Blue();
    return str;
}

ttlib::cstr ConvertSystemColourToString(long colour)
{
    ttlib::cstr str;

#define SystemColourConvertCase(name) \
    case name:                        \
        str = (#name);                \
        break;

    // clang-format off
    switch (colour)
    {
        SystemColourConvertCase(wxSYS_COLOUR_SCROLLBAR)
        SystemColourConvertCase(wxSYS_COLOUR_BACKGROUND)
        SystemColourConvertCase(wxSYS_COLOUR_ACTIVECAPTION)
        SystemColourConvertCase(wxSYS_COLOUR_INACTIVECAPTION)
        SystemColourConvertCase(wxSYS_COLOUR_MENU)
        SystemColourConvertCase(wxSYS_COLOUR_WINDOW)
        SystemColourConvertCase(wxSYS_COLOUR_WINDOWFRAME)
        SystemColourConvertCase(wxSYS_COLOUR_MENUTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_WINDOWTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_CAPTIONTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_ACTIVEBORDER)
        SystemColourConvertCase(wxSYS_COLOUR_INACTIVEBORDER)
        SystemColourConvertCase(wxSYS_COLOUR_APPWORKSPACE)
        SystemColourConvertCase(wxSYS_COLOUR_HIGHLIGHT)
        SystemColourConvertCase(wxSYS_COLOUR_HIGHLIGHTTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_BTNFACE)
        SystemColourConvertCase(wxSYS_COLOUR_BTNSHADOW)
        SystemColourConvertCase(wxSYS_COLOUR_GRAYTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_BTNTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_INACTIVECAPTIONTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_BTNHIGHLIGHT)
        SystemColourConvertCase(wxSYS_COLOUR_3DDKSHADOW)
        SystemColourConvertCase(wxSYS_COLOUR_3DLIGHT)
        SystemColourConvertCase(wxSYS_COLOUR_INFOTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_INFOBK)

        SystemColourConvertCase(wxSYS_COLOUR_LISTBOX)
        SystemColourConvertCase(wxSYS_COLOUR_HOTLIGHT)
        SystemColourConvertCase(wxSYS_COLOUR_GRADIENTACTIVECAPTION)
        SystemColourConvertCase(wxSYS_COLOUR_GRADIENTINACTIVECAPTION)
        SystemColourConvertCase(wxSYS_COLOUR_MENUHILIGHT)
        SystemColourConvertCase(wxSYS_COLOUR_MENUBAR)
        SystemColourConvertCase(wxSYS_COLOUR_LISTBOXTEXT)
        SystemColourConvertCase(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT)

    }
    // clang-format on

    return str;
}

wxSystemColour ConvertToSystemColour(ttlib::cview value)
{
    // clang-format off
    #define IS_SYSCOLOUR(name) if (value.is_sameas(#name)) return name;
    #define ELSE_IS_SYSCOLOUR(name) else if (value.is_sameas(#name)) return name;

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

wxColour ConvertToColour(ttlib::cview value)
{
    // check for system colour
    if (ttlib::is_sameprefix(value, "wx"))
    {
        return wxSystemSettings::GetColour(ConvertToSystemColour(value));
    }
    else
    {
        ttlib::multiview mstr(value, ',');
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

ttlib::cstr ConvertEscapeSlashes(ttlib::cview str)
{
    ttlib::cstr result;

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

ttlib::cstr CreateEscapedText(ttlib::cview str)
{
    ttlib::cstr result;

    for (auto ch: str)
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

std::vector<ttlib::cstr> ConvertToArrayString(ttlib::cview value)
{
    std::vector<ttlib::cstr> array;
    if (value.empty())
        return array;
    ttlib::cstr parse;
    auto pos = parse.ExtractSubString(value);
    array.emplace_back(parse);

    for (auto tmp_value = ttlib::stepover(value.data() + pos); tmp_value.size();
         tmp_value = ttlib::stepover(tmp_value.data() + pos))
    {
        pos = parse.ExtractSubString(tmp_value);
        array.emplace_back(parse);
    }

    return array;
}

wxArrayString ConvertToWxArrayString(ttlib::cview value)
{
    wxArrayString array;
    if (value.empty())
        return array;
    ttlib::cstr parse;
    auto pos = parse.ExtractSubString(value);
    array.push_back(parse.wx_str());

    for (auto tmp_value = ttlib::stepover(value.data() + pos); tmp_value.size();
         tmp_value = ttlib::stepover(tmp_value.data() + pos))
    {
        pos = parse.ExtractSubString(tmp_value);
        array.push_back(parse.wx_str());
    }

    return array;
}

wxPoint DlgPoint(wxObject* parent, Node* node, GenEnum::PropName prop)
{
    if (node->prop_as_string(prop).contains("d", tt::CASE::either))
    {
        return wxStaticCast(parent, wxWindow)->ConvertPixelsToDialog(node->prop_as_wxPoint(prop));
    }
    else
    {
        return node->prop_as_wxPoint(prop);
    }
}

wxSize DlgSize(wxObject* parent, Node* node, GenEnum::PropName prop)
{
    if (node->prop_as_string(prop).contains("d", tt::CASE::either))
    {
        return wxStaticCast(parent, wxWindow)->ConvertPixelsToDialog(node->prop_as_wxSize(prop));
    }
    else
    {
        return node->prop_as_wxSize(prop);
    }
}

void GetScaleInfo(wxSize& size, ttlib::sview description)
{
    ttlib::multiview scale;
    if (description.contains(";"))
        scale.SetString(description, ';', tt::TRIM::left);
    else
        scale.SetString(description, ',');

    size_t start = scale[0].front() == '[' ? 1 : 0;
    size.x = scale[0].atoi(start);
    if (scale.size() > 1)
        size.y = scale[1].atoi();
}
