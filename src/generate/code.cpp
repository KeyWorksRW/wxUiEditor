/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <ttmultistr_wx.h>  // ttMultiString -- Class for handling multiple strings

#include "code.h"

#include "node.h"           // Node class
#include "project_class.h"  // Project class

Code& Code::Tab(int tabs)
{
    while (tabs)
    {
        m_code << '\t';
        --tabs;
    }
    return *this;
}

Code& Code::Add(ttlib::sview text)
{
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_code << text;
    }
    else
    {
        if (text.is_sameprefix("wx"))
        {
            m_code << "wx." << text.substr(2);
        }
        else
        {
            m_code << text;
        }
    }
    return *this;
}

Code& Code::Function(ttlib::sview text)
{
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_code << "->" << text;
    }
    else
    {
        m_code << '.';
        if (text.is_sameprefix("wx"))
        {
            m_code << "wx." << text.substr(2);
        }
        else
        {
            m_code << text;
        }
    }
    return *this;
}

Code& Code::Class(ttlib::sview text)
{
    if (is_cpp())
    {
        m_code += text;
    }
    else
    {
        if (text.is_sameprefix("wx"))
        {
            m_code << "wx." << text.substr(2);
        }
        else
        {
            m_code += text;
        }
    }
    return *this;
}
Code& Code::CreateClass(bool use_generic)
{
    m_code << " = ";
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_code << "new ";
        if (m_node->HasValue(prop_derived_class))
        {
            m_code << m_node->prop_as_string(prop_derived_class);
            m_code << '(';
            return *this;
        }
    }

    ttlib::cstr class_name = m_node->DeclName();
    if (use_generic)
    {
        class_name.Replace("wx", "wxGeneric");
    }
    else if (m_node->isGen(gen_BookPage))
    {
        class_name << "wxPanel";
    }

    if (m_language == GEN_LANG_CPLUSPLUS)
        m_code << class_name;
    else
        m_code << "wx." << class_name.substr(2);
    m_code << '(';
    return *this;
}

Code& Code::Assign(ttlib::sview class_name)
{
    m_code << " = ";
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_code << "new ";
    }
    else
    {
        m_code << "wx." << class_name.substr(2);
    }
    return *this;
}

Code& Code::EndFunction()
{
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_code << ");";
    }
    else
    {
        m_code << ')';
    }
    return *this;
}

Code& Code::as_string(PropName prop_name)
{
    if (is_cpp())
    {
        m_code << m_node->prop_as_string(prop_name);
    }
    else
    {
        ttlib::multiview multistr(m_node->prop_as_string(prop_name), "|", tt::TRIM::both);
        bool first = true;
        for (auto& iter: multistr)
        {
            if (iter.empty())
                continue;
            if (!first)
                m_code << '|';
            else
                first = false;

            if (iter == "wxEmptyString")
                m_code << "\"\"";
            else if (iter.is_sameprefix("wx"))
                m_code << "wx." << iter.substr(2);
            else
                m_code << iter;
        }
    }
    return *this;
}

Code& Code::NodeName()
{
    m_code << m_node->get_node_name();
    return *this;
}

Code& Code::ParentName()
{
    m_code << m_node->GetParent()->get_node_name();
    return *this;
}

bool Code::is_local_var() const
{
    return m_node->IsLocal();
}

// clang-format off

// List of valid component parent types
static constexpr GenType s_GenParentTypes[] = {

    type_auinotebook,
    type_bookpage,
    type_choicebook,
    type_container,
    type_listbook,
    type_notebook,
    type_ribbonpanel,
    type_simplebook,
    type_splitter,
    type_wizardpagesimple,

};

// clang-format on

Code& Code::GetParentName()
{
    auto parent = m_node->GetParent();
    while (parent)
    {
        if (parent->IsSizer())
        {
            if (parent->IsStaticBoxSizer())
            {
                m_code << parent->get_node_name();
                Function("GetStaticBox(").EndFunction();
                return *this;
            }
        }
        if (parent->IsForm())
        {
            m_code += (m_language == GEN_LANG_CPLUSPLUS) ? "this" : "self";
            return *this;
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->isType(iter))
            {
                m_code << parent->get_node_name();
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    Function("GetPane(").EndFunction();
                }
                return *this;
            }
        }
        parent = parent->GetParent();
    }

    ASSERT_MSG(parent, ttlib::cstr() << m_node->get_node_name() << " has no parent!");
    return *this;
}

Code& Code::QuotedString(GenEnum::PropName prop_name)
{
    if (!m_node->HasValue(prop_name))
    {
        m_code += is_cpp() ? "wxEmptyString" : "\"\"";
        return *this;
    }

    if (GetProject()->prop_as_bool(prop_internationalize))
    {
        m_code += is_cpp() ? "_(" : "wx.GetTranslation(";
    }

    // This is only used by C++, but we need to know if it was set in order to generate closing parenthesis.
    bool has_utf_char = false;
    if (is_cpp())
    {
        for (auto iter: m_node->as_string(prop_name))
        {
            if (iter < 0)
            {
                has_utf_char = true;
                break;
            }
        }

        if (has_utf_char)
        {
            m_code += "wxString::FromUTF8(";
        }
    }

    m_code += '"';
    for (auto c: m_node->as_string(prop_name))
    {
        switch (c)
        {
            case '"':
                m_code += "\\\"";
                break;

            // This generally isn't needed for C++, but is needed for Python
            case '\'':
                m_code += "\\'";
                break;

            case '\\':
                m_code += "\\\\";
                break;

            case '\t':
                m_code += "\\t";
                break;

            case '\n':
                m_code += "\\n";
                break;

            case '\r':
                m_code += "\\r";
                break;

            default:
                m_code += c;
                break;
        }
    }
    m_code += '"';

    if (has_utf_char)
    {
        m_code += ')';
    }
    if (GetProject()->prop_as_bool(prop_internationalize))
    {
        m_code += ')';
    }

    return *this;
}

