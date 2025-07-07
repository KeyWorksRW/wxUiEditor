/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class Creation functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "propgrid_panel.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

// Various customized wxPGProperty classes

#include "../customprops/code_single_prop.h"    // Derived wxStringProperty class for code
#include "../customprops/code_string_prop.h"    // Derived wxStringProperty class for code
#include "../customprops/custom_colour_prop.h"  // Property editor for colour
#include "../customprops/custom_param_prop.h"   // dialog for editing CustomControl parameter
#include "../customprops/directory_prop.h"      // DirectoryDialogAdapter
#include "../customprops/edit_custom_mockup.h"  // Custom Property editor for pop_custom_mockup
#include "../customprops/font_string_prop.h"    // FontStringProperty
#include "../customprops/html_string_prop.h"    // EditHtmlProperty
#include "../customprops/id_prop.h"             // ID_Property
#include "../customprops/include_files_prop.h"  // IncludeFilesProperty
#include "../customprops/pg_animation.h"        // Custom property grid class for animations
#include "../customprops/pg_image.h"            // Custom property grid class for images
#include "../customprops/pg_point.h"            // custom wxPGProperty for handling wxPoint
#include "../customprops/rearrange_prop.h"      // Property editor for rearranging items
#include "../customprops/sb_fields_prop.h"      // Property editor for status bar fields
#include "../customprops/sizer_grow_columns.h"  // Property editor for Growable Sizer Columns
#include "../customprops/sizer_grow_rows.h"     // Property editor for Growable Sizer Columns
#include "../customprops/tt_file_property.h"    // ttFileProperty -- Property editor for file names
#include "../customprops/txt_string_prop.h"     // dialog for editing single-line strings

extern std::map<GenLang, std::string> s_lang_category_prefix;

