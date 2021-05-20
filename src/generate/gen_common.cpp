/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common component functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <regex>

#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "gen_common.h"

#include "mainapp.h"  // App -- App class
#include "node.h"     // Node class
#include "utils.h"    // Utility functions that work with properties

ttlib::cstr GenerateSizerFlags(Node* node)
{
    ttlib::cstr code("wxSizerFlags");

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
            // generating an assert if you try to use one that is invalid if the sizer parent's orientation doesn't support
            // it. Center() just works without the assertion check.
            code << ".Center()";
        }

        if (prop.contains("wxALIGN_LEFT"))
        {
            code << ".Left()";
        }
        else if (prop.contains("wxALIGN_RIGHT"))
        {
            code << ".Right()";
        }

        if (prop.contains("wxALIGN_TOP"))
        {
            code << ".Top()";
        }
        else if (prop.contains("wxALIGN_BOTTOM"))
        {
            code << ".Bottom()";
        }
    }

    if (auto& prop = node->prop_as_string(prop_flags); prop.size())
    {
        if (prop.contains("wxEXPAND"))
        {
            code << ".Expand()";
        }
        if (prop.contains("wxSHAPED"))
        {
            code << ".Shaped()";
        }
        if (prop.contains("wxFIXED_MINSIZE"))
        {
            code << ".FixedMinSize()";
        }
        if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
        {
            code << ".ReserveSpaceEvenIfHidden()";
        }
    }

    if (auto& prop = node->prop_as_string(prop_borders); prop.size())
    {
        auto border_size = node->prop_as_string(prop_border_size);
        if (prop.contains("wxALL"))
        {
            if (border_size == "5")
                code << ".Border(wxALL)";
            else if (border_size == "10")
                code << ".DoubleBorder(wxALL)";
            else if (border_size == "15")
                code << ".TripleBorder(wxALL)";
            else
            {
                code << ".Border(wxALL, " << border_size << ')';
            }
        }
        else
        {
            code << ".Border(";
            ttlib::cstr border_flags;

            if (prop.contains("wxLEFT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << "wxLEFT";
            }
            if (prop.contains("wxRIGHT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << "wxRIGHT";
            }
            if (prop.contains("wxTOP"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << "wxTOP";
            }
            if (prop.contains("wxBOTTOM"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << "wxBOTTOM";
            }
            if (border_flags.empty())
                border_flags = "0";

            code << border_flags << ", ";
            if (border_size == "5")
            {
                code << "wxSizerFlags::GetDefaultBorder())";
            }
            else
            {
                code << border_size << ')';
            }
        }
    }

    return code;
}

void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr)
{
    if (node->isPropValue(prop_class_access, "none"))
    {
        set_src.insert(include);
    }
    else
    {
        set_hdr.insert(include);
    }
}

ttlib::cstr GenerateQuotedString(const ttlib::cstr& str)
{
    ttlib::cstr code;

    if (str.size())
    {
        auto str_with_escapes = ConvertToCodeString(str);
        if (wxGetApp().GetProject()->prop_as_bool(prop_internationalize))
            code << "_(wxString::FromUTF8(\"" << str_with_escapes << "\"))";
        else
            code << "wxString::FromUTF8(\"" << str_with_escapes << "\")";
    }
    else
    {
        code << "wxEmptyString";
    }

    return code;
}

ttlib::cstr GenerateQuotedString(Node* node, GenEnum::PropName prop_name)
{
    ttlib::cstr code;

    if (node->HasValue(prop_name))
    {
        auto str_with_escapes = ConvertToCodeString(node->prop_as_string(prop_name));
        if (wxGetApp().GetProject()->prop_as_bool(prop_internationalize))
            code << "_(wxString::FromUTF8(\"" << str_with_escapes << "\"))";
        else
            code << "wxString::FromUTF8(\"" << str_with_escapes << "\")";
    }
    else
    {
        code << "wxEmptyString";
    }

    return code;
}

// clang-format off

// List of valid component parent types
static constexpr GenType s_GenParentTypes[] = {

    type_auinotebook,
    type_bookpage,
    type_choicebook,
    type_container,
    type_listbook,
    type_notebook,
    type_simplebook,
    type_splitter,
    type_wizardpagesimple,

};

// clang-format on

ttlib::cstr GetParentName(Node* node)
{
    auto parent = node->GetParent();
    while (parent)
    {
        if (parent->IsSizer())
        {
            if (parent->IsStaticBoxSizer())
            {
                return (ttlib::cstr() << parent->get_node_name() << "->GetStaticBox()");
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
                ttlib::cstr name = parent->get_node_name();
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    name << "->GetPane()";
                }
                return name;
            }
        }
        parent = parent->GetParent();
    }

    ASSERT_MSG(parent, ttlib::cstr() << node->get_node_name() << " has no parent!");
    return ttlib::cstr("internal error");
}

void GenPos(Node* node, ttlib::cstr& code)
{
    auto point = node->prop_as_wxPoint(prop_pos);
    if (point.x != -1 || point.y != -1)
        code << "wxPoint(" << point.x << ", " << point.y << ")";
    else
        code << "wxDefaultPosition";
}

void GenSize(Node* node, ttlib::cstr& code)
{
    auto size = node->prop_as_wxPoint(prop_size);
    if (size.x != -1 || size.y != -1)
        code << "wxSize(" << size.x << ", " << size.y << ")";
    else
        code << "wxDefaultSize";
}

void GenStyle(Node* node, ttlib::cstr& code, ttlib::cview extra_style, ttlib::cview extra_def_value)
{
    auto& style = node->prop_as_string(prop_style);
    auto& win_style = node->prop_as_string(prop_window_style);

    if (style.empty() && win_style.empty() && extra_style.empty())
        code << "0";
    else
    {
        if (extra_style.size() && node->prop_as_string(prop_extra_style) != extra_def_value)
        {
            code << node->prop_as_string(prop_extra_style);
            if (style.size())
            {
                if (style.size())
                {
                    code << '|' << style;
                    if (win_style.size())
                    {
                        code << '|' << win_style;
                    }
                }
                else if (win_style.size())
                {
                    code << '|' << win_style;
                }
            }
        }
        else if (style.size())
        {
            code << style;
            if (win_style.size())
            {
                code << '|' << win_style;
            }
        }
        else if (win_style.size())
        {
            code << win_style;
        }
    }
}

void GeneratePosSizeFlags(Node* node, ttlib::cstr& code, bool uses_def_validator, ttlib::cview extra_style,
                          ttlib::cview extra_def_value)
{
    auto pos = node->prop_as_wxPoint(prop_pos);
    auto size = node->prop_as_wxPoint(prop_size);
    auto& style = node->prop_as_string(prop_style);
    auto& win_style = node->prop_as_string(prop_window_style);
    auto& win_name = node->prop_as_string(prop_window_name);

    if (win_name.size())
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        if (code.size() < 80)
            code << ", ";
        else
            code << ",\n\t\t";

        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code, extra_style, extra_def_value);
        if (uses_def_validator)
            code << ", wxDefaultValidator";
        code << ", " << node->prop_as_string(prop_window_name) << ");";
        return;
    }

    ttlib::cstr all_styles;
    if (extra_style.size())
        all_styles << node->prop_as_string(prop_extra_style);
    if (style.size())
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << style;
    }
    if (win_style.size())
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << win_style;
    }

    // If the only style specified is the default extra style, then clear it since we don't need to write anything.
    if (all_styles.size() && all_styles.is_sameas(extra_def_value))
        all_styles.clear();

    bool isPosSet = false;
    if (pos.x != -1 || pos.y != -1)
    {
        code << ", wxPoint(" << pos.x << ", " << pos.y << ")";
        ;
        isPosSet = true;
    }

    bool isSizeSet = false;
    if (size.x != -1 || size.y != -1)
    {
        if (!isPosSet)
        {
            code << ", wxDefaultPosition";
            isPosSet = true;
        }
        code << ", wxSize(" << size.x << ", " << size.y << ")";
        isSizeSet = true;
    }

    if (win_style.size() && win_style != "wxTAB_TRAVERSAL")
    {
        if (!isPosSet)
            code << ", wxDefaultPosition";
        if (!isSizeSet)
            code << ", wxDefaultSize";

        code << ", " << all_styles << ");";
        return;
    }

    if (all_styles.size())
    {
        if (!isPosSet)
            code << ", wxDefaultPosition";
        if (!isSizeSet)
            code << ", wxDefaultSize";

        if (code.size() < 100)
            code << ", ";
        else
        {
            code << ",\n\t\t";
            if (code.is_sameprefix("    "))
                code.insert(0, 4, ' ');
        }

        code << all_styles << ");";
        return;
    }

    code << ");";
}

