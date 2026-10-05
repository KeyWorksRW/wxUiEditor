/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [10-03-2026]

#include <wx/menu.h>      // wxMenu and wxMenuBar classes
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statline.h>  // wxStaticLine class interface
#include <wx/stattext.h>  // wxStaticText base header

#include "gen_common.h"                  // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"               // Common XRC generating functions
#include "mockup_parent.h"               // Top-level MockUp Parent window
#include "node.h"                        // Node class
#include "node_creator.h"                // NodeCreator -- NodeCreator class
#include "utils.h"                       // Utility functions that work with properties
#include "write_code.h"                  // Write code to Scintilla or file
#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "menu_widgets.h"

//////////////////////////////////////////  MenuBarBase  //////////////////////////////////////////

wxObject* MenuBarBase::CreateMockup(Node* node, wxObject* parent)
{
    // A real wxMenubar requires a frame window, which we don't have in the Mockup panel.
    // Instead, we create a panel with static text for each top level menu. If the user clicks
    // on one of the static text controls, then we locate which child menu node contains that
    // label and then create a Popup menu to display it.

    auto* panel = new wxPanel(wxStaticCast(parent, wxWindow));
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);

    if (node->is_Gen(gen_PopupMenu))
    {
        auto* label = new wxStaticText(panel, wxID_ANY, node->as_wxString(prop_class_name));
        sizer->Add(label, wxSizerFlags().Border(wxALL));
        label->Bind(wxEVT_LEFT_DOWN, &MenuBarBase::OnLeftMenuClick, this);
    }
    else
    {
        for (const auto& child: node->get_ChildNodePtrs())
        {
            wxString label;
            if (child->as_string(prop_stock_id) != "none")
            {
                label = wxGetStockLabel(
                    NodeCreation.get_ConstantAsInt(child->as_string(prop_stock_id)));
            }
            else
            {
                label = child->as_wxString(prop_label);
            }
            auto* menu_name = new wxStaticText(panel, wxID_ANY, label);
            sizer->Add(menu_name, wxSizerFlags().Border(wxALL));
            menu_name->Bind(wxEVT_LEFT_DOWN, &MenuBarBase::OnLeftMenuClick, this);
        }
    }

    panel->SetSizerAndFit(sizer);

    m_node_menubar = node;
    return panel;
}

void MenuBarBase::OnLeftMenuClick(wxMouseEvent& event)
{
    // To simulate what a real wxMenuBar would do, we get the label from the static text
    // control, find the matching child, and create a popup menu based on that child.

    const wxStaticText* menu_label = wxStaticCast(event.GetEventObject(), wxStaticText);
    const wxue::string text = menu_label->GetLabel().utf8_string();

    Node* menu_node = nullptr;

    if (m_node_menubar->is_Gen(gen_PopupMenu))
    {
        menu_node = m_node_menubar;
    }
    else
    {
        for (const auto& child: m_node_menubar->get_ChildNodePtrs())
        {
            wxString label;
            if (child->as_string(prop_stock_id) != "none")
            {
                label = wxGetStockLabel(
                    NodeCreation.get_ConstantAsInt(child->as_string(prop_stock_id)));
            }
            else
            {
                label = child->as_wxString(prop_label);
            }
            if (label.utf8_string() == text)
            {
                menu_node = child.get();
                break;
            }
        }
    }
    ASSERT_MSG(menu_node, "menu label and static text label don't match!");

    if (!menu_node)
    {
        return;
    }

    wxMenu* popup_menu = MakeSubMenu(menu_node);
    // wxWindow::PopupMenu runs a modal loop and does not return until the menu has been
    // dismissed, so it is safe to delete popup_menu immediately after the call returns.
    getMockup()->PopupMenu(popup_menu);
    delete popup_menu;
}

