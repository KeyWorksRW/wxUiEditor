/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>      // wxMenu and wxMenuBar classes
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statline.h>  // wxStaticLine class interface
#include <wx/stattext.h>  // wxStaticText base header

#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- NodeCreator class
#include "utils.h"         // Utility functions that work with properties
#include "write_code.h"    // Write code to Scintilla or file

#include "menu_widgets.h"

//////////////////////////////////////////  MenuBarBase  //////////////////////////////////////////

wxObject* MenuBarBase::CreateMockup(Node* node, wxObject* parent)
{
    // A real wxMenubar requires a frame window, which we don't have in the Mockup panel. Instead, we create a panel
    // with static text for each top level menu. If the user clicks on one of the static text controls, then we locate
    // which child menu node contains that label and then create a Popup menu to display it.

    auto panel = new wxPanel(wxStaticCast(parent, wxWindow));
    auto sizer = new wxBoxSizer(wxHORIZONTAL);

    if (node->isGen(gen_PopupMenu))
    {
        auto label = new wxStaticText(panel, wxID_ANY, node->prop_as_wxString(prop_class_name));
        sizer->Add(label, wxSizerFlags().Border(wxALL));
        label->Bind(wxEVT_LEFT_DOWN, &MenuBarBase::OnLeftMenuClick, this);
    }
    else
    {
        for (auto& iter: node->GetChildNodePtrs())
        {
            auto label = new wxStaticText(panel, wxID_ANY, iter->prop_as_wxString(prop_label));
            sizer->Add(label, wxSizerFlags().Border(wxALL));
            label->Bind(wxEVT_LEFT_DOWN, &MenuBarBase::OnLeftMenuClick, this);
        }
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

    if (m_node_menubar->isGen(gen_PopupMenu))
    {
        menu_node = m_node_menubar;
    }
    else
    {
        for (size_t pos_menu = 0; pos_menu < m_node_menubar->GetChildCount(); ++pos_menu)
        {
            if (m_node_menubar->GetChild(pos_menu)->prop_as_string(prop_label) == text)
            {
                menu_node = m_node_menubar->GetChild(pos_menu);
                break;
            }
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
            auto item = sub_menu->AppendSubMenu(result, menu_item->prop_as_wxString(prop_label));
            if (menu_item->HasValue(prop_bitmap))
                item->SetBitmap(menu_item->prop_as_wxBitmapBundle(prop_bitmap));
        }
        else if (menu_item->isGen(gen_separator))
        {
            sub_menu->AppendSeparator();
        }
        else
        {
            auto menu_label = menu_item->prop_as_string(prop_label);
            auto shortcut = menu_item->prop_as_string(prop_shortcut);
            if (shortcut.size())
            {
                menu_label << "\t" << shortcut;
            }

            // If the user specified a stock ID, then we need to use that id in order to have wxWidgets generate the
            // label and bitmap.

            int id = wxID_ANY;
            if (menu_item->prop_as_string(prop_id) != "wxID_ANY" &&
                menu_item->prop_as_string(prop_id).is_sameprefix("wxID_"))
                id = g_NodeCreator.GetConstantAsInt(menu_item->prop_as_string(prop_id), wxID_ANY);

            auto item = new wxMenuItem(sub_menu, id, menu_label, menu_item->prop_as_wxString(prop_help),
                                       (wxItemKind) menu_item->prop_as_int(prop_kind));

            if (menu_item->HasValue(prop_bitmap))
            {
#ifdef __WXMSW__
                if (menu_item->HasValue(prop_unchecked_bitmap))
                {
                    auto unchecked = menu_item->prop_as_wxBitmapBundle(prop_unchecked_bitmap);
                    item->SetBitmaps(menu_item->prop_as_wxBitmapBundle(prop_bitmap), unchecked);
                }
                else
#endif
                    item->SetBitmap(menu_item->prop_as_wxBitmapBundle(prop_bitmap));
            }
#ifdef __WXMSW__
            else
            {
                if (menu_item->HasValue(prop_unchecked_bitmap))
                {
                    item->SetBitmaps(wxNullBitmap, menu_item->prop_as_wxBitmapBundle(prop_unchecked_bitmap));
                }
            }
#endif

            sub_menu->Append(item);

            if (item->GetKind() == wxITEM_CHECK && menu_item->prop_as_bool(prop_checked))
            {
                item->Check(true);
            }

            if (menu_item->prop_as_bool(prop_disabled))
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

std::optional<ttlib::cstr> MenuBarGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_after_children)
    {
        code << "\tSetMenuBar(" << node->get_node_name() << ");";
        return code;
    }

    return {};
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

    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(long style) : wxMenuBar(style)\n{";

    return code;
}

std::optional<ttlib::cstr> MenuBarFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_header)
    {
        // This is the code to add to the header file
        code << "\t" << node->get_node_name() << "(long style = ";
        GenStyle(node, code);
        code << ");";
        return code;
    }

    else if (cmd == code_base_class)
    {
        if (node->HasValue(prop_derived_class))
        {
            code << node->prop_as_string(prop_derived_class);
        }
        else
        {
            code << "wxMenuBar";
        }
        return code;
    }
    return {};
}

bool MenuBarFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  PopupMenuGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> PopupMenuGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "() : wxMenu()\n{";

    return code;
}

