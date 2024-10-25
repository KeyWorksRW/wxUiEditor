/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize NodeCreator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <functional>

#include <frozen/set.h>

#include "node_creator.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "bitmaps.h"         // Contains various images handling functions
#include "gen_enums.h"       // Enumerations for generators
#include "mainapp.h"         // App -- Main application class
#include "node.h"            // Node class
#include "node_types.h"      // NodeType -- Class for storing node types and allowable child count
#include "prop_decl.h"       // PropChildDeclaration and PropDeclaration classes
#include "wxue_data.h"       // Generated wxue_data strings and functions

#include "pugixml.hpp"

// clang-format off

const static std::function<std::string()> functionArray[] = {
    wxue_data::get_bars,
    wxue_data::get_boxes,
    wxue_data::get_buttons,
    wxue_data::get_containers,
    wxue_data::get_data_ctrls,
    wxue_data::get_forms,
    wxue_data::get_mdi,
    wxue_data::get_pickers,
    wxue_data::get_project,
    wxue_data::get_sizers,
    wxue_data::get_text_ctrls,
    wxue_data::get_widgets,
};

// var_names for these generators will default to "none" for class access
// inline const GenName set_no_class_access[] = {
constexpr auto set_no_class_access = frozen::make_set<GenName>({

    gen_BookPage,
    gen_CloseButton,
    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,
    gen_TextSizer,
    gen_VerticalBoxSizer,
    gen_auitool,
    gen_auitool_label,
    gen_separator,
    gen_submenu,
    gen_tool,
    gen_tool_dropdown,
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

});

// clang-format on

using namespace child_count;
using namespace GenEnum;

struct ParentChild
{
    GenType parent;
    GenType child;

    ptrdiff_t max_children;
};

