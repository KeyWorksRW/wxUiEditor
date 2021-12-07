/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize NodeCreator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node_creator.h"

#include "bitmaps.h"     // Contains various images handling functions
#include "gen_enums.h"   // Enumerations for generators
#include "gen_enums.h"   // Enumerations for nodes
#include "node.h"        // Node class
#include "node_types.h"  // NodeType -- Class for storing node types and allowable child count
#include "prop_decl.h"   // PropChildDeclaration and PropDeclaration classes

#include "pugixml.hpp"

// The following are interfaces

#include "../xml/interface_xml.xml"
#include "../xml/sizer_child_xml.xml"
#include "../xml/validators_xml.xml"
#include "../xml/window_interfaces_xml.xml"

// clang-format off
inline const char* lst_xml_interfaces[] = {

    // Note that interface_xml must *not* be added to this list! It is loaded as the root document.
    sizer_child_xml,
    validators_xml,
    window_interfaces_xml,

};
// clang-format on

// The following are generators

#include "../xml/aui_xml.xml"
#include "../xml/bars_xml.xml"
#include "../xml/books_xml.xml"
#include "../xml/boxes_xml.xml"
#include "../xml/buttons_xml.xml"
#include "../xml/containers_xml.xml"
#include "../xml/dataview_xml.xml"
#include "../xml/dialogs_xml.xml"
#include "../xml/forms_xml.xml"
#include "../xml/grid_xml.xml"
#include "../xml/images_xml.xml"
#include "../xml/listview_xml.xml"
#include "../xml/menus_xml.xml"
#include "../xml/pickers_xml.xml"
#include "../xml/project_xml.xml"
#include "../xml/propgrid_xml.xml"
#include "../xml/ribbon_xml.xml"
#include "../xml/scintilla_xml.xml"
#include "../xml/sizers_xml.xml"
#include "../xml/std_dlg_btns_xml.xml"
#include "../xml/textctrls_xml.xml"
#include "../xml/toolbars_xml.xml"
#include "../xml/trees_xml.xml"
#include "../xml/widgets_xml.xml"
#include "../xml/wizard_xml.xml"

// clang-format off
inline const char* lst_xml_generators[] = {

    aui_xml,
    bars_xml,
    books_xml,
    boxes_xml,
    buttons_xml,
    containers_xml,
    dataview_xml,
    dialogs_xml,
    forms_xml,
    grid_xml,
    images_xml,
    listview_xml,
    menus_xml,
    pickers_xml,
    project_xml,
    propgrid_xml,
    ribbon_xml,
    scintilla_xml,
    sizers_xml,
    std_dlg_btns_xml,
    textctrls_xml,
    toolbars_xml,
    trees_xml,
    widgets_xml,
    wizard_xml,

};

// var_names for these generators will default to "none" for class access
inline const GenName lst_no_class_access[] = {

    gen_BookPage,
    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,
    gen_TextSizer,
    gen_VerticalBoxSizer,
    gen_separator,
    gen_submenu,
    gen_tool,
    gen_wxBoxSizer,
    gen_wxFlexGridSizer,
    gen_wxGridBagSizer,
    gen_wxGridSizer,
    gen_wxMenuItem,
    gen_wxPanel,
    gen_wxRibbonButtonBar,
    gen_wxRibbonGallery,
    gen_wxRibbonPage,
    gen_wxRibbonPanel,
    gen_wxRibbonToolBar,
    gen_wxStaticBitmap,
    gen_wxStaticBoxSizer,
    gen_wxStaticLine,
    gen_wxStdDialogButtonSizer,
    gen_wxWizardPageSimple,
    gen_wxWrapSizer,

};

