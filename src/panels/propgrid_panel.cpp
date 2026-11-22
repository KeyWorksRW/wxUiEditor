/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-21-2026]

// See propgrid_events.cpp for event handlers for this class
// See progrid_modify.cpp for functions that modify properties in the property grid

#include <map>
#include <string_view>
#include <utility>  // std::to_underlying
#include <vector>

#include <wx/arrstr.h>             // wxArrayString class
#include <wx/aui/auibook.h>        // wxaui: wx advanced user interface - notebook
#include <wx/config.h>             // wxConfig base header
#include <wx/dirdlg.h>             // wxDirDialog base class
#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/infobar.h>            // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/advprops.h>  // wxPropertyGrid Advanced Properties (font, colour, etc.)
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid
#include <wx/tokenzr.h>            // wxStringTokenizer class
#include <wx/wupdlock.h>           // wxWindowUpdateLocker prevents window redrawing

#include "propgrid_panel.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "base_panel.h"       // BasePanel -- Code generation panel
#include "category.h"         // NodeCategory class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "font_prop.h"        // FontProperty -- FontProperty class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "node_decl.h"        // NodeDeclaration class
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "prop_decl.h"        // PropChildDeclaration and PropDeclaration classes
#include "utils.h"            // Utility functions that work with properties

#include "../customprops/evt_string_prop.h"  // EventStringProperty -- dialog for editing event handlers

#include "id_lists.h"  // wxID_ strings

using namespace GenEnum;

constexpr wxWindowID PROPERTY_ID = wxID_HIGHEST + 1;
constexpr wxWindowID EVENT_ID = PROPERTY_ID + 1;

// Disables a property and every private child property it created, so a composite value (a
// wxPoint, font, bitmap, and so on) cannot be edited through an expanded child row when the node
// belongs to a read-only linked project.
static void DisableGridProperty(wxPGProperty* property)
{
    if (property == nullptr)
    {
        return;
    }

    property->Enable(false);

    const unsigned int child_count = property->GetChildCount();
    for (unsigned int i = 0; i < child_count; ++i)
    {
        DisableGridProperty(property->Item(i));
    }
}

const std::map<GenLang, std::string>& PropGridPanel::get_LangCategoryPrefixes()
{
    // Built once, on first use. The shift guard keeps the loop terminating even if xrc were the
    // top bit of size_t.
    static const std::map<GenLang, std::string> s_lang_category_prefix = []
    {
        std::map<GenLang, std::string> prefixes;
        for (size_t lang = 1; lang != 0 && lang <= std::to_underlying(GenLang::xrc); lang <<= 1)
        {
            prefixes[static_cast<GenLang>(lang)] = GenLangToString(static_cast<GenLang>(lang));
        }
        return prefixes;
    }();

    return s_lang_category_prefix;
}

