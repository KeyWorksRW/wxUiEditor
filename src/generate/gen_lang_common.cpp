/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common mulit-language functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <ttmultistr_wx.h>  // ttMultiString -- Class for handling multiple strings

#include "gen_lang_common.h"

#include "gen_common.h"     // Common component functions
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
// List of valid component parent types
static constexpr GenType s_GenParentTypes[] = {

    type_auinotebook, type_bookpage,    type_choicebook, type_container, type_listbook,
    type_notebook,    type_ribbonpanel, type_simplebook, type_splitter,  type_wizardpagesimple,

};

ttlib::cstr GetPythonParentName(Node* node)
{
    auto parent = node->GetParent();
    while (parent)
    {
        if (parent->IsSizer())
        {
            if (parent->IsStaticBoxSizer())
            {
                return (ttlib::cstr() << parent->get_node_name() << ".GetStaticBox()");
            }
        }
        if (parent->IsForm())
        {
            return ttlib::cstr("self");
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->isType(iter))
            {
                ttlib::cstr name = parent->get_node_name();
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    name << ".GetPane()";
                }
                return name;
            }
        }
        parent = parent->GetParent();
    }

    ASSERT_MSG(parent, ttlib::cstr() << node->get_node_name() << " has no parent!");
    return ttlib::cstr("internal error");
}

ttlib::cstr GenerateNewAssignment(int language, Node* node, bool use_generic)
{
    ttlib::cstr code(" = ");
    if (language == GEN_LANG_CPLUSPLUS)
        code << "new ";
    if (language == GEN_LANG_CPLUSPLUS && node->HasValue(prop_derived_class))
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
        code << GetWidgetName(language, node->DeclName());
    }
    code << '(';

    return code;
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

static void AddPropIfUsed(int language, PropName prop_name, ttlib::sview func_name, Node* node, ttlib::cstr& code)
{
    if (prop_name == prop_background_colour)
    {
        auto& color = node->prop_as_string(prop_background_colour);
        if (color.size())
        {
            if (code.size())
                code << "\n\t";
            if (!node->IsForm())
                code << node->get_node_name() << LangPtr(language);
            code << "SetBackgroundColour(";
            if (color.contains("wx"))
            {
                code << GetWidgetName(language, "wxSystemSettings") << LangPtr(language) << "GetColour(" << color << "))";
            }
            else
            {
                wxColour colour = ConvertToColour(color);
                code << GetWidgetName(language, "wxColour")
                     << ttlib::cstr().Format("(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
            }
        }
    }
    else if (prop_name == prop_foreground_colour)
    {
        auto& color = node->prop_as_string(prop_foreground_colour);
        if (color.size())
        {
            if (code.size())
                code << "\n\t";
            if (!node->IsForm())
                code << node->get_node_name() << LangPtr(language);
            code << "SetForegroundColour(";
            if (color.contains("wx"))
            {
                code << GetWidgetName(language, "wxSystemSettings") << LangPtr(language) << "GetColour(" << color << "))";
            }
            else
            {
                wxColour colour = ConvertToColour(color);
                code << GetWidgetName(language, "wxColour")
                     << ttlib::cstr().Format("(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
            }
        }
    }
    else if (node->prop_as_string(prop_name).size())
    {
        if (code.size())
            code << "\n\t";
        if (!node->IsForm())
            code << node->get_node_name() << LangPtr(language);
        code << func_name << GetWidgetName(language, node->prop_as_string(prop_name)) << ")";
    }
}

void GenerateWindowSettings(int language, Node* node, ttlib::cstr& code)
{
    AddPropIfUsed(language, prop_window_extra_style, "SetExtraStyle(", node, code);
    code << GenFontColourSettings(node);

    // Note that \t is added after the \n in case the caller needs to keep indents

    if (node->prop_as_bool(prop_disabled))
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << LangPtr(language);
        code << "Enable(false)";
    }

    if (node->prop_as_bool(prop_hidden))
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << LangPtr(language);
        code << "Hide()";
    }

    bool allow_minmax { true };
    bool is_smart_size { false };

    // REVIEW: [KeyWorks - 05-20-2021] Because of issue #242, this was shut off for all forms.

    // The following needs to match GenFormSettings() in gen_common.cpp. If these conditions are met, then GenFormSettings()
    // will generate SetSizeHints(min, max) so there is no reason to generate SetMinSize()/SetMaxSize()
    if (node->IsForm() && !node->isGen(gen_PanelForm) && !node->isGen(gen_wxToolBar))
        allow_minmax = false;

    auto size = node->prop_as_wxSize(prop_smart_size);
    if (size != wxDefaultSize)
    {
        is_smart_size = true;  // Set to true means prop_size and prop_minimum_size will be ignored

        // REVIEW: [KeyWorks - 12-07-2021] Do we need to block if allow_minmax is false?

        if (code.size())
            code << "\n";
        code << node->get_node_name() << LangPtr(language) << "SetInitialSize(";
        if (node->prop_as_string(prop_smart_size).contains("d", tt::CASE::either))
            code << "ConvertDialogToPixels(";

        if (size.IsFullySpecified())
            code << "\n\t";
        code << "wxSize(";

        if (size.x != -1)
            code << size.x << " > GetBestSize().x ? " << size.x << " : -1, ";
        else
            code << "-1, ";
        if (size.y != -1)
            code << size.y << " > GetBestSize().y ? " << size.y << " : -1";
        else
            code << "-1";
        if (node->prop_as_string(prop_smart_size).contains("d", tt::CASE::either))
            code << ')';  // close the ConvertDialogToPixels function call
        code << "))";
    }

    if (!is_smart_size && allow_minmax)
    {
        if (node->prop_as_wxSize(prop_minimum_size) != wxDefaultSize)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << LangPtr(language);
            code << "SetMinSize(" << GenerateWxSize(node, prop_minimum_size) << ")";
        }
    }

    if (node->prop_as_wxSize(prop_maximum_size) != wxDefaultSize)
    {
        if (allow_minmax)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << LangPtr(language);
            code << "SetMaxSize(" << GenerateWxSize(node, prop_maximum_size) << ")";
        }
    }

    if (!node->IsForm() && !node->isPropValue(prop_variant, "normal"))
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << LangPtr(language) << "SetWindowVariant(";

        if (node->isPropValue(prop_variant, "small"))
            code << "wxWINDOW_VARIANT_SMALL)";
        else if (node->isPropValue(prop_variant, "mini"))
            code << "wxWINDOW_VARIANT_MINI)";
        else
            code << "wxWINDOW_VARIANT_LARGE)";
    }

    if (node->prop_as_string(prop_tooltip).size())
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << LangPtr(language);
        code << "SetToolTip(" << GenerateQuotedString(node->prop_as_string(prop_tooltip)) << ")";
    }

    if (node->prop_as_string(prop_context_help).size())
    {
        if (code.size())
            code << "\n";
        if (!node->IsForm())
            code << node->get_node_name() << LangPtr(language);
        code << "SetHelpText(" << GenerateQuotedString(node->prop_as_string(prop_context_help)) << ")";
    }
}
