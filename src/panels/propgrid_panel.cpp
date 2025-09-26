/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// See propgrid_events.cpp for event handlers for this class
// See progrid_modify.cpp for functions that modify properties in the property grid

#include <wx/arrstr.h>             // wxArrayString class
#include <wx/aui/auibook.h>        // wxaui: wx advanced user interface - notebook
#include <wx/config.h>             // wxConfig base header
#include <wx/dirdlg.h>             // wxDirDialog base class
#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/infobar.h>            // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/advprops.h>  // wxPropertyGrid Advanced Properties (font, colour, etc.)
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid
#include <wx/wupdlock.h>           // wxWindowUpdateLocker prevents window redrawing

#include "propgrid_panel.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "base_panel.h"       // BasePanel -- Code generation panel
#include "category.h"         // NodeCategory class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "cstm_propman.h"     // CustomPropertyManager -- Derived wxPropertyGrid class
#include "font_prop.h"        // FontProperty -- FontProperty class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "node_decl.h"        // NodeDeclaration class
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "paths.h"            // Handles *_directory properties
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "prop_decl.h"        // PropChildDeclaration and PropDeclaration classes
#include "utils.h"            // Utility functions that work with properties

#include "../customprops/evt_string_prop.h"  // EventStringProperty -- dialog for editing event handlers

#include "id_lists.h"  // wxID_ strings

using namespace GenEnum;

constexpr auto PROPERTY_ID = wxID_HIGHEST + 1;
constexpr auto EVENT_ID = PROPERTY_ID + 1;

std::map<GenLang, std::string> s_lang_category_prefix;

PropGridPanel::PropGridPanel(wxWindow* parent, MainFrame* frame) : wxPanel(parent)
{
    for (size_t lang = 1; lang <= GEN_LANG_XRC; lang <<= 1)
    {
        s_lang_category_prefix[static_cast<GenLang>(lang)] =
            GenLangToString(static_cast<GenLang>(lang));
    }

    for (auto& iter: list_wx_ids)
    {
        m_astr_wx_ids.Add(iter);
    }

    m_astr_wx_decorations.Add("__declspec(dllexport)");
    m_astr_wx_decorations.Add("__attribute__((dllexport))");
    m_astr_wx_decorations.Add("[[gnu::dllexport]]");

    m_notebook_parent =
        new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook_parent->SetArtProvider(new wxAuiSimpleTabArt());

    m_prop_grid = new CustomPropertyManager;
    m_prop_grid->Create(m_notebook_parent, PROPERTY_ID, wxDefaultPosition, wxDefaultSize,
                        wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION);

    m_event_grid = new wxPropertyGridManager(
        m_notebook_parent, EVENT_ID, wxDefaultPosition, wxDefaultSize,
        wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION);

    m_notebook_parent->AddPage(m_prop_grid, "Properties", false, wxWithImages::NO_IMAGE);
    m_notebook_parent->AddPage(m_event_grid, "Events", false, wxWithImages::NO_IMAGE);

    RestoreDescBoxHeight();

    auto topSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(m_notebook_parent, wxSizerFlags(1).Expand());
    SetSizer(topSizer);

    Bind(wxEVT_PG_CHANGED, &PropGridPanel::OnPropertyGridChanged, this, PROPERTY_ID);
    Bind(wxEVT_PG_CHANGING, &PropGridPanel::OnPropertyGridChanging, this, PROPERTY_ID);
    Bind(wxEVT_PG_ITEM_COLLAPSED, &PropGridPanel::OnPropertyGridExpand, this, PROPERTY_ID);
    Bind(wxEVT_PG_ITEM_EXPANDED, &PropGridPanel::OnPropertyGridExpand, this, PROPERTY_ID);
    Bind(wxEVT_PG_SELECTED, &PropGridPanel::OnPropertyGridItemSelected, this, PROPERTY_ID);

    Bind(wxEVT_PG_CHANGED, &PropGridPanel::OnEventGridChanged, this, EVENT_ID);
    Bind(wxEVT_PG_ITEM_COLLAPSED, &PropGridPanel::OnEventGridExpand, this, EVENT_ID);
    Bind(wxEVT_PG_ITEM_EXPANDED, &PropGridPanel::OnEventGridExpand, this, EVENT_ID);
    Bind(wxEVT_PG_SELECTED, &PropGridPanel::OnPropertyGridItemSelected, this, EVENT_ID);

    Bind(EVT_NodePropChange, &PropGridPanel::OnNodePropChange, this);

    Bind(EVT_NodeSelected,
         [this](CustomEvent&)
         {
             Create();
         });
    Bind(EVT_ProjectUpdated,
         [this](CustomEvent&)
         {
             Create();
         });
    Bind(EVT_MultiPropChange,
         [this](CustomEvent&)
         {
             Create();
         });

    m_notebook_parent->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED,
                            &PropGridPanel::OnAuiNotebookPageChanged, this);

    frame->AddCustomEventHandler(GetEventHandler());
}

