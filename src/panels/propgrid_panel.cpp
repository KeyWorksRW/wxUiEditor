/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
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
#include <wx/wupdlock.h>           // wxWindowUpdateLocker prevents window redrawing

#include "propgrid_panel.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "base_panel.h"       // BasePanel -- Code generation panel
#include "bitmaps.h"          // Map of bitmaps accessed by name
#include "category.h"         // NodeCategory class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "font_prop.h"        // FontProperty -- FontProperty class
#include "image_handler.h"    // ImageHandler class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "node_decl.h"        // NodeDeclaration class
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "paths.h"            // Handles *_directory properties
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "prop_decl.h"        // PropChildDeclaration and PropDeclaration classes
#include "undo_cmds.h"        // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties

// Various customized wxPGProperty classes

#include "../customprops/code_string_prop.h"    // EditCodeDialogAdapter -- Derived wxStringProperty class for code
#include "../customprops/custom_colour_prop.h"  // EditColourDialogAdapter -- Property editor for colour
#include "../customprops/custom_param_prop.h"   // EditParamProperty -- dialog for editing CustomControl parameter
#include "../customprops/directory_prop.h"      // DirectoryDialogAdapter
#include "../customprops/edit_custom_mockup.h"  // Custom Property editor for pop_custom_mockup
#include "../customprops/evt_string_prop.h"     // EventStringProperty -- dialog for editing event handlers
#include "../customprops/font_string_prop.h"    // FontStringProperty
#include "../customprops/html_string_prop.h"    // EditHtmlProperty
#include "../customprops/id_prop.h"             // ID_Property
#include "../customprops/include_files_prop.h"  // IncludeFilesProperty
#include "../customprops/pg_animation.h"        // PropertyGrid_Animation -- Custom property grid class for animations
#include "../customprops/pg_image.h"            // PropertyGrid_Image -- Custom property grid class for images
#include "../customprops/pg_point.h"            // CustomPointProperty -- custom wxPGProperty for handling wxPoint
#include "../customprops/rearrange_prop.h"      // RearrangeProperty -- Property editor for rearranging items
#include "../customprops/sb_fields_prop.h"      // SBarFieldsProperty -- Property editor for status bar fields
#include "../customprops/tt_file_property.h"    // ttFileProperty -- Property editor for file names
#include "../customprops/txt_string_prop.h"     // EditStringProperty -- dialog for editing single-line strings

#include "id_lists.h"  // wxID_ strings

using namespace GenEnum;

constexpr auto PROPERTY_ID = wxID_HIGHEST + 1;
constexpr auto EVENT_ID = PROPERTY_ID + 1;

static std::map<GenLang, std::string> s_lang_category_prefix;

