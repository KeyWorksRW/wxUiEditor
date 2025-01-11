/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPanel generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>  // Base header for wxPanel

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_panel.h"

wxObject* PanelGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos), DlgSize(node, prop_size),
                              GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool PanelGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName();
    if (code.IsDefaultPosSizeFlags() && code.IsEqualTo(prop_id, "wxID_ANY"))
    {
        code.EndFunction();
    }
    else
    {
        code.Comma().as_string(prop_id);
        code.PosSizeFlags();
    }
    return true;
}

bool PanelGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                 GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    return true;
}

int PanelGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxPanel");
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void PanelGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxPanelXmlHandler");
}

bool PanelGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports, GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use Wx qw(wxTAB_TRAVERSAL);");
    }
    return false;
}