// A child node can only be created if it is listed below as valid for the current parent.
static const ParentChild lstParentChild[] = {

    // Books

    { type_bookpage, type_gbsizer, one },
    { type_bookpage, type_sizer, one },
    { type_bookpage, type_bookpage, infinite },  // only valid when grandparent is a wxTreebook
    { type_bookpage, type_widget, infinite },

    { type_page, type_auinotebook, one },
    { type_page, type_choicebook, one },
    { type_page, type_container, one },
    { type_page, type_dataviewctrl, one },
    { type_page, type_dataviewlistctrl, one },
    { type_page, type_dataviewtreectrl, one },
    { type_page, type_listbook, one },
    { type_page, type_notebook, one },
    { type_page, type_panel, one },
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
    { type_frame_form, type_panel, infinite },
    { type_frame_form, type_splitter, infinite },

    { type_frame_form, type_statusbar, one },
    { type_frame_form, type_toolbar, one },
    { type_frame_form, type_aui_toolbar, one },
    { type_frame_form, type_menubar, one },
    { type_frame_form, type_ctx_menu, one },
    { type_frame_form, type_timer, infinite },

    { type_frame_form, type_choicebook, infinite },
    { type_frame_form, type_listbook, infinite },
    { type_frame_form, type_simplebook, infinite },
    { type_frame_form, type_notebook, infinite },
    { type_frame_form, type_widget, infinite },

    { type_form, type_ctx_menu, one },
    { type_form, type_gbsizer, one },
    { type_form, type_sizer, one },
    { type_form, type_timer, infinite },

    { type_panel_form, type_gbsizer, one },
    { type_panel_form, type_sizer, one },

    { type_panel_form, type_ctx_menu, one },
    { type_panel_form, type_timer, infinite },

    { type_panel_form, type_aui_toolbar, infinite },
    { type_panel_form, type_panel_form, infinite },
    { type_panel_form, type_splitter, infinite },
    { type_panel_form, type_toolbar, infinite },

    { type_panel_form, type_auinotebook, infinite },
    { type_panel_form, type_choicebook, infinite },
    { type_panel_form, type_container, infinite },
    { type_panel_form, type_dataviewctrl, infinite },
    { type_panel_form, type_dataviewlistctrl, infinite },
    { type_panel_form, type_dataviewtreectrl, infinite },
    { type_panel_form, type_listbook, infinite },
    { type_panel_form, type_notebook, infinite },
    { type_panel_form, type_propgrid, infinite },
    { type_panel_form, type_propgridman, infinite },
    { type_panel_form, type_ribbonbar, infinite },
    { type_panel_form, type_simplebook, infinite },
    { type_panel_form, type_splitter, infinite },
    { type_panel_form, type_treelistctrl, infinite },

    { type_panel_form, type_widget, infinite },

    { type_propsheetform, type_bookpage, infinite },

    { type_menubar_form, type_menu, infinite },
    { type_ribbonbar_form, type_ribbonpage, infinite },
    { type_toolbar_form, type_tool, infinite },
    { type_toolbar_form, type_tool_separator, infinite },
    { type_toolbar_form, type_widget, infinite },
    { type_aui_toolbar_form, type_aui_tool, infinite },
    { type_aui_toolbar_form, type_tool_separator, infinite },
    { type_aui_toolbar_form, type_widget, infinite },

    { type_data_list, type_data_string, infinite },
    { type_data_list, type_data_folder, infinite },
    { type_data_folder, type_data_string, infinite },
    { type_images, type_embed_image, infinite },
    { type_wizard, type_wizardpagesimple, infinite },

    { type_project, type_data_list, one },
    { type_project, type_form, infinite },
    { type_project, type_folder, infinite },
    { type_project, type_frame_form, infinite },
    { type_project, type_images, one },
    { type_project, type_menubar_form, infinite },
    { type_project, type_panel_form, infinite },
    { type_project, type_popup_menu, infinite },
    { type_project, type_ribbonbar_form, infinite },
    { type_project, type_toolbar_form, infinite },
    { type_project, type_aui_toolbar_form, infinite },
    { type_project, type_wizard, infinite },
    { type_project, type_propsheetform, infinite },

    // Folders and sub-folders

    { type_folder, type_sub_folder, infinite },
    { type_folder, type_form, infinite },
    { type_folder, type_frame_form, infinite },
    { type_folder, type_menubar_form, infinite },
    { type_folder, type_panel_form, infinite },
    { type_folder, type_popup_menu, infinite },
    { type_folder, type_ribbonbar_form, infinite },
    { type_folder, type_toolbar_form, infinite },
    { type_folder, type_aui_toolbar_form, infinite },
    { type_folder, type_wizard, infinite },
    { type_folder, type_DocViewApp, one },
    { type_folder, type_wx_document, infinite },
    { type_folder, type_wx_view, infinite },
    { type_folder, type_propsheetform, infinite },

    { type_sub_folder, type_form, infinite },
    { type_sub_folder, type_sub_folder, infinite },
    { type_sub_folder, type_frame_form, infinite },
    { type_sub_folder, type_menubar_form, infinite },
    { type_sub_folder, type_panel_form, infinite },
    { type_sub_folder, type_popup_menu, infinite },
    { type_sub_folder, type_ribbonbar_form, infinite },
    { type_sub_folder, type_toolbar_form, infinite },
    { type_sub_folder, type_aui_toolbar_form, infinite },
    { type_sub_folder, type_wizard, infinite },
    { type_sub_folder, type_propsheetform, infinite },

    // Containers

    { type_container, type_gbsizer, one },
    { type_container, type_sizer, one },

    { type_panel, type_gbsizer, one },
    { type_panel, type_sizer, one },

    { type_panel, type_ctx_menu, one },
    { type_panel, type_timer, infinite },

    { type_panel, type_aui_toolbar, infinite },
    { type_panel, type_panel, infinite },
    { type_panel, type_splitter, infinite },
    { type_panel, type_toolbar, infinite },

    { type_panel, type_auinotebook, infinite },
    { type_panel, type_choicebook, infinite },
    { type_panel, type_container, infinite },
    { type_panel, type_dataviewctrl, infinite },
    { type_panel, type_dataviewlistctrl, infinite },
    { type_panel, type_dataviewtreectrl, infinite },
    { type_panel, type_listbook, infinite },
    { type_panel, type_notebook, infinite },
    { type_panel, type_propgrid, infinite },
    { type_panel, type_propgridman, infinite },
    { type_panel, type_ribbonbar, infinite },
    { type_panel, type_simplebook, infinite },
    { type_panel, type_splitter, infinite },
    { type_panel, type_treelistctrl, infinite },

    { type_panel, type_widget, infinite },

    // DataView

    { type_dataviewctrl, type_dataviewcolumn, infinite },
    { type_dataviewlistctrl, type_dataviewlistcolumn, infinite },

    { type_propgrid, type_propgriditem, infinite },
    { type_propgrid, type_propgrid_category, infinite },
    { type_propgrid_category, type_propgriditem, infinite },
    { type_propgriditem, type_propgridpage, infinite },
    { type_propgridman, type_propgridpage, infinite },
    { type_propgridpage, type_propgrid_category, infinite },
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

    { type_sizer, type_aui_toolbar, infinite },
    { type_sizer, type_auinotebook, infinite },
    { type_sizer, type_choicebook, infinite },
    { type_sizer, type_container, infinite },
    { type_sizer, type_dataviewctrl, infinite },
    { type_sizer, type_dataviewlistctrl, infinite },
    { type_sizer, type_dataviewtreectrl, infinite },
    { type_sizer, type_gbsizer, infinite },
    { type_sizer, type_listbook, infinite },
    { type_sizer, type_notebook, infinite },
    { type_sizer, type_panel, infinite },
    { type_sizer, type_propgrid, infinite },
    { type_sizer, type_propgridman, infinite },
    { type_sizer, type_ribbonbar, infinite },
    { type_sizer, type_simplebook, infinite },
    { type_sizer, type_sizer, infinite },
    { type_sizer, type_splitter, infinite },
    { type_sizer, type_staticbox, infinite },
    { type_sizer, type_toolbar, infinite },
    { type_sizer, type_treelistctrl, infinite },
    { type_sizer, type_widget, infinite },

    // Toolbars

    { type_aui_toolbar, type_aui_tool, infinite },
    { type_aui_toolbar, type_tool_separator, infinite },
    { type_aui_toolbar, type_widget, infinite },
    // type_tool_dropdown only works in wxToolBar -- wxAuiToolBar requires the caller to create the menu on demand

    { type_toolbar, type_tool, infinite },
    { type_toolbar, type_tool_dropdown, infinite },
    { type_toolbar, type_tool_separator, infinite },
    { type_toolbar, type_widget, infinite },
    { type_tool_dropdown, type_menuitem, infinite },

    // wxStaticBox

    { type_staticbox, type_widget, infinite },
    { type_staticbox, type_sizer, infinite },
    { type_staticbox, type_gbsizer, infinite },

    // Misc

    { type_splitter, type_auinotebook, two },
    { type_splitter, type_choicebook, two },
    { type_splitter, type_container, two },
    { type_splitter, type_dataviewctrl, two },
    { type_splitter, type_dataviewlistctrl, two },
    { type_splitter, type_dataviewtreectrl, two },
    { type_splitter, type_listbook, two },
    { type_splitter, type_notebook, two },
    { type_splitter, type_panel, two },
    { type_splitter, type_propgrid, two },
    { type_splitter, type_propgridman, two },
    { type_splitter, type_simplebook, two },
    { type_splitter, type_splitter, two },
    { type_splitter, type_treelistctrl, two },
    { type_splitter, type_widget, two },

    { type_treelistctrl, type_treelistctrlcolumn, infinite },

    { type_wx_document, type_mdi_menubar, one },  // default menu bar when no document is loaded
    { type_wx_document, type_doc_menubar, one },  // menu bar when a document is loaded
    { type_mdi_menubar, type_menu, infinite },
    { type_doc_menubar, type_menu, infinite },

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
    for (auto& iter: GenEnum::map_PropNames)
    {
        GenEnum::rmap_PropNames[iter.second] = iter.first;
    }

    for (auto& iter: map_PropMacros)
    {
        map_MacroProps[iter.second] = iter.first;
    }

    for (auto& iter: GenEnum::map_GenNames)
    {
        rmap_GenNames[iter.second] = iter.first;
    }

    for (auto& iter: map_GenTypes)
    {
        m_a_node_types[static_cast<size_t>(iter.first)].Create(iter.first);
    }

    for (auto& iter: lstParentChild)
    {
        getNodeType(iter.parent)->addChild(iter.child, iter.max_children);
        if (iter.parent == type_sizer)
            getNodeType(type_gbsizer)->addChild(iter.child, iter.max_children);
    }

    {
        pugi::xml_document interface_doc;
        m_pdoc_interface = &interface_doc;

        auto result = interface_doc.load_string(wxue_data::get_interfaces().c_str());
        if (!result)
        {
            FAIL_MSG("xml/interfaces.xml is corrupted!");
            throw std::runtime_error("Internal XML file is corrupted.");
        }

        // Now parse the completed m_pdoc_interface document
        parseGeneratorFile("");

        for (auto& iter: functionArray)
        {
            auto xml_data = iter();
            if (xml_data.size())
            {
                parseGeneratorFile(xml_data.c_str());
            }
        }

        m_interfaces.clear();
        m_pdoc_interface = nullptr;
    }

    initGenerators();

    for (auto& iter: fb_ImportTypes)
    {
        m_setOldHostTypes.emplace(iter);
    }
}

