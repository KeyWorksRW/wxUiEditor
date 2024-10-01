/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCalendarCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/calctrl.h>  // date-picker control

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "code.h"  // Code -- Helper class for generating code

#include "gen_calendar_ctrl.h"

//////////////////////////////////////////  CalendarCtrlGenerator  //////////////////////////////////////////

wxObject* CalendarCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxCalendarCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime, DlgPoint(node, prop_pos),
                                     DlgSize(node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool CalendarCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();
    if (code.is_ruby())
    {
        code.Str("DateTime.now");
    }
    else
    {
        code.Add("wxDefaultDateTime");
    }
    code.PosSizeFlags();

    return true;
}

bool CalendarCtrlGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
        {
            code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
        }
    }
    return true;
}

bool CalendarCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                        GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/calctrl.h>", set_src, set_hdr);
    return true;
}

bool CalendarCtrlGenerator::GetPythonImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("import wx.adv");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_cald.cpp
// ../../../wxWidgets/src/xrc/xh_cald.cpp

int CalendarCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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
