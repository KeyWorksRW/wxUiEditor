/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top-level MockUp Parent window
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    * MockupParent is the top-level panel which maintains a border around the virtual "form" window as well as providing
    * scrollbars if needed. It has a dark background to make it easier to see the virtual "form" and is why we we use a
    * different wxPanel to host the title bar and content panels (since it will have a standard background).

    * m_MockupWindow is a simple wxPanel that hosts the title bar and container panels.

*/

#include "pch.h"

#if defined(_WIN32)
    #include <wx/msw/uxtheme.h>
#endif  // _WIN32

#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/statbox.h>   // wxStaticBox base header
#include <wx/stattext.h>  // wxStaticText base header

#include "mockup_parent.h"

#include "auto_freeze.h"     // AutoFreeze -- Automatically Freeze/Thaw a window
#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "cstm_event.h"      // CustomEvent -- Custom Event class
#include "mainframe.h"       // MainFrame -- Main window frame
#include "mockup_content.h"  // Contains all the forms objects
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "../art_headers/title_close_png.hxx"

MockupParent::MockupParent(wxWindow* parent, MainFrame* frame) : wxScrolled<wxPanel>(parent)
{
    // Don't allow events to propagate any higher than this window.
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

    // Make the background around the window darker to enhance the contrast with the form
    SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE).ChangeLightness(100));

    auto mockup_sizer = new wxBoxSizer(wxVERTICAL);
    auto form_sizer = new wxBoxSizer(wxVERTICAL);

    m_MockupWindow = new wxPanel(this);

    m_panelTitleBar = new wxPanel(m_MockupWindow);
    m_panelTitleBar->SetMinSize(wxSize(46, 26));
    m_panelTitleBar->SetBackgroundColour(wxColour(127, 188, 248));

    auto title_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_text_title = new wxStaticText(m_panelTitleBar, wxID_ANY, wxEmptyString);
    title_sizer->Add(m_text_title, wxSizerFlags(1).Center().Border());
    auto bmp =
        new wxStaticBitmap(m_panelTitleBar, wxID_ANY, wxBitmap(LoadHeaderImage(title_close_png, sizeof(title_close_png))));
    title_sizer->Add(bmp, wxSizerFlags());

    m_panelTitleBar->SetSizerAndFit(title_sizer);

    m_panelContent = new MockupContent(m_MockupWindow, this);

    form_sizer->Add(m_panelTitleBar, wxSizerFlags().Expand());
    form_sizer->Add(m_panelContent, wxSizerFlags(1).Expand());

    m_MockupWindow->Hide();

    m_MockupWindow->SetSizer(form_sizer);
    m_MockupWindow->Layout();
    mockup_sizer->Add(m_MockupWindow, wxSizerFlags().Border(wxALL, wxSizerFlags::GetDefaultBorder()));

    SetSizerAndFit(mockup_sizer);

    // We need this in order to get optional scrollbars in either direction
    SetScrollRate(5, 5);

    Bind(EVT_NodeSelected, &MockupParent::OnNodeSelected, this);
    Bind(EVT_NodePropChange, &MockupParent::OnNodePropModified, this);

    Bind(EVT_NodeCreated, [this](CustomEvent&) { CreateContent(); });
    Bind(EVT_NodeDeleted, [this](CustomEvent&) { CreateContent(); });
    Bind(EVT_ParentChanged, [this](CustomEvent&) { CreateContent(); });
    Bind(EVT_PositionChanged, [this](CustomEvent&) { CreateContent(); });
    Bind(EVT_ProjectUpdated, [this](CustomEvent&) { CreateContent(); });

    frame->AddCustomEventHandler(GetEventHandler());
}

