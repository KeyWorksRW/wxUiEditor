/////////////////////////////////////////////////////////////////////////////
// Purpose:   Splitter and Scroll window component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>     // Event classes
#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper
#include <wx/splitter.h>  // Base header for wxSplitterWindow

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "window_widgets.h"

// Used by the Mockup window to change the sash position.
class wxCustomSplitterWindow : public wxSplitterWindow
{
public:
    wxCustomSplitterWindow(wxWindow* parent, wxWindowID id, const wxPoint& point = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize, long style = wxSP_3D) :
        wxSplitterWindow(parent, id, point, size, style)
    {
    }

    // REVIEW: [KeyWorks - 12-19-2020]  I removed this because this would cause a redraw on every idle event.

#if 0
    // Used to ensure sash position is correct
    void OnIdle(wxIdleEvent&)
    {
        Disconnect(wxEVT_IDLE, wxIdleEventHandler(wxCustomSplitterWindow::OnIdle));

        // So the selection of the sizer at its initial position is cleared, then shown at the correct position
        Freeze();
        SetSashPosition(m_initialSashPos);
        Layout();
        Refresh();
        Update();
        Thaw();
    }
#endif

    void SetCustomMinPaneSize(int pane_size) { m_customMinPaneSize = pane_size; }

protected:
    bool OnSashPositionChange(int newSashPosition) override
    {
        m_customSashPos = newSashPosition;
        return wxSplitterWindow::OnSashPositionChange(newSashPosition);
    }

private:
    int m_customSashPos { 0 };
    int m_customMinPaneSize { 0 };
    // int m_initialSashPos;
};

//////////////////////////////////////////  SplitterWindowGenerator  //////////////////////////////////////////

wxObject* SplitterWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto splitter = new wxCustomSplitterWindow(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                               DlgSize(parent, node, prop_size), (GetStyleInt(node)) & ~wxSP_PERMIT_UNSPLIT);

    if (node->HasValue(prop_sashgravity))
    {
        auto gravity = node->prop_as_double(prop_sashgravity);
        gravity = (gravity < 0.0 ? 0.0 : gravity);
        gravity = (gravity > 1.0 ? 1.0 : gravity);
        splitter->SetSashGravity(gravity);
    }
    if (node->HasValue(prop_min_pane_size))
    {
        int minPaneSize = node->prop_as_int(prop_min_pane_size);
        splitter->SetCustomMinPaneSize(minPaneSize);
        minPaneSize = (minPaneSize < 1 ? 1 : minPaneSize);
        splitter->SetMinimumPaneSize(minPaneSize);
    }

    // Always have a child so it is drawn consistently
    splitter->Initialize(new wxPanel(splitter));

    // REVIEW: [KeyWorks - 12-19-2020] I removed this because this would cause a redraw on every idle event.

    // Used to ensure sash position is correct
    // splitter->m_initialSashPos = node->prop_as_int("sashpos");
    // splitter->Connect(wxEVT_IDLE, wxIdleEventHandler(wxCustomSplitterWindow::OnIdle));

    return splitter;
}

void SplitterWindowGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/)
{
    auto splitter = wxStaticCast(wxobject, wxCustomSplitterWindow);
    if (!splitter)
    {
        FAIL_MSG("This should be a wxSplitterWindow");
        return;
    }

    // Remove default panel
    auto firstChild = splitter->GetWindow1();

    auto node = GetMockup()->GetNode(wxobject);
    size_t childCount = node->GetChildCount();
    switch (childCount)
    {
        case 1:
            {
                auto subwindow = wxDynamicCast(GetMockup()->GetChild(wxobject, 0), wxWindow);
                if (!subwindow)
                {
                    FAIL_MSG("Child of splitter is not derived from wxWindow class.");
                    return;
                }

                if (firstChild)
                {
                    splitter->ReplaceWindow(firstChild, subwindow);
                    firstChild->Destroy();
                }
                else
                {
                    splitter->Initialize(subwindow);
                }
                // splitter->PushEventHandler(new ContainerBarEvtHandler(splitter));
                break;
            }
        case 2:
            {
                auto subwindow0 = wxDynamicCast(GetMockup()->GetChild(wxobject, 0), wxWindow);
                auto subwindow1 = wxDynamicCast(GetMockup()->GetChild(wxobject, 1), wxWindow);

                if (!subwindow0 || !subwindow1)
                {
                    FAIL_MSG("Child of splitter is not derived from wxWindow class.");
                    return;
                }

                // Get the split mode and sash position
                node = GetMockup()->GetNode(wxobject);
                if (!node)
                {
                    // REVIEW: [KeyWorks - 12-06-2020] If this is actually possible, we should let the user know
                    return;
                }

                int sashPos = node->prop_as_int(prop_sashpos);
                int splitmode = node->prop_as_int(prop_splitmode);

                if (firstChild)
                {
                    splitter->ReplaceWindow(firstChild, subwindow0);
                    firstChild->Destroy();
                }

                if (splitmode == wxSPLIT_VERTICAL)
                {
                    splitter->SplitVertically(subwindow0, subwindow1, sashPos);
                }
                else
                {
                    splitter->SplitHorizontally(subwindow0, subwindow1, sashPos);
                }

                // splitter->PushEventHandler(new ContainerBarEvtHandler(splitter));
                break;
            }
        default:
            return;
    }
}

std::optional<ttlib::cstr> SplitterWindowGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> SplitterWindowGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue(prop_sashgravity) && node->prop_as_string(prop_sashgravity) != "0")
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->SetSashGravity(" << node->prop_as_string(prop_sashgravity) << ");";
    }

    if (node->HasValue(prop_sashsize) && node->prop_as_string(prop_sashsize) != "-1")
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->SetSashSize(" << node->prop_as_string(prop_sashsize) << ");";
    }

    if (node->HasValue(prop_min_pane_size) && node->prop_as_string(prop_min_pane_size) != "0")
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->SetMinimumPaneSize(" << node->prop_as_string(prop_min_pane_size) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> SplitterWindowGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool SplitterWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/splitter.h>", set_src, set_hdr);
    if (node->HasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/splitter.h>");
    }

    return true;
}

//////////////////////////////////////////  ScrolledWindowGenerator  //////////////////////////////////////////

wxObject* ScrolledWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxScrolled<wxPanel>(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                          DlgSize(parent, node, prop_size), GetStyleInt(node));
    widget->SetScrollRate(node->prop_as_int(prop_scroll_rate_x), node->prop_as_int(prop_scroll_rate_y));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ScrolledWindowGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxScrolled<wxPanel>(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> ScrolledWindowGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue(prop_scroll_rate_x) || node->HasValue(prop_scroll_rate_y))
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->SetScrollRate(" << node->prop_as_string(prop_scroll_rate_x) << ", "
             << node->prop_as_string(prop_scroll_rate_y) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> ScrolledWindowGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ScrolledWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/scrolwin.h>", set_src, set_hdr);

    return true;
}