void PropGridPanel::RestoreDescBoxHeight()
{
    auto config = wxConfig::Get();
    config->SetPath(txt_main_window_config);
    auto prop_height = config->ReadLong("prop_height", 100);
    auto event_height = config->ReadLong("event_height", 100);
    config->SetPath("/");

    m_prop_grid->SetDescBoxHeight(prop_height);
    m_event_grid->SetDescBoxHeight(event_height);
}

void PropGridPanel::SaveDescBoxHeight()
{
    auto config = wxConfig::Get();
    config->SetPath(txt_main_window_config);
    config->Write("prop_height", m_prop_grid->GetDescBoxHeight());
    config->Write("event_height", m_event_grid->GetDescBoxHeight());
    config->SetPath("/");
}
int PropGridPanel::GetBitlistValue(const wxString& strVal, wxPGChoices& bit_flags)
{
    wxStringTokenizer strTok(strVal, " |");
    int value = 0;
    while (strTok.HasMoreTokens())
    {
        auto token = strTok.GetNextToken();
        for (unsigned int index = 0; index < bit_flags.GetCount(); ++index)
        {
            if (bit_flags.GetLabel(index) == token)
            {
                value |= bit_flags.GetValue(index);
                break;
            }
        }
    }
    return value;
}

void PropGridPanel::AddProperties(tt_string_view name, Node* node, NodeCategory& category,
                                  PropNameSet& prop_set, bool is_child_cat)
{
    size_t propCount = category.get_PropNameCount();
    for (size_t i = 0; i < propCount; i++)
    {
        auto prop_name = category.get_PropName(i);
        auto prop = node->get_PropPtr(prop_name);

        if (!prop)
            continue;

        if (node->get_NodeDeclaration()->IsPropHidden(prop_name))
            continue;

        if (prop_set.find(prop_name) == prop_set.end())
        {
            if (!IsPropAllowed(node, prop))
                continue;

            auto pg = m_prop_grid->Append(CreatePGProperty(prop));
            auto propType = prop->type();
            if (propType != type_option)
            {
                if (auto gen = node->get_Generator(); gen)
                {
                    if (auto result = gen->GetHint(prop); result)
                    {
                        m_prop_grid->SetPropertyAttribute(pg, wxPG_ATTR_HINT,
                                                          result->make_wxString());
                    }
                }
                m_prop_grid->SetPropertyHelpString(pg, GetPropHelp(prop));

                if (propType == type_id)
                {
                    if (prop->isProp(prop_id))
                    {
                        m_prop_grid->SetPropertyAttribute(pg, wxPG_ATTR_AUTOCOMPLETE,
                                                          m_astr_wx_ids);
                    }
                }
                else if (propType == type_image || propType == type_animation)
                {
                    m_prop_grid->Expand(pg);
                    if (UserPrefs.is_DarkMode())
                        m_prop_grid->SetPropertyBackgroundColour(pg, wxColour("#996900"));
                    else
                        m_prop_grid->SetPropertyBackgroundColour(pg, wxColour("#fff1d2"));

                    // This causes it to display the bitmap in the image/id property
                    pg->RefreshChildren();
                }
                else if (propType == type_string)
                {
                    if (prop->isProp(prop_class_decoration))
                    {
                        m_prop_grid->SetPropertyAttribute(pg, wxPG_ATTR_AUTOCOMPLETE,
                                                          m_astr_wx_decorations);
                    }
                }
            }

            if (name.is_sameas("wxWindow") || name.is_sameas("wxMdiWindow") ||
                category.GetName().Contains("Window Settings"))
            {
                if (UserPrefs.is_DarkMode())
                    m_prop_grid->SetPropertyBackgroundColour(pg, wxColour("#386d2c"));
                else
                    m_prop_grid->SetPropertyBackgroundColour(pg, wxColour("#e7f4e4"));
            }

            // Automatically collapse properties that are rarely used
            if (prop_name == prop_unchecked_bitmap)
                m_prop_grid->Collapse(pg);

            auto prop_name_iter = map_PropNames.find(prop_name);
            if (prop_name_iter != map_PropNames.end())
            {
                if (auto it = m_expansion_map.find(std::string(prop_name_iter->second));
                    it != m_expansion_map.end())
                {
                    if (it->second)
                    {
                        m_prop_grid->Expand(pg);
                    }
                    else
                    {
                        m_prop_grid->Collapse(pg);
                    }
                }
            }

            prop_set.emplace(prop_name);
            m_property_map[pg] = prop;
        }
        else
        {
            MSG_WARNING(tt_string("The property ")
                        << map_PropNames.at(prop_name) << " appears more than once in "
                        << node->get_DeclName());
        }
    }

    for (size_t i = 0; i < propCount; i++)
    {
        ChangeEnableState(node->get_PropPtr(category.get_PropName(i)));
    }

    for (auto& nextCat: category.getCategories())
    {
        if (!nextCat.getCategoryCount() && !nextCat.get_PropNameCount())
        {
            continue;
        }

        wxPGProperty* catId;
        if (is_child_cat)
        {
            catId = m_prop_grid->AppendIn(GetCategoryDisplayName(category.GetName()),
                                          new wxPropertyCategory(nextCat.GetName()));
        }
        else
        {
            catId = m_prop_grid->Append(new wxPropertyCategory(nextCat.GetName()));
        }

        AddProperties(name, node, nextCat, prop_set, true);

        // wxStyledTextCtrl has several categories most of which are rarely used, so it makes sense
        // to collapse them initially.
        if (nextCat.GetName() == "Margin Columns" || nextCat.GetName() == "Selections" ||
            nextCat.GetName() == "Tabs and Indentation" || nextCat.GetName() == "Wrapping")
        {
            m_prop_grid->Collapse(catId);
        }
        else if (auto it = m_expansion_map.find(nextCat.getName()); it != m_expansion_map.end())
        {
            if (it->second)
            {
                m_prop_grid->Expand(catId);
            }
            else
            {
                m_prop_grid->Collapse(catId);
            }
        }
    }
}