wxMenu* MenuBarBase::MakeSubMenu(Node* menu_node)
{
    auto* sub_menu = new wxMenu;

    for (const auto& menu_item: menu_node->get_ChildNodePtrs())
    {
        if (menu_item->is_Type(type_submenu))
        {
            wxMenu* result = MakeSubMenu(menu_item.get());
            wxMenuItem* item = sub_menu->AppendSubMenu(result, menu_item->as_wxString(prop_label));
            if (menu_item->as_bool(prop_disabled))
            {
                item->Enable(false);
            }
            if (menu_item->HasValue(prop_bitmap))
            {
                item->SetBitmap(menu_item->as_wxBitmapBundle(prop_bitmap));
            }
        }
        else if (menu_item->is_Gen(gen_separator))
        {
            sub_menu->AppendSeparator();
        }
        else
        {
            wxue::string menu_label = menu_item->as_string(prop_label);
            const wxue::string shortcut = menu_item->as_string(prop_shortcut);
            if (!shortcut.empty())
            {
                menu_label << "\t" << shortcut;
            }

            // If the user specified a stock ID, then we need to use that id in order to have
            // wxWidgets generate the label and bitmap.

            int menu_id = wxID_ANY;
            if (menu_item->as_string(prop_id) != "wxID_ANY" &&
                menu_item->as_string(prop_id).starts_with("wxID_"))
            {
                menu_id = NodeCreation.get_ConstantAsInt(menu_item->as_string(prop_id), wxID_ANY);
            }

            // node->as_int() resolves option-typed properties via
            // NodeCreation.get_ConstantAsInt(), and prop_kind is type_option, so
            // wxITEM_CHECK/wxITEM_RADIO map to their integer values here.
            auto* item =
                new wxMenuItem(sub_menu, menu_id, menu_label, menu_item->as_wxString(prop_help),
                               (wxItemKind) menu_item->as_int(prop_kind));

            if (menu_item->HasValue(prop_bitmap))
#if !defined(__WXMSW__)
            {
                item->SetBitmap(menu_item->as_wxBitmapBundle(prop_bitmap));
            }
#else  // defined(__WXMSW__)
            {
                if (menu_item->HasValue(prop_unchecked_bitmap))
                {
                    const wxBitmapBundle unchecked =
                        menu_item->as_wxBitmapBundle(prop_unchecked_bitmap);
                    item->SetBitmaps(menu_item->as_wxBitmapBundle(prop_bitmap), unchecked);
                }
                else
                {
                    item->SetBitmap(menu_item->as_wxBitmapBundle(prop_bitmap));
                }
            }
            else
            {
                if (menu_item->HasValue(prop_unchecked_bitmap))
                {
                    item->SetBitmaps(wxNullBitmap,
                                     menu_item->as_wxBitmapBundle(prop_unchecked_bitmap));
                }
            }
#endif

            sub_menu->Append(item);

            if (item->GetKind() == wxITEM_CHECK && menu_item->as_bool(prop_checked))
            {
                item->Check(true);
            }

            if (menu_item->as_bool(prop_disabled))
            {
                item->Enable(false);
            }
        }
    }

    return sub_menu;
}

///////////////////////////////////////  MenuBarGenerator ////////////////////////////////////////

bool MenuBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    if (code.HasValue(prop_style))
    {
        code.Add(code.node()->as_string(prop_style));
    }
    code.EndFunction();

    return true;
}

bool MenuBarGenerator::AfterChildrenCode(Code& code)
{
    code.Eol().FormFunction("SetMenuBar(").NodeName().EndFunction();
    return true;
}

bool MenuBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                   std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_menu.cpp
// ../../../wxWidgets/src/xrc/xh_menu.cpp

int MenuBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    const int result = (node->get_Parent() && node->get_Parent()->is_Sizer()) ?
                           BaseGenerator::xrc_sizer_item_created :
                           BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxMenuBar");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void MenuBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxMenuBarXmlHandler");
}

//////////////////////////////////////////  MenuBarFormGenerator
/////////////////////////////////////////////

bool MenuBarFormGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code.Str("(long style) : ");
        if (code.HasValue(prop_subclass))
        {
            code.as_string(prop_subclass);
        }
        else
        {
            code.Str("wxMenuBar");
        }
        code.Str("(style)\n{");
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.MenuBar):\n");
        code.Eol().Tab().Add("def __init__(self, style=").Style();
        code.Str("):");
        code.Indent(3);
        code.Eol() += "wx.MenuBar.__init__(self, style)";
        code.ResetIndent();
    }

    return true;
}

bool MenuBarFormGenerator::HeaderCode(Code& code)
{
    code.NodeName().Str("(long style = ").Style().EndFunction();

    return true;
}

bool MenuBarFormGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += "wxMenuBar";
    }

    return true;
}

bool MenuBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                       std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_menu.cpp
// ../../../wxWidgets/src/xrc/xh_menu.cpp

int MenuBarFormGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    const int result = (node->get_Parent() && node->get_Parent()->is_Sizer()) ?
                           BaseGenerator::xrc_sizer_item_created :
                           BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxMenuBar");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void MenuBarFormGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxMenuBarXmlHandler");
}

//////////////////////////////////////////  PopupMenuGenerator
/////////////////////////////////////////////

bool PopupMenuGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.as_string(prop_class_name).Str("::").as_string(prop_class_name);
        code.Str("() : ");
        if (code.HasValue(prop_subclass))
        {
            code.as_string(prop_subclass);
        }
        else
        {
            code.Str("wxMenu");
        }
        code.Str("()\n{");
    }
    else
    {
        code.Add("class ").NodeName().Add("(wx.Menu):\n");
        code.Eol().Tab().Add("def __init__(self");
        code.Str("):");
        code.Indent(3);
        code.Eol() += "wx.Menu.__init__(self)";
        code.ResetIndent();
    }

    return true;
}

bool PopupMenuGenerator::HeaderCode(Code& code)
{
    code.NodeName().Str("(").EndFunction();

    return true;
}

bool PopupMenuGenerator::BaseClassNameCode(Code& code)
{
    if (code.HasValue(prop_subclass))
    {
        code.as_string(prop_subclass);
    }
    else
    {
        code += "wxMenu";
    }

    return true;
}

bool PopupMenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  SeparatorGenerator
/////////////////////////////////////////////

bool SeparatorGenerator::ConstructionCode(Code& code)
{
    if (code.node()->get_Parent()->is_Gen(gen_PopupMenu))
    {
        code.FormFunction("AppendSeparator(").EndFunction();
    }
    else
    {
        code.ParentName().Function("AppendSeparator(").EndFunction();
    }

    return true;
}

bool SeparatorGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                     std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

int SeparatorGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "separator");
    return BaseGenerator::xrc_updated;
}