void PropGridPanel::Create()
{
    if (m_locked)
        return;

    if (auto node = wxGetFrame().getSelectedNode(); node)
    {
        wxWindowUpdateLocker freeze(this);

#if defined(_DEBUG)
        if (wxGetApp().isFireCreationMsgs())
        {
            MSG_INFO("Property window recreated.");
        }
#endif  // _DEBUG

        wxGetMainFrame()->SetStatusText(wxEmptyString, 2);

        m_currentSel = node;
        m_preferred_lang = Project.getCodePreference(node);

        wxString pageName;
        if (int pageNumber = m_prop_grid->GetSelectedPage(); pageNumber != wxNOT_FOUND)
        {
            pageName = m_prop_grid->GetPageName(pageNumber);
        }

        // Note that AddPage() won't actually add a page, it simply sets an internal flag to
        // indicate there is one page. That's required for m_prop_grid->Clear() to work -- because
        // Clear() *only* clears pages.

        m_prop_grid->Clear();
        m_prop_grid->AddPage();
        m_event_grid->Clear();
        m_event_grid->AddPage();

        m_property_map.clear();
        m_event_map.clear();

        tt_string lang_created;

        if (auto declaration = node->getNodeDeclaration(); declaration)
        {
            // These sets are used to prevent trying to add a duplicate property or event to the
            // property grid. In Debug builds, attempting to do so will generate an assert message
            // telling you the name of the duplicate and the node declaration it occurs in. In
            // release builds, only the first instance will be displayed.

            PropNameSet prop_set;
            EventSet event_set;

            CreatePropCategory(declaration->declName(), node, declaration, prop_set);
            CreateEventCategory(declaration->declName(), node, declaration, event_set);

            // Calling GetBaseClassCount() is exepensive, so do it once and store the result
            auto num_base_classes = declaration->GetBaseClassCount();

            auto lang_prefix = GenLangToString(Project.getCodePreference());

            if (node->isForm() || node->isGen(gen_Project))
            {
                bool lang_found = false;
                size_t lang_start = 0;
                for (size_t i = 0; i < num_base_classes; i++)
                {
                    auto* info_base = declaration->GetBaseClass(i);
                    if (info_base->isGen(gen_sizer_child))
                        continue;
                    if (!lang_found)
                    {
                        // There are a few forms like gen_wxDialog which have have a category
                        // that appears *before* the various language categories. All
                        // non-language categories need to be created in the same order as they
                        // were specified in the XML interface file, so create those here if we
                        // haven't seen a language category yet.

                        for (auto& iter: s_lang_category_prefix)
                        {
                            if (info_base->declName().contains(iter.second))
                            {
                                lang_found = true;
                                lang_start = i;  // save this for the for loop used later
                                break;
                            }
                        }
                        if (!lang_found)
                        {
                            if (!info_base->declName().is_sameas("Window Events"))
                            {
                                CreatePropCategory(info_base->declName(), node, info_base,
                                                   prop_set);
                            }
                            else
                                CreateEventCategory(info_base->declName(), node, info_base,
                                                    event_set);
                            continue;
                        }
                    }

                    // We get here if we've seen a language category, so we check to see if it
                    // is the preferred language, and if so, create it now and break out of the
                    // loop.
                    if (info_base->declName().is_sameprefix(lang_prefix))
                    {
                        CreatePropCategory(info_base->declName(), node, info_base, prop_set);

                        // C++ settings are divided into three categories in consecutive order,
                        // so we need to create the other two categories here if the preferred
                        // language is C++.

                        if (m_preferred_lang == GEN_LANG_CPLUSPLUS &&
                            info_base->declName().contains("Settings"))
                        {
                            info_base = declaration->GetBaseClass(++i);
                            CreatePropCategory(info_base->declName(), node, info_base, prop_set);
                            info_base = declaration->GetBaseClass(++i);
                            CreatePropCategory(info_base->declName(), node, info_base, prop_set);
                        }

                        break;
                    }
                }

                // At this point, we've created any pre-language categories, and the preferred
                // language categories. Now we create any remaining categories.
                for (; lang_start < num_base_classes; lang_start++)
                {
                    auto* info_base = declaration->GetBaseClass(lang_start);
                    if (info_base->isGen(gen_sizer_child))
                        continue;
                    if (!info_base->declName().is_sameas("Window Events"))
                    {
                        if (info_base->declName().is_sameprefix(lang_prefix))
                        {
                            if (m_preferred_lang == GEN_LANG_CPLUSPLUS &&
                                info_base->declName().contains("Settings"))
                            {
                                lang_start +=
                                    2;  // skip over Header Settings and Derived Class Settings
                            }
                            continue;  // already added above
                        }
                        CreatePropCategory(info_base->declName(), node, info_base, prop_set);
                    }
                    CreateEventCategory(info_base->declName(), node, info_base, event_set);
                }
            }
            else
            {
                for (size_t i = 0; i < num_base_classes; i++)
                {
                    auto* info_base = declaration->GetBaseClass(i);
                    if (info_base->isGen(gen_sizer_child))
                        continue;
                    if (!info_base->declName().is_sameas("Window Events"))
                    {
                        if ((node->isForm() || node->isGen(gen_Project)) &&
                            info_base->declName().is_sameprefix(lang_prefix))
                            continue;  // already added above
                        CreatePropCategory(info_base->declName(), node, info_base, prop_set);
                    }
                    CreateEventCategory(info_base->declName(), node, info_base, event_set);
                }
            }
            if (node->isSpacer())
            {
                if (node->isParent(gen_wxGridBagSizer))
                    CreateLayoutCategory(node);
            }
            else if (node->getParent() && node->getParent()->isSizer())
            {
                CreateLayoutCategory(node);
            }

            if (m_prop_grid->GetPageCount() > 0)
            {
                int pageIndex = m_prop_grid->GetPageByName(pageName);
                if (wxNOT_FOUND != pageIndex)
                {
                    m_prop_grid->SelectPage(pageIndex);
                }
                else
                {
                    m_prop_grid->SelectPage(0);
                }
            }
            m_prop_grid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, (long) 1);
        }

        ReselectItem();

        m_prop_grid->Refresh();
        m_prop_grid->Update();
        m_event_grid->Refresh();
        m_event_grid->Update();
    }
}