// clang-format on

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

    { type_aui_toolbar, type_aui_tool, infinite },
    // { type_aui_toolbar, type_widget, infinite },

    // Books

    { type_bookpage, type_gbsizer, one },
    { type_bookpage, type_sizer, one },
    { type_bookpage, type_bookpage, infinite },  // only valid when grandparent is a wxTreebook

    { type_page, type_auinotebook, one },
    { type_page, type_choicebook, one },
    { type_page, type_container, one },
    { type_page, type_dataviewctrl, one },
    { type_page, type_dataviewlistctrl, one },
    { type_page, type_dataviewtreectrl, one },
    { type_page, type_listbook, one },
    { type_page, type_notebook, one },
    { type_page, type_propgrid, one },
    { type_page, type_propgridman, one },
    { type_page, type_ribbonbar, one },
    { type_page, type_simplebook, one },
    { type_page, type_splitter, one },
    { type_page, type_treelistctrl, one },
    { type_page, type_widget, one },

    { type_choicebook, type_bookpage, infinite },
    { type_choicebook, type_page, infinite },
    { type_choicebook, type_widget, infinite },  // The only book that allows adding a widget
    { type_listbook, type_bookpage, infinite },
    { type_listbook, type_page, infinite },
    { type_notebook, type_bookpage, infinite },
    { type_notebook, type_page, infinite },
    { type_simplebook, type_bookpage, infinite },
    { type_simplebook, type_page, infinite },

    // Menus

    { type_menu, type_menuitem, infinite },
    { type_menu, type_submenu, infinite },
    { type_menubar, type_menu, infinite },

    { type_submenu, type_menuitem, infinite },
    { type_submenu, type_submenu, infinite },

    { type_popup_menu, type_menuitem, infinite },
    { type_popup_menu, type_submenu, infinite },

    { type_ctx_menu, type_menuitem, infinite },
    { type_ctx_menu, type_submenu, infinite },

    // Forms

    { type_frame_form, type_gbsizer, one },
    { type_frame_form, type_sizer, one },

    { type_frame_form, type_container, infinite },
    { type_frame_form, type_splitter, infinite },

    { type_frame_form, type_statusbar, one },
    { type_frame_form, type_toolbar, one },
    { type_frame_form, type_menubar, one },
    { type_frame_form, type_ctx_menu, one },

    { type_frame_form, type_choicebook, infinite },
    { type_frame_form, type_listbook, infinite },
    { type_frame_form, type_simplebook, infinite },
    { type_frame_form, type_notebook, infinite },

    { type_form, type_ctx_menu, one },
    { type_form, type_gbsizer, one },
    { type_form, type_sizer, one },

    { type_menubar_form, type_menu, infinite },
    { type_ribbonbar_form, type_ribbonpage, infinite },
    { type_toolbar_form, type_tool, infinite },
    { type_toolbar_form, type_widget, infinite },

    { type_images, type_embed_image, infinite },
    { type_wizard, type_wizardpagesimple, infinite },

    { type_project, type_form, infinite },
    { type_project, type_frame_form, infinite },
    { type_project, type_images, one },
    { type_project, type_menubar_form, infinite },
    { type_project, type_popup_menu, infinite },
    { type_project, type_ribbonbar_form, infinite },
    { type_project, type_toolbar_form, infinite },
    { type_project, type_wizard, infinite },

    // Containers

    { type_container, type_gbsizer, one },
    { type_container, type_sizer, one },

    // DataView

    { type_dataviewctrl, type_dataviewcolumn, infinite },
    { type_dataviewlistctrl, type_dataviewlistcolumn, infinite },

    { type_propgrid, type_propgriditem, infinite },
    { type_propgriditem, type_propgridpage, infinite },
    { type_propgridman, type_propgridpage, infinite },
    { type_propgridpage, type_propgriditem, infinite },

    // Ribbon bar

    { type_ribbonbar, type_ribbonpage, infinite },
    { type_ribbonbuttonbar, type_ribbonbutton, infinite },
    { type_ribbongallery, type_ribbongalleryitem, infinite },
    { type_ribbonpage, type_ribbonpanel, infinite },
    { type_ribbonpanel, type_ribbonbuttonbar, one },
    { type_ribbonpanel, type_ribbongallery, one },
    { type_ribbonpanel, type_ribbontoolbar, one },
    { type_ribbonpanel, type_sizer, one },
    { type_ribbonpanel, type_gbsizer, one },
    { type_ribbontoolbar, type_ribbontool, infinite },
    { type_ribbontoolbar, type_ribbontool, infinite },

    // Sizers

    { type_sizer, type_auinotebook, infinite },
    // { type_sizer, type_aui_toolbar, infinite },
    { type_sizer, type_choicebook, infinite },
    { type_sizer, type_container, infinite },
    { type_sizer, type_dataviewctrl, infinite },
    { type_sizer, type_dataviewlistctrl, infinite },
    { type_sizer, type_dataviewtreectrl, infinite },
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

    // Misc

    { type_splitter, type_container, two },

    { type_toolbar, type_tool, infinite },
    { type_toolbar, type_widget, infinite },

    { type_treelistctrl, type_treelistctrlcolumn, infinite },

    { type_wizardpagesimple, type_gbsizer, one },
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
        pugi::xml_document interface_doc;
        m_pdoc_interface = &interface_doc;

        // We start by loading the main interface_xml string, then we append the nodes from all the other interface strings.
        auto result = interface_doc.load_string(interface_xml);
        if (!result)
        {
            FAIL_MSG("xml/interface_xml.xml is corrupted!");
            throw std::runtime_error("Internal XML file is corrupted.");
        }
        auto interface_root = interface_doc.child("GeneratorDefinitions");

        for (auto& iter: lst_xml_interfaces)
        {
            pugi::xml_document sub_interface;
            result = sub_interface.load_string(iter);
            if (!result)
            {
                FAIL_MSG("XML file is corrupted!");
                throw std::runtime_error("Internal XML file is corrupted.");
            }
            auto root = sub_interface.child("GeneratorDefinitions");
            for (auto& child_node: root)
            {
                interface_root.append_copy(child_node);
            }
        }

        // Now parse the completed m_pdoc_interface document
        ParseGeneratorFile(ttlib::emptystring);

        for (auto& iter: lst_xml_generators)
        {
            ParseGeneratorFile(iter);
        }

        m_interfaces.clear();
        m_pdoc_interface = nullptr;
    }

    InitGenerators();

    for (auto& iter: fb_ImportTypes)
    {
        m_setOldHostTypes.emplace(iter);
    }
}

