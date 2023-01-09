/////////////////////////////////////////////////////////////////////////////
// Purpose:   SubMenu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>  // wxMenu and wxMenuBar classes

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "gen_submenu.h"

bool SubMenuGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass(false, "wxMenu").EndFunction();

    return true;
}

bool SubMenuGenerator::AfterChildrenCode(Code& code)
{
    if (code.node()->GetParent()->isGen(gen_PopupMenu))
    {
        code.FormFunction("AppendSubMenu(").NodeName().Comma().QuotedString(prop_label).EndFunction();
    }
    else
    {
        code.ParentName().Function("AppendSubMenu(").NodeName().Comma().QuotedString(prop_label).EndFunction();
    }

    return true;
}

#if 0
// BUGBUG: [Randalphwa - 12-16-2022] See issue #865 -- this should be in AdditionalCode, not here

std::optional<ttlib::cstr> SubMenuGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;


    if (node->HasValue(prop_bitmap))
    {
        ttlib::cstr bundle_code;
        bool is_code_block = GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
        if (is_code_block)
        {
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)\n";
            }
            // GenerateBundleCode assumes an indent within an indent
            bundle_code.Replace("\t\t\t", "\t", true);
            code << bundle_code;
            code << "\t";
            if (node->IsLocal())
                code << "auto* ";
            code << node->get_node_name() << "Item->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));";
            code << "\n}";
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                if (node->IsLocal())
                    code << "auto* ";

                code << node->get_node_name() << "Item->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap))
                     << ");";
                code << "\n#endif";
            }
        }
        else
        {
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "#if wxCHECK_VERSION(3, 1, 6)\n";
            }
            if (node->IsLocal())
                code << "auto* ";
            code << node->get_node_name() << "Item->SetBitmap(" << bundle_code << ");";
            if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
            {
                code << "\n#else\n";
                if (node->IsLocal())
                    code << "auto* ";

                code << node->get_node_name() << "Item->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap))
                     << ");";
                code << "\n#endif";
            }
        }
    }

    return code;
}
#endif

bool SubMenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

int SubMenuGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxMenu");

    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}