// An event can be a regular function, a lambda, or a function in another class.
ttlib::cstr GenEventCode(NodeEvent* event, const std::string& class_name)
{
    ttlib::cstr code;
    ttlib::cstr handler;

    // This is what we normally use if an ID is needed. However, a lambda needs to put the ID on it's own line, so we
    // use a string for this to allow the lambda processing code to replace it.
    std::string comma(", ");

    bool is_lambda { false };

    if (event->get_value().contains("["))
    {
        handler << event->get_name() << ',' << event->get_value();
        // Put the lambda expression on it's own line
        handler.Replace("[", "\n\t[");
        comma = ",\n\t";
        is_lambda = true;
    }
    else if (event->get_value().contains("::"))
    {
        handler << event->get_name() << ", ";
        if (event->get_value()[0] != '&')
            handler << '&';
        handler << event->get_value();
    }
    else
    {
        handler << event->get_name() << ", &" << class_name << "::" << event->get_value() << ", this";
    }

    auto node = event->GetNode();

    if (node->IsStaticBoxSizer())
    {
        if (event->get_name() == "wxEVT_CHECKBOX")
        {
            code << node->prop_as_string(prop_checkbox_var_name);
        }
        else if (event->get_name() == "wxEVT_RADIOBUTTON")
        {
            code << node->prop_as_string(prop_radiobtn_var_name);
        }
        else
        {
            code << node->get_node_name() << "->GetStaticBox()";
        }
        code << "->Bind(" << handler << (is_lambda ? "\n\t);" : ");");
    }

    else if (node->isGen(gen_wxMenuItem) || node->isGen(gen_tool))
    {
        code << "Bind(" << handler << comma;
        if (event->GetNode()->prop_as_string(prop_id) != "wxID_ANY")
            code << event->GetNode()->prop_as_string(prop_id) << ");";
        else
            code << event->GetNode()->get_node_name() << "->GetId());";
    }
    else if (event->GetNode()->isGen(gen_ribbonTool))
    {
        if (event->GetNode()->prop_as_string(prop_id).empty())
        {
            code << "// **WARNING** -- tool id not specified, event handler may never be called\n    ";
            code << "Bind(" << handler << comma << "wxID_ANY);";
        }
        else
        {
            code << "Bind(" << handler << comma << event->GetNode()->prop_as_string(prop_id) << ");";
        }
    }
    else if (event->GetNode()->IsForm())
    {
        code << "Bind(" << handler << (is_lambda ? "\n\t);" : ");");
    }
    else
    {
        code << event->GetNode()->get_node_name() << "->Bind(" << handler << (is_lambda ? "\n\t);" : ");");
    }

    return code;
}

