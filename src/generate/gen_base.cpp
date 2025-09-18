/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for the Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/datetime.h>  // wxDateTime
#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "gen_base.h"

#include "base_generator.h"   // BaseGenerator -- Base Generator class
#include "code.h"             // Code -- Helper class for generating code
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node class
#include "node_decl.h"        // NodeDeclaration class
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_view_vector -- Read/Write line-oriented strings/files
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // Write code to Scintilla or file

using namespace GenEnum;

std::map<wxBitmapType, std::string> g_map_handlers;
std::map<wxBitmapType, std::string> g_map_types;

#define ADD_TYPE(name) g_map_types[name] = #name;

BaseCodeGenerator::BaseCodeGenerator(GenLang language, Node* form_node)
{
    m_language = language;
    m_form_node = form_node;

    if (g_map_types.empty())
    {
        ADD_TYPE(wxBITMAP_TYPE_BMP);  // We include this, but the handler is always loaded
        ADD_TYPE(wxBITMAP_TYPE_ICO);
        ADD_TYPE(wxBITMAP_TYPE_CUR);
        ADD_TYPE(wxBITMAP_TYPE_XPM);
        ADD_TYPE(wxBITMAP_TYPE_ICO_RESOURCE);
#ifndef __WXOSX__
        ADD_TYPE(wxBITMAP_TYPE_TIFF);
#endif
        ADD_TYPE(wxBITMAP_TYPE_GIF);
        ADD_TYPE(wxBITMAP_TYPE_PNG);
        ADD_TYPE(wxBITMAP_TYPE_JPEG);
        ADD_TYPE(wxBITMAP_TYPE_PNM);
        ADD_TYPE(wxBITMAP_TYPE_ANI);
        ADD_TYPE(wxBITMAP_TYPE_WEBP);

        g_map_handlers[wxBITMAP_TYPE_ICO] = "wxICOHandler";
        g_map_handlers[wxBITMAP_TYPE_CUR] = "wxCURHandler";
        g_map_handlers[wxBITMAP_TYPE_XPM] = "wxXPMHandler";
#ifndef __WXOSX__
        g_map_handlers[wxBITMAP_TYPE_TIFF] = "wxTIFFHandler";
#endif
        g_map_handlers[wxBITMAP_TYPE_GIF] = "wxGIFHandler";
        g_map_handlers[wxBITMAP_TYPE_PNG] = "wxPNGHandler";
        g_map_handlers[wxBITMAP_TYPE_JPEG] = "wxJPEGHandler";
        g_map_handlers[wxBITMAP_TYPE_PNM] = "wxPNMHandler";
        g_map_handlers[wxBITMAP_TYPE_ANI] = "wxANIHandler";
        g_map_handlers[wxBITMAP_TYPE_WEBP] = "wxWEBPHandler";
    }
}