PropGridPanel::PropGridPanel(wxWindow* parent, MainFrame* frame) : wxPanel(parent)
{
    for (const auto& iter: list_wx_ids)
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
    m_prop_grid->InitializeHtmlDescription();

    m_event_grid = new CustomPropertyManager;
    m_event_grid->Create(m_notebook_parent, EVENT_ID, wxDefaultPosition, wxDefaultSize,
                         wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION);
    m_event_grid->InitializeHtmlDescription();

    m_notebook_parent->AddPage(m_prop_grid, "Properties", false, wxWithImages::NO_IMAGE);
    m_notebook_parent->AddPage(m_event_grid, "Events", false, wxWithImages::NO_IMAGE);

    RestoreDescBoxHeight();

    auto* topSizer = new wxBoxSizer(wxVERTICAL);
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
    // The description box now renders markdown, which needs more room than the previous default.
    constexpr long DEFAULT_DESC_BOX_HEIGHT = 200;
    // 100 is the previous default, written by older builds when the user never dragged the
    // description-box splitter -- treat it as unset so existing installs get the taller box.
    constexpr long PREVIOUS_DEFAULT_DESC_BOX_HEIGHT = 100;

    wxConfigBase* config = wxConfig::Get();
    if (!config)
    {
        return;
    }
    const wxString saved_path = config->GetPath();
    config->SetPath(txt_main_window_config);
    long prop_height = config->ReadLong("prop_height", DEFAULT_DESC_BOX_HEIGHT);
    long event_height = config->ReadLong("event_height", DEFAULT_DESC_BOX_HEIGHT);
    config->SetPath(saved_path);

    // Migrate only the previous default; arbitrary user-set heights are left untouched.
    if (prop_height == PREVIOUS_DEFAULT_DESC_BOX_HEIGHT)
    {
        prop_height = DEFAULT_DESC_BOX_HEIGHT;
    }
    if (event_height == PREVIOUS_DEFAULT_DESC_BOX_HEIGHT)
    {
        event_height = DEFAULT_DESC_BOX_HEIGHT;
    }

    m_prop_grid->SetDescBoxHeight(prop_height);
    m_event_grid->SetDescBoxHeight(event_height);

    // The height change moves the description box, so snap the HTML description windows to it.
    m_prop_grid->SyncDescriptionWindow();
    m_event_grid->SyncDescriptionWindow();
}

void PropGridPanel::SaveDescBoxHeight()
{
    wxConfigBase* config = wxConfig::Get();
    if (!config)
    {
        return;
    }
    const wxString saved_path = config->GetPath();
    config->SetPath(txt_main_window_config);
    config->Write("prop_height", m_prop_grid->GetDescBoxHeight());
    config->Write("event_height", m_event_grid->GetDescBoxHeight());
    config->SetPath(saved_path);
    config->Flush();
}

// static method
int PropGridPanel::GetBitlistValue(const wxString& strVal, wxPGChoices& bit_flags)
{
    if (strVal.IsEmpty())
    {
        return 0;
    }

    wxStringTokenizer strTok(strVal, " |");
    int value = 0;
    while (strTok.HasMoreTokens())
    {
        const wxString token = strTok.GetNextToken();
        bool is_known_token = false;
        for (unsigned int index = 0; index < bit_flags.GetCount(); ++index)
        {
            if (bit_flags.GetLabel(index) == token)
            {
                value |= bit_flags.GetValue(index);
                is_known_token = true;
                break;
            }
        }
        if (!is_known_token)
        {
            MSG_WARNING((wxString("Unknown bitlist value: ") << token).ToStdString());
        }
    }
    return value;
}