// This gets called when a different form is selected, a different project loaded, controls added and/or deleted, etc.
void MockupParent::CreateContent()
{
    // Just in case this gets called when we aren't being shown, only clear the panel if we haven't cleared it already.
    if (m_AreNodesCreated)
    {
        m_panelContent->RemoveNodes();
        m_AreNodesCreated = false;
    }

    if (!IsShown())
        return;

    m_form = wxGetFrame().GetSelectedForm();
    if (!m_form)
    {
        m_MockupWindow->Hide();
        return;
    }

    // Uncomment this to check whether the Mockup window is being created multiple times for a single action, or it's being recreated
    // by a property change that doesn't need the Mockup to be recreated.

    MSG_INFO("Mockup window recreated.");

    AutoFreeze freeze(this);

    // Note that we show the form even if it's property has it set to hidden
    m_MockupWindow->Show();

    if (m_form->HasValue(prop_background_colour))
    {
        m_panelContent->SetBackgroundColour(m_form->prop_as_wxColour(prop_background_colour));
    }
    else if (m_form->isGen(gen_wxFrame))
    {
        m_panelContent->SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
    }
    else if (m_form->isGen(gen_wxPopupTransientWindow))
    {
        m_panelContent->SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    }
    else
    {
#ifdef __WXGTK__
        wxVisualAttributes attribs = wxToolBar::GetClassDefaultAttributes();
        m_panelContent->SetOwnBackgroundColour(attribs.colBg);
#else
        m_panelContent->SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    }

    if (m_form->isGen(gen_wxFrame) || m_form->isGen(gen_wxDialog) || m_form->isGen(gen_wxWizard))
    {
        m_text_title->SetLabel(m_form->prop_as_wxString(prop_title));
        m_panelTitleBar->Show();
    }
    else
    {
        m_panelTitleBar->Hide();
    }

    auto maxSize = m_form->prop_as_wxSize(prop_maximum_size);
    m_MockupWindow->SetMaxSize(maxSize);

    m_panelContent->CreateAllGenerators();
    m_AreNodesCreated = true;

    auto min_size = m_form->prop_as_wxSize(prop_minimum_size);
    min_size.IncTo(m_panelContent->GetSize());

    if (m_form->HasValue(prop_size))
    {
        min_size.IncTo(m_form->prop_as_wxSize(prop_size));
    }
    if (m_form->HasValue(prop_mockup_size))
    {
        min_size.IncTo(m_form->prop_as_wxSize(prop_mockup_size));
    }

    if (m_panelTitleBar->IsShown())
    {
        // The title bar should be no wider than the content window.
        auto size = m_panelTitleBar->GetSize();
        size.SetWidth(min_size.GetWidth());
        m_panelTitleBar->SetSize(size);
        // Until Fit() is called, the height won't be correct.
        m_panelTitleBar->Fit();
        size = m_panelTitleBar->GetSize();
        min_size.y += size.GetHeight();
    }

    m_MockupWindow->SetMinSize(min_size);
    Layout();

    if (m_form->isPropValue(prop_disabled, true))
    {
        m_MockupWindow->Enable(false);
    }
    else
        m_MockupWindow->Enable();
}

void MockupParent::OnNodeSelected(CustomEvent& event)
{
    if (m_isIgnoreSelection)
    {
        m_isIgnoreSelection = false;
        return;
    }

    if (!IsShown())
        return;

    if (wxGetFrame().GetSelectedForm() != m_form)
    {
        CreateContent();
    }

    m_panelContent->OnNodeSelected(event.GetNode());
}

void MockupParent::ShowHiddenControls(bool show)
{
    m_ShowHiddenControls = show;
    CreateContent();
}

void MockupParent::MagnifyWindow(bool show)
{
    m_IsMagnifyWindow = show;
    CreateContent();
}

void MockupParent::SelectNode(wxObject* wxobject)
{
    // Setting this to true will ignore the next selection event, and clear the flag
    m_isIgnoreSelection = true;

    // This will select the node in the Navigation pane which will send us a selection event
    m_panelContent->SelectNode(wxobject);
}

///////////////////////////////////////////// Generator functions ////////////////////////////////////////////////

// These are functions the component windows need access to

Node* MockupParent::GetNode(wxObject* wxobject)
{
    return m_panelContent->GetNode(wxobject);
}

wxObject* MockupParent::Get_wxObject(Node* node)
{
    return m_panelContent->Get_wxObject(node);
}

wxObject* MockupParent::GetChild(wxObject* wxobject, size_t childIndex)
{
    if (auto node = GetNode(wxobject); node)
    {
        if (childIndex >= node->GetChildCount())
        {
            return nullptr;
        }
        return Get_wxObject(node->GetChild(childIndex));
    }

    FAIL_MSG("wxobject not found!");

    return nullptr;
}

wxObject* MockupParent::GetParentNode(wxObject* wxobject)
{
    ASSERT(wxobject);

    if (auto node = GetNode(wxobject); node)
        return Get_wxObject(node->GetParent());

    FAIL_MSG("wxobject not found!");

    return nullptr;
}

// clang-format off

// These properties do not affect the component's display in the Mockup window, so changes to them are ignored.

