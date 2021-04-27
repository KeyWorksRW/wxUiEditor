/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains user-modifiable node
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "node.h"

#include "appoptions.h"    // AppOptions -- Application-wide options
#include "mainframe.h"     // MainFrame -- Main window frame
#include "node_creator.h"  // NodeCreator class
#include "node_decl.h"     // NodeDeclaration class
#include "node_prop.h"     // NodeProperty -- NodeProperty class
#include "uifuncs.h"       // Miscellaneous functions for displaying UI
#include "undo_cmds.h"     // InsertNodeAction -- Undoable command classes derived from UndoAction

using namespace GenEnum;

// Same as wxGetApp() only this returns a reference to the project node
Node& wxGetProject()
{
    ASSERT_MSG(wxGetApp().GetProjectPtr(), "MainFrame hasn't been created yet.");
    return *wxGetApp().GetProjectPtr().get();
}

Node::Node(NodeDeclaration* declaration) : m_declaration(declaration) {}

Node::~Node()
{
    // REVIEW: [KeyWorks - 09-09-2020] This makes no sense to me. We're being destroyed because our shared reference
    // count hit zero. If we then tell the parent to destroy us, the destructor will be called again, and since m_parent
    // didn't get cleared, we'll keep trying to have our parent destroy us.

    ASSERT(!m_parent);

#if 0  // [KeyWorks - 09-09-2020] Code disabled unless we can find a valid reason for it to be here and not blow up.
    auto parent = m_parent;

    if (parent)
    {
        try
        {
            NodeSharedPtr pobj(GetSharedPtr());
            parent->RemoveChild(pobj->get());
        }
        catch (const std::exception& e)
        {
            FAIL_MSG(e.what());
            MSG_ERROR(e.what());
        }
    }
#endif
}

NodeProperty* Node::get_prop_ptr(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return &m_properties[result->second];
    else
        return nullptr;
}

NodeProperty* Node::get_prop_ptr(ttlib::cview name)
{
    if (auto it = m_prop_map.find(name.c_str()); it != m_prop_map.end())
        return &m_properties[it->second];
    else
        return nullptr;
}

ttlib::cstr* Node::get_value_ptr(ttlib::cview name)
{
    if (auto it = m_prop_map.find(name.c_str()); it != m_prop_map.end())
        return &m_properties[it->second].get_value();
    else
        return nullptr;
}

NodeEvent* Node::GetEvent(ttlib::cview name)
{
    if (auto it = m_event_map.find(name.c_str()); it != m_event_map.end())
        return &m_events[it->second];
    else
        return nullptr;
}

NodeEvent* Node::GetEvent(size_t index)
{
    ASSERT(index < m_events.size());
    return &m_events[index];
}

size_t Node::GetInUseEventCount()
{
    size_t count = 0;

    for (auto& iter: m_events)
    {
        if (iter.get_value().size())
            ++count;
    }

    return count;
}

NodeProperty* Node::AddNodeProperty(PropDeclaration* declaration)
{
    auto& prop = m_properties.emplace_back(declaration, this);
    m_prop_map[prop.DeclName().c_str()] = (m_properties.size() - 1);
    m_prop_indices[prop.get_name()] = (m_properties.size() - 1);
    return &m_properties[m_properties.size() - 1];
}

NodeEvent* Node::AddNodeEvent(const NodeEventInfo* info)
{
    auto& event = m_events.emplace_back(info, this);
    m_event_map[event.get_name()] = (m_events.size() - 1);
    return &m_events[m_events.size() - 1];
}

Node* Node::FindNearAncestor(GenType type)
{
    Node* result = nullptr;
    auto parent = GetParent();
    if (parent)
    {
        if (parent->isType(type))
            result = parent;
        else
            result = parent->FindNearAncestor(type);
    }

    return result;
}