// clang-format off
inline constexpr const char* lst_key_events[] = {

    "wxEVT_CHAR",
    "wxEVT_CHAR_HOOK",
    "wxEVT_KEY_DOWN",
    "wxEVT_KEY_UP",

};

inline constexpr const char* lst_mouse_events[] = {

    "wxEVT_ENTER_WINDOW",
    "wxEVT_LEAVE_WINDOW",
    "wxEVT_LEFT_DCLICK",
    "wxEVT_LEFT_DOWN",
    "wxEVT_LEFT_UP",
    "wxEVT_MIDDLE_DCLICK",
    "wxEVT_MIDDLE_DOWN",
    "wxEVT_MIDDLE_UP",
    "wxEVT_RIGHT_DCLICK",
    "wxEVT_RIGHT_DOWN",
    "wxEVT_RIGHT_UP",
    "wxEVT_AUX1_DCLICK",
    "wxEVT_AUX1_DOWN",
    "wxEVT_AUX1_UP",
    "wxEVT_AUX2_DCLICK",
    "wxEVT_AUX2_DOWN",
    "wxEVT_AUX2_UP",
    "wxEVT_MOTION",
    "wxEVT_MOUSEWHEEL",

};
// clang-format on

void PropGridPanel::AddEvents(tt_string_view name, Node* node, NodeCategory& category,
                              EventSet& event_set)
{
    auto& eventList = category.get_Events();
    for (auto& eventName: eventList)
    {
        auto event = node->get_Event(eventName);

        if (!event)
            continue;

        auto eventInfo = event->get_EventInfo();

        ASSERT_MSG(event_set.find(eventName) == event_set.end(),
                   tt_string("Encountered a duplicate event in ") << node->get_DeclName());
        if (event_set.find(eventName) == event_set.end())
        {
            auto grid_property = new EventStringProperty(event->get_name(), event);

            auto id = m_event_grid->Append(grid_property);

            m_event_grid->SetPropertyHelpString(id, wxGetTranslation(eventInfo->get_help()));

            if (name.is_sameas("Window Events") || name.is_sameas("wxTopLevelWindow"))
            {
                if (UserPrefs.is_DarkMode())
                    m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#386d2c"));
                else
                    m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#e7f4e4"));
            }

            if (auto it = m_expansion_map.find(eventName); it != m_expansion_map.end())
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

            event_set.emplace(eventName);
            m_event_map[id] = event;
        }
    }

    size_t catCount = category.getCategoryCount();
    for (size_t i = 0; i < catCount; i++)
    {
        auto& nextCat = category.getCategories()[i];
        if (nextCat.GetName() == "Keyboard Events")
        {
            if (node->get_NodeDeclaration()->GetGeneratorFlags().find("no_key_events") !=
                std::string::npos)
                continue;
        }
        else if (nextCat.GetName() == "Mouse Events")
        {
            if (node->get_NodeDeclaration()->GetGeneratorFlags().find("no_mouse_events") !=
                std::string::npos)
                continue;
        }
        else if (nextCat.GetName() == "Focus Events")
        {
            if (node->get_NodeDeclaration()->GetGeneratorFlags().find("no_focus_events") !=
                std::string::npos)
                continue;
        }

        if (!nextCat.getCategoryCount() && !nextCat.get_EventCount())
        {
            continue;
        }
        wxPGProperty* catId = m_event_grid->AppendIn(GetCategoryDisplayName(category.GetName()),
                                                     new wxPropertyCategory(nextCat.GetName()));

        AddEvents(name, node, nextCat, event_set);

        if (auto it = m_expansion_map.find(nextCat.getName()); it != m_expansion_map.end())
        {
            if (it->second)
            {
                m_event_grid->Expand(catId);
            }
            else
            {
                m_event_grid->Collapse(catId);
            }
        }
        else
        {
            // Keyboard and Mouse events aren't used a lot, but are quite lengthy, so we collapse
            // them by default.

            if (nextCat.getName() == "Keyboard Events")
            {
                bool has_event { false };
                for (auto& iter: lst_key_events)
                {
                    if (auto event = node->get_Event(iter); event && event->get_value().size())
                    {
                        has_event = true;
                        break;
                    }
                }

                if (!has_event)
                    m_event_grid->Collapse(catId);
            }

            else if (nextCat.getName() == "Mouse Events")
            {
                bool has_event { false };
                for (auto& iter: lst_mouse_events)
                {
                    if (auto event = node->get_Event(iter); event && event->get_value().size())
                    {
                        has_event = true;
                        break;
                    }
                }

                if (!has_event)
                    m_event_grid->Collapse(catId);
            }
        }
    }
}

