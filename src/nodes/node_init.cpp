/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize NodeCreator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "node_creator.h"

#include "bitmaps.h"     // Contains various images handling functions
#include "gen_enums.h"   // Enumerations for generators
#include "gen_enums.h"   // Enumerations for nodes
#include "node.h"        // Node class
#include "node_types.h"  // NodeType -- Class for storing node types and allowable child count
#include "prop_decl.h"   // PropChildDeclaration and PropDeclaration classes

#include "../pugixml/pugixml.hpp"

bool LoadInternalXmlDocFile(ttlib::cview file, pugi::xml_document* doc);

using namespace child_count;
using namespace GenEnum;

struct ParentChild
{
    GenType parent;
    GenType child;

    int_t max_children;
};

// A child node can only be created if it is listed below as valid for the current parent.
static const ParentChild lstParentChild[] = {

    { type_bookpage, type_gbsizer, one },
    { type_bookpage, type_sizer, one },

    { type_choicebook, type_bookpage, infinite },
    { type_choicebook, type_widget, infinite },

    { type_container, type_gbsizer, one },
    { type_container, type_menu, one },
    { type_container, type_sizer, one },

    { type_dataviewctrl, type_dataviewcolumn, infinite },
    { type_dataviewlistctrl, type_dataviewlistcolumn, infinite },

    { type_form, type_sizer, one },
    { type_form, type_gbsizer, one },

    { type_listbook, type_bookpage, infinite },

    { type_menu, type_menuitem, infinite },
    { type_menu, type_submenu, infinite },
    { type_menubar, type_menu, infinite },
    { type_menubar_form, type_menu, infinite },

    { type_notebook, type_bookpage, infinite },

    { type_project, type_form, infinite },
    { type_project, type_menubar_form, infinite },
    { type_project, type_toolbar_form, infinite },
    { type_project, type_wizard, infinite },

    { type_propgrid, type_menu, one },
    { type_propgrid, type_propgriditem, infinite },
    { type_propgriditem, type_propgridpage, infinite },
    { type_propgridman, type_menu, one },
    { type_propgridman, type_propgridpage, infinite },
    { type_propgridpage, type_propgriditem, infinite },

    { type_ribbonbar, type_ribbonpage, infinite },
    { type_ribbonbuttonbar, type_ribbonbutton, infinite },
    { type_ribbongallery, type_ribbongalleryitem, infinite },
    { type_ribbonpage, type_ribbonpanel, infinite },
    { type_ribbonpanel, type_ribbonbuttonbar, one },
    { type_ribbonpanel, type_ribbongallery, one },
    { type_ribbonpanel, type_ribbontoolbar, one },
    { type_ribbontoolbar, type_ribbontool, infinite },

    { type_simplebook, type_bookpage, infinite },

    { type_sizer, type_auinotebook, infinite },
    { type_sizer, type_aui_toolbar, infinite },
    { type_sizer, type_choicebook, infinite },
    { type_sizer, type_container, infinite },
    { type_sizer, type_dataviewctrl, infinite },
    { type_sizer, type_dataviewlistctrl, infinite },
    { type_sizer, type_dataviewtreectrl, infinite },
    { type_sizer, type_expanded_widget, infinite },
    { type_sizer, type_gbsizer, infinite },
    { type_sizer, type_listbook, infinite },
    { type_sizer, type_notebook, infinite },
    { type_sizer, type_propgrid, infinite },
    { type_sizer, type_propgridman, infinite },
    { type_sizer, type_ribbonbar, infinite },
    { type_sizer, type_simplebook, infinite },
    { type_sizer, type_sizer, infinite },
    { type_sizer, type_splitter, infinite },
    { type_sizer, type_toolbar, infinite },
    { type_sizer, type_treelistctrl, infinite },
    { type_sizer, type_widget, infinite },

    { type_splitter, type_container, two },

    { type_submenu, type_menuitem, infinite },
    { type_submenu, type_submenu, infinite },

    { type_tool, type_menu, one },

    { type_toolbar, type_expanded_widget, infinite },
    { type_toolbar, type_tool, infinite },
    { type_toolbar, type_widget, infinite },
    { type_toolbar_form, type_expanded_widget, infinite },
    { type_toolbar_form, type_tool, infinite },
    { type_toolbar_form, type_widget, infinite },

    { type_aui_toolbar, type_aui_tool, infinite },
    { type_aui_toolbar, type_widget, infinite },

    { type_treelistctrl, type_expanded_widget, infinite },
    { type_treelistctrl, type_menu, one },
    { type_treelistctrl, type_treelistctrlcolumn, infinite },

    { type_widget, type_expanded_widget, one },
    { type_widget, type_menu, one },

    { type_wizard, type_menu, one },
    { type_wizard, type_wizardpagesimple, infinite },

    { type_wizardpagesimple, type_gbsizer, one },
    { type_wizardpagesimple, type_menu, one },
    { type_wizardpagesimple, type_sizer, one },

};

