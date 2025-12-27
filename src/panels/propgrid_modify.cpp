/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class modification functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "prop_decl.h"
#include "propgrid_panel.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "image_handler.h"    // ImageHandler class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties

#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector
#include "wxue_namespace/wxue_view_vector.h"    // wxue::ViewVector

void PropGridPanel::ModifyProperty(NodeProperty* prop, const wxString& str)
{
    m_isPropChangeSuspended = true;
    if (prop->get_name() == prop_id)
    {
        auto win_id =
            static_cast<wxWindowID>(NodeCreation.get_ConstantAsInt(str.ToStdString(), -1));
        if (wxIsStockID(win_id))
        {
            auto* node = prop->getNode();
            if (node->HasProp(prop_label))
            {
                auto* prop_ptr = node->get_PropPtr(prop_label);
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

    if (auto* gen = prop->getNode()->get_Generator();
        (gen == nullptr) || !gen->ModifyProperty(prop, str.ToStdString()))
    {
        wxGetFrame().PushUndoAction(
            std::make_shared<ModifyPropertyAction>(prop, str.ToStdString()));
    }
    m_isPropChangeSuspended = false;
}

void PropGridPanel::ModifyBitlistProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    auto* node = node_prop->getNode();

    auto value = m_prop_grid->GetPropertyValueAsString(grid_prop);
    value.Replace(" ", "", true);
    value.Replace(",", "|", true);
    if (node_prop->isProp(prop_style))
    {
        // Don't allow the user to combine incompatible styles
        if (value.Contains("wxFLP_OPEN") && value.Contains("wxFLP_SAVE"))
        {
            auto* style_prop = node->get_PropPtr(prop_style);
            const auto& old_value = style_prop->as_wxString();
            if (old_value.Contains("wxFLP_OPEN"))
            {
                value.Replace("wxFLP_OPEN", "");
                value.Replace("wxFLP_FILE_MUST_EXIST", "");
                value.Replace("||", "|", true);  // Fix all cases of a doubled pipe

                // Change the format to what the grid_prop grid wants
                value.Replace("|", ",");
                m_prop_grid->SetPropertyValue("style", value);

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
                m_prop_grid->SetPropertyValue("style", value);

                // Now put it back into the format we use internally
                value.Replace(",", "|");
            }
        }
    }
    if (node_prop->isProp(prop_generate_languages))
    {
        ModifyProperty(node_prop, value);
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().UpdateLanguagePanels();
        return;
    }
    if (node_prop->isProp(prop_window_style) && value.empty())
    {
        value = "0";
    }
    ModifyProperty(node_prop, value);
}

void PropGridPanel::ModifyBoolProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    if (!m_prop_grid->GetPropertyValueAsBool(grid_prop))
    {
        auto* node = node_prop->getNode();
        if (node->is_Gen(gen_wxStdDialogButtonSizer))
        {
            auto* def_prop = node->get_PropPtr(prop_default_button);
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
    auto value = m_prop_grid->GetPropertyValue(grid_prop).GetString();
    wxue::StringVector parts(value.utf8_string(), BMP_PROP_SEPARATOR, wxue::TRIM::both);
    // If the image field is empty, then the entire property needs to be cleared
    if (parts.size() <= IndexImage || parts[IndexImage].empty())
    {
        value.clear();
    }
    if (!value.starts_with("Art"))
    {
        const wxString& image_path(parts[IndexImage]);
        auto* embed = ProjectImages.GetEmbeddedImage(image_path.ToStdString());
        if ((embed != nullptr) && image_path == embed->base_image().filename)
        {
            // If the user is adding a node to a gen_Images node, then be sure that the embed
            // entry form is pointing to the gen_Images node.
            if (node_prop->getNode()->is_Gen(gen_embedded_image))
            {
                embed->set_Form(node_prop->getNode()->get_Parent());
            }
            // If there is an Images List node and it is set to auto add, then fall through
            // to the section below that adds the image to the Images List node.
            if ((Project.get_ImagesForm() == nullptr) ||
                !Project.get_ImagesForm()->as_bool(prop_auto_add))
            {
                ModifyProperty(node_prop, value);
                return;
            }
        }
        if (embed == nullptr || image_path != embed->base_image().filename)
        {
            // This ensures that all images from a bitmap bundle get added
            ProjectImages.UpdateBundle(&parts, node_prop->getNode());
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
            const auto& description_a = embedded_image->as_wxString(prop_bitmap);
            wxue::ViewVector parts_a(description_a.ToStdString(), BMP_PROP_SEPARATOR,
                                     wxue::TRIM::both);
            if (parts_a.size() <= IndexImage || parts_a[IndexImage].empty())
            {
                break;
            }
            if (filename.compare(parts_a[IndexImage].filename()) < 0)
            {
                // We found the position where the new image should be inserted
                break;
            }
            ++pos;
        }
        if (pos < parent->get_ChildCount())
        {
            auto group = std::make_shared<GroupUndoActions>("Update bitmap property", node);

            auto prop_bitmap_action =
                std::make_shared<ModifyPropertyAction>(node_prop, value.ToStdString());
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
        auto* image_list_node = Project.get_ImagesForm();
        if ((image_list_node == nullptr) || !image_list_node->as_bool(prop_auto_add))
        {
            ModifyProperty(node_prop, value);
            return;
        }
        auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if ((image_list_node != nullptr) && (embed != nullptr) &&
            embed->get_Form() != image_list_node)
        {
            embed->set_Form(image_list_node);
            wxString filename(parts[IndexImage].filename());
            size_t pos = 0;
            for (const auto& embedded_image: image_list_node->get_ChildNodePtrs())
            {
                const auto& description_a = embedded_image->as_wxString(prop_bitmap);
                wxue::ViewVector parts_a(description_a.ToStdString(), BMP_PROP_SEPARATOR,
                                         wxue::TRIM::both);
                if (parts_a.size() <= IndexImage || parts_a[IndexImage].empty())
                {
                    break;
                }
                if (filename.compare(parts_a[IndexImage].filename().as_str()) < 0)
                {
                    // We found the position where the new image should be inserted
                    break;
                }
                ++pos;
            }

            auto group = std::make_shared<GroupUndoActions>("Update bitmap property", node);

            auto new_embedded = NodeCreation.CreateNode(gen_embedded_image, image_list_node).first;
            new_embedded->set_value(prop_bitmap, value);
            auto insert_action = std::make_shared<InsertNodeAction>(
                new_embedded.get(), image_list_node, wxue::wxue_empty_string, pos);
            insert_action->AllowSelectEvent(false);
            insert_action->SetFireCreatedEvent(true);
            group->Add(insert_action);

            auto prop_bitmap_action =
                std::make_shared<ModifyPropertyAction>(node_prop, value.ToStdString());
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
        wxue::string newValue =
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

    wxue::string newValue = grid_prop->GetValueAsString().utf8_string();

    // The base_file grid_prop was already processed in OnPropertyGridChanging so only modify the
    // value if it's a different grid_prop
    if (!node_prop->isProp(prop_base_file) && !node_prop->isProp(prop_perl_file) &&
        !node_prop->isProp(prop_python_file) && !node_prop->isProp(prop_ruby_file) &&
        !node_prop->isProp(prop_xrc_file))
    {
        if (!newValue.empty())
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

namespace
{
    void UpdateDescription(const wxString& value, wxString& description, PropDeclaration* propInfo)
    {
        for (auto& iter: propInfo->getOptions())
        {
            if (iter.name == value.ToStdString())
            {
                if (iter.help.empty())
                {
                    description = value + "\n";
                }
                else
                {
                    description += "\n\n" + value + "\n" + std::string(iter.help);
                }
                break;
            }
        }
    }

    void UpdateCppMemberPrefix(bool access, wxString& name, Node* node, bool& is_name_changed)
    {
        if (access && name.starts_with("m_"))
        {
            name.erase(0, 2);
            auto final_name = node->get_UniqueName(name.ToStdString());
            if (!final_name.empty())
            {
                name = final_name;
            }
            is_name_changed = true;
        }
        if (!access && !name.starts_with("m_"))
        {
            name.insert(0, "m_");
            auto final_name = node->get_UniqueName(name.ToStdString());
            if (!final_name.empty())
            {
                name = final_name;
            }
            is_name_changed = true;
        }
    };

    void UpdatePythonMemberPrefix(bool access, wxString& name, Node* node, bool& is_name_changed)
    {
        // The convention in Python is to use a leading underscore for local members.
        if (!access && !name.starts_with("_"))
        {
            name.insert(0, "_");
            auto final_name = node->get_UniqueName(name.ToStdString());
            if (!final_name.empty())
            {
                name = final_name;
            }
            is_name_changed = true;
        }
        if (access && name.starts_with("_"))
        {
            name.erase(0, 1);
            auto final_name = node->get_UniqueName(name.ToStdString());
            if (!final_name.empty())
            {
                name = final_name;
            }
            is_name_changed = true;
        }
    };

    void UpdateRubyMemberPrefix(bool access, wxString& name, Node* node, bool& is_name_changed)
    {
        if (access && name.starts_with("@"))
        {
            name.erase(0, 1);
            auto final_name = node->get_UniqueName(name.ToStdString());
            if (!final_name.empty())
            {
                name = final_name;
            }
            is_name_changed = true;
        }
        if (!access && !name.starts_with("@"))
        {
            name.insert(0, "@");
            auto final_name = node->get_UniqueName(name.ToStdString());
            if (!final_name.empty())
            {
                name = final_name;
            }
            is_name_changed = true;
        }
    };
}  // namespace

void PropGridPanel::ModifyOptionsProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    auto* node = node_prop->getNode();

    auto value = m_prop_grid->GetPropertyValueAsString(grid_prop);
    ModifyProperty(node_prop, value);

    // Update displayed description for the new selection
    auto* propInfo = node_prop->get_PropDeclaration();

    wxString description = GetPropHelp(node_prop);
    UpdateDescription(value, description, propInfo);

    m_prop_grid->SetPropertyHelpString(grid_prop, description);
    m_prop_grid->SetDescription(grid_prop->GetLabel(), description);

    if (auto* selected_node = wxGetFrame().getSelectedNode(); selected_node)
    {
        if (node_prop->isProp(prop_class_access) && wxGetApp().isPjtMemberPrefix())
        {
            auto name = node->as_wxString(prop_var_name);
            bool is_name_changed = false;
            auto access = (value == "none");
            switch (Project.get_CodePreference(selected_node))
            {
                case GEN_LANG_CPLUSPLUS:
                    // If access is changed to local and the name starts with "m_", then the "m_"
                    // will be stripped off. Conversely, if the name is changed from local to a
                    // class member, a "m_" is added as a prefix if preferred language isw C++.
                    UpdateCppMemberPrefix(access, name, node, is_name_changed);
                    break;

                case GEN_LANG_PYTHON:
                    // The convention in Python is to use a leading underscore for local members.
                    UpdatePythonMemberPrefix(access, name, node, is_name_changed);
                    break;

                case GEN_LANG_RUBY:
                    // The convention in Ruby is to use a leading @ for non-local members.
                    UpdateRubyMemberPrefix(access, name, node, is_name_changed);
                    break;

                default:
                    {
                        auto lang_str = GenLangToString(Project.get_CodePreference(selected_node));
                        FAIL_MSG(wxString("Unsupported language: ")
                                 << wxString(lang_str.data(), lang_str.size()));
                    }
                    return;  // Only C++, Python and Ruby have naming conventions for members
            }

            if (is_name_changed)
            {
                auto* propChange = selected_node->get_PropPtr(prop_var_name);
                auto* grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                grid_property->SetValueFromString(name);
                ModifyProperty(propChange, name);
            }
        }
    }
}
