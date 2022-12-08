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

Code::Code(Node* node, int language) : m_node(node), m_language(language)
{
    if (language == GEN_LANG_PYTHON)
    {
        // This allows for the extra '.' between wx and wxWidget name
        m_break_length = 79;
    }

    // Reserve large enough for multiple lines -- goal is to avoid multiple reallocations
    m_code.reserve(256);
}

// The Code ctor will have already accounted for the extra character that wxWidgets names use
// in Python.
void Code::CheckLineBreak(size_t add_length)
{
    if (!m_auto_break || m_code.size() < m_minium_length)
        return;

    if (m_code.size() + add_length > m_break_at)
    {
        Eol().Tab();
    }
}

Code& Code::CheckLineLength()
{
    if (m_code.size() > m_break_at)
    {
        Eol().Tab();
    }

    return *this;
}

Code& Code::Eol(bool check_size)
{
    if (check_size)
    {
        if (m_code.size())
            m_code += '\n';
    }
    else
    {
        m_code += '\n';
    }

    if (m_auto_break)
    {
        m_break_at = m_code.size() + m_break_length;
        m_minium_length = m_code.size() + 10;
    }
    return *this;
}

void Code::InsertLineBreak(size_t cur_pos)
{
    ASSERT(cur_pos > 1 && cur_pos < m_code.size());
    if (m_code[cur_pos - 1] == ' ')
    {
        m_code[cur_pos - 1] = '\n';
        m_code.insert(cur_pos, "\t");
    }
    else
    {
        m_code.insert(cur_pos, "\n\t");
    }
    m_break_at = cur_pos + m_break_length;
    m_minium_length = cur_pos + 10;
}

Code& Code::Tab(int tabs)
{
    while (tabs)
    {
        m_code += '\t';
        --tabs;
    }
    return *this;
}