// The xml_data parameter is the char* pointer to the XML data. It will be empty when processing the interface document.
void NodeCreator::ParseGeneratorFile(ttlib::cview xml_data)
{
    // All but one of the possible files will use the doc file, so we create it even if it gets ignored because this is an
    // interface file
    pugi::xml_document doc;
    pugi::xml_node root;

    if (xml_data.empty())
    {
        root = m_pdoc_interface->child("GeneratorDefinitions");
    }
    else
    {
        auto result = doc.load_string(xml_data);
        if (!result)
        {
            FAIL_MSG("XML file is corrupted!");
            throw std::runtime_error("Internal XML file is corrupted.");
        }
        root = doc.child("GeneratorDefinitions");
    }

    if (!root)
    {
        FAIL_MSG("GeneratorDefinitions not found in XML file.");
        throw std::runtime_error("Internal XML file is corrupted.");
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

        if (xml_data.empty())
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
            if (image.GetWidth() != GenImageSize || image.GetHeight() != GenImageSize)
            {
                MSG_INFO(ttlib::cstr() << image_name << " width: " << image.GetWidth() << "height: " << image.GetHeight());
                declaration->SetImage(image.Scale(GenImageSize, GenImageSize));
            }
            else
            {
                declaration->SetImage(image);
            }
        }
        else
        {
            declaration->SetImage(GetInternalImage("unknown").Scale(GenImageSize, GenImageSize));
        }

        // ParseProperties(generator, declaration.get(), declaration->GetCategory());
        ParseProperties(generator, declaration, declaration->GetCategory());

        declaration->ParseEvents(generator, declaration->GetCategory());

        generator = generator.next_sibling("gen");
    }

    // Interface processing doesn't have a xml_data
    if (xml_data.size())
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

                    inheritedProperty = elem_base.child("hide");
                    while (inheritedProperty)
                    {
                        auto lookup_name = rmap_PropNames.find(inheritedProperty.attribute("name").as_string());
                        if (lookup_name == rmap_PropNames.end())
                        {
                            MSG_ERROR(ttlib::cstr("Unrecognized inherited property name -- ")
                                      << inheritedProperty.attribute("name").as_string());
                            continue;
                        }
                        class_info->HideProperty(lookup_name->second);
                        inheritedProperty = inheritedProperty.next_sibling("hide");
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

        auto prop_info = std::make_shared<PropDeclaration>(prop_name, property_type, def_value, description, customEditor);
        obj_info->GetPropInfoMap()[name] = prop_info;

        if (property_type == type_bitlist || property_type == type_option || property_type == type_editoption)
        {
            auto& opts = prop_info->GetOptions();
            auto elem_opt = elem_prop.child("option");
            while (elem_opt)
            {
                auto& opt = opts.emplace_back();
                opt.name = elem_opt.attribute("name").as_string();
                opt.help = elem_opt.attribute("help").as_string();

                elem_opt = elem_opt.next_sibling("option");
            }
        }

        // Any time there is a var_name property, it needs to be followed by a var_comment and class_access property. Rather
        // than add this to all the XML generator specifications, we simply insert it here if it doesn't exist.

        if (ttlib::is_sameas(name, map_PropNames[prop_var_name]))
        {
            category.AddProperty(prop_var_comment);
            prop_info = std::make_shared<PropDeclaration>(prop_var_comment, type_string_edit_single, tt_empty_cstr,
                                                          "Comment to add to the variable name in the generated header file "
                                                          "if the class access is set to protected of public",
                                                          "");
            obj_info->GetPropInfoMap()[map_PropNames[prop_var_comment]] = prop_info;

            category.AddProperty(prop_class_access);
            ttlib::cstr access("protected:");

            // Most widgets will default to protected: as their class access. Those in the lst_no_class_access array should
            // have "none" as the default class access.

            for (auto generator: lst_no_class_access)
            {
                if (obj_info->isGen(generator))
                {
                    access = "none";
                    break;
                }
            }

            prop_info =
                std::make_shared<PropDeclaration>(prop_class_access, type_option, access,
                                                  "Determines the type of access your derived class has to this item.", "");
            obj_info->GetPropInfoMap()[map_PropNames[prop_class_access]] = prop_info;

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

        elem_prop = elem_prop.next_sibling("property");
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
