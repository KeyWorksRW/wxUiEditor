/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common XRC generating functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_xrc_utils.h"

#include "gen_base.h"     // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainapp.h"      // App -- App class
#include "node.h"         // Node class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties
#include "write_code.h"   // WriteCode -- Write code to Scintilla or file

// clang-format off

const char* g_xrc_keywords =

    "accel align animation art-provider "
    "base best_size bg bitmap bitmap-small bitmap2 bitmapposition bitmapsize border borders bottom bottom_dockable border buttons buttonsize "
    "caption caption_visible cellpos cellspan center center_pane centered centre centre_pane checkable checked class close_button col cols "
        "collapsed content current "
    "data default defaultdirectory defaultfilename defaultfilter defaultfolder default_pane default_size depth digitis dimension "
        "disabled  direction disabled-bitmap dock dock_fixed dontattachtoframe dropdown "
    "effectduration enabled expanded exstyle extra-accels "
    "fields filter flag flexibledirection float floatable floating_size fg focus focused font "
    "gradient-end gradient-start gravity gripper growablecols growablerows "
    "help helptext hgap hidden hideeffect hint horizontal htmlcode hybrid "
    "icon inc image image-small imagelist imagelist-small inactive-bitmap item "
    "label layer left left_dockable linesize longhelp "
    "margins markup max max_size maximize_button maxlength message min minsize min_size minimize_button movable "
    "name nonflexiblegrowmode null-text "
    "object object_ref option orient orientation "
    "packing pagesize pane_border perspective pin_button pos pressed proportion "
    "radio range ratio resizable resource right right_dockable row rows "
    "sashpos scrollrate selected selection selmax selmin separation showeffect size small-bitmap small-disabled-bitmap state stock_client "
        "stock_id style styles "
    "text textcolour thumb thumbsize tick tickfreq title toggle toolbar_pane tooltip top top_dockable "
    "url "
    "value variant vertical vgap "
    "width widths wildcard windowlabel wrap wrapmode"

    ;

// clang-format on

void GenXrcSizerItem(Node* node, BaseCodeGenerator* code_gen)
{
    auto m_source = code_gen->GetSrcWriter();
    m_source->writeLine("<object class=\"sizeritem\">");
    m_source->Indent();
    ttlib::cstr flags;
    flags << node->prop_as_string(prop_borders);
    if (node->HasValue(prop_flags))
    {
        if (flags.size())
        {
            flags << '|';
        }
        flags << node->prop_as_string(prop_flags);
    }
    if (node->HasValue(prop_alignment))
    {
        if (flags.size())
        {
            flags << '|';
        }
        flags << node->prop_as_string(prop_alignment);
    }
    if (flags.size())
    {
        m_source->writeLine(ttlib::cstr("<flag>") << flags << "</flag>");
    }

    if (node->HasValue(prop_border_size))
    {
        m_source->writeLine(ttlib::cstr("<border>") << node->prop_as_string(prop_border_size) << "</border>");
    }
}

void GenXrcSizerItem(Node* node, pugi::xml_node& object)
{
    object.append_attribute("class").set_value("sizeritem");
    ttlib::cstr flags;
    flags << node->prop_as_string(prop_borders);
    if (node->HasValue(prop_flags))
    {
        if (flags.size())
        {
            flags << '|';
        }
        flags << node->prop_as_string(prop_flags);
    }
    if (node->HasValue(prop_alignment))
    {
        if (flags.size())
        {
            flags << '|';
        }
        flags << node->prop_as_string(prop_alignment);
    }
    object.append_child("flag").text().set(flags.c_str());
    if (node->HasValue(prop_border_size))
    {
        object.append_child("border").text().set(node->prop_as_string(prop_border_size));
    }
    if (node->prop_as_string(prop_proportion) != "0")
    {
        object.append_child("option").text().set(node->prop_as_string(prop_proportion));
    }
    if (node->HasValue(prop_minimum_size))
    {
        object.append_child("minsize").text().set(node->prop_as_string(prop_minimum_size));
    }
}

void GenXrcComments(Node* node, pugi::xml_node& object, size_t supported_flags)
{
    if (node->HasValue(prop_smart_size))
    {
        object.append_child(pugi::node_comment).set_value(" smart size cannot be be set in the XRC file. ");
    }
    if (node->HasValue(prop_maximum_size) && !(supported_flags & xrc::max_size_supported))
    {
        object.append_child(pugi::node_comment).set_value(" maximum size cannot be be set in the XRC file. ");
    }
}

