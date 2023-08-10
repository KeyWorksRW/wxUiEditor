/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPopupTransientWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/popupwin.h>  // wxBitmapComboBox base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_popup_trans_win.h"

bool PopupWinGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code += "(wxWindow* parent, int style) : wxPopupTransientWindow(parent, style)\n{";
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.PopupTransientWindow):\n");
        code.Tab().Add("def __init__(self, parent):").Eol().Tab(2);
        code << "wx.PopupTransientWindow.__init__(self, parent, flags=";
        code.Add(prop_border);
        if (code.hasValue(prop_style))
        {
            code.Str(" | ").Add(prop_style);
        }
        code.EndFunction();
    }

    return true;
}
bool PopupWinGenerator::SettingsCode(Code& code)
{
    code.GenFontColourSettings();

    return true;
}

bool PopupWinGenerator::HeaderCode(Code& code)
{
    code.NodeName().Str("(wxWindow* parent, int style = ").as_string(prop_border);
    if (code.hasValue(prop_style))
    {
        code.Str(" | ").Add(prop_style);
    }
    code.EndFunction();

    return true;
}

bool PopupWinGenerator::BaseClassNameCode(Code& code)
{
    if (code.hasValue(prop_derived_class))
    {
        code.as_string(prop_derived_class);
    }
    else
    {
        code += code.node()->declName();
    }

    return true;
}

bool PopupWinGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/popupwin.h>", set_src, set_hdr);
    return true;
}

int PopupWinGenerator::GenXrcObject(Node*, pugi::xml_node& object, size_t xrc_flags)
{
    if (xrc_flags & xrc::add_comments)
    {
        object.append_child(pugi::node_comment).set_value(" wxPopupTransientWindow is not supported by XRC. ");
    }
    return BaseGenerator::xrc_form_not_supported;
}