// These are types used to convert wxFormBuilder projects
static constexpr const char* fb_ImportTypes[] = {

    "sizeritem",
    "gbsizeritem",
    "splitteritem",

    "oldbookpage",

};

void NodeCreator::Initialize()
{
    for (auto& iter: GenEnum::map_PropTypes)
    {
        GenEnum::rmap_PropTypes[iter.second] = iter.first;
    }

    for (auto& iter: GenEnum::map_PropNames)
    {
        GenEnum::rmap_PropNames[iter.second] = iter.first;
    }

    for (auto& iter: GenEnum::map_GenTypes)
    {
        GenEnum::rmap_GenTypes[iter.second] = iter.first;
    }

    for (auto& iter: GenEnum::map_GenNames)
    {
        GenEnum::rmap_GenNames[iter.second] = iter.first;
    }

    for (auto& iter: map_GenTypes)
    {
        m_a_node_types[static_cast<size_t>(iter.first)].Create(iter.first);
    }

    for (auto& iter: lstParentChild)
    {
        GetNodeType(iter.parent)->AddChild(iter.child, iter.max_children);
        if (iter.parent == type_sizer)
            GetNodeType(type_gbsizer)->AddChild(iter.child, iter.max_children);
    }

    {
        pugi::xml_document doc;
        m_pdoc_interface = &doc;

        // This *MUST* be the first file processed so that m_interfaces is initialized
        m_is_interface = true;
        ParseGeneratorFile("interface");
        m_is_interface = false;

        ParseGeneratorFile("aui");
        ParseGeneratorFile("bars");
        ParseGeneratorFile("forms");
        ParseGeneratorFile("containers");
        ParseGeneratorFile("sizers");
        ParseGeneratorFile("widgets");

        m_interfaces.clear();
        m_pdoc_interface = nullptr;
    }

    InitGenerators();

    for (auto& iter: fb_ImportTypes)
    {
        m_setOldHostTypes.emplace(iter);
    }
}

void NodeCreator::ParseGeneratorFile(ttlib::cview name)
{
    // All but one of the possible files will use the doc file, so we create it even if it gets ignored because this is an
    // interface file
    pugi::xml_document doc;
    pugi::xml_node root;

    if (m_is_interface)
    {
        if (!LoadInternalXmlDocFile(name, m_pdoc_interface))
        {
            // In _DEBUG builds, an assertion will have already been generated.
            return;
        }

        root = m_pdoc_interface->child("GeneratorDefinitions");
    }
    else
    {
        if (!LoadInternalXmlDocFile(name, &doc))
        {
            // In _DEBUG builds, an assertion will have already been generated.
            return;
        }

        root = doc.child("GeneratorDefinitions");
    }

    if (!root)
    {
        FAIL_MSG(ttlib::cstr("Cannot locate group in the name ") << name);
        return;
    }

    auto generator = root.child("gen");
    while (generator)
    {
        auto class_name = generator.attribute("class").as_string();
#if defined(_DEBUG)
        if (rmap_GenNames.find(class_name) == rmap_GenNames.end())
        {
            MSG_WARNING(ttlib::cstr("Unrecognized class name -- ") << class_name);
        }
#endif  // _DEBUG

        auto type = generator.attribute("type").as_cview();
#if defined(_DEBUG)
        if (rmap_GenTypes.find(type.c_str()) == rmap_GenTypes.end())
        {
            MSG_WARNING(ttlib::cstr("Unrecognized class type -- ") << type);
        }
#endif  // _DEBUG

        if (m_is_interface)
        {
            m_interfaces[class_name] = generator;
        }

        auto declaration = new NodeDeclaration(class_name, GetNodeType(rmap_GenTypes[type.c_str()]));
        m_a_declarations[declaration->gen_name()] = declaration;

        if (auto flags = generator.attribute("flags").as_cview(); flags.size())
        {
            declaration->SetGeneratorFlags(flags);
        }

        auto image_name = generator.attribute("image").as_cview();
        if (image_name.size())
        {
            auto image = GetInternalImage(image_name);
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
            declaration->SetImage(GetInternalImage("unknown").Scale(CompImgSize, CompImgSize));
        }

        // ParseProperties(generator, declaration.get(), declaration->GetCategory());
        ParseProperties(generator, declaration, declaration->GetCategory());

        declaration->ParseEvents(generator, declaration->GetCategory());

        generator = generator.next_sibling("gen");
    }

    if (!name.is_sameas("interface"))
    {
        auto elem_obj = root.child("gen");
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
                    class_info->AddBaseClass(base_info);

                    auto inheritedProperty = elem_base.child("property");
                    while (inheritedProperty)
                    {
                        auto lookup_name = rmap_PropNames.find(inheritedProperty.attribute("name").as_string());
                        if (lookup_name == rmap_PropNames.end())
                        {
                            MSG_ERROR(ttlib::cstr("Unrecognized inherited property name -- ")
                                      << inheritedProperty.attribute("name").as_string());
                            continue;
                        }
                        class_info->SetOverRideDefValue(lookup_name->second, inheritedProperty.text().as_cview());
                        inheritedProperty = inheritedProperty.next_sibling("property");
                    }
                }
                elem_base = elem_base.next_sibling("inherits");
            }

            elem_obj = elem_obj.next_sibling("gen");
        }
    }
}