// The xml_data parameter is the char* pointer to the XML data. It will be empty when
// processing an interface document.
void NodeCreator::parseGeneratorFile(const char* xml_data)
{
    // All but one of the possible files will use the doc file, so we create it even if it gets
    // ignored because this is an interface file
    pugi::xml_document doc;
    pugi::xml_node root;
    bool is_interface = (xml_data == nullptr || !*xml_data);

    if (!xml_data || !*xml_data)
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
        auto class_name = generator.attribute("class").as_str();
        if (class_name.starts_with("gen_"))
        {
            class_name.erase(0, sizeof("gen_") - 1);
        }
        if (wxGetApp().isTestingMenuEnabled())
        {
            if (is_interface)
            {
                if (!rmap_GenNames.contains(class_name))
                {
                    MSG_WARNING(tt_string("Unrecognized interface name -- ") << class_name);
                }
            }
            else
            {
                if (!rmap_GenNames.contains(class_name))
                {
                    MSG_WARNING(tt_string("Unrecognized class name -- ") << class_name);
                }
            }
        }

        // This code makes it possible to add `enable="internal"` to an XML class/interface to
        // prevent it from being used when not testing.
        if (auto enable = generator.attribute("enable"); enable.as_sview() == "internal")
        {
            if (!wxGetApp().isTestingMenuEnabled())
            {
                // Skip this class if we're not testing
                generator = generator.next_sibling("gen");
                continue;
            }
        }

        GenType type { gen_type_unknown };
        if (is_interface)
        {
            type = type_interface;
        }
        else
        {
            auto type_name = generator.attribute("type").as_view();
#if defined(_DEBUG)
            if (is_interface && type_name != "interface")
            {
                ASSERT_MSG(type_name == "interface", "Don't put a non-interface generation in an interace xml file!");
            }
#endif  // _DEBUG
            for (auto& iter: map_GenTypes)
            {
                if (type_name == iter.second)
                {
                    type = iter.first;
                    break;
                }
            }

#if defined(_DEBUG)
            ASSERT_MSG(type != gen_type_unknown, tt_string("Unrecognized class type -- ") << type_name);
            if (type == gen_type_unknown)
            {
                generator = generator.next_sibling("gen");
                continue;
            }
#endif  // _DEBUG
        }

        if (is_interface)
        {
            m_interfaces[class_name] = generator;
        }

        auto declaration = new NodeDeclaration(class_name, getNodeType(type));
        m_a_declarations[declaration->getGenName()] = declaration;

        if (auto flags = generator.attribute("flags").as_view(); flags.size())
        {
            declaration->SetGeneratorFlags(flags);
        }

        auto image_name = generator.attribute("image").as_view();
        if (image_name.size())
        {
            if (auto bndl_function = GetSvgFunction(image_name); bndl_function)
            {
                declaration->SetBundleFunction(bndl_function);
            }
            else
            {
                auto image = GetInternalImage(image_name);
                if (image.GetWidth() != GenImageSize || image.GetHeight() != GenImageSize)
                {
                    MSG_INFO(tt_string() << image_name << " width: " << image.GetWidth() << "height: " << image.GetHeight());
                    declaration->SetImage(image.Scale(GenImageSize, GenImageSize));
                }
                else
                {
                    declaration->SetImage(image);
                }
            }
        }
        else
        {
            if (auto bndl_function = GetSvgFunction("unknown"); bndl_function)
            {
                declaration->SetBundleFunction(bndl_function);
            }
            else
            {
                declaration->SetImage(GetInternalImage("unknown").Scale(GenImageSize, GenImageSize));
            }
        }

        // parseProperties(generator, declaration.get(), declaration->GetCategory());
        parseProperties(generator, declaration, declaration->GetCategory());

        declaration->ParseEvents(generator, declaration->GetCategory());

        generator = generator.next_sibling("gen");
    }

    // Interface processing doesn't have a xml_data
    if (xml_data && *xml_data)
    {
        auto elem_obj = root.child("gen");
        while (elem_obj)
        {
            auto class_name = elem_obj.attribute("class").as_sview();
            if (class_name.starts_with("gen_"))
            {
                class_name.remove_prefix(sizeof("gen_") - 1);
            }

            auto class_info = getNodeDeclaration(class_name);

            // This can happen if the project file is corrupted, or it it a newer version of the project file
            // that the current version doesn't support.
            if (!class_info)
                break;

            auto elem_base = elem_obj.child("inherits");
            while (elem_base)
            {
                auto base_name = elem_base.attribute("class").as_view();
                if (base_name == "Language Settings")
                {
                    class_info->AddBaseClass(getNodeDeclaration("C++ Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("C++ Header Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("C++ Derived Class Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("wxPython Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("wxRuby Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("wxFortran Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("wxHaskell Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("wxLua Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("wxPerl Settings"));
                    class_info->AddBaseClass(getNodeDeclaration("wxRust Settings"));

                    elem_base = elem_base.next_sibling("inherits");
                    continue;
                }

                // Add a reference to its base class
                auto base_info = getNodeDeclaration(base_name);

                if (class_info && base_info)
                {
                    class_info->AddBaseClass(base_info);

                    auto inheritedProperty = elem_base.child("property");
                    while (inheritedProperty)
                    {
                        auto lookup_name = rmap_PropNames.find(inheritedProperty.attribute("name").as_view());
                        if (lookup_name == rmap_PropNames.end())
                        {
                            MSG_ERROR(tt_string("Unrecognized inherited property name -- ")
                                      << inheritedProperty.attribute("name").as_view());
                            inheritedProperty = inheritedProperty.next_sibling("property");
                            continue;
                        }
                        class_info->SetOverRideDefValue(lookup_name->second, inheritedProperty.text().as_view());
                        inheritedProperty = inheritedProperty.next_sibling("property");
                    }

                    inheritedProperty = elem_base.child("hide");
                    while (inheritedProperty)
                    {
                        auto lookup_name = rmap_PropNames.find(inheritedProperty.attribute("name").as_view());
                        if (lookup_name == rmap_PropNames.end())
                        {
                            MSG_ERROR(tt_string("Unrecognized inherited property name -- ")
                                      << inheritedProperty.attribute("name").as_view());
                            inheritedProperty = inheritedProperty.next_sibling("hide");
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

void NodeCreator::parseProperties(pugi::xml_node& elem_obj, NodeDeclaration* node_declaration, NodeCategory& category)
{
    auto elem_category = elem_obj.child("category");
    while (elem_category)
    {
        auto name = elem_category.attribute("name").as_view();
        auto& new_cat = category.addCategory(name);

        if (auto base_name = elem_category.attribute("base_name").value(); base_name.size())
        {
            if (auto node = m_interfaces.find(base_name); node != m_interfaces.end())
            {
                parseProperties(node->second, node_declaration, new_cat);
            }
        }
        else
        {
            parseProperties(elem_category, node_declaration, new_cat);
        }

        elem_category = elem_category.next_sibling("category");
    }

    auto elem_prop = elem_obj.child("property");
    while (elem_prop)
    {
        auto name = elem_prop.attribute("name").as_str();
        if (name.starts_with("prop_"))
        {
            name.erase(0, sizeof("prop_") - 1);
        }

        GenEnum::PropName prop_name;
        auto lookup_name = rmap_PropNames.find(name);
        if (lookup_name == rmap_PropNames.end())
        {
            MSG_ERROR(tt_string("Unrecognized property name -- ") << name);
            elem_prop = elem_prop.next_sibling("property");
            continue;
        }
        prop_name = lookup_name->second;

        category.addProperty(prop_name);

        auto description = elem_prop.attribute("help").as_view();

        auto prop_type = elem_prop.attribute("type").as_sview();
        if (prop_type.starts_with("type_"))
        {
            prop_type.remove_prefix(sizeof("type_") - 1);
        }

        GenEnum::PropType property_type { type_unknown };

        if (auto result = umap_PropTypes.find(prop_type); result != umap_PropTypes.end())
        {
            property_type = result->second;
        }

        if (property_type == type_unknown)
        {
            MSG_ERROR(tt_string("Unrecognized property type -- ") << prop_type);
            elem_prop = elem_prop.next_sibling("property");
            continue;
        }

        tt_string def_value;
        if (auto lastChild = elem_prop.last_child(); lastChild && !lastChild.text().empty())
        {
            def_value = lastChild.text().get();
            if (tt::is_found(def_value.find('\n')))
            {
                def_value.trim(tt::TRIM::both);
            }
        }

        auto prop_info = new PropDeclaration(prop_name, property_type, def_value, description);
        node_declaration->GetPropInfoMap()[name] = prop_info;
        if (elem_prop.attribute("hide").as_bool())
        {
            node_declaration->HideProperty(prop_name);
        }

        if (property_type == type_bitlist || property_type == type_option || property_type == type_editoption)
        {
            auto& opts = prop_info->getOptions();
            auto elem_opt = elem_prop.child("option");
            while (elem_opt)
            {
                auto& opt = opts.emplace_back();
                opt.name = elem_opt.attribute("name").as_view();
                opt.help = elem_opt.attribute("help").as_view();

                elem_opt = elem_opt.next_sibling("option");
            }
        }

        // Any time there is a var_name property, it needs to be followed by a var_comment and class_access property. Rather
        // than add this to all the XML generator specifications, we simply insert it here if it doesn't exist.

        if (tt::is_sameas(name, map_PropNames[prop_var_name]) && !node_declaration->isGen(gen_data_string) &&
            !node_declaration->isGen(gen_data_xml))
        {
            category.addProperty(prop_var_comment);
            prop_info = new PropDeclaration(prop_var_comment, type_string_edit_single, tt_empty_cstr,
                                            "Comment to add to the variable name in the generated header file "
                                            "if the class access is set to protected or public");
            node_declaration->GetPropInfoMap()[map_PropNames[prop_var_comment]] = prop_info;

            category.addProperty(prop_class_access);
            tt_string access("protected:");

            // Most widgets will default to protected: as their class access. Those in the
            // set_no_class_access array should have "none" as the default class access.

            if (set_no_class_access.contains(node_declaration->getGenName()))
            {
                access = "none";
            }

            prop_info = new PropDeclaration(prop_class_access, type_option, access,
                                            "Determines the type of access your inherited class has to this item.");
            node_declaration->GetPropInfoMap()[map_PropNames[prop_class_access]] = prop_info;

            auto& opts = prop_info->getOptions();

            if (!node_declaration->isGen(gen_wxTimer))
            {
                opts.emplace_back();
                opts[opts.size() - 1].name = "none";
                opts[opts.size() - 1].help = "Derived classes do not have access to this item.";
            }

            opts.emplace_back();
            opts[opts.size() - 1].name = "protected:";
            opts[opts.size() - 1].help = "Derived classes can access this item. In Python, item will have a self. prefix.";

            opts.emplace_back();
            opts[opts.size() - 1].name = "public:";
            opts[opts.size() - 1].help =
                "In C++, item is added as a public: class member. In Python, item will have a self. prefix.";
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
            auto name = elem_category.attribute("name").as_view();
            auto& new_cat = category.addCategory(name);

            ParseEvents(elem_category, new_cat);
        }
        elem_category = elem_category.next_sibling("category");
    }

    auto nodeEvent = elem_obj.child("event");
    while (nodeEvent)
    {
        auto evt_name = nodeEvent.attribute("name").as_str();
        category.addEvent(evt_name);

        auto evt_class = nodeEvent.attribute("class").as_str("wxEvent");
        auto description = nodeEvent.attribute("help").as_str();

        m_events[evt_name] = new NodeEventInfo(evt_name, evt_class, description);

        nodeEvent = nodeEvent.next_sibling("event");
    }
}
