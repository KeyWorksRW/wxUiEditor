/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common component functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <charconv>  // for std::to_chars

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "gen_common.h"

#include "lambdas.h"      // Functions for formatting and storage of lamda events
#include "mainapp.h"      // App -- App class
#include "node.h"         // Node class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties

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

ttlib::cstr GenerateColourCode(Node* node, GenEnum::PropName prop_name)
{
    ttlib::cstr code;

    if (!node->HasValue(prop_name))
    {
        code = "wxNullColour";
    }
    else
    {
        if (node->prop_as_string(prop_name).contains("wx"))
        {
            code << "wxSystemSettings::GetColour(" << node->prop_as_string(prop_name) << ")";
        }
        else
        {
            auto colour = node->prop_as_wxColour(prop_name);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
        }
    }

    return code;
}

ttlib::cstr GenerateQuotedString(const ttlib::cstr& str)
{
    ttlib::cstr code;

    if (str.size())
    {
        auto str_with_escapes = ConvertToCodeString(str);

        bool has_utf_char = false;
        for (auto iter: str_with_escapes)
        {
            if (static_cast<signed char>(iter) < 0)
            {
                has_utf_char = true;
                break;
            }
        }

        if (has_utf_char)
        {
            // While this may not be necessary for non-Windows systems, it does ensure the code compiles on all platforms.
            if (wxGetApp().GetProject()->prop_as_bool(prop_internationalize))
                code << "_(wxString::FromUTF8(\"" << str_with_escapes << "\"))";
            else
                code << "wxString::FromUTF8(\"" << str_with_escapes << "\")";
        }
        else
        {
            if (wxGetApp().GetProject()->prop_as_bool(prop_internationalize))
                code << "_(\"" << str_with_escapes << "\")";
            else
                code << "\"" << str_with_escapes << "\"";
        }
    }
    else
    {
        code << "wxEmptyString";
    }

    return code;
}

ttlib::cstr GenerateQuotedString(Node* node, GenEnum::PropName prop_name)
{
    if (node->HasValue(prop_name))
    {
        return GenerateQuotedString(node->prop_as_string(prop_name));
    }
    else
    {
        return ttlib::cstr("wxEmptyString");
    }
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
    type_ribbonpanel,
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
    {
        if (node->prop_as_string(prop_pos).contains("d", tt::CASE::either))
        {
            code << ", ConvertPixelsToDialog(wxPoint(" << point.x << ", " << point.y << "))";
        }
        else
        {
            code << "wxPoint(" << point.x << ", " << point.y << ")";
        }
    }
    else
        code << "wxDefaultPosition";
}

void GenSize(Node* node, ttlib::cstr& code)
{
    auto size = node->prop_as_wxSize(prop_size);
    if (size != wxDefaultSize)
    {
        if (node->prop_as_string(prop_size).contains("d", tt::CASE::either))
        {
            code << ", ConvertPixelsToDialog(wxSize(" << size.x << ", " << size.y << "))";
        }
        else
        {
            code << "wxSize(" << size.x << ", " << size.y << ")";
        }
    }
    else
        code << "wxDefaultSize";
}

void GenStyle(Node* node, ttlib::cstr& code, const char* prefix)
{
    ttlib::cstr all_styles;

    if (node->HasValue(prop_tab_position) && !node->prop_as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_tab_position);
    }

    if (node->HasValue(prop_orientation) && !node->prop_as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_orientation);
    }

    if (node->isGen(gen_wxRichTextCtrl))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << "wxRE_MULTILINE";
    }

    if (node->HasValue(prop_style))
    {
        if (all_styles.size())
        {
            all_styles << '|';
        }
        if (prefix)
        {
            all_styles << node->prop_as_constant(prop_style, prefix);
        }
        else
        {
            all_styles << node->prop_as_string(prop_style);
        }
    }

    if (node->HasValue(prop_window_style))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_window_style);
    }

    if (node->isGen(gen_wxListView))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_mode);
    }

    if (all_styles.empty())
    {
        code << "0";
    }
    else
    {
        code << all_styles;
    }
}