std::optional<ttlib::cstr> PopupMenuGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_header)
    {
        // This is the code to add to the header file
        code << node->get_node_name() << "();";
        return code;
    }

    else if (cmd == code_base_class)
    {
        if (node->HasValue(prop_derived_class))
        {
            code << node->prop_as_string(prop_derived_class);
        }
        else
        {
            code << "wxMenu";
        }
        return code;
    }
    return {};
}

bool PopupMenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
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

    // BUGBUG: [KeyWorks - 05-20-2021] This never gets deleted!
    code << node->get_node_name() << " = new wxMenu();";

    return code;
}

std::optional<ttlib::cstr> MenuGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_after_children)
    {
        auto parent_type = node->GetParent()->gen_type();
        if (parent_type == type_menubar)
        {
            code << "\t" << node->get_parent_name() << "->Append(" << node->get_node_name() << ", ";
            code << GenerateQuotedString(node->prop_as_string(prop_label)) << ");";
        }
        else if (parent_type == type_menubar_form)
        {
            code << "\t"
                 << "Append(" << node->get_node_name() << ", ";
            code << GenerateQuotedString(node->prop_as_string(prop_label)) << ");";
        }
        else
        {
            // The parent can disable generation of Bind by shutting of the context menu
            if (!node->GetParent()->prop_as_bool(prop_context_menu))
            {
                return {};
            }

            if (parent_type == type_form || parent_type == type_frame_form || parent_type == type_wizard)
            {
                code << "\tBind(wxEVT_RIGHT_DOWN, &" << node->get_parent_name() << "::" << node->get_parent_name()
                     << "OnContextMenu, this);";
            }
            else
            {
                code << "\t" << node->get_parent_name() << "->Bind(wxEVT_RIGHT_DOWN, &" << node->get_form_name()
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

std::optional<ttlib::cstr> SubMenuGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_after_children)
    {
        if (node->GetParent()->isGen(gen_PopupMenu))
        {
            code << "\t"
                    "AppendSubMenu("
                 << node->get_node_name() << ", ";
        }
        else
        {
            code << "\t" << node->get_parent_name() << "->AppendSubMenu(" << node->get_node_name() << ", ";
        }

        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ");";
    }
    else
    {
        return {};
    }

    return code;
}

std::optional<ttlib::cstr> SubMenuGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    if (node->HasValue(prop_bitmap))
    {
        auto_indent = indent::auto_keep_whitespace;
        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
        if (is_code_block)
        {
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)\n";
            }
            // GenerateBundleCode assumes an indent within an indent
            bundle_code.Replace("\t\t\t", "\t", true);
            code << bundle_code;
            code << "\t";
            if (node->IsLocal())
                code << "auto ";
            code << node->get_node_name() << "Item->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));";
            code << "\n}";
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                if (node->IsLocal())
                    code << "auto ";

                code << node->get_node_name() << "Item->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap))
                     << ");";
                code << "\n#endif";
            }
        }
        else
        {
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)\n";
            }
            if (node->IsLocal())
                code << "auto ";
            code << node->get_node_name() << "Item->SetBitmap(" << bundle_code << ");";
            if (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                if (node->IsLocal())
                    code << "auto ";

                code << node->get_node_name() << "Item->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap))
                     << ");";
                code << "\n#endif";
            }
        }
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
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    if (node->GetParent()->isGen(gen_PopupMenu))
    {
        code << node->get_node_name() << " = Append(" << node->prop_as_string(prop_id) << ", ";
    }
    else
    {
        code << node->get_node_name() << " = new wxMenuItem(" << node->get_parent_name() << ", "
             << node->prop_as_string(prop_id) << ", ";
    }
    auto& label = node->prop_as_string(prop_label);
    if (label.size())
    {
        if (node->HasValue(prop_shortcut))
        {
            code << GenerateQuotedString(ttlib::cstr() << label << '\t' << node->prop_as_string(prop_shortcut));
        }
        else
        {
            code << GenerateQuotedString(label);
        }
    }
    else
    {
        code << "wxEmptyString";
    }

    if (node->HasValue(prop_help) || node->prop_as_string(prop_kind) != "wxITEM_NORMAL")
    {
        code.insert(0, 1, '\t');
        code << ",\n\t\t" << GenerateQuotedString(node->prop_as_string(prop_help)) << ", "
             << node->prop_as_string(prop_kind);
    }

    code << ");";

    return code;
}

