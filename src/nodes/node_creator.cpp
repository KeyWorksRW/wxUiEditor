/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <ttcstr.h>
#include <ttlibspace.h>

#include "node_creator.h"

#include "../pugixml/pugixml.hpp"

#include "bitmaps.h"       // Map of bitmaps accessed by name
#include "enum_classes.h"  // Enumerations for nodes
#include "node.h"          // Node class
#include "prop_info.h"     // PropDefinition and PropertyInfo classes

NodeCreator g_NodeCreator;

using namespace NodeEnums;

void NodeCreator::Initialize()
{
    for (auto& iter: NodeEnums::map_PropTypes)
    {
        NodeEnums::rmap_PropTypes[iter.second] = iter.first;
    }

    for (auto& iter: NodeEnums::map_PropNames)
    {
        NodeEnums::rmap_PropNames[iter.second] = iter.first;
    }

    for (auto& iter: NodeEnums::map_ClassTypes)
    {
        NodeEnums::rmap_ClassTypes[iter.second] = iter.first;
    }

    for (auto& iter: NodeEnums::map_ClassNames)
    {
        NodeEnums::rmap_ClassNames[iter.second] = iter.first;
    }

    InitCompTypes();
    InitDeclarations();
}

NodeCreator::~NodeCreator()
{
    for (auto& iter: m_a_declarations)
    {
        delete iter;
    }
}

NodeDeclaration* NodeCreator::GetNodeDeclaration(ttlib::cview className)
{
    return m_a_declarations[static_cast<size_t>(rmap_ClassNames[className.c_str()])];
}

// This will add all properties and events, including any base interface classes such as wxWindow, sizeritem, etc.
NodeSharedPtr NodeCreator::NewNode(NodeDeclaration* declaration)
{
    auto node = std::make_shared<Node>(declaration);

    // Calling GetBaseClassCount() is expensive, so do it once and store the result
    auto node_info_base_count = declaration->GetBaseClassCount();

    size_t base = 0;
    for (auto class_info = declaration; class_info; class_info = declaration->GetBaseClass(base++))
    {
        for (size_t index = 0; index < class_info->GetPropertyCount(); ++index)
        {
            auto prop_info = class_info->GetPropertyInfo(index);

            // Set the default value, either from the property info, or an override from this class
            auto defaultValue = prop_info->GetDefaultValue();
            if (base > 0)
            {
                auto defaultValueTemp =
                    declaration->GetBaseClassDefaultPropertyValue(base - 1, prop_info->GetName().c_str());
                if (!defaultValueTemp.empty())
                {
                    defaultValue = defaultValueTemp;
                }
            }

            auto prop = node->AddNodeProperty(prop_info);
            prop->set_value(defaultValue);
        }

        for (size_t index = 0; index < class_info->GetEventCount(); ++index)
        {
            node->AddNodeEvent(class_info->GetEventInfo(index));
        }

        if (base >= node_info_base_count)
            break;
    }

    return node;
}

size_t NodeCreator::CountChildrenWithSameType(Node* parent, NodeType* type)
{
    size_t count = 0;
    size_t numChildren = parent->GetChildCount();
    for (size_t i = 0; i < numChildren; ++i)
    {
        if (type == parent->GetChild(i)->GetNodeDeclaration()->GetNodeType())
            ++count;
    }

    return count;
}

/*

    * This will return NULL if the parent doesn't allow this child type, or the parent already has the maximum number of
    * children allowed. E.g., containers can only have one child, splitters can only have two, and sizers can have an
    * unlimited number.

*/