void PropGridPanel::ChangeEnableState(NodeProperty* changed_prop)
{
    if (!changed_prop)
        return;

    // Project properties don't have a generator, so always check if generator exists
    if (auto gen = changed_prop->getNode()->get_Generator(); gen)
    {
        gen->ChangeEnableState(m_prop_grid, changed_prop);
    }
}

void PropGridPanel::ReselectItem()
{
    if (m_pageName == "Properties")
    {
        if (auto property = m_prop_grid->GetPropertyByName(m_selected_prop_name); property)
        {
            m_prop_grid->SelectProperty(property, true);
        }
        else
        {
            m_prop_grid->SetDescription(wxEmptyString, wxEmptyString);
        }
    }
    else if (m_pageName == "Events")
    {
        if (auto property = m_event_grid->GetPropertyByName(m_selected_event_name); property)
        {
            m_event_grid->SelectProperty(property, true);
        }
        else
        {
            m_event_grid->SetDescription(wxEmptyString, wxEmptyString);
        }
    }
}

// Replace internal cateogry names with user-friendly names
wxString PropGridPanel::GetCategoryDisplayName(const wxString& original)
{
    wxString category_name = original;
    if (category_name == "PanelForm")
        category_name = "wxPanel";
    else if (category_name == "MenuBar")
        category_name = "wxMenuBar";
    else if (category_name == "ToolBar")
        category_name = "wxToolBar";

    else if (category_name == "wxWindow" || category_name == "wxMdiWindow")
        category_name = "wxWindow Properties";
    else if (category_name == "Project")
        category_name = "Project Settings";

    return category_name;
}

