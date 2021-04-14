/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "import_xml.h"

#include "node.h"     // Node class
#include "uifuncs.h"  // Miscellaneous functions for displaying UI

std::optional<pugi::xml_document> ImportXML::LoadDocFile(const ttString& file)
{
    pugi::xml_document doc;

    if (auto result = doc.load_file(file.wx_str()); !result)
    {
        appMsgBox(_ttc(strIdCantOpen) << file.wx_str() << "\n\n" << result.description(), _tt(strIdImportFormBuilder));
        return {};
    }

    m_importProjectFile = file;

    return doc;
}

void ImportXML::HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent)
{
    auto flag_value = xml_prop.text().as_cview();

    ttlib::cstr border_value;
    if (flag_value.contains("wxALL"))
        border_value = "wxALL";
    else
    {
        if (flag_value.contains("wxLEFT"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxLEFT";
        }
        if (flag_value.contains("wxRIGHT"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxRIGHT";
        }
        if (flag_value.contains("wxTOP"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxTOP";
        }
        if (flag_value.contains("wxBOTTOM"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxBOTTOM";
        }
    }

    if (border_value.size())
    {
        node->prop_set_value(prop_borders, border_value);
    }

    bool is_VerticalSizer = false;
    bool is_HorizontalSizer = false;

    if (parent && parent->IsSizer())
    {
        if (parent->prop_as_string(prop_orientation).contains("wxVERTICAL"))
            is_VerticalSizer = true;
        if (parent->prop_as_string(prop_orientation).contains("wxHORIZONTAL"))
            is_HorizontalSizer = true;
    }

    ttlib::cstr align_value;
    if (flag_value.contains("wxALIGN_LEFT") && !is_HorizontalSizer)
    {
        align_value << "wxALIGN_LEFT";
    }
    if (flag_value.contains("wxALIGN_TOP") && !is_VerticalSizer)
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_TOP";
    }
    if (flag_value.contains("wxALIGN_RIGHT") && !is_HorizontalSizer)
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_RIGHT";
    }
    if (flag_value.contains("wxALIGN_BOTTOM") && !is_VerticalSizer)
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_BOTTOM";
    }

    if (flag_value.contains("wxALIGN_CENTER") || flag_value.contains("wxALIGN_CENTRE"))
    {
        if (flag_value.contains("wxALIGN_CENTER_VERTICAL") || flag_value.contains("wxALIGN_CENTRE_VERTICAL"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER_VERTICAL";
        }
        else if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") || flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER_HORIZONTAL";
        }
        if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") || flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER_HORIZONTAL";
        }

        // Because we use contains(), all we know is that a CENTER flag was used, but not which one.
        // If we get here and no CENTER flag has been added, then assume that "wxALIGN_CENTER" or
        // "wxALIGN_CENTRE" was specified.

        if (!align_value.contains("wxALIGN_CENTER"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER";
        }
    }
    if (align_value.size())
    {
        auto prop = node->get_prop_ptr(prop_alignment);
        prop->set_value(align_value);
    }

    ttlib::cstr flags_value;
    if (flag_value.contains("wxEXPAND") || flag_value.contains("wxGROW"))
    {
        // Only add the flag if the expansion will happen in at least one of the directions.

        if (!(flag_value.contains("wxALIGN_BOTTOM") || flag_value.contains("wxALIGN_CENTER_VERTICAL")) &&
            !(flag_value.contains("wxALIGN_RIGHT") || flag_value.contains("wxALIGN_CENTER_HORIZONTAL")))
            flags_value << "wxEXPAND";
    }
    if (flag_value.contains("wxSHAPED"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxSHAPED";
    }
    if (flag_value.contains("wxFIXED_MINSIZE"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxFIXED_MINSIZE";
    }
    if (flag_value.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxRESERVE_SPACE_EVEN_IF_HIDDEN";
    }
    if (flag_value.contains("wxTILE"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxSHAPED|wxFIXED_MINSIZE";
    }
    if (flags_value.size())
    {
        auto prop = node->get_prop_ptr(prop_flags);
        prop->set_value(flags_value);
    }
}
