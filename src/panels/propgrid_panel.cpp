/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <unordered_set>

#include <wx/arrstr.h>             // wxArrayString class
#include <wx/aui/auibook.h>        // wxaui: wx advanced user interface - notebook
#include <wx/config.h>             // wxConfig base header
#include <wx/dirdlg.h>             // wxDirDialog base class
#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/infobar.h>            // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/advprops.h>  // wxPropertyGrid Advanced Properties (font, colour, etc.)
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "propgrid_panel.h"

#include "appoptions.h"      // AppOptions -- Application-wide options
#include "auto_freeze.h"     // AutoFreeze -- Automatically Freeze/Thaw a window
#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "bitmaps.h"         // Map of bitmaps accessed by name
#include "category.h"        // NodeCategory class
#include "cstm_event.h"      // CustomEvent -- Custom Event class
#include "font_prop.h"       // FontProperty -- FontProperty class
#include "mainframe.h"       // MainFrame -- Main window frame
#include "node.h"            // Node class
#include "node_decl.h"       // NodeDeclaration class
#include "node_prop.h"       // NodeProperty -- NodeProperty class
#include "prop_decl.h"       // PropChildDeclaration and PropDeclaration classes
#include "utils.h"           // Utility functions that work with properties

// Various customized wxPGProperty classes

#include "../customprops/code_string_prop.h"    // EditCodeDialogAdapter -- Derived wxStringProperty class for code
#include "../customprops/custom_colour_prop.h"  // EditColourDialogAdapter -- Property editor for colour
#include "../customprops/custom_param_prop.h"   // EditParamProperty -- dialog for editing CustomControl parameter
#include "../customprops/directory_prop.h"      // DirectoryDialogAdapter
#include "../customprops/evt_string_prop.h"     // EventStringProperty -- dialog for editing event handlers
#include "../customprops/pg_animation.h"        // PropertyGrid_Animation -- Custom property grid class for animations
#include "../customprops/pg_image.h"            // PropertyGrid_Image -- Custom property grid class for images
#include "../customprops/pg_point.h"            // CustomPointProperty -- custom wxPGProperty for handling wxPoint
#include "../customprops/txt_string_prop.h"     // EditStringProperty -- dialog for editing single-line strings

#include "wx_id_list.cpp"  // wxID_ strings

using namespace GenEnum;

constexpr auto PROPERTY_ID = wxID_HIGHEST + 1;
constexpr auto EVENT_ID = PROPERTY_ID + 1;

