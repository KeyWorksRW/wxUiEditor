/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith or XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-15-2026]

#include <algorithm>
#include <filesystem>
#include <format>
#include <map>
#include <vector>

#include "import_wxsmith.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "dlg_msgs.h"        // wxMessageDialog dialogs
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "utils.h"           // Utility functions that work with properties

// wxWidgets XRC allows an <object_ref> element anywhere an <object> element may appear. XRC
// replaces the reference with a copy of the named object, so the import expands each reference into
// a copy of the target element instead of discarding it (wxUiEditor has no reference support).

static constexpr int MAX_OBJECT_REF_DEPTH = 20;

static bool IsXrcElement(const pugi::xml_node& node, std::string_view name)
{
    return node.type() == pugi::node_element &&
           wxue::is_sameas(node.name(), name, wxue::CASE::either);
}

// Recursively index every named <object> element in document order. The first definition of a name
// wins; later duplicates are ignored.
static void IndexNamedObjects(pugi::xml_node parent,
                              std::map<std::string, pugi::xml_node>& named_objects)
{
    for (pugi::xml_node child: parent.children())
    {
        if (child.type() != pugi::node_element)
        {
            continue;
        }

        if (IsXrcElement(child, "object"))
        {
            const std::string name(child.attribute("name").value());
            if (!name.empty())
            {
                named_objects.try_emplace(name, child);
            }
        }

        IndexNamedObjects(child, named_objects);
    }
}

// Attributes on the <object_ref> other than "ref" override or add to the referenced element. This
// is how <object_ref name="alias" ref="template"/> gives the copy its own name.
static void MergeRefAttributes(pugi::xml_node clone, const pugi::xml_node& ref_element)
{
    for (pugi::xml_attribute ref_attr: ref_element.attributes())
    {
        if (wxue::is_sameas(ref_attr.name(), "ref", wxue::CASE::either))
        {
            continue;
        }

        pugi::xml_attribute clone_attr = clone.attribute(ref_attr.name());
        if (clone_attr)
        {
            clone_attr.set_value(ref_attr.value());
        }
        else
        {
            clone.append_attribute(ref_attr.name()).set_value(ref_attr.value());
        }
    }
}

static pugi::xml_node FindMatchingChild(const pugi::xml_node& clone,
                                        const pugi::xml_node& override_node)
{
    const pugi::xml_attribute override_name = override_node.attribute("name");

    for (pugi::xml_node child: clone.children())
    {
        if (!IsXrcElement(child, override_node.name()))
        {
            continue;
        }

        if (!override_name ||
            wxue::is_sameas(child.attribute("name").value(), override_name.value()))
        {
            return child;
        }
    }

    return {};
}

// Approximates XRC's child merge rules: a child of the <object_ref> replaces an existing child of
// the copy that has the same element name (and the same name attribute when the override carries
// one); anything else is appended, or prepended when it specifies insert_at="begin".
static void MergeRefChildren(pugi::xml_node clone, const pugi::xml_node& ref_element)
{
    // Collect the override children first -- inserting and removing nodes would otherwise
    // invalidate the iterator.
    std::vector<pugi::xml_node> overrides;
    for (pugi::xml_node child: ref_element.children())
    {
        if (child.type() == pugi::node_element)
        {
            overrides.push_back(child);
        }
    }

    for (pugi::xml_node override_node: overrides)
    {
        pugi::xml_node copy;
        const pugi::xml_node match = FindMatchingChild(clone, override_node);
        if (match)
        {
            copy = clone.insert_copy_before(override_node, match);
            clone.remove_child(match);
        }
        else if (wxue::is_sameas(override_node.attribute("insert_at").value(), "begin",
                                 wxue::CASE::either))
        {
            copy = clone.prepend_copy(override_node);
        }
        else
        {
            copy = clone.append_copy(override_node);
        }

        if (copy)
        {
            std::ignore = copy.remove_attribute("insert_at");
        }
    }
}

WxSmith::WxSmith() = default;