Node* Node::FindParentForm()
{
    if (auto retObj = FindNearAncestor(type_form); retObj)
        return retObj;
    if (auto retObj = FindNearAncestor(type_menubar_form); retObj)
        return retObj;
    if (auto retObj = FindNearAncestor(type_toolbar_form); retObj)
        return retObj;
    if (auto retObj = FindNearAncestor(type_wizard); retObj)
        return retObj;

    return nullptr;
}

bool Node::AddChild(NodeSharedPtr node)
{
    if (IsChildAllowed(node))
    {
        m_children.push_back(node);
        return true;
    }

    return false;
}

bool Node::AddChild(Node* node)
{
    if (IsChildAllowed(node))
    {
        m_children.push_back(node->GetSharedPtr());
        return true;
    }

    return false;
}

bool Node::AddChild(size_t idx, NodeSharedPtr node)
{
    if (IsChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node);
        return true;
    }

    return false;
}

bool Node::AddChild(size_t idx, Node* node)
{
    if (IsChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node->GetSharedPtr());
        return true;
    }

    return false;
}

bool Node::IsChildAllowed(Node* child)
{
    ASSERT(child);
    auto child_type = child->GetNodeType();
    int_t max_children;

    if (isType(type_form))
    {
        // max_children = GetNodeDeclaration()->GetNodeType()->GetAllowableChildren(child_type, prop_as_bool(prop_aui));
        max_children = GetNodeDeclaration()->GetNodeType()->GetAllowableChildren(child_type, false);
    }
    else
        max_children = GetNodeDeclaration()->GetNodeType()->GetAllowableChildren(child_type);

    if (max_children == child_count::none)
        return false;

    if (max_children == child_count::infinite)
        return true;

    // Because m_children containts shared_ptrs, we don't want to use an iteration loop which will get/release the shared
    // ptr. Using an index into the vector lets us access the raw pointer.

    int_t children = 0;
    for (size_t i = 0; i < m_children.size() && children <= max_children; ++i)
    {
        if (GetChild(i)->GetNodeDeclaration()->GetNodeType() == child_type)
            ++children;
    }

    if (children >= max_children)
        return false;

    return true;
}

void Node::RemoveChild(NodeSharedPtr node)
{
    auto iter = m_children.begin();
    while (iter != m_children.end() && iter->get() != node.get())
        iter++;

    if (iter != m_children.end())
        m_children.erase(iter);
}

void Node::RemoveChild(Node* node)
{
    auto iter = m_children.begin();
    while (iter != m_children.end() && iter->get() != node)
        iter++;

    if (iter != m_children.end())
        m_children.erase(iter);
}

void Node::RemoveChild(size_t pos)
{
    ASSERT(pos < m_children.size());

    auto iter = m_children.begin() + pos;
    m_children.erase(iter);
}

bool Node::IsChildType(size_t index, ttlib::cview type)
{
    if (index >= m_children.size())
        return false;
    return (m_children[index]->GetNodeTypeName() == type);
}

size_t Node::GetChildPosition(Node* node)
{
    size_t pos = 0;
    while (pos < GetChildCount() && m_children[pos].get() != node)
        ++pos;

    return pos;
}

bool Node::ChangeChildPosition(NodeSharedPtr node, size_t pos)
{
    size_t cur_pos = GetChildPosition(node.get());

    if (cur_pos == GetChildCount() || pos >= GetChildCount())
        return false;

    if (pos == cur_pos)
        return true;

    RemoveChild(node.get());
    AddChild(pos, node);
    return true;
}

bool Node::IsLocal()
{
    return isPropValue(prop_class_access, "none");
}

bool Node::HasValue(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].HasValue();
    else
        return false;
}

bool Node::isPropValue(PropName name, const char* value)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].GetValue().is_sameas(value));
    }

    return false;
}

bool Node::isPropValue(PropName name, bool value)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_bool() == value);
    }

    return false;
}

bool Node::prop_as_bool(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_bool();
    else
        return false;
}

