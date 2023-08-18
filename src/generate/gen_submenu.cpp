/////////////////////////////////////////////////////////////////////////////
// Purpose:   SubMenu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>  // wxMenu and wxMenuBar classes

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "gen_submenu.h"

bool SubMenuGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass(false, "wxMenu").EndFunction();

    return true;
}

bool SubMenuGenerator::AfterChildrenCode(Code& code)
{
    auto* node = code.node();  // This is just for code readability -- could just use code.node() everywhere
    tt_string submenu_item_name;

    if (node->hasValue(prop_bitmap))
    {
        if (code.is_cpp())
        {
            code += "auto* ";
        }
        code.NodeName().Str("_item = ");
        submenu_item_name = node->getNodeName() + "_item";
    }

    if (node->getParent()->isGen(gen_PopupMenu))
    {
        code.FormFunction("AppendSubMenu(").NodeName().Comma().QuotedString(prop_label).EndFunction();
    }
    else
    {
        code.ParentName().Function("AppendSubMenu(").NodeName().Comma().QuotedString(prop_label).EndFunction();
    }

    if (node->hasValue(prop_bitmap))
    {
        code.Eol();
        if (code.is_cpp())
        {
            auto& description = node->as_string(prop_bitmap);
            bool is_vector_code = GenerateVectorCode(description, code.GetCode());
            code.UpdateBreakAt();

            if (!is_vector_code)
            {
                code.Str(submenu_item_name).Function("SetBitmap(");
                if (Project.as_string(prop_wxWidgets_version) != "3.1")
                {
                    GenerateBundleCode(description, code.GetCode());
                    code.EndFunction();
                }
                else
                {
                    code.Eol() += "#if wxCHECK_VERSION(3, 1, 6)\n\t";
                    GenerateBundleCode(description, code.GetCode());
                    code.Eol() += "#else";
                    code.Eol().Tab() << "wxBitmap(" << GenerateBitmapCode(description) << ")";
                    code.Eol() += "#endif";
                    code.Eol().EndFunction();
                }
                code.Eol();
            }
            else
            {
                code.Tab().Str(submenu_item_name).Function("SetBitmap(");
                if (Project.as_string(prop_wxWidgets_version) != "3.1")
                {
                    code += "wxBitmapBundle::FromBitmaps(bitmaps)";
                    code.UpdateBreakAt();
                    code.EndFunction().CloseBrace();
                }
                else
                {
                    code += "\n#if wxCHECK_VERSION(3, 1, 6)\n\t";
                    code.Tab() += "wxBitmapBundle::FromBitmaps(bitmaps)";
                    code += "\n#else\n\t";
                    code.Tab() << "wxBitmap(" << GenerateBitmapCode(description) << ")\n";
                    code << "#endif\n";
                    code.UpdateBreakAt();
                    code.Tab().EndFunction().CloseBrace();
                }
            }
        }

        else if (code.is_python())
        {
            bool is_list_created = PythonBitmapList(code, prop_bitmap);
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
