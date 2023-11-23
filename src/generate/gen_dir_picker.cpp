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
    if (node->hasValue(prop_message))
    {
        prompt = node->as_wxString(prop_message);
    }
    else
    {
        prompt = wxDirSelectorPromptStr;
    }

    auto widget = new wxDirPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_initial_path), prompt,
                                      DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool DirPickerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();

    if (auto& path = code.node()->as_string(prop_initial_path); path.size())
    {
        code.QuotedString(path);
    }
    else
    {
        code.Add("wxEmptyString");
    }

    code.Comma();
    if (auto& msg = code.node()->as_string(prop_message); msg.size())
    {
        code.QuotedString(msg);
    }
    else
    {
        if (code.is_ruby())
            code.Str("Wx::DIR_SELECTOR_PROMPT_STR");
        else
            code.Add("wxDirSelectorPromptStr");
    }

    code.PosSizeFlags(false, "wxDIRP_DEFAULT_STYLE");

    return true;
}

bool DirPickerGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
        {
            code.NodeName().Function("SetFocus(").EndFunction();
        }
    }

    return true;
}

bool DirPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                     int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/filepicker.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dirpicker.cpp
// ../../../wxWidgets/src/xrc/xh_dirpicker.cpp

int DirPickerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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