int Node::prop_as_int(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_int();
    else
        return 0;
}

wxColour Node::prop_as_wxColour(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_color();
    else
        return wxColour();
}

wxFont Node::prop_as_font(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_font();
    else
        return *wxNORMAL_FONT;
}

wxPoint Node::prop_as_wxPoint(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_point();
    else
        return wxDefaultPosition;
}

wxSize Node::prop_as_wxSize(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_size();
    else
        return wxDefaultSize;
}

wxBitmap Node::prop_as_wxBitmap(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_bitmap();
    else
        return wxNullBitmap;
}

wxArrayString Node::prop_as_wxArrayString(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_wxArrayString();
    else
        return wxArrayString();
}

FontProperty Node::prop_as_font_prop(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_font_prop();
    else
        return FontProperty(wxNORMAL_FONT);
}

double Node::prop_as_double(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_float();
    else
        return 0;
}

wxString Node::prop_as_wxString(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_wxString();
    else
        return wxString();
}

const ttlib::cstr& Node::prop_as_string(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].get_value();
    else
        return tt_empty_cstr;
}

const ttlib::cstr& Node::get_node_name()
{
    if (auto it = m_prop_indices.find(prop_var_name); it != m_prop_indices.end())
        return m_properties[it->second].get_value();
    else if (it = m_prop_indices.find(prop_class_name); it != m_prop_indices.end())
        return m_properties[it->second].get_value();
    else
        return tt_empty_cstr;
}

const ttlib::cstr& Node::get_parent_name()
{
    if (m_parent)
        return m_parent->get_node_name();

    return tt_empty_cstr;
}

const ttlib::cstr& Node::get_form_name()
{
    if (auto form = FindParentForm(); form)
    {
        return form->get_node_name();
    }
    return tt_empty_cstr;
}

wxSizerFlags Node::GetSizerFlags()
{
    wxSizerFlags flags;
    flags.Proportion(prop_as_int(prop_proportion));
    auto border_size = prop_as_int(prop_border_size);
    int direction = 0;
    auto& border_settings = prop_as_string(prop_borders);
    if (border_settings.contains("wxALL"))
    {
        direction = wxALL;
    }
    else
    {
        if (border_settings.contains("wxLEFT"))
            direction |= wxLEFT;
        if (border_settings.contains("wxRIGHT"))
            direction |= wxRIGHT;
        if (border_settings.contains("wxTOP"))
            direction |= wxTOP;
        if (border_settings.contains("wxBOTTOM"))
            direction |= wxBOTTOM;
    }
    flags.Border(direction, border_size);

    if (auto& alignment = prop_as_string(prop_alignment); alignment.size())
    {
        if (alignment.contains("wxALIGN_CENTER"))
        {
            if (alignment.contains("wxALIGN_CENTER_VERTICAL") && alignment.contains("wxALIGN_CENTER_HORIZONTAL"))
                flags.Center();
            else if (alignment.contains("wxALIGN_CENTER_VERTICAL"))
                flags.CenterVertical();
            else if (alignment.contains("wxALIGN_CENTER_HORIZONTAL"))
                flags.CenterHorizontal();
            else
                flags.Center();
        }
        else
        {
            if (alignment.contains("wxALIGN_LEFT"))
                flags.Left();
            if (alignment.contains("wxALIGN_RIGHT"))
                flags.Right();
            if (alignment.contains("wxALIGN_TOP"))
                flags.Top();
            if (alignment.contains("wxALIGN_BOTTOM"))
                flags.Bottom();
        }
    }

    if (auto& prop = prop_as_string(prop_flags); prop.size())
    {
        if (prop.contains("wxEXPAND"))
            flags.Expand();
        if (prop.contains("wxSHAPED"))
            flags.Shaped();
        if (prop.contains("wxFIXED_MINSIZE"))
            flags.FixedMinSize();
        if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
            flags.ReserveSpaceEvenIfHidden();
    }

    return flags;
}

