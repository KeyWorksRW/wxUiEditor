/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCalendarCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/calctrl.h>           // date-picker control
#include <wx/generic/calctrlg.h>  // date-picker control

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "code.h"  // Code -- Helper class for generating code

#include "gen_calendar_ctrl.h"

auto CalendarCtrlGenerator::CreateMockup(Node* node, wxObject* parent) -> wxObject*
{
    wxCalendarCtrlBase* widget = nullptr;
    if (node->as_view(prop_subclass).starts_with("wxGeneric"))
    {
        widget = new wxGenericCalendarCtrl(wxStaticCast(parent, wxWindow), wxID_ANY,
                                           wxDefaultDateTime, DlgPoint(node, prop_pos),
                                           DlgSize(node, prop_size), GetStyleInt(node));
    }
    else
    {
        widget = new wxCalendarCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime,
                                    DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                                    GetStyleInt(node));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

auto CalendarCtrlGenerator::ConstructionCode(Code& code) -> bool
{
    bool use_generic_version =
        code.is_cpp() && code.node()->as_string(prop_subclass).starts_with("wxGeneric");
    code.AddAuto().NodeName().CreateClass(use_generic_version);
    code.ValidParentName().Comma().as_string(prop_id).Comma();
    if (code.is_ruby())
    {
        code.Str("DateTime.now");
    }
    else if (code.is_perl())
    {
        code.Str("Wx::DateTime->new()");
    }
    else
    {
        code.Add("wxDefaultDateTime");
    }
    code.PosSizeFlags();

    return true;
}

auto CalendarCtrlGenerator::SettingsCode(Code& code) -> bool
{
    if (code.IsTrue(prop_focus))
    {
        auto* form = code.node()->get_Form();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->is_Gen(gen_wxDialog) && !form->is_Type(type_frame_form))
        {
            code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
        }
    }
    return true;
}

auto CalendarCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                        std::set<std::string>& set_hdr,
                                        GenLang /* language unused */) -> bool
{
    if (node->as_view(prop_subclass).starts_with("wxGeneric"))
    {
        InsertGeneratorInclude(node, "#include <wx/calctrl.h>\n#include <wx/generic/calctrlg.h>",
                               set_src, set_hdr);
    }
    else
    {
        InsertGeneratorInclude(node, "#include <wx/calctrl.h>", set_src, set_hdr);
    }

    return true;
}

auto CalendarCtrlGenerator::GetPythonImports(Node* /*unused*/, std::set<std::string>& set_imports)
    -> bool
{
    set_imports.insert("import wx.adv");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_cald.cpp
// ../../../wxWidgets/src/xrc/xh_cald.cpp

auto CalendarCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
    -> int
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxCalendarCtrl");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void CalendarCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxCalendarCtrlXmlHandler");
}

auto CalendarCtrlGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                       GenLang language) -> bool
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use base qw[Wx::Calendar];");
        set_imports.emplace("use Wx qw[:calendar];");
    }

    return false;
}