void NodeCreator::ParseProperties(pugi::xml_node& elem_obj, NodeDeclaration* obj_info, NodeCategory& category)
{
    auto elem_category = elem_obj.child("category");
    while (elem_category)
    {
        auto name = elem_category.attribute("name").as_cview();
        auto& new_cat = category.AddCategory(name);

        if (auto base_name = elem_category.attribute("base_name").value(); *base_name)
        {
            if (auto node = m_interfaces.find(base_name); node != m_interfaces.end())
            {
                ParseProperties(node->second, obj_info, new_cat);
            }
        }
        else
        {
            ParseProperties(elem_category, obj_info, new_cat);
        }

        elem_category = elem_category.next_sibling("category");
    }

    auto elem_prop = elem_obj.child("property");
    while (elem_prop)
    {
        auto name = elem_prop.attribute("name").as_string();
        GenEnum::PropName prop_name;
        auto lookup_name = rmap_PropNames.find(name);
        if (lookup_name == rmap_PropNames.end())
        {
            MSG_ERROR(ttlib::cstr("Unrecognized property name -- ") << name);
            elem_prop = elem_prop.next_sibling("property");
            continue;
        }
        prop_name = lookup_name->second;

        category.AddProperty(prop_name);

        auto description = elem_prop.attribute("help").as_cview();
        auto customEditor = elem_prop.attribute("editor").as_cview();

        auto prop_type = elem_prop.attribute("type").as_cview();

        GenEnum::PropType property_type;
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

        std::vector<PropChildDeclaration> children;
        if (property_type == type_parent)
        {
            // If the property is a parent, then get the children
            def_value.clear();
            auto elem_child = elem_prop.child("child");
            while (elem_child)
            {
                PropChildDeclaration child;

                child.m_help = elem_child.attribute("help").as_string();
                if (child.m_help.empty())
                {
                    if (child.isProp(prop_class_access))
                        child.m_help = "Determines the type of access your derived class has to this item.";
                }

                // BUGBUG: [KeyWorks - 04-09-2021] We're setting child.m_name which but not any of the NodeEnum members.

                // auto child_type = elem_child.attribute("type").as_cview("wxString");
                // child.m_type = ParsePropertyType(child_type);

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

        // auto prop_info = std::make_shared<PropDeclaration>(name, ptype, def_value, description, customEditor, children);
        auto prop_info =
            std::make_shared<PropDeclaration>(prop_name, property_type, def_value, description, customEditor, children);
        obj_info->GetPropInfoMap()[name] = prop_info;

        if (property_type == type_bitlist || property_type == type_option || property_type == type_editoption)
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
                category.AddProperty(prop_class_access);
                children.clear();
                prop_info = std::make_shared<PropDeclaration>(
                    prop_class_access, type_option,
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
        // Only create the category if there is at least one event.
        if (elem_category.child("event"))
        {
            auto name = elem_category.attribute("name").as_cview();
            auto& new_cat = category.AddCategory(name);

            ParseEvents(elem_category, new_cat);
        }
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