Node* Node::CreateChildNode(ttlib::cview name)
{
    auto& frame = wxGetFrame();

    auto new_node = g_NodeCreator.CreateNode(name, this);

    if (new_node)
    {
#if defined(_WIN32)

        // In a Windows build, the default background colour of white doesn't match the normal background color of the parent
        // so we set it to the more normal Windows colour.

        // REVIEW: [KeyWorks - 03-17-2021] Need to figure out a better way to do this which is cross platform. As it
        // currently exists, if the Windows version of wxUiEditor is used, then ALL versions of the app the user creates will
        // use this background color.

        if (name.is_sameas("BookPage"))
        {
            if (auto prop = new_node->get_prop_ptr(prop_background_colour); prop)
            {
                prop->set_value("wxSYS_COLOUR_BTNFACE");
                frame.FirePropChangeEvent(prop);
            }
        }
#endif  // _WIN32

        ttlib::cstr undo_str = "insert " + name;
        frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), this, undo_str));
        new_node->FixDuplicateName();
    }

    // A "ribbonButton" component is used for both wxRibbonButtonBar and wxRibbonToolBar. If creating the node failed,
    // then assume the parent is wxRibbonToolBar and retry with "ribbonTool"
    else if (name.is_sameas("ribbonButton"))
    {
        new_node = g_NodeCreator.CreateNode(gen_ribbonTool, this);
        if (new_node)
        {
            ttlib::cstr undo_str = "insert ribbon tool";
            frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), this, undo_str));
            new_node->FixDuplicateName();
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        // If we get here, then the current node cannot be a parent of the new node. We look at the current node's
        // parent and see if it accepts the new node as a child. This deals with the case where the user selected a
        // widget, and wants a new widget created right after the selected widget with both having the same parent
        // (typically a sizer).

        auto parent = GetParent();

        if (parent)
        {
            new_node = g_NodeCreator.CreateNode(name, parent);
            if (new_node)
            {
                auto pos = parent->FindInsertionPos(this);
                ttlib::cstr undo_str = "insert " + name;
                frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
                new_node->FixDuplicateName();
            }
        }
        else
        {
            appMsgBox(ttlib::cstr() << "You cannot add " << name << " as a child of " << DeclName());
            return nullptr;
        }
    }

    if (new_node)
    {
        frame.FireCreatedEvent(new_node.get());
        frame.SelectNode(new_node.get(), true, true);
    }
    return new_node.get();
}

Node* Node::CreateNode(ttlib::cview name)
{
    auto& frame = wxGetFrame();
    auto cur_selection = frame.GetSelectedNode();
    if (!cur_selection)
    {
        appMsgBox("You need to select something first in order to properly place this widget.");
        return nullptr;
    }
    return cur_selection->CreateChildNode(name);
}

bool Node::CreateToolNode(GenName name)
{
    // TODO: [KeyWorks - 04-15-2021] Currently we convert the name into a string and call CreateToolNode with that string.
    // Ultimately, we should reverse that and convert the string into a GenName and make this the primary function.

    return CreateToolNode(map_GenNames[name]);
}

