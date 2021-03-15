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
#include <wx/stattext.h>  // wxStaticText base header

#include "mockup_parent.h"

#include "auto_freeze.h"     // AutoFreeze -- Automatically Freeze/Thaw a window
#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "cstm_event.h"      // CustomEvent -- Custom Event class
#include "mainframe.h"       // MainFrame -- Main window frame
#include "mockup_content.h"  // Contains all the forms objects
#include "node.h"            // Node class
#include "utils.h"           // Utility functions that work with properties

#include "../xpm/title_close.xpm"

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
    title_sizer->Add(m_text_title, wxSizerFlags(1).Center().Border(wxALL, 5));
    auto bmp = new wxStaticBitmap(m_panelTitleBar, wxID_ANY, wxBitmap(wxImage(title_close_xpm)));
    title_sizer->Add(bmp, wxSizerFlags().Border(0, 0));

    m_panelTitleBar->SetSizerAndFit(title_sizer);

    m_panelContent = new MockupContent(m_MockupWindow, this);

    form_sizer->Add(m_panelTitleBar, wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT | wxTOP, 0));
    form_sizer->Add(m_panelContent, wxSizerFlags(1).Expand().Border(wxALL, 0));

    m_MockupWindow->Hide();

    m_MockupWindow->SetSizer(form_sizer);
    // m_MockupWindow->SetAutoLayout(true);
    m_MockupWindow->Layout();

    mockup_sizer->Add(m_MockupWindow, wxSizerFlags().Border(wxALL, wxSizerFlags::GetDefaultBorder()));

    SetSizerAndFit(mockup_sizer);

    // We need this in order to get optional scrollbars in either direction
    SetScrollRate(5, 5);

    Bind(EVT_NodeSelected, &MockupParent::OnNodeSelected, this);
    Bind(EVT_NodePropChange, &MockupParent::OnNodePropModified, this);

    Bind(EVT_ProjectUpdated, [this](CustomEvent&) { CreateContent(); });
    Bind(EVT_NodeCreated, [this](CustomEvent&) { CreateContent(); });
    Bind(EVT_NodeDeleted, [this](CustomEvent&) { CreateContent(); });

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

    m_AreNodesCreated = true;  // Set this now to ensure content gets cleared if CreateContent() is called again

    AutoFreeze freeze(this);

    // Note that we show the form even if it's property has it set to hidden
    m_MockupWindow->Show();

    if (auto background = m_form->get_prop_ptr("background_colour"); background && background->GetValue().size())
    {
        m_panelContent->SetBackgroundColour(ConvertToColour(background->GetValue()));
    }
    else if (m_form->GetClassName() == "wxFrame")
    {
        m_panelContent->SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
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

    if (m_form->GetClassName() == "wxFrame" || m_form->GetClassName() == "wxDialog" || m_form->GetClassName() == "wxWizard")
    {
        m_text_title->SetLabel(m_form->prop_as_wxString("title"));
        m_panelTitleBar->Show();
    }
    else
    {
        m_panelTitleBar->Hide();
    }

    auto minSize = m_form->prop_as_wxSize(txt_minimum_size);
    m_MockupWindow->SetMinSize(minSize);

    auto maxSize = m_form->prop_as_wxSize("maximum_size");
    m_MockupWindow->SetMaxSize(maxSize);

    if (m_form->GetClassName() == "wxWizard")
        m_panelContent->CreateWizard();

    m_panelContent->CreateAllGenerators();

    auto org_size = m_form->prop_as_wxSize("org_size");
    if (m_IsMagnifyWindow && !(m_form->GetClassName() == "ToolBar" || m_form->GetClassName() == "MenuBar"))
    {
        org_size.IncTo(m_size_magnified);
    }

    wxSize size_mockup = org_size;
    if (size_mockup.GetWidth() < minSize.GetWidth() && size_mockup.GetWidth() != wxDefaultCoord)
    {
        size_mockup.SetWidth(minSize.GetWidth());
    }
    if (size_mockup.GetHeight() < minSize.GetHeight() && size_mockup.GetHeight() != wxDefaultCoord)
    {
        size_mockup.SetHeight(minSize.GetHeight());
    }
    if (size_mockup.GetWidth() > maxSize.GetWidth() && maxSize.GetWidth() != wxDefaultCoord)
    {
        size_mockup.SetWidth(maxSize.GetWidth());
    }
    if (size_mockup.GetHeight() > maxSize.GetHeight() && maxSize.GetHeight() != wxDefaultCoord)
    {
        size_mockup.SetHeight(maxSize.GetHeight());
    }

    if (org_size != size_mockup)
    {
        // TODO: [KeyWorks - 07-16-2020] We need some way to let the user know about this -- best would be when
        // the form's size property is modified.

        MSG_WARNING("Invalid size -- it is not between minimum_size and maximum_size");
    }

    Layout();
    // Set size after fitting so if only one dimesion is -1, it still fits that dimension
    m_MockupWindow->SetSize(size_mockup);

    // Enable and Hidden state may have changed, so update state accordingly

    if (auto disabled = m_form->prop_as_bool("disabled"); disabled)
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

    if (m_form->GetClassName() == "ToolBar" || m_form->GetClassName() == "MenuBar")
        return;

    auto cur_size = m_MockupWindow->GetSize();
    if (m_IsMagnifyWindow && cur_size.y >= m_size_magnified.y && cur_size.x >= m_size_magnified.x)
        return;

    Freeze();

    // You have to reset the minimum size to allow the window to shrink
    m_panelContent->SetMinSize(wxSize(-1, -1));
    m_panelContent->Fit();

    auto new_size = m_panelContent->GetSize();
    if (m_panelTitleBar)
    {
        auto size_title = m_panelTitleBar->GetSize();
        new_size.y += size_title.y;
    }

    if (m_IsMagnifyWindow)
    {
        new_size.IncTo(m_size_magnified);
    }

    // Need to be at least as large as any dimensions the user set.
    new_size.IncTo(m_form->prop_as_wxSize("size"));
    new_size.IncTo(m_form->prop_as_wxSize(txt_minimum_size));

    new_size.DecToIfSpecified(m_form->prop_as_wxSize("maximum_size"));

    m_MockupWindow->SetSize(new_size);
    m_MockupWindow->Refresh();

    Thaw();
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

static const auto NonUiProps = {

    "base_class_name",
    "base_file",
    "class_access",
    "colour",
    "context_help",
    "defaultfilter",
    "defaultfolder",
    "derived_class_name",
    "derived_file",
    "digits",
    "filter",
    "forward_declare",
    "get_function",
    "header",
    "hover_color",
    "id",
    "inc",
    "initial",
    "max",
    "max_point_size",
    "maxlength",
    "message",
    "min",
    "normal_color",
    "pagesize",
    "period",
    "persist",
    "range",
    "select",
    "selection",
    "set_function",
    "show_hidden",
    "thumbsize",
    "tooltip",
    "url",
    "validator_data_type",
    "validator_style",
    "validator_type",
    "validator_variable",
    "var_name",
    "radiobtn_var_name",
    "checkbox_var_name",
    "visited_color",
    "wildcard",
    "window_name",

};

// clang-format on

void MockupParent::OnNodePropModified(CustomEvent& event)
{
    auto prop = event.GetNodeProperty();
    auto& prop_name = prop->GetPropName();

    if (prop_name == "tooltip")
    {
        if (auto node = wxGetFrame().GetSelectedNode(); node)
        {
            if (auto window = wxStaticCast(Get_wxObject(node), wxWindow); window)
            {
                window->SetToolTip(prop->as_wxString());
            }
        }
        return;
    }

    for (auto& iter: NonUiProps)
    {
        if (prop_name == iter)
            return;
    }

    // Some properties can be changed after the widget is created. We call the component to update the widget, and if returns
    // true then we resize and repaint the entire Mockup window. There are cases where the resize isn't necessary, but since
    // the updating happens in a Freeze/Thaw section, there shouldn't be any noticeable effect to the user with a resize that
    // doesn't actually change the size.

    bool is_updated = false;

    if (auto node = wxGetFrame().GetSelectedNode(); node)
    {
        if (prop_name == "disabled")
        {
            wxStaticCast(Get_wxObject(node), wxControl)->Enable(!prop->as_bool());
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

            if (m_IsMagnifyWindow && !(m_form->GetClassName() == "ToolBar" || m_form->GetClassName() == "MenuBar"))
            {
                new_size.IncTo(m_size_magnified);
            }

            // Need to be at least as large as any dimensions the user set.
            new_size.IncTo(m_form->prop_as_wxSize("size"));
            new_size.IncTo(m_form->prop_as_wxSize(txt_minimum_size));

            new_size.DecToIfSpecified(m_form->prop_as_wxSize("maximum_size"));

            m_MockupWindow->SetSize(new_size);
            m_MockupWindow->Refresh();
            is_updated = true;
        }
    }

    if (!is_updated)
    {
        CreateContent();
    }
}