tt_string BaseCodeGenerator::GetDeclaration(Node* node)
{
    tt_string code;

    tt_string class_name(node->get_DeclName());

    if (class_name.starts_with("wx"))
    {
        if (node->HasValue(prop_subclass))
        {
            code << node->as_string(prop_subclass) << "* " << node->get_NodeName() << ';';
        }
        else
        {
            if (node->get_Generator()->IsGeneric(node))
            {
                class_name = node->get_DeclName();
                class_name.Replace("wx", "wxGeneric");
            }
            code << class_name << "* " << node->get_NodeName() << ';';
        }

        if (class_name == "wxStdDialogButtonSizer")
        {
            if (!node->get_Form()->is_Gen(gen_wxDialog) || node->as_bool(prop_Save) ||
                node->as_bool(prop_ContextHelp))
            {
                if (node->as_bool(prop_OK))
                    code << "\n\twxButton* " << node->get_NodeName() << "OK;";
                if (node->as_bool(prop_Yes))
                    code << "\n\twxButton* " << node->get_NodeName() << "Yes;";
                if (node->as_bool(prop_Save))
                    code << "\n\twxButton* " << node->get_NodeName() << "Save;";
                if (node->as_bool(prop_Apply))
                    code << "\n\twxButton* " << node->get_NodeName() << "Apply;";
                if (node->as_bool(prop_No))
                    code << "\n\twxButton* " << node->get_NodeName() << "No;";
                if (node->as_bool(prop_Cancel))
                    code << "\n\twxButton* " << node->get_NodeName() << "Cancel;";
                if (node->as_bool(prop_Close))
                    code << "\n\twxButton* " << node->get_NodeName() << "Close;";
                if (node->as_bool(prop_Help))
                    code << "\n\twxButton* " << node->get_NodeName() << "Help;";
                if (node->as_bool(prop_ContextHelp))
                    code << "\n\twxButton* " << node->get_NodeName() << "ContextHelp;";
            }
        }
        else if (class_name == "wxStaticBitmap")
        {
            // If scaling was specified, then we need to switch to wxGenericStaticBitmap in order to
            // support it.
            if (node->as_string(prop_scale_mode) != "None")
                code.Replace("wxStaticBitmap", "wxGenericStaticBitmap");
        }
    }
    else if (node->HasValue(prop_subclass))
    {
        code << node->as_string(prop_subclass) << "* " << node->get_NodeName() << ';';
    }
    else if (class_name == "CloseButton")
    {
        code << "wxBitmapButton* " << node->get_NodeName() << ';';
    }
    else if (class_name == "StaticCheckboxBoxSizer")
    {
        if (node->HasValue(prop_checkbox_var_name))
            code << "wxCheckBox* " << node->as_string(prop_checkbox_var_name) << ';';

        if (!node->is_Local())
        {
            if (code.size())
                code << "\n";
            code << "wxStaticBoxSizer* " << node->get_NodeName() << ';';
        }
    }
    else if (class_name == "StaticRadioBtnBoxSizer")
    {
        if (node->HasValue(prop_radiobtn_var_name))
            code << "wxRadioButton* " << node->as_string(prop_radiobtn_var_name) << ';';

        if (!node->is_Local())
        {
            if (code.size())
                code << "\n";
            code << "wxStaticBoxSizer* " << node->get_NodeName() << ';';
        }
    }

    else if (class_name == "propGridItem" || class_name == "propGridCategory")
    {
        code << "wxPGProperty* " << node->get_NodeName() << ';';
    }
    else if (class_name == "BookPage")
    {
        code << "wxPanel* " << node->get_NodeName() << ';';
    }
    else if (class_name == "propGridPage")
    {
        code << "wxPropertyGridPage* " << node->get_NodeName() << ';';
    }
    else if (class_name == "submenu")
    {
        code << "wxMenu* " << node->get_NodeName() << ';';
    }
    else if (class_name == "Check3State")
    {
        code << "wxCheckBox* " << node->get_NodeName() << ';';
    }
    else if (class_name == "tool")
    {
        class_name = node->get_Parent()->get_DeclName();
        if (class_name == "wxAuiToolBar")
        {
            code << "wxAuiToolBarItem* " << node->get_NodeName() << ';';
        }
        else if (class_name == "wxToolBar")
        {
            code << "wxToolBarToolBase* " << node->get_NodeName() << ';';
        }
        else if (class_name == "ToolBar")
        {
            code << "wxToolBarToolBase* " << node->get_NodeName() << ';';
        }
        else
        {
            FAIL_MSG("Unrecognized class name so no idea how to declare it in the header file.")
        }
    }
    else if (class_name.is_sameas("CustomControl"))
    {
        if (auto* node_namespace = node->get_Folder();
            node_namespace && node_namespace->HasValue(prop_folder_namespace))
        {
            code << node_namespace->as_string(prop_folder_namespace) << "::";
        }
        else if (node->HasValue(prop_namespace))
        {
            code << node->as_string(prop_namespace) << "::";
        }
        code << node->as_string(prop_class_name) << "* " << node->get_NodeName() << ';';
    }

    else if (class_name.is_sameas("dataViewColumn") || class_name.is_sameas("dataViewListColumn"))
    {
        code << "wxDataViewColumn* " << node->get_NodeName() << ';';
    }

    if (node->HasValue(prop_var_comment))
    {
        code << "  // " << node->as_string(prop_var_comment);
    }

    return code;
}