bool WxSmith::Import(const std::string& filename, bool write_doc, bool allow_ui)
{
    std::optional<pugi::xml_document> result = LoadDocFile(filename, allow_ui);
    if (!result)
    {
        return false;
    }
    pugi::xml_node root = result.value().first_child();

    if (!wxue::is_sameas(root.name(), "wxsmith", wxue::CASE::either) &&
        !wxue::is_sameas(root.name(), "resource", wxue::CASE::either))
    {
        if (allow_ui)
        {
            dlgInvalidProject(filename, "wxSmith or XRC", "Import project");
        }
        return false;
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid
    // and we cannot recover, then we can throw an error and give a standard response about an
    // invalid file.

    try
    {
        // Must be done before any node is created -- XRC <object_ref> elements are expanded in
        // place so that CreateXrcNode() only ever sees <object> elements.
        ExpandObjectRefs(root);

        m_project = NodeCreation.CreateNode(gen_Project, nullptr).first;
        for (auto& iter: root.children())
        {
            CreateXrcNode(iter, m_project.get());
        }

        if (!m_project->get_ChildCount())
        {
            if (allow_ui)
            {
                wxMessageBox(wxString() << wxString::FromUTF8(filename)
                                        << " does not contain any top level forms.",
                             "Import");
            }
            return false;
        }

        if (write_doc)
        {
            m_project->CreateDoc(m_docOut);
        }
    }

    catch (const std::exception& err)
    {
        MSG_ERROR(err.what());
        if (allow_ui)
        {
            dlgImportError(err, filename, "Import Project");
        }
        return false;
    }

    if (!m_errors.empty())
    {
        std::string errMsg("Not everything in the project could be converted:\n\n");
        MSG_ERROR(std::format("------  {}------",
                              std::filesystem::path(m_importProjectFile).filename().string()));
        for (const auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg += iter;
            errMsg += '\n';
        }

        if (allow_ui)
        {
            wxMessageDialog dlg_msg(nullptr, wxString::FromUTF8(errMsg), "Import Project",
                                    wxICON_WARNING | wxOK);
            dlg_msg.ShowModal();
        }
    }

    return true;
}

void WxSmith::ExpandObjectRefs(pugi::xml_node root)
{
    std::map<std::string, pugi::xml_node> named_objects;
    IndexNamedObjects(root, named_objects);

    const std::vector<std::string> ref_chain;
    ExpandObjectRefsIn(root, named_objects, ref_chain, 0);
}

void WxSmith::ExpandObjectRefsIn(pugi::xml_node parent,
                                 const std::map<std::string, pugi::xml_node>& named_objects,
                                 const std::vector<std::string>& ref_chain, int depth)
{
    // Snapshot the children before mutating the tree -- removing an <object_ref> invalidates its
    // handle, and inserting a copy shifts the sibling list.
    std::vector<pugi::xml_node> ref_elements;
    std::vector<pugi::xml_node> other_elements;
    for (pugi::xml_node child: parent.children())
    {
        if (child.type() != pugi::node_element)
        {
            continue;
        }

        if (IsXrcElement(child, "object_ref"))
        {
            ref_elements.push_back(child);
        }
        else
        {
            other_elements.push_back(child);
        }
    }

    for (pugi::xml_node ref_element: ref_elements)
    {
        const std::string ref_name(ref_element.attribute("ref").value());
        if (ref_name.empty())
        {
            m_errors.insert("Unable to convert XRC object_ref: missing 'ref' attribute.");
            parent.remove_child(ref_element);
            continue;
        }

        const std::map<std::string, pugi::xml_node>::const_iterator object_iter =
            named_objects.find(ref_name);
        if (object_iter == named_objects.end())
        {
            m_errors.insert(std::format(
                "Unable to convert XRC object_ref '{}': no matching named object.", ref_name));
            parent.remove_child(ref_element);
            continue;
        }

        if (depth >= MAX_OBJECT_REF_DEPTH ||
            std::ranges::find(ref_chain, ref_name) != ref_chain.end())
        {
            m_errors.insert(std::format(
                "Unable to convert XRC object_ref '{}': circular reference.", ref_name));
            parent.remove_child(ref_element);
            continue;
        }

        pugi::xml_node clone = parent.insert_copy_before(object_iter->second, ref_element);
        if (!clone)
        {
            m_errors.insert(std::format(
                "Unable to convert XRC object_ref '{}': unable to copy the referenced object.",
                ref_name));
            parent.remove_child(ref_element);
            continue;
        }

        MergeRefAttributes(clone, ref_element);
        MergeRefChildren(clone, ref_element);
        parent.remove_child(ref_element);

        std::vector<std::string> child_chain = ref_chain;
        child_chain.push_back(ref_name);
        ExpandObjectRefsIn(clone, named_objects, child_chain, depth + 1);
    }

    for (pugi::xml_node child: other_elements)
    {
        ExpandObjectRefsIn(child, named_objects, ref_chain, depth);
    }
}

bool WxSmith::HandleUnknownProperty(const pugi::xml_node& xml_obj, Node* node,
                                    Node* /* parent unused */)
{
    std::string_view node_name = xml_obj.name();

    if (node_name == "id_arg" || node_name == "pos_arg" || node_name == "size_arg")
    {
        return true;
    }
    if (node_name == "labelrowheight")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            std::ignore = node->set_value(prop_default_row_size, xml_obj.text().as_int());
            return true;
        }
    }
    else if (node_name == "labelcolwidth")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            // wxGrid doesn't have a default column width, so we'll ignore this property
            return true;
        }
    }
    else if (node_name == "defaultcolsize")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            std::ignore = node->set_value(prop_default_col_size, xml_obj.text().as_int());
            return true;
        }
    }
    else if (node_name == "collabels")
    {
        if (node->is_Gen(gen_wxGrid))
        {
            wxString choices;
            for (const auto& iter: xml_obj.children())
            {
                if (iter.name() == "item")
                {
                    wxString child = iter.child_as_str();
                    child.Replace("\"", "\\\"", true);
                    if (!choices.empty())
                    {
                        choices << " ";
                    }
                    choices << '\"' << child << '\"';
                }
            }

            if (!choices.empty())
            {
                std::ignore = node->set_value(prop_col_label_values, choices);
            }
            return true;
        }
    }
    else if (node_name == "col" && node->is_Gen(gen_gbsizeritem))
    {
        std::ignore = node->set_value(prop_column, xml_obj.text().as_int());
        return true;
    }
    else if (node_name == "val")
    {
        std::ignore = node->set_value(prop_validator_variable, xml_obj.text().as_view());
        return true;
    }
    return false;
}