bool Node::CreateToolNode(const ttlib::cstr& name)
{
    auto new_node = CreateChildNode(name);
    if (!new_node)
        return false;

    auto& frame = wxGetFrame();

    if (name == "wxDialog" || name == "PanelForm" || name == "wxPanel" || name == "wxPopupTransientWindow")
    {
        auto child_node = new_node->CreateChildNode("VerticalBoxSizer");
        if (child_node)
        {
            if (auto prop = child_node->get_prop_ptr(prop_orientation); prop)
            {
                prop->set_value("wxVERTICAL");
                frame.FirePropChangeEvent(prop);
            }
            if (auto prop = child_node->get_prop_ptr(prop_var_name); prop)
            {
                child_node->ModifyProperty(prop, "parent_sizer");
                if (child_node->FixDuplicateName())
                {
                    frame.FirePropChangeEvent(prop);
                }
            }

            frame.SelectNode(new_node);
        }
    }
    else if (name == "wxNotebook" || name == "wxSimplebook" || name == "wxChoicebook" || name == "wxListbook" ||
             name == "wxAuiNotebook")
    {
        new_node = new_node->CreateChildNode("BookPage");

        new_node = new_node->CreateChildNode("VerticalBoxSizer");
        if (new_node)
        {
            if (auto prop = new_node->get_prop_ptr(prop_orientation); prop)
            {
                prop->set_value("wxVERTICAL");
                frame.FirePropChangeEvent(prop);
            }
            if (auto prop = new_node->get_prop_ptr(prop_var_name); prop)
            {
                new_node->ModifyProperty(prop, "parent_sizer");
                if (new_node->FixDuplicateName())
                {
                    frame.FirePropChangeEvent(prop);
                }
            }
        }
    }
    else if (name == "BookPage")
    {
        new_node = new_node->CreateChildNode("VerticalBoxSizer");
        if (new_node)
        {
            if (auto prop = new_node->get_prop_ptr(prop_orientation); prop)
            {
                prop->set_value("wxVERTICAL");
                frame.FirePropChangeEvent(prop);
            }
            if (auto prop = new_node->get_prop_ptr(prop_var_name); prop)
            {
                new_node->ModifyProperty(prop, "parent_sizer");
                if (new_node->FixDuplicateName())
                {
                    frame.FirePropChangeEvent(prop);
                }
            }
        }
    }
    else if (name == "wxWizard")
    {
        new_node = new_node->CreateChildNode("wxWizardPageSimple");
        new_node->CreateChildNode("VerticalBoxSizer");
    }
    else if (name == "wxWizardPageSimple")
    {
        new_node->CreateChildNode("VerticalBoxSizer");
    }
    else if (name == "wxBoxSizer" || name == "VerticalBoxSizer" || name == "wxWrapSizer" || name == "wxGridSizer" ||
             name == "wxFlexGridSizer" || name == "wxGridBagSizer" || name == "wxStaticBoxSizer" ||
             name == "StaticCheckboxBoxSizer" || name == "StaticRadioBtnBoxSizer")
    {
        auto node = new_node->GetParent();
        ASSERT(node);

        if (auto prop = node->get_prop_ptr(prop_borders); prop)
        {
            if (GetAppOptions().get_SizersAllBorders())
                prop->set_value("wxALL");
        }

        if (auto prop = node->get_prop_ptr(prop_flags); prop)
        {
            if (GetAppOptions().get_SizersExpand())
                prop->set_value("wxEXPAND");
        }
    }
    else if (name == "wxStdDialogButtonSizer" || name == "wxStaticLine")
    {
        if (auto prop = new_node->get_prop_ptr(prop_flags); prop)
        {
            prop->set_value("wxEXPAND");
            frame.FirePropChangeEvent(prop);
        }
    }

    return true;
}

void Node::ModifyProperty(PropName name, ttlib::cview value)
{
    auto prop = get_prop_ptr(name);
    if (prop && value != prop->as_cview())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        wxGetFrame().FirePropChangeEvent(prop);
    }
}

void Node::ModifyProperty(ttlib::cview name, int value)
{
    auto prop = get_prop_ptr(name);
    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        wxGetFrame().FirePropChangeEvent(prop);
    }
}

void Node::ModifyProperty(ttlib::cview name, ttlib::cview value)
{
    auto prop = get_prop_ptr(name);
    if (prop && value != prop->as_cview())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        wxGetFrame().FirePropChangeEvent(prop);
    }
}

void Node::ModifyProperty(NodeProperty* prop, int value)
{
    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        wxGetFrame().FirePropChangeEvent(prop);
    }
}

void Node::ModifyProperty(NodeProperty* prop, ttlib::cview value)
{
    if (prop && value != prop->as_cview())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        wxGetFrame().FirePropChangeEvent(prop);
    }
}