ttlib::cstr GenerateBitmapCode(const ttlib::cstr& description)
{
    ttlib::cstr code;

    if (description.empty())
    {
        code << "wxNullBitmap";
        return code;
    }

    static std::regex words_regex("\\[.+;.+\\]");

    ttlib::cstr desc_copy(description);

    // Convert "[num; num]" to "num, num"

    std::cmatch match;
    if (std::regex_search(description.c_str(), match, words_regex))
    {
        ttlib::cstr fix(match[0]);
        fix.Replace(";", ",");
        fix.Replace("[", "");
        fix.Replace("]", "");
        desc_copy.Replace(ttlib::cview(match[0]), fix);
    }

    ttlib::multistr parts(desc_copy, BMP_PROP_SEPARATOR);
    for (auto& iter: parts)
    {
        iter.BothTrim();
    }

    if (parts[IndexImage].empty())
    {
        code << "wxNullBitmap";
        return code;
    }

    if (parts[IndexType].contains("Art"))
    {
        code << "wxArtProvider::GetBitmap(" << parts[IndexArtID];
        if (parts[IndexArtClient].size())
            code << ", " << parts[IndexArtClient];
        code << ')';

        // This code is obsolete!
        if (!parts[IndexType].is_sameas("Art"))
            return code;

        // Scale if needed
        if (parts.size() > IndexConvert && parts[IndexSize].size())
        {
            auto scale_size = ConvertToSize(parts[IndexSize]);
            if (scale_size.x != -1 || scale_size.y != -1)
            {
                auto bmp = wxGetApp().GetImage(parts[IndexImage]);
                auto original_size = bmp.GetSize();
                if (scale_size.x != -1)
                    original_size.x = scale_size.x;
                if (scale_size.y != -1)
                    original_size.y = scale_size.y;
                code << ttlib::cstr().Format(".ConvertToImage().Scale(%d, %d)", original_size.x, original_size.y);
            }
        }

        return code;
    }

    ttlib::cstr result;
    if (parts[IndexType].is_sameas("XPM") || parts[IndexImage].filename().has_extension(".xpm"))
    {
        code << "wxImage(";

        ttlib::cstr name(parts[IndexImage].filename());
        name.remove_extension();
        code << name << "_xpm)";

        if (parts[IndexType].is_sameas("XPM") || parts[IndexType].is_sameas("Header"))
        {
            // Scale if needed
            if (parts.size() > IndexConvert && parts[IndexSize].size())
            {
                auto scale_size = ConvertToSize(parts[IndexSize]);
                if (scale_size.x != -1 || scale_size.y != -1)
                {
                    auto bmp = wxGetApp().GetImage(parts[IndexImage]);
                    auto original_size = bmp.GetSize();
                    if (scale_size.x != -1)
                        original_size.x = scale_size.x;
                    if (scale_size.y != -1)
                        original_size.y = scale_size.y;
                    // XPM files use a mask which does not scale well when wxIMAGE_QUALITY_HIGH is used
                    code << ttlib::cstr().Format(".Scale(%d, %d)", original_size.x, original_size.y);
                }
            }
        }

        // This code is obsolete!
        else
        {
            // Scale if needed
            if (parts.size() > 2 && parts[2].size())
            {
                auto scale_size = ConvertToSize(parts[2]);
                if (scale_size.x != -1 || scale_size.y != -1)
                {
                    auto bmp = wxGetApp().GetImage(parts[1]);
                    auto original_size = bmp.GetSize();
                    if (scale_size.x != -1)
                        original_size.x = scale_size.x;
                    if (scale_size.y != -1)
                        original_size.y = scale_size.y;
                    code << ttlib::cstr().Format(".Scale(%d, %d)", original_size.x, original_size.y);
                }
            }
        }
    }
    else
    {
        code << "GetImgFromHdr(";

        ttlib::cstr name(parts[1].filename());
        name.remove_extension();
        name.Replace(".", "_", true);  // wxFormBuilder writes files with the extra dots that have to be converted to '_'
        code << name << ", sizeof(" << name << "))";

        if (parts[IndexType].is_sameas("Header"))
        {
            // Scale if needed
            if (parts.size() > IndexConvert && parts[IndexSize].size())
            {
                auto scale_size = ConvertToSize(parts[IndexSize]);
                if (scale_size.x != -1 || scale_size.y != -1)
                {
                    auto bmp = wxGetApp().GetImage(description);
                    auto original_size = bmp.GetSize();
                    if (scale_size.x != -1)
                        original_size.x = scale_size.x;
                    if (scale_size.y != -1)
                        original_size.y = scale_size.y;
                    // PNG will have an alpha channel, so high-quality scaling makes sense
                    code << ttlib::cstr().Format(".Scale(%d, %d, wxIMAGE_QUALITY_HIGH)", original_size.x, original_size.y);
                }
            }
        }

        // This code is obsolete!
        else
        {
            // Scale if needed
            if (parts.size() > 2 && parts[2].size())
            {
                auto scale_size = ConvertToSize(parts[2]);
                if (scale_size.x != -1 || scale_size.y != -1)
                {
                    auto bmp = wxGetApp().GetImage(parts[1]);
                    auto original_size = bmp.GetSize();
                    if (scale_size.x != -1)
                        original_size.x = scale_size.x;
                    if (scale_size.y != -1)
                        original_size.y = scale_size.y;
                    code << ttlib::cstr().Format(".Scale(%d, %d, wxIMAGE_QUALITY_HIGH)", original_size.x, original_size.y);
                }
            }
        }
    }
    return code;
}

