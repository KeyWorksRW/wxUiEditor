/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class modification functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "propgrid_panel.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_view_vector -- read/write line-oriented strings/files
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction

void PropGridPanel::ModifyProperty(NodeProperty* prop, tt_string_view str)
{
    m_isPropChangeSuspended = true;
    if (prop->get_name() == prop_id)
    {
        tt_string id_string = str;
        wxWindowID id = static_cast<wxWindowID>(NodeCreation.get_ConstantAsInt(id_string, -1));
        if (wxIsStockID(id))
        {
            auto node = prop->getNode();
            if (node->HasProp(prop_label))
            {
                auto prop_ptr = node->get_PropPtr(prop_label);
                if (prop_ptr->as_string() == prop_ptr->getDefaultValue())
                {
                    prop_ptr->set_value("");
                    // Find prop_ptr as a value in m_property_map and clear it in the property grid
                    for (const auto& [pg_property, value]: m_property_map)
                    {
                        if (value == prop_ptr)
                        {
                            m_prop_grid->SetPropertyValue(pg_property, "");
                            break;
                        }
                    }
                }
            }
        }
    }

    if (auto* gen = prop->getNode()->get_Generator(); !gen || !gen->ModifyProperty(prop, str))
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, str));
    }
    m_isPropChangeSuspended = false;
}

void PropGridPanel::ModifyBitlistProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    auto node = node_prop->getNode();

    tt_string value = m_prop_grid->GetPropertyValueAsString(grid_prop);
    value.Replace(" ", "", true);
    value.Replace(",", "|", true);
    if (node_prop->isProp(prop_style))
    {
        // Don't allow the user to combine incompatible styles
        if (value.contains("wxFLP_OPEN") && value.contains("wxFLP_SAVE"))
        {
            auto style_prop = node->get_PropPtr(prop_style);
            auto& old_value = style_prop->as_string();
            if (old_value.contains("wxFLP_OPEN"))
            {
                value.Replace("wxFLP_OPEN", "");
                value.Replace("wxFLP_FILE_MUST_EXIST", "");
                value.Replace("||", "|", true);  // Fix all cases of a doubled pipe

                // Change the format to what the grid_prop grid wants
                value.Replace("|", ",");
                m_prop_grid->SetPropertyValue("style", value.make_wxString());

                // Now put it back into the format we use internally
                value.Replace(",", "|");
            }
            else
            {
                value.Replace("wxFLP_SAVE", "");
                value.Replace("wxFLP_OVERWRITE_PROMPT", "");
                value.Replace("||", "|", true);  // Fix all cases of a doubled pipe

                // Change the format to what the grid_prop grid wants
                value.Replace("|", ",");
                m_prop_grid->SetPropertyValue("style", value.make_wxString());

                // Now put it back into the format we use internally
                value.Replace(",", "|");
            }
        }
    }
    else if (node_prop->isProp(prop_generate_languages))
    {
        ModifyProperty(node_prop, value);
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().UpdateLanguagePanels();
        return;
    }
    else if (node_prop->isProp(prop_window_style) && value.empty())
    {
        value = "0";
    }
    ModifyProperty(node_prop, value);
}

void PropGridPanel::ModifyBoolProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    if (!m_prop_grid->GetPropertyValueAsBool(grid_prop))
    {
        auto node = node_prop->getNode();
        if (node->is_Gen(gen_wxStdDialogButtonSizer))
        {
            auto def_prop = node->get_PropPtr(prop_default_button);
            if (def_prop->as_string() == node_prop->get_DeclName())
            {
                m_prop_grid->SetPropertyValue("default_button", "none");
                ModifyProperty(def_prop, "none");
            }
        }
    }
    ModifyProperty(node_prop, m_prop_grid->GetPropertyValueAsBool(grid_prop) ? "1" : "0");
}