void PropGridPanel::CreateEventCategory(tt_string_view name, Node* node,
                                        NodeDeclaration* declaration, EventSet& event_set)
{
    auto& category = declaration->GetCategory();

    if (!category.getCategoryCount() && !category.getEventCount())
        return;

    if (category.GetName() == "wxWindow")
    {
        if (node->getNodeDeclaration()->GetGeneratorFlags().contains("no_win_events"))
            return;
    }

    auto id =
        m_event_grid->Append(new wxPropertyCategory(GetCategoryDisplayName(category.GetName())));

    AddEvents(name, node, category, event_set);

    if (auto it = m_expansion_map.find(GetCategoryDisplayName(category.GetName()).ToStdString());
        it != m_expansion_map.end())
    {
        if (it->second)
        {
            m_event_grid->Expand(id);
        }
        else
        {
            m_event_grid->Collapse(id);
        }
    }
}

// clang-format off
static constexpr std::initializer_list<PropName> lst_LayoutProps = {

    prop_alignment,
    prop_borders,
    prop_border_size,
    prop_scale_border_size,
    prop_flags

};

static constexpr std::initializer_list<PropName> lst_GridBagProps = {

    prop_borders,
    prop_border_size,
    prop_scale_border_size,
    prop_flags,
    prop_row,
    prop_column,
    prop_rowspan,
    prop_colspan

};
// clang-format on

void PropGridPanel::CreateLayoutCategory(Node* node)
{
    auto id = m_prop_grid->Append(new wxPropertyCategory("Layout"));

    if (!node->isParent(gen_wxGridBagSizer))
    {
        for (auto iter: lst_LayoutProps)
        {
            auto prop = node->getPropPtr(iter);
            if (!prop)
                continue;

            auto id_prop = m_prop_grid->Append(CreatePGProperty(prop));

            auto description = GetPropHelp(prop);
            m_prop_grid->SetPropertyHelpString(id_prop, description);

            m_property_map[id_prop] = prop;
            if (prop->isProp(prop_alignment))
            {
                prop->getNode()->getGenerator()->ChangeEnableState(m_prop_grid, prop);
            }
        }

        if (auto prop = node->getPropPtr(prop_proportion); prop)
        {
            auto id_prop = m_prop_grid->Append(CreatePGProperty(prop));

            auto description = GetPropHelp(prop);
            m_prop_grid->SetPropertyHelpString(id_prop, description);

            m_property_map[id_prop] = prop;
        }
    }
    else
    {
        for (auto iter: lst_GridBagProps)
        {
            auto prop = node->getPropPtr(iter);
            if (!prop)
                continue;

            auto id_prop = m_prop_grid->Append(CreatePGProperty(prop));

            auto description = GetPropHelp(prop);
            m_prop_grid->SetPropertyHelpString(id_prop, description);

            m_property_map[id_prop] = prop;
        }
    }

    m_prop_grid->Expand(id);

    if (UserPrefs.is_DarkMode())
        m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#1d677c"));
    else
        m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#e1f3f8"));
}

