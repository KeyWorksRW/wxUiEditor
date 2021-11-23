/////////////////////////////////////////////////////////////////////////////
// Purpose:   resCtrl class image/icon functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "winres_ctrl.h"

#include "import_winres.h"  // WinResource -- Parse a Windows resource file
#include "node_creator.h"   // NodeCreator -- Class used to create nodes

static std::map<int, const char*> map_win_stock_cursors = {

    { 32512, "IDC_ARROW" },        // Standard arrow cursor.
    { 32513, "IDC_IBEAM" },        // I-beam cursor.
    { 32514, "IDC_WAIT" },         // Hourglass cursor.
    { 32515, "IDC_CROSS" },        // Crosshair cursor.
    { 32516, "IDC_UPARROW" },      // Vertical arrow cursor.
    { 32642, "IDC_SIZENWSE" },     // Double-pointed arrow cursor pointing northwest and southeast.
    { 32643, "IDC_SIZENESW" },     // Double-pointed arrow cursor pointing northeast and southwest.
    { 32644, "IDC_SIZEWE" },       // Double-pointed arrow cursor pointing west and east.
    { 32645, "IDC_SIZENS" },       // Double-pointed arrow cursor pointing north and south.
    { 32646, "IDC_SIZEALL" },      // Four-pointed arrow cursor pointing north, south, east, and west.
    { 32648, "IDC_NO" },           // Slashed circle cursor.
    { 32649, "IDC_HAND" },         // Hand cursor.
    { 32650, "IDC_APPSTARTING" },  // Standard arrow and small hourglass cursor.
    { 32651, "IDC_HELP" },         // Arrow and question mark cursor.

};

// Note that the first 5 numbers are identical to the map_win_stock_cursors numbers, even
// though the images are different.
static std::map<int, const char*> map_win_stock_icons = {

    { 32512, "IDI_APPLICATION" },  // Application icon.
    { 32513, "IDI_HAND" },         // Stop sign icon.
    { 32514, "IDI_QUESTION" },     // Question-mark icon.
    { 32515, "IDI_EXCLAMATION" },  // Exclamation point icon.
    { 32516, "IDI_ASTERISK" },     // Asterisk icon.
    { 32517, "IDI_WINLOGO" },

};

// clang-format off

static std::map<std::string, const char*> map_win_wx_stock = {

    {"IDI_EXCLAMATION", "wxART_INFORMATION"},
    {"IDI_HAND", "wxART_ERROR"},
    {"IDI_QUESTION", "wxART_HELP"},

};

// clang-format on

void resCtrl::ParseIconControl(ttlib::cview line)
{
    line.moveto_nextword();

    ttlib::cstr icon_name;

    // Unlike a normal text parameter, for the ICON directive it might or might not be in quotes.
    if (line.at(0) == '"')
    {
        line = StepOverQuote(line, icon_name);
    }
    else
    {
        auto pos_comma = line.find(',');
        if (!ttlib::is_found(pos_comma))
        {
            MSG_ERROR(ttlib::cstr() << "Missing comma after control text :" << m_original_line);
            return;
        }
        icon_name = line.subview(0, pos_comma);
        line.remove_prefix(pos_comma);

        if (ttlib::is_digit(icon_name[0]))
        {
            if (auto icon = map_win_stock_icons.find(ttlib::atoi(icon_name)); icon != map_win_stock_icons.end())
            {
                icon_name = icon->second;
            }
            else if (auto cursor = map_win_stock_cursors.find(ttlib::atoi(icon_name)); cursor != map_win_stock_cursors.end())
            {
                icon_name = cursor->second;
            }
        }
    }

    if (auto stock_image = map_win_wx_stock.find(icon_name); stock_image != map_win_wx_stock.end())
    {
        ttlib::cstr prop;
        prop << "Art; " << stock_image->second << "|wxART_TOOLBAR; [-1; -1]";
        m_node = g_NodeCreator.NewNode(gen_wxStaticBitmap);
        m_node->prop_set_value(prop_bitmap, prop);
    }
    else
    {
        if (icon_name.empty())
        {
            m_node = g_NodeCreator.NewNode(gen_wxStaticBitmap);
        }
        else
        {
            auto result = m_pWinResource->FindIcon(icon_name);
            if (!result)
            {
                MSG_ERROR(ttlib::cstr() << "Icon not found :" << m_original_line);
                return;
            }

            m_node = g_NodeCreator.NewNode(gen_wxStaticBitmap);
            ttlib::cstr prop;
            prop << "Embed;" << result.value() << ";[-1; -1]";

            // Note that this sets up the filename to convert, but doesn't actually do the conversion -- that will require
            // the code to be generated.
            m_node->prop_set_value(prop_bitmap, prop);
        }
    }
    line = GetID(line);
    ParseDimensions(line, m_du_rect, m_pixel_rect);
}

