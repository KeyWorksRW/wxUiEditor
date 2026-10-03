/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Item Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [10-03-2026]

#include <unordered_map>

#include <wx/artprov.h>            // wxArtProvider class
#include <wx/menu.h>               // wxMenu and wxMenuBar classes
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "image_gen.h"      // CommonArtHeaderBundleName()
#include "image_handler.h"  // ImageHandler class
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "node_creator.h"   // NodeCreator -- NodeCreator class
#include "undo_cmds.h"      // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "gen_menuitem.h"

// clang-format off

static const std::unordered_map<std::string, std::string> map_id_artid = {

    { "wxID_OPEN", "wxART_FILE_OPEN" },
    { "wxID_SAVE", "wxART_FILE_SAVE" },
    { "wxID_SAVEAS", "wxART_FILE_SAVE_AS" },
    { "wxID_PRINT", "wxART_PRINT" },
    { "wxID_NEW", "wxART_NEW" },
    { "wxID_CLOSE", "wxART_CLOSE" },
    { "wxID_EXIT", "wxART_QUIT" },

    { "wxID_EDIT", "wxART_EDIT" },
    { "wxID_COPY", "wxART_COPY" },
    { "wxID_CUT", "wxART_CUT" },
    { "wxID_PASTE", "wxART_PASTE" },
    { "wxID_DELETE", "wxART_DELETE" },

    { "wxID_FIRST", "wxART_GOTO_FIRST" },
    { "wxID_LAST", "wxART_GOTO_LAST" },
    { "wxID_FIND", "wxART_FIND" },
    { "wxID_REPLACE", "wxART_FIND_AND_REPLACE" },
    { "wxID_UNDO", "wxART_UNDO" },
    { "wxID_REDO", "wxART_REDO" },

};

// clang-format on

bool MenuItemGenerator::ConstructionCode(Code& code)
{
    const Node* node =
        code.node();  // This is just for code readability -- could just use code.node() everywhere
    code.AddAuto();

    // MenuItem nodes always have a parent (menu, menu bar, or popup menu).
    if (node->get_Parent()->is_Gen(gen_PopupMenu))
    {
        code.NodeName();
        code.AddIfCpp(" = Append(");
        code.AddIfPython(" = self.Append(");
        // wxRuby has no Append(); the equivalent is the snake_case append().
        code.AddIfRuby(" = append(");
        code.as_string(prop_id).Comma();
    }
    else
    {
        code.NodeName().CreateClass().ParentName().Comma();
        if (node->as_string(prop_stock_id) != "none")
        {
            // Code::Add(prop_stock_id) translates the wxID_* constant to each target language.
            code.Add(prop_stock_id).EndFunction();
            return true;
        }
        code.as_string(prop_id).Comma();
    }

    const wxue::string& label = node->as_string(prop_label);
    if (!label.empty())
    {
        if (node->HasValue(prop_shortcut))
        {
            code.QuotedString(wxue::string() << label << '\t' << node->as_string(prop_shortcut));
        }
        else
        {
            code.QuotedString(label);
        }
    }
    else
    {
        // Code::Add() translates "wxEmptyString" per target language ("''" for Ruby,
        // "wx.EmptyString" for Python), so this literal is valid for every language.
        code.Add("wxEmptyString");
    }

    if (code.HasValue(prop_help) || node->as_string(prop_kind) != "wxITEM_NORMAL")
    {
        // Code::Add(prop_kind) routes through Code::as_string() -> Code::Add(), which converts
        // the wx-prefixed constant (e.g. wxITEM_CHECK) to each target language's prefix.
        code.Comma().CheckLineLength().QuotedString(prop_help).Comma().Add(prop_kind);
    }
    code.EndFunction();

    return true;
}

