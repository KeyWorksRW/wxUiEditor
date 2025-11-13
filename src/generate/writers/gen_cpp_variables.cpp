/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++ code generation variable and validator functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_cpp.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "gen_common.h"      // Common component functions
#include "utils.h"           // Miscellaneous utilities
#include "write_code.h"      // Write code to Scintilla or file

void CppCodeGenerator::CollectMemberVariables(Node* node, Permission perm,
                                              std::set<std::string>& code_lines)
{
    if (auto* prop = node->get_PropPtr(prop_class_access); prop)
    {
        if (prop->as_string() != "none")
        {
            if ((perm == Permission::Public && prop->as_string() == "public:") ||
                (perm == Permission::Protected && prop->as_string() == "protected:"))
            {
                auto code = GetDeclaration(node);

                auto ChangeClass = [&](const std::string& generic_class)
                {
                    if (auto pos = code.find('*'); pos != std::string::npos)
                    {
                        code = generic_class + code.substr(pos);
                    }
                };

                if (node->is_Gen(gen_wxAnimationCtrl))
                {
                    if ((node->HasValue(prop_animation) &&
                         node->as_string(prop_animation).contains(".ani", tt::CASE::either)) ||
                        node->as_string(prop_subclass).starts_with("wxGeneric"))
                    {
                        // The generic version is required to display .ANI files on wxGTK.
                        ChangeClass("wxGenericAnimationCtrl");
                    }
                }
                else if (node->is_Gen(gen_wxHyperlinkCtrl))
                {
                    if (!node->as_bool(prop_underlined) ||
                        node->as_string(prop_subclass).starts_with("wxGeneric"))
                    {
                        // If the underlined property is false, we need to use the generic
                        // version.
                        ChangeClass("wxGenericHyperlinkCtrl");
                    }
                }
                else if (node->is_Gen(gen_wxStaticBitmap))
                {
                    if (node->as_string(prop_scale_mode) != "None" ||
                        node->as_string(prop_subclass).starts_with("wxGeneric"))
                    {
                        // If we are using a scale mode, we must use wxGenericStaticBitmap.
                        ChangeClass("wxGenericStaticBitmap");
                    }
                }
                else if (node->is_Gen(gen_wxStaticText))
                {
                    if (node->as_string(prop_subclass).starts_with("wxGeneric") ||
                        (node->as_bool(prop_markup) && node->as_int(prop_wrap) <= 0))
                    {
                        // If we are using markup or the wrap is <= 0, we must use
                        // wxGenericStaticText.
                        ChangeClass("wxGenericStaticText");
                    }
                }
                else if (node->is_Gen(gen_wxCalendarCtrl))
                {
                    if (node->as_string(prop_subclass).starts_with("wxGeneric"))
                    {
                        ChangeClass("wxGenericCalendarCtrl");
                    }
                }
                else if (node->is_Gen(gen_wxTreeCtrl))
                {
                    if (node->as_string(prop_subclass).starts_with("wxGeneric"))
                    {
                        ChangeClass("wxGenericTreeCtrl");
                    }
                }

                else if (node->is_Gen(gen_wxTimer))
                {
                    // Remove the pointer, wxTimer needs to be a class.
                    code.Replace("*", "");
                }

                if (code.empty() && node->is_Gen(gen_auitool))
                {
                    code += "wxAuiToolBarItem* " + node->as_string(prop_var_name) + ';';
                }

                if (code.size())
                {
                    if (node->HasProp(prop_platforms) &&
                        node->as_string(prop_platforms) != "Windows|Unix|Mac")
                    {
                        if (perm == Permission::Public)
                        {
                            if (!m_map_public_members.contains(node->as_string(prop_platforms)))
                            {
                                m_map_public_members[node->as_string(prop_platforms)] =
                                    std::set<tt_string>();
                            }
                            m_map_public_members[node->as_string(prop_platforms)].insert(code);
                        }
                        else
                        {
                            if (!m_map_protected.contains(node->as_string(prop_platforms)))
                            {
                                m_map_protected[node->as_string(prop_platforms)] =
                                    std::set<tt_string>();
                            }
                            m_map_protected[node->as_string(prop_platforms)].insert(code);
                        }
                    }
                    // If node_container is non-null, it means the current node is within a
                    // container that has a conditional.
                    else if (auto* node_container = node->get_PlatformContainer(); node_container)
                    {
                        if (perm == Permission::Public)
                        {
                            if (!m_map_public_members.contains(
                                    node_container->as_string(prop_platforms)))
                            {
                                m_map_public_members[node_container->as_string(prop_platforms)] =
                                    std::set<tt_string>();
                            }
                            m_map_public_members[node_container->as_string(prop_platforms)].insert(
                                code);
                        }
                        else
                        {
                            if (!m_map_protected.contains(
                                    node_container->as_string(prop_platforms)))
                            {
                                m_map_protected[node_container->as_string(prop_platforms)] =
                                    std::set<tt_string>();
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
        if (auto* generator = node->get_Generator(); generator)
        {
            generator->CollectMemberVariables(node, code_lines);
        }

        // StaticCheckboxBoxSizer and StaticRadioBtnBoxSizer have internal variables
        if (node->HasValue(prop_checkbox_var_name) || node->HasValue(prop_radiobtn_var_name))
        {
            auto code = GetDeclaration(node);
            if (code.size())
            {
                if (node->HasProp(prop_platforms) &&
                    node->as_string(prop_platforms) != "Windows|Unix|Mac")
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

    for (const auto& child: node->get_ChildNodePtrs())
    {
        CollectMemberVariables(child.get(), perm, code_lines);
    }
}

void CppCodeGenerator::CollectValidatorVariables(Node* node, std::set<std::string>& code_lines)
{
    GenCppValVarsBase(node->get_NodeDeclaration(), node, code_lines);

    for (const auto& child: node->get_ChildNodePtrs())
    {
        CollectValidatorVariables(child.get(), code_lines);
    }
}

void CppCodeGenerator::GenCppValidatorFunctions(Node* node)
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (node->HasValue(prop_validator_variable))
    {
        auto result = GenGetSetCode(node);
        if (result)
        {
            m_header->writeLine(result.value());
        }
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        GenCppValidatorFunctions(child.get());
    }
}

void CppCodeGenerator::GenCppValVarsBase(const NodeDeclaration* declaration, Node* node,
                                         std::set<std::string>& code_lines)
{
    ASSERT(m_language == GEN_LANG_CPLUSPLUS);

    if (const auto& var_name = node->as_string(prop_validator_variable); var_name.size())
    {
        if (auto val_data_type = node->get_ValidatorDataType(); val_data_type.size())
        {
            tt_string code;
            code << val_data_type << ' ' << var_name;

            if (val_data_type == "bool")
            {
                auto prop = node->get_PropPtr(prop_checked);
                bool bState = (prop && prop->as_bool());
                if (!prop)
                {
                    prop = node->get_PropPtr(prop_initial_state);
                    if (prop && prop->as_string() == "wxCHK_CHECKED")
                    {
                        bState = true;
                    }
                }
                code << " { " << (bState ? "true" : "false") << " };";
            }
            else if (val_data_type.find("int") != std::string::npos ||
                     val_data_type.find("short") != std::string::npos ||
                     val_data_type.find("long") != std::string::npos ||
                     val_data_type.find("double") != std::string::npos ||
                     val_data_type.find("float") != std::string::npos)
            {
                auto* prop = node->get_PropPtr(prop_value);
                if (!prop)
                {
                    prop = node->get_PropPtr(prop_initial);
                }
                if (!prop)
                {
                    prop = node->get_PropPtr(prop_selection);
                }
                if (!prop)
                {
                    prop = node->get_PropPtr(prop_position);
                }
                if (prop && prop->as_string().size())
                {
                    code << " { " << prop->as_string() << " };";
                }
                else
                {
                    code << ';';
                }
            }
            else if (val_data_type == "wxString" || val_data_type == "wxFileName")
            {
                const auto& value = node->as_string(prop_value);
                if (value.size())
                {
                    code << " { " << GenerateQuotedString(value) << " };";
                }
                else
                {
                    code << ';';
                }
            }
            // BUGBUG: [Randalphwa - 07-31-2023] We need to handle wxArrayInt

            // REVIEW: [Randalphwa - 08-30-2025] I *think* the bug is still valid since we do have a
            // wxArrayInt val_data_type, however for 3.3.x code, it needs to be replaced with std::
            // containers.
            else
            {
                code << ';';
            }

            // Validator variables are always written to the protected: section even if the
            // node variable is marked as public:

            if (node->HasProp(prop_platforms) &&
                node->as_string(prop_platforms) != "Windows|Unix|Mac")
            {
                if (!m_map_protected.contains(node->as_string(prop_platforms)))
                {
                    m_map_protected[node->as_string(prop_platforms)] = std::set<tt_string>();
                }
                m_map_protected[node->as_string(prop_platforms)].insert(code);
            }
            // If node_container is non-null, it means the current node is within a container
            // that has a conditional.
            else if (auto* node_container = node->get_PlatformContainer(); node_container)
            {
                if (!m_map_protected.contains(node_container->as_string(prop_platforms)))
                {
                    m_map_protected[node_container->as_string(prop_platforms)] =
                        std::set<tt_string>();
                }
                m_map_protected[node_container->as_string(prop_platforms)].insert(code);
            }
            else
            {
                code_lines.insert(code);
            }
        }
    }

    for (size_t i = 0; i < declaration->GetBaseClassCount(false); i++)
    {
        GenCppValVarsBase(declaration->GetBaseClass(i, false), node, code_lines);
    }
}
