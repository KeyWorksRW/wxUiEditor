/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/menu.h>      // wxMenu and wxMenuBar classes
#include <wx/statline.h>  // wxStaticLine class interface
#include <wx/stattext.h>  // wxStaticText base header

#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- NodeCreator class

#include "menu_widgets.h"

//////////////////////////////////////////  MenuBarBase  //////////////////////////////////////////

wxObject* MenuBarBase::Create(Node* node, wxObject* parent)
{
    // A real wxMenubar requires a frame window, which we don't have in the Mockup panel. Instead, we create a panel
    // with static text for each top level menu. If the user clicks on one of the static text controls, then we locate
    // which child menu node contains that label and then create a Popup menu to display it.

    auto panel = new wxPanel(wxStaticCast(parent, wxWindow));
    auto sizer = new wxBoxSizer(wxHORIZONTAL);

    for (auto& iter: node->GetChildNodePtrs())
    {
        auto label = new wxStaticText(panel, wxID_ANY, iter->prop_as_wxString(txt_label));
        sizer->Add(label, wxSizerFlags().Border(wxALL));
        label->Bind(wxEVT_LEFT_DOWN, &MenuBarBase::OnLeftMenuClick, this);
    }

    panel->SetSizerAndFit(sizer);

    m_node_menubar = node;
    return panel;
}

void MenuBarBase::OnLeftMenuClick(wxMouseEvent& event)
{
    // To simulate what a real wxMenuBar would do, we get the label from the static text control, find the matching
    // child, and create a popup menu based on that child.

    auto menu_label = wxStaticCast(event.GetEventObject(), wxStaticText);
    ttlib::cstr text = menu_label->GetLabel().utf8_str().data();

    Node* menu_node = nullptr;
    for (size_t pos_menu = 0; pos_menu < m_node_menubar->GetChildCount(); ++pos_menu)
    {
        if (m_node_menubar->GetChild(pos_menu)->prop_as_string(txt_label) == text)
        {
            menu_node = m_node_menubar->GetChild(pos_menu);
            break;
        }
    }
    ASSERT_MSG(menu_node, "menu label and static text label don't match!");

    if (!menu_node)
        return;

    auto popup = MakeSubMenu(menu_node);
    GetMockup()->PopupMenu(popup);
    delete popup;
}

wxMenu* MenuBarBase::MakeSubMenu(Node* menu_node)
{
    auto sub_menu = new wxMenu;

    for (auto& menu_item: menu_node->GetChildNodePtrs())
    {
        if (menu_item->isType(type_submenu))
        {
            auto result = MakeSubMenu(menu_item.get());
            auto item = sub_menu->AppendSubMenu(result, menu_item->GetPropertyAsString(txt_label));
            if (menu_item->HasValue("bitmap"))
                item->SetBitmap(menu_item->prop_as_wxBitmap("bitmap"));
        }
        else if (menu_item->GetClassName() == "separator")
        {
            sub_menu->AppendSeparator();
        }
        else
        {
            auto menu_label = menu_item->prop_as_string(txt_label);
            auto shortcut = menu_item->prop_as_string("shortcut");
            if (shortcut.size())
            {
                menu_label << "    " << shortcut;
            }

            // If the user specified a stock ID, then we need to use that id in order to have wxWidgets generate the
            // label and bitmap.

            int id = wxID_ANY;
            if (menu_item->prop_as_string("id") != "wxID_ANY" && menu_item->prop_as_string("id").is_sameprefix("wxID_"))
                id = g_NodeCreator.GetConstantAsInt(menu_item->prop_as_string("id"), wxID_ANY);

            auto item = new wxMenuItem(sub_menu, id, menu_label, menu_item->prop_as_wxString("help"),
                                       (wxItemKind) menu_item->prop_as_int("kind"));

            if (menu_item->HasValue("bitmap"))
            {
                wxBitmap unchecked = wxNullBitmap;
                if (menu_item->HasValue("unchecked_bitmap"))
                {
                    unchecked = menu_item->prop_as_wxBitmap("unchecked_bitmap");
                }
#ifdef __WXMSW__
                item->SetBitmaps(menu_item->prop_as_wxBitmap("bitmap"), unchecked);
#else
                item->SetBitmap(menu_item->GetPropertyAsBitmap("bitmap"));
#endif
            }
#ifdef __WXMSW__
            else
            {
                if (menu_item->HasValue("unchecked_bitmap"))
                {
                    item->SetBitmaps(wxNullBitmap, menu_item->prop_as_wxBitmap("unchecked_bitmap"));
                }
            }
#endif

            sub_menu->Append(item);

            if (item->GetKind() == wxITEM_CHECK && menu_item->prop_as_bool("checked"))
            {
                item->Check(true);
            }

            if (menu_item->prop_as_bool("disabled"))
                item->Enable(false);
        }
    }

    return sub_menu;
}