void GeneratePosSizeFlags(Node* node, ttlib::cstr& code, bool uses_def_validator, ttlib::cview def_style)
{
    if (node->HasValue(prop_window_name))
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
        GenStyle(node, code);
        if (uses_def_validator)
            code << ", wxDefaultValidator";
        code << ", " << node->prop_as_string(prop_window_name) << ");";
        return;
    }

    ttlib::cstr all_styles;
    GenStyle(node, all_styles);
    if (all_styles.is_sameas("0") || all_styles.is_sameas(def_style))
        all_styles.clear();

    bool isPosSet { false };
    auto pos = node->prop_as_wxPoint(prop_pos);
    if (pos.x != -1 || pos.y != -1)
    {
        if (node->prop_as_string(prop_pos).contains("d", tt::CASE::either))
        {
            code << ", ConvertPixelsToDialog(wxPoint(" << pos.x << ", " << pos.y << "))";
        }
        else
        {
            code << ", wxPoint(" << pos.x << ", " << pos.y << ")";
        }

        isPosSet = true;
    }

    bool isSizeSet { false };
    auto size = node->prop_as_wxSize(prop_size);
    if (size.x != -1 || size.y != -1)
    {
        if (!isPosSet)
        {
            code << ", wxDefaultPosition";
            isPosSet = true;
        }
        if (node->prop_as_string(prop_size).contains("d", tt::CASE::either))
        {
            code << ", ConvertPixelsToDialog(wxSize(" << size.x << ", " << size.y << "))";
        }
        else
        {
            code << ", wxSize(" << size.x << ", " << size.y << ")";
        }

        isSizeSet = true;
    }

    if (node->HasValue(prop_window_style) && !node->prop_as_string(prop_window_style).is_sameas("wxTAB_TRAVERSAL"))
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
            {
                code.insert(0, 4, ' ');
            }
            else if (code.at(0) != '\t')
            {
                code.insert(0, 1, '\t');
            }
        }

        code << all_styles << ");";
        return;
    }

    code << ");";
}

