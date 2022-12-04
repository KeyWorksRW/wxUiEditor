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

std::optional<ttlib::cstr> PopupWinGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, int style) : wxPopupTransientWindow(parent, style)\n{";

    return code;
}

std::optional<ttlib::cstr> PopupWinGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_header)
    {
        code << node->get_node_name() << "(wxWindow* parent, int style = " << node->prop_as_string(prop_border);
        if (node->HasValue(prop_style))
        {
            code << " | " << node->prop_as_string(prop_style);
        }
        code << ");";
        return code;
    }
    else if (cmd == code_base_class)
        return GenFormCode(cmd, node);

    return {};
}

std::optional<ttlib::cstr> PopupWinGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    auto code = GenFontColourSettings(node);

    if (code.empty())
    {
        return {};
    }
    else
    {
        return code;
    }
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