//////////////////////////////////////////  MenuBarGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> MenuBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxMenuBar(";
    GenStyle(node, code);
    code << ");";
    code.Replace("(0)", "()");

    return code;
}

std::optional<ttlib::cstr> MenuBarGenerator::GenCode(const std::string& cmd, Node* node)
{
    ttlib::cstr code;
    if (cmd == "after_addchild")
    {
        code << "\tSetMenuBar(" << node->get_node_name() << ");";
    }
    else
    {
        return {};
    }

    return code;
}

std::optional<ttlib::cstr> MenuBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool MenuBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  MenuBarFormGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> MenuBarFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->prop_as_string(txt_class_name) << "::" << node->prop_as_string(txt_class_name);
    code << "(long style) : wxMenuBar(style)\n{";

    return code;
}

std::optional<ttlib::cstr> MenuBarFormGenerator::GenCode(const std::string& cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == "ctor_declare")
    {
        // This is the code to add to the header file
        code << "    " << node->get_node_name() << "(long style = ";
        GenStyle(node, code);
        code << ");";
    }
    else if (cmd == "base")
    {
        code << "public ";
        if (node->HasValue(txt_base_class_name))
        {
            code << node->prop_as_string(txt_base_class_name);
        }
        else
        {
            code << "wxMenuBar";
        }
    }
    else
    {
        return {};
    }

    return code;
}

bool MenuBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  MenuGenerator (wxMenu)  //////////////////////////////////////////

std::optional<ttlib::cstr> MenuGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    // REVIEW: [KeyWorks - 12-08-2020] See comment below for "dtor" -- this really shouldn't be here if this is a popup menu
    code << node->get_node_name() << " = new wxMenu();";

    return code;
}

std::optional<ttlib::cstr> MenuGenerator::GenCode(const std::string& cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == "dtor")
    {
        if (auto parent_type = node->GetParent()->GetNodeType();
            parent_type->get_name() != "menubar" && parent_type->get_name() != "menubar_form")
        {
            // REVIEW: [KeyWorks - 12-08-2020] This is only because the constructor is creating the menu via new. What
            // really should happen is that the menu should be in the header file as the actual menu, rather than a
            // pointer. Then a destructor isn't needed.

            // If the parent isn't a menubar, then it's being used as a popup menu, so we need to delete it.
            code << "\tdelete " << node->get_node_name() << ';';
        }
    }
    else if (cmd == "after_addchild")
    {
        auto parent_type = node->GetParent()->GetNodeType();
        if (parent_type->get_name() == "menubar")
        {
            code << "    " << node->get_parent_name() << "->Append(" << node->get_node_name() << ", ";
            code << GenerateQuotedString(node->prop_as_string(txt_label)) << ");";
        }
        else if (parent_type->get_name() == "menubar_form")
        {
            code << "    "
                 << "Append(" << node->get_node_name() << ", ";
            code << GenerateQuotedString(node->prop_as_string(txt_label)) << ");";
        }
        else
        {
            // The parent can disable generation of Bind by shutting of the context menu
            if (!node->GetParent()->prop_as_bool("context_menu"))
            {
                return {};
            }

            if (parent_type->get_name() == "form" || parent_type->get_name() == "wizard")
            {
                code << "\tBind(wxEVT_RIGHT_DOWN, &" << node->get_parent_name() << "::" << node->get_parent_name()
                     << "OnContextMenu, this);";
            }
            else if (parent_type->get_name() == "tool")
            {
                // REVIEW: [KeyWorks - 12-08-2020] I have no idea if this actually works since the original template code
                // used: Connect(#parent $name->GetId(), wxEVT_AUITOOLBAR_TOOL_DROPDOWN...

                code << "\tBind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &" << node->get_parent_name()
                     << "::" << node->get_parent_name() << "OnDropDownMenu, this);";
            }
            else
            {
                code << "    " << node->get_parent_name() << "->Bind(wxEVT_RIGHT_DOWN, &" << node->get_form_name()
                     << "::" << node->get_parent_name() << "OnContextMenu, this);";
            }
        }
    }

    else
    {
        return {};
    }

    return code;
}

