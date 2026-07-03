/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Src and Hdr files for the Base Class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include <wx/datetime.h>  // wxDateTime
#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include <format>

#include <frozen/map.h>

#include "gen_base.h"

#include "base_generator.h"                   // BaseGenerator -- Base Generator class
#include "code.h"                             // Code -- Helper class for generating code
#include "image_handler.h"                    // ImageHandler class
#include "language_traits.h"                  // LanguageTraits, CreateLanguageStrategy
#include "mainframe.h"                        // MainFrame class
#include "node.h"                             // Node class
#include "node_decl.h"                        // NodeDeclaration class
#include "project_handler.h"                  // ProjectHandler class
#include "utils.h"                            // Utility functions that work with properties
#include "write_code.h"                       // Write code to Scintilla or file
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector

using namespace GenEnum;

BaseCodeGenerator::BaseCodeGenerator(GenLang language, Node* form_node) :
    m_header(nullptr),
    m_source(nullptr),
    m_form_node(form_node),
    m_language(language),
    m_strategy(CreateLanguageStrategy(language))
{
}

wxue::string BaseCodeGenerator::GetDeclaration(Node* node)
{
    wxue::string code;
    const wxue::string class_name(node->get_DeclName());

    if (class_name.starts_with("wx"))
    {
        ProcessWxClassDeclaration(class_name, node, code);
    }
    else if (node->HasValue(prop_subclass))
    {
        code << node->as_view(prop_subclass) << "* " << node->get_NodeName() << ';';
    }
    else if (class_name == "CloseButton")
    {
        code << "wxBitmapButton* " << node->get_NodeName() << ';';
    }
    else if (class_name == "StaticCheckboxBoxSizer" || class_name == "StaticRadioBtnBoxSizer")
    {
        ProcessStaticBoxSizerDeclaration(class_name, node, code);
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
        ProcessToolDeclaration(node, code);
    }
    else if (class_name.is_sameas("CustomControl"))
    {
        ProcessCustomClassDeclaration(node, code);
    }
    else if (class_name.is_sameas("dataViewColumn") || class_name.is_sameas("dataViewListColumn"))
    {
        code << "wxDataViewColumn* " << node->get_NodeName() << ';';
    }

    if (node->HasValue(prop_var_comment))
    {
        code << "  // " << node->as_view(prop_var_comment);
    }

    return code;
}

static constexpr frozen::map<GenEnum::PropName, std::string_view, 9> button_map = {
    { prop_OK, "OK" },       { prop_Yes, "Yes" },   { prop_Save, "Save" },
    { prop_Apply, "Apply" }, { prop_No, "No" },     { prop_Cancel, "Cancel" },
    { prop_Close, "Close" }, { prop_Help, "Help" }, { prop_ContextHelp, "ContextHelp" },
};

static void AddStdDialogButtonDeclaration(Node* node, wxue::string& code)
{
    for (const auto& [prop, button_name]: button_map)
    {
        if (node->as_bool(prop))
        {
            code << "\n\twxButton* " << node->get_NodeName() << button_name << ";";
        }
    }
}

void BaseCodeGenerator::ProcessWxClassDeclaration(const wxue::string& class_name, Node* node,
                                                  wxue::string& code)
{
    if (node->HasValue(prop_subclass))
    {
        code << node->as_view(prop_subclass) << "* " << node->get_NodeName() << ';';
    }
    else
    {
        wxue::string adjusted_class_name(class_name);
        if (node->get_Generator()->IsGeneric(node))
        {
            std::ignore = adjusted_class_name.Replace("wx", "wxGeneric");
        }
        code << adjusted_class_name << "* " << node->get_NodeName() << ';';
    }

    if (class_name == "wxStdDialogButtonSizer")
    {
        if (!node->get_Form()->is_Gen(gen_wxDialog) || node->as_bool(prop_Save) ||
            node->as_bool(prop_ContextHelp))
        {
            AddStdDialogButtonDeclaration(node, code);
        }
        return;
    }
    if (class_name == "wxStaticBitmap")
    {
        // If scaling was specified, then we need to switch to wxGenericStaticBitmap in order to
        // support it.
        if (node->as_view(prop_scale_mode) != "None")
        {
            std::ignore = code.Replace("wxStaticBitmap", "wxGenericStaticBitmap");
        }
    }
}

void BaseCodeGenerator::ProcessStaticBoxSizerDeclaration(const wxue::string& class_name, Node* node,
                                                         wxue::string& code)
{
    if (class_name == "StaticCheckboxBoxSizer")
    {
        if (node->HasValue(prop_checkbox_var_name))
        {
            code << "wxCheckBox* " << node->as_view(prop_checkbox_var_name) << ';';
        }
    }
    if (class_name == "StaticRadioBtnBoxSizer")
    {
        if (node->HasValue(prop_radiobtn_var_name))
        {
            code << "wxRadioButton* " << node->as_view(prop_radiobtn_var_name) << ';';
        }
    }

    if (!node->is_Local())
    {
        if (!code.empty())
        {
            code << "\n";
        }
        code << "wxStaticBoxSizer* " << node->get_NodeName() << ';';
    }
}

