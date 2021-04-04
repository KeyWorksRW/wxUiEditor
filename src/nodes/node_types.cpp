/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores node types and allowable child count
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "node_types.h"

#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class

using namespace child_count;

struct ParentChildInfo
{
    const char* parent;
    const char* name;

    int_t max_children;
};

// clang-format off

// A child node can only be created if it is listed below as valid for the current parent.
static const ParentChildInfo lstParentChildren[] = {

    { "auinotebook", "auinotebookpage", infinite },
    { "auinotebookpage", "container", one },

    { "bookpage", "gbsizer", one },
    { "bookpage", "sizer", one },

    { "choicebook", "bookpage", infinite },
    { "choicebook", "widget", infinite },

    { "container", "gbsizer", one },
    { "container", "menu", one },
    { "container", "sizer", one },

    { "dataviewctrl", "dataviewcolumn", infinite },
    { "dataviewlistctrl", "dataviewlistcolumn", infinite },

    { "form", "auinotebook", none },
    { "form", "choicebook", none },
    { "form", "container", none },
    { "form", "dataviewctrl", none },
    { "form", "dataviewlistctrl", none },
    { "form", "dataviewtreectrl", none },
    { "form", "expanded_widget", none },
    { "form", "flatnotebook", none },
    { "form", "listbook", none },
    { "form", "nonvisual", infinite },
    { "form", "notebook", none },
    { "form", "propgrid", none },
    { "form", "propgridman", none },
    { "form", "ribbonbar", none },
    { "form", "simplebook", none },
    { "form", "splitter", none },
    { "form", "treelistctrl", none },
    { "form", "widget", none },

    { "form", "sizer", one },
    { "form", "gbsizer", one },

    { "form", "menu", one },
    { "form", "menubar", one },
    { "form", "statusbar", one },
    { "form", "toolbar", one },

    { "gbsizer", "auinotebook", infinite },
    { "gbsizer", "choicebook", infinite },
    { "gbsizer", "container", infinite },
    { "gbsizer", "dataviewctrl", infinite },
    { "gbsizer", "dataviewlistctrl", infinite },
    { "gbsizer", "dataviewtreectrl", infinite },
    { "gbsizer", "expanded_widget", infinite },
    { "gbsizer", "flatnotebook", infinite },
    { "gbsizer", "gbsizer", infinite },
    { "gbsizer", "listbook", infinite },
    { "gbsizer", "notebook", infinite },
    { "gbsizer", "propgrid", infinite },
    { "gbsizer", "propgridman", infinite },
    { "gbsizer", "ribbonbar", infinite },
    { "gbsizer", "simplebook", infinite },
    { "gbsizer", "sizer", infinite },
    { "gbsizer", "splitter", infinite },
    { "gbsizer", "toolbar", infinite },
    { "gbsizer", "treelistctrl", infinite },
    { "gbsizer", "widget", infinite },

    { "listbook", "bookpage", infinite },

    { "menu", "menuitem", infinite },
    { "menu", "submenu", infinite },
    { "menubar", "menu", infinite },
    { "menubar_form", "menu", infinite },

    { "notebook", "bookpage", infinite },

    { "project", "form", infinite },
    { "project", "menubar_form", infinite },
    { "project", "toolbar_form", infinite },
    { "project", "wizard", infinite },

    { "propgrid", "menu", one },
    { "propgrid", "propgriditem", infinite },
    { "propgriditem", "propgridpage", infinite },
    { "propgridman", "menu", one },
    { "propgridman", "propgridpage", infinite },
    { "propgridpage", "propgriditem", infinite },

    { "ribbonbar", "ribbonpage", infinite },
    { "ribbonbuttonbar", "ribbonbutton", infinite },
    { "ribbongallery", "ribbongalleryitem", infinite },
    { "ribbonpage", "ribbonpanel", infinite },
    { "ribbonpanel", "ribbonbuttonbar", one },
    { "ribbonpanel", "ribbongallery", one },
    { "ribbonpanel", "ribbontoolbar", one },
    { "ribbontoolbar", "ribbontool", infinite },

    { "simplebook", "bookpage", infinite },

    { "sizer", "auinotebook", infinite },
    { "sizer", "choicebook", infinite },
    { "sizer", "container", infinite },
    { "sizer", "dataviewctrl", infinite },
    { "sizer", "dataviewlistctrl", infinite },
    { "sizer", "dataviewtreectrl", infinite },
    { "sizer", "expanded_widget", infinite },
    { "sizer", "flatnotebook", infinite },
    { "sizer", "gbsizer", infinite },
    { "sizer", "listbook", infinite },
    { "sizer", "notebook", infinite },
    { "sizer", "propgrid", infinite },
    { "sizer", "propgridman", infinite },
    { "sizer", "ribbonbar", infinite },
    { "sizer", "simplebook", infinite },
    { "sizer", "sizer", infinite },
    { "sizer", "splitter", infinite },
    { "sizer", "toolbar", infinite },
    { "sizer", "treelistctrl", infinite },
    { "sizer", "widget", infinite },

    { "splitter", "container", two },

    { "submenu", "menuitem", infinite },
    { "submenu", "submenu", infinite },

    { "tool", "menu", one },

    { "toolbar", "expanded_widget", infinite },
    { "toolbar", "tool", infinite },
    { "toolbar", "widget", infinite },
    { "toolbar_form", "expanded_widget", infinite },
    { "toolbar_form", "tool", infinite },
    { "toolbar_form", "widget", infinite },

    { "treelistctrl", "expanded_widget", infinite },
    { "treelistctrl", "menu", one },
    { "treelistctrl", "treelistctrlcolumn", infinite },

    { "widget", "expanded_widget", one },
    { "widget", "menu", one },
    { "widget", "image", infinite },

    { "wizard", "menu", one },
    { "wizard", "wizardpagesimple", infinite },

    { "wizardpagesimple", "gbsizer", one },
    { "wizardpagesimple", "menu", one },
    { "wizardpagesimple", "sizer", one },

};

