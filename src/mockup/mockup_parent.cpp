/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top-level MockUp Parent window
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    * MockupParent is the top-level panel which maintains a border around the virtual "form" window as well as providing
    * scrollbars if needed. It has a dark background to make it easier to see the virtual "form" and is why we we use a
    * different wxPanel to host the title bar and content panels (since it will have a standard background).

    * m_MockupWindow is a simple wxPanel that hosts the title bar and container panels.

*/

#if defined(_WIN32)
    #include <wx/msw/uxtheme.h>
#endif  // _WIN32

#include <wx/dcclient.h>  // wxClientDC base header
#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/statbox.h>   // wxStaticBox base header
#include <wx/stattext.h>  // wxStaticText base header
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "mockup_parent.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "bitmaps.h"         // Contains various images handling functions
#include "cstm_event.h"      // CustomEvent -- Custom Event class
#include "mainframe.h"       // MainFrame -- Main window frame
#include "mockup_content.h"  // Contains all the forms objects
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "ui_images.h"

using namespace wxue_img;

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

    Bind(EVT_GridBagAction,
         [this](CustomEvent&)
         {
             CreateContent();
         });
    Bind(EVT_NodeCreated,
         [this](CustomEvent&)
         {
             CreateContent();
         });
    Bind(EVT_NodeDeleted,
         [this](CustomEvent& event)
         {
             OnNodeDeleted(event);
         });
    Bind(EVT_ParentChanged,
         [this](CustomEvent&)
         {
             CreateContent();
         });
    Bind(EVT_PositionChanged,
         [this](CustomEvent&)
         {
             CreateContent();
         });
    Bind(EVT_ProjectUpdated,
         [this](CustomEvent&)
         {
             CreateContent();
         });
    Bind(EVT_MultiPropChange,
         [this](CustomEvent&)
         {
             CreateContent();
         });

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

    m_form = wxGetFrame().getSelectedForm();
    if (!m_form)
    {
        m_MockupWindow->Hide();
        return;
    }

#if defined(_DEBUG)
    if (wxGetApp().isFireCreationMsgs())
    {
        MSG_INFO("Mockup window recreated.");
    }
