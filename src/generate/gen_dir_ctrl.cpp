/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGenericDirCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dirctrl.h>  // Directory control base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_dir_ctrl.h"

//////////////////////////////////////////  GenericDirCtrlGenerator  //////////////////////////////////////////

wxObject* GenericDirCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxGenericDirCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_defaultfolder),
                                       DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node),
                                       node->prop_as_wxString(prop_filter), node->prop_as_int(prop_defaultfilter));

    widget->ShowHidden(node->prop_as_bool(prop_show_hidden));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> GenericDirCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << '\t';
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->HasValue(prop_defaultfolder))
        code << GenerateQuotedString(node->prop_as_string(prop_defaultfolder));
    else
        code << "wxDirDialogDefaultFolderStr";

    if (!node->HasValue(prop_filter) && node->prop_as_int(prop_defaultfilter) == 0 && !node->HasValue(prop_window_name))
    {
        GeneratePosSizeFlags(node, code, false, "wxDIRCTRL_DEFAULT_STYLE");
    }
    else
    {
        code << ",\n\t\t\t";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", " << GenerateQuotedString(node->prop_as_string(prop_filter)) << ", "
             << node->prop_as_string(prop_defaultfilter);
        if (node->HasValue(prop_window_name))
        {
            code << ", " << node->prop_as_string(prop_window_name);
        }
        code << ");";
    }

    if (node->prop_as_bool(prop_show_hidden))
    {
        code << "\n\t" << node->get_node_name() << "->ShowHidden(true);";
    }

    return code;
}

std::optional<ttlib::cstr> GenericDirCtrlGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
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
std::optional<ttlib::cstr> GenericDirCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool GenericDirCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dirctrl.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_gdctl.cpp
// ../../../wxWidgets/src/xrc/xh_gdctl.cpp

int GenericDirCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxGenericDirCtrl");

    ADD_ITEM_PROP(prop_defaultfolder, "defaultfolder")
    ADD_ITEM_PROP(prop_filter, "filter")
    ADD_ITEM_PROP(prop_defaultfilter, "defaultfilter")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void GenericDirCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxGenericDirCtrlXmlHandler");
}