// This is a static function
void BaseCodeGenerator::CollectIDs(Node* node, std::set<std::string>& set_enum_ids,
                                   std::set<std::string>& set_const_ids)
{
    for (auto& iter: node->get_PropsVector())
    {
        if (iter.type() == type_id)
        {
            auto& prop_id = iter.as_string();
            if (prop_id.size() && !prop_id.starts_with("wxID_"))
            {
                if (tt::is_found(
                        prop_id.find('=')))  // If it has an assignment operator, it's a constant
                    set_const_ids.insert(prop_id);
                else
                    set_enum_ids.insert(prop_id);
            }
        }
    }

    for (const auto& iter: node->get_ChildNodePtrs())
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

    for (auto& iter: node->get_MapEvents())
    {
        // Only add the event if a handler was specified
        if (iter.second.get_value().size())
        {
            // Because the NodeEvent* gets stored in a set if there is a conditional, it won't get
            // duplicated even if it is added by both the Node and any container containing the same
            // conditional

            if (node->HasProp(prop_platforms) &&
                node->as_string(prop_platforms) != "Windows|Unix|Mac")
            {
                if (!m_map_conditional_events.contains(node->as_string(prop_platforms)))
                {
                    m_map_conditional_events[node->as_string(prop_platforms)] =
                        std::vector<NodeEvent*>();
                }
                if (!CheckIfEventExists(m_map_conditional_events[node->as_string(prop_platforms)],
                                        &iter.second))
                {
                    m_map_conditional_events[node->as_string(prop_platforms)].push_back(
                        &iter.second);
                }
            }

            // If node_container is non-null, it means the current node is within a container that
            // has a conditional.
            else if (auto node_container = node->get_PlatformContainer(); node_container)
            {
                if (!m_map_conditional_events.contains(node_container->as_string(prop_platforms)))
                {
                    m_map_conditional_events[node_container->as_string(prop_platforms)] =
                        std::vector<NodeEvent*>();
                }
                if (!CheckIfEventExists(
                        m_map_conditional_events[node_container->as_string(prop_platforms)],
                        &iter.second))
                {
                    m_map_conditional_events[node_container->as_string(prop_platforms)].push_back(
                        &iter.second);
                }
            }

            else
            {
                if (node->get_Parent()->is_Gen(gen_wxContextMenuEvent))
                    m_ctx_menu_events.push_back(&iter.second);
                else
                    events.push_back(&iter.second);
            }
        }
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_wxContextMenuEvent))
        {
            for (const auto& ctx_child: child->get_ChildNodePtrs())
            {
                CollectEventHandlers(ctx_child.get(), m_ctx_menu_events);
            }
            continue;
        }
        CollectEventHandlers(child.get(), events);
    }
}

