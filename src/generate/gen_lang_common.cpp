/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common mulit-language functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <ttmultistr_wx.h>  // ttMultiString -- Class for handling multiple strings

#include "gen_lang_common.h"

#include "code.h"
#include "gen_common.h"     // Common component functions
#include "lambdas.h"        // Functions for formatting and storage of lamda events
#include "node.h"           // Node class
#include "project_class.h"  // Project class
#include "utils.h"          // Utility functions that work with properties

const char* LangPtr(int language)
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            return "->";

        case GEN_LANG_PYTHON:
            return ".";

        default:
            FAIL_MSG("Unsupported language!")
            return "";
    }
}

ttlib::cstr GetWidgetName(int language, ttlib::sview name)
{
    ttlib::cstr widget_name;
    if (language == GEN_LANG_CPLUSPLUS)
    {
        widget_name = name;
        return widget_name;
    }

    ttlib::multistr multistr(name, "|", tt::TRIM::both);
    for (auto& iter: multistr)
    {
        if (iter.empty())
            continue;
        if (widget_name.size())
            widget_name << "|";
        if (iter == "wxEmptyString")
            widget_name = "\"\"";
        else
        {
            iter.erase(0, 2);
            widget_name << "wx." << iter;
        }
    }
    return widget_name;
}

ttlib::cstr GetPythonName(ttlib::sview name)
{
    ttlib::cstr widget_name;
    ttlib::multistr multistr(name, "|", tt::TRIM::both);
    for (auto& iter: multistr)
    {
        if (iter.empty())
            continue;
        if (widget_name.size())
            widget_name << "|";
        if (iter == "wxEmptyString")
            widget_name = "\"\"";
        else
        {
            iter.erase(0, 2);
            widget_name << "wx." << iter;
        }
    }
    return widget_name;
}

ttlib::cstr GeneratePythonQuotedString(const ttlib::cstr& str)
{
    ttlib::cstr code;

    if (str.size())
    {
        auto str_with_escapes = ConvertToCodeString(str);

        bool has_utf_char = false;
        for (auto iter: str_with_escapes)
        {
            if (iter < 0)
            {
                has_utf_char = true;
                break;
            }
        }

        if (has_utf_char)
        {
            // While this may not be necessary for non-Windows systems, it does ensure the code compiles on all platforms.
            if (GetProject()->prop_as_bool(prop_internationalize))
            {
                code << "_(wx.String.FromUTF8(\"" << str_with_escapes << "\"))";
            }
            else
            {
                code << "wx.String.FromUTF8(\"" << str_with_escapes << "\")";
            }
        }
        else
        {
            if (GetProject()->prop_as_bool(prop_internationalize))
                code << "_(\"" << str_with_escapes << "\")";
            else
                code << "\"" << str_with_escapes << "\"";
        }
    }
    else
    {
        code << "\"\"";
    }

    return code;
}

