/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Item Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <unordered_map>
#include <unordered_set>

#include <wx/artprov.h>            // wxArtProvider class
#include <wx/menu.h>               // wxMenu and wxMenuBar classes
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- NodeCreator class
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // InsertNodeAction -- Undoable command classes derived from UndoAction

#include "gen_menuitem.h"

// clang-format off

std::unordered_map<std::string, std::string> map_id_artid = {

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
    Node* node = code.node();  // This is just for code readability -- could just use code.node() everywhere
    code.AddAuto();

    if (node->getParent()->isGen(gen_PopupMenu))
    {
        code.NodeName();
        code.AddIfCpp(" = Append(");
        code.AddIfPython(" = self.Append(");
        code.as_string(prop_id).Comma();
    }
    else
    {
        code.NodeName().CreateClass().ParentName().Comma();
        if (node->as_string(prop_stock_id) != "none")
        {
            code.Add(prop_stock_id).EndFunction();
            return true;
        }
        code.as_string(prop_id).Comma();
    }

    auto& label = node->as_string(prop_label);
    if (label.size())
    {
        if (node->hasValue(prop_shortcut))
        {
            code.QuotedString(tt_string() << label << '\t' << node->as_string(prop_shortcut));
        }
        else
        {
            code.QuotedString(label);
        }
    }
    else
    {
        code.Add("wxEmptyString");
    }

    if (code.hasValue(prop_help) || node->as_string(prop_kind) != "wxITEM_NORMAL")
    {
        code.Comma().CheckLineLength().QuotedString(prop_help).Comma().Add(prop_kind);
    }
    code.EndFunction();

    return true;
}

bool MenuItemGenerator::SettingsCode(Code& code)
{
    Node* node = code.node();
    if (code.hasValue(prop_extra_accels))
    {
        tt_string_vector accel_list;
        accel_list.SetString(node->as_string(prop_extra_accels), '"', tt::TRIM::both);

        if (code.is_cpp())
        {
            // auto_indent = indent::auto_keep_whitespace;
            code.OpenBrace().Add("wxAcceleratorEntry entry;").Eol();

            bool is_old_widgets = (Project.as_string(prop_wxWidgets_version) == "3.1");
            if (is_old_widgets)
            {
                code += "#if wxCHECK_VERSION(3, 1, 6)\n";
            }

            for (auto& accel: accel_list)
            {
                if (accel.size())
                {
                    code.Eol(eol_if_needed) << "if (entry.FromString(" << GenerateQuotedString(accel) << "))";
                    code.Eol().Tab().NodeName().Function("AddExtraAccel(entry").EndFunction();
                }
            }

            if (is_old_widgets)
            {
                code.Eol(eol_if_needed) += "#endif";
            }
            code.CloseBrace();
            code.UpdateBreakAt();
        }
        else  // python
        {
            code.Str("entry = ").Add("wxAcceleratorEntry()").Eol();
            for (auto& accel: accel_list)
            {
                code.Str("if entry.FromString(").QuotedString(accel).Str(") :").Eol();
                code.Tab().Str("menuQuit.AddExtraAccel(entry)").Eol();
            }
        }
    }

    if (code.hasValue(prop_bitmap))
    {
        code.Eol(eol_if_empty);
        if (code.is_cpp())
        {
            auto& description = node->as_string(prop_bitmap);
            bool is_vector_code = GenerateVectorCode(description, code.GetCode());
            code.UpdateBreakAt();

            if (!is_vector_code)
            {
                code.NodeName().Function("SetBitmap(");
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
                code.Tab().NodeName().Function("SetBitmap(");
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

        // wxPython version
        else
        {
            bool is_list_created = PythonBitmapList(code, prop_bitmap);
            code.NodeName().Function("SetBitmap(");
            if (is_list_created)
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
            else
            {
                PythonBundleCode(code, prop_bitmap);
            }
            code.EndFunction();
        }
    }
    if (code.hasValue(prop_unchecked_bitmap))
    {
        code.Eol();
        code.AddComment("Set the unchecked bitmap").Eol();
        if (code.is_cpp())
        {
            auto& description = node->as_string(prop_unchecked_bitmap);
            bool is_vector_code = GenerateVectorCode(description, code.GetCode());
            code.UpdateBreakAt();

            if (!is_vector_code)
            {
                code.NodeName().Function("SetBitmap(");
                if (Project.as_string(prop_wxWidgets_version) != "3.1")
                {
                    GenerateBundleCode(description, code.GetCode());
                    code.UpdateBreakAt();
                    code.Comma() += "false";
                }
                else
                {
                    code += "\n#if wxCHECK_VERSION(3, 1, 6)\n\t";
                    GenerateBundleCode(description, code.GetCode());
                    code += "\n#else\n\t";
                    code << "wxBitmap(" << GenerateBitmapCode(description) << ", false)\n";
                    code << "#endif\n";
                }
                code.UpdateBreakAt();
                code.EndFunction();
            }
            else
            {
                code.Tab().NodeName().Function("SetBitmap(");
                if (Project.as_string(prop_wxWidgets_version) != "3.1")
                {
                    code += "wxBitmapBundle::FromBitmaps(bitmaps)";
                    code.UpdateBreakAt();
                    code.Comma() += "false";
                    code.EndFunction();
                    code += "\n}\n";
                }
                else
                {
                    code += "\n#if wxCHECK_VERSION(3, 1, 6)\n\t";
                    code.Tab() += "wxBitmapBundle::FromBitmaps(bitmaps), false";
                    code += "\n#else\n\t";
                    code.Tab() << "wxBitmap(" << GenerateBitmapCode(description) << ", false)\n";
                    code << "#endif\n";
                    code.UpdateBreakAt();
                    code.Tab().EndFunction();
                }
            }
        }

        // wxPython version
        else
        {
            code.Eol(eol_if_needed);
            bool is_list_created = PythonBitmapList(code, prop_unchecked_bitmap);
            code.NodeName().Function("SetBitmap(");
            if (is_list_created)
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
            else
            {
                PythonBundleCode(code, prop_bitmap);
            }
            code.Comma() += "False";
            code.EndFunction();
        }
    }

    if (!node->getParent()->isGen(gen_PopupMenu))
    {
        code.Eol(eol_if_empty).ParentName().Function("Append(").NodeName().EndFunction();
    }

    if ((node->as_string(prop_kind) == "wxITEM_CHECK" || node->as_string(prop_kind) == "wxITEM_RADIO") && code.IsTrue(prop_checked))
    {
        code.Eol(eol_if_empty).NodeName().Function("Check(").EndFunction();
    }

    return true;
}

bool MenuItemGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);
    if (node->hasValue(prop_extra_accels))
    {
        InsertGeneratorInclude(node, "#include <wx/accel.h>", set_src, set_hdr);
    }

    return true;
}

int MenuItemGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

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
    if (node->hasValue(prop_extra_accels))
    {
        auto child = item.append_child("extra-accels");
        tt_string_vector accel_list;
        accel_list.SetString(node->as_string(prop_extra_accels), '"', tt::TRIM::both);
        for (auto& accel: accel_list)
        {
            if (accel.size())
            {
                child.append_child("accel").text().set(accel);
            }
        }
    }
    ADD_ITEM_PROP(prop_help, "help")
    ADD_ITEM_BOOL(prop_checked, "checked")
    if (node->as_bool(prop_disabled))
        item.append_child("enabled").text().set("0");

    if (node->as_string(prop_kind) == "wxITEM_RADIO")
        item.append_child("radio").text().set("1");
    else if (node->as_string(prop_kind) == "wxITEM_CHECK")
        item.append_child("checkable").text().set("1");

    GenXrcBitmap(node, item, xrc_flags);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void MenuItemGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop)
{
    if (changed_prop->isProp(prop_stock_id))
    {
        if (auto pg_setting = prop_grid->GetProperty(map_PropNames[prop_label]); pg_setting)
        {
            pg_setting->Enable(changed_prop->value() == "none");
        }
        if (auto pg_setting = prop_grid->GetProperty(map_PropNames[prop_help]); pg_setting)
        {
            pg_setting->Enable(changed_prop->value() == "none");
        }
        if (auto pg_setting = prop_grid->GetProperty(map_PropNames[prop_id]); pg_setting)
        {
            pg_setting->Enable(changed_prop->value() == "none");
        }
    }
}

bool MenuItemGenerator::modifyProperty(NodeProperty* prop, tt_string_view value)
{
    if (prop->isProp(prop_stock_id))
    {
        if (value != "none")
        {
            auto undo_stock_id = std::make_shared<ModifyProperties>("Stock ID");
            undo_stock_id->addProperty(prop, value);
            undo_stock_id->addProperty(prop->getNode()->getPropPtr(prop_label),
                                       wxGetStockLabel(NodeCreation.getConstantAsInt(value.as_str())).utf8_string());
            undo_stock_id->addProperty(prop->getNode()->getPropPtr(prop_help),
                                       wxGetStockHelpString(NodeCreation.getConstantAsInt(value.as_str())).utf8_string());
            undo_stock_id->addProperty(prop->getNode()->getPropPtr(prop_id), value);

            if (auto result = map_id_artid.find(value.as_str()); result != map_id_artid.end())
            {
                undo_stock_id->addProperty(prop->getNode()->getPropPtr(prop_bitmap),
                                           tt_string("Art;") << result->second << "|wxART_MENU");
            }
            wxGetFrame().PushUndoAction(undo_stock_id);
            return true;
        }
    }
    return false;
}