// This function is called by the thread thrd_collect_img_headers
void BaseCodeGenerator::CollectImageHeaders(Node* node, std::set<std::string>& embedset)
{
    for (auto& iter: node->get_PropsVector())
    {
        if (!iter.HasValue())
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
                        if (auto* embed = ProjectImages.GetEmbeddedImage(idx_image); embed)
                        {
                            bool is_found = false;
                            for (auto* pimage: m_embedded_images)
                            {
                                if (pimage == embed)
                                {
                                    is_found = true;
                                    break;
                                }
                            }
                            if (!is_found)
                            {
                                if (embed->base_image().filename.file_exists())
                                {
                                    auto file_time = embed->base_image().filename.last_write_time();
                                    if (file_time != embed->base_image().file_time)
                                    {
                                        embed->UpdateImage(embed->base_image());
                                        embed->base_image().file_time = file_time;
                                    }
                                    m_embedded_images.emplace_back(embed);
                                }
                                else
                                {
                                    MSG_INFO(tt_string() << "Unable to get file time for "
                                                         << embed->base_image().filename);
                                }
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
                            auto output_dir = Project.get_BaseDirectory(node, m_language);
                            output_dir.append_filename(path);
                            if (!output_dir.file_exists())
                            {
                                art_dir.append_filename(path.filename());
                                if (art_dir.file_exists())
                                {
                                    path = art_dir;
                                    path.make_relative(Project.get_BaseDirectory(node, m_language));
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
                // Since this is a thread, you can't send the standard MSG_WARNING if the window is
                // opened, or it will lock the debugger.
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
                    auto output_dir = Project.get_BaseDirectory(node, m_language);
                    output_dir.append_filename(path);
                    if (!output_dir.file_exists())
                    {
                        art_dir.append_filename(path);
                        if (art_dir.file_exists())
                        {
                            path = art_dir;
                            path.make_relative(Project.get_BaseDirectory(node, m_language));
                        }
                    }
                }
                path.backslashestoforward();
                embedset.insert(tt_string() << "#include \"" << path << "\"");
            }
        }
    }

    for (auto& child: node->get_ChildNodePtrs())
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
    if (node->is_Form() && node->HasValue(prop_icon))
    {
        tt_view_vector parts(node->as_string(prop_icon), BMP_PROP_SEPARATOR, tt::TRIM::both);
        if (parts.size() >= IndexImage + 1)
        {
            // If ProjectImages returns a function name, then the function will be in the
            // Images List header file, so we don't need to generate any functions for it in
            // the source file.
            if (auto function_name = ProjectImages.GetBundleFuncName(node->as_string(prop_icon));
                function_name.empty())
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
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        for (auto& iter: child->get_PropsVector())
        {
            if ((iter.type() == type_image || iter.type() == type_animation) && iter.HasValue())
            {
                tt_string_vector parts(iter.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
                if (parts.size() < IndexImage + 1)
                    continue;

                if (parts[IndexType] == "Embed")
                {
                    if (iter.type() == type_animation)
                    {
                        m_NeedAnimationFunction = true;
                    }
                    else
                    {
                        if (!m_ImagesForm)
                        {
                            m_NeedImageFunction = true;
                        }

                        // If we haven't already encountered an image that requires a function,
                        // then check to see if this image is in the Images List file and has a
                        // bundle function to access it. If it does, then we still don't need
                        // to generate an image function in the class file.
                        else if (!m_NeedImageFunction)
                        {
                            if (auto bundle = ProjectImages.GetPropertyImageBundle(parts);
                                bundle && bundle->lst_filenames.size())
                            {
                                if (auto embed =
                                        ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                                    embed)
                                {
#if defined(_DEBUG)
                                    auto name = ProjectImages.GetBundleFuncName(embed);
                                    if (name.empty())
#else
                                    if (ProjectImages.GetBundleFuncName(embed).empty())
#endif  // _DEBUG
                                    {
                                        m_NeedImageFunction = true;
                                    }
                                }
                            }
                        }
                    }
                }
                else if ((parts[IndexType] == "SVG"))
                {
                    if (!m_ImagesForm)
                    {
                        m_NeedSVGFunction = true;
                    }
                    else if (!m_NeedSVGFunction)
                    {
                        if (auto bundle = ProjectImages.GetPropertyImageBundle(parts);
                            bundle && bundle->lst_filenames.size())
                        {
                            if (auto embed =
                                    ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                                embed)
                            {
#if defined(_DEBUG)
                                auto name = ProjectImages.GetBundleFuncName(embed);
                                if (name.empty())
#else
                                if (ProjectImages.GetBundleFuncName(embed).empty())
#endif  // _DEBUG
                                {
                                    m_NeedSVGFunction = true;
                                }
                            }
                        }
                    }
                }
                else if (parts[IndexType] == "Header")
                {
                    if (iter.type() == type_animation)
                        m_NeedAnimationFunction = true;
                    else if (!tt::is_sameas(parts[IndexImage].extension(), ".xpm",
                                            tt::CASE::either))
                        m_NeedHeaderFunction = true;
                }
            }
        }
        if (child->get_ChildCount())
        {
            ParseImageProperties(child.get());
        }
    }
}

void BaseCodeGenerator::AddPersistCode(Node* node)
{
    if (node->HasValue(prop_persist_name))
    {
        tt_string code("wxPersistentRegisterAndRestore(");
        code << node->get_NodeName() << ", \"" << node->as_string(prop_persist_name) << "\");";
        m_source->writeLine(code);
    }

    for (const auto& child: node->get_ChildNodePtrs())
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
    if (auto generator = node_ctx_menu->get_Generator(); generator)
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

void BaseCodeGenerator::SetImagesForm()
{
    m_ImagesForm = nullptr;
    for (const auto& form: Project.get_ChildNodePtrs())
    {
        if (form->is_Gen(gen_folder))
        {
            for (const auto& child_form: form->get_ChildNodePtrs())
            {
                if (child_form->is_Gen(gen_Images))
                {
                    m_ImagesForm = child_form.get();
                    break;
                }
            }
            break;
        }

        else if (form->is_Gen(gen_Images))
        {
            m_ImagesForm = form.get();
            break;
        }
    }
}