wxPGProperty* PropGridPanel::CreatePGProperty(NodeProperty* prop)
{
    auto type = prop->type();

    switch (type)
    {
        case type_id:
            return new ID_Property(prop->declName().make_wxString(), prop);

        case type_int:
            return new wxIntProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_int());

        case type_uint:
            return new wxUIntProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_int());

        case type_statbar_fields:
            // This includes a button that triggers a dialog to edit the fields.
            return new SBarFieldsProperty(prop->declName().make_wxString(), prop);

        case type_checklist_item:
            // This includes a button that triggers a dialog to edit the fields.
            return new RearrangeProperty(prop->declName().make_wxString(), prop);

        case type_string_code_grow_columns:
            return new GrowColumnsProperty(prop->declName().make_wxString(), prop);

        case type_string_code_grow_rows:
            return new GrowRowsProperty(prop->declName().make_wxString(), prop);

        case type_string_code_cstm_param:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditParamProperty(prop->declName().make_wxString(), prop);

        case type_string_code_single:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditCodeSingleProperty(prop->declName().make_wxString(), prop);

        case type_string_escapes:
            // This first doubles the backslash in escaped characters: \n, \t, \r, and \.
            return new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                        prop->as_escape_text().make_wxString());

        case type_string:
            return new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                        prop->as_wxString());

        case type_string_edit_escapes:
            // This includes a button that triggers a small text editor dialog
            // This doubles the backslash in escaped characters: \n, \t, \r, and \.
            return new wxLongStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                            prop->as_escape_text().make_wxString());

        case type_string_edit:
            // This includes a button that triggers a small text editor dialog
            return new wxLongStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                            prop->as_wxString());

        case type_string_edit_single:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditStringProperty(prop->declName().make_wxString(), prop);

        case type_code_edit:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditCodeProperty(prop->declName().make_wxString(), prop);

        case type_custom_mockup:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditCustomMockupProperty(prop->declName().make_wxString(), prop);

        case type_html_edit:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditHtmlProperty(prop->declName().make_wxString(), prop);

        case type_include_files:
            // This includes a button that triggers a custom dialog
            return new IncludeFilesProperty(prop->declName().make_wxString(), prop);

        case type_bool:
            return new wxBoolProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                      prop->as_string() == "1");

        case type_wxPoint:
            return new CustomPointProperty(prop->declName().make_wxString(), prop,
                                           CustomPointProperty::type_point);

        case type_wxSize:
            return new CustomPointProperty(prop->declName().make_wxString(), prop,
                                           CustomPointProperty::type_size);

        case type_wxFont:
            // This includes a button that triggers a custom font selector dialog
            return new FontStringProperty(prop->declName().make_wxString(), prop);

        case type_path:
            return new DirectoryProperty(prop->declName().make_wxString(), prop);

        case type_animation:
            return new PropertyGrid_Animation(prop->declName().make_wxString(), prop);

        case type_image:
            return new PropertyGrid_Image(prop->declName().make_wxString(), prop);

        case type_float:
            return new wxFloatProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                       prop->as_float());

        default:
            break;
    }

    wxPGProperty* new_pg_property = nullptr;

    switch (type)
    {
        case type_bitlist:
            {
                auto propInfo = prop->getPropDeclaration();

                wxPGChoices bit_flags;
                int index = 0;
                if (prop->get_name() == prop_generate_languages && !wxGetApp().isTestingSwitch())
                {
                    for (auto& iter: propInfo->getOptions())
                    {
                        // If not testing, do not show Code preference options for code we don't
                        // currently generate
                        if (iter.name != "C++" && iter.name != "Perl" && iter.name != "Python" &&
                            iter.name != "Ruby" && iter.name != "XRC")
                            continue;
                        bit_flags.Add(iter.name.make_wxString(), 1 << index++);
                    }
                }
                else
                {
                    for (auto& iter: propInfo->getOptions())
                    {
                        bit_flags.Add(iter.name.make_wxString(), 1 << index++);
                    }
                }

                int val = GetBitlistValue(prop->as_string(), bit_flags);
                new_pg_property = new wxFlagsProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                                      bit_flags, val);

                wxFlagsProperty* flagsProp = dynamic_cast<wxFlagsProperty*>(new_pg_property);
                if (flagsProp)
                {
                    for (size_t i = 0; i < flagsProp->GetItemCount(); i++)
                    {
                        auto id = flagsProp->Item(static_cast<unsigned int>(i));
                        auto& label = id->GetLabel();
                        for (auto& iter: propInfo->getOptions())
                        {
                            if (iter.name == label.ToStdString())
                            {
                                if (iter.help.size())
                                {
                                    wxString description = iter.help;
                                    description.Replace("\\n", "\n", true);
                                    m_prop_grid->SetPropertyHelpString(id, description);
                                }
                                break;
                            }
                        }
                    }
                }
            }
            return new_pg_property;

        case type_option:
        case type_editoption:
            {
                auto propInfo = prop->getPropDeclaration();

                auto value = prop->as_string();
                const tt_string* pHelp = nullptr;

                wxPGChoices constants;
                int i = 0;
                if (prop->get_name() == prop_code_preference && !wxGetApp().isTestingSwitch())
                {
                    for (auto& iter: propInfo->getOptions())
                    {
                        // If not testing, do not show Code preference options for code we don't
                        // currently generate
                        if (iter.name != "C++" && iter.name != "Python" && iter.name != "Ruby" &&
                            iter.name != "XRC")
                            continue;
                        constants.Add(iter.name, i++);
                        if (iter.name == value)
                        {
                            pHelp = &iter.help;
                        }
                    }
                }
                else
                {
                    for (auto& iter: propInfo->getOptions())
                    {
                        constants.Add(iter.name, i++);
                        if (iter.name == value)
                        {
                            pHelp = &iter.help;
                        }
                    }
                }

                if (type == type_editoption)
                {
                    new_pg_property = new wxEditEnumProperty(prop->declName().make_wxString(),
                                                             wxPG_LABEL, constants);
                }
                else
                {
                    new_pg_property =
                        new wxEnumProperty(prop->declName().make_wxString(), wxPG_LABEL, constants);
                }

                new_pg_property->SetValueFromString(value);

                tt_string description = GetPropHelp(prop);
                if (description.empty())
                {
                    description << value;
                }
                else
                {
                    description << "\n\n" << value;
                }
                if (pHelp)
                {
                    if (description.size())
                        description << "\n\n";
                    description << *pHelp;
                }

                new_pg_property->SetHelpString(description.make_wxString());
            }
            return new_pg_property;

        case type_wxColour:
            {
                auto value = prop->as_string();
                return new EditColourProperty(prop->declName().make_wxString(), prop);
            }

        case type_file:
            {
                switch (prop->get_name())
                {
                    case prop_base_file:
                    case prop_cmake_file:
                    case prop_combined_xrc_file:
                    case prop_data_file:
                    case prop_derived_file:
                    case prop_folder_cmake_file:
                    case prop_folder_combined_xrc_file:
                    case prop_fortran_file:
                    case prop_haskell_file:
                    case prop_initial_filename:
                    case prop_lua_file:
                    case prop_output_file:
                    case prop_perl_file:
                    case prop_python_combined_file:
                    case prop_python_file:
                    case prop_ruby_combined_file:
                    case prop_ruby_file:
                    case prop_rust_file:
                    case prop_subclass_header:
                    case prop_xrc_file:
                        return new ttFileProperty(prop);

                    default:
                        break;
                }

                new_pg_property = new wxFileProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                                     prop->as_string());

                switch (prop->get_name())
                {
#if 0
// REVIEW: [Randalphwa - 05-17-2024] Currently prop_header is used for both the header file and any
// preamble. If it does get broken into two properties, then this should be added to ttFileProperty,
// and the case statement moved into the switch statement above.

                    case prop_header:
                        new_pg_property->SetAttribute(wxPG_DIALOG_TITLE, "Custom Control Header");
                        new_pg_property->SetAttribute(wxPG_FILE_WILDCARD, "Header Files|*.h;*.hh;*.hpp;*.hxx");
                        new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, Project.getProjectPath());
                        new_pg_property->SetAttribute(wxPG_FILE_SHOW_RELATIVE_PATH, Project.getProjectPath());
                        return new_pg_property;