PropGridPanel::PropGridPanel(wxWindow* parent, MainFrame* frame) : wxPanel(parent)
{
    for (size_t lang = 1; lang <= GEN_LANG_LAST; lang <<= 1)
    {
        s_lang_category_prefix[static_cast<GenLang>(lang)] = ConvertFromGenLang(static_cast<GenLang>(lang));
    }

    for (auto& iter: list_wx_ids)
    {
        m_astr_wx_ids.Add(iter);
    }

    m_astr_wx_decorations.Add("__declspec(dllexport)");
    m_astr_wx_decorations.Add("__attribute__((dllexport))");
    m_astr_wx_decorations.Add("[[gnu::dllexport]]");

    m_notebook_parent = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook_parent->SetArtProvider(new wxAuiSimpleTabArt());

    m_prop_grid = new CustomPropertyManager;
    m_prop_grid->Create(m_notebook_parent, PROPERTY_ID, wxDefaultPosition, wxDefaultSize,
                        wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_DESCRIPTION);

    m_event_grid = new wxPropertyGridManager(m_notebook_parent, EVENT_ID, wxDefaultPosition, wxDefaultSize,
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

    m_notebook_parent->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &PropGridPanel::OnAuiNotebookPageChanged, this);

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

        // Note that AddPage() won't actually add a page, it simply sets an internal flag to indicate there is one page.
        // That's required for m_prop_grid->Clear() to work -- because Clear() *only* clears pages.

        m_prop_grid->Clear();
        m_prop_grid->AddPage();
        m_event_grid->Clear();
        m_event_grid->AddPage();

        m_property_map.clear();
        m_event_map.clear();

        tt_string lang_created;

        if (auto declaration = node->getNodeDeclaration(); declaration)
        {
            // These sets are used to prevent trying to add a duplicate property or event to the property grid. In Debug
            // builds, attempting to do so will generate an assert message telling you the name of the duplicate and the node
            // declaration it occurs in. In release builds, only the first instance will be displayed.

            PropNameSet prop_set;
            EventSet event_set;

            CreatePropCategory(declaration->declName(), node, declaration, prop_set);
            CreateEventCategory(declaration->declName(), node, declaration, event_set);

            // Calling GetBaseClassCount() is exepensive, so do it once and store the result
            auto num_base_classes = declaration->GetBaseClassCount();

            auto lang_prefix = ConvertFromGenLang(Project.getCodePreference());

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
                                CreatePropCategory(info_base->declName(), node, info_base, prop_set);
                            }
                            else
                                CreateEventCategory(info_base->declName(), node, info_base, event_set);
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

                        if (m_preferred_lang == GEN_LANG_CPLUSPLUS && info_base->declName().contains("Settings"))
                        {
                            info_base = declaration->GetBaseClass(++i);
                            CreatePropCategory(info_base->declName(), node, info_base, prop_set);
                            info_base = declaration->GetBaseClass(++i);
                            CreatePropCategory(info_base->declName(), node, info_base, prop_set);
                        }

                        break;
                    }
                }

                // At this point, we've created any pre-language categories, and the preferred language
                // categories. Now we create any remaining categories.
                for (; lang_start < num_base_classes; lang_start++)
                {
                    auto* info_base = declaration->GetBaseClass(lang_start);
                    if (info_base->isGen(gen_sizer_child))
                        continue;
                    if (!info_base->declName().is_sameas("Window Events"))
                    {
                        if (info_base->declName().is_sameprefix(lang_prefix))
                        {
                            if (m_preferred_lang == GEN_LANG_CPLUSPLUS && info_base->declName().contains("Settings"))
                            {
                                lang_start += 2;  // skip over Header Settings and Derived Class Settings
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
                        if ((node->isForm() || node->isGen(gen_Project)) && info_base->declName().is_sameprefix(lang_prefix))
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

        case type_string_code_single:
            // This includes a button that triggers a small single-line custom text editor dialog
            return new EditParamProperty(prop->declName().make_wxString(), prop);

        case type_string_escapes:
            // This first doubles the backslash in escaped characters: \n, \t, \r, and \.
            return new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                        prop->as_escape_text().make_wxString());

        case type_string:
            return new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_wxString());

        case type_string_edit_escapes:
            // This includes a button that triggers a small text editor dialog
            // This doubles the backslash in escaped characters: \n, \t, \r, and \.
            return new wxLongStringProperty(prop->declName().make_wxString(), wxPG_LABEL,
                                            prop->as_escape_text().make_wxString());

        case type_string_edit:
            // This includes a button that triggers a small text editor dialog
            return new wxLongStringProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_wxString());

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
            return new wxBoolProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_string() == "1");

        case type_wxPoint:
            return new CustomPointProperty(prop->declName().make_wxString(), prop, CustomPointProperty::type_point);

        case type_wxSize:
            return new CustomPointProperty(prop->declName().make_wxString(), prop, CustomPointProperty::type_size);

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
            return new wxFloatProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_float());

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
                        // If not testing, do not show Code preference options for code we don't currently generate
                        if (iter.name != "C++" && iter.name != "Python" && iter.name != "Ruby" && iter.name != "XRC")
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
                new_pg_property = new wxFlagsProperty(prop->declName().make_wxString(), wxPG_LABEL, bit_flags, val);

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
                        // If not testing, do not show Code preference options for code we don't currently generate
                        if (iter.name != "C++" && iter.name != "Python" && iter.name != "Ruby" && iter.name != "XRC")
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
                    new_pg_property = new wxEditEnumProperty(prop->declName().make_wxString(), wxPG_LABEL, constants);
                }
                else
                {
                    new_pg_property = new wxEnumProperty(prop->declName().make_wxString(), wxPG_LABEL, constants);
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
                    case prop_derived_file:
                    case prop_xrc_file:
                    case prop_combined_xrc_file:
                    case prop_folder_combined_xrc_file:
                    case prop_perl_file:
                    case prop_python_file:
                    case prop_python_combined_file:
                    case prop_ruby_file:
                    case prop_ruby_combined_file:
                    case prop_rust_file:
                    case prop_fortran_file:
                    case prop_haskell_file:
                    case prop_lua_file:
                    case prop_cmake_file:
                    case prop_folder_cmake_file:
                    case prop_subclass_header:
                    case prop_output_file:
                    case prop_data_file:
                        return new ttFileProperty(prop);

                    default:
                        break;
                }

                new_pg_property = new wxFileProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_string());

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
                            new_pg_property->SetAttribute(wxPG_FILE_WILDCARD, "Header Files|*.h;*.hh;*.hpp;*.hxx");

                            // Often the project file will be kept in a sub-directory, with the precompiled header file in
                            // the parent directory. If we can find a standard precompiled header filename in the parent
                            // directory, then use that as the starting directory.

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

                            new_pg_property->SetAttribute(wxPG_FILE_INITIAL_PATH, Project.getProjectPath().make_wxString());
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
                new_pg_property =
                    new wxArrayStringProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_wxArrayString());
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
                new_pg_property =
                    new wxArrayStringProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_wxArrayString());
                wxVariant delimiter(";");
                new_pg_property->SetAttribute(wxPG_ARRAY_DELIMITER, delimiter);
            }
            return new_pg_property;

        case type_stringlist_escapes:
            {
                new_pg_property =
                    new wxArrayStringProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_wxArrayString());
                wxVariant var_quote("\"");
                new_pg_property->SetAttribute(wxPG_ARRAY_DELIMITER, var_quote);
            }
            return new_pg_property;

        case type_uintpairlist:
            return new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_string());

        default:  // Unknown property
            {
                new_pg_property = new wxStringProperty(prop->declName().make_wxString(), wxPG_LABEL, prop->as_string());
                new_pg_property->SetAttribute(wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, wxVariant(true, "true"));

                if (wxGetApp().isTestingMenuEnabled())
                {
                    for (auto& iter: umap_PropTypes)
                    {
                        if (iter.second == type)
                        {
                            MSG_ERROR(tt_string("NodeProperty type is unsupported: ") << iter.first);
                            break;
                        }
                    }
                }
            }
            return new_pg_property;
    }  // end switch (type)
}