std::optional<ttlib::cstr> MenuItemGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;
    bool has_bitmap = node->HasValue(prop_bitmap);

    if (has_bitmap)
    {
        auto_indent = indent::auto_keep_whitespace;

        bool is_old_widgets = (wxGetProject().prop_as_string(prop_wxWidgets_version) == "3.1");
        if (is_old_widgets)
        {
            code << "#if wxCHECK_VERSION(3, 1, 6)\n";
        }

        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);

        if (node->HasValue(prop_unchecked_bitmap))
        {
            ttlib::cstr unchecked_code;
            bool is_checked_block = GenerateBundleCode(node->prop_as_string(prop_unchecked_bitmap), unchecked_code);
            if (is_code_block || is_checked_block)
            {
                code << "{\n";
                if (is_code_block)
                {
                    // GenerateBundleCode assumes an indent within an indent
                    bundle_code.Replace("\t\t\t", "\t", true);
                    // we already generated the opening brace
                    bundle_code.erase(0, 1);
                    code << bundle_code.c_str() + 1;
                }

                if (is_checked_block)
                {
                    // GenerateBundleCode assumes an indent within an indent
                    unchecked_code.Replace("\t\t\t", "\t", true);
                    // we already generated the opening brace
                    unchecked_code.erase(0, 1);
                    if (is_code_block)
                    {
                        unchecked_code.Replace("bitmaps", "unchecked_bmps", true);
                    }
                    code << '\n' << unchecked_code.c_str() + 1;
                }

                code << "\t" << node->get_node_name() << "->SetBitmaps(";

                if (is_code_block)
                {
                    code << "wxBitmapBundle::FromBitmaps(bitmaps), ";
                }
                else
                {
                    code << bundle_code << ", ";
                }

                if (is_checked_block)
                {
                    code << "wxBitmapBundle::FromBitmaps(";
                    code << (is_code_block ? "unchecked_bmps" : "bitmaps");
                }
                else
                {
                    code << bundle_code << ", " << unchecked_code;
                }

                code << ");\n}";
            }
            else
            {
                code << node->get_node_name() << "->SetBitmaps(" << bundle_code;
                code << ", " << unchecked_code << ");";
            }

            if (is_old_widgets)
            {
                code << "\n#else\n";
                if (node->HasValue(prop_unchecked_bitmap))
                {
                    code << node->get_node_name() << "->SetBitmaps("
                         << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
                    code << ", " << GenerateBitmapCode(node->prop_as_string(prop_unchecked_bitmap)) << ");";
                }
                else
                {
                    code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap))
                         << ");";
                }
                code << "\n#endif  // wxCHECK_VERSION(3, 1, 6)";
            }
        }

        // single bitmap (no unchecked bitmap)
        else
        {
            if (is_code_block)
            {
                // GenerateBundleCode assumes an indent within an indent
                bundle_code.Replace("\t\t\t", "\t", true);
                code << bundle_code;
                code << "\t" << node->get_node_name() << "->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));\n}";
            }
            else
            {
                code << node->get_node_name() << "->SetBitmap(" << bundle_code << ");";
            }
            if (is_old_widgets)
            {
                code << "\n#else\n";

                code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap))
                     << ");";
                code << "\n#endif  // wxCHECK_VERSION(3, 1, 6)";
            }
        }
    }

    if (code.size())
        code << '\n';

    if (!node->GetParent()->isGen(gen_PopupMenu))
    {
        if (!has_bitmap)
            code << "\t";
        code << node->get_parent_name() << "->Append(" << node->get_node_name() << ");";
    }

    if ((node->prop_as_string(prop_kind) == "wxITEM_CHECK" || node->prop_as_string(prop_kind) == "wxITEM_RADIO") &&
        node->prop_as_bool(prop_checked))
    {
        if (code.size())
            code << '\n';
        if (!has_bitmap)
            code << "\t";
        code << node->get_node_name() << "->Check();";
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

    if (node->GetParent()->isGen(gen_PopupMenu))
        code << "AppendSeparator();";
    else
        code << node->get_parent_name() << "->AppendSeparator();";

    return code;
}

bool SeparatorGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  CtxMenuGenerator  //////////////////////////////////////////

bool CtxMenuGenerator::GetIncludes(Node* /* node */, std::set<std::string>& set_src, std::set<std::string>&
                                   /* set_hdr */)
{
    set_src.insert("#include <wx/event.h>");
    set_src.insert("#include <wx/menu.h>");
    set_src.insert("#include <wx/window.h>");

    return true;
}