bool MenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  SubMenuGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> SubMenuGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxMenu();\n";

    return code;
}

std::optional<ttlib::cstr> SubMenuGenerator::GenCode(const std::string& cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == "after_addchild")
    {
        code << "    " << node->get_parent_name() << "->AppendSubMenu(" << node->get_node_name() << ", ";
        code << GenerateQuotedString(node->prop_as_string(txt_label)) << ");";
    }
    else
    {
        return {};
    }

    return code;
}

std::optional<ttlib::cstr> SubMenuGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue("bitmap"))
    {
        code << "    " << node->get_node_name() << "Item->SetBitmap(" << GenerateBitmapCode(node->prop_as_string("bitmap"))
             << ");";
    }

    return code;
}

bool SubMenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  MenuItemGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> MenuItemGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("    ");
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxMenuItem(" << node->get_parent_name() << ", " << node->prop_as_string("id")
         << ", ";
    auto& label = node->prop_as_string(txt_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);

        // REVIEW: [KeyWorks - 12-08-2020] Frankly, I think this is kind of silly to add -- the documentation for label
        // clearly lays out how to do this as part of the actual string. Removing this means the wxFormBuilder import needs
        // to merge it into the main label.

        if (node->HasValue("shortcut"))
        {
            code << " + '\\t' + \"" << node->prop_as_string("shortcut") << '\"';
        }
    }
    else
    {
        code << "wxEmptyString";
    }

    if (node->HasValue("help") || node->prop_as_string("kind") != "wxITEM_NORMAL")
    {
        code << ",\n            " << GenerateQuotedString(node->prop_as_string("help")) << ", "
             << node->prop_as_string("kind");
    }

    code << ");";

    return code;
}

std::optional<ttlib::cstr> MenuItemGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue("bitmap"))
    {
        if (node->HasValue("unchecked_bitmap"))
        {
            code << "    " << node->get_node_name() << "->SetBitmaps(" << GenerateBitmapCode(node->prop_as_string("bitmap"));
            code << ", " << GenerateBitmapCode(node->prop_as_string("unchecked_bitmap")) << ");";
        }
        else
        {
            code << "    " << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string("bitmap"))
                 << ");";
        }
    }

    if (code.size())
        code << '\n';
    code << "    " << node->get_parent_name() << "->Append(" << node->get_node_name() << ");";

    if ((node->prop_as_string("kind") == "wxITEM_CHECK" || node->prop_as_string("kind") == "wxITEM_RADIO") &&
        node->prop_as_bool("checked"))
    {
        if (code.size())
            code << '\n';
        code << "    " << node->get_node_name() << "->Check();";
    }

    return code;
}

std::optional<ttlib::cstr> MenuItemGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool MenuItemGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  SeparatorGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> SeparatorGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->get_parent_name() << "->AppendSeparator();";

    return code;
}

bool SeparatorGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}