#endif  // _DEBUG

    wxWindowUpdateLocker freeze(this);

    // Note that we show the form even if it's property has it set to hidden
    m_MockupWindow->Show();

    if (m_form->hasValue(prop_background_colour))
    {
        m_panelContent->SetBackgroundColour(m_form->as_wxColour(prop_background_colour));
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

    if (m_form->isGen(gen_wxFrame) || m_form->isGen(gen_wxDialog) || m_form->isGen(gen_wxWizard) ||
        m_form->isGen(gen_wxPropertySheetDialog))
    {
        m_text_title->SetLabel(m_form->as_wxString(prop_title));
        m_panelTitleBar->Show();
    }
    else
    {
        m_panelTitleBar->Hide();
    }

    auto maxSize = m_form->as_wxSize(prop_maximum_size);
    m_MockupWindow->SetMaxSize(maxSize);

    m_panelContent->CreateAllGenerators();
    m_AreNodesCreated = true;

    auto min_size = m_form->as_wxSize(prop_minimum_size);
    min_size.IncTo(m_panelContent->GetSize());

    if (m_form->hasValue(prop_size))
    {
        min_size.IncTo(m_form->as_wxSize(prop_size));
    }
    if (m_form->hasValue(prop_mockup_size))
    {
        min_size.IncTo(m_form->as_wxSize(prop_mockup_size));
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

    // If there are no controls, or the controls are all hidden, the width will be 0 which
    // results in no UI being displayed at all. If this is the case and there is a title, use
    // the width of the title as the minimum width. Otherwise, use a default of 32.
    if (min_size.x == 0)
    {
        if (m_form->hasValue(prop_title))
        {
            wxClientDC dc(m_MockupWindow);
            auto text_size = dc.GetTextExtent(m_form->as_string(prop_title));
            min_size.x = text_size.x;
        }
        else
        {
            min_size.x = 32;
        }
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

void MockupParent::OnNodeDeleted(CustomEvent& /* event */)
{
    // When we get the deleted event, the node being deleted is still selected, which can cause
    // a crash if we try to process it. After the node is deleted, a new node will be selected
    // (which might be a different form entirely), so we delete everything now and hide the
    // window. Once a new node is selected, CreateContent() will be called which will recreate
    // everything and show the window again.
    if (m_AreNodesCreated)
    {
        m_panelContent->RemoveNodes();
        m_AreNodesCreated = false;
        // This ensures that the we regenerate all content when a new node is selected. See
        // the check for wxGetFrame().getSelectedForm() != m_form in OnNodeSelected().
        m_form = nullptr;
    }

    if (!IsShown())
        return;

    m_MockupWindow->Hide();
}

void MockupParent::OnNodeSelected(CustomEvent& event)
{
    if (!IsShown())
        return;

    if (wxGetFrame().getSelectedForm() != m_form)
        m_isIgnoreSelection = false;

    if (m_isIgnoreSelection)
    {
        m_isIgnoreSelection = false;
        return;
    }

    if (wxGetFrame().getSelectedForm() != m_form)
    {
        CreateContent();
    }

    m_panelContent->OnNodeSelected(event.getNode());
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
    if (m_isPropertyChanging)
        return;

    // Setting this to true will ignore the next selection event, and clear the flag
    m_isIgnoreSelection = true;

    // This will select the node in the Navigation pane which will send us a selection event
    m_panelContent->SelectNode(wxobject);
}

///////////////////////////////////////////// Generator functions ////////////////////////////////////////////////

// These are functions the component windows need access to

Node* MockupParent::getNode(wxObject* wxobject)
{
    return m_panelContent->getNode(wxobject);
}

wxObject* MockupParent::Get_wxObject(Node* node)
{
    return m_panelContent->Get_wxObject(node);
}

wxObject* MockupParent::getChild(wxObject* wxobject, size_t childIndex)
{
    if (auto node = getNode(wxobject); node)
    {
        if (childIndex >= node->getChildCount())
        {
            return nullptr;
        }
        return Get_wxObject(node->getChild(childIndex));
    }

    FAIL_MSG("wxobject not found!");

    return nullptr;
}

wxObject* MockupParent::GetParentNode(wxObject* wxobject)
{
    ASSERT(wxobject);

    if (auto node = getNode(wxobject); node)
        return Get_wxObject(node->getParent());

    FAIL_MSG("wxobject not found!");

    return nullptr;
}

// clang-format off

// These properties do not affect the component's display in the Mockup window, so changes to them are ignored.

static const PropName NonUiProps[] = {

    prop_base_file,
    prop_checkbox_var_name,
    prop_class_access,
    prop_colour,
    prop_context_help,
    prop_defaultfilter,
    prop_defaultfolder,
    prop_subclass,
    prop_derived_class_name,
    prop_derived_file,
    prop_subclass_header,
    prop_filter,
    prop_get_function,
    prop_hover_color,
    prop_id,
    prop_initial,
    prop_max,
    prop_max_point_size,
    prop_maxlength,
    prop_message,
    prop_min,
    prop_pagesize,
    prop_persist,
    prop_radiobtn_var_name,
    prop_range,
    // prop_select,     // wxSimpleHtmlListBox won't redraw correctly if this or prop_selection is enabled.
    // prop_selection,
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
    prop_visited_color,
    prop_window_name,

};

// clang-format on

void MockupParent::OnNodePropModified(CustomEvent& event)
{
    if (!IsShown())
        return;

    auto prop = event.GetNodeProperty();
    if (prop->isProp(prop_tooltip))
    {
        if (auto node = wxGetFrame().getSelectedNode(); node)
        {
            if (node->isStaticBoxSizer())
            {
                node->getGenerator()->OnPropertyChange(Get_wxObject(node), node, prop);
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
        if (auto node = wxGetFrame().getSelectedNode(); node)
        {
            node->getGenerator()->OnPropertyChange(Get_wxObject(node), node, prop);
        }
        return;
    }

    for (auto iter: NonUiProps)
    {
        if (prop->isProp(iter))
        {
            if (prop->isProp(prop_message) && prop->getNode()->isGen(gen_wxBannerWindow))
                break;  // In this case, Mockup does need to be redrawn
            else if (prop->isProp(prop_id) && prop->getNode()->isGen(gen_wxButton))
                break;  // In this case, Mockup does need to be redrawn since label could have changed
            else
                return;
        }
    }

    // Some properties can be changed after the widget is created. We call the generator to update the widget, and if returns
    // true then we resize and repaint the entire Mockup window. There are cases where the resize isn't necessary, but since
    // the updating happens in a Freeze/Thaw section, there shouldn't be any noticeable effect to the user with a resize that
    // doesn't actually change the size.

    bool is_updated = false;

    if (auto node = wxGetFrame().getSelectedNode(); node)
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
            else if (node->isStaticBoxSizer())
                wxStaticCast(window, wxStaticBoxSizer)->GetStaticBox()->Enable(!prop->as_bool());
            else
                wxStaticCast(window, wxWindow)->Enable(!prop->as_bool());
            return;
        }

        auto generator = node->getGenerator();
        if (generator && generator->OnPropertyChange(Get_wxObject(node), node, prop))
        {
            wxWindowUpdateLocker freeze(this);
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
            new_size.IncTo(m_form->as_wxSize(prop_size));
            new_size.IncTo(m_form->as_wxSize(prop_minimum_size));

            new_size.DecToIfSpecified(m_form->as_wxSize(prop_maximum_size));

            m_MockupWindow->SetSize(new_size);
            m_MockupWindow->Refresh();
            is_updated = true;
        }
    }

    if (!is_updated)
    {
        // We set m_isPropertyChanging so that we ignore generators calling our SelectNode() because a page changed
        m_isPropertyChanging = true;
        CreateContent();
        m_panelContent->OnNodeSelected(wxGetFrame().getSelectedNode());
        m_isPropertyChanging = false;
    }
}