#endif

                    case prop_local_pch_file:
                        {
                            new_pg_property->SetAttribute(wxPG_DIALOG_TITLE, "Precompiled header");
                            new_pg_property->SetAttribute(wxPG_FILE_WILDCARD,
                                                          "Header Files|*.h;*.hh;*.hpp;*.hxx");

                            // Often the project file will be kept in a sub-directory, with the
                            // precompiled header file in the parent directory. If we can find a
                            // standard precompiled header filename in the parent directory, then
                            // use that as the starting directory.

                            tt_string pch(Project.getProjectPath());
                            pch.append_filename("../");
                            pch.append_filename("pch.h");
                            if (pch.file_exists())
                            {
                                pch.remove_filename();
                                pch.make_absolute();
                                new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, pch);
                                return new_pg_property;
                            }

                            pch.replace_filename("stdafx.h");  // Older Microsoft standard filename
                            if (pch.file_exists())
                            {
                                pch.remove_filename();
                                pch.make_absolute();
                                new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, pch);
                                return new_pg_property;
                            }

                            pch.replace_filename("precomp.h");  // Less common, but sometimes used
                            if (pch.file_exists())
                            {
                                pch.remove_filename();
                                pch.make_absolute();
                                new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, pch);
                                return new_pg_property;
                            }

                            new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH,
                                                          Project.getProjectPath().make_wxString());
                        }
                        return new_pg_property;

                    default:
                        FAIL_MSG(tt_string("Unsupported file property: ") << prop->declName());
                        return new_pg_property;
                }
            }
            break;

        case type_stringlist:
            {
                new_pg_property = new wxArrayStringProperty(prop->declName().make_wxString(),
                                                            wxPG_LABEL, prop->as_wxArrayString());
                if (prop->value().size() > 0 && prop->value()[0] != '"')
                {
                    wxVariant delimiter(";");
                    new_pg_property->SetAttribute(wxPG_ARRAY_DELIMITER, delimiter);
                }
                else
                {
                    wxVariant delimiter("\"");
                    new_pg_property->SetAttribute(wxPG_ARRAY_DELIMITER, delimiter);
                }
            }
            return new_pg_property;

        case type_stringlist_semi:
            {
                new_pg_property = new wxArrayStringProperty(prop->declName().make_wxString(),
                                                            wxPG_LABEL, prop->as_wxArrayString());
                wxVariant delimiter(";");
                new_pg_property->SetAttribute(wxPG_ARRAY_DELIMITER, delimiter);
            }
            return new_pg_property;

        case type_stringlist_escapes:
            {
                new_pg_property = new wxArrayStringProperty(prop->declName().make_wxString(),
                                                            wxPG_LABEL, prop->as_wxArrayString());
                wxVariant var_quote("\"");
                new_pg_property->SetAttribute(wxPG_ARRAY_DELIMITER, var_quote);
            }
            return new_pg_property;

        case type_uintpairlist:
            return new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                        prop->as_string());

        default:  // Unknown property
            {
                new_pg_property = new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                                       prop->as_string());
                new_pg_property->SetAttribute(wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING,
                                              wxVariant(true, "true"));

                if (wxGetApp().isTestingMenuEnabled())
                {
                    for (auto& iter: umap_PropTypes)
                    {
                        if (iter.second == type)
                        {
                            MSG_ERROR(tt_string("NodeProperty type is unsupported: ")
                                      << iter.first);
                            break;
                        }
                    }
                }
            }
            return new_pg_property;
    }  // end switch (type)
}