void PropGridPanel::AddProperties(wxue::string_view name, Node* node, NodeCategory& category,
                                  PropNameSet& prop_set, bool is_child_cat)
{
    const size_t propCount = category.get_PropNameCount();
    for (size_t i = 0; i < propCount; ++i)
    {
        PropName prop_name = category.get_PropName(i);
        NodeProperty* prop = node->get_PropPtr(prop_name);

        if (!prop)
        {
            continue;
        }

        if (node->get_NodeDeclaration()->IsPropHidden(prop_name))
        {
            continue;
        }

        if (!prop_set.contains(prop_name))
        {
            if (!IsPropAllowed(node, prop))
            {
                continue;
            }

            wxPGProperty* grid_prop = m_prop_grid->Append(CreatePGProperty(prop));
            if (node->is_ReadOnly())
            {
                DisableGridProperty(grid_prop);
            }
            const PropType propType = prop->type();
            if (propType != type_option)
            {
                if (BaseGenerator* generator = node->get_Generator(); generator)
                {
                    if (auto result = generator->GetHint(prop); result)
                    {
                        m_prop_grid->SetPropertyAttribute(grid_prop, wxPG_ATTR_HINT, result->wx());
                    }
                }
                m_prop_grid->SetPropertyHelpString(grid_prop, GetPropDescription(prop).wx());

                if (propType == type_id)
                {
                    if (prop->isProp(prop_id))
                    {
                        m_prop_grid->SetPropertyAttribute(grid_prop, wxPG_ATTR_AUTOCOMPLETE,
                                                          m_astr_wx_ids);
                    }
                }
                if (propType == type_image || propType == type_animation)
                {
                    m_prop_grid->Expand(grid_prop);
                    if (UserPrefs.is_DarkMode())
                    {
                        m_prop_grid->SetPropertyBackgroundColour(grid_prop, wxColour("#996900"));
                    }
                    else
                    {
                        m_prop_grid->SetPropertyBackgroundColour(grid_prop, wxColour("#fff1d2"));
                    }

                    // This causes it to display the bitmap in the image/id property
                    grid_prop->RefreshChildren();
                }
                if (propType == type_string)
                {
                    if (prop->isProp(prop_class_decoration))
                    {
                        m_prop_grid->SetPropertyAttribute(grid_prop, wxPG_ATTR_AUTOCOMPLETE,
                                                          m_astr_wx_decorations);
                    }
                }
            }

            if (name.is_sameas("wxWindow") || name.is_sameas("wxMdiWindow") ||
                category.GetName().Contains("Window Settings"))
            {
                if (UserPrefs.is_DarkMode())
                {
                    m_prop_grid->SetPropertyBackgroundColour(grid_prop, wxColour("#386d2c"));
                }
                else
                {
                    m_prop_grid->SetPropertyBackgroundColour(grid_prop, wxColour("#e7f4e4"));
                }
            }

            // Automatically collapse properties that are rarely used
            if (prop_name == prop_unchecked_bitmap)
            {
                m_prop_grid->Collapse(grid_prop);
            }

            const std::map<PropName, std::string_view>::const_iterator prop_name_iter =
                map_PropNames.find(prop_name);
            if (prop_name_iter != map_PropNames.end())
            {
                if (auto found_iter = m_expansion_map.find(std::string(prop_name_iter->second));
                    found_iter != m_expansion_map.end())
                {
                    if (found_iter->second)
                    {
                        m_prop_grid->Expand(grid_prop);
                    }
                    else
                    {
                        m_prop_grid->Collapse(grid_prop);
                    }
                }
            }

            prop_set.emplace(prop_name);
            m_property_map[grid_prop] = prop;
        }
        else
        {
            if (const auto prop_it = map_PropNames.find(prop_name); prop_it != map_PropNames.end())
            {
                MSG_WARNING((wxString("The property ")
                             << wxString(prop_it->second.data(), prop_it->second.size())
                             << " appears more than once in "
                             << wxString(node->get_DeclName().data(), node->get_DeclName().size()))
                                .ToStdString());
            }
        }
    }

    for (size_t i = 0; i < propCount; ++i)
    {
        const PropName prop_name = category.get_PropName(i);
        NodeProperty* prop = node->get_PropPtr(prop_name);
        if (prop != nullptr && prop_set.contains(prop_name) &&
            !node->get_NodeDeclaration()->IsPropHidden(prop_name) && IsPropAllowed(node, prop))
        {
            ChangeEnableState(prop);
        }
    }

    // ChangeEnableState can re-enable a row, so re-apply the read-only disable to the rows that
    // are actually in the grid.
    if (node->is_ReadOnly())
    {
        for (const auto& [grid_prop, prop]: m_property_map)
        {
            if (prop != nullptr && prop_set.contains(prop->get_name()))
            {
                DisableGridProperty(grid_prop);
            }
        }
    }

    for (auto& nextCat: category.getCategories())
    {
        if (!nextCat.getCategoryCount() && !nextCat.get_PropNameCount())
        {
            continue;
        }

        const wxPGProperty* catId = nullptr;
        if (is_child_cat)
        {
            // Create the child with the same display name used to look up the parent, otherwise
            // AppendIn fails and catId stays null.
            const wxString child_cat_name = GetCategoryDisplayName(nextCat.GetName());
            catId = m_prop_grid->AppendIn(GetCategoryDisplayName(category.GetName()),
                                          new wxPropertyCategory(child_cat_name));
        }
        else
        {
            catId = m_prop_grid->Append(
                new wxPropertyCategory(GetCategoryDisplayName(nextCat.GetName())));
        }

        AddProperties(name, node, nextCat, prop_set, true);

        // wxStyledTextCtrl has several categories most of which are rarely used, so it makes sense
        // to collapse them initially.
        if (nextCat.GetName() == "Margin Columns" || nextCat.GetName() == "Selections" ||
            nextCat.GetName() == "Tabs and Indentation" || nextCat.GetName() == "Wrapping")
        {
            if (catId != nullptr)
            {
                m_prop_grid->Collapse(catId);
            }
        }
        else if (auto found_iter = m_expansion_map.find(nextCat.getName());
                 found_iter != m_expansion_map.end() && catId != nullptr)
        {
            if (found_iter->second)
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

void PropGridPanel::AddEvents(wxue::string_view name, Node* node, NodeCategory& category,
                              EventSet& event_set)
{
    std::vector<std::string>& eventList = category.get_Events();
    for (auto& eventName: eventList)
    {
        NodeEvent* event = node->get_Event(eventName);

        if (!event)
        {
            continue;
        }

        const NodeEventInfo* eventInfo = event->get_EventInfo();
        if (!eventInfo)
        {
            continue;
        }

        {
            ASSERT_MSG(event_set.find(eventName) == event_set.end(),
                       wxString("Encountered a duplicate event in ")
                           << wxString(node->get_DeclName().data(), node->get_DeclName().size()));
        }
        if (!event_set.contains(eventName))
        {
            auto* grid_property = new EventStringProperty(event->get_name(), event);

            wxPGProperty* prop_id = m_event_grid->Append(grid_property);
            if (node->is_ReadOnly())
            {
                DisableGridProperty(prop_id);
            }

            m_event_grid->SetPropertyHelpString(prop_id, wxGetTranslation(eventInfo->get_help()));

            if (name.is_sameas("Window Events") || name.is_sameas("wxTopLevelWindow"))
            {
                if (UserPrefs.is_DarkMode())
                {
                    m_event_grid->SetPropertyBackgroundColour(prop_id, wxColour("#386d2c"));
                }
                else
                {
                    m_event_grid->SetPropertyBackgroundColour(prop_id, wxColour("#e7f4e4"));
                }
            }

            if (auto found_iter = m_expansion_map.find(eventName);
                found_iter != m_expansion_map.end())
            {
                if (found_iter->second)
                {
                    m_event_grid->Expand(prop_id);
                }
                else
                {
                    m_event_grid->Collapse(prop_id);
                }
            }

            event_set.emplace(eventName);
            m_event_map[prop_id] = event;
        }
    }

    const size_t catCount = category.getCategoryCount();
    for (size_t i = 0; i < catCount; ++i)
    {
        NodeCategory& nextCat = category.getCategories()[i];
        if (nextCat.GetName() == "Keyboard Events")
        {
            if (node->get_NodeDeclaration()->GetGeneratorFlags().find("no_key_events") !=
                std::string::npos)
            {
                continue;
            }
        }
        else if (nextCat.GetName() == "Mouse Events")
        {
            if (node->get_NodeDeclaration()->GetGeneratorFlags().find("no_mouse_events") !=
                std::string::npos)
            {
                continue;
            }
        }
        else if (nextCat.GetName() == "Focus Events")
        {
            if (node->get_NodeDeclaration()->GetGeneratorFlags().find("no_focus_events") !=
                std::string::npos)
            {
                continue;
            }
        }

        if (!nextCat.getCategoryCount() && !nextCat.get_EventCount())
        {
            continue;
        }
        const wxPGProperty* const catId = m_event_grid->AppendIn(
            GetCategoryDisplayName(category.GetName()), new wxPropertyCategory(nextCat.GetName()));

        AddEvents(name, node, nextCat, event_set);

        if (auto found_iter = m_expansion_map.find(nextCat.getName());
            found_iter != m_expansion_map.end())
        {
            if (found_iter->second)
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
                for (const auto& iter: lst_key_events)
                {
                    if (const NodeEvent* event = node->get_Event(iter);
                        event && !event->get_value().empty())
                    {
                        has_event = true;
                        break;
                    }
                }

                if (!has_event)
                {
                    m_event_grid->Collapse(catId);
                }
            }

            else if (nextCat.getName() == "Mouse Events")
            {
                bool has_event { false };
                for (const auto& iter: lst_mouse_events)
                {
                    if (const NodeEvent* event = node->get_Event(iter);
                        event && !event->get_value().empty())
                    {
                        has_event = true;
                        break;
                    }
                }

                if (!has_event)
                {
                    m_event_grid->Collapse(catId);
                }
            }
        }
    }
}

void PropGridPanel::ChangeEnableState(NodeProperty* changed_prop)
{
    if (!changed_prop)
    {
        return;
    }

    // Project properties don't have a generator, so always check if generator exists
    if (BaseGenerator* generator = changed_prop->getNode()->get_Generator(); generator)
    {
        generator->ChangeEnableState(m_prop_grid, changed_prop);
    }
}

void PropGridPanel::ReselectItem()
{
    if (m_pageName == "Properties")
    {
        if (auto* property = m_prop_grid->GetPropertyByName(m_selected_prop_name); property)
        {
            m_prop_grid->SelectProperty(property, true);
        }
        else
        {
            m_prop_grid->ClearDescription();
        }
    }
    else if (m_pageName == "Events")
    {
        if (auto* property = m_event_grid->GetPropertyByName(m_selected_event_name); property)
        {
            m_event_grid->SelectProperty(property, true);
        }
        else
        {
            m_event_grid->ClearDescription();
        }
    }
}

// Replace internal category names with user-friendly names
wxString PropGridPanel::GetCategoryDisplayName(const wxString& original)
{
    wxString category_name = original;
    if (category_name == "PanelForm")
    {
        category_name = "wxPanel";
    }
    else if (category_name == "MenuBar")
    {
        category_name = "wxMenuBar";
    }
    else if (category_name == "ToolBar")
    {
        category_name = "wxToolBar";
    }
    else if (category_name == "wxWindow" || category_name == "wxMdiWindow")
    {
        category_name = "wxWindow Properties";
    }
    else if (category_name == "Project")
    {
        category_name = "Project Settings";
    }

    return category_name;
}

void PropGridPanel::ReplaceDerivedName(const wxue::string& formName, NodeProperty* propType)
{
    wxue::string drvName = formName;
    if (drvName.ends_with("Base"))
    {
        drvName = StripBaseSuffix(drvName);
    }
    else
    {
        drvName += "Derived";
    }

    wxPGProperty* grid_property = m_prop_grid->GetPropertyByLabel("derived_class_name");
    ASSERT(grid_property);
    if (!grid_property)
    {
        return;
    }
    grid_property->SetValueFromString(drvName.wx());
    ModifyProperty(propType, drvName);
}

void PropGridPanel::CheckOutputFile(const wxue::string& formName, Node* node)
{
    Node* form_node = node->get_Form();
    if (!form_node)
    {
        return;
    }

    auto ChangeOutputFile = [&](PropName prop_name)
    {
        if (form_node->HasValue(prop_name))
        {
            return;
        }
        if (auto label = GetPropStringName(prop_name); label)
        {
            const wxue::string output_filename = CreateBaseFilename(form_node, formName);
            wxPGProperty* grid_property = m_prop_grid->GetPropertyByLabel(label->wx());
            ASSERT(grid_property);
            if (!grid_property)
            {
                return;
            }
            NodeProperty* output_prop = form_node->get_PropPtr(prop_name);
            if (output_prop == nullptr)
            {
                return;
            }
            grid_property->SetValueFromString(output_filename.wx());
            ModifyProperty(output_prop, output_filename);
        }
    };

    switch (Project.get_CodePreference(node))
    {
        case GenLang::cplusplus:
            ChangeOutputFile(prop_base_file);
            break;

        case GenLang::python:
            ChangeOutputFile(prop_python_file);
            break;

        case GenLang::ruby:
            ChangeOutputFile(prop_ruby_file);
            break;

        case GenLang::fortran:
            ChangeOutputFile(prop_fortran_file);
            break;

        case GenLang::go:
            ChangeOutputFile(prop_go_file);
            break;

        case GenLang::julia:
            ChangeOutputFile(prop_julia_file);
            break;

        case GenLang::luajit:
            ChangeOutputFile(prop_lua_file);
            break;

        case GenLang::rust:
            ChangeOutputFile(prop_rust_file);
            break;

        case GenLang::typescript:
            ChangeOutputFile(prop_typescript_file);
            break;

        case GenLang::xrc:
            ChangeOutputFile(prop_xrc_file);
            break;

        default:
            break;
    }
}

void PropGridPanel::ReplaceDerivedFile(const wxue::string& formName, NodeProperty* propType)
{
    const wxue::string derived_filename =
        CreateDerivedFilename(propType->getNode()->get_Form(), formName);
    wxPGProperty* grid_property = m_prop_grid->GetPropertyByLabel("derived_file");
    ASSERT(grid_property);
    if (!grid_property)
    {
        return;
    }
    grid_property->SetValueFromString(derived_filename.wx());
    ModifyProperty(propType, derived_filename);
    Project.InvalidateDerivedFileCache();
}

bool PropGridPanel::IsPropAllowed(Node* node, NodeProperty* prop)
{
    // If this function returns false, the property will not be created in the property grid. Note
    // that properties marked as hidden in the XML interface will not be passed to this function.

    if (prop->isProp(prop_field_index))
    {
        // field_index only applies to a control that is a direct child of a wxStatusBar, and the
        // generated code (wxStatusBar::AddFieldControl()) is only emitted for C++.
        const Node* parent = node->get_Parent();
        return parent && parent->is_Gen(gen_wxStatusBar) &&
               Project.get_CodePreference(node) == GenLang::cplusplus;
    }

    // The folder art_directory override and link_generate_code only make sense on a folder that
    // links another project. The project's own art_directory must always stay editable.
    if ((prop->isProp(prop_link_generate_code) || prop->isProp(prop_art_directory)) &&
        node->is_Folder() && !node->is_LinkedProject())
    {
        return false;
    }

    return true;
}

bool PropGridPanel::IsEventPageShowing()
{
    if (auto* page = m_notebook_parent->GetCurrentPage(); page)
    {
        return (page == m_event_grid);
    }
    return false;
}

wxue::string PropGridPanel::GetPropHelp(NodeProperty* prop)
{
    wxue::string description;
    if (BaseGenerator* generator = prop->getNode()->get_Generator(); generator)
    {
        // First let the generator specify the description
        if (auto result = generator->GetPropertyDescription(prop); result)
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
    std::ignore = description.Replace("\\n", "\n", true);
    return description;
}

wxue::string PropGridPanel::GetPropDescription(NodeProperty* prop)
{
    wxue::string description = GetPropHelp(prop);

    const PropType prop_type = prop->type();
    if (prop_type == type_option || prop_type == type_editoption)
    {
        const std::string& value = prop->as_string();
        for (const auto& option: prop->get_PropDeclaration()->getOptions())
        {
            if (option.name != value || option.help.empty())
            {
                continue;
            }

            if (!description.empty())
            {
                description << "\n\n";
            }
            description << option.help;
            break;
        }
    }

    // Option help can also contain escaped newlines from the XML declaration files.
    std::ignore = description.Replace("\\n", "\n", true);
    return description;
}
