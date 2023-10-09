/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for the Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <future>
#include <thread>
#include <unordered_set>

#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "gen_base.h"

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // Common component functions
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "node_decl.h"        // NodeDeclaration class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // Write code to Scintilla or file

using namespace GenEnum;

std::map<wxBitmapType, std::string> g_map_handlers;
std::map<wxBitmapType, std::string> g_map_types;

#define ADD_TYPE(name) g_map_types[name] = #name;

BaseCodeGenerator::BaseCodeGenerator(int language, Node* form_node)
{
    m_language = language;
    m_form_node = form_node;

    if (g_map_types.empty())
    {
        ADD_TYPE(wxBITMAP_TYPE_BMP);  // We include this, but the handler is always loaded
        ADD_TYPE(wxBITMAP_TYPE_ICO);
        ADD_TYPE(wxBITMAP_TYPE_CUR);
        ADD_TYPE(wxBITMAP_TYPE_XPM);
        ADD_TYPE(wxBITMAP_TYPE_TIFF);
        ADD_TYPE(wxBITMAP_TYPE_GIF);
        ADD_TYPE(wxBITMAP_TYPE_PNG);
        ADD_TYPE(wxBITMAP_TYPE_JPEG);
        ADD_TYPE(wxBITMAP_TYPE_PNM);
        ADD_TYPE(wxBITMAP_TYPE_PCX);
        ADD_TYPE(wxBITMAP_TYPE_ANI);
        ADD_TYPE(wxBITMAP_TYPE_TGA);

        g_map_handlers[wxBITMAP_TYPE_ICO] = "wxICOHandler";
        g_map_handlers[wxBITMAP_TYPE_CUR] = "wxCURHandler";
        g_map_handlers[wxBITMAP_TYPE_XPM] = "wxXPMHandler";
        g_map_handlers[wxBITMAP_TYPE_TIFF] = "wxTIFFHandler";
        g_map_handlers[wxBITMAP_TYPE_GIF] = "wxGIFHandler";
        g_map_handlers[wxBITMAP_TYPE_PNG] = "wxPNGHandler";
        g_map_handlers[wxBITMAP_TYPE_JPEG] = "wxJPEGHandler";
        g_map_handlers[wxBITMAP_TYPE_PNM] = "wxPNMHandler";
        g_map_handlers[wxBITMAP_TYPE_PCX] = "wxPCXHandler";
        g_map_handlers[wxBITMAP_TYPE_ANI] = "wxANIHandler";
        g_map_handlers[wxBITMAP_TYPE_TGA] = "wxTGAHandler";
    }
}

