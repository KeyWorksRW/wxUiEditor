/////////////////////////////////////////////////////////////////////////////
// Purpose:   SubMenu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [10-03-2026]

#include <wx/menu.h>  // wxMenu and wxMenuBar classes

#include "gen_common.h"                  // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"               // Common XRC generating functions
#include "image_gen.h"                   // CommonArtHeaderBundleName()
#include "image_handler.h"               // ImageHandler class
#include "node.h"                        // Node class
#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "gen_submenu.h"

bool SubMenuGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass(false, "wxMenu").EndFunction();

    return true;
}

bool SubMenuGenerator::AfterChildrenCode(Code& code)
{
    const Node* node =
        code.node();  // This is just for code readability -- could just use code.node() everywhere
    wxue::string submenu_item_name;

    if (node->HasValue(prop_bitmap))
    {
        if (code.is_cpp())
        {
            code += "auto* ";
        }
        const size_t name_offset = code.size();
        code.NodeName();
        submenu_item_name.assign(code.data() + name_offset, code.size() - name_offset);
        code.Str("_item = ");
        submenu_item_name << "_item";
    }

    // When an id is set, use the id-taking wxMenu::Append() overload so the submenu item can be
    // enabled, disabled, or updated by id. Leaving it at the wxID_ANY default keeps
    // AppendSubMenu(), whose item is created with wxID_ANY -- the same output as before this
    // property existed. HasValue() is not usable here: it only reports whether the value is
    // non-empty, and the property's default is the non-empty string "wxID_ANY".
    const wxue::string id_value = node->as_string(prop_id);
    const bool has_id = !id_value.empty() && !id_value.is_sameas("wxID_ANY", wxue::CASE::either);

    if (node->get_Parent() != nullptr && node->get_Parent()->is_Gen(gen_PopupMenu))
    {
        code.FormFunction(has_id ? "Append(" : "AppendSubMenu(");
    }
    else
    {
        code.ParentName().Function(has_id ? "Append(" : "AppendSubMenu(");
    }

    // Every supported target exposes the id-taking submenu overload in the same
    // (id, label, submenu) argument order as wxMenu::Append(int, const wxString&, wxMenu*),
    // so a single argument sequence is emitted for all languages.
    if (has_id)
    {
        code.as_string(prop_id).Comma().QuotedString(prop_label).Comma().NodeName();
    }
    else
    {
        code.NodeName().Comma().QuotedString(prop_label);
    }
    code.EndFunction();

    if (node->HasValue(prop_bitmap))
    {
        code.Eol(eol_if_empty);
        if (code.is_cpp())
        {
            const wxue::string& description = node->as_string(prop_bitmap);
            const wxue::StringVector description_parts(description, BMP_PROP_SEPARATOR,
                                                       wxue::TRIM::both);
            wxue::string function_name = ProjectImages.GetBundleFuncName(description);
            if (function_name.empty())
            {
                function_name = CommonArtHeaderBundleName(&description_parts);
            }
            if (!function_name.empty())
            {
                // We get here if there is an Image List that contains the function to retrieve this
                // bundle.
                code.Str(submenu_item_name).Function("SetBitmap(");
                code << function_name;
                code.EndFunction();
            }

            else
            {
                wxue::string bundle_code;
                const bool is_vector_code = GenerateBundleCode(description, bundle_code);
                code.UpdateBreakAt();

                if (!is_vector_code)
                {
                    code.Str(submenu_item_name).Function("SetBitmap(");
                    code += bundle_code;
                    code.EndFunction();
                    // No Eol() here: EndFunction() plus the code writer handle statement
                    // termination, matching the function_name/vector/Python/Ruby branches below.
                }
                else  // bundle_code contains a vector
                {
                    code += bundle_code;
                    code.Str(submenu_item_name)
                        .Function("SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));");
                }
            }
        }

        else if (code.is_python())
        {
            const bool is_list_created = PythonBitmapList(code, prop_bitmap);
            code.Str(submenu_item_name).Function("SetBitmap(");
            if (is_list_created)
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
            else
            {
                code.Bundle(prop_bitmap);
            }
            code.EndFunction();
        }
        else if (code.is_ruby())
        {
            code.Str(submenu_item_name).Function("SetBitmap(").Bundle(prop_bitmap).EndFunction();
        }
    }

    return true;
}

bool SubMenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                   std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

int SubMenuGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxMenu");

    // A non-default prop_id is emitted as this object's name attribute by
    // GenXrcObjectAttributes(), so the XRC preview keeps the same id as the generated
    // Append(id, label, submenu) call; no separate id handling is needed here.
    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}
