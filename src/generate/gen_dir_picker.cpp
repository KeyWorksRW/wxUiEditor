/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDirPickerCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filepicker.h>  // wxFilePickerCtrl, wxDirPickerCtrl base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_dir_picker.h"

wxObject* DirPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxString prompt;
    if (node->HasValue(prop_message))
    {
        prompt = node->prop_as_wxString(prop_message);
    }
    else
    {
        prompt = wxDirSelectorPromptStr;
    }

    auto widget =
        new wxDirPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_initial_path), prompt,
                            DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> DirPickerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    {
        auto& path = node->prop_as_string(prop_initial_path);
        if (path.size())
        {
            code << GenerateQuotedString(path);
        }
        else
        {
            code << "wxEmptyString";
        }
    }

    code << ", ";
    {
        auto& msg = node->prop_as_string(prop_message);
        if (msg.size())
        {
            code << "\n\t\t" << GenerateQuotedString(msg);
        }
        else
        {
            code << " wxDirSelectorPromptStr";
        }
    }

    GeneratePosSizeFlags(node, code, true, "wxDIRP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> DirPickerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
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
std::optional<ttlib::cstr> DirPickerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool DirPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/filepicker.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dirpicker.cpp
// ../../../wxWidgets/src/xrc/xh_dirpicker.cpp

int DirPickerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxDirPickerCtrl");

    ADD_ITEM_PROP(prop_initial_path, "value")
    ADD_ITEM_PROP(prop_message, "message")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void DirPickerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxDirPickerCtrlXmlHandler");
}