PropGridPanel::PropGridPanel(wxWindow* parent, MainFrame* frame) : wxPanel(parent)
{
    for (auto& iter: list_wx_ids)
    {
        m_astr_wx_ids.Add(iter);
    }

    m_astr_wx_decorations.Add("__declspec(dllexport)");

    m_notebook_parent = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook_parent->SetArtProvider(new wxAuiSimpleTabArt());

    m_prop_grid = new CustomPropertyManager;
    m_prop_grid->Create(m_notebook_parent, PROPERTY_ID, wxDefaultPosition, wxDefaultSize,
                        wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION);

    m_event_grid = new wxPropertyGridManager(m_notebook_parent, EVENT_ID, wxDefaultPosition, wxDefaultSize,
                                             wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION);

    m_notebook_parent->AddPage(m_prop_grid, "Properties", false, 0);
    m_notebook_parent->AddPage(m_event_grid, "Events", false, 1);

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

void PropGridPanel::Create()
{
    if (m_locked)
        return;

    if (auto node = wxGetFrame().GetSelectedNode(); node)
    {
        AutoFreeze freeze(this);

#if defined(_DEBUG)
        if (wxGetApp().isFireCreationMsgs())
        {
            MSG_INFO("Property window recreated.");
        }
#endif  // _DEBUG

        wxGetApp().GetMainFrame()->SetStatusText(wxEmptyString, 2);

        m_currentSel = node;

        wxString pageName;
        if (int pageNumber = m_prop_grid->GetSelectedPage(); pageNumber != wxNOT_FOUND)
        {
            pageName = m_prop_grid->GetPageName(pageNumber);
        }

        m_prop_grid->Clear();
        m_event_grid->Clear();

        m_property_map.clear();
        m_event_map.clear();

        if (auto declaration = node->GetNodeDeclaration(); declaration)
        {
            // These sets are used to prevent trying to add a duplicate property or event to the property grid. In Debug
            // builds, attempting to do so will generate an assert message telling you the name of the duplicate and the node
            // declaration it occurs in. In release builds, only the first instance will be displayed.

            PropNameSet prop_set;
            EventSet event_set;

            CreatePropCategory(declaration->DeclName(), node, declaration, prop_set);
            CreateEventCategory(declaration->DeclName(), node, declaration, event_set);

            // Calling GetBaseClassCount() is exepensive, so do it once and store the result
            auto num_base_classes = declaration->GetBaseClassCount();
            for (size_t i = 0; i < num_base_classes; i++)
            {
                auto info_base = declaration->GetBaseClass(i);
                if (info_base->isGen(gen_sizer_child))
                    continue;
                if (!info_base->DeclName().is_sameas("Window Events"))
                    CreatePropCategory(info_base->DeclName(), node, info_base, prop_set);
                CreateEventCategory(info_base->DeclName(), node, info_base, event_set);
            }

            if (node->IsSpacer())
            {
                if (node->isParent(gen_wxGridBagSizer))
                    CreateLayoutCategory(node);
            }
            else if (node->GetParent() && node->GetParent()->IsSizer())
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

wxPGProperty* PropGridPanel::GetProperty(NodeProperty* prop)
{
    auto type = prop->type();

    // Note that prop->as_string() does NOT do a UTF16 conversion on Windows unless you call wx_str().
    // prop->as_wxString() automatically calls wx_str().

    switch (type)
    {
        case type_id:
            return new wxStringProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_wxString());

        case type_int:
            return new wxIntProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_int());

        case type_uint:
            return new wxUIntProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_int());

        case type_string_code_single:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditParamProperty(prop->DeclName().wx_str(), prop);

        case type_string_escapes:
            // This first doubles the backslash in escaped characters: \n, \t, \r, and \.
            return new wxStringProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_escape_text().wx_str());

        case type_string:
            return new wxStringProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_wxString());

        case type_string_edit_escapes:
            // This includes a button that triggers a small text editor dialog
            // This doubles the backslash in escaped characters: \n, \t, \r, and \.
            return new wxLongStringProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_escape_text().wx_str());

        case type_string_edit:
            // This includes a button that triggers a small text editor dialog
            return new wxLongStringProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_wxString());

        case type_string_edit_single:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditStringProperty(prop->DeclName().wx_str(), prop);

        case type_code_edit:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditCodeProperty(prop->DeclName().wx_str(), prop);

        case type_bool:
            return new wxBoolProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_string() == "1");

        case type_wxPoint:
            return new CustomPointProperty(prop->DeclName().wx_str(), prop, CustomPointProperty::type_point);

        case type_wxSize:
            return new CustomPointProperty(prop->DeclName().wx_str(), prop, CustomPointProperty::type_size);

        case type_wxFont:
            if (prop->as_string().empty())
            {
                return new wxFontProperty(prop->DeclName().wx_str(), wxPG_LABEL);
            }
            else
            {
                return new wxFontProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_font());
            }

        case type_path:
            return new DirectoryProperty(prop->DeclName().wx_str(), prop);

        case type_animation:
            return new PropertyGrid_Animation(prop->DeclName().wx_str(), prop);

        case type_image:
            return new PropertyGrid_Image(prop->DeclName().wx_str(), prop);

        case type_float:
            return new wxFloatProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_float());

        default:
            break;
    }

    wxPGProperty* new_pg_property = nullptr;

    if (type == type_bitlist)
    {
        auto propInfo = prop->GetPropDeclaration();

        wxPGChoices bit_flags;
        int index = 0;
        for (auto& iter: propInfo->GetOptions())
        {
            bit_flags.Add(iter.name, 1 << index++);
        }

        int val = GetBitlistValue(prop->as_string(), bit_flags);
        new_pg_property = new wxFlagsProperty(prop->DeclName().wx_str(), wxPG_LABEL, bit_flags, val);

        wxFlagsProperty* flagsProp = dynamic_cast<wxFlagsProperty*>(new_pg_property);
        if (flagsProp)
        {
            for (size_t i = 0; i < flagsProp->GetItemCount(); i++)
            {
                auto id = flagsProp->Item(static_cast<unsigned int>(i));
                auto& label = id->GetLabel();
                for (auto& iter: propInfo->GetOptions())
                {
                    if (iter.name == label)
                    {
                        m_prop_grid->SetPropertyHelpString(id, iter.help);
                        break;
                    }
                }
            }
        }
    }
    else if (type == type_option || type == type_editoption)
    {
        auto propInfo = prop->GetPropDeclaration();

        auto value = prop->as_wxString();
        const wxString* pHelp = nullptr;

        wxPGChoices constants;
        int i = 0;
        for (auto& iter: propInfo->GetOptions())
        {
            constants.Add(iter.name, i++);
            if (iter.name == value)
            {
                pHelp = &iter.help;
            }
        }

        if (type == type_editoption)
        {
            new_pg_property = new wxEditEnumProperty(prop->DeclName().wx_str(), wxPG_LABEL, constants);
        }
        else
        {
            new_pg_property = new wxEnumProperty(prop->DeclName().wx_str(), wxPG_LABEL, constants);
        }

        new_pg_property->SetValueFromString(value, 0);

        wxString desc = propInfo->GetDescription();
        if (desc.empty())
        {
            desc << value << ":\n";
        }
        else
        {
            desc << "\n\n" << value << ":\n";
        }

        if (pHelp)
            desc << *pHelp;

        new_pg_property->SetHelpString(desc);
    }
    else if (type == type_wxColour)
    {
        auto value = prop->as_string();
        new_pg_property = new EditColourProperty(prop->DeclName().wx_str(), prop);
    }
    else if (type == type_file)
    {
        new_pg_property = new wxFileProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_string());
        if (prop->isProp(prop_base_file))
        {
            new_pg_property->SetAttribute(wxPG_DIALOG_TITLE, "Base class filename");
            new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, wxGetApp().GetBaseDirectory());
            new_pg_property->SetAttribute(wxPG_FILE_SHOW_RELATIVE_PATH, wxGetApp().GetProjectPath());
            new_pg_property->SetAttribute(wxPG_FILE_DIALOG_STYLE, wxFD_SAVE);
            new_pg_property->SetAttribute(wxPG_FILE_WILDCARD, "C++ Files|*.cpp;*.cc;*.cxx");
        }
        else if (prop->isProp(prop_derived_file))
        {
            new_pg_property->SetAttribute(wxPG_DIALOG_TITLE, "Derived class filename");
            new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, wxGetApp().GetDerivedDirectory());
            new_pg_property->SetAttribute(wxPG_FILE_SHOW_RELATIVE_PATH, wxGetApp().GetProjectPath());
            new_pg_property->SetAttribute(wxPG_FILE_DIALOG_STYLE, wxFD_SAVE);
            new_pg_property->SetAttribute(wxPG_FILE_WILDCARD, "C++ Files|*.cpp;*.cc;*.cxx");
        }
        else if (prop->isProp(prop_header))
        {
            new_pg_property->SetAttribute(wxPG_DIALOG_TITLE, "Custom Control Header");
            new_pg_property->SetAttribute(wxPG_FILE_WILDCARD, "Header Files|*.h;*.hh;*.hpp;*.hxx");
            new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, wxGetApp().GetProjectPath());
            new_pg_property->SetAttribute(wxPG_FILE_SHOW_RELATIVE_PATH, wxGetApp().GetProjectPath());
        }
        else if (prop->isProp(prop_local_pch_file))
        {
            new_pg_property->SetAttribute(wxPG_DIALOG_TITLE, "Precompiled header");
            new_pg_property->SetAttribute(wxPG_FILE_WILDCARD, "Header Files|*.h;*.hh;*.hpp;*.hxx");

            // Often the project file will be kept in a sub-directory, with the precompiled header file in the parent
            // directory. If we can find a standard precompiled header filename in the parent directory, then use that
            // as the starting directory.

            ttString pch(wxGetApp().GetProjectPath());
            pch.append_filename("../");
            pch.append_filename("pch.h");
            if (pch.file_exists())
            {
                pch.remove_filename();
                new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, pch);
                return new_pg_property;
            }

            pch.replace_filename("stdafx.h");  // Older Microsoft standard filename
            if (pch.file_exists())
            {
                pch.remove_filename();
                new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, pch);
                return new_pg_property;
            }

            pch.replace_filename("precomp.h");  // Less common, but sometimes used
            if (pch.file_exists())
            {
                pch.remove_filename();
                new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, pch);
                return new_pg_property;
            }

            new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, wxGetApp().GetProjectPath());
        }
    }
    else if (type == type_stringlist)
    {
        new_pg_property = new wxArrayStringProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_wxArrayString());
        wxVariant var_quote("\"");
        new_pg_property->DoSetAttribute(wxPG_ARRAY_DELIMITER, var_quote);
    }
    else if (type == type_uintpairlist)
    {
        new_pg_property = new wxStringProperty(prop->DeclName().wx_str(), wxPG_LABEL);
    }
    else  // Unknown property
    {
        new_pg_property = new wxStringProperty(prop->DeclName().wx_str(), wxPG_LABEL, prop->as_string());
        new_pg_property->SetAttribute(wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, wxVariant(true, "true"));
        MSG_ERROR(ttlib::cstr("NodeProperty type is unsupported: ") << map_PropTypes[type]);
    }

    return new_pg_property;
}

