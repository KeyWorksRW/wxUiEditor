/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top level Object construction code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_base.h"    // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "node_decl.h"   // NodeDeclaration class
#include "write_code.h"  // Write code to Scintilla or file

// clang-format off

// These are the types that need to have generator->GenAdditionalCode() called after the type is constructed
GenType aftercode_types[] = {
    type_menubar,
    type_menu,
    type_submenu,
    type_tool,
    type_listbook,
    type_simplebook,
    type_notebook,
    type_auinotebook,
    type_treelistctrl,
    gen_type_unknown
};
// clang-format on

void BaseCodeGenerator::GenConstruction(Node* node)
{
    auto type = node->gen_type();
    auto declaration = node->GetNodeDeclaration();
    auto generator = declaration->GetGenerator();
    if (!generator)
        return;

    if (node->HasValue(prop_platforms) && node->value(prop_platforms) != "Windows|Unix|Mac")
    {
        BeginPlatformCode(node);
    }

    bool need_closing_brace = false;

    auto result = generator->GenConstruction(node, m_language);
    if (!result)
    {
        if (m_language == GEN_LANG_CPLUSPLUS)
            result = generator->GenConstruction(node);
        else if (m_language == GEN_LANG_PYTHON)
            result = generator->GenPythonConstruction(node);
        else if (m_language == GEN_LANG_LUA)
            result = generator->GenLuaConstruction(node);
        else if (m_language == GEN_LANG_PHP)
            result = generator->GenPhpConstruction(node);
    }

    if (result)
    {
        // Don't add blank lines when adding tools to a toolbar
        if (type != type_aui_tool && type != type_tool)
        {
            m_source->writeLine();
        }

        if (m_language == GEN_LANG_CPLUSPLUS)
        {  // Some code generation may put added lines in a { } block, in which case we need to keep indents.
            m_source->writeLine(result.value(), (ttlib::is_found(result.value().find('{')) ||
                                                 ttlib::is_found(result.value().find("\n\t\t"))) ?
                                                    indent::none :
                                                    indent::auto_no_whitespace);
            if (result.value().starts_with("\t{"))
            {
                need_closing_brace = true;
            }
        }
        else
        {
            m_source->writeLine(result.value());
        }
    }
    GenSettings(node);

    if (type == type_ribbontoolbar)
    {
        BeginBrace();
        // A wxRibbonToolBar can only have abstract children that consist of the tools.
        for (const auto& child: node->GetChildNodePtrs())
        {
            auto child_generator = child->GetNodeDeclaration()->GetGenerator();
            result = child_generator->GenConstruction(child.get(), m_language);
            if (!result)
            {
                if (m_language == GEN_LANG_CPLUSPLUS)
                    result = child_generator->GenConstruction(child.get());
                else if (m_language == GEN_LANG_PYTHON)
                    result = child_generator->GenPythonConstruction(child.get());
                else if (m_language == GEN_LANG_LUA)
                    result = child_generator->GenLuaConstruction(child.get());
                else if (m_language == GEN_LANG_PHP)
                    result = child_generator->GenPhpConstruction(child.get());
            }

            if (result)
                m_source->writeLine(result.value());
        }
        EndBrace();
        m_source->writeLine(ttlib::cstr() << node->get_node_name(m_language) << LangPtr() << "Realize();");
        return;
    }
    else if (type == type_tool_dropdown && node->GetChildCount())
    {
        BeginBrace();
        m_source->writeLine("wxMenu* menu = new wxMenu;");
        auto menu_node_ptr = g_NodeCreator.NewNode(gen_wxMenu);
        menu_node_ptr->prop_set_value(prop_var_name, "menu");
        for (const auto& child: node->GetChildNodePtrs())
        {
            auto old_parent = child->GetParent();
            child->SetParent(menu_node_ptr.get());
            auto child_generator = child->GetNodeDeclaration()->GetGenerator();

            result = child_generator->GenConstruction(child.get(), m_language);
            if (!result)
            {
                if (m_language == GEN_LANG_CPLUSPLUS)
                    result = child_generator->GenConstruction(child.get());
                else if (m_language == GEN_LANG_PYTHON)
                    result = child_generator->GenPythonConstruction(child.get());
                else if (m_language == GEN_LANG_LUA)
                    result = child_generator->GenLuaConstruction(child.get());
                else if (m_language == GEN_LANG_PHP)
                    result = child_generator->GenPhpConstruction(child.get());
            }
            if (result)
                m_source->writeLine(result.value());

            GenSettings(child.get());
            // A submenu can have children
            if (child->GetChildCount())
            {
                for (const auto& grandchild: child->GetChildNodePtrs())
                {
                    auto grandchild_generator = grandchild->GetNodeDeclaration()->GetGenerator();
                    result = grandchild_generator->GenConstruction(grandchild.get(), m_language);
                    if (!result)
                    {
                        if (m_language == GEN_LANG_CPLUSPLUS)
                            result = grandchild_generator->GenConstruction(grandchild.get());
                        else if (m_language == GEN_LANG_PYTHON)
                            result = grandchild_generator->GenPythonConstruction(grandchild.get());
                        else if (m_language == GEN_LANG_LUA)
                            result = grandchild_generator->GenLuaConstruction(grandchild.get());
                        else if (m_language == GEN_LANG_PHP)
                            result = grandchild_generator->GenPhpConstruction(grandchild.get());
                    }
                    if (result)
                        m_source->writeLine(result.value());
                    GenSettings(grandchild.get());
                    // A submenu menu item can also be a submenu with great grandchildren.
                    if (grandchild->GetChildCount())
                    {
                        for (const auto& great_grandchild: grandchild->GetChildNodePtrs())
                        {
                            auto great_grandchild_generator = great_grandchild->GetNodeDeclaration()->GetGenerator();
                            result = great_grandchild_generator->GenConstruction(great_grandchild.get(), m_language);
                            if (!result)
                            {
                                if (m_language == GEN_LANG_CPLUSPLUS)
                                    result = great_grandchild_generator->GenConstruction(great_grandchild.get());
                                else if (m_language == GEN_LANG_PYTHON)
                                    result = great_grandchild_generator->GenPythonConstruction(great_grandchild.get());
                                else if (m_language == GEN_LANG_LUA)
                                    result = great_grandchild_generator->GenLuaConstruction(great_grandchild.get());
                                else if (m_language == GEN_LANG_PHP)
                                    result = great_grandchild_generator->GenPhpConstruction(great_grandchild.get());
                            }
                            if (result)
                                m_source->writeLine(result.value());
                            GenSettings(great_grandchild.get());
                            // It's possible to have even more levels of submenus, but we'll stop here.
                        }
                    }
                }
            }
            child->SetParent(old_parent);
        }
        m_source->writeLine(ttlib::cstr() << node->get_node_name(m_language) << LangPtr() << "SetDropdownMenu(menu);");
        EndBrace();
        return;
    }

    auto parent = node->GetParent();

    if (GenAfterChildren(node, need_closing_brace))
        return;

    if (parent->IsSizer())
    {
        GenParentSizer(node, need_closing_brace);
    }
    else if (parent->IsToolBar() && !node->isType(type_tool) && !node->isType(type_tool_separator))
    {
        ttlib::cstr code;
        if (parent->isType(type_toolbar_form))
            code << "AddControl(" << node->prop_as_string(prop_var_name) << ");";
        else
            code << parent->prop_as_string(prop_var_name) << LangPtr() << "AddControl("
                 << node->prop_as_string(prop_var_name) << ");";
        m_source->writeLine(code);
    }
    else if (node->gen_type() == type_widget && parent->isGen(gen_wxChoicebook))
    {
        ttlib::cstr code;
        code << parent->get_node_name(m_language) << LangPtr() << "GetControlSizer()" << LangPtr() << "Add("
             << node->get_node_name(m_language);
        code << ", wxSizerFlags().Expand().Border(wxALL));";
        m_source->writeLine(code);
    }

    if (node->isGen(gen_PageCtrl) && node->GetChildCount())
    {
        // type_page will have already constructed the code for the child. However, we still need to generate
        // settings and process any grandchildren.

        auto page_child = node->GetChild(0);
        if (page_child)
        {
            GenSettings(page_child);

            for (const auto& child: page_child->GetChildNodePtrs())
            {
                GenConstruction(child.get());
            }
        }
    }
    else
    {
        for (const auto& child: node->GetChildNodePtrs())
        {
            GenConstruction(child.get());
        }
    }

    if (node->IsSizer())
    {
        if (!parent->IsSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
        {
            // The parent node is not a sizer -- which is expected if this is the parent sizer underneath a form or
            // wxPanel.

            ttlib::cstr code;
            if (parent->isGen(gen_wxRibbonPanel))
            {
                code << parent->get_node_name(m_language) << LangPtr() << "SetSizerAndFit("
                     << node->get_node_name(m_language) << ");";
            }
            else
            {
                if (GetParentName(node) != "this")
                    code << GetParentName(node) << LangPtr();
                code << "SetSizerAndFit(" << node->get_node_name(m_language) << ");";
            }

            m_source->writeLine();
            m_source->writeLine(code);
        }
    }
    else if (type == type_splitter)
    {
        ttlib::cstr code(node->get_node_name(m_language));

        if (node->GetChildCount() == 1)
        {
            code << LangPtr() << "Initialize(" << node->GetChild(0)->get_node_name(m_language) << ");";
            m_source->writeLine(code);
        }
        else if (node->GetChildCount() > 1)
        {
            if (node->prop_as_string(prop_splitmode) == "wxSPLIT_VERTICAL")
                code << LangPtr() << "SplitVertically(";
            else
                code << LangPtr() << "SplitHorizontally(";

            code << node->GetChild(0)->get_node_name(m_language) << ", " << node->GetChild(1)->get_node_name(m_language)
                 << ");";
            m_source->writeLine(code);

            if (auto sash_pos = node->get_prop_ptr(prop_sashpos)->as_int(); sash_pos != 0 && sash_pos != -1)
            {
                code = node->get_node_name(m_language);
                code << LangPtr() << "SetSashPosition(" << node->prop_as_string(prop_sashpos) << ");";
                m_source->writeLine(code);
            }
        }
    }

    else
    {
        for (size_t idx = 0; aftercode_types[idx] != gen_type_unknown; ++idx)
        {
            if (type == aftercode_types[idx])
            {
                result = generator->GenAdditionalCode(code_after_children, node, m_language);
                if (!result)
                {
                    if (m_language == GEN_LANG_CPLUSPLUS)
                        result = generator->GenAdditionalCode(code_after_children, node);
                    else if (m_language == GEN_LANG_PYTHON)
                        result = generator->GenPythonAdditionalCode(code_after_children, node);
                    else if (m_language == GEN_LANG_LUA)
                        result = generator->GenLuaAdditionalCode(code_after_children, node);
                    else if (m_language == GEN_LANG_PHP)
                        result = generator->GenPhpAdditionalCode(code_after_children, node);
                }

                if (result && result.value().size())
                {
                    m_source->writeLine(result.value(), indent::none);
                }
                m_source->writeLine();
                break;
            }
        }
    }

    // A wxRibbonBar needs to be realized after all children have been created

    if (node->isGen(gen_wxRibbonBar))
    {
        m_source->writeLine(ttlib::cstr() << node->get_node_name(m_language) << LangPtr() << "Realize();");
    }

    if (node->HasValue(prop_platforms) && node->value(prop_platforms) != "Windows|Unix|Mac")
    {
        EndPlatformCode();
    }
}

const char* BaseCodeGenerator::LangPtr() const
{
    switch (m_language)
    {
        case GEN_LANG_CPLUSPLUS:
        case GEN_LANG_PHP:
            return "->";

        case GEN_LANG_PYTHON:
            return ".";

        case GEN_LANG_LUA:
            return ":";

        default:
            FAIL_MSG("Unsupported language!")
            return "";
    }
}

void BaseCodeGenerator::BeginPlatformCode(Node* node)
{
    ttlib::cstr code;
    if (node->value(prop_platforms).contains("Windows"))
    {
        switch (m_language)
        {
            case GEN_LANG_CPLUSPLUS:
            case GEN_LANG_PHP:
                code << "\n#if defined(__WINDOWS__)";
                break;

            case GEN_LANG_PYTHON:
                code << "\nif defined(__WINDOWS__)";
                break;

            case GEN_LANG_LUA:
                code << "\n#ifdef __WINDOWS__";
                break;
        }
    }
    if (node->value(prop_platforms).contains("Unix"))
    {
        switch (m_language)
        {
            case GEN_LANG_CPLUSPLUS:
            case GEN_LANG_PHP:
                if (code.size())
                    code << " || ";
                else
                    code << "\n#if ";
                code << "defined(__UNIX__)";
                break;

            case GEN_LANG_PYTHON:
                if (code.size())
                    code << " || ";
                else
                    code << "\nif ";
                code << "defined(__UNIX__)";
                break;

            case GEN_LANG_LUA:
                if (code.empty())
                    code << "\n#ifdef __UNIX__";
                break;
        }
    }
    if (node->value(prop_platforms).contains("Mac"))
    {
        switch (m_language)
        {
            case GEN_LANG_CPLUSPLUS:
            case GEN_LANG_PHP:
                if (code.size())
                    code << " || ";
                else
                    code << "\n#if ";
                code << "defined(__WXOSX__)";
                break;

            case GEN_LANG_PYTHON:
                if (code.size())
                    code << " || ";
                else
                    code << "\nif ";
                code << "defined(__WXOSX__)";
                break;

            case GEN_LANG_LUA:
                if (code.empty())
                    code << "\n#ifdef __WXOSX__";
                break;
        }
    }

    m_source->writeLine(code);
    m_source->SetLastLineBlank();
    if (m_language == GEN_LANG_PYTHON || m_language == GEN_LANG_LUA)
        m_source->Indent();
}

void BaseCodeGenerator::EndPlatformCode()
{
    switch (m_language)
    {
        case GEN_LANG_CPLUSPLUS:
        case GEN_LANG_PHP:
            m_source->writeLine("#endif  // limited to specific platforms");
            break;

        case GEN_LANG_PYTHON:
            m_source->Unindent();
            break;

        case GEN_LANG_LUA:
            m_source->Unindent();
            m_source->writeLine("#endif  // limited to specific platforms\n");
            break;
    }
}

void BaseCodeGenerator::BeginBrace()
{
    if (m_language == GEN_LANG_CPLUSPLUS)
        m_source->writeLine("{");
    m_source->Indent();
}

void BaseCodeGenerator::EndBrace()
{
    m_source->Unindent();
    if (m_language == GEN_LANG_CPLUSPLUS)
        m_source->writeLine("}");
}

void BaseCodeGenerator::GenSettings(Node* node)
{
    size_t auto_indent = indent::auto_no_whitespace;
    auto generator = node->GetNodeDeclaration()->GetGenerator();

    switch (m_language)
    {
        case GEN_LANG_CPLUSPLUS:
            if (auto result = generator->GenSettings(node, auto_indent); result && result.value().size())
            {
                m_source->writeLine(result.value(), auto_indent);
            }
            break;

        case GEN_LANG_PYTHON:
            if (auto result = generator->GenPythonSettings(node, auto_indent); result && result.value().size())
            {
                m_source->writeLine(result.value(), auto_indent);
            }
            break;

        case GEN_LANG_LUA:
            if (auto result = generator->GenLuaSettings(node, auto_indent); result && result.value().size())
            {
                m_source->writeLine(result.value(), auto_indent);
            }
            break;

        case GEN_LANG_PHP:
            if (auto result = generator->GenPhpSettings(node, auto_indent); result && result.value().size())
            {
                m_source->writeLine(result.value(), auto_indent);
            }
            break;

        default:
            FAIL_MSG("Unknown language")
            break;
    }

    if (node->get_prop_ptr(prop_window_extra_style))
    {
        ttlib::cstr code;
        if (m_language == GEN_LANG_CPLUSPLUS)
        {
            if (auto result = GenValidatorSettings(node); result)
            {
                m_source->writeLine(result.value());
            }
        }
        switch (m_language)
        {
            case GEN_LANG_CPLUSPLUS:
                GenerateWindowSettings(node, code);
                break;

            case GEN_LANG_PYTHON:
                GeneratePythonWindowSettings(node, code);
                break;

            case GEN_LANG_LUA:
                GenerateLuaWindowSettings(node, code);
                break;

            case GEN_LANG_PHP:
                GeneratePhpWindowSettings(node, code);
                break;

            default:
                FAIL_MSG("Unknown language")
                break;
        }

        if (code.size())
            m_source->writeLine(code, indent::auto_keep_whitespace);
    }
}

bool BaseCodeGenerator::GenAfterChildren(Node* node, bool need_closing_brace)
{
    auto declaration = node->GetNodeDeclaration();
    auto generator = declaration->GetGenerator();

    auto result = generator->GenAfterChildren(node, m_language);
    if (!result)
    {
        if (m_language == GEN_LANG_CPLUSPLUS)
            result = generator->GenAfterChildren(node);
        else if (m_language == GEN_LANG_PYTHON)
            result = generator->GenPythonAfterChildren(node);
        else if (m_language == GEN_LANG_LUA)
            result = generator->GenLuaAfterChildren(node);
        else if (m_language == GEN_LANG_PHP)
            result = generator->GenPhpfterChildren(node);
    }
    if (result)
    {
        // If the node needs to write code after all children are constructed, then create the children first, then write
        // the post-child code. Note that in this case, no further handling of the node is done, so GenAfterChildren() is
        // required to handle all post-child construction code.

        for (const auto& child: node->GetChildNodePtrs())
        {
            GenConstruction(child.get());
        }

        m_source->writeLine(result.value(), indent::none);
        auto parent = node->GetParent();

        // Code for spacer's is handled by the component's GenConstruction() call
        if (parent->IsSizer() && !node->isGen(gen_spacer))
        {
            ttlib::cstr code;
            if (code.size())
                code << '\n';

            if (need_closing_brace)
                code << '\t';
            code << '\t' << node->GetParent()->get_node_name(m_language) << LangPtr() << "Add("
                 << node->get_node_name(m_language) << ", ";

            if (parent->isGen(gen_wxGridBagSizer))
            {
                code << "wxGBPosition(" << node->prop_as_string(prop_row) << ", " << node->prop_as_string(prop_column)
                     << "), ";
                code << "wxGBSpan(" << node->prop_as_string(prop_rowspan) << ", " << node->prop_as_string(prop_colspan)
                     << "), ";
                ttlib::cstr flags(node->prop_as_string(prop_borders));
                if (node->prop_as_string(prop_flags).size())
                {
                    if (flags.size())
                        flags << '|';
                    flags << node->prop_as_string(prop_flags);
                }

                if (flags.empty())
                    flags << '0';

                code << flags << ", " << node->prop_as_string(prop_border_size) << ");";
                code.Replace(", 0, 0);", ");");
            }
            else
            {
                code << GenerateSizerFlags(node) << ");";
            }

            if (need_closing_brace)
            {
                m_source->writeLine(code, indent::auto_keep_whitespace);
                if (m_language == GEN_LANG_CPLUSPLUS)
                {
                    m_source->writeLine("\t}");
                }
            }
            else
            {
                m_source->writeLine(code);
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

void BaseCodeGenerator::GenParentSizer(Node* node, bool need_closing_brace)
{
    auto declaration = node->GetNodeDeclaration();
    auto generator = declaration->GetGenerator();

    auto result = generator->GenAdditionalCode(code_after_children, node, m_language);
    if (!result)
    {
        if (m_language == GEN_LANG_CPLUSPLUS)
            result = generator->GenAdditionalCode(code_after_children, node);
        else if (m_language == GEN_LANG_PYTHON)
            result = generator->GenPythonAdditionalCode(code_after_children, node);
        else if (m_language == GEN_LANG_LUA)
            result = generator->GenLuaAdditionalCode(code_after_children, node);
        else if (m_language == GEN_LANG_PHP)
            result = generator->GenPhpAdditionalCode(code_after_children, node);
    }

    ttlib::cstr code;
    if (result && result.value().size())
    {
        m_source->writeLine(result.value(), indent::none);
    }

    // Code for spacer's is handled by the component's GenConstruction() call
    if (!node->isGen(gen_spacer))
    {
        if (node->isGen(gen_wxStdDialogButtonSizer))
        {
            if (node->get_form()->isGen(gen_wxDialog) && node->prop_as_bool(prop_static_line))
                code << node->GetParent()->get_node_name(m_language) << LangPtr() << "Add(CreateSeparatedSizer("
                     << node->get_node_name(m_language) << "), ";
            else
                code << node->GetParent()->get_node_name(m_language) << LangPtr() << "Add("
                     << node->get_node_name(m_language) << ", ";
        }
        else
        {
            if (need_closing_brace)
            {
                code << "\t";
            }
            code << node->GetParent()->get_node_name(m_language) << LangPtr() << "Add(" << node->get_node_name(m_language)
                 << ", ";
        }

        if (node->GetParent()->isGen(gen_wxGridBagSizer))
        {
            code << "wxGBPosition(" << node->prop_as_string(prop_row) << ", " << node->prop_as_string(prop_column) << "), ";
            code << "wxGBSpan(" << node->prop_as_string(prop_rowspan) << ", " << node->prop_as_string(prop_colspan) << "), ";
            ttlib::cstr flags(node->prop_as_string(prop_borders));
            if (node->prop_as_string(prop_flags).size())
            {
                if (flags.size())
                    flags << '|';
                flags << node->prop_as_string(prop_flags);
            }

            if (flags.empty())
                flags << '0';

            code << flags << ", " << node->prop_as_string(prop_border_size) << ");";
            code.Replace(", 0, 0);", ");");
        }
        else
        {
            code << GenerateSizerFlags(node) << ");";
        }
    }

    if (need_closing_brace)
    {
        m_source->writeLine(code, indent::auto_keep_whitespace);
        if (m_language == GEN_LANG_CPLUSPLUS)
        {
            m_source->writeLine("\t}");
        }
    }
    else
    {
        m_source->writeLine(code);
    }
}