ttlib::cstr GenerateSizerFlags(int language, Node* node)
{
    if (language == GEN_LANG_CPLUSPLUS)
        return GenerateSizerFlags(node);

    ttlib::cstr code;

    // wxPython does not support wxSizerFlags

    if (language == GEN_LANG_PYTHON)
    {
        code << node->prop_as_string(prop_proportion) << ", ";
        ttlib::cstr flags;
        if (auto& prop = node->prop_as_string(prop_alignment); prop.size())
        {
            if (prop.contains("wxALIGN_CENTER"))
            {
                flags << "wx.ALIGN_CENTER";
            }

            if (prop.contains("wxALIGN_LEFT"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.ALIGN_LEFT";
            }
            else if (prop.contains("wxALIGN_RIGHT"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.ALIGN_RIGHT";
            }

            if (prop.contains("wxALIGN_TOP"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.ALIGN_TOP";
            }
            else if (prop.contains("wxALIGN_BOTTOM"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.ALIGN_BOTTOM";
            }

            if (prop.contains("wxEXPAND"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.EXPAND";
            }
            if (prop.contains("wxSHAPED"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.SHAPED";
            }
            if (prop.contains("wxFIXED_MINSIZE"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.FIXED_MINSIZE";
            }
            if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.RESERVE_SPACE_EVEN_IF_HIDDEN";
            }

            if (prop.contains("wxALL"))
            {
                if (flags.size())
                    flags << "|";
                flags << "wx.ALL";
            }
            else
            {
                if (prop.contains("wxLEFT"))
                {
                    if (flags.size())
                        flags << "|";
                    flags << "wx.LEFT";
                }
                if (prop.contains("wxRIGHT"))
                {
                    if (flags.size())
                        flags << "|";
                    flags << "wx.RIGHT";
                }
                if (prop.contains("wxTOP"))
                {
                    if (flags.size())
                        flags << "|";
                    flags << "wx.TOP";
                }
                if (prop.contains("wxBOTTOM"))
                {
                    if (flags.size())
                        flags << "|";
                    flags << "wx.BOTTOM";
                }
            }
        }
        if (flags.empty())
            flags << "0";
        code << flags << ", ";

        auto border_size = node->prop_as_string(prop_border_size);
        if (border_size.empty())
            border_size = "0";
        code << border_size;
    }
    else
    {
        code << (GetWidgetName(language, "wxSizerFlags"));

        if (auto& prop = node->prop_as_string(prop_proportion); prop != "0")
        {
            code << '(' << prop << ')';
        }
        else
        {
            code << "()";
        }

        if (auto& prop = node->prop_as_string(prop_alignment); prop.size())
        {
            if (prop.contains("wxALIGN_CENTER"))
            {
                // Note that CenterHorizontal() and CenterVertical() require wxWidgets 3.1 or higher. Their advantage is
                // generating an assert if you try to use one that is invalid if the sizer parent's orientation doesn't
                // support it. Center() just works without the assertion check.
                code << LangPtr(language) << "Center()";
            }

            if (prop.contains("wxALIGN_LEFT"))
            {
                code << LangPtr(language) << "Left()";
            }
            else if (prop.contains("wxALIGN_RIGHT"))
            {
                code << LangPtr(language) << "Right()";
            }

            if (prop.contains("wxALIGN_TOP"))
            {
                code << LangPtr(language) << "Top()";
            }
            else if (prop.contains("wxALIGN_BOTTOM"))
            {
                code << LangPtr(language) << "Bottom()";
            }
        }

        if (auto& prop = node->prop_as_string(prop_flags); prop.size())
        {
            if (prop.contains("wxEXPAND"))
            {
                code << LangPtr(language) << "Expand()";
            }
            if (prop.contains("wxSHAPED"))
            {
                code << LangPtr(language) << "Shaped()";
            }
            if (prop.contains("wxFIXED_MINSIZE"))
            {
                code << LangPtr(language) << "FixedMinSize()";
            }
            if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
            {
                code << LangPtr(language) << "ReserveSpaceEvenIfHidden()";
            }
        }

        if (auto& prop = node->prop_as_string(prop_borders); prop.size())
        {
            auto border_size = node->prop_as_string(prop_border_size);
            if (prop.contains("wxALL"))
            {
                if (border_size == "5")
                    code << LangPtr(language) << "Border(" << GetWidgetName(language, "wxALL") << ")";
                else if (border_size == "10")
                    code << LangPtr(language) << "DoubleBorder(" << GetWidgetName(language, "wxALL") << ")";
                else if (border_size == "15")
                    code << LangPtr(language) << "TripleBorder(" << GetWidgetName(language, "wxALL") << ")";
                else
                {
                    code << LangPtr(language) << "Border(" << GetWidgetName(language, "wxALL") << ", " << border_size << ')';
                }
            }
            else
            {
                code << LangPtr(language) << "Border(";
                ttlib::cstr border_flags;

                if (prop.contains("wxLEFT"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << GetWidgetName(language, "wxLEFT");
                }
                if (prop.contains("wxRIGHT"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << GetWidgetName(language, "wxRIGHT");
                }
                if (prop.contains("wxTOP"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << GetWidgetName(language, "wxTOP");
                }
                if (prop.contains("wxBOTTOM"))
                {
                    if (border_flags.size())
                        border_flags << '|';
                    border_flags << GetWidgetName(language, "wxBOTTOM");
                }
                if (border_flags.empty())
                    border_flags = "0";

                code << border_flags << ", ";
                if (border_size == "5")
                {
                    code << GetWidgetName(language, "wxSizerFlags") << LangPtr(language) << "GetDefaultBorder())";
                }
                else
                {
                    code << border_size << ')';
                }
            }
        }
    }

    return code;
}
