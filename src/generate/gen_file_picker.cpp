/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFilePickerCtrl generator
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

#include "gen_file_picker.h"

wxObject* FilePickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxString msg;
    if (node->HasValue(prop_message))
    {
        msg = std::move(node->prop_as_wxString(prop_message));
    }
    else
    {
        msg = wxFileSelectorPromptStr;
    }
    wxString wildcard;
    if (node->HasValue(prop_wildcard))
    {
        wildcard = std::move(node->prop_as_wxString(prop_wildcard));
    }
    else
    {
        wildcard = wxFileSelectorDefaultWildcardStr;
    }

    auto widget = new wxFilePickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_initial_path),
                                       msg, wildcard, DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size),
                                       GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::sview> FilePickerGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();

    if (auto& path = code.node()->prop_as_string(prop_initial_path); path.size())
    {
        code.QuotedString(path);
    }
    else
    {
        code.Add("wxEmptyString");
    }

    code.Comma();
    if (auto& msg = code.node()->prop_as_string(prop_message); msg.size())
    {
        code.QuotedString(msg);
    }
    else
    {
        code.Add("wxFileSelectorPromptStr");
    }

    code.Comma();
    if (auto& msg = code.node()->prop_as_string(prop_wildcard); msg.size())
    {
        code.QuotedString(msg);
    }
    else
    {
        code.Add("wxFileSelectorDefaultWildcardStr");
    }

    code.PosSizeFlags(true, "wxFLP_DEFAULT_STYLE");

    return code.m_code;
}

std::optional<ttlib::sview> FilePickerGenerator::CommonSettings(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        code.NodeName().Function("SetFocus(").EndFunction();
    }

    return code.m_code;
}

bool FilePickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/filepicker.h>", set_src, set_hdr);
    return true;
}

std::optional<ttlib::cstr> FilePickerGenerator::GetPropertyDescription(NodeProperty* prop)
{
    if (prop->isProp(prop_message))
    {
        return (
            ttlib::cstr() << "Title bar text for the file picker dialog. If not specified, \"Select a file\" will be used.");
    }
    else
    {
        return {};
    }
}

// ../../wxSnapShot/src/xrc/xh_filepicker.cpp
// ../../../wxWidgets/src/xrc/xh_filepicker.cpp

int FilePickerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxFilePickerCtrl");

    ADD_ITEM_PROP(prop_initial_path, "value")
    ADD_ITEM_PROP(prop_message, "message")
    ADD_ITEM_PROP(prop_wildcard, "wildcard")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void FilePickerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxFilePickerCtrlXmlHandler");
}
