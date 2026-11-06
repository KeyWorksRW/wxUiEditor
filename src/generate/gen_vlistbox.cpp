/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxVListBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dc.h>     // wxDC class (needed by the mockup drawing code)
#include <wx/vlbox.h>  // wxVListBox class interface

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_vlistbox.h"

namespace
{

    // wxVListBox is abstract -- XRC requires a subclass. This concrete class is only used to draw
    // a preview in the Mockup panel.
    class MockupVListBox : public wxVListBox
    {
    public:
        using wxVListBox::wxVListBox;

    protected:
        void OnDrawItem(wxDC& draw_dc, const wxRect& rect, size_t item_index) const override
        {
            draw_dc.DrawText(wxString::Format("Item %u", static_cast<unsigned>(item_index) + 1),
                             rect.x + 4, rect.y + 4);
        }

        wxCoord OnMeasureItem([[maybe_unused]] size_t item_index) const override { return 20; }
    };

}  // namespace

wxObject* VListBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget =
        new MockupVListBox(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(node, prop_pos),
                           DlgSize(node, prop_size), GetStyleInt(node));

    widget->SetItemCount(3);
    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool VListBoxGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags(code::allow_scaling, false, "wxTAB_TRAVERSAL");

    return true;
}

bool VListBoxGenerator::SettingsCode(Code& code)
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

bool VListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/vlbox.h>", set_src, set_hdr);
    return true;
}

int VListBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxVListBox");

    // Attributes have to be added before any child nodes, and wxVListBox absolutely requires a
    // subclass since the XRC handler cannot instantiate an abstract class.
    if (node->HasValue(prop_subclass))
    {
        item.append_attribute("subclass").set_value(node->as_string(prop_subclass));
    }
    else if (xrc_flags & xrc::add_comments)
    {
        item.append_child(pugi::node_comment)
            .set_value("wxVListBox is an abstract class -- XRC requires a subclass attribute, "
                       "otherwise this object cannot be created");
    }

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void VListBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxVListBoxXmlHandler");
}