void PropGridPanel::ModifyEmbeddedProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    // Do NOT call GetPropertyValueAsString() -- we need to return the value the way the custom
    // property formatted it
    tt_string value = m_prop_grid->GetPropertyValue(grid_prop).GetString().utf8_string();
    tt_string_vector parts(value, BMP_PROP_SEPARATOR, tt::TRIM::both);
    // If the image field is empty, then the entire property needs to be cleared
    if (parts.size() <= IndexImage || parts[IndexImage].empty())
    {
        value.clear();
    }
    else if (!value.starts_with("Art"))
    {
        tt_string image_path(parts[IndexImage]);
        auto* embed = ProjectImages.GetEmbeddedImage(image_path);
        if (embed && image_path == embed->imgs[0].filename)
        {
            // If the user is adding a node to a gen_Images node, then be sure that the embed
            // entry form is pointing to the gen_Images node.
            if (node_prop->getNode()->is_Gen(gen_embedded_image))
            {
                embed->form = node_prop->getNode()->get_Parent();
            }
            // If there is an Images List node and it is set to auto add, then fall through
            // to the section below that adds the image to the Images List node.
            else if (!Project.get_ImagesForm() || !Project.get_ImagesForm()->as_bool(prop_auto_add))
            {
                ModifyProperty(node_prop, value);
                return;
            }
        }
        else
        {
            // This ensures that all images from a bitmap bundle get added
            ProjectImages.UpdateBundle(parts, node_prop->getNode());
        }
    }

    if (value.empty() || node_prop->type() == type_animation || value.starts_with("Art") ||
        value.starts_with("XPM"))
    {
        ModifyProperty(node_prop, value);
        return;  // Don't do anything else for animations, art providers or XPMs
    }
    if (value == "Embed;" || value == "SVG;")
    {
        // Don't do anything else for empty embedded images
        ModifyProperty(node_prop, value);
        return;
    }

    // We do *not* call ModifyProperty() until we are certain that we aren't going to add an
    // image to a gen_Images node. That's because if we do add it, the GroupUndoActions will
    // handle the modification of the property via an ModifyPropertyAction class.

    auto* node = node_prop->getNode();
    auto* parent = node->get_Parent();

    if (parent->is_Gen(gen_Images))
    {
        auto filename = parts[IndexImage].filename();
        size_t pos = 0;
        for (const auto& embedded_image: parent->get_ChildNodePtrs())
        {
            auto& description_a = embedded_image->as_string(prop_bitmap);
            tt_view_vector parts_a(description_a, BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts_a.size() <= IndexImage || parts_a[IndexImage].empty())
                break;
            if (filename.compare(parts_a[IndexImage].filename()) < 0)
                // We found the position where the new image should be inserted
                break;
            ++pos;
        }
        if (pos < parent->get_ChildCount())
        {
            auto group = std::make_shared<GroupUndoActions>("Update bitmap property", node);

            auto prop_bitmap_action = std::make_shared<ModifyPropertyAction>(node_prop, value);
            prop_bitmap_action->AllowSelectEvent(false);
            group->Add(prop_bitmap_action);

            auto change_pos_action = std::make_shared<ChangePositionAction>(node, pos);
            group->Add(change_pos_action);
            wxGetFrame().PushUndoAction(group);
            return;  // The group Undo will handle modifying the bitmap property, so simply return
        }
    }
    else
    {
        auto image_list_node = Project.get_ImagesForm();
        if (!image_list_node || !image_list_node->as_bool(prop_auto_add))
        {
            ModifyProperty(node_prop, value);
            return;
        }
        auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (image_list_node && embed && embed->form != image_list_node)
        {
            embed->form = image_list_node;
            auto filename = parts[IndexImage].filename();
            size_t pos = 0;
            for (const auto& embedded_image: image_list_node->get_ChildNodePtrs())
            {
                auto& description_a = embedded_image->as_string(prop_bitmap);
                tt_view_vector parts_a(description_a, BMP_PROP_SEPARATOR, tt::TRIM::both);
                if (parts_a.size() <= IndexImage || parts_a[IndexImage].empty())
                    break;
                if (filename.compare(parts_a[IndexImage].filename()) < 0)
                    // We found the position where the new image should be inserted
                    break;
                ++pos;
            }

            auto group = std::make_shared<GroupUndoActions>("Update bitmap property", node);

            auto new_embedded = NodeCreation.CreateNode(gen_embedded_image, image_list_node).first;
            new_embedded->set_value(prop_bitmap, value);
            auto insert_action = std::make_shared<InsertNodeAction>(
                new_embedded.get(), image_list_node, tt_empty_cstr, pos);
            insert_action->AllowSelectEvent(false);
            insert_action->SetFireCreatedEvent(true);
            group->Add(insert_action);

            auto prop_bitmap_action = std::make_shared<ModifyPropertyAction>(node_prop, value);
            prop_bitmap_action->AllowSelectEvent(false);
            group->Add(prop_bitmap_action);

            wxGetFrame().PushUndoAction(group);
            return;  // The group action will handle modifying the bitmap property, so simply return
        }
    }

    // If we get here, then we didn't find an Images node at all, or it didn't need updating,
    // so just modify the property.
    ModifyProperty(node_prop, value);
}