void BaseCodeGenerator::CollectMemberVariables(Node* node, Permission perm, std::set<std::string>& code_lines)
{
    if (auto prop = node->getPropPtr(prop_class_access); prop)
    {
        if (prop->as_string() != "none")
        {
            if ((perm == Permission::Public && prop->as_string() == "public:") ||
                (perm == Permission::Protected && prop->as_string() == "protected:"))
            {
                auto code = GetDeclaration(node);
                if (code.size())
                {
                    if (node->hasProp(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
                    {
                        if (perm == Permission::Public)
                        {
                            if (!m_map_public_members.contains(node->as_string(prop_platforms)))
                            {
                                m_map_public_members[node->as_string(prop_platforms)] = std::set<tt_string>();
                            }
                            m_map_public_members[node->as_string(prop_platforms)].insert(code);
                        }
                        else
                        {
                            if (!m_map_protected.contains(node->as_string(prop_platforms)))
                            {
                                m_map_protected[node->as_string(prop_platforms)] = std::set<tt_string>();
                            }
                            m_map_protected[node->as_string(prop_platforms)].insert(code);
                        }
                    }
                    // If node_container is non-null, it means the current node is within a container that
                    // has a conditional.
                    else if (auto node_container = node->getPlatformContainer(); node_container)
                    {
                        if (perm == Permission::Public)
                        {
                            if (!m_map_public_members.contains(node_container->as_string(prop_platforms)))
                            {
                                m_map_public_members[node_container->as_string(prop_platforms)] = std::set<tt_string>();
                            }
                            m_map_public_members[node_container->as_string(prop_platforms)].insert(code);
                        }
                        else
                        {
                            if (!m_map_protected.contains(node_container->as_string(prop_platforms)))
                            {
                                m_map_protected[node_container->as_string(prop_platforms)] = std::set<tt_string>();
                            }
                            m_map_protected[node_container->as_string(prop_platforms)].insert(code);
                        }
                    }
                    else
                    {
                        code_lines.insert(code);
                    }
                }
            }
        }
    }

    if (perm == Permission::Protected)
    {
        // StaticCheckboxBoxSizer and StaticRadioBtnBoxSizer have internal variables
        if (node->hasValue(prop_checkbox_var_name) || node->hasValue(prop_radiobtn_var_name))
        {
            auto code = GetDeclaration(node);
            if (code.size())
            {
                if (node->hasProp(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
                {
                    if (!m_map_protected.contains(node->as_string(prop_platforms)))
                    {
                        m_map_protected[node->as_string(prop_platforms)] = std::set<tt_string>();
                    }
                    m_map_protected[node->as_string(prop_platforms)].insert(code);
                }
                else
                {
                    code_lines.insert(code);
                }
            }
        }
    }

    for (const auto& child: node->getChildNodePtrs())
    {
        CollectMemberVariables(child.get(), perm, code_lines);
    }

    return;
}

void BaseCodeGenerator::CollectValidatorVariables(Node* node, std::set<std::string>& code_lines)
{
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        GenCppValVarsBase(node->getNodeDeclaration(), node, code_lines);
    }

    for (const auto& child: node->getChildNodePtrs())
    {
        CollectValidatorVariables(child.get(), code_lines);
    }
}

void BaseCodeGenerator::CollectIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    GatherGeneratorIncludes(node, set_src, set_hdr);

    // If an include is going to be generated in the header file, then don't also generate it
    // in the src file.
    for (auto& iter: set_hdr)
    {
        if (auto pos = set_src.find(iter); pos != set_src.end())
        {
            set_src.erase(pos);
        }
    }
}

void BaseCodeGenerator::GatherGeneratorIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    if (node->isGen(gen_Images) || node->isGen(gen_folder))
    {
        return;
    }

    bool isAddToSrc = false;

    // If the component is set for local access only, then add the header file to the source
    // set. Once all processing is done, if this header was also used by a component with
    // non-local access, then it will be removed from the source set.
    if (node->isPropValue(prop_class_access, "none"))
        isAddToSrc = true;

    auto generator = node->getNodeDeclaration()->getGenerator();
    ASSERT(generator);
    if (!generator)
        return;

    generator->GetIncludes(node, set_src, set_hdr);

    if (node->hasValue(prop_derived_header))
    {
        tt_string header("#include \"");
        header << node->as_string(prop_derived_header) << '"';
        set_src.insert(header);
    }

    if (node->hasValue(prop_derived_class) && !node->isPropValue(prop_class_access, "none"))
    {
        set_hdr.insert(tt_string() << "class " << node->as_string(prop_derived_class) << ';');
    }

    // A lot of widgets have wxWindow and/or wxAnyButton as derived classes, and those classes contain properties for
    // font, color, and bitmaps. If the property is used, then we add a matching header file.

    for (auto& iter: node->getPropsVector())
    {
        if (iter.hasValue())
        {
            if (iter.type() == type_wxFont)
            {
                if (isAddToSrc)
                {
                    set_src.insert("#include <wx/font.h>");
                }
                else
                {
                    set_hdr.insert("#include <wx/font.h>");
                }
            }
            else if (iter.type() == type_wxColour)
            {
                if (isAddToSrc)
                {
                    set_src.insert("#include <wx/colour.h>");
                    set_src.insert("#include <wx/settings.h>");  // This is needed for the system colours
                }
                else
                {
                    set_hdr.insert("#include <wx/colour.h>");
                    set_hdr.insert("#include <wx/settings.h>");  // This is needed for the system colours
                }
            }
            else if (iter.type() == type_image)
            {
                if (!iter.isProp(prop_icon))
                {
                    if (auto function_name = ProjectImages.GetBundleFuncName(iter.as_string()); function_name.size())
                    {
                        std::vector<Node*> forms;
                        Project.CollectForms(forms);

                        for (const auto& form: forms)
                        {
                            if (form->isGen(gen_Images))
                            {
                                tt_string image_file = Project.getProjectPath();
                                image_file.append_filename(form->as_string(prop_base_file));
                                image_file.replace_extension(m_header_ext);
                                image_file.make_relative(m_baseFullPath);
                                set_src.insert(tt_string() << "#include \"" << image_file << '\"');
                                break;
                            }
                        }
                        continue;
                    }
                }

                // The problem at this point is that we don't know how the bitmap will be used. It could be just a
                // wxBitmap, or it could be handed to a wxImage for sizing, or it might be handed to
                // wxWindow->SetIcon(). We play it safe and supply all three header files.

                if (isAddToSrc)
                {
                    set_src.insert("#include <wx/bitmap.h>");
                    set_src.insert("#include <wx/icon.h>");
                    set_src.insert("#include <wx/image.h>");
                }
                else
                {
                    set_hdr.insert("#include <wx/bitmap.h>");
                    set_hdr.insert("#include <wx/icon.h>");
                    set_hdr.insert("#include <wx/image.h>");
                }
            }
        }
    }

    // Now parse all the children
    for (const auto& child: node->getChildNodePtrs())
    {
        GatherGeneratorIncludes(child.get(), set_src, set_hdr);
    }
}

tt_string BaseCodeGenerator::GetDeclaration(Node* node)
{
    tt_string code;

    tt_string class_name(node->declName());

    if (class_name.starts_with("wx"))
    {
        if (node->hasValue(prop_derived_class))
        {
            code << node->as_string(prop_derived_class) << "* " << node->getNodeName() << ';';
        }
        else
        {
            if (node->getGenerator()->IsGeneric(node))
            {
                class_name = node->declName();
                class_name.Replace("wx", "wxGeneric");
            }
            code << class_name << "* " << node->getNodeName() << ';';
        }

        if (class_name == "wxStdDialogButtonSizer")
        {
            if (!node->getForm()->isGen(gen_wxDialog) || node->as_bool(prop_Save) || node->as_bool(prop_ContextHelp))
            {
                if (node->as_bool(prop_OK))
                    code << "\n\twxButton* " << node->getNodeName() << "OK;";
                if (node->as_bool(prop_Yes))
                    code << "\n\twxButton* " << node->getNodeName() << "Yes;";
                if (node->as_bool(prop_Save))
                    code << "\n\twxButton* " << node->getNodeName() << "Save;";
                if (node->as_bool(prop_Apply))
                    code << "\n\twxButton* " << node->getNodeName() << "Apply;";
                if (node->as_bool(prop_No))
                    code << "\n\twxButton* " << node->getNodeName() << "No;";
                if (node->as_bool(prop_Cancel))
                    code << "\n\twxButton* " << node->getNodeName() << "Cancel;";
                if (node->as_bool(prop_Close))
                    code << "\n\twxButton* " << node->getNodeName() << "Close;";
                if (node->as_bool(prop_Help))
                    code << "\n\twxButton* " << node->getNodeName() << "Help;";
                if (node->as_bool(prop_ContextHelp))
                    code << "\n\twxButton* " << node->getNodeName() << "ContextHelp;";
            }
        }
        else if (class_name == "wxStaticBitmap")
        {
            // If scaling was specified, then we need to switch to wxGenericStaticBitmap in order to support it.
            if (node->as_string(prop_scale_mode) != "None")
                code.Replace("wxStaticBitmap", "wxGenericStaticBitmap");
        }
    }

    else if (class_name == "StaticCheckboxBoxSizer")
    {
        if (node->hasValue(prop_checkbox_var_name))
            code << "wxCheckBox* " << node->as_string(prop_checkbox_var_name) << ';';

        if (!node->isLocal())
        {
            if (code.size())
                code << "\n";
            code << "wxStaticBoxSizer* " << node->getNodeName() << ';';
        }
    }
    else if (class_name == "StaticRadioBtnBoxSizer")
    {
        if (node->hasValue(prop_radiobtn_var_name))
            code << "wxRadioButton* " << node->as_string(prop_radiobtn_var_name) << ';';

        if (!node->isLocal())
        {
            if (code.size())
                code << "\n";
            code << "wxStaticBoxSizer* " << node->getNodeName() << ';';
        }
    }

    else if (class_name == "propGridItem" || class_name == "propGridCategory")
    {
        code << "wxPGProperty* " << node->getNodeName() << ';';
    }
    else if (class_name == "BookPage")
    {
        code << "wxPanel* " << node->getNodeName() << ';';
    }
    else if (class_name == "propGridPage")
    {
        code << "wxPropertyGridPage* " << node->getNodeName() << ';';
    }
    else if (class_name == "submenu")
    {
        code << "wxMenu* " << node->getNodeName() << ';';
    }
    else if (class_name == "Check3State")
    {
        code << "wxCheckBox* " << node->getNodeName() << ';';
    }
    else if (class_name == "tool")
    {
        class_name = node->getParent()->declName();
        if (class_name == "wxAuiToolBar")
        {
            code << "wxAuiToolBarItem* " << node->getNodeName() << ';';
        }
        else if (class_name == "wxToolBar")
        {
            code << "wxToolBarToolBase* " << node->getNodeName() << ';';
        }
        else if (class_name == "ToolBar")
        {
            code << "wxToolBarToolBase* " << node->getNodeName() << ';';
        }
        else
        {
            FAIL_MSG("Unrecognized class name so no idea how to declare it in the header file.")
        }
    }
    else if (class_name.is_sameas("CustomControl"))
    {
        if (auto* node_namespace = node->getFolder(); node_namespace && node_namespace->hasValue(prop_folder_namespace))
        {
            code << node_namespace->as_string(prop_folder_namespace) << "::";
        }
        else if (node->hasValue(prop_namespace))
        {
            code << node->as_string(prop_namespace) << "::";
        }
        code << node->as_string(prop_class_name) << "* " << node->getNodeName() << ';';
    }
    else if (class_name.is_sameas("dataViewColumn"))
    {
        code << "wxDataViewColumn* " << node->getNodeName() << ';';
    }

    if (node->hasValue(prop_var_comment))
    {
        code << "  // " << node->as_string(prop_var_comment);
    }

    return code;
}

// This is a static function
void BaseCodeGenerator::CollectIDs(Node* node, std::set<std::string>& set_enum_ids, std::set<std::string>& set_const_ids)
{
    for (auto& iter: node->getPropsVector())
    {
        if (iter.type() == type_id)
        {
            auto& prop_id = iter.as_string();
            if (prop_id.size() && !prop_id.starts_with("wxID_"))
            {
                if (tt::is_found(prop_id.find('=')))  // If it has an assignment operator, it's a constant
                    set_const_ids.insert(prop_id);
                else
                    set_enum_ids.insert(prop_id);
            }
        }
    }

    for (const auto& iter: node->getChildNodePtrs())
    {
        CollectIDs(iter.get(), set_enum_ids, set_const_ids);
    }
}

void BaseCodeGenerator::CollectEventHandlers(Node* node, std::vector<NodeEvent*>& events)
{
    ASSERT(node);

    auto CheckIfEventExists = [](const EventVector& vectors, const NodeEvent* event) -> bool
    {
        for (const auto vector_event: vectors)
        {
            if (vector_event == event)
                return true;
        }
        return false;
    };

    for (auto& iter: node->getMapEvents())
    {
        // Only add the event if a handler was specified
        if (iter.second.get_value().size())
        {
            // Because the NodeEvent* gets stored in a set if there is a conditional, it won't get duplicated
            // even if it is added by both the Node and any container containing the same conditional

            if (node->hasProp(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
            {
                if (!m_map_conditional_events.contains(node->as_string(prop_platforms)))
                {
                    m_map_conditional_events[node->as_string(prop_platforms)] = std::vector<NodeEvent*>();
                }
                if (!CheckIfEventExists(m_map_conditional_events[node->as_string(prop_platforms)], &iter.second))
                {
                    m_map_conditional_events[node->as_string(prop_platforms)].push_back(&iter.second);
                }
            }

            // If node_container is non-null, it means the current node is within a container that
            // has a conditional.
            else if (auto node_container = node->getPlatformContainer(); node_container)
            {
                if (!m_map_conditional_events.contains(node_container->as_string(prop_platforms)))
                {
                    m_map_conditional_events[node_container->as_string(prop_platforms)] = std::vector<NodeEvent*>();
                }
                if (!CheckIfEventExists(m_map_conditional_events[node_container->as_string(prop_platforms)], &iter.second))
                {
                    m_map_conditional_events[node_container->as_string(prop_platforms)].push_back(&iter.second);
                }
            }

            else
            {
                if (node->getParent()->isGen(gen_wxContextMenuEvent))
                    m_ctx_menu_events.push_back(&iter.second);
                else
                    m_events.push_back(&iter.second);
            }
        }
    }

    for (const auto& child: node->getChildNodePtrs())
    {
        if (child->isGen(gen_wxContextMenuEvent))
        {
            for (const auto& ctx_child: child->getChildNodePtrs())
            {
                CollectEventHandlers(ctx_child.get(), m_CtxMenuEvents);
            }
            continue;
        }
        CollectEventHandlers(child.get(), events);
    }
}

// This function is called by the thread thrd_collect_img_headers
void BaseCodeGenerator::CollectImageHeaders(Node* node, std::set<std::string>& embedset)
{
    for (auto& iter: node->getPropsVector())
    {
        if (!iter.hasValue())
            continue;

        auto& value = iter.as_string();
        if (iter.type() == type_image)
        {
            if (auto bundle = ProjectImages.GetPropertyImageBundle(iter.as_string()); bundle)
            {
                if (value.starts_with("Embed") || value.starts_with("SVG"))
                {
                    for (auto& idx_image: bundle->lst_filenames)
                    {
                        if (auto embed = ProjectImages.GetEmbeddedImage(idx_image); embed)
                        {
                            bool is_found = false;
                            for (auto pimage: m_embedded_images)
                            {
                                if (pimage == embed)
                                {
                                    is_found = true;
                                    break;
                                }
                            }
                            if (!is_found)
                            {
                                m_embedded_images.emplace_back(embed);
                            }
                        }
                    }
                }
                else if (value.starts_with("Header") || value.starts_with("XPM"))
                {
                    for (auto& idx_image: bundle->lst_filenames)
                    {
                        tt_string path(idx_image);
                        auto art_dir = Project.ArtDirectory();
                        if (art_dir.size())
                        {
                            auto output_dir = Project.getBaseDirectory(node, m_language);
                            output_dir.append_filename(path);
                            if (!output_dir.file_exists())
                            {
                                art_dir.append_filename(path);
                                if (art_dir.file_exists())
                                {
                                    path = art_dir;
                                    path.make_relative(Project.getBaseDirectory(node, m_language));
                                }
                            }
                        }
                        path.backslashestoforward();
                        embedset.insert(tt_string() << "#include \"" << path << "\"");
                    }
                }
            }
            else
            {
                // Since this is a thread, you can't send the standard MSG_WARNING if the window is opened, or it will
                // lock the debugger.
            }
        }

        else if (iter.type() == type_animation)
        {
            if (value.starts_with("Embed"))
            {
                tt_view_vector parts(value, BMP_PROP_SEPARATOR, tt::TRIM::both);

                if (parts[IndexImage].size())
                {
                    auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                    if (!embed)
                    {
                        if (!ProjectImages.AddEmbeddedImage(parts[IndexImage], m_form_node))
                            continue;
                        embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                        if (!embed)
                            continue;
                    }
                    else
                    {
                        bool is_found = false;
                        for (auto& pimage: m_embedded_images)
                        {
                            if (pimage == embed)
                            {
                                is_found = true;
                                break;
                            }
                        }
                        if (is_found)
                            continue;  // we already have this image
                    }

                    m_embedded_images.emplace_back(embed);
                }
            }
            else if (value.starts_with("Header") || value.starts_with("XPM"))
            {
                tt_view_vector parts(value);
                if (tt::is_whitespace(parts[IndexImage].front()))
                {
                    parts[IndexImage].remove_prefix(1);
                }
                tt_string path = parts[IndexImage];
                auto art_dir = Project.ArtDirectory();
                if (art_dir.size())
                {
                    auto output_dir = Project.getBaseDirectory(node, m_language);
                    output_dir.append_filename(path);
                    if (!output_dir.file_exists())
                    {
                        art_dir.append_filename(path);
                        if (art_dir.file_exists())
                        {
                            path = art_dir;
                            path.make_relative(Project.getBaseDirectory(node, m_language));
                        }
                    }
                }
                path.backslashestoforward();
                embedset.insert(tt_string() << "#include \"" << path << "\"");
            }
        }
    }

    for (auto& child: node->getChildNodePtrs())
    {
        CollectImageHeaders(child.get(), embedset);
    }
}

// Called by the thread thrd_need_img_func
//
// Determine if Header or Animation functions need to be generated, and whether the
// wx/artprov.h header is needed
void BaseCodeGenerator::ParseImageProperties(Node* node)
{
    ASSERT(node);
    if (node->isForm() && node->hasValue(prop_icon))
    {
        tt_view_vector parts(node->as_string(prop_icon), BMP_PROP_SEPARATOR, tt::TRIM::both);
        if (parts.size() >= IndexImage + 1)
        {
            if (parts[IndexType] == "Header")
            {
                m_NeedHeaderFunction = true;
            }
            else if (parts[IndexType] == "Embed")
            {
                m_NeedImageFunction = true;
            }
            else if ((parts[IndexType] == "Art"))
            {
                m_NeedArtProviderHeader = true;
            }
            else if ((parts[IndexType] == "SVG"))
            {
                m_NeedSVGFunction = true;
            }
        }
    }

    for (const auto& child: node->getChildNodePtrs())
    {
        for (auto& iter: child->getPropsVector())
        {
            if ((iter.type() == type_image || iter.type() == type_animation) && iter.hasValue())
            {
                tt_string_vector parts(iter.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
                if (parts.size() < IndexImage + 1)
                    continue;

                // If this is an Images List, then we need to see if the image property refers
                // to an image within the Images List. If so, a function call will be made to
                // the Image List's source code to load the image and therefore we don't need
                // to generate any special header files or generate the general purpose image
                // loading function.

                if (m_ImagesForm && m_form_node != m_ImagesForm)
                {
                    if (auto bundle = ProjectImages.GetPropertyImageBundle(parts); bundle && bundle->lst_filenames.size())
                    {
                        if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                        {
                            if (embed->form == m_ImagesForm)
                            {
                                continue;
                            }
                        }
                    }
                }

                if (parts[IndexType] == "Embed")
                {
                    if (iter.type() == type_animation)
                        m_NeedAnimationFunction = true;
                    else
                        m_NeedImageFunction = true;
                }
                else if ((parts[IndexType] == "Art"))
                {
                    m_NeedArtProviderHeader = true;
                }
                else if ((parts[IndexType] == "SVG"))
                {
                    m_NeedSVGFunction = true;
                }
                else if (parts[IndexType] == "Header")
                {
                    if (iter.type() == type_animation)
                        m_NeedAnimationFunction = true;
                    else if (!tt::is_sameas(parts[IndexImage].extension(), ".xpm", tt::CASE::either))
                        m_NeedHeaderFunction = true;
                }
            }
        }
        if (child->getChildCount())
        {
            ParseImageProperties(child.get());
        }
    }
}

void BaseCodeGenerator::AddPersistCode(Node* node)
{
    if (node->hasValue(prop_persist_name))
    {
        tt_string code("wxPersistentRegisterAndRestore(");
        code << node->getNodeName() << ", \"" << node->as_string(prop_persist_name) << "\");";
        m_source->writeLine(code);
    }

    for (const auto& child: node->getChildNodePtrs())
    {
        AddPersistCode(child.get());
    }
}

void BaseCodeGenerator::WriteSetLines(WriteCode* out, std::set<std::string>& code_lines)
{
    for (auto iter: code_lines)
    {
        // out->writeLine((tt_string&) (iter));
        out->writeLine(iter);
    }
    code_lines.clear();
}

void BaseCodeGenerator::GenContextMenuHandler(Node* node_ctx_menu)
{
    if (auto generator = node_ctx_menu->getGenerator(); generator)
    {
        Code code(node_ctx_menu, m_language);
        if (generator->AfterChildrenCode(code))
        {
            m_source->writeLine(code);
        }
    }
}

void BaseCodeGenerator::WritePropSourceCode(Node* node, GenEnum::PropName prop)
{
    tt_string convert(node->as_string(prop));
    convert.Replace("@@", "\n", tt::REPLACE::all);
    tt_string_vector lines(convert, '\n');
    bool initial_bracket = false;
    for (auto& code: lines)
    {
        if (code.contains("}"))
        {
            m_source->Unindent();
        }
        else if (!initial_bracket && code.contains("["))
        {
            initial_bracket = true;
            m_source->Indent();
        }

        m_source->writeLine(code, indent::auto_no_whitespace);

        if (code.contains("{"))
        {
            m_source->Indent();
        }
    }
    m_source->Unindent();
    m_source->writeLine();
}

void BaseCodeGenerator::WritePropHdrCode(Node* node, GenEnum::PropName prop)
{
    tt_string convert(node->as_string(prop));
    convert.Replace("@@", "\n", tt::REPLACE::all);
    tt_string_vector lines(convert, '\n', tt::TRIM::right);
    bool initial_bracket = false;
    for (auto& code: lines)
    {
        if (code.contains("}") && !code.contains("{"))
        {
            m_header->Unindent();
        }
        else if (!initial_bracket && code.contains("["))
        {
            initial_bracket = true;
            m_header->Indent();
        }

        if (code.is_sameas("public:") || code.is_sameas("protected:") || code.is_sameas("private:"))
        {
            m_header->Unindent();
            m_header->writeLine(code, indent::auto_no_whitespace);
            m_header->Indent();
        }
        else
        {
            m_header->writeLine(code, indent::auto_no_whitespace);
        }

        if (code.contains("{") && !code.contains("}"))
        {
            m_header->Indent();
        }
    }
    m_header->writeLine();
}