static const PropName NonUiProps[] = {

    prop_base_class_name,
    prop_base_file,
    prop_class_access,
    prop_colour,
    prop_context_help,
    prop_defaultfilter,
    prop_defaultfolder,
    prop_derived_class_name,
    prop_derived_file,
    prop_digits,
    prop_filter,
    prop_get_function,
    prop_hover_color,
    prop_id,
    prop_inc,
    prop_initial,
    prop_max,
    prop_max_point_size,
    prop_maxlength,
    prop_message,
    prop_min,
    prop_normal_color,
    prop_pagesize,
    prop_persist,
    prop_range,
    prop_select,
    prop_selection,
    prop_set_function,
    prop_show_hidden,
    prop_thumbsize,
    prop_tooltip,
    prop_url,
    prop_validator_data_type,
    prop_validator_style,
    prop_validator_type,
    prop_validator_variable,
    prop_var_name,
    prop_radiobtn_var_name,
    prop_checkbox_var_name,
    prop_visited_color,
    prop_wildcard,
    prop_window_name,

};

// clang-format on

void MockupParent::OnNodePropModified(CustomEvent& event)
{
    auto prop = event.GetNodeProperty();

    if (prop->isProp(prop_tooltip))
    {
        if (auto node = wxGetFrame().GetSelectedNode(); node)
        {
            if (node->IsStaticBoxSizer())
            {
                node->GetGenerator()->OnPropertyChange(Get_wxObject(node), node, prop);
            }
            else if (auto window = wxDynamicCast(Get_wxObject(node), wxWindow); window)
            {
                window->SetToolTip(prop->as_wxString());
            }
        }
        return;
    }

    if (prop->isProp(prop_initial) || prop->isProp(prop_min) || prop->isProp(prop_max))
    {
        if (auto node = wxGetFrame().GetSelectedNode(); node)
        {
            node->GetGenerator()->OnPropertyChange(Get_wxObject(node), node, prop);
        }
        return;
    }

    for (auto iter: NonUiProps)
    {
        if (prop->isProp(iter))
            return;
    }

    // Some properties can be changed after the widget is created. We call the component to update the widget, and if returns
    // true then we resize and repaint the entire Mockup window. There are cases where the resize isn't necessary, but since
    // the updating happens in a Freeze/Thaw section, there shouldn't be any noticeable effect to the user with a resize that
    // doesn't actually change the size.

    bool is_updated = false;

    if (auto node = wxGetFrame().GetSelectedNode(); node)
    {
        if (prop->isProp(prop_disabled))
        {
            auto window = Get_wxObject(node);
            if (!window)
            {
                // For some content such as FormPanel, the selected node doesn't have a window that can be enabled/disabled
                CreateContent();
                return;
            }
            else if (node->IsStaticBoxSizer())
                wxStaticCast(window, wxStaticBoxSizer)->GetStaticBox()->Enable(!prop->as_bool());
            else
                wxStaticCast(window, wxWindow)->Enable(!prop->as_bool());
            return;
        }

        auto comp = node->GetGenerator();
        if (comp && comp->OnPropertyChange(Get_wxObject(node), node, prop))
        {
            AutoFreeze freeze(this);
            // You have to reset minimum size to allow the window to shrink
            m_panelContent->SetMinSize(wxSize(-1, -1));
            m_panelContent->Fit();

            auto new_size = m_panelContent->GetSize();
            if (m_panelTitleBar)
            {
                auto size_title = m_panelTitleBar->GetSize();
                new_size.y += size_title.y;
            }

            if (m_IsMagnifyWindow &&
                !(m_form->isGen(gen_RibbonBar) || m_form->isGen(gen_ToolBar) || m_form->isGen(gen_MenuBar)))
            {
                new_size.IncTo(m_size_magnified);
            }

            // Need to be at least as large as any dimensions the user set.
            new_size.IncTo(m_form->prop_as_wxSize(prop_size));
            new_size.IncTo(m_form->prop_as_wxSize(prop_minimum_size));

            new_size.DecToIfSpecified(m_form->prop_as_wxSize(prop_maximum_size));

            m_MockupWindow->SetSize(new_size);
            m_MockupWindow->Refresh();
            is_updated = true;
        }
    }

    if (!is_updated)
    {
        CreateContent();
        m_panelContent->OnNodeSelected(wxGetFrame().GetSelectedNode());
    }
}