// The parent parameter is used to determine if the parent allows this type of child, and if so how many of those
// children are allowed.
NodeSharedPtr NodeCreator::CreateNode(ttlib::cview classname, Node* parent)
{
    NodeSharedPtr node;
    NodeDeclaration* declaration;

    // This is a way for a ribbon panel button to indicate a wxBoxSizer with vertical orientation
    if (classname.is_sameas("VerticalBoxSizer"))
        declaration = GetNodeDeclaration("wxBoxSizer");
    else
        declaration = GetNodeDeclaration(classname);

    if (!declaration)
    {
        FAIL_MSG(ttlib::cstr() << "No component definition for " << classname);
        throw std::runtime_error("Internal error: missing component definition");
    }

    if (!parent)
        return NewNode(declaration);

    // This happens when importing wxFormBuilder and old wxUiEditor projects
    if (IsOldHostType(classname))
        return NewNode(declaration);

    bool aui = false;
    if (parent->GetNodeTypeName() == "form")
    {
        aui = parent->prop_as_int("aui_managed") != 0;
    }

    auto comp_type = declaration->GetNodeType();

    // Check for widgets which can ONLY have a frame for a parent.
    if (comp_type->get_name() == "statusbar" || comp_type->get_name() == "menubar" || comp_type->get_name() == "ribbonbar" ||
        comp_type->get_name() == "toolbar")
    {
        if (parent->GetNodeTypeName() == "form" && parent->GetClassName() != "wxFrame")
        {
            return NodeSharedPtr();
        }
    }
    else if (parent->GetNodeTypeName() == "tool")
    {
        auto grand_parent = parent->GetParent();
        if (grand_parent->GetClassName() == "wxToolBar" && comp_type->get_type() == ClassType::type_menu)
            return NodeSharedPtr();
    }

    auto max_children = GetAllowableChildren(parent, comp_type->get_type(), aui);

    if (max_children == child_count::infinite)
    {
        node = NewNode(declaration);
        if (classname.is_sameas("VerticalBoxSizer"))
            node->get_prop_ptr(txt_orientation)->set_value("wxVERTICAL");
    }
    else if (max_children != child_count::none)
    {
        if (comp_type == GetNodeType(ClassType::type_sizer))
        {
            node = NewNode(declaration);
            if (classname.is_sameas("VerticalBoxSizer"))
                node->get_prop_ptr(txt_orientation)->set_value("wxVERTICAL");
        }
        else if (comp_type == GetNodeType(ClassType::type_gbsizer))
        {
            node = NewNode(declaration);
        }
        else
        {
            auto count = CountChildrenWithSameType(parent, comp_type);
            ASSERT_MSG(count < (size_t) max_children,
                       "Parent allows one of this child type, a second of the same type is not allowed");
            if (count < (size_t) max_children)
                node = NewNode(declaration);
        }
    }
    else
    {
        // The parent doesn't allow this child type. Since node is empty at this point, we simply fall through and return
        // it. It's the caller's responsibility to try to find a proper parent.
    }

    return node;
}

NodeSharedPtr NodeCreator::MakeCopy(Node* node)
{
    ASSERT(node);

    auto objInfo = node->GetNodeDeclaration();

    auto copyObj = NewNode(objInfo);
    ASSERT(copyObj);

    for (auto& iter: node->get_props_vector())
    {
        auto copyProp = copyObj->get_prop_ptr(iter.GetPropName());
        ASSERT(copyProp);

        copyProp->set_value(iter.as_string());
    }

    auto count = node->GetEventCount();
    for (size_t i = 0; i < count; i++)
    {
        auto event = node->GetEvent(i);
        auto copyEvent = copyObj->GetEvent(event->get_name());
        ASSERT(copyEvent)
        copyEvent->set_value(event->get_value());
    }

    count = node->GetChildCount();
    for (size_t i = 0; i < count; i++)
    {
        auto childCopy = MakeCopy(node->GetChild(i));
        ASSERT(childCopy)
        copyObj->AddChild(childCopy);
        childCopy->SetParent(copyObj);
    }

    return copyObj;
}