void GenXrcStylePosSize(Node* node, pugi::xml_node& object, PropName other_style)
{
    ttlib::cstr combined_style(node->prop_as_string(prop_style));
    if (other_style != prop_unknown && node->HasValue(other_style))
    {
        if (combined_style.size())
        {
            combined_style << '|';
        }
        combined_style << node->prop_as_string(other_style);
    }

    if (node->HasValue(prop_window_style))
    {
        if (combined_style.size())
        {
            combined_style << '|';
        }
        combined_style << node->prop_as_string(prop_window_style);
    }

    if (combined_style.size())
    {
        object.append_child("style").text().set(combined_style);
    }

    if (node->HasValue(prop_pos))
    {
        object.append_child("pos").text().set(node->prop_as_string(prop_pos));
    }
    if (node->HasValue(prop_size))
    {
        object.append_child("size").text().set(node->prop_as_string(prop_size));
    }
}

void GenXrcPreStylePosSize(Node* node, pugi::xml_node& object, std::string_view processed_style)
{
    ttlib::cstr combined_style(processed_style);

    if (node->HasValue(prop_window_style))
    {
        if (combined_style.size())
        {
            combined_style << '|';
        }
        combined_style << node->prop_as_string(prop_window_style);
    }

    if (combined_style.size())
    {
        object.append_child("style").text().set(combined_style);
    }

    if (node->HasValue(prop_pos))
    {
        object.append_child("pos").text().set(node->prop_as_string(prop_pos));
    }
    if (node->HasValue(prop_size))
    {
        object.append_child("size").text().set(node->prop_as_string(prop_size));
    }
}

// clang-format off
static std::map<wxFontWeight, const char*> s_weight_pairs = {
    { wxFONTWEIGHT_THIN, "thin" },
    { wxFONTWEIGHT_EXTRALIGHT, "extralight" },
    { wxFONTWEIGHT_LIGHT,"light" },
    { wxFONTWEIGHT_NORMAL, "normalweight" },
    { wxFONTWEIGHT_MEDIUM, "medium" },
    { wxFONTWEIGHT_SEMIBOLD, "semibold" },
    { wxFONTWEIGHT_BOLD, "bold" },
    { wxFONTWEIGHT_EXTRABOLD, "extrabold" },
    { wxFONTWEIGHT_HEAVY, "heavy" },
    { wxFONTWEIGHT_EXTRAHEAVY, "extraheavy" },
};

static std::map<wxFontFamily, const char*> s_family_pairs = {
    { wxFONTFAMILY_DEFAULT, "default" },
    { wxFONTFAMILY_DECORATIVE, "decorative"},
    { wxFONTFAMILY_ROMAN, "roman"},
    { wxFONTFAMILY_SCRIPT, "script"},
    { wxFONTFAMILY_SWISS, "swiss"},
    { wxFONTFAMILY_MODERN, "modern"},
    { wxFONTFAMILY_TELETYPE, "teletype"},
};
// clang-format on

void GenXrcFont(pugi::xml_node& object, FontProperty& font_prop)
{
    auto font_object = object.append_child("font");
    font_object.append_child("size").text().set(font_prop.GetFractionalPointSize());
    if (font_prop.GetStyle() == wxFONTSTYLE_ITALIC)
        font_object.append_child("style").text().set("italic");
    else if (font_prop.GetStyle() == wxFONTSTYLE_SLANT)
        font_object.append_child("style").text().set("slant");
    if (font_prop.GetWeight() != wxFONTWEIGHT_NORMAL)
        font_object.append_child("weight").text().set(s_weight_pairs[font_prop.GetWeight()]);
    if (font_prop.GetFamily() != wxFONTFAMILY_DEFAULT)
        font_object.append_child("family").text().set(s_family_pairs[font_prop.GetFamily()]);
    if (font_prop.HasFaceName() && font_prop.GetFaceName() != "default")
        font_object.append_child("face").text().set(font_prop.GetFaceName().ToUTF8().data());
    if (font_prop.IsUnderlined())
        font_object.append_child("underlined").text().set("1");
    if (font_prop.IsStrikethrough())
        font_object.append_child("strikethrough").text().set("1");
}