ttlib::cstr Node::GetUniqueName(const ttlib::cstr& proposed_name)
{
    ttlib::cstr new_name(proposed_name);
    if (IsForm())
        return {};

    auto form = FindParentForm();
    if (!form)
        return {};

    std::unordered_set<std::string> name_set;
    form->CollectUniqueNames(name_set, this);

    if (auto it = name_set.find(new_name); it != name_set.end())
    {
        // We get here if the name has already been used.

        std::string org_name(proposed_name);
        while (ttlib::is_digit(org_name.back()))
        {
            // remove any trailing digits
            org_name.erase(org_name.size() - 1, 1);
        }

        for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
        {
            new_name.clear();
            new_name << org_name << '_' << i;
        }
    }

    // We return this name whether or not it has actually changed.
    return new_name;
}

bool Node::FixDuplicateName(bool is_validator)
{
    auto name = get_value_ptr(is_validator ? "validator_variable" : "var_name");
    if (!name || name->empty())
        return false;

    auto form = FindParentForm();
    if (!form)
        form = wxGetApp().GetProjectPtr().get();

    std::unordered_set<std::string> name_set;
    form->CollectUniqueNames(name_set, this);

    if (auto it = name_set.find(*name); it != name_set.end())
    {
        // We get here if the name has already been used.

        std::string org_name(*name);
        while (ttlib::is_digit(org_name.back()))
        {
            // remove any trailing digits
            org_name.erase(org_name.size() - 1, 1);
        }

        ttlib::cstr new_name;
        for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
        {
            new_name.clear();
            new_name << org_name << '_' << i;
        }
        *name = new_name;
        return true;
    }
    return false;
}

void Node::FixPastedNames()
{
    if (!IsForm())
    {
        FixDuplicateName();      // fix variable names
        FixDuplicateName(true);  // fix validator names
    }

    for (auto& iter: GetChildNodePtrs())
    {
        iter->FixDuplicateName();
        iter->FixDuplicateName(true);
    }
}

void Node::CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node)
{
    if (!IsForm() && cur_node != this)
    {
        if (auto name = get_value_ptr(txt_var_name); name && name->size())
        {
            name_set.emplace(*name);
        }

        // Check for StaticCheckboxBoxSizer checkbox variable
        if (auto& name = prop_as_string(prop_checkbox_var_name); name.size())
        {
            name_set.emplace(name);
        }

        // Check for StaticRadioBtnBoxSizer radiobtn variable
        if (auto& name = prop_as_string(prop_radiobtn_var_name); name.size())
        {
            name_set.emplace(name);
        }

        if (auto& name = prop_as_string(prop_validator_variable); name.size())
        {
            name_set.emplace(name);
        }
    }

    for (auto& iter: GetChildNodePtrs())
    {
        iter->CollectUniqueNames(name_set, cur_node);
    }
}

int_t Node::FindInsertionPos(Node* child)
{
    if (child)
    {
        for (size_t new_pos = 0; new_pos < GetChildCount(); ++new_pos)
        {
            if (GetChild(new_pos) == child)
                return new_pos + 1;
        }
    }
    return -1;
}

size_t Node::GetNodeSize()
{
    auto size = sizeof(*this);
    // Add the size of all the node pointers, but not the size of the individual children
    size += (m_children.size() * sizeof(void*));

    for (auto& iter: m_properties)
    {
        size += iter.GetPropSize();
    }

    for (auto& iter: m_events)
    {
        size += iter.GetEventSize();
    }

    // Add the size of our maps

    size += (m_prop_indices.size() * (sizeof(size_t) * 2));
    size += (m_event_map.size() * (sizeof(std::string) + sizeof(size_t)));

    // Hopefully, this map will get removed at some point
    size += (m_prop_map.size() * (sizeof(std::string) + sizeof(size_t)));

    return size;
}