void PropGridPanel::CreatePropCategory(tt_string_view name, Node* node,
                                       NodeDeclaration* declaration, PropNameSet& prop_set)
{
    auto& category = declaration->GetCategory();

    if (!category.getCategoryCount() && !category.getPropNameCount())
        return;

    auto generate_languages = Project.getGenerateLanguages();

    // Ignore if the user doesn't want to generate this language
    if (!(static_cast<size_t>(ConvertToGenLang(name)) & generate_languages))
        return;

    auto id =
        m_prop_grid->Append(new wxPropertyCategory(GetCategoryDisplayName(category.GetName())));
    AddProperties(name, node, category, prop_set);

    // Collapse categories that aren't likely to be used with the current object
    if (name.is_sameas("AUI"))
    {
        // TODO: [KeyWorks - 07-25-2020] Need to see if parent is using AUI, and if so, don't
        // collapse this
        m_prop_grid->Collapse(id);
    }
    else if (name.is_sameas("Bitmaps") || name.is_sameas("Command Bitmaps"))
    {
        if (!node->isGen(gen_wxBitmapToggleButton))
        {
            m_prop_grid->Collapse(id);
        }
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#304869"));
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#dce4ef"));
    }
    else if (name.contains("Validator"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#996900"));
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#fff1d2"));

        // It's going to be rare to want a validator for these classes, so collapse the validator
        // for them
        if (node->isGen(gen_wxButton) || node->isGen(gen_wxStaticText))
            m_prop_grid->Collapse(id);
    }
    else if (name.contains("C++"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#000099"));
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#ccccff"));  // Light blue
        if (Project.getCodePreference(node) != GEN_LANG_CPLUSPLUS)
        {
            m_prop_grid->Collapse(id);
        }
    }
    else if (name.contains("wxPerl"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#996900"));
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#ffe7b3"));  // Light yellow
        if (Project.getCodePreference(node) != GEN_LANG_PERL)
        {
            m_prop_grid->Collapse(id);
        }
    }
    else if (name.contains("wxPython"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#009900"));
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#ccffcc"));  // Light green
        if (Project.getCodePreference(node) != GEN_LANG_PYTHON)
        {
            m_prop_grid->Collapse(id);
        }
    }
    else if (name.contains("wxRuby"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#8e0b3d"));
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#f8a9c7"));  // Ruby
        if (Project.getCodePreference(node) != GEN_LANG_RUBY)
        {
            m_prop_grid->Collapse(id);
        }
    }
    else if (name.contains("wxRust"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#b35900"));  // Dark orange
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#ffa64d"));  // Light orange
        if (Project.getCodePreference(node) != GEN_LANG_RUST)
        {
            m_prop_grid->Collapse(id);
        }
    }
    else if (name.contains("XRC"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#00b35c"));  // Gainsboro
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#ccffe6"));  // Mint Cream
        if (Project.getCodePreference(node) != GEN_LANG_XRC)
        {
            m_prop_grid->Collapse(id);
        }
    }

#if GENERATE_NEW_LANG_CODE
    else if (name.contains("wxFortran"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#9900e6"));  // Dark Purple
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#ff99ff"));  // Light Purple
        if (Project.getCodePreference(node) != GEN_LANG_FORTRAN)
        {
            m_prop_grid->Collapse(id);
        }
    }
    else if (name.contains("wxHaskell"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#0000e6"));  // Dark Blue
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#99bbff"));  // Light Blue
        if (Project.getCodePreference(node) != GEN_LANG_HASKELL)
        {
            m_prop_grid->Collapse(id);
        }
    }
    else if (name.contains("wxLua"))
    {
        if (UserPrefs.is_DarkMode())
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#0073e6"));  // Dark Blue
        else
            m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#80bfff"));  // Light Blue
        if (Project.getCodePreference(node) != GEN_LANG_LUA)
        {
            m_prop_grid->Collapse(id);
        }
    }
#endif  // GENERATE_NEW_LANG_CODE

    if (auto it = m_expansion_map.find(GetCategoryDisplayName(category.GetName()).ToStdString());
        it != m_expansion_map.end())
    {
        if (it->second)
        {
            m_prop_grid->Expand(id);
        }
        else
        {
            m_prop_grid->Collapse(id);
        }
    }
}