ttlib::cstr GenFormCode(GenEnum::GenCodeType command, Node* node)
{
    ttlib::cstr code;

    switch (command)
    {
        case code_base_class:
            if (node->HasValue(prop_base_class_name))
            {
                code << node->prop_as_string(prop_base_class_name);
            }
            else
            {
                code << node->DeclName();
            }
            break;

        case code_after_children:
            if (node->isGen(gen_wxToolBar))
            {
                if (code.size())
                    code << "\n";
                code << "\tRealize();";
            }
            else
            {
                auto& center = node->prop_as_string(prop_center);
                if (center.size() && !center.is_sameas("no"))
                {
                    code << "\tCentre(" << center << ");";
                }
            }
            break;

        case code_header:
            code << node->get_node_name() << "(wxWindow* parent, wxWindowID id = " << node->prop_as_string(prop_id);
            if (!node->isGen(gen_wxPanel) && !node->isGen(gen_wxToolBar))
            {
                code << ",\n\tconst wxString& title = ";
                auto& title = node->prop_as_string(prop_title);
                if (title.size())
                {
                    code << GenerateQuotedString(title) << ",\n\t";
                }
                else
                {
                    code << "wxEmptyString,\n\t";
                }
            }
            else
            {
                code << ", ";
            }
            code << "const wxPoint& pos = ";
            auto point = node->prop_as_wxPoint(prop_pos);
            if (point.x != -1 || point.y != -1)
                code << "wxPoint(" << point.x << ", " << point.y << ")";
            else
                code << "wxDefaultPosition";

            // BUGBUG: [KeyWorks - 05-20-2021] This doesn't make sense in a wxDialog because the generated code calls SetSize
            // with this value -- and without that call, SetSizerAndFit() will ignore this setting.
            code << ", const wxSize& size = ";
            auto size = node->prop_as_wxPoint(prop_size);
            if (size.x != -1 || size.y != -1)
                code << "wxSize(" << size.x << ", " << size.y << ")";
            else
                code << "wxDefaultSize";

            code << ",\n\tlong style = ";
            auto& style = node->prop_as_string(prop_style);
            auto& win_style = node->prop_as_string(prop_window_style);
            if (style.empty() && win_style.empty())
                code << "0";
            else
            {
                if (style.size())
                {
                    code << style;
                    if (win_style.size())
                    {
                        code << '|' << win_style;
                    }
                }
                else if (win_style.size())
                {
                    code << win_style;
                }
            }

            if (node->prop_as_string(prop_window_name).size())
            {
                code << ", const wxString& name = " << node->prop_as_string(prop_window_name);
            }

            code << ");\n\n";
            break;
    }

    return code;
}