void BaseCodeGenerator::ProcessToolDeclaration(Node* node, wxue::string& code)
{
    const wxue::string parent_class_name(node->get_Parent()->get_DeclName());
    if (parent_class_name == "wxAuiToolBar")
    {
        code << "wxAuiToolBarItem* " << node->get_NodeName() << ';';
        return;
    }
    if (parent_class_name == "wxToolBar" || parent_class_name == "ToolBar")
    {
        code << "wxToolBarToolBase* " << node->get_NodeName() << ';';
        return;
    }
    FAIL_MSG("Unrecognized class name so no idea how to declare it in the header file.")
}

void BaseCodeGenerator::ProcessCustomClassDeclaration(Node* node, wxue::string& code)
{
    if (auto* node_namespace = node->get_Folder();
        node_namespace && node_namespace->HasValue(prop_folder_namespace))
    {
        code << node_namespace->as_view(prop_folder_namespace) << "::";
    }
    if (node->HasValue(prop_namespace))
    {
        code << node->as_view(prop_namespace) << "::";
    }
    code << node->as_view(prop_class_name) << "* " << node->get_NodeName() << ';';
}

// This is a static function
void BaseCodeGenerator::CollectIDs(Node* node, std::set<std::string>& set_enum_ids,
                                   std::set<std::string>& set_const_ids)
{
    for (const auto& iter: node->get_PropsVector())
    {
        if (iter.type() == type_id)
        {
            const std::string& prop_id = iter.as_string();
            if (!prop_id.empty() && !prop_id.starts_with("wxID_"))
            {
                if (prop_id.find('=') !=
                    std::string_view::npos)  // If it has an assignment operator, it's a constant
                {
                    set_const_ids.insert(prop_id);
                }
                else
                {
                    set_enum_ids.insert(prop_id);
                }
            }
        }
    }

    for (const auto& iter: node->get_ChildNodePtrs())
    {
        CollectIDs(iter.get(), set_enum_ids, set_const_ids);
    }
}

void BaseCodeGenerator::AddConditionalEvent(std::string_view platform, NodeEvent* event)
{
    if (!m_map_conditional_events.contains(platform))
    {
        m_map_conditional_events[platform] = std::vector<NodeEvent*>();
    }

    std::vector<NodeEvent*>& event_vector = m_map_conditional_events[platform];
    if (!std::ranges::any_of(event_vector,
                             [event](const NodeEvent* vector_event)
                             {
                                 return vector_event == event;
                             }))
    {
        event_vector.push_back(event);
    }
}

void BaseCodeGenerator::AddEventToProperContainer(Node* node, NodeEvent* event, EventVector& events)
{
    if (node->get_Parent()->is_Gen(gen_wxContextMenuEvent))
    {
        m_ctx_menu_events.push_back(event);
        return;
    }

    events.push_back(event);
}