void PropGridPanel::AddProperties(tt_string_view name, Node* node, NodeCategory& category, PropNameSet& prop_set,
                                  bool is_child_cat)
{
    size_t propCount = category.getPropNameCount();
    for (size_t i = 0; i < propCount; i++)
    {
        auto prop_name = category.getPropName(i);
        auto prop = node->getPropPtr(prop_name);

        if (!prop)
            continue;

        if (node->getNodeDeclaration()->IsPropHidden(prop_name))
            continue;

        if (prop_set.find(prop_name) == prop_set.end())
        {
            if (!IsPropAllowed(node, prop))
                continue;

            auto pg = m_prop_grid->Append(CreatePGProperty(prop));
            auto propType = prop->type();
            if (propType != type_option)
            {
                if (auto gen = node->getGenerator(); gen)
                {
                    if (auto result = gen->GetHint(prop); result)
                    {
                        m_prop_grid->SetPropertyAttribute(pg, wxPG_ATTR_HINT, result->make_wxString());
                    }
                }
                m_prop_grid->SetPropertyHelpString(pg, GetPropHelp(prop));

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
                        m_prop_grid->SetPropertyAttribute(pg, wxPG_ATTR_AUTOCOMPLETE, m_astr_wx_decorations);
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
            MSG_WARNING(tt_string("The property ")
                        << map_PropNames[prop_name] << " appears more than once in " << node->declName());
        }
    }

    for (size_t i = 0; i < propCount; i++)
    {
        ChangeEnableState(node->getPropPtr(category.getPropName(i)));
    }

    for (auto& nextCat: category.getCategories())
    {
        if (!nextCat.getCategoryCount() && !nextCat.getPropNameCount())
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

        // wxStyledTextCtrl has several categories most of which are rarely used, so it makes sense to collapse them
        // initially.
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

void PropGridPanel::AddEvents(tt_string_view name, Node* node, NodeCategory& category, EventSet& event_set)
{
    auto& eventList = category.getEvents();
    for (auto& eventName: eventList)
    {
        auto event = node->getEvent(eventName);

        if (!event)
            continue;

        auto eventInfo = event->getEventInfo();

        ASSERT_MSG(event_set.find(eventName) == event_set.end(), tt_string("Encountered a duplicate event in ")
                                                                     << node->declName());
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
            if (node->getNodeDeclaration()->GetGeneratorFlags().contains("no_key_events"))
                continue;
        }
        else if (nextCat.GetName() == "Mouse Events")
        {
            if (node->getNodeDeclaration()->GetGeneratorFlags().contains("no_mouse_events"))
                continue;
        }
        else if (nextCat.GetName() == "Focus Events")
        {
            if (node->getNodeDeclaration()->GetGeneratorFlags().contains("no_focus_events"))
                continue;
        }

        if (!nextCat.getCategoryCount() && !nextCat.getEventCount())
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
                    if (auto event = node->getEvent(iter); event && event->get_value().size())
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
                    if (auto event = node->getEvent(iter); event && event->get_value().size())
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
    auto node = prop->getNode();
    auto generator = node->getGenerator();
    if (generator)
    {
        if (!generator->AllowPropertyChange(&event, prop, node))
        {
            return;
        }
    }

    switch (prop->type())
    {
        case type_file:
            AllowFileChange(event, prop, node);
            break;

        case type_path:
            if (!node->isGen(gen_wxFilePickerCtrl))
            {
                AllowDirectoryChange(event, prop, node);
            }
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
    if (prop->get_name() == prop_code_preference)
    {
        // TODO: [Randalphwa - 10-23-2024] Either code preferences should only show
        // prop_generate_languages, or prop_generate_languages should always update the code
        // preferences. Even better would be to disable the matching generate language bit so that
        // the user can't shut it off.

        modifyProperty(prop, m_prop_grid->GetPropertyValueAsString(property).utf8_string());
        auto grid_iterator = m_prop_grid->GetCurrentPage()->GetIterator(wxPG_ITERATE_CATEGORIES);
        while (!grid_iterator.AtEnd())
        {
            auto grid_property = grid_iterator.GetProperty();
            if (grid_property->GetLabel().Contains("C++"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "C++")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Perl"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Perl")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Python"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Python")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Ruby"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Ruby")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Rust"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Rust")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("XRC"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "XRC")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
#if GENERATE_NEW_LANG_CODE
            else if (grid_property->GetLabel().Contains("Fortran"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Fortran")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Haskell"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Haskell")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Lua"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Lua")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
#endif  // GENERATE_NEW_LANG_CODE

            grid_iterator.Next();
        }

        // This will cause GenerateDlg to initialize the code generation choice to the
        // preferred language, clearing any previously selected language. We can't just use
        // the property, because the dialog needs to know if inherited C++ code should be
        // written.
        auto config = wxConfig::Get();
        config->Write("GenCode", 0);

        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().UpdateLanguagePanels();

        return;
    }

    auto node = prop->getNode();

    switch (prop->type())
    {
        case type_float:
            {
                double val = m_prop_grid->GetPropertyValueAsDouble(property);

                modifyProperty(prop, DoubleToStr(val));
                break;
            }

        case type_code_edit:
        case type_html_edit:
        case type_wxFont:
        case type_string_edit:
        case type_id:
        case type_int:
        case type_include_files:
        case type_uint:
        case type_statbar_fields:
        case type_checklist_item:
            {
                modifyProperty(prop, m_prop_grid->GetPropertyValueAsString(property).utf8_string());
                break;
            }

        case type_option:
        case type_editoption:
            ModifyOptionsProperty(prop, property);
            break;

        case type_string_escapes:
        case type_string_edit_escapes:
            {
                auto value = ConvertEscapeSlashes(m_prop_grid->GetPropertyValueAsString(property).utf8_string());
                modifyProperty(prop, value);
            }
            break;

        case type_stringlist:
#if defined(_WIN32)
            if (prop->isProp(prop_contents))
            {
                // REVIEW: [Randalphwa - 06-26-2023] This will only work if we use quotes to
                // separate items.
                tt_string newValue = property->GetValueAsString().utf8_string();
                // Under Windows 10 using wxWidgets 3.1.3, the last character of the string is partially clipped.
                // Adding a trailing space prevents this clipping.

                if (m_currentSel->isGen(gen_wxRadioBox) && newValue.size())
                {
                    size_t result;
                    for (size_t pos = 0; pos < newValue.size();)
                    {
                        result = newValue.find("\" \"", pos);
                        if (tt::is_found(result))
                        {
                            if (newValue.at(result - 1) != ' ')
                                newValue.insert(result, 1, ' ');
                            pos = result + 3;
                        }
                        else
                        {
                            break;
                        }
                    }

                    result = newValue.find_last_of('"');
                    if (tt::is_found(result))
                    {
                        if (newValue.at(result - 1) != ' ')
                            newValue.insert(result, 1, ' ');
                    }
                    modifyProperty(prop, newValue);
                    break;
                }
            }
#endif  // _WIN32
            modifyProperty(prop, property->GetValueAsString().utf8_string());
            break;

        case type_stringlist_semi:
            modifyProperty(prop, property->GetValueAsString().utf8_string());
            break;

        case type_bool:
            ModifyBoolProperty(prop, property);
            break;

        case type_bitlist:
            ModifyBitlistProperty(prop, property);
            break;

        case type_wxPoint:
        case type_wxSize:
            {
                auto value = event.GetPropertyValue().GetString();
                modifyProperty(prop, value.utf8_string());
            }
            break;

        case type_animation:
        case type_image:
            ModifyEmbeddedProperty(prop, property);
            break;

        case type_file:
            ModifyFileProperty(prop, property);
            break;

        case type_path:
            {
                m_isPropChangeSuspended = true;
                OnPathChanged(event, prop, node);
                m_isPropChangeSuspended = false;
            }
            break;

        default:
            {
                tt_string newValue = property->GetValueAsString().utf8_string();

                if (prop->isProp(prop_var_name))
                {
                    if (newValue.empty())
                    {
                        // An empty name will generate uncompilable code, so we simply switch it to the default name
                        auto new_name = prop->getPropDeclaration()->getDefaultValue();
                        auto final_name = node->getUniqueName(new_name);
                        newValue = final_name.size() ? final_name : new_name;

                        auto grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                        grid_property->SetValueFromString(newValue.make_wxString());
                    }
                }

                modifyProperty(prop, newValue);

                if (prop->isProp(prop_class_name))
                {
                    if (auto selected_node = wxGetFrame().getSelectedNode(); selected_node && selected_node->isForm())
                    {
                        CheckOutputFile(newValue, selected_node);

                        if (Project.getCodePreference() == GEN_LANG_CPLUSPLUS)
                        {
                            if (!selected_node->as_bool(prop_use_derived_class))
                                return;

                            if (!selected_node->hasValue(prop_derived_class_name))
                            {
                                ReplaceDerivedName(newValue, selected_node->getPropPtr(prop_derived_class_name));
                                ReplaceDerivedFile(selected_node->as_string(prop_derived_class_name),
                                                   selected_node->getPropPtr(prop_derived_file));
                            }
                        }
                    }
                    return;
                }
            }
            break;
    }

    if (node->isGen(gen_wxContextMenuEvent))
    {
        auto event_prop = node->getParent()->getEvent("wxEVT_CONTEXT_MENU");
        if (event_prop)
        {
            event_prop->set_value(node->as_string(prop_handler_name));
        }
    }

    ChangeEnableState(prop);

    if (auto gen = prop->getNode()->getGenerator(); gen)
    {
        auto result = gen->isLanguagePropSupported(prop->getNode(), Project.getCodePreference(), prop->get_name());
        if (result.has_value())
        {
            wxGetFrame().ShowInfoBarMsg(result.value());
        }
        else
        {
            wxGetFrame().DismissInfoBar();
        }
    }
    else
    {
        wxGetFrame().DismissInfoBar();
    }
}

void PropGridPanel::ChangeEnableState(NodeProperty* changed_prop)
{
    if (!changed_prop)
        return;

    // Project properties don't have a generator, so always check if generator exists
    if (auto gen = changed_prop->getNode()->getGenerator(); gen)
    {
        gen->ChangeEnableState(m_prop_grid, changed_prop);
    }
}

void PropGridPanel::OnEventGridChanged(wxPropertyGridEvent& event)
{
    if (auto it = m_event_map.find(event.GetProperty()); it != m_event_map.end())
    {
        NodeEvent* evt = it->second;
        wxString handler = event.GetPropertyValue();
        auto value = ConvertEscapeSlashes(handler.utf8_string());
        value.trim(tt::TRIM::both);
        wxGetFrame().ChangeEventHandler(evt, value);
    }
}

void PropGridPanel::OnPropertyGridExpand(wxPropertyGridEvent& event)
{
    m_expansion_map[event.GetPropertyName().utf8_string()] = event.GetProperty()->IsExpanded();

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
    m_expansion_map[event.GetPropertyName().utf8_string()] = event.GetProperty()->IsExpanded();

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
        // If the property was modified in the property grid, then we are receiving this event after the node in the
        // property has already been changed. We don't need to process it since we already saw it, but we can use the
        // oppoprtunity to do some additional processing, such as notifying the user that the Mockup can't display the
        // property change.

        OnPostPropChange(event);
        return;
    }

    auto prop = event.GetNodeProperty();
    auto grid_property = m_prop_grid->GetPropertyByLabel(prop->declName().make_wxString());
    if (!grid_property)
        return;

    switch (prop->type())
    {
        case type_float:
            grid_property->SetValue(WXVARIANT(prop->as_float()));
            break;

        case type_int:
        case type_uint:
            grid_property->SetValueFromString(prop->as_string());
            break;

        case type_string:
        case type_string_edit:
            grid_property->SetValueFromString(prop->as_string());
            break;

        case type_string_edit_escapes:
        case type_string_escapes:
        case type_stringlist_escapes:
            grid_property->SetValueFromString(prop->as_escape_text().make_wxString());
            break;

        case type_id:
        case type_option:
        case type_editoption:
            grid_property->SetValueFromString(prop->as_escape_text());
            break;

        case type_bool:
            grid_property->SetValueFromInt(prop->as_string() == "0" ? 0 : 1);
            break;

        case type_bitlist:
            {
                auto value = prop->as_string();
                value.Replace("|", ", ", true);
                if (value == "0")
                    value = "";
                grid_property->SetValueFromString(value);
            }
            break;

        case type_wxPoint:
            {
                // m_prop_grid->SetPropertyValue( grid_property, prop->GetValue() );
                auto aux = prop->as_string();
                aux.Replace(",", ";");
                grid_property->SetValueFromString(aux);
            }
            break;

        case type_wxSize:
            {
                // m_prop_grid->SetPropertyValue( grid_property, prop->GetValue() );
                auto aux = prop->as_string();
                aux.Replace(",", ";");
                grid_property->SetValueFromString(aux);
            }
            break;

        case type_wxColour:
            {
                wxColourPropertyValue def(wxPG_COLOUR_CUSTOM, prop->as_color());
                m_prop_grid->SetPropertyValue(grid_property, def);
            }
            break;

        case type_animation:
        case type_image:
            break;

        default:
            grid_property->SetValueFromString(prop->as_string(), wxPGPropValFormatFlags::FullValue);
    }
    m_prop_grid->Refresh();
}

void PropGridPanel::modifyProperty(NodeProperty* prop, tt_string_view str)
{
    m_isPropChangeSuspended = true;
    if (auto* gen = prop->getNode()->getGenerator(); !gen || !gen->modifyProperty(prop, str))
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
            auto style_prop = node->getPropPtr(prop_style);
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
        modifyProperty(node_prop, value);
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().UpdateLanguagePanels();
        return;
    }
    else if (node_prop->isProp(prop_window_style) && value.empty())
    {
        value = "0";
    }
    modifyProperty(node_prop, value);
}

void PropGridPanel::ModifyBoolProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    if (!m_prop_grid->GetPropertyValueAsBool(grid_prop))
    {
        auto node = node_prop->getNode();
        if (node->isGen(gen_wxStdDialogButtonSizer))
        {
            auto def_prop = node->getPropPtr(prop_default_button);
            if (def_prop->as_string() == node_prop->declName())
            {
                m_prop_grid->SetPropertyValue("default_button", "none");
                modifyProperty(def_prop, "none");
            }
        }
    }
    modifyProperty(node_prop, m_prop_grid->GetPropertyValueAsBool(grid_prop) ? "1" : "0");
}