ttlib::cstr GenFormSettings(Node* node)
{
    ttlib::cstr code;

    if (!node->isGen(gen_PanelForm) && !node->isGen(gen_wxToolBar))
    {
        auto min_size = node->prop_as_wxSize(prop_minimum_size);
        auto max_size = node->prop_as_wxSize(prop_maximum_size);

#if 0
// REVIEW: [KeyWorks - 05-20-2021] This is definitely wrong for a wxDialog (see issue #242) -- is it valid for a
// wxFrame or any other type of form?
        if (min_size != wxDefaultSize || max_size != wxDefaultSize)
        {
            code << "SetSizeHints(";
            if (min_size.GetX() != -1 || min_size.GetY() != -1)
                code << "wxSize(" << min_size.x << ", " << min_size.y << ")";
            else
                code << "wxDefaultSize";

            if (max_size.GetX() != -1 || max_size.GetY() != -1)
                code << ", wxSize(" << max_size.x << ", " << max_size.y << ")";

            code << ");";
        }
#endif

        if (node->HasValue(prop_icon))
        {
            if (code.size())
                code << '\n';
            auto image_code = GenerateBitmapCode(node->prop_as_string(prop_icon));
            if (!image_code.contains(".Scale") && image_code.is_sameprefix("wxImage("))
            {
                code << "SetIcon(wxIcon(" << image_code.subview(sizeof("wxImage")) << ");";
            }
            else
            {
                code << "wxIcon icon;\n";
                code << "icon.CopyFromBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_icon)) << ");\n";
                code << "SetIcon(wxIcon(icon));";
            }
        }
    }

    if (node->prop_as_string(prop_window_extra_style).size())
        code << "\nSetExtraStyle(GetExtraStyle() | " << node->prop_as_string(prop_window_extra_style) << ");";

    if (node->prop_as_string(prop_font).size())
    {
        code << "\nSetFont(wxFont(";
        auto fontprop = node->prop_as_font_prop(prop_font);
        wxFont font = fontprop.GetFont();
        auto pointSize = fontprop.GetPointSize();

        if (pointSize <= 0)
            code << "wxNORMAL_FONT->GetPointSize(), ";
        else
            code << pointSize << ", ";
        code << ConvertFontFamilyToString(fontprop.GetFamily()) << ", " << font.GetStyleString().wx_str();
        code << ", " << font.GetWeightString().wx_str() << ", " << (fontprop.isUnderlined() ? "true" : "false");
        if (fontprop.GetFaceName().empty())
            code << ", wxEmptyString";
        else
            code << ", \"" << fontprop.GetFaceName().wx_str() << "\"";
        code << ");";
    }

    auto& fg_clr = node->prop_as_string(prop_foreground_colour);
    if (fg_clr.size())
    {
        code << "\nSetForegroundColour(";
        if (fg_clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << fg_clr << "));";
        else
        {
            wxColour colour = ConvertToColour(fg_clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
    }

    auto& bg_clr = node->prop_as_string(prop_background_colour);
    if (bg_clr.size())
    {
        code << "\nSetBackgroundColour(";
        if (bg_clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << bg_clr << "));";
        else
        {
            wxColour colour = ConvertToColour(bg_clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
    }

    if (node->prop_as_bool(prop_disabled))
        code << "\nDisable();";

    if (node->prop_as_bool(prop_hidden))
        code << "\nHide();";

    return code;
}

ttlib::cstr GenerateColorCode(Node* node, PropName prop_name)
{
    ttlib::cstr code;
    auto& clr = node->prop_as_string(prop_name);
    if (clr.contains("wx"))
        code << "wxSystemSettings::GetColour(" << clr << ")";
    else
    {
        wxColour colour = ConvertToColour(clr);
        code << ttlib::cstr().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
    }

    return code;
}

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string. Used when generating
// code.
ttlib::cstr ConvertToCodeString(const ttlib::cstr& text)
{
    ttlib::cstr result;

    for (auto c: text)
    {
        switch (c)
        {
            case '"':
                result += "\\\"";
                break;

            case '\\':
                result += "\\\\";
                break;

            case '\t':
                result += "\\t";
                break;

            case '\n':
                result += "\\n";
                break;

            case '\r':
                result += "\\r";
                break;

            default:
                result += c;
                break;
        }
    }
    return result;
}