bool MenuItemGenerator::SettingsCode(Code& code)
{
    const Node* node = code.node();
    if (code.HasValue(prop_extra_accels))
    {
        wxue::StringVector accel_list;
        accel_list.SetString(std::string_view(node->as_string(prop_extra_accels)), '"',
                             wxue::TRIM::both);

        if (code.is_cpp())
        {
            // auto_indent = indent::auto_keep_whitespace;
            code.OpenBrace().Add("wxAcceleratorEntry entry;").Eol();

            for (auto& accel: accel_list)
            {
                if (!accel.empty())
                {
                    code.Eol(eol_if_needed)
                        << "if (entry.FromString(" << GenerateQuotedString(accel) << "))";
                    code.Eol().Tab().NodeName().Function("AddExtraAccel(entry").EndFunction();
                }
            }
            code.CloseBrace();
            code.UpdateBreakAt();
        }
        else if (code.is_python())  // python
        {
            code.Str("entry = ").Add("wxAcceleratorEntry()").Eol();
            for (auto& accel: accel_list)
            {
                if (!accel.empty())
                {
                    code.Str("if entry.FromString(").QuotedString(accel).Str(") :").Eol();
                    code.Tab().NodeName().Str(".AddExtraAccel(entry)").Eol();
                }
            }
        }
        else if (code.is_ruby())
        {
            // TODO: [Randalphwa - 08-02-2023] Fill in once we have figured out how to handle
            // wxAcceleratorEntry in Ruby
            code.AddComment("Extra accelerators are not supported in wxRuby", true);
        }
        else
        {
            ASSERT_MSG(false, "Unknown language in MenuItemGenerator::SettingsCode()");
        }
    }

    if (code.HasValue(prop_bitmap))
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
                code.NodeName().Function("SetBitmap(");
                code << function_name;
                code.EndFunction();
            }

            else
            {
                wxue::string bundle_code;
                const bool is_vector_code = GenerateBundleCode(description, bundle_code);

                if (!is_vector_code)
                {
                    code.NodeName().Function("SetBitmap(");
                    code += bundle_code;
                    code.EndFunction();
                    code.Eol();
                }
                else  // bundle_code contains a vector
                {
                    code += bundle_code;
                    code.Tab().NodeName().Function("SetBitmap(");
                    code += "wxBitmapBundle::FromBitmaps(bitmaps)";
                    code.EndFunction().CloseBrace();
                }
            }
        }
        else if (code.is_python())
        {
            const bool is_list_created = PythonBitmapList(code, prop_bitmap);
            code.NodeName().Function("SetBitmap(");
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
            code.NodeName().Function("SetBitmap(");
            code.Bundle(prop_bitmap).EndFunction();
        }
        else
        {
            ASSERT_MSG(false, "Unknown language in MenuItemGenerator::SettingsCode()");
        }
    }

    if (code.HasValue(prop_unchecked_bitmap))
    {
        code.AddComment("Set the unchecked bitmap");
        if (code.is_cpp())
        {
            const wxue::string& description = node->as_string(prop_unchecked_bitmap);
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
                code.NodeName().Function("SetBitmap(");
                code << function_name << ", false";
                code.EndFunction();
            }
            else
            {
                wxue::string bundle_code;
                const bool is_vector_code = GenerateBundleCode(description, bundle_code);
                code.UpdateBreakAt();

                if (!is_vector_code)
                {
                    code.NodeName().Function("SetBitmap(");
                    code += bundle_code;
                    code.UpdateBreakAt();
                    code.Comma() += "false";
                    code.EndFunction();
                    code.UpdateBreakAt();
                }
                else  // bundle_code contains a vector
                {
                    code += bundle_code;
                    code.Tab().NodeName().Function("SetBitmap(");
                    code += "wxBitmapBundle::FromBitmaps(bitmaps)";
                    code.UpdateBreakAt();
                    code.Comma() += "false";
                    code.EndFunction().CloseBrace();
                }
            }
        }

        else if (code.is_python())
        {
            code.Eol(eol_if_needed);
            const bool is_list_created = PythonBitmapList(code, prop_unchecked_bitmap);
            code.NodeName().Function("SetBitmap(");
            if (is_list_created)
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
            else
            {
                code.Bundle(prop_unchecked_bitmap);
            }
            code.Comma().False().EndFunction();
        }
        else if (code.is_ruby())
        {
            code.Eol(eol_if_needed).NodeName().Function("SetBitmap(");
            code.Bundle(prop_unchecked_bitmap).Comma().False().EndFunction();
        }
    }

    // MenuItem nodes always have a parent (menu, menu bar, or popup menu).
    if (!node->get_Parent()->is_Gen(gen_PopupMenu))
    {
        code.Eol(eol_if_empty).ParentName().Function("Append(").NodeName().EndFunction();
    }

    if ((node->as_string(prop_kind) == "wxITEM_CHECK" ||
         node->as_string(prop_kind) == "wxITEM_RADIO") &&
        code.IsTrue(prop_checked))
    {
        code.Eol(eol_if_empty).NodeName().Function("Check(").EndFunction();
    }

    return true;
}

