/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDirPickerCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filepicker.h>  // wxFilePickerCtrl, wxDirPickerCtrl base header

#include "gen_dir_picker.h"

#include "gen_common.h"                  // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"               // Common XRC generating functions
#include "node.h"                        // Node class
#include "pugixml.hpp"                   // xml read/write/create/process
#include "utils.h"                       // Utility functions that work with properties
#include "wxue_namespace/wxue_string.h"  // wxue::string, wxue::string_view

wxObject* DirPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxString prompt;
    if (node->HasValue(prop_message))
    {
        prompt = node->as_wxString(prop_message);
    }
    else
    {
        prompt = wxDirSelectorPromptStr;
    }

    auto* widget = new wxDirPickerCtrl(
        wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_initial_path), prompt,
        DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool DirPickerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();

    if (const auto& path = code.node()->as_string(prop_initial_path); path.size())
    {
        code.QuotedString(path);
    }
    else
    {
        code.Add("wxEmptyString");
    }

    code.Comma();
    if (const auto& msg = code.node()->as_string(prop_message); msg.size())
    {
        code.QuotedString(msg);
    }
    else
    {
        if (code.is_ruby())
        {
            code.Str("Wx::DIR_SELECTOR_PROMPT_STR");
        }
        else if (code.is_perl())
        {
            code.QuotedString(wxue::string_view("Select a directory"));
        }
        else
        {
            code.Add("wxDirSelectorPromptStr");
        }
    }

    code.PosSizeFlags(code::allow_scaling, false, "wxDIRP_DEFAULT_STYLE");

    return true;
}

bool DirPickerGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        auto* form = code.node()->get_Form();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->is_Gen(gen_wxDialog) && !form->is_Type(type_frame_form))
        {
            code.NodeName().Function("SetFocus(").EndFunction();
        }
    }

    return true;
}

bool DirPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/filepicker.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_dirpicker.cpp
// ../../../wxWidgets/src/xrc/xh_dirpicker.cpp

int DirPickerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
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

bool DirPickerGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                    GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace(
            "use Wx qw(wxDIRP_DEFAULT_STYLE wxDIRP_USE_TEXTCTRL wxDIRP_DIR_MUST_EXIST\n"
            "          wxDIRP_CHANGE_DIR wxDIRP_SMALL);");

        return true;
    }
    return false;
}