void PropGridPanel::ModifyFileProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    if (node_prop->isProp(prop_data_file))
    {
        tt_string newValue = grid_prop->GetValueAsString(wxPGPropValFormatFlags::FullValue).utf8_string();
        auto result = Project.GetOutputPath(node_prop->getNode()->getForm(), GEN_LANG_CPLUSPLUS);
        auto path = result.first;
        if (result.second)  // true if the the base filename was returned
            path.remove_filename();
        newValue.make_relative(path);
        newValue.backslashestoforward();
        modifyProperty(node_prop, newValue);
        return;
    }

    tt_string newValue = grid_prop->GetValueAsString().utf8_string();

    // The base_file grid_prop was already processed in OnPropertyGridChanging so only modify the value if
    // it's a different grid_prop
    if (!node_prop->isProp(prop_base_file) && !node_prop->isProp(prop_perl_file) && !node_prop->isProp(prop_python_file) &&
        !node_prop->isProp(prop_ruby_file) && !node_prop->isProp(prop_xrc_file) && !node_prop->isProp(prop_fortran_file) &&
        !node_prop->isProp(prop_haskell_file) && !node_prop->isProp(prop_lua_file) && !node_prop->isProp(prop_rust_file))
    {
        if (newValue.size())
        {
            newValue.make_absolute();
            newValue.make_relative(Project.getProjectPath());
            newValue.backslashestoforward();
            grid_prop->SetValueFromString(newValue);
        }
    }
    modifyProperty(node_prop, newValue);

    // Review: [Randalphwa - 06-26-2023] The panel should already have been created
    // wxGetFrame().UpdateLanguagePanels();
}