static constexpr const char* lstNodeTypes[] = {

    "auinotebook",
    "bookpage",
    "choicebook",
    "container",
    "dataviewcolumn",
    "dataviewctrl",
    "dataviewlistcolumn",
    "dataviewlistctrl",
    "dataviewtreectrl",
    "expanded_widget",
    "flatnotebook",
    "form",
    "gbsizer",
    "image",
    "interface",
    "listbook",
    "menu",
    "menubar",
    "menubar_form",
    "menuitem",
    "nonvisual",
    "notebook",
    "project",
    "propgrid",
    "propgrid",
    "propgriditem",
    "propgridman",
    "propgridman",
    "propgridpage",
    "propgridpage",
    "ribbonbar",
    "ribbonbutton",
    "ribbonbuttonbar",
    "ribbongallery",
    "ribbongalleryitem",
    "ribbonpage",
    "ribbonpanel",
    "ribbontool",
    "ribbontoolbar",
    "simplebook",
    "sizer",
    "splitter",
    "statusbar",
    "submenu",
    "tool",
    "toolbar",
    "toolbar_form",
    "treelistctrl",
    "treelistctrlcolumn",
    "widget",
    "wizard",
    "wizardpagesimple",

};

// These are types used to convert wxFormBuilder projects
static constexpr const char* fb_ImportTypes[] = {

    "sizeritem",
    "gbsizeritem",
    "splitteritem",

    "oldbookpage",

    // BUGBUG: [KeyWorks - 12-10-2020] This thing still exists and is bogus!
    "auinotebookpage",

};

// clang-format on

void NodeCreator::InitCompTypes()
{
    for (auto& name: lstNodeTypes)
    {
        m_component_types[name] = std::make_unique<NodeType>(name);
    }

    for (auto& name: fb_ImportTypes)
    {
        m_setOldHostTypes.emplace(name);

        // REVIEW: [KeyWorks - 12-10-2020] We still need this while auinotebookpage exists...
        m_component_types[name] = std::make_unique<NodeType>(name);
    }

    for (auto& child: lstParentChildren)
    {
        ASSERT(m_component_types.find(child.parent) != m_component_types.end());
        auto parent_type = m_component_types.find(child.parent)->second.get();

        parent_type->AddChild(child.name, child.max_children);
    }
}

int_t NodeCreator::GetAllowableChildren(Node* parent, ttlib::cview child_type, bool is_aui_parent) const
{
    return parent->GetNodeDeclaration()->GetNodeType()->GetAllowableChildren(child_type, is_aui_parent);
}

void NodeType::AddChild(const char* name, int_t max_children)
{
    m_children[name] = std::make_unique<AllowableChildren>(max_children);
}

int_t NodeType::GetAllowableChildren(ttlib::cview child_name, bool is_aui_parent) const
{
    if (auto iter = m_children.find(child_name.c_str()); iter != m_children.end())
    {
        if (is_aui_parent && m_name == "form")
        {
            // wxAui forms do not use a top-level sizer

            if (child_name.is_sameas("sizer") || child_name.is_sameas("gbsizer"))
                return none;

            // Except for the "bar" types which both regular and aui forms only allow one of, all the other types can have
            // multiple children.
            if (iter->second.get()->max_children == none)
                return infinite;
        }
        return iter->second.get()->max_children;
    }
    return none;
}