Code& Code::WxSize(GenEnum::PropName prop_name)
{
    if (m_node->prop_as_wxSize(prop_name) == wxDefaultSize)
    {
        m_code += is_cpp() ? "wxDefaultSize" : "wx.DefaultSize";
        return *this;
    }

    bool dialog_units = m_node->value(prop_name).contains("d", tt::CASE::either);
    if (dialog_units)
    {
        if (is_python())
            Add("self.");
        m_code += "ConvertDialogToPixels(";
    }

    auto size = m_node->prop_as_wxSize(prop_name);
    Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';

    if (dialog_units)
        m_code += ')';

    return *this;
}

Code& Code::Pos(GenEnum::PropName prop_name)
{
    if (m_node->prop_as_wxPoint(prop_name) == wxDefaultPosition)
    {
        m_code += is_cpp() ? "wxDefaultPosition" : "wx.DefaultPosition";
        return *this;
    }

    bool dialog_units = m_node->value(prop_name).contains("d", tt::CASE::either);
    if (dialog_units)
    {
        if (is_python())
            Add("self.");
        m_code += "ConvertDialogToPixels(";
    }

    auto size = m_node->prop_as_wxSize(prop_name);
    Class("wxPoint(").itoa(size.x).Comma().itoa(size.y) << ')';

    if (dialog_units)
        m_code += ')';
    return *this;
}

Code& Code::Style(const char* prefix)
{
    bool style_set = false;

    if (m_node->HasValue(prop_tab_position) && !m_node->prop_as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        as_string(prop_tab_position);
    }
    if (m_node->HasValue(prop_orientation) && !m_node->prop_as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        as_string(prop_orientation);
    }

    if (m_node->isGen(gen_wxRichTextCtrl))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        m_code += is_cpp() ? "wxRE_MULTILINE" : "wx.RE_MULTILINE";
    }

    if (m_node->HasValue(prop_style))
    {
        if (style_set)
            m_code += '|';
        if (prefix)
        {
            if (is_cpp())
            {
                m_code += m_node->prop_as_constant(prop_style, prefix);
            }
            else
            {
                ttlib::multiview multistr(m_node->prop_as_constant(prop_style, prefix), "|", tt::TRIM::both);
                for (auto& iter: multistr)
                {
                    if (iter.empty())
                        continue;
                    if (style_set)
                        m_code += '|';
                    if (iter.is_sameprefix("wx"))
                        m_code << "wx." << iter.substr(2);
                    else
                        m_code += iter;
                    style_set = true;
                }
            }
        }
        else
        {
            as_string(prop_style);
        }
        style_set = true;
    }

    if (m_node->HasValue(prop_window_style))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        as_string(prop_window_style);
    }

    if (m_node->isGen(gen_wxListView))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        as_string(prop_mode);
    }

    if (!style_set)
    {
        m_code += "0";
    }

    return *this;
}

Code& Code::PosSizeFlags(bool uses_def_validator, ttlib::sview def_style)
{
    if (m_node->HasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        Comma().CheckLineLength();
        Pos().Comma().CheckLineLength().WxSize().Comma().CheckLineLength();
        Style();
        if (uses_def_validator && is_cpp())
            m_code += ", wxDefaultValidator";
        Comma().CheckLineLength();
        QuotedString(prop_window_name).EndFunction();
        return *this;
    }

    // Do we need a style and/or a default validator?
    if ((uses_def_validator && is_cpp()) || m_node->HasValue(prop_style) ||
        (m_node->HasValue(prop_tab_position) && !m_node->prop_as_string(prop_tab_position).is_sameas("wxBK_DEFAULT")) ||
        (m_node->HasValue(prop_orientation) && !m_node->prop_as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL")) ||
        m_node->isGen(gen_wxRichTextCtrl) || m_node->isGen(gen_wxRichTextCtrl) || m_node->HasValue(prop_window_style) ||
        m_node->isGen(gen_wxListView))
    {
        Comma().CheckLineLength();
        Pos().Comma().CheckLineLength().WxSize().Comma().CheckLineLength().Style();
        if (uses_def_validator && is_cpp())
        {
            m_code += ", wxDefaultValidator";
        }
        else if (def_style.size() && m_code.ends_with(def_style))
        {
            m_code.erase(m_code.size() - def_style.size());
        }
    }
    else if (m_node->prop_as_wxSize(prop_size) != wxDefaultSize)
    {
        Comma().CheckLineLength();
        Pos().Comma().CheckLineLength().WxSize();
    }
    else if (m_node->prop_as_wxPoint(prop_pos) != wxDefaultPosition)
    {
        Comma().CheckLineLength();
        Pos();
    }
    EndFunction();
    return *this;
}

Code& Code::CheckLineLength()
{
    if (m_code.size() > m_break_at)
    {
        m_code += "\n\t\t";
        if (is_python())
            m_code += '\t';
        m_break_at += 80;
    }

    return *this;
}