void PropGridPanel::ModifyEmbeddedProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    // Do NOT call GetPropertyValueAsString() -- we need to return the value the way the custom property formatted it
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
            if (node_prop->getNode()->isGen(gen_embedded_image))
            {
                embed->form = node_prop->getNode()->getParent();
            }
            // If there is an Images List node and it is set to auto add, then fall through
            // to the section below that adds the image to the Images List node.
            else if (!Project.getImagesForm() || !Project.getImagesForm()->as_bool(prop_auto_add))
            {
                modifyProperty(node_prop, value);
                return;
            }
        }
        else
        {
            // This ensures that all images from a bitmap bundle get added
            ProjectImages.UpdateBundle(parts, node_prop->getNode());
        }
    }

    if (value.empty() || node_prop->type() == type_animation || value.starts_with("Art") || value.starts_with("XPM"))
    {
        modifyProperty(node_prop, value);
        return;  // Don't do anything else for animations, art providers or XPMs
    }
    if (value == "Embed;" || value == "SVG;")
    {
        // Don't do anything else for empty embedded images
        modifyProperty(node_prop, value);
        return;
    }

    // We do *not* call modifyProperty() until we are certain that we aren't going to add an
    // image to a gen_Images node. That's because if we do add it, the GroupUndoActions will
    // handle the modification of the property via an ModifyPropertyAction class.

    auto* node = node_prop->getNode();
    auto* parent = node->getParent();

    if (parent->isGen(gen_Images))
    {
        auto filename = parts[IndexImage].filename();
        size_t pos = 0;
        for (const auto& embedded_image: parent->getChildNodePtrs())
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
        if (pos < parent->getChildCount())
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
        auto image_list_node = Project.getImagesForm();
        if (!image_list_node || !image_list_node->as_bool(prop_auto_add))
        {
            modifyProperty(node_prop, value);
            return;
        }
        auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (image_list_node && embed && embed->form != image_list_node)
        {
            embed->form = image_list_node;
            auto filename = parts[IndexImage].filename();
            size_t pos = 0;
            for (const auto& embedded_image: image_list_node->getChildNodePtrs())
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

            auto new_embedded = NodeCreation.createNode(gen_embedded_image, image_list_node).first;
            new_embedded->set_value(prop_bitmap, value);
            auto insert_action = std::make_shared<InsertNodeAction>(new_embedded.get(), image_list_node, tt_empty_cstr, pos);
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
    modifyProperty(node_prop, value);
}

void PropGridPanel::ModifyOptionsProperty(NodeProperty* node_prop, wxPGProperty* grid_prop)
{
    auto node = node_prop->getNode();

    tt_string value = m_prop_grid->GetPropertyValueAsString(grid_prop).utf8_string();
    modifyProperty(node_prop, value);

    // Update displayed description for the new selection
    auto propInfo = node_prop->getPropDeclaration();

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
            if (Project.getCodePreference(selected_node) == GEN_LANG_CPLUSPLUS)
            {
                // If access is changed to local and the name starts with "m_", then
                // the "m_" will be stripped off. Conversely, if the name is changed
                // from local to a class member, a "m_" is added as a prefix if
                // preferred language isw C++.

                if (*access == "none" && name.starts_with("m_"))
                {
                    name.erase(0, 2);
                    auto final_name = node->getUniqueName(name);
                    if (final_name.size())
                        name = final_name;
                    is_name_changed = true;
                }
                else if (*access != "none" && !name.starts_with("m_") &&
                         Project.getCodePreference(selected_node) == GEN_LANG_CPLUSPLUS)
                {
                    name.insert(0, "m_");
                    auto final_name = node->getUniqueName(name);
                    if (final_name.size())
                        name = final_name;
                    is_name_changed = true;
                }
            }
            else if (Project.getCodePreference(selected_node) == GEN_LANG_PYTHON)
            {
                // The convention in Python is to use a leading underscore for local members.

                if (*access == "none" && !name.starts_with("_"))
                {
                    name.insert(0, "_");
                    if (auto final_name = node->getUniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
                else if (*access != "none" && name.starts_with("_"))
                {
                    name.erase(0, 1);
                    if (auto final_name = node->getUniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
            }
            else if (Project.getCodePreference(selected_node) == GEN_LANG_RUBY)
            {
                // The convention in Ruby is to use a leading @ for non-local members.

                if (*access == "none" && name.starts_with("@"))
                {
                    name.erase(0, 1);
                    if (auto final_name = node->getUniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
                else if (*access != "none" && !name.starts_with("@"))
                {
                    name.insert(0, "@");
                    if (auto final_name = node->getUniqueName(name); final_name.size())
                    {
                        name = final_name;
                    }
                    is_name_changed = true;
                }
            }

            if (is_name_changed)
            {
                auto propChange = selected_node->getPropPtr(prop_var_name);
                auto grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                grid_property->SetValueFromString(name);
                modifyProperty(propChange, name);
            }
        }
    }
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

    else if (category_name == "wxWindow" || category_name == "wxMdiWindow")
        category_name = "wxWindow Properties";
    else if (category_name == "Project")
        category_name = "Project Settings";

    return category_name;
}

void PropGridPanel::CreatePropCategory(tt_string_view name, Node* node, NodeDeclaration* declaration, PropNameSet& prop_set)
{
    auto& category = declaration->GetCategory();

    if (!category.getCategoryCount() && !category.getPropNameCount())
        return;

    auto generate_languages = Project.getGenerateLanguages();

    // Ignore if the user doesn't want to generate this language
    if (!(static_cast<size_t>(ConvertToGenLang(name)) & generate_languages))
        return;

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

        // It's going to be rare to want a validator for these classes, so collapse the validator for them
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

void PropGridPanel::CreateEventCategory(tt_string_view name, Node* node, NodeDeclaration* declaration, EventSet& event_set)
{
    auto& category = declaration->GetCategory();

    if (!category.getCategoryCount() && !category.getEventCount())
        return;

    if (category.GetName() == "wxWindow")
    {
        if (node->getNodeDeclaration()->GetGeneratorFlags().contains("no_win_events"))
            return;
    }

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
    modifyProperty(propType, drvName);
}

void PropGridPanel::CheckOutputFile(const tt_string& newValue, Node* node)
{
    auto form_node = node->getForm();

    auto ChangeOutputFile = [&](PropName prop_name)
    {
        if (form_node->hasValue(prop_name))
            return;
        if (auto label = GetPropStringName(prop_name); label)
        {
            auto output_filename = CreateBaseFilename(form_node, newValue);
            auto grid_property = m_prop_grid->GetPropertyByLabel(label->make_wxString());
            grid_property->SetValueFromString(output_filename.make_wxString());
            modifyProperty(form_node->getPropPtr(prop_name), output_filename);
        }
    };

    switch (Project.getCodePreference())
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

#if GENERATE_NEW_LANG_CODE
        case GEN_LANG_FORTRAN:
            ChangeOutputFile(prop_fortran_file);
            break;

        case GEN_LANG_HASKELL:
            ChangeOutputFile(prop_haskell_file);
            break;

        case GEN_LANG_LUA:
            ChangeOutputFile(prop_lua_file);
            break;
#endif  // GENERATE_NEW_LANG_CODE

        default:
            break;
    }
}

void PropGridPanel::ReplaceDerivedFile(const tt_string& newValue, NodeProperty* propType)
{
    auto derived_filename = CreateDerivedFilename(propType->getNode()->getForm(), newValue);
    auto grid_property = m_prop_grid->GetPropertyByLabel("derived_file");
    grid_property->SetValueFromString(derived_filename.make_wxString());
    modifyProperty(propType, derived_filename);
}

bool PropGridPanel::IsPropAllowed(Node* /* node */, NodeProperty* /* prop */)
{
    // If this function returns false, the property will not be created in the property grid. Note
    // that properties marked as hidden in the XML interface will not be passed to this function.

    return true;
}

void PropGridPanel::OnPostPropChange(CustomEvent& event)
{
    if (event.GetNodeProperty()->isProp(prop_border))
    {
        auto info = wxGetFrame().GetPropInfoBar();
        info->Dismiss();
        if (event.GetNodeProperty()->as_string() == "wxBORDER_RAISED")
        {
            info->ShowMessage("The Mockup panel is not able to show a mockup of the raised border.", wxICON_INFORMATION);
        }
    }
    else if (event.GetNodeProperty()->isProp(prop_focus))
    {
        auto node = event.getNode();
        auto form = node->getForm();
        auto list = form->findAllChildProperties(prop_focus);
        size_t count = 0;
        for (auto iter: list)
        {
            if (iter->as_bool())
            {
                ++count;
            }
        }

        if (count > 1)
        {
            wxGetFrame().GetPropInfoBar()->ShowMessage("More than one control has focus set.", wxICON_INFORMATION);
        }
        else
        {
            wxGetFrame().GetPropInfoBar()->Dismiss();
        }
    }
    else if (event.GetNodeProperty()->isProp(prop_size) || event.GetNodeProperty()->isProp(prop_minimum_size) ||
             event.GetNodeProperty()->isProp(prop_maximum_size))
    {
        auto node = event.getNode();
        auto new_size = node->as_wxSize(prop_size);
        auto min_size = node->as_wxSize(prop_minimum_size);
        auto max_size = node->as_wxSize(prop_maximum_size);
        if (new_size != wxDefaultSize || min_size != wxDefaultSize || max_size != wxDefaultSize)
        {
            // If any value is -1 then it's not actually set and no comparison is needed
            if (min_size != wxDefaultSize && ((new_size.x != -1 && min_size.x != -1 && new_size.x < min_size.x) ||
                                              (new_size.y != -1 && min_size.y != -1 && new_size.y < min_size.y)))
            {
                wxGetFrame().GetPropInfoBar()->ShowMessage("The size property is smaller than the minimum size property.",
                                                           wxICON_WARNING);
            }
            else if (max_size != wxDefaultSize && ((new_size.x != -1 && max_size.x != -1 && new_size.x > max_size.x) ||
                                                   (new_size.y != -1 && max_size.y != -1 && new_size.y > max_size.y)))
            {
                wxGetFrame().GetPropInfoBar()->ShowMessage("The size property is larger than the maximum size property.",
                                                           wxICON_WARNING);
            }
            else if (min_size != wxDefaultSize && max_size != wxDefaultSize &&
                     ((min_size.x != -1 && max_size.x != -1 && min_size.x > max_size.x) ||
                      (min_size.y != -1 && max_size.y != -1 && min_size.y > max_size.y)))
            {
                wxGetFrame().GetPropInfoBar()->ShowMessage(
                    "The minimum size property is larger than the maximum size property.", wxICON_WARNING);
            }
            else
            {
                wxGetFrame().GetPropInfoBar()->Dismiss();
            }
        }
    }
}

bool PropGridPanel::IsEventPageShowing()
{
    if (auto page = m_notebook_parent->GetCurrentPage(); page)
    {
        return (page == m_event_grid);
    }
    return false;
}

void PropGridPanel::OnAuiNotebookPageChanged(wxAuiNotebookEvent& /* event */)
{
    if (auto panel = wxGetFrame().GetFirstCodePanel(); panel)
    {
        CustomEvent custom_event(EVT_NodeSelected, wxGetFrame().getSelectedNode());
        panel->OnNodeSelected(custom_event);
    }
}

tt_string PropGridPanel::GetPropHelp(NodeProperty* prop) const
{
    tt_string description;
    if (auto gen = prop->getNode()->getGenerator(); gen)
    {
        // First let the generator specify the description
        if (auto result = gen->GetPropertyDescription(prop); result)
        {
            description = result.value();
        }
    }
    if (description.empty())
    {
        // If the generator didn't specify a description, then look for a description in the help map
        if (auto map_help = GenEnum::map_PropHelp.find(prop->get_name()); map_help != GenEnum::map_PropHelp.end())
        {
            description = map_help->second;
        }
        else
        {
            // If we still don't have a description, get whatever was in the XML interface
            description = prop->getPropDeclaration()->getDescription();
        }
    }
    description.Replace("\\n", "\n", true);
    return description;
}
