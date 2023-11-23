/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSplitterWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/splitter.h>  // Base header for wxSplitterWindow

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_split_win.h"

// Used by the Mockup window to change the sash position.
class wxCustomSplitterWindow : public wxSplitterWindow
{
public:
    wxCustomSplitterWindow(wxWindow* parent, wxWindowID id, const wxPoint& point = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize, long style = wxSP_3D) :
        wxSplitterWindow(parent, id, point, size, style)
    {
    }

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
};

wxObject* SplitterWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto splitter = new wxCustomSplitterWindow(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                               DlgSize(parent, node, prop_size), (GetStyleInt(node)) & ~wxSP_PERMIT_UNSPLIT);

    if (node->hasValue(prop_sashgravity))
    {
        auto gravity = node->as_double(prop_sashgravity);
        gravity = (gravity < 0.0 ? 0.0 : gravity);
        gravity = (gravity > 1.0 ? 1.0 : gravity);
        splitter->SetSashGravity(gravity);
    }
    if (node->hasValue(prop_min_pane_size))
    {
        int minPaneSize = node->as_int(prop_min_pane_size);
        splitter->SetCustomMinPaneSize(minPaneSize);
        minPaneSize = (minPaneSize < 1 ? 1 : minPaneSize);
        splitter->SetMinimumPaneSize(minPaneSize);
    }

    // Always have a child so it is drawn consistently
    splitter->Initialize(new wxPanel(splitter));

    return splitter;
}

void SplitterWindowGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool is_preview)
{
    auto splitter = wxStaticCast(wxobject, wxCustomSplitterWindow);
    if (!splitter)
    {
        FAIL_MSG("This should be a wxSplitterWindow");
        return;
    }

    // Remove default panel
    auto firstChild = splitter->GetWindow1();

    size_t childCount = node->getChildCount();
    switch (childCount)
    {
        case 1:
            {
                // BUGBUG: [Randalphwa - 06-12-2022] Don't use getMockup() if is_preview is true!
                wxWindow* subwindow;
                if (!is_preview)
                    subwindow = wxDynamicCast(getMockup()->getChild(wxobject, 0), wxWindow);
                else
                    subwindow = wxDynamicCast(node->getChild(0)->getMockupObject(), wxWindow);
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
                break;
            }

        case 2:
            {
                wxWindow *subwindow0, *subwindow1;

                if (!is_preview)
                {
                    subwindow0 = wxDynamicCast(getMockup()->getChild(wxobject, 0), wxWindow);
                    subwindow1 = wxDynamicCast(getMockup()->getChild(wxobject, 1), wxWindow);
                }
                else
                {
                    subwindow0 = wxDynamicCast(node->getChild(0)->getMockupObject(), wxWindow);
                    subwindow1 = wxDynamicCast(node->getChild(1)->getMockupObject(), wxWindow);
                }

                if (!subwindow0 || !subwindow1)
                {
                    FAIL_MSG("Child of splitter is not derived from wxWindow class.");
                    return;
                }

                // Get the split mode and sash position
                node = getMockup()->getNode(wxobject);
                if (!node)
                {
                    // REVIEW: [KeyWorks - 12-06-2020] If this is actually possible, we should let the user know
                    return;
                }

                int sashPos = node->as_int(prop_sashpos);
                int splitmode = node->as_int(prop_splitmode);

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

bool SplitterWindowGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags();

    return true;
}

bool SplitterWindowGenerator::SettingsCode(Code& code)
{
    Node* node = code.node();

    if (node->hasValue(prop_sashgravity) && node->as_string(prop_sashgravity) != "0")
    {
        code.Eol(eol_if_empty).NodeName().Function("SetSashGravity(").Add(prop_sashgravity).EndFunction();
    }

    if (node->hasValue(prop_sashsize) && node->as_string(prop_sashsize) != "-1")
    {
        code.Eol(eol_if_empty).NodeName().Function("SetSashSize(").Add(prop_sashsize).EndFunction();
    }

    if (node->hasValue(prop_min_pane_size) && node->as_string(prop_min_pane_size) != "0")
    {
        code.Eol(eol_if_empty).NodeName().Function("SetMinimumPaneSize(").Add(prop_min_pane_size).EndFunction();
    }

    return true;
}

bool SplitterWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                          int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/splitter.h>", set_src, set_hdr);
    if (node->hasValue(prop_persist_name))
    {
        set_src.insert("#include <wx/persist/splitter.h>");
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_split.cpp
// ../../../wxWidgets/src/xrc/xh_split.cpp

int SplitterWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSplitterWindow");

    if (node->as_int(prop_sashpos) >= 0)
        item.append_child("sashpos").text().set(node->as_string(prop_sashpos));
    if (node->as_int(prop_min_pane_size) >= 0)
        item.append_child("minsize").text().set(node->as_string(prop_min_pane_size));
    ADD_ITEM_PROP(prop_sashgravity, "gravity")
    item.append_child("orientation")
        .text()
        .set(node->as_string(prop_splitmode) == "wxSPLIT_HORIZONTAL" ? "horizontal" : "vertical");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_int(prop_sashsize) >= 0)
            ADD_ITEM_COMMENT(" XRC does not support calling SetSashSize() ")

        GenXrcComments(node, item);
    }

    return result;
}

void SplitterWindowGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSplitterWindowXmlHandler");
}