Code& Code::Add(ttlib::sview text)
{
    CheckLineBreak(text.size());

    if (is_cpp())
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

Code& Code::FormFunction(ttlib::sview text)
{
    if (m_language == GEN_LANG_PYTHON)
    {
        m_code += "self.";
    }
    m_code += text;
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
    auto& str = m_node->as_string(prop_name);
    if (is_cpp())
    {
        CheckLineBreak(str.size());
        m_code += str;
        return *this;
    }

    if (!ttlib::is_found(str.find('|')))
    {
        if (str == "wxEmptyString")
        {
            m_code += "\"\"";
        }
        else
        {
            CheckLineBreak(str.size());
            if (str.is_sameprefix("wx"))
                m_code << "wx." << str.substr(2);
            else
                m_code += str;
            return *this;
        }
    }

    auto cur_pos = m_code.size();

    ttlib::multiview multistr(str, "|", tt::TRIM::both);
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

    if (m_auto_break && m_code.size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
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

bool Code::HasValue(GenEnum::PropName prop_name) const
{
    return m_node->HasValue(prop_name);
}

bool Code::IsTrue(GenEnum::PropName prop_name) const
{
    return m_node->prop_as_bool(prop_name);
}

bool Code::PropContains(GenEnum::PropName prop_name, ttlib::sview text) const
{
    return m_node->prop_as_string(prop_name).contains(text);
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
                Function("GetStaticBox()");
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
                    Function("GetPane()");
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
        if (is_cpp())
        {
            CheckLineBreak(sizeof("wxEmptyString"));
            m_code += "wxEmptyString";
        }
        else
        {
            m_code += "\"\"";
        }
        return *this;
    }

    auto cur_pos = m_code.size();

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

    if (m_auto_break && m_code.size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::WxSize(GenEnum::PropName prop_name)
{
    if (m_node->prop_as_wxSize(prop_name) == wxDefaultSize)
    {
        CheckLineBreak(sizeof("wxDefaultSize"));
        m_code += is_cpp() ? "wxDefaultSize" : "wx.DefaultSize";
        return *this;
    }

    auto cur_pos = m_code.size();

    bool dialog_units = m_node->value(prop_name).contains("d", tt::CASE::either);
    if (dialog_units)
    {
        CheckLineBreak(sizeof("self.ConvertDialogToPixels(wxSize(999, 999))"));
        FormFunction("ConvertDialogToPixels(");
    }

    auto size = m_node->prop_as_wxSize(prop_name);
    Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';

    if (dialog_units)
        m_code += ')';

    if (m_auto_break && m_code.size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::Pos(GenEnum::PropName prop_name)
{
    if (m_node->prop_as_wxPoint(prop_name) == wxDefaultPosition)
    {
        CheckLineBreak(sizeof("wxDefaultPosition"));
        m_code += is_cpp() ? "wxDefaultPosition" : "wx.DefaultPosition";
        return *this;
    }

    auto cur_pos = m_code.size();

    bool dialog_units = m_node->value(prop_name).contains("d", tt::CASE::either);
    if (dialog_units)
    {
        CheckLineBreak(sizeof("self.ConvertDialogToPixels(wxPoint(999, 999))"));
        FormFunction("ConvertDialogToPixels(");
    }

    auto size = m_node->prop_as_wxSize(prop_name);
    Class("wxPoint(").itoa(size.x).Comma().itoa(size.y) << ')';

    if (dialog_units)
        m_code += ')';

    if (m_auto_break && m_code.size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

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

    // Note that as_string() may break the line, so recalculate any time as_string() is called
    auto cur_pos = m_code.size();

    if (m_node->isGen(gen_wxRichTextCtrl))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        Add("wxRE_MULTILINE");
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
            cur_pos = m_code.size();
        }
        style_set = true;
    }

    if (m_node->HasValue(prop_window_style))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        as_string(prop_window_style);
        cur_pos = m_code.size();
    }

    if (m_node->isGen(gen_wxListView))
    {
        if (style_set)
            m_code += '|';
        style_set = true;
        as_string(prop_mode);
        cur_pos = m_code.size();
    }

    if (!style_set)
    {
        m_code += "0";
    }

    if (m_auto_break && m_code.size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::PosSizeFlags(bool uses_def_validator, ttlib::sview def_style)
{
    if (m_node->HasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        Comma();
        Pos().Comma().WxSize().Comma();
        Style();
        if (uses_def_validator)
            Comma().Add("wxDefaultValidator");
        Comma();
        QuotedString(prop_window_name).EndFunction();
        return *this;
    }

    // Do we need a style and/or a default validator?
    if (m_node->HasValue(prop_style) ||
        (m_node->HasValue(prop_tab_position) && !m_node->prop_as_string(prop_tab_position).is_sameas("wxBK_DEFAULT")) ||
        (m_node->HasValue(prop_orientation) && !m_node->prop_as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL")) ||
        m_node->isGen(gen_wxRichTextCtrl) || m_node->isGen(gen_wxRichTextCtrl) || m_node->HasValue(prop_window_style) ||
        m_node->isGen(gen_wxListView))
    {
        Comma();
        Pos().Comma().WxSize().Comma().Style();
        if (def_style.size() && m_code.ends_with(def_style))
        {
            m_code.erase(m_code.size() - def_style.size());
        }
    }
    else if (m_node->prop_as_wxSize(prop_size) != wxDefaultSize)
    {
        Comma();
        Pos().Comma().WxSize();
    }
    else if (m_node->prop_as_wxPoint(prop_pos) != wxDefaultPosition)
    {
        Comma();
        Pos();
    }
    EndFunction();
    return *this;
}

Code& Code::GenSizerFlags()
{
    // wxSizerFlags functions are chained together, so we don't want to break them. Instead,
    // shut off auto_break and then restore it when we are done, after which we can check whether
    // or note the entire wxSizerFlags() statement needs to be broken.

    bool save_auto_break = m_auto_break;
    m_auto_break = false;
    auto cur_pos = m_code.size();

    Add("wxSizerFlags");

    if (auto& prop = m_node->prop_as_string(prop_proportion); prop != "0")
    {
        m_code << '(' << prop << ')';
    }
    else
    {
        m_code << "()";
    }

    if (auto& prop = m_node->prop_as_string(prop_alignment); prop.size())
    {
        if (prop.contains("wxALIGN_CENTER"))
        {
            // Note that CenterHorizontal() and CenterVertical() require wxWidgets 3.1 or higher. Their advantage is
            // generating an assert if you try to use one that is invalid if the sizer parent's orientation doesn't
            // support it. Center() just works without the assertion check.
            m_code << ".Center()";
        }

        if (prop.contains("wxALIGN_LEFT"))
        {
            m_code += ".Left()";
        }
        else if (prop.contains("wxALIGN_RIGHT"))
        {
            m_code += ".Right()";
        }

        if (prop.contains("wxALIGN_TOP"))
        {
            m_code += ".Top()";
        }
        else if (prop.contains("wxALIGN_BOTTOM"))
        {
            m_code += ".Bottom()";
        }
    }

    if (auto& prop = m_node->prop_as_string(prop_flags); prop.size())
    {
        if (prop.contains("wxEXPAND"))
        {
            m_code += ".Expand()";
        }
        if (prop.contains("wxSHAPED"))
        {
            m_code += ".Shaped()";
        }
        if (prop.contains("wxFIXED_MINSIZE"))
        {
            m_code += ".FixedMinSize()";
        }
        if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
        {
            m_code += ".ReserveSpaceEvenIfHidden()";
        }
    }

    if (auto& prop = m_node->prop_as_string(prop_borders); prop.size())
    {
        auto border_size = m_node->prop_as_string(prop_border_size);
        if (prop.contains("wxALL"))
        {
            if (border_size == "5")
                Add(".Border(").Add("wxALL)");
            else if (border_size == "10")
                Add(".DoubleBorder(").Add("wxALL)");
            else if (border_size == "15")
                Add(".TripleBorder(").Add("wxALL)");
            else
            {
                Add(".Border(").Add("wxAll, ") << border_size << ')';
            }
        }
        else
        {
            m_code << ".Border(";
            ttlib::cstr border_flags;

            if (prop.contains("wxLEFT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags += is_cpp() ? "wxLEFT" : "wx.LEFT";
            }
            if (prop.contains("wxRIGHT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags += is_cpp() ? "wxRIGHT" : "wx.RIGHT";
            }
            if (prop.contains("wxTOP"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags += is_cpp() ? "wxTOP" : "wx.TOP";
            }
            if (prop.contains("wxBOTTOM"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags += is_cpp() ? "wxBOTTOM" : "wx.BOTTOM";
            }
            if (border_flags.empty())
                border_flags = "0";

            m_code << border_flags << ", ";
            if (border_size == "5")
            {
                m_code += is_cpp() ? "wxSizerFlags::GetDefaultBorder())" : "wx.SizerFlags.GetDefaultBorder())";
            }
            else
            {
                m_code << border_size << ')';
            }
        }
    }

    m_auto_break = save_auto_break;

    if (m_auto_break && m_code.size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}
