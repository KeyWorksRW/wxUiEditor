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