void GenXrcWindowSettings(Node* node, pugi::xml_node& object)
{
    if (node->prop_as_bool(prop_hidden))
    {
        // Hidden is set in the XRC_MAKE_INSTANCE macro
        object.append_child("hidden").text().set("1");
    }
    if (node->HasValue(prop_variant) && node->prop_as_string(prop_variant) != "normal")
    {
        object.append_child("variant").text().set(node->prop_as_string(prop_variant));
    }
    if (node->HasValue(prop_tooltip))
    {
        object.append_child("tooltip").text().set(node->prop_as_string(prop_tooltip));
    }
    if (node->HasValue(prop_font))
    {
        auto font_prop = node->prop_as_font_prop(prop_font);
        GenXrcFont(object, font_prop);
    }
    if (node->HasValue(prop_background_colour))
    {
        object.append_child("bg").text().set(
            node->prop_as_wxColour(prop_background_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }
    if (node->HasValue(prop_foreground_colour))
    {
        object.append_child("fg").text().set(
            node->prop_as_wxColour(prop_foreground_colour).GetAsString(wxC2S_HTML_SYNTAX).ToUTF8().data());
    }
    if (node->prop_as_bool(prop_disabled))
    {
        object.append_child("enabled").text().set("0");
    }
    if (node->prop_as_bool(prop_focus))
    {
        object.append_child("focused").text().set("1");
    }
    if (node->HasValue(prop_extra_style))
    {
        object.append_child("exstyle").text().set(node->prop_as_string(prop_extra_style));
    }
    if (node->HasValue(prop_context_help))
    {
        object.append_child("help").text().set(node->prop_as_string(prop_context_help));
    }
}

struct PropNamePair
{
    GenEnum::PropName prop;
    const char* xrc_name;
};

static PropNamePair props[] = {

    { prop_bitmap, "bitmap" },
    { prop_pressed_bmp, "pressed" },
    { prop_focus_bmp, "focus" },
    { prop_disabled_bmp, "disabled" },
    { prop_current, "current" }
};

void GenXrcBitmap(Node* node, pugi::xml_node& object)
{
    for (auto& prop_pair: props)
    {
        if (node->HasValue(prop_pair.prop))
        {
            ttlib::multistr parts(node->prop_as_string(prop_pair.prop), ';', tt::TRIM::both);
            ASSERT(parts.size() > 1)
            if (parts[IndexType].is_sameas("Art"))
            {
                ttlib::multistr art_parts(parts[IndexArtID], '|');
                auto bmp = object.append_child(prop_pair.xrc_name);
                bmp.append_attribute("stock_id").set_value(art_parts[0].c_str());
                bmp.append_attribute("stock_client").set_value(art_parts[1].c_str());
            }
            else if (parts[IndexType].is_sameas("SVG"))
            {
                auto svg_object = object.append_child(prop_pair.xrc_name);
                svg_object.text().set(parts[IndexImage]);
                auto size = get_image_prop_size(parts[IndexSize]);
                svg_object.append_attribute("default_size").set_value(ttlib::cstr() << size.x << ',' << size.y);
            }
            else
            {
                if (auto bundle = wxGetApp().GetPropertyImageBundle(parts); bundle)
                {
                    ttlib::cstr names;
                    for (auto& file: bundle->lst_filenames)
                    {
                        if (names.size())
                        {
                            names << ';';
                        }
                        names << file;
                    }
                    object.append_child(prop_pair.xrc_name).text().set(names);
                }
            }
        }
    }

    if (node->HasValue(prop_position))
    {
        object.append_child("bitmapposition").text().set(node->prop_as_string(prop_position));
    }

    if (node->HasValue(prop_margins))
    {
        object.append_child("margins").text().set(node->prop_as_string(prop_margins));
    }
}

void GenXrcObjectAttributes(Node* node, pugi::xml_node& object, std::string_view xrc_class)
{
    object.append_attribute("class").set_value(xrc_class);

    // A non default ID takes precedence, followed by a variable name and finally a classname.
    // Note that forms can use either an ID or a class name.

    if (node->HasProp(prop_id) && node->prop_as_string(prop_id) != "wxID_ANY")
        object.append_attribute("name").set_value(node->prop_as_string(prop_id));
    else if (node->HasProp(prop_var_name))
        object.append_attribute("name").set_value(node->prop_as_string(prop_var_name));
    else
        object.append_attribute("name").set_value(node->prop_as_string(prop_class_name));
}

pugi::xml_node InitializeXrcObject(Node* node, pugi::xml_node& object)
{
    if (node->GetParent()->IsSizer() || node->GetParent()->isGen(gen_wxStaticBox))
    {
        GenXrcSizerItem(node, object);
        return object.append_child("object");
    }
    else
    {
        return object;
    }
}