int GetStyleInt(Node* node, const char* prefix)
{
    ttlib::cstr styles;

    // If prefix is non-null, this will convert friendly names to wxWidgets constants
    GenStyle(node, styles, prefix);

    int result = 0;
    // Can't use multiview because GetConstantAsInt() searches an unordered_map which requires a std::string to pass to it
    ttlib::multistr mstr(styles, '|');
    for (auto& iter: mstr)
    {
        // Friendly names will have already been converted, so normal lookup works fine.
        result |= g_NodeCreator.GetConstantAsInt(iter);
    }
    return result;
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
        ExpandLambda(handler);
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
        code << "->Bind(" << handler << (is_lambda ? " );" : ");");
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
        code << "Bind(" << handler << (is_lambda ? " );" : ");");
    }
    else
    {
        code << event->GetNode()->get_node_name() << "->Bind(" << handler << (is_lambda ? " );" : ");");
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

    ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty())
    {
        code << "wxNullBitmap";
        return code;
    }

    wxSize scale_size { -1, -1 };

    // If a dimension was specified, then it will have been split out, so we need to combine them
    if (parts.size() > IndexScale)
    {
        GetScaleInfo(scale_size, parts[IndexScale]);
    }

    if (parts[IndexType].contains("Art"))
    {
        ttlib::cstr art_id(parts[IndexArtID]);
        ttlib::cstr art_client;
        if (auto pos = art_id.find('|'); ttlib::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "wxArtProvider::GetBitmap(" << art_id;
        if (art_client.size())
            code << ", " << art_client;
        code << ')';

        // Scale if needed
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

        return code;
    }

    ttlib::cstr result;
    if (parts[IndexType].is_sameas("XPM") || parts[IndexImage].extension().is_sameas(".xpm", tt::CASE::either))
    {
        code << "wxImage(";

        ttlib::cstr name(parts[IndexImage].filename());
        name.remove_extension();
        code << name << "_xpm)";

        // Scale if needed
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
    else
    {
        code << "GetImageFromArray(";

        ttlib::cstr name(parts[1].filename());
        name.remove_extension();
        name.Replace(".", "_", true);  // wxFormBuilder writes files with the extra dots that have to be converted to '_'

        if (parts[IndexType].is_sameprefix("Embed"))
        {
            auto embed = wxGetApp().GetProjectSettings()->GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                name = "wxue_img::" + embed->array_name;
            }
        }

        code << name << ", sizeof(" << name << "))";

        // Scale if needed
        if (scale_size.x != -1 || scale_size.y != -1)
        {
            auto bmp = wxGetApp().GetImage(description);
            auto original_size = bmp.GetSize();
            if (scale_size.x != -1)
                original_size.x = scale_size.x;
            if (scale_size.y != -1)
                original_size.y = scale_size.y;
            // Assume an alpha channel, so high-quality scaling makes sense
            code << ttlib::cstr().Format(".Scale(%d, %d, wxIMAGE_QUALITY_HIGH)", original_size.x, original_size.y);
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
            if (node->HasValue(prop_derived_class))
            {
                code << node->prop_as_string(prop_derived_class);
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
            else if (node->isGen(gen_RibbonBar))
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
            if (!node->isGen(gen_wxPanel) && !node->isGen(gen_wxToolBar) && !node->isGen(gen_ToolBar) &&
                !node->isGen(gen_wxAuiToolBar) && !node->isGen(gen_wxPopupTransientWindow) && !node->isGen(gen_PanelForm))
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
        auto max_size = node->prop_as_wxSize(prop_maximum_size);
        auto min_size = node->prop_as_wxSize(prop_minimum_size);
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

    code << GenFontColourSettings(node);

    if (node->prop_as_bool(prop_disabled))
        code << "\nDisable();";

    if (node->prop_as_bool(prop_hidden))
        code << "\nHide();";

    return code;
}

ttlib::cstr GenFontColourSettings(Node* node)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_font).size())
    {
        FontProperty fontprop(node->get_prop_ptr(prop_font));
        if (fontprop.isDefGuiFont())
        {
            code << "{\n\twxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));";
            if (fontprop.GetSymbolSize() != wxFONTSIZE_MEDIUM)
                code << "\n\tfont.SetSymbolicSize(" << font_symbol_pairs.GetValue(fontprop.GetSymbolSize()) << ");";
            if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
                code << "\n\tfont.SetStyle(" << font_style_pairs.GetValue(fontprop.GetStyle()) << ");";
            if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
                code << "\n\tfont.SetWeight(" << font_weight_pairs.GetValue(fontprop.GetWeight()) << ");";
            if (fontprop.IsUnderlined())
                code << "\n\tfont.SetUnderlined(true);";
            if (fontprop.IsStrikethrough())
                code << "\n\tfont.SetStrikethrough(true);";

            if (node->IsForm())
            {
                code << "\n\t"
                     << "SetFont(font);\n}";
            }
            else
            {
                code << "\n\t" << node->get_node_name() << "->SetFont(font);\n}";
            }
        }
        else
        {
            auto point_size = fontprop.GetFractionalPointSize();
            code << "{\n\twxFontInfo font_info(";
            if (point_size != static_cast<int>(point_size))
            {
                code << "\n#if (wxMAJOR_VERSION < 3) || ((wxMAJOR_VERSION == 3) && (wxMINOR_VERSION < 2) && "
                        "(wxRELEASE_NUMBER < 2))\n\t";
                if (point_size <= 0)
                {
                    code << "wxSystemSettings::GetFont()->GetPointSize());";
                }
                else
                {
                    // GetPointSize() will round the result rather than truncating the decimal
                    code << fontprop.GetPointSize() << ");";
                }
                code << "\n#else  // fractional point sizes are new to wxWidgets 3.1.2\n\t";
                {
                    std::array<char, 10> float_str;
                    if (auto [ptr, ec] = std::to_chars(float_str.data(), float_str.data() + float_str.size(), point_size);
                        ec == std::errc())
                    {
                        code << std::string_view(float_str.data(), ptr - float_str.data()) << ");";
                    }
                }

                // leave a trailing dot -- we'll remove the final dot after all the params are added
                code << "\n#endif";
            }
            else
            {
                if (point_size <= 0)
                {
                    code << "wxSystemSettings::GetFont()->GetPointSize());";
                }
                else
                {
                    // GetPointSize() will round the result rather than truncating the decimal
                    code << fontprop.GetPointSize() << ");";
                }
            }

            ttlib::cstr info_code("\n\tfont_info.");

            if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
                info_code << "FaceName(" << fontprop.GetFaceName().wx_str() << ").";
            if (fontprop.GetFamily() != wxFONTFAMILY_DEFAULT)
                info_code << "Family(" << font_family_pairs.GetValue(fontprop.GetFamily()) << ").";
            if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
                info_code << "Style(" << font_style_pairs.GetValue(fontprop.GetStyle()) << ").";
            if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
                info_code << "Weight(" << font_weight_pairs.GetValue(fontprop.GetWeight()) << ").";
            if (fontprop.IsUnderlined())
                info_code << "Underlined().";
            if (fontprop.IsStrikethrough())
                info_code << "Strikethrough()";

            if (info_code.back() == '.')
                info_code.pop_back();

            if (!info_code.is_sameas("font_info"))
                code << info_code << ';';

            if (node->IsForm())
            {
                code << "\n\t"
                        "SetFont(wxFont(font_info));\n}";
            }
            else
            {
                code << "\n\t" << node->get_node_name() << "->SetFont(wxFont(font_info));\n}";
            }
        }
    }
    auto& fg_clr = node->prop_as_string(prop_foreground_colour);
    if (fg_clr.size())
    {
        if (code.size())
            code << '\n';

        if (node->IsForm())
        {
            code << "SetForegroundColour(";
        }
        else
        {
            code << node->get_node_name() << "->SetForegroundColour(";
        }
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
        if (node->IsForm())
        {
            code << "SetBackgroundColour(";
        }
        else
        {
            code << node->get_node_name() << "->SetBackgroundColour(";
        }
        if (bg_clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << bg_clr << "));";
        else
        {
            wxColour colour = ConvertToColour(bg_clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
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

ttlib::cstr GenerateNewAssignment(Node* node, bool use_generic)
{
    ttlib::cstr code(" = new ");
    if (node->HasValue(prop_derived_class))
    {
        code << node->prop_as_string(prop_derived_class);
    }
    else if (use_generic)
    {
        ttlib::cstr class_name = node->DeclName();
        class_name.Replace("wx", "wxGeneric");
        code << class_name;
    }
    else
    {
        code << node->DeclName();
    }
    code << '(';
    return code;
}