void PropGridPanel::ReplaceDerivedName(const tt_string& newValue, NodeProperty* propType)
{
    auto drvName = newValue;
    if (drvName.ends_with("Base"))
    {
        drvName.erase(drvName.size() - (sizeof("Base") - 1));
    }
    else
    {
        drvName += "Derived";
    }

    auto grid_property = m_prop_grid->GetPropertyByLabel("derived_class_name");
    grid_property->SetValueFromString(drvName.make_wxString());
    ModifyProperty(propType, drvName);
}

void PropGridPanel::CheckOutputFile(const tt_string& newValue, Node* node)
{
    auto form_node = node->get_Form();

    auto ChangeOutputFile = [&](PropName prop_name)
    {
        if (form_node->HasValue(prop_name))
            return;
        if (auto label = GetPropStringName(prop_name); label)
        {
            auto output_filename = CreateBaseFilename(form_node, newValue);
            auto grid_property = m_prop_grid->GetPropertyByLabel(label->make_wxString());
            grid_property->SetValueFromString(output_filename.make_wxString());
            ModifyProperty(form_node->get_PropPtr(prop_name), output_filename);
        }
    };

    switch (Project.get_CodePreference())
    {
        case GEN_LANG_CPLUSPLUS:
            ChangeOutputFile(prop_base_file);
            break;

        case GEN_LANG_PERL:
            ChangeOutputFile(prop_perl_file);
            break;

        case GEN_LANG_PYTHON:
            ChangeOutputFile(prop_python_file);
            break;

        case GEN_LANG_RUBY:
            ChangeOutputFile(prop_ruby_file);
            break;

        case GEN_LANG_RUST:
            ChangeOutputFile(prop_rust_file);
            break;

        case GEN_LANG_XRC:
            ChangeOutputFile(prop_xrc_file);
            break;

        default:
            break;
    }
}

void PropGridPanel::ReplaceDerivedFile(const tt_string& newValue, NodeProperty* propType)
{
    auto derived_filename = CreateDerivedFilename(propType->getNode()->get_Form(), newValue);
    auto grid_property = m_prop_grid->GetPropertyByLabel("derived_file");
    grid_property->SetValueFromString(derived_filename.make_wxString());
    ModifyProperty(propType, derived_filename);
}

bool PropGridPanel::IsPropAllowed(Node* /* node */, NodeProperty* /* prop */)
{
    // If this function returns false, the property will not be created in the property grid. Note
    // that properties marked as hidden in the XML interface will not be passed to this function.

    return true;
}

bool PropGridPanel::IsEventPageShowing()
{
    if (auto page = m_notebook_parent->GetCurrentPage(); page)
    {
        return (page == m_event_grid);
    }
    return false;
}

tt_string PropGridPanel::GetPropHelp(NodeProperty* prop) const
{
    tt_string description;
    if (auto gen = prop->getNode()->get_Generator(); gen)
    {
        // First let the generator specify the description
        if (auto result = gen->GetPropertyDescription(prop); result)
        {
            description = result.value();
        }
    }
    if (description.empty())
    {
        // If the generator didn't specify a description, then look for a description in the help
        // map
        if (auto map_help = GenEnum::map_PropHelp.find(prop->get_name());
            map_help != GenEnum::map_PropHelp.end())
        {
            description = map_help->second;
        }
        else
        {
            // If we still don't have a description, get whatever was in the XML interface
            description = prop->get_PropDeclaration()->getDescription();
        }
    }
    description.Replace("\\n", "\n", true);
    return description;
}
