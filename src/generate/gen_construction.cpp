/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top level Object construction code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "code.h"        // Code -- Helper class for generating code
#include "gen_base.h"    // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"  // Common component functions
#include "node.h"        // Node class
#include "node_decl.h"   // NodeDeclaration class
#include "write_code.h"  // Write code to Scintilla or file

// clang-format off

// These are the types that need to have generator->AdditionalCode() called after the type is constructed
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
    auto type = node->getGenType();
    auto declaration = node->getNodeDeclaration();
    auto generator = declaration->getGenerator();
    if (!generator)
        return;

    if (node->hasValue(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
    {
        BeginPlatformCode(node);
    }

    bool need_closing_brace = false;
    Code gen_code(node, m_language);

    if (generator->ConstructionCode(gen_code))
    {
        // Don't add blank lines when adding tools to a toolbar, or creating menu items
        if (gen_code.size() && gen_code[0] != '{' && type != type_aui_tool && type != type_tool && type != type_menuitem)
        {
            m_source->writeLine();
        }

        m_source->writeLine(gen_code);

        if (gen_code.starts_with("{") && !gen_code.ends_with("}\n"))
        {
            need_closing_brace = true;
        }
    }

    GenSettings(node);

    if (type == type_ribbontoolbar || type == type_ribbonbuttonbar || type == type_ribbongallery)
    {
        BeginBrace();
        // A wxRibbonToolBar can only have abstract children that consist of the tools.
        for (const auto& child: node->getChildNodePtrs())
        {
            Code child_code(child.get(), m_language);
            if (child->getGenerator()->ConstructionCode(child_code))
            {
                m_source->writeLine(child_code);
            }
        }
        EndBrace();
        gen_code.clear();
        gen_code.NodeName().Function("Realize(").EndFunction();
        m_source->writeLine(gen_code);
        return;
    }
    else if (type == type_tool_dropdown)
    {
        return;
    }

    auto parent = node->getParent();

    if (GenAfterChildren(node, need_closing_brace))
    {
        return;
    }

    if (parent->isSizer())
    {
        GenParentSizer(node, need_closing_brace);
    }
    else if (parent->isToolBar() && !node->isType(type_tool) && !node->isType(type_aui_tool) &&
             !node->isType(type_tool_separator) && !node->isType(type_tool_dropdown))
    {
        tt_string code;
        gen_code.clear();
        if (parent->isType(type_toolbar_form) || parent->isType(type_aui_toolbar_form))
            gen_code.Str("AddControl(").as_string(prop_var_name).EndFunction();
        else
            gen_code.ParentName().Function("AddControl(").NodeName().EndFunction();
        m_source->writeLine(gen_code);
    }
    else if (node->getGenType() == type_widget && parent->isGen(gen_wxChoicebook))
    {
        gen_code.clear();
        if (gen_code.is_python())
        {
            gen_code << "# wxPython 4.2.0 does not support wx.Choicebook.GetControlSizer()";
            gen_code.Eol().Str("# so ").NodeName().Str(" cannot be added to the Choicebook.");
        }
        else
        {
            gen_code.ParentName().Function("GetControlSizer()").Function("Add(").NodeName();
            gen_code.CheckLineLength().Comma().Add("wxSizerFlags()").Add(".Expand().Border(").Add("wxALL)").EndFunction();
        }
        m_source->writeLine(gen_code);
    }

    if (node->isGen(gen_PageCtrl) && node->getChildCount())
    {
        // type_page will have already constructed the code for the child. However, we still need to generate
        // settings and process any grandchildren.

        auto page_child = node->getChild(0);
        if (page_child)
        {
            GenSettings(page_child);

            for (const auto& child: page_child->getChildNodePtrs())
            {
                GenConstruction(child.get());
            }
        }
    }
    else
    {
        for (const auto& child: node->getChildNodePtrs())
        {
            GenConstruction(child.get());
        }
    }

    if (node->isSizer())
    {
        if (!parent->isSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
        {
            // The parent node is not a sizer -- which is expected if this is the parent sizer underneath a form or
            // wxPanel.

            gen_code.clear();

            tt_string code;
            if (parent->isGen(gen_wxRibbonPanel))
            {
                gen_code.ParentName().Function("SetSizerAndFit(").NodeName().EndFunction();
            }
            else
            {
                if (GetParentName(node) != "this")
                    gen_code.ParentName();
                else if (gen_code.is_python())
                    gen_code.Str("self");
                gen_code.Function("SetSizerAndFit(").NodeName().EndFunction();
            }

            m_source->writeLine();
            m_source->writeLine(gen_code.GetCode());
        }
    }
    else if (type == type_splitter)
    {
        gen_code.clear();

        if (node->getChildCount() == 1)
        {
            gen_code.NodeName();
            gen_code.Function("Initialize(").NodeName(node->getChild(0)).EndFunction();
        }
        else if (node->getChildCount() > 1)
        {
            gen_code.NodeName();
            if (node->as_string(prop_splitmode) == "wxSPLIT_VERTICAL")
                gen_code.Function("SplitVertically(");
            else
                gen_code.Function("SplitHorizontally(");

            gen_code.NodeName(node->getChild(0)).Comma().NodeName(node->getChild(1)).EndFunction();

            if (auto sash_pos = node->getPropPtr(prop_sashpos)->as_int(); sash_pos != 0 && sash_pos != -1)
            {
                gen_code.Eol().NodeName().Function("SetSashPosition(").Add(prop_sashpos).EndFunction();
            }
        }
        m_source->writeLine(gen_code);
    }

    else
    {
        for (size_t idx = 0; aftercode_types[idx] != gen_type_unknown; ++idx)
        {
            if (type == aftercode_types[idx])
            {
                gen_code.clear();
                if (generator->AfterChildrenCode(gen_code))
                {
                    if (gen_code.size())
                    {
                        m_source->writeLine(gen_code);
                    }
                }
                m_source->writeLine();
                break;
            }
        }
    }

    if (node->hasValue(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
    {
        EndPlatformCode();
    }
}

const char* BaseCodeGenerator::LangPtr() const
{
    switch (m_language)
    {
        case GEN_LANG_CPLUSPLUS:
            return "->";

        case GEN_LANG_PYTHON:
            return ".";

        default:
            FAIL_MSG("Unsupported language!")
            return "";
    }
}

void BaseCodeGenerator::BeginPlatformCode(Node* node)
{
    tt_string code;
    if (node->as_string(prop_platforms).contains("Windows"))
    {
        switch (m_language)
        {
            case GEN_LANG_CPLUSPLUS:
                code << "\n#if defined(__WINDOWS__)";
                break;

            case GEN_LANG_PYTHON:
                code << "\nif defined(__WINDOWS__)";
                break;
        }
    }
    if (node->as_string(prop_platforms).contains("Unix"))
    {
        switch (m_language)
        {
            case GEN_LANG_CPLUSPLUS:
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
        }
    }
    if (node->as_string(prop_platforms).contains("Mac"))
    {
        switch (m_language)
        {
            case GEN_LANG_CPLUSPLUS:
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
        }
    }

    m_source->writeLine(code);
    m_source->SetLastLineBlank();
    if (m_language == GEN_LANG_PYTHON)
        m_source->Indent();
}

void BaseCodeGenerator::EndPlatformCode()
{
    switch (m_language)
    {
        case GEN_LANG_CPLUSPLUS:
            m_source->writeLine("#endif  // limited to specific platforms");
            break;

        case GEN_LANG_PYTHON:
            m_source->Unindent();
            break;
    }
}

void BaseCodeGenerator::BeginBrace()
{
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_source->writeLine("{");
        m_source->Indent();
    }
}

void BaseCodeGenerator::EndBrace()
{
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_source->Unindent();
        m_source->writeLine("}");
    }
}

void BaseCodeGenerator::GenSettings(Node* node)
{
    auto generator = node->getGenerator();

    Code code(node, m_language);
    if (generator->SettingsCode(code))
    {
        if (code.size())
        {
            m_source->writeLine(code);
            code.clear();
        }
    }

    if (node->getPropPtr(prop_window_extra_style))
    {
        if (m_language == GEN_LANG_CPLUSPLUS)
        {
            GenValidatorSettings(code);
            if (code.size())
            {
                m_source->writeLine(code);
                code.clear();
            }
        }
        code.GenWindowSettings();
        if (code.size())
            m_source->writeLine(code);
    }
}

bool BaseCodeGenerator::GenAfterChildren(Node* node, bool need_closing_brace)
{
    auto generator = node->getGenerator();
    Code gen_code(node, m_language);
    if (generator->AfterChildrenCode(gen_code))
    {
        // If the node needs to write code after all children are constructed, then create the children first, then write
        // the post-child code.

        for (const auto& child: node->getChildNodePtrs())
        {
            GenConstruction(child.get());
        }

        m_source->writeLine(gen_code);
        auto parent = node->getParent();

        // Code for spacer's is handled by the component's GenConstruction() call
        if (parent->isSizer() && !node->isGen(gen_spacer))
        {
            gen_code.clear();

            if (need_closing_brace)
                gen_code.Tab();
            gen_code.Tab().ParentName().Function("Add(").NodeName().Comma();

            if (parent->isGen(gen_wxGridBagSizer))
            {
                gen_code.Object("wxGBPosition").as_string(prop_row).Comma().as_string(prop_column) << "), ";
                gen_code.Object("wxGBSpan").as_string(prop_rowspan).Comma().as_string(prop_colspan) << "), ";

                if (node->hasValue(prop_borders))
                    gen_code.as_string(prop_borders);
                if (node->as_string(prop_flags).size())
                {
                    if (node->hasValue(prop_borders))
                        gen_code.GetCode() += '|';
                    gen_code.as_string(prop_flags);
                }

                if (!node->hasValue(prop_borders) && !node->hasValue(prop_flags))
                    gen_code.GetCode() += '0';

                gen_code.Comma().BorderSize().EndFunction();
                gen_code.GetCode().Replace(", 0, 0)", ")");
            }
            else
            {
                gen_code.GenSizerFlags().EndFunction();
            }

            if (need_closing_brace)
            {
                m_source->writeLine(gen_code.GetCode(), indent::auto_keep_whitespace);
                if (m_language == GEN_LANG_CPLUSPLUS)
                {
                    m_source->writeLine("}");
                }
            }
            else
            {
                m_source->writeLine(gen_code.GetCode());
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
    auto declaration = node->getNodeDeclaration();
    auto generator = declaration->getGenerator();

    Code code(node, m_language);
    if (generator->AfterChildrenCode(code))
    {
        m_source->writeLine(code);
    }
    code.clear();

    // Code for spacer's is handled by the component's GenConstruction() call
    if (!node->isGen(gen_spacer))
    {
        if (node->isGen(gen_wxStdDialogButtonSizer))
        {
            if (node->getForm()->isGen(gen_wxDialog) && node->as_bool(prop_static_line))
            {
                if (is_cpp())
                    code.ParentName().Function("Add(CreateSeparatedSizer(").NodeName() << "), ";
                else
                    code.ParentName().Function("Add(").NodeName() << ", ";
            }
            else
                code.ParentName().Function("Add(").NodeName() << ", ";
        }
        else
        {
            if (need_closing_brace && is_cpp())
            {
                code << "\t";
            }
            code.ParentName().Function("Add(").NodeName() << ", ";
        }

        if (node->getParent()->isGen(gen_wxGridBagSizer))
        {
            code.Object("wxGBPosition").as_string(prop_row).Comma().as_string(prop_column) << "), ";
            code.Object("wxGBSpan").as_string(prop_rowspan).Comma().as_string(prop_colspan) << "), ";
            tt_string flags(node->as_string(prop_borders));
            if (node->as_string(prop_flags).size())
            {
                if (flags.size())
                    flags << '|';
                flags << node->as_string(prop_flags);
            }

            if (flags.empty())
                flags << '0';

            code.Add(flags).Comma().BorderSize().EndFunction();
            if (is_cpp())
                code.Replace(", 0, 0);", ");");
            else
                code.Replace(", 0, 0)", ")");
        }
        else
        {
            code.GenSizerFlags();
            code.EndFunction();
        }
    }

    if (need_closing_brace)
    {
        m_source->writeLine(code.GetCode(), indent::auto_keep_whitespace);
        if (m_language == GEN_LANG_CPLUSPLUS)
        {
            m_source->writeLine("}");
        }
    }
    else
    {
        m_source->writeLine(code.GetCode(), indent::auto_keep_whitespace);
    }
}