void PropGridPanel::ModifyFileProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    if (node_prop->isProp(prop_data_file))
    {
        tt_string newValue =
            grid_prop->GetValueAsString(wxPGPropValFormatFlags::FullValue).utf8_string();
        auto result = Project.GetOutputPath(node_prop->getNode()->get_Form(), GEN_LANG_CPLUSPLUS);
        auto path = result.first;
        if (result.second)  // true if the the base filename was returned
            path.remove_filename();
        newValue.make_relative(path);
        newValue.backslashestoforward();
        ModifyProperty(node_prop, newValue);
        return;
    }

    tt_string newValue = grid_prop->GetValueAsString().utf8_string();

    // The base_file grid_prop was already processed in OnPropertyGridChanging so only modify the
    // value if it's a different grid_prop
    if (!node_prop->isProp(prop_base_file) && !node_prop->isProp(prop_perl_file) &&
        !node_prop->isProp(prop_python_file) && !node_prop->isProp(prop_ruby_file) &&
        !node_prop->isProp(prop_xrc_file) && !node_prop->isProp(prop_rust_file))
    {
        if (newValue.size())
        {
            newValue.make_absolute();
            newValue.make_relative(Project.get_ProjectPath());
            newValue.backslashestoforward();
            grid_prop->SetValueFromString(newValue);
        }
    }
    ModifyProperty(node_prop, newValue);

    // Review: [Randalphwa - 06-26-2023] The panel should already have been created
    // wxGetFrame().UpdateLanguagePanels();
}

void PropGridPanel::ModifyOptionsProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    auto node = node_prop->getNode();

    tt_string value = m_prop_grid->GetPropertyValueAsString(grid_prop).utf8_string();
    ModifyProperty(node_prop, value);

    // Update displayed description for the new selection
    auto propInfo = node_prop->get_PropDeclaration();

    tt_string description = GetPropHelp(node_prop);

    for (auto& iter: propInfo->getOptions())
    {
        if (iter.name == value)
        {
            if (iter.help.empty())
                description = value + "\n";
            else
                description += "\n\n" + value + "\n" + iter.help;

            break;
        }
    }

    m_prop_grid->SetPropertyHelpString(grid_prop, description);
    m_prop_grid->SetDescription(grid_prop->GetLabel(), description);

    if (auto selected_node = wxGetFrame().getSelectedNode(); selected_node)
    {
        if (node_prop->isProp(prop_class_access) && wxGetApp().isPjtMemberPrefix())
        {
            tt_string name = node->as_string(prop_var_name);
            bool is_name_changed = false;
            auto access = &value;  // this is just to make the code more readable
            if (Project.get_CodePreference(selected_node) == GEN_LANG_CPLUSPLUS)
            {
                // If access is changed to local and the name starts with "m_", then
                // the "m_" will be stripped off. Conversely, if the name is changed
                // from local to a class member, a "m_" is added as a prefix if
                // preferred language isw C++.

                if (*access == "none" && name.starts_with("m_"))
                {
                    name.erase(0, 2);
                    auto final_name = node->get_UniqueName(name);
                    if (final_name.size())
                        name = final_name;
                    is_name_changed = true;
                }
                else if (*access != "none" && !name.starts_with("m_") &&
                         Project.get_CodePreference(selected_node) == GEN_LANG_CPLUSPLUS)
                {
                    name.insert(0, "m_");
                    auto final_name = node->get_UniqueName(name);
                    if (final_name.size())
                        name = final_name;
                    is_name_changed = true;
                }
            }
            else if (Project.get_CodePreference(selected_node) == GEN_LANG_PYTHON)
            {
                // The convention in Python is to use a leading underscore for local members.

                if (*access == "none" && !name.starts_with("_"))
                {
                    name.insert(0, "_");
                    if (auto final_name = node->get_UniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
                else if (*access != "none" && name.starts_with("_"))
                {
                    name.erase(0, 1);
                    if (auto final_name = node->get_UniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
            }
            else if (Project.get_CodePreference(selected_node) == GEN_LANG_RUBY)
            {
                // The convention in Ruby is to use a leading @ for non-local members.

                if (*access == "none" && name.starts_with("@"))
                {
                    name.erase(0, 1);
                    if (auto final_name = node->get_UniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
                else if (*access != "none" && !name.starts_with("@"))
                {
                    name.insert(0, "@");
                    if (auto final_name = node->get_UniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
            }

            if (is_name_changed)
            {
                auto propChange = selected_node->get_PropPtr(prop_var_name);
                auto grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                grid_property->SetValueFromString(name);
                ModifyProperty(propChange, name);
            }
        }
    }
}
