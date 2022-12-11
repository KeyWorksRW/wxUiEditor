/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFileCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filectrl.h>  // Header for wxFileCtrlBase and other common functions used by

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_file_ctrl.h"

wxObject* FileCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxString wild;
    if (node->HasValue(prop_wildcard))
        wild = node->prop_as_wxString(prop_wildcard);
    else
        wild = wxFileSelectorDefaultWildcardStr;

    auto widget = new wxFileCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_initial_folder),
                                 node->prop_as_wxString(prop_initial_filename), wild, GetStyleInt(node),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size));

    if (!(node->prop_as_int(prop_style) & wxFC_NOSHOWHIDDEN))
        widget->ShowHidden(node->prop_as_bool(prop_show_hidden));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> FileCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << "\t";
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->HasValue(prop_initial_folder))
        code << GenerateQuotedString(node->prop_as_string(prop_initial_folder));
    else
        code << "wxEmptyString";
    code << ", ";

    if (node->HasValue(prop_initial_filename))
        code << GenerateQuotedString(node->prop_as_string(prop_initial_filename));
    else
        code << "wxEmptyString";
    code << ", ";

    if (node->HasValue(prop_wildcard))
        code << GenerateQuotedString(node->prop_as_string(prop_wildcard));
    else
        code << "wxFileSelectorDefaultWildcardStr";
    code << ", ";

    GenStyle(node, code);

    code << ",\n\t\t\t";
    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    if (node->HasValue(prop_window_name))
    {
        code << ", " << node->prop_as_string(prop_window_name);
    }
    code << ");";

    if (node->prop_as_bool(prop_show_hidden))
    {
        code << "\n" << node->get_node_name() << "->ShowHidden(true);";
    }

    return code;
}

std::optional<ttlib::cstr> FileCtrlGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (code.size())
        return code;
    else

        return {};
}

bool FileCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/filectrl.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_filectrl.cpp
// ../../../wxWidgets/src/xrc/xh_filectrl.cpp

int FileCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxFileCtrl");

    ADD_ITEM_PROP(prop_initial_folder, "defaultdirectory")
    ADD_ITEM_PROP(prop_initial_filename, "defaultfilename")
    ADD_ITEM_PROP(prop_wildcard, "wildcard")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->prop_as_int(prop_filter_index) > 0)
            ADD_ITEM_COMMENT("XRC does not support calling SetFilterIndex()")
        if (node->prop_as_bool(prop_show_hidden))
            ADD_ITEM_COMMENT("XRC does not support calling ShowHidden()")
        GenXrcComments(node, item);
    }

    return result;
}

void FileCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxFileCtrlXmlHandler");
}