void PropGridPanel::AddProperties(ttlib::cview name, Node* node, NodeCategory& category, PropNameSet& prop_set,
                                  bool is_child_cat)
{
    size_t propCount = category.GetPropNameCount();
    for (size_t i = 0; i < propCount; i++)
    {
        auto prop_name = category.GetPropName(i);
        auto prop = node->get_prop_ptr(prop_name);

        if (!prop)
            continue;

        if (node->GetNodeDeclaration()->IsPropHidden(prop_name))
            continue;

        if (prop_set.find(prop_name) == prop_set.end())
        {
            if (!IsPropAllowed(node, prop))
                continue;

            auto propInfo = prop->GetPropDeclaration();
            auto pg = m_prop_grid->Append(GetProperty(prop));
            auto propType = prop->type();
            if (propType != type_option)
            {
                m_prop_grid->SetPropertyHelpString(pg, propInfo->GetDescription());
                if (propType == type_id)
                {
                    if (prop->isProp(prop_id))
                    {
                        m_prop_grid->SetPropertyAttribute(pg, wxPG_ATTR_AUTOCOMPLETE, m_astr_wx_ids);
                    }
                }
                else if (propType == type_image || propType == type_animation)
                {
                    m_prop_grid->Expand(pg);
                    m_prop_grid->SetPropertyBackgroundColour(pg, wxColour("#fff1d2"));

                    // This causes it to display the bitmap in the image/id property
                    pg->RefreshChildren();
                }
                else if (propType == type_string)
                {
                    if (prop->isProp(prop_class_decoration))
                    {
                        m_prop_grid->SetPropertyAttribute(pg, wxPG_ATTR_AUTOCOMPLETE, m_astr_wx_decorations);
                    }
                }
            }

            auto& customEditor = propInfo->GetCustomEditor();
            if (!customEditor.empty())
            {
                wxPGEditor* editor = m_prop_grid->GetEditorByName(customEditor);
                if (editor)
                {
                    m_prop_grid->SetPropertyEditor(pg, editor);
                }
            }

            if (name.is_sameas("wxWindow") || category.GetName() == "Window Settings")
                m_prop_grid->SetPropertyBackgroundColour(pg, wxColour("#e7f4e4"));

            // Automatically collapse properties that are rarely used
            if (prop_name == prop_unchecked_bitmap)
                m_prop_grid->Collapse(pg);

            if (auto it = m_expansion_map.find(map_PropNames[prop_name]); it != m_expansion_map.end())
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

            prop_set.emplace(prop_name);
            m_property_map[pg] = prop;
        }
        else
        {
            MSG_WARNING(ttlib::cstr("The property ")
                        << map_PropNames[prop_name] << " appears more than once in " << node->DeclName());
        }
    }

    for (auto& nextCat: category.GetCategories())
    {
        if (!nextCat.GetCategoryCount() && !nextCat.GetPropNameCount())
        {
            continue;
        }

        wxPGProperty* catId;
        if (is_child_cat)
        {
            catId =
                m_prop_grid->AppendIn(GetCategoryDisplayName(category.GetName()), new wxPropertyCategory(nextCat.GetName()));
        }
        else
        {
            catId = m_prop_grid->Append(new wxPropertyCategory(nextCat.GetName()));
        }

        AddProperties(name, node, nextCat, prop_set, true);

        if (auto it = m_expansion_map.find(nextCat.getName()); it != m_expansion_map.end())
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

void PropGridPanel::AddEvents(ttlib::cview name, Node* node, NodeCategory& category, EventSet& event_set)
{
    auto& eventList = category.GetEvents();
    for (auto& eventName: eventList)
    {
        auto event = node->GetEvent(eventName);

        if (!event)
            continue;

        auto eventInfo = event->GetEventInfo();

        ASSERT_MSG(event_set.find(eventName) == event_set.end(), ttlib::cstr("Encountered a duplicate event in ")
                                                                     << node->DeclName());
        if (event_set.find(eventName) == event_set.end())
        {
            auto grid_property = new EventStringProperty(event->get_name(), event);

            auto id = m_event_grid->Append(grid_property);

            m_event_grid->SetPropertyHelpString(id, wxGetTranslation(eventInfo->get_help()));

            if (name.is_sameas("Window Events") || name.is_sameas("wxTopLevelWindow"))
                m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#e7f4e4"));

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

    size_t catCount = category.GetCategoryCount();
    for (size_t i = 0; i < catCount; i++)
    {
        auto& nextCat = category.GetCategories()[i];
        if (nextCat.GetName() == "Keyboard Events")
        {
            if (node->GetNodeDeclaration()->GetGeneratorFlags().contains("no_key_events"))
                continue;
        }
        else if (nextCat.GetName() == "Mouse Events")
        {
            if (node->GetNodeDeclaration()->GetGeneratorFlags().contains("no_mouse_events"))
                continue;
        }
        else if (nextCat.GetName() == "Focus Events")
        {
            if (node->GetNodeDeclaration()->GetGeneratorFlags().contains("no_focus_events"))
                continue;
        }

        if (!nextCat.GetCategoryCount() && !nextCat.GetEventCount())
        {
            continue;
        }
        wxPGProperty* catId =
            m_event_grid->AppendIn(GetCategoryDisplayName(category.GetName()), new wxPropertyCategory(nextCat.GetName()));

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
            // Keyboard and Mouse events aren't used a lot, but are quite lengthy, so we collapse them by default.

            if (nextCat.getName() == "Keyboard Events")
            {
                bool has_event { false };
                for (auto& iter: lst_key_events)
                {
                    if (auto event = node->GetEvent(iter); event && event->get_value().size())
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
                    if (auto event = node->GetEvent(iter); event && event->get_value().size())
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

// Only process property changes that we may need to cancel here.
void PropGridPanel::OnPropertyGridChanging(wxPropertyGridEvent& event)
{
    m_failure_handled = false;

    auto property = event.GetProperty();

    auto it = m_property_map.find(property);
    if (it == m_property_map.end())
    {
        property = property->GetParent();
        it = m_property_map.find(property);
    }

    if (it == m_property_map.end())
        return;

    auto prop = it->second;
    auto node = prop->GetNode();
    auto generator = node->GetGenerator();
    if (generator)
    {
        if (!generator->AllowPropertyChange(&event, prop, node))
        {
            m_failure_handled = true;
            return;
        }
    }

    switch (prop->type())
    {
        case type_file:
            VerifyChangeFile(event, prop, node);
            break;

        default:
            break;
    }
}

void PropGridPanel::OnPropertyGridChanged(wxPropertyGridEvent& event)
{
    auto property = event.GetProperty();

    auto it = m_property_map.find(property);
    if (it == m_property_map.end())
    {
        property = property->GetParent();
        it = m_property_map.find(property);
    }

    if (it == m_property_map.end())
        return;

    auto prop = it->second;
    auto node = prop->GetNode();

    switch (prop->type())
    {
        case type_float:
            {
                double val = m_prop_grid->GetPropertyValueAsDouble(property);

                modifyProperty(prop, DoubleToStr(val));
                break;
            }

        case type_code_edit:
        case type_string_edit:
        case type_id:
        case type_int:
        case type_uint:
            {
                ModifyProperty(prop, m_prop_grid->GetPropertyValueAsString(property));
                break;
            }

        case type_option:
        case type_editoption:
            {
                wxString value = m_prop_grid->GetPropertyValueAsString(property);
                ModifyProperty(prop, value);

                // Update displayed description for the new selection
                auto propInfo = prop->GetPropDeclaration();

                wxString helpString = wxString::FromUTF8Unchecked(propInfo->GetDescription());

                for (auto& iter: propInfo->GetOptions())
                {
                    if (iter.name == value)
                    {
                        if (iter.help.empty())
                            helpString = value + ":\n";
                        else
                            helpString += "\n\n" + value + ":\n" + iter.help;

                        break;
                    }
                }

                wxString localized = wxGetTranslation(helpString);
                m_prop_grid->SetPropertyHelpString(property, localized);
                m_prop_grid->SetDescription(property->GetLabel(), localized);

                if (auto selected_node = wxGetFrame().GetSelectedNode(); selected_node)
                {
                    if (prop->isProp(prop_validator_data_type) && selected_node->isGen(gen_wxTextCtrl))
                    {
                        // You can only use a wxTextValidator if the validator data type is wxString. If it's not a string,
                        // the program will compile just fine, but the data member will not be read or written to. To prevent
                        // that, we switch the validator type to match the data type. The downside is that this is two
                        // actions, and so it takes two calls to Undo to get back to where we were.

                        if (value == "wxString")
                        {
                            auto propType = selected_node->get_prop_ptr(prop_validator_type);
                            if (propType->as_string() != "wxTextValidator")
                            {
                                auto grid_property = m_prop_grid->GetPropertyByLabel("validator_type");
                                grid_property->SetValueFromString("wxTextValidator", 0);
                                modifyProperty(propType, "wxTextValidator");
                            }
                        }
                        else
                        {
                            auto propType = selected_node->get_prop_ptr(prop_validator_type);
                            if (propType->as_string() == "wxTextValidator")
                            {
                                auto grid_property = m_prop_grid->GetPropertyByLabel("validator_type");
                                grid_property->SetValueFromString("wxGenericValidator", 0);
                                modifyProperty(propType, "wxGenericValidator");
                            }
                        }
                    }
                    else if (prop->isProp(prop_class_access) && wxGetApp().IsPjtMemberPrefix())
                    {
                        // TODO: [KeyWorks - 08-23-2020] This needs to be a preference

                        // If access is changed to local and the name starts with "m_", then the "m_" will be stripped off.
                        // Conversely, if the name is changed from local to a class member, a "m_" is added as a prefix (if
                        // it doesn't already have one).
                        ttlib::cstr name = node->prop_as_string(prop_var_name);
                        if (value == "none" && name.is_sameprefix("m_"))
                        {
                            name.erase(0, 2);
                            auto final_name = node->GetUniqueName(name);
                            if (final_name.size())
                                name = final_name;
                            auto propChange = selected_node->get_prop_ptr(prop_var_name);
                            auto grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                            grid_property->SetValueFromString(name, 0);
                            modifyProperty(propChange, name);
                        }
                        else if (value != "none" && !name.is_sameprefix("m_"))
                        {
                            name.insert(0, "m_");
                            auto final_name = node->GetUniqueName(name);
                            if (final_name.size())
                                name = final_name;
                            auto propChange = selected_node->get_prop_ptr(prop_var_name);
                            auto grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                            grid_property->SetValueFromString(name, 0);
                            modifyProperty(propChange, name);
                        }
                    }
                }
            }
            break;

        case type_string_escapes:
        case type_string_edit_escapes:
            {
                auto value = ConvertEscapeSlashes(ttlib::cstr() << m_prop_grid->GetPropertyValueAsString(property).wx_str());
                modifyProperty(prop, value);
            }
            break;

        case type_bool:
            {
                if (!m_prop_grid->GetPropertyValueAsBool(property))
                {
                    if (node->isGen(gen_wxStdDialogButtonSizer))
                    {
                        auto def_prop = node->get_prop_ptr(prop_default_button);
                        if (def_prop->as_string() == prop->DeclName())
                        {
                            m_prop_grid->SetPropertyValue("default_button", "none");
                            modifyProperty(def_prop, "none");
                        }
                    }
                }
                modifyProperty(prop, m_prop_grid->GetPropertyValueAsBool(property) ? "1" : "0");
            }
            break;

        case type_bitlist:
            {
                ttString value = m_prop_grid->GetPropertyValueAsString(property);
                value.Replace(" ", "");
                value.Replace(",", "|");
                if (prop->isProp(prop_style))
                {
                    // Don't allow the user to combine incompatible styles
                    if (value.contains("wxFLP_OPEN") && value.contains("wxFLP_SAVE"))
                    {
                        auto style_prop = node->get_prop_ptr(prop_style);
                        auto& old_value = style_prop->as_string();
                        if (old_value.contains("wxFLP_OPEN"))
                        {
                            value.Replace("wxFLP_OPEN", "");
                            value.Replace("wxFLP_FILE_MUST_EXIST", "");
                            value.Replace("||", "|", true);  // Fix all cases of a doubled pipe

                            // Change the format to what the property grid wants
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

                            // Change the format to what the property grid wants
                            value.Replace("|", ",");
                            m_prop_grid->SetPropertyValue("style", value);

                            // Now put it back into the format we use internally
                            value.Replace(",", "|");
                        }
                    }
                }

                ModifyProperty(prop, value);
            }
            break;

        case type_wxPoint:
        case type_wxSize:
            {
                auto value = event.GetPropertyValue().GetString();
                modifyProperty(prop, value.utf8_string());
            }
            break;

        case type_wxFont:
            {
                wxFont font;
                font << event.GetPropertyValue();
                if (font.IsOk())
                {
                    FontProperty font_prop(font);
                    modifyProperty(prop, font_prop.as_string());
                }
            }
            break;

        case type_animation:
        case type_image:
            {
                ttlib::cstr value;
                // Do NOT call GetValueAsString() -- we need to return the value the way the custom property formatted it
                value << m_prop_grid->GetPropertyValue(property).GetString().wx_str();
                ttlib::multistr parts(value, BMP_PROP_SEPARATOR);
                // If the image field is empty, then the entire property needs to be cleared
                if (parts.size() > IndexImage && parts[IndexImage].empty())
                    value.clear();
                modifyProperty(prop, value);
            }
            break;

        case type_file:
            {
                ttString newValue = property->GetValueAsString();

                // The base_file property was already processed in OnPropertyGridChanging so only modify the value if it's a
                // different property
                if (!prop->isProp(prop_base_file))
                {
                    if (newValue.size())
                    {
                        newValue.make_absolute();
                        newValue.make_relative_wx(wxGetApp().GetProjectPath());
                        newValue.backslashestoforward();
                        property->SetValueFromString(newValue, 0);
                    }
                }
                ModifyProperty(prop, newValue);
            }
            break;

        case type_path:
            {
                ttString newValue = property->GetValueAsString();
                newValue.make_absolute();
                newValue.make_relative_wx(wxGetApp().GetProjectPath());
                newValue.backslashestoforward();

                // Note that on Windows, even though we changed the property to a forward slash, it will still be displayed
                // with a backslash. However, ModifyProperty() will save our forward slash version, so even thought the
                // display isn't correct, it will be stored in the project file correctly.

                property->SetValueFromString(newValue, 0);
                ModifyProperty(prop, newValue);
            }
            break;

        default:
            {
                ttString newValue = property->GetValueAsString();

                if (prop->isProp(prop_var_name))
                {
                    if (newValue.empty())
                    {
                        // An empty name will generate uncompilable code, so we simply switch it to the default name
                        auto new_name = prop->GetPropDeclaration()->GetDefaultValue();
                        auto final_name = node->GetUniqueName(new_name);
                        newValue = final_name.size() ? final_name : new_name;

                        auto grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                        grid_property->SetValueFromString(newValue, 0);
                    }
                }
                else if (prop->isProp(prop_contents))
                {
#if defined(_WIN32)
                    // Under Windows 10 using wxWidgets 3.1.3, the last character of the string is partially clipped. Adding
                    // a trailing space prevents this clipping.

                    if (m_currentSel->isGen(gen_wxRadioBox) && newValue.size())
                    {
                        size_t result;
                        for (size_t pos = 0; pos < newValue.size();)
                        {
                            result = newValue.find("\" \"", pos);
                            if (ttlib::is_found(result))
                            {
                                if (newValue.at(result - 1) != ' ')
                                    newValue.insert(result, ' ');
                                pos = result + 3;
                            }
                            else
                            {
                                break;
                            }
                        }

                        result = newValue.find_last_of('"');
                        if (ttlib::is_found(result))
                        {
                            if (newValue.at(result - 1) != ' ')
                                newValue.insert(result, ' ');
                        }
                    }
#endif  // _WIN32
                }

                ModifyProperty(prop, newValue);

                if (prop->isProp(prop_class_name))
                {
                    auto selected_node = wxGetFrame().GetSelectedNode();
                    if (!selected_node)
                        return;

                    if (selected_node->IsForm())
                    {
                        if (newValue.Right(4) != "Base")
                            return;

                        if (auto propType = selected_node->get_prop_ptr(prop_derived_class_name);
                            propType && propType->as_string() == "DerivedClass")
                            ReplaceDrvName(newValue, propType);
                        if (auto propType = selected_node->get_prop_ptr(prop_base_file);
                            propType && propType->as_string() == "filename_base")
                            ReplaceBaseFile(newValue, propType);
                        if (auto propType = selected_node->get_prop_ptr(prop_derived_file);
                            propType && propType->as_string().empty())
                            ReplaceDrvFile(newValue, propType);
                    }
                }
            }
            break;
    }

    if (node->isGen(gen_wxContextMenuEvent))
    {
        auto event_prop = node->GetParent()->GetEvent("wxEVT_CONTEXT_MENU");
        if (event_prop)
        {
            event_prop->set_value(node->prop_as_string(prop_handler_name));
        }
    }
}

void PropGridPanel::OnEventGridChanged(wxPropertyGridEvent& event)
{
    if (auto it = m_event_map.find(event.GetProperty()); it != m_event_map.end())
    {
        NodeEvent* evt = it->second;
        wxString handler = event.GetPropertyValue();
        auto value = ConvertEscapeSlashes(ttlib::cstr() << handler.wx_str());
        value.trim(tt::TRIM::both);
        wxGetFrame().ChangeEventHandler(evt, value);
    }
}

void PropGridPanel::OnPropertyGridExpand(wxPropertyGridEvent& event)
{
    m_expansion_map[event.GetPropertyName().utf8_str().data()] = event.GetProperty()->IsExpanded();

    auto egProp = m_event_grid->GetProperty(event.GetProperty()->GetName());
    if (egProp)
    {
        if (event.GetProperty()->IsExpanded())
        {
            m_event_grid->Expand(egProp);
        }
        else
        {
            m_event_grid->Collapse(egProp);
        }
    }
}

void PropGridPanel::OnEventGridExpand(wxPropertyGridEvent& event)
{
    m_expansion_map[event.GetPropertyName().utf8_str().data()] = event.GetProperty()->IsExpanded();

    auto grid_property = m_prop_grid->GetProperty(event.GetProperty()->GetName());
    if (grid_property)
    {
        if (event.GetProperty()->IsExpanded())
        {
            m_prop_grid->Expand(grid_property);
        }
        else
        {
            m_prop_grid->Collapse(grid_property);
        }
    }
}

// This handles the custom event fired when a node's property is changed.
void PropGridPanel::OnNodePropChange(CustomEvent& event)
{
    if (m_isPropChangeSuspended)
    {
        // If the property was modified in the property grid, then we are receiving this event after the node in the property
        // has already been changed. We don't need to process it since we already saw it, but we can use the oppoprtunity to
        // do some additional processing, such as notifying the user that the Mockup can't display the property change.

        if (event.GetNodeProperty()->isProp(prop_border))
        {
            auto info = wxGetFrame().GetPropInfoBar();
            info->Dismiss();
            if (event.GetNodeProperty()->as_string() == "wxBORDER_RAISED")
            {
                info->ShowMessage("The Mockup panel is not able to show a mockup of the raised border.", wxICON_INFORMATION);
            }
        }
        return;
    }

    auto prop = event.GetNodeProperty();
    auto grid_property = m_prop_grid->GetPropertyByLabel(prop->DeclName().wx_str());
    if (!grid_property)
        return;

    switch (prop->type())
    {
        case type_float:
            grid_property->SetValue(WXVARIANT(prop->as_float()));
            break;

        case type_int:
        case type_uint:
            grid_property->SetValueFromString(prop->as_string(), 0);
            break;

        case type_string:
        case type_string_edit:
            grid_property->SetValueFromString(prop->as_string(), 0);
            break;

        case type_string_edit_escapes:
        case type_string_escapes:
            grid_property->SetValueFromString(prop->as_escape_text().wx_str(), 0);
            break;

        case type_id:
        case type_option:
        case type_editoption:
            grid_property->SetValueFromString(prop->as_escape_text(), 0);
            break;

        case type_bool:
            grid_property->SetValueFromInt(prop->as_string() == "0" ? 0 : 1, 0);
            break;

        case type_bitlist:
            {
                auto value = prop->as_string();
                value.Replace("|", ", ", true);
                if (value == "0")
                    value = "";
                grid_property->SetValueFromString(value, 0);
            }
            break;

        case type_wxPoint:
            {
                // m_prop_grid->SetPropertyValue( grid_property, prop->GetValue() );
                auto aux = prop->as_string();
                aux.Replace(",", ";");
                grid_property->SetValueFromString(aux, 0);
            }
            break;

        case type_wxSize:
            {
                // m_prop_grid->SetPropertyValue( grid_property, prop->GetValue() );
                auto aux = prop->as_string();
                aux.Replace(",", ";");
                grid_property->SetValueFromString(aux, 0);
            }
            break;

        case type_wxFont:
            // REVIEW: [KeyWorks - 07-03-2020] Why not just use SetValueFromString like the others? And for that matter,
            // when is this being called?
            grid_property->SetValue(WXVARIANT(prop->as_string()));
            break;

        case type_wxColour:
            {
                auto value = prop->as_string();
                if (value.empty())  // Default Colour
                {
                    wxColourPropertyValue def;
                    def.m_type = wxSYS_COLOUR_WINDOW;
                    def.m_colour = ConvertToSystemColour("wxSYS_COLOUR_WINDOW");
                    m_prop_grid->SetPropertyValue(grid_property, def);
                }
                else
                {
                    if (value.find_first_of("wx") == 0)
                    {
                        // System Colour
                        wxColourPropertyValue def;
                        def.m_type = ConvertToSystemColour(value);
                        def.m_colour = prop->as_color();
                        m_prop_grid->SetPropertyValue(grid_property, def);
                    }
                    else
                    {
                        wxColourPropertyValue def(wxPG_COLOUR_CUSTOM, prop->as_color());
                        m_prop_grid->SetPropertyValue(grid_property, def);
                    }
                }
            }
            break;

        case type_animation:
        case type_image:
            break;

        default:
            grid_property->SetValueFromString(prop->as_string(), wxPG_FULL_VALUE);
    }
    m_prop_grid->Refresh();
}

void PropGridPanel::ModifyProperty(NodeProperty* prop, const wxString& str)
{
    m_isPropChangeSuspended = true;
    wxGetFrame().ModifyProperty(prop, ttlib::cstr() << str.wx_str());
    m_isPropChangeSuspended = false;
}

void PropGridPanel::modifyProperty(NodeProperty* prop, ttlib::cview str)
{
    m_isPropChangeSuspended = true;
    wxGetFrame().ModifyProperty(prop, str);
    m_isPropChangeSuspended = false;
}

void PropGridPanel::OnChildFocus(wxChildFocusEvent&)
{
    // do nothing to avoid "scrollbar jump" if wx2.9 is used
}

void PropGridPanel::OnPropertyGridItemSelected(wxPropertyGridEvent& event)
{
    auto property = event.GetProperty();
    if (property)
    {
        if (m_notebook_parent->GetSelection() == 0)
        {
            m_selected_prop_name = m_prop_grid->GetPropertyName(property);
            m_pageName = "Properties";
        }
        else
        {
            m_selected_event_name = m_event_grid->GetPropertyName(property);
            m_pageName = "Events";
        }
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

    else if (category_name == "wxWindow")
        category_name = "wxWindow Properties";
    else if (category_name == "Project")
        category_name = "Project Settings";

    return category_name;
}

void PropGridPanel::CreatePropCategory(ttlib::cview name, Node* node, NodeDeclaration* declaration, PropNameSet& prop_set)
{
    auto& category = declaration->GetCategory();

    if (!category.GetCategoryCount() && !category.GetPropNameCount())
        return;

    m_prop_grid->AddPage();

    auto id = m_prop_grid->Append(new wxPropertyCategory(GetCategoryDisplayName(category.GetName())));
    AddProperties(name, node, category, prop_set);

    // Collapse categories that aren't likely to be used with the current object
    if (name.is_sameas("AUI"))
    {
        // TODO: [KeyWorks - 07-25-2020] Need to see if parent is using AUI, and if so, don't collapse this
        m_prop_grid->Collapse(id);
    }
    else if (name.is_sameas("Bitmaps") || name.is_sameas("Command Bitmaps"))
    {
        m_prop_grid->Collapse(id);
        m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#dce4ef"));
    }
    else if (name.contains("Validator"))
    {
        m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#fff1d2"));

        // It's going to be rare to want a validator for these classes, so collapse the validator for them
        if (node->isGen(gen_wxButton) || node->isGen(gen_wxStaticText))
            m_prop_grid->Collapse(id);
    }
    else if (name.contains("CMake"))
    {
        m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#fff1d2"));
    }

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

static constexpr std::initializer_list<PropName> lst_LayoutProps = {

    prop_alignment, prop_borders, prop_border_size, prop_flags

};

// clang-format off
static constexpr std::initializer_list<PropName> lst_GridBagProps = {

    prop_borders,
    prop_border_size,
    prop_flags,
    prop_row,
    prop_column,
    prop_rowspan,
    prop_colspan

};
// clang-format on

void PropGridPanel::CreateLayoutCategory(Node* node)
{
    m_prop_grid->AddPage();

    auto id = m_prop_grid->Append(new wxPropertyCategory("Layout"));

    if (!node->isParent(gen_wxGridBagSizer))
    {
        for (auto iter: lst_LayoutProps)
        {
            auto prop = node->get_prop_ptr(iter);
            if (!prop)
                continue;

            auto id_prop = m_prop_grid->Append(GetProperty(prop));

            auto propInfo = prop->GetPropDeclaration();
            m_prop_grid->SetPropertyHelpString(id_prop, propInfo->GetDescription());

            m_property_map[id_prop] = prop;
        }

        if (auto prop = node->get_prop_ptr(prop_proportion); prop)
        {
            auto id_prop = m_prop_grid->Append(GetProperty(prop));

            auto propInfo = prop->GetPropDeclaration();
            m_prop_grid->SetPropertyHelpString(id_prop, propInfo->GetDescription());

            m_property_map[id_prop] = prop;
        }
    }
    else
    {
        for (auto iter: lst_GridBagProps)
        {
            auto prop = node->get_prop_ptr(iter);
            if (!prop)
                continue;

            auto id_prop = m_prop_grid->Append(GetProperty(prop));

            auto propInfo = prop->GetPropDeclaration();
            m_prop_grid->SetPropertyHelpString(id_prop, propInfo->GetDescription());

            m_property_map[id_prop] = prop;
        }
    }

    m_prop_grid->Expand(id);

    m_prop_grid->SetPropertyBackgroundColour(id, wxColour("#e1f3f8"));
}

void PropGridPanel::CreateEventCategory(ttlib::cview name, Node* node, NodeDeclaration* declaration, EventSet& event_set)
{
    auto& category = declaration->GetCategory();

    if (!category.GetCategoryCount() && !category.GetEventCount())
        return;

    if (category.GetName() == "wxWindow")
    {
        if (node->GetNodeDeclaration()->GetGeneratorFlags().contains("no_win_events"))
            return;
    }

    m_event_grid->AddPage();

    auto id = m_event_grid->Append(new wxPropertyCategory(GetCategoryDisplayName(category.GetName())));

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

void PropGridPanel::ReplaceDrvName(const wxString& newValue, NodeProperty* propType)
{
    wxString drvName = newValue;
    drvName.Replace("Base", wxEmptyString);
    auto grid_property = m_prop_grid->GetPropertyByLabel("derived_class_name");
    grid_property->SetValueFromString(drvName, 0);
    ModifyProperty(propType, drvName);
}

void PropGridPanel::ReplaceBaseFile(const wxString& newValue, NodeProperty* propType)
{
    ttString baseName = newValue;
    if (baseName.Right(4) == "Base")
        baseName.Replace("Base", wxEmptyString);
    baseName.MakeLower();
    baseName << "_base";
    if (wxGetApp().GetProject()->HasValue(prop_base_directory))
        baseName.insert(0, wxGetApp().GetProject()->prop_as_wxString(prop_base_directory) << '/');
    auto grid_property = m_prop_grid->GetPropertyByLabel("base_file");
    grid_property->SetValueFromString(baseName, 0);
    ModifyProperty(propType, baseName);
}

void PropGridPanel::ReplaceDrvFile(const wxString& newValue, NodeProperty* propType)
{
    ttString drvName = newValue;
    if (drvName.contains("Base"))
        drvName.Replace("Base", wxEmptyString);
    else
        drvName << "_drv";
    drvName.MakeLower();
    if (wxGetApp().GetProject()->HasValue(prop_base_directory))
        drvName.insert(0, wxGetApp().GetProject()->prop_as_wxString(prop_base_directory) << '/');
    auto grid_property = m_prop_grid->GetPropertyByLabel("derived_file");
    grid_property->SetValueFromString(drvName, 0);
    ModifyProperty(propType, drvName);
}

bool PropGridPanel::IsPropAllowed(Node* /* node */, NodeProperty* /* prop */)
{
    // TODO: [KeyWorks - 04-10-2021] The original properties that were ignored were replaced, so this is now just a
    // placeholder. It is called, so if needed, this would be where properties could be disabled, presumably based on the
    // parent.

    return true;
}

void PropGridPanel::VerifyChangeFile(wxPropertyGridEvent& event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_base_file))
    {
        ttString newValue = event.GetPropertyValue().GetString();
        if (newValue.empty())
            return;

        newValue.make_absolute();
        newValue.make_relative_wx(wxGetApp().GetProjectPath());
        newValue.backslashestoforward();
        auto filename = newValue.sub_cstr();
        auto project = wxGetApp().GetProject();
        for (size_t child_idx = 0; child_idx < project->GetChildCount(); ++child_idx)
        {
            if (project->GetChild(child_idx) == node)
                continue;
            if (project->GetChild(child_idx)->prop_as_string(prop_base_file).filename() == filename)
            {
                wxMessageBox(wxString() << "The base filename " << filename << " is already in use by "
                                        << project->GetChild(child_idx)->prop_as_string(prop_class_name)
                                        << "\n\nEither change the name, or press ESC to restore the original name.");
                m_failure_handled = true;
                event.Veto();
                return;
            }
        }

        // If the event was previously veto'd, and the user corrected the file, then we have to set it here,
        // otherwise it will revert back to the original name before the Veto.

        event.GetProperty()->SetValueFromString(newValue, 0);
    }
}