// TODO: [KeyWorks - 11-21-2020] Once a new node is created and placed in it's heirachy, the code that created the
// node should call this to setup default sizer settings.
void NodeCreator::SetDefaultLayoutProperties(Node* node)
{
    auto declaration = node->GetNodeDeclaration();

    // Caution: Do NOT place spaces around the | that combines flags. Other parts of the codebase rely on there being no
    // spaces...

    if (declaration->GetClassName() == "wxStdDialogButtonSizer")
    {
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
        return;
    }

    auto node_type = node->GetNodeTypeName();
    auto proportion = node->get_prop_ptr(txt_proportion);

    if (declaration->GetClassName() == "wxStaticLine" || declaration->GetClassName() == "wxStdDialogButtonSizer")
    {
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
    else if (node->IsSizer() || node_type == "splitter" || declaration->GetClassName() == "spacer")
    {
        if (proportion)
        {
            proportion->set_value("1");
        }
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
    else if (declaration->GetClassName() == "wxToolBar")
    {
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
    else if (node_type == "widget" || node_type == "statusbar")
    {
        if (proportion)
        {
            proportion->set_value("0");
        }
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
    }
    else if (node_type == "notebook" || node_type == "listbook" || node_type == "simplebook" || node_type == "choicebook" ||
             node_type == "auinotebook" || node_type == "treelistctrl" || node_type == "expanded_widget" ||
             node_type == "container")
    {
        if (proportion)
        {
            proportion->set_value("1");
        }
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
}

// clang-format off
constexpr const char* GeneratorDefFiles[] = {

    "bars",
    "forms",
    "containers",
    "sizers",
    "widgets",
    // "aui.xml",

};
// clang-format on

void NodeCreator::InitDeclarations()
{
    ParseGeneratorFile("interface");

    for (auto& iter: GeneratorDefFiles)
    {
        ParseGeneratorFile(iter);
        SetupGroup(iter);
    }

    InitGenerators();
}

int NodeCreator::GetConstantAsInt(const std::string& name, int defValue)
{
    if (auto iter = m_map_constants.find(name); iter != m_map_constants.end())
        return iter->second;
    else
        return defValue;
}

bool NodeCreator::ParseGeneratorFile(ttlib::cview name)
{
    pugi::xml_document doc;
    if (!LoadInternalXmlDocFile(name, doc))
    {
        // In _DEBUG builds, an assertion will have already been generated.
        return false;
    }

    auto root = doc.child("GeneratorDefitions");
    if (!root)
    {
        FAIL_MSG(ttlib::cstr("Cannot locate group in the name ") << name);
        return false;
    }

    ParseCompInfo(root);

    return true;
}

void NodeCreator::ParseCompInfo(pugi::xml_node root)
{
    auto comp_info = root.child("compinfo");
    while (comp_info)
    {
        auto class_name = comp_info.attribute("class").as_string();
#if defined(_DEBUG)
        if (rmap_ClassNames.find(class_name) == rmap_ClassNames.end())
        {
            MSG_WARNING(ttlib::cstr("Unrecognized class name -- ") << class_name);
        }
#endif  // _DEBUG

        auto type = comp_info.attribute("type").as_cview();
#if defined(_DEBUG)
        if (rmap_ClassTypes.find(type.c_str()) == rmap_ClassTypes.end())
        {
            MSG_WARNING(ttlib::cstr("Unrecognized class type -- ") << type);
        }
#endif  // _DEBUG

        auto declaration = new NodeDeclaration(class_name, GetNodeType(rmap_ClassTypes[type.c_str()]));
        m_a_declarations[declaration->class_index()] = declaration;

        if (auto flags = comp_info.attribute("flags").as_cview(); flags.size())
        {
            declaration->SetCompFlags(flags);
        }

        auto image_name = comp_info.attribute("image").as_cview();
        if (image_name.size())
        {
            auto image = GetXPMImage(image_name);
            if (image.GetWidth() != CompImgSize || image.GetHeight() != CompImgSize)
            {
                MSG_INFO(ttlib::cstr() << image_name << " width: " << image.GetWidth() << "height: " << image.GetHeight());
                declaration->SetImage(image.Scale(CompImgSize, CompImgSize));
            }
            else
            {
                declaration->SetImage(image);
            }
        }
        else
        {
            declaration->SetImage(GetXPMImage("unknown").Scale(CompImgSize, CompImgSize));
        }

        // ParseProperties(comp_info, declaration.get(), declaration->GetCategory());
        ParseProperties(comp_info, declaration, declaration->GetCategory());

        declaration->ParseEvents(comp_info, declaration->GetCategory());

        comp_info = comp_info.next_sibling("compinfo");
    }
}

void NodeCreator::SetupGroup(ttlib::cview name)
{
    pugi::xml_document doc;
    if (!LoadInternalXmlDocFile(name, doc))
    {
        // In _DEBUG builds, an assertion will have already been generated.

        // TODO: [KeyWorks - 05-29-2020] We should throw since wxUiEditor will be unable to continue...
        return;
    }

    auto root = doc.child("GeneratorDefitions");
    if (!root)
    {
        FAIL_MSG(ttlib::cstr("Cannot locate \"group\" child in the name ") << name);
        return;
    }

    auto elem_obj = root.child("compinfo");
    while (elem_obj)
    {
        auto class_name = elem_obj.attribute("class").as_cview();
        auto class_info = GetNodeDeclaration(class_name);

        auto elem_base = elem_obj.child("inherits");
        while (elem_base)
        {
            auto base_name = elem_base.attribute("class").as_cview();

            // Add a reference to its base class
            auto base_info = GetNodeDeclaration(base_name);
            if (class_info && base_info)
            {
                size_t baseIndex = class_info->AddBaseClass(base_info);

                auto inheritedProperty = elem_base.child("property");
                while (inheritedProperty)
                {
                    auto prop_name = inheritedProperty.attribute("name").as_cview();
                    auto value = inheritedProperty.text().as_string();
                    class_info->AddBaseClassDefaultPropertyValue(baseIndex, prop_name, value);
                    inheritedProperty = inheritedProperty.next_sibling("property");
                }
            }
            elem_base = elem_base.next_sibling("inherits");
        }

        elem_obj = elem_obj.next_sibling("compinfo");
    }
}

void NodeCreator::ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* obj_info, NodeCategory& category)
{
    auto elem_category = elem_obj.child("category");
    while (elem_category)
    {
        auto name = elem_category.attribute("name").as_cview();
        auto& new_cat = category.AddCategory(name);

        // Recurse
        ParseProperties(elem_category, obj_info, new_cat);

        elem_category = elem_category.next_sibling("category");
    }

    auto elem_prop = elem_obj.child("property");
    while (elem_prop)
    {
        auto name = elem_prop.attribute("name").as_string();
        NodeEnums::Prop prop_name = Prop::missing_property_name;
        auto lookup_name = rmap_PropNames.find(name);
        if (lookup_name == rmap_PropNames.end())
        {
            MSG_ERROR(ttlib::cstr("Unrecognized property name -- ") << name);
            elem_prop = elem_prop.next_sibling("property");
            continue;
        }
        prop_name = lookup_name->second;

        category.AddProperty(name);

        auto description = elem_prop.attribute("help").as_cview();
        auto customEditor = elem_prop.attribute("editor").as_cview();

        auto prop_type = elem_prop.attribute("type").as_cview();

        auto property_type = enum_missing_property_type;
        auto lookup_type = rmap_PropTypes.find(prop_type.c_str());
        if (lookup_type == rmap_PropTypes.end())
        {
            MSG_ERROR(ttlib::cstr("Unrecognized property type -- ") << prop_type);
            elem_prop = elem_prop.next_sibling("property");
            continue;
        }
        property_type = lookup_type->second;

        ttlib::cstr def_value;
        if (auto lastChild = elem_prop.last_child(); lastChild && !lastChild.text().empty())
        {
            def_value = lastChild.text().get();
            if (ttlib::is_found(def_value.find('\n')))
            {
                def_value.trim(tt::TRIM::both);
            }
        }

        std::vector<PropDefinition> children;
        if (property_type == enum_bool)
        {
            // If the property is a parent, then get the children
            def_value.clear();
            auto elem_child = elem_prop.child("child");
            while (elem_child)
            {
                PropDefinition child;

                child.m_name = elem_child.attribute("name").as_string();
                child.m_help = elem_child.attribute("help").as_string();

                if (child.m_help.empty())
                {
                    if (child.m_name == txt_class_access)
                        child.m_help = "Determines the type of access your derived class has to this item.";
                }

                auto child_type = elem_child.attribute("type").as_cview("wxString");

                // Note: empty tags don't contain any child
                std::string child_value;
                auto lastChild = elem_child.last_child();
                if (lastChild && !lastChild.text().empty())
                {
                    child_value = lastChild.text().get();
                }
                child.m_def_value = child_value;

                // build parent default value
                if (children.size() > 0)
                {
                    def_value += "; ";
                }
                def_value += child_value;

                children.emplace_back(child);

                elem_child = elem_child.next_sibling("child");
            }
        }

        // auto prop_info = std::make_shared<PropertyInfo>(name, ptype, def_value, description, customEditor, children);
        auto prop_info =
            std::make_shared<PropertyInfo>(prop_name, property_type, def_value, description, customEditor, children);
        obj_info->GetPropInfoMap()[name] = prop_info;

        if (property_type == enum_bitlist || property_type == enum_option || property_type == enum_editoption)
        {
            auto& opts = prop_info->GetOptions();
            auto elem_opt = elem_prop.child("option");
            while (elem_opt)
            {
                wxString OptionName = elem_opt.attribute("name").as_string();
                ttlib::cstr help = elem_opt.attribute("help").as_string();
                if (help.empty())
                {
                    if (OptionName == "none")
                        help = "Your derived class has no access to this item.";
                    else if (OptionName == "protected:")
                        help = "Item is added as a protected: class member";
                    else if (OptionName == "public:")
                        help = "Item is added as a public: class member";
                }

                auto& opt = opts.emplace_back();
                opt.name = OptionName;
                opt.help = help.wx_str();

                elem_opt = elem_opt.next_sibling("option");
            }
        }

        elem_prop = elem_prop.next_sibling("property");

        // All widgets need to have an access property after their name property. The XML file typically won't supply
        // this, so we add it here.
        if (elem_prop && ttlib::is_sameas(name, txt_var_name) &&
            !elem_prop.attribute("name").as_cview().is_sameas(txt_class_access))
        {
            if (auto type = elem_prop.parent().attribute("type").as_cview();
                type.is_sameas("widget") || type.is_sameas("expanded_widget"))
            {
                category.AddProperty(txt_class_access);
                children.clear();
                prop_info = std::make_shared<PropertyInfo>(
                    Prop::class_access, enum_option,
                    "protected:", "Determines the type of access your derived class has to this item.", "", children);
                obj_info->GetPropInfoMap()[txt_class_access] = prop_info;

                auto& opts = prop_info->GetOptions();

                opts.emplace_back();
                opts[opts.size() - 1].name = "none";
                opts[opts.size() - 1].help = "none: Your derived class has no access to this item.";

                opts.emplace_back();
                opts[opts.size() - 1].name = "protected:";
                opts[opts.size() - 1].help = "protected: Item is added as a protected: class member";

                opts.emplace_back();
                opts[opts.size() - 1].name = "public:";
                opts[opts.size() - 1].help = "public: Item is added as a public: class member";
            }
        }
    }
}

void NodeDeclaration::ParseEvents(pugi::xml_node& elem_obj, NodeCategory& category)
{
    auto elem_category = elem_obj.child("category");
    while (elem_category)
    {
        auto name = elem_category.attribute("name").as_cview();
        auto& new_cat = category.AddCategory(name);

        ParseEvents(elem_category, new_cat);

        elem_category = elem_category.next_sibling("category");
    }

    auto nodeEvent = elem_obj.child("event");
    while (nodeEvent)
    {
        auto evt_name = nodeEvent.attribute("name").as_string();
        category.AddEvent(evt_name);

        auto evt_class = nodeEvent.attribute("class").as_cview("wxEvent");
        auto description = nodeEvent.attribute("help").as_cview();

        m_events[evt_name] = std::make_unique<NodeEventInfo>(evt_name, evt_class, description);

        nodeEvent = nodeEvent.next_sibling("event");
    }
}