void BaseCodeGenerator::ProcessEventHandler(Node* node, NodeEvent* event, EventVector& events)
{
    // Check if node has platform-specific constraint
    if (node->HasProp(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
    {
        AddConditionalEvent(node->as_string(prop_platforms), event);
        return;
    }

    // Check if node is within a platform container
    if (auto* node_container = node->get_PlatformContainer(); node_container)
    {
        AddConditionalEvent(node_container->as_string(prop_platforms), event);
        return;
    }

    // No platform constraints, add to appropriate container
    AddEventToProperContainer(node, event, events);
}

void BaseCodeGenerator::CollectEventHandlers(Node* node, std::vector<NodeEvent*>& events)
{
    ASSERT(node);

    for (auto& iter: node->get_MapEvents())
    {
        // Only add the event if a handler was specified
        if (!iter.second.get_value().empty())
        {
            // Because the NodeEvent* gets stored in a set if there is a conditional, it won't get
            // duplicated even if it is added by both the Node and any container containing the same
            // conditional
            ProcessEventHandler(node, &iter.second, events);
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

bool BaseCodeGenerator::IsEmbeddedImageInCollection(const EmbeddedImage* embed)
{
    const std::lock_guard<std::mutex> lock(m_embedded_images_mutex);
    return std::ranges::any_of(m_embedded_images,
                               [embed](const EmbeddedImage* pimage)
                               {
                                   return pimage == embed;
                               });
}

void BaseCodeGenerator::ProcessEmbeddedImages(const std::vector<wxue::string>& filenames)
{
    size_t processed_count = 0;
    for (const auto& idx_image: filenames)
    {
        if (auto* embed = ProjectImages.GetEmbeddedImage(idx_image); embed)
        {
            if (!IsEmbeddedImageInCollection(embed))
            {
                if (embed->base_image().filename.file_exists())
                {
                    const wxDateTime file_time = embed->base_image().filename.last_write_time();
                    if (file_time != embed->base_image().file_time)
                    {
                        // Update status before the expensive UpdateImage() call
                        ++processed_count;
                        if (auto* frame = wxGetMainFrame(); frame)
                        {
                            if (processed_count == 1 || processed_count % 10 == 0)
                            {
                                frame->setStatusText(
                                    std::format("Processing embedded image {}", processed_count));
                            }
                        }
                        embed->UpdateImage(embed->base_image());
                        embed->base_image().file_time = file_time;
                    }
                    const std::lock_guard<std::mutex> lock(m_embedded_images_mutex);
                    m_embedded_images.emplace_back(embed);
                }
                else
                {
                    MSG_INFO(std::format("Unable to get file time for {}",
                                         embed->base_image().filename.ToStdString()));
                }
            }
        }
    }
    if (processed_count > 0)
    {
        if (auto* frame = wxGetMainFrame(); frame)
        {
            frame->setStatusText(std::format("Processed {} embedded images", processed_count));
        }
    }
}

void BaseCodeGenerator::ProcessHeaderImages(Node* node, const std::vector<wxue::string>& filenames,
                                            std::set<std::string>& embedset)
{
    for (const auto& idx_image: filenames)
    {
        wxue::string path(idx_image);
        wxue::string art_dir = Project.ArtDirectory();
        if (!art_dir.empty())
        {
            wxue::string output_dir = Project.get_BaseDirectory(node, m_language);
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
        embedset.insert(wxue::string() << "#include \"" << path << "\"");
    }
}

void BaseCodeGenerator::ProcessAnimationEmbed(std::string_view value)
{
    wxue::ViewVector parts(value, BMP_PROP_SEPARATOR, wxue::TRIM::both);

    if (!parts[IndexImage].empty())
    {
        const EmbeddedImage* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (!embed)
        {
            if (!ProjectImages.AddEmbeddedImage(parts[IndexImage], m_form_node))
            {
                return;
            }
            embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            if (!embed)
            {
                return;
            }
        }
        else if (IsEmbeddedImageInCollection(embed))
        {
            return;  // we already have this image
        }

        {
            const std::lock_guard<std::mutex> lock(m_embedded_images_mutex);
            m_embedded_images.emplace_back(embed);
        }
    }
}

void BaseCodeGenerator::ProcessAnimationHeaders(std::string_view value, Node* node,
                                                std::set<std::string>& embedset)
{
    wxue::ViewVector parts(value, ';');
    if (parts.size() <= static_cast<size_t>(IndexImage) || parts[IndexImage].empty())
        return;
    if (wxue::is_whitespace(parts[IndexImage].front()))
    {
        parts[IndexImage].remove_prefix(1);
    }
    wxue::string path = parts[IndexImage];
    wxue::string art_dir = Project.ArtDirectory();
    if (!art_dir.empty())
    {
        wxue::string output_dir = Project.get_BaseDirectory(node, m_language);
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
    embedset.insert(wxue::string() << "#include \"" << path << "\"");
}

// This function is called by the thread thrd_collect_img_headers
void BaseCodeGenerator::CollectImageHeaders(Node* node, std::set<std::string>& embedset)
{
    for (const auto& iter: node->get_PropsVector())
    {
        if (!iter.HasValue())
        {
            continue;
        }

        if (iter.type() == type_image)
        {
            if (const auto* bundle = ProjectImages.GetPropertyImageBundle(iter.as_string()); bundle)
            {
                const std::string& value = iter.as_string();
                if (value.starts_with("Embed") || value.starts_with("SVG"))
                {
                    ProcessEmbeddedImages(bundle->lst_filenames);
                    continue;
                }
                if (value.starts_with("Header") || value.starts_with("XPM"))
                {
                    ProcessHeaderImages(node, bundle->lst_filenames, embedset);
                }
            }
        }
        else if (iter.type() == type_animation)
        {
            const std::string& value = iter.as_string();
            if (value.starts_with("Embed"))
            {
                ProcessAnimationEmbed(value);
                continue;
            }
            if (value.starts_with("Header") || value.starts_with("XPM"))
            {
                ProcessAnimationHeaders(value, node, embedset);
            }
        }
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        CollectImageHeaders(child.get(), embedset);
    }
}

void BaseCodeGenerator::ProcessFormIcon(Node* node)
{
    if (node->is_Form() && node->HasValue(prop_icon))
    {
        wxue::ViewVector parts(node->as_string(prop_icon), BMP_PROP_SEPARATOR, wxue::TRIM::both);
        if (parts.size() >= IndexImage + 1)
        {
            // If ProjectImages returns a function name, then the function will be in the
            // Images List header file, so we don't need to generate any functions for it in
            // the source file.
            if (ProjectImages.GetBundleFuncName(node->as_string(prop_icon)).empty())
            {
                if (parts[IndexType] == "Header" || parts[IndexType] == "XPM")
                {
                    if (!wxue::is_sameas(parts[IndexImage].extension(), ".xpm", wxue::CASE::either))
                    {
                        m_NeedHeaderFunction = true;
                    }
                }
                else if (parts[IndexType] == "SVG")
                {
                    m_NeedSVGFunction = true;
                }
                else if (parts[IndexType] == "Embed")
                {
                    m_NeedImageFunction = true;
                }
                else if (parts[IndexType] == "Art")
                {
                    m_NeedArtProviderHeader = true;
                }
            }
        }
    }
}

void BaseCodeGenerator::ProcessChildEmbedType(const wxue::StringVector& parts, bool is_animation)
{
    if (is_animation)
    {
        m_NeedAnimationFunction = true;
        return;
    }

    if (!m_ImagesForm)
    {
        m_NeedImageFunction = true;
        return;
    }

    // If we haven't already encountered an image that requires a function, then check to see
    // if this image is in the Images List file and has a bundle function to access it. If it
    // does, then we still don't need to generate an image function in the class file.
    if (!m_NeedImageFunction)
    {
        if (const auto* bundle = ProjectImages.GetPropertyImageBundle(&parts);
            bundle && !bundle->lst_filenames.empty())
        {
            if (auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
            {
                if (ProjectImages.GetBundleFuncName(embed).empty())
                {
                    m_NeedImageFunction = true;
                }
            }
        }
    }
}

void BaseCodeGenerator::ProcessChildSVGType(const wxue::StringVector& parts,
                                            [[maybe_unused]] bool is_animation)
{
    if (!m_ImagesForm)
    {
        m_NeedSVGFunction = true;
        return;
    }

    if (m_NeedSVGFunction)
    {
        return;
    }

    if (const auto* bundle = ProjectImages.GetPropertyImageBundle(&parts);
        bundle && !bundle->lst_filenames.empty())
    {
        if (auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
        {
            if (ProjectImages.GetBundleFuncName(embed).empty())
            {
                m_NeedSVGFunction = true;
            }
        }
        else
        {
            // Image not found in the embedded images
            m_NeedSVGFunction = true;
        }
    }
    else
    {
        // No bundle found for this SVG, need the function
        m_NeedSVGFunction = true;
    }
}

void BaseCodeGenerator::ProcessChildHeaderType(const wxue::StringVector& parts, bool is_animation)
{
    if (is_animation)
    {
        m_NeedAnimationFunction = true;
        return;
    }
    if (!wxue::is_sameas(parts[IndexImage].extension(), ".xpm", wxue::CASE::either))
    {
        m_NeedHeaderFunction = true;
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
        ProcessFormIcon(node);
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        for (const auto& iter: child->get_PropsVector())
        {
            if ((iter.type() == type_image || iter.type() == type_animation) && iter.HasValue())
            {
                wxue::StringVector parts(iter.as_string(), BMP_PROP_SEPARATOR, wxue::TRIM::both);
                if (parts.size() < IndexImage + 1)
                {
                    continue;
                }

                if (parts[IndexType] == "Embed")
                {
                    ProcessChildEmbedType(parts, iter.type() == type_animation);
                    continue;
                }
                if ((parts[IndexType] == "SVG"))
                {
                    ProcessChildSVGType(parts, iter.type() == type_animation);
                    continue;
                }
                if (parts[IndexType] == "Header")
                {
                    ProcessChildHeaderType(parts, iter.type() == type_animation);
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
        wxue::string code("wxPersistentRegisterAndRestore(");
        code << node->get_NodeName() << ", \"" << node->as_string(prop_persist_name) << "\");";
        m_source->writeLine(code);
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        AddPersistCode(child.get());
    }
}

void BaseCodeGenerator::WriteSetLines(WriteCode* write_code, std::set<std::string>& set_lines)
{
    for (const auto& iter: set_lines)
    {
        write_code->writeLine(iter);
    }
    set_lines.clear();
}

void BaseCodeGenerator::GenContextMenuHandler(Node* node_ctx_menu)
{
    if (auto* generator = node_ctx_menu->get_Generator(); generator)
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
    wxue::string convert(node->as_string(prop));
    std::ignore = convert.Replace("@@", "\n", wxue::REPLACE::all);
    const wxue::ViewVector lines(convert, '\n');
    bool initial_bracket = false;
    for (const auto& code: lines)
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
    if (initial_bracket)
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
        }

        if (form->is_Gen(gen_Images))
        {
            m_ImagesForm = form.get();
            break;
        }
    }
}
