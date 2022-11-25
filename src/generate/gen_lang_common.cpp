/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common mulit-language functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_lang_common.h"

#include "gen_common.h"     // Common component functions
#include "node.h"           // Node class
#include "project_class.h"  // Project class

const char* LangPtr(int language)
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
        case GEN_LANG_PHP:
            return "->";

        case GEN_LANG_PYTHON:
            return ".";

        case GEN_LANG_LUA:
            return ":";

        default:
            FAIL_MSG("Unsupported language!")
            return "";
    }
}

const char* LineEnding(int language)
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
        case GEN_LANG_PHP:
            return ";";

        default:
            return "";
    }
}

ttlib::cstr GetWidgetName(int language, ttlib::sview name)
{
    ttlib::cstr widget_name;
    if (language == GEN_LANG_CPLUSPLUS || language == GEN_LANG_PHP)
    {
        widget_name = name;
    }
    else if (language == GEN_LANG_PYTHON)
    {
        if (name == "wxEmptyString")
            widget_name = "\"\"";
        else
        {
            name.remove_prefix(2);
            widget_name << "wx." << name;
        }
    }
    else if (language == GEN_LANG_LUA)
    {
        widget_name << "wx." << name;
    }
    return widget_name;
}

// List of valid component parent types
static constexpr GenType s_GenParentTypes[] = {

    type_auinotebook, type_bookpage,    type_choicebook, type_container, type_listbook,
    type_notebook,    type_ribbonpanel, type_simplebook, type_splitter,  type_wizardpagesimple,

};

ttlib::cstr GetParentName(int language, Node* node)
{
    auto parent = node->GetParent();
    while (parent)
    {
        if (parent->IsSizer())
        {
            if (parent->IsStaticBoxSizer())
            {
                return (ttlib::cstr() << parent->get_node_name(language) << LangPtr(language) << "GetStaticBox()");
            }
        }
        if (parent->IsForm())
        {
            return ttlib::cstr("this");
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->isType(iter))
            {
                ttlib::cstr name = parent->get_node_name(language);
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    name << LangPtr(language) << "GetPane()";
                }
                return name;
            }
        }
        parent = parent->GetParent();
    }

    ASSERT_MSG(parent, ttlib::cstr() << node->get_node_name(language) << " has no parent!");
    return ttlib::cstr("internal error");
}

ttlib::cstr GenerateNewAssignment(int language, Node* node, bool use_generic)
{
    ttlib::cstr code(" = ");
    if (language == GEN_LANG_CPLUSPLUS || language == GEN_LANG_PHP)
        code << "new ";
    if (node->HasValue(prop_derived_class))
    {
        code << node->prop_as_string(prop_derived_class);
    }
    else if (use_generic)
    {
        ttlib::cstr class_name = node->DeclName();
        class_name.Replace("wx", "wxGeneric");
        class_name = GetWidgetName(language, class_name);
        code << class_name;
    }
    else if (node->isGen(gen_BookPage))
    {
        code << GetWidgetName(language, "wxPanel");
    }
    else
    {
        code << node->DeclName();
    }
    code << '(';

    return code;
}

ttlib::cstr GenerateQuotedString(int language, const ttlib::cstr& str)
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
            if (GetProject()->prop_as_bool(prop_internationalize) && language != GEN_LANG_LUA)
            {
                code << "_(" << GetWidgetName(language, "wxString") << LangPtr(language);
                code << str_with_escapes << "\"))";
            }
            else
            {
                code << "_(" << GetWidgetName(language, "wxString") << LangPtr(language);
                code << str_with_escapes << "\")";
            }

        }
        else
        {
            if (GetProject()->prop_as_bool(prop_internationalize) && language != GEN_LANG_LUA)
                code << "_(\"" << str_with_escapes << "\")";
            else
                code << "\"" << str_with_escapes << "\"";
        }
    }
    else
    {
        code << GetWidgetName(language, "wxEmptyString");
    }

    return code;
}