// Similar to ParseIconControl only in this case line is pointing to the id, and the Node
// has already been created.
//
// Works with either SS_BITMAP or SS_ICON.
void resCtrl::ParseImageControl(ttlib::cview line)
{
    ttlib::cstr image_name;

    // Unlike a normal text parameter, for the ICON directive it might or might not be in quotes.
    if (line.at(0) == '"')
    {
        line = StepOverQuote(line, image_name);
    }
    else
    {
        auto pos_comma = line.find(',');
        if (!ttlib::is_found(pos_comma))
        {
            MSG_ERROR(ttlib::cstr() << "Missing comma after control text :" << m_original_line);
            return;
        }
        image_name = line.subview(0, pos_comma);
        line.remove_prefix(pos_comma);

        if (ttlib::is_digit(image_name[0]))
        {
            if (auto icon = map_win_stock_icons.find(ttlib::atoi(image_name)); icon != map_win_stock_icons.end())
            {
                image_name = icon->second;
            }
            else if (auto cursor = map_win_stock_cursors.find(ttlib::atoi(image_name));
                     cursor != map_win_stock_cursors.end())
            {
                image_name = cursor->second;
            }
        }
    }

    if (auto stock_image = map_win_wx_stock.find(image_name); stock_image != map_win_wx_stock.end())
    {
        ttlib::cstr prop;
        prop << "Art; " << stock_image->second << "; wxART_TOOLBAR; [-1; -1]";
        m_node->prop_set_value(prop_bitmap, prop);
    }
    else
    {
        ttlib::cstr final_name;
        std::optional<ttlib::cstr> result;

        if (line.contains("SS_ICON"))
        {
            result = m_pWinResource->FindIcon(image_name);
            if (!result)
            {
                MSG_ERROR(ttlib::cstr() << "Image not found :" << m_original_line);
                return;
            }
            final_name = result.value();
        }
        else
        {
            result = m_pWinResource->FindBitmap(image_name);

            /*
                Visual Studio (as if version 16.09) won't necessarily use the correct name if and ICON and BITMAP resource
                both have the same numerical value. The resource compiler will convert the id name to it's value, and get
                the correct bitmap, but we don't have that capability.

            */

            if (result)
            {
                final_name = result.value();
            }
            else
            {
                MSG_ERROR(ttlib::cstr() << "Image not found :" << m_original_line);
            }
        }

        if (final_name.size())
        {
            final_name.remove_extension();
            if (line.contains("SS_ICON"))
                final_name << "_ico.h";
            else
                final_name << "_png.h";
            ttlib::cstr prop;
            prop << "Header; " << final_name << "; " << result.value() << "; [-1; -1]";

            // Note that this sets up the filename to convert, but doesn't actually do the conversion -- that will require
            // the code to be generated.
            m_node->prop_set_value(prop_bitmap, prop);
        }
    }

    line = GetID(line);

    if (line.empty())
    {
        MSG_ERROR(ttlib::cstr() << "Missing class :" << m_original_line);
        return;
    }

    // This should be the class
    if (line.size() && line.at(0) == '"')
    {
        auto pos_comma = line.find(',');
        if (!ttlib::is_found(pos_comma))
        {
            MSG_ERROR(ttlib::cstr() << "Missing style after class :" << m_original_line);
            return;
        }
        // Now step over the style
        pos_comma = line.find(',');
        if (!ttlib::is_found(pos_comma))
        {
            MSG_ERROR(ttlib::cstr() << "Missing dimension after style :" << m_original_line);
            return;
        }
        line.remove_prefix(pos_comma);
    }
    else
    {
        MSG_ERROR(ttlib::cstr() << "Missing class :" << m_original_line);
    }

    ParseDimensions(line, m_du_rect, m_pixel_rect);
}