bool MenuItemGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);
    if (node->HasValue(prop_extra_accels))
    {
        InsertGeneratorInclude(node, "#include <wx/accel.h>", set_src, set_hdr);
    }

    return true;
}

int MenuItemGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    // MenuItem nodes always have a parent; a MenuItem whose parent is a sizer is a menu item
    // inside a popup menu.
    const int result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                        BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxMenuItem");

    if (node->as_string(prop_stock_id) != "none")
    {
        GenXrcBitmap(node, item, xrc_flags);

        if (xrc_flags & xrc::add_comments)
        {
            GenXrcComments(node, item);
        }

        return result;
    }

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_PROP(prop_shortcut, "accel")
    if (node->HasValue(prop_extra_accels))
    {
        pugi::xml_node child = item.append_child("extra-accels");
        wxue::StringVector accel_list;
        accel_list.SetString(std::string_view(node->as_string(prop_extra_accels)), '"',
                             wxue::TRIM::both);
        for (auto& accel: accel_list)
        {
            if (!accel.empty())
            {
                child.append_child("accel").text().set(accel);
            }
        }
    }
    ADD_ITEM_PROP(prop_help, "help")
    ADD_ITEM_BOOL(prop_checked, "checked")
    if (node->as_bool(prop_disabled))
    {
        item.append_child("enabled").text().set("0");
    }

    if (node->as_string(prop_kind) == "wxITEM_RADIO")
    {
        item.append_child("radio").text().set("1");
    }
    else if (node->as_string(prop_kind) == "wxITEM_CHECK")
    {
        item.append_child("checkable").text().set("1");
    }

    GenXrcBitmap(node, item, xrc_flags);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void MenuItemGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid,
                                          NodeProperty* changed_prop)
{
    // map_PropNames is a compile-time table containing every PropName, so the .at() lookups
    // below (prop_label, prop_help, prop_id) cannot throw.
    if (changed_prop->isProp(prop_stock_id))
    {
        if (auto* pg_setting = prop_grid->GetProperty(wxString(map_PropNames.at(prop_label)));
            pg_setting)
        {
            pg_setting->Enable(changed_prop->value() == "none");
        }
        if (auto* pg_setting = prop_grid->GetProperty(wxString(map_PropNames.at(prop_help)));
            pg_setting)
        {
            pg_setting->Enable(changed_prop->value() == "none");
        }
        if (auto* pg_setting = prop_grid->GetProperty(wxString(map_PropNames.at(prop_id)));
            pg_setting)
        {
            pg_setting->Enable(changed_prop->value() == "none");
        }
    }
}

bool MenuItemGenerator::ModifyProperty(NodeProperty* prop, wxue::string_view value)
{
    if (prop->isProp(prop_stock_id))
    {
        if (value != "none")
        {
            auto undo_stock_id = std::make_shared<ModifyProperties>("Stock ID");
            undo_stock_id->addProperty(prop, value);
            // A MenuItem node always has prop_label, prop_help, prop_id, and prop_bitmap, so
            // get_PropPtr() here is guaranteed to be non-null.
            undo_stock_id->addProperty(
                prop->getNode()->get_PropPtr(prop_label),
                wxGetStockLabel(NodeCreation.get_ConstantAsInt(value.as_str())).utf8_string());
            undo_stock_id->addProperty(
                prop->getNode()->get_PropPtr(prop_help),
                wxGetStockHelpString(NodeCreation.get_ConstantAsInt(value.as_str())).utf8_string());
            undo_stock_id->addProperty(prop->getNode()->get_PropPtr(prop_id), value);

            if (auto result = map_id_artid.find(value.as_str()); result != map_id_artid.end())
            {
                undo_stock_id->addProperty(prop->getNode()->get_PropPtr(prop_bitmap),
                                           wxue::string("Art;") << result->second << "|wxART_MENU");
            }
            wxGetFrame().PushUndoAction(undo_stock_id);
            return true;
        }
    }
    return false;
}
