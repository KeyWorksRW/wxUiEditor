/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <map>

#include "code.h"

#include "mainapp.h"          // App class
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

using namespace code;

// clang-format off

static const std::map<tt_string_view, std::string_view, std::less<>> s_short_map
{
    { "wxAUI_", "wx.aui."},
    { "wxCAL_", "wx.adv."},
    { "wxDV_", "wx.dataview."},
    { "wxPG_", "wx.propgrid."},
    { "wxRE_", "wx.richtext."},
    { "wxRIBBON", "wx.ribbon."},
    { "wxSTC_", "wx.stc."},
    { "wxWIZARD_", "wx.adv."},
    { "wxGRID_", "wx.grid."},

    { "wxEVT_DATAVIEW_", "wx.dataview."},
    { "wxEVT_DATE_", "wx.adv."},
    { "wxEVT_GRID_", "wx.grid." },
    { "wxEVT_RIBBON", "wx.ribbon." },
    { "wxEVT_STC_", "wx.stc." },
    { "wxEVT_TIME_", "wx.adv."},
    { "wxEVT_WIZARD_", "wx.adv."},

};

static const std::map<std::string_view, std::string_view, std::less<>> s_map_wx_prefix
{
    { "wxAC_DEFAULT_STYLE", "wx.adv."},
    { "wxAC_NO_AUTORESIZE", "wx.adv."},
    { "wxNullAnimation", "wx.adv."},
    { "wxEL_ALLOW_NEW", "wx.adv."},
    { "wxEL_ALLOW_EDIT", "wx.adv."},
    { "wxEL_ALLOW_DELETE", "wx.adv."},
    { "wxEL_NO_REORDER", "wx.adv."},
    { "wxHL_DEFAULT_STYLE", "wx.adv."},
    { "wxHL_ALIGN_LEFT", "wx.adv."},
    { "wxHL_ALIGN_RIGHT", "wx.adv."},
    { "wxHL_ALIGN_CENTRE", "wx.adv."},
    { "wxHL_CONTEXTMENU", "wx.adv."},

    { "wxHLB_DEFAULT_STYLE", "wx.html."},
    { "wxHLB_MULTIPLE", "wx.html."},

    { "wxHW_SCROLLBAR_AUTO", "wx.html."},
    { "wxHW_NO_SELECTION", "wx.html."},
    { "wxHW_NO_SELECTION", "wx.html."},

    { "wxEVT_PG_CHANGED", "wx.propgrid." },
    { "wxEVT_PG_CHANGING", "wx.propgrid." },
    { "wxPropertyCategory", "wx.propgrid." },
    { "wxBoolProperty", "wx.propgrid." },
    { "wxColourProperty", "wx.propgrid." },
    { "wxCursorProperty", "wx.propgrid." },
    { "wxDateProperty", "wx.propgrid." },
    { "wxDirProperty", "wx.propgrid." },
    { "wxEditEnumProperty", "wx.propgrid." },
    { "wxEnumProperty", "wx.propgrid." },
    { "wxFileProperty", "wx.propgrid." },
    { "wxFlagsProperty", "wx.propgrid." },
    { "wxFloatProperty", "wx.propgrid." },
    { "wxFontProperty", "wx.propgrid." },
    { "wxImageFileProperty", "wx.propgrid." },
    { "wxIntProperty", "wx.propgrid." },
    { "wxLongStringProperty", "wx.propgrid." },
    { "wxMultiChoiceProperty", "wx.propgrid." },
    { "wxStringProperty", "wx.propgrid." },
    { "wxSystemColourProperty", "wx.propgrid." },
    { "wxUIntProperty", "wx.propgrid." },

    { "wxRibbonAUIArtProvider", "wx.ribbon." },
    { "wxRibbonDefaultArtProvider", "wx.ribbon." },
    { "wxRibbonMSWArtProvider", "wx.ribbon." },

    // This doesn't get created as a class, so we have to add it as if it was a constant.
    { "wxWebView", "wx.html2."},
    { "wxWebViewBackendDefault", "wx.html2."},
};

std::map<std::string_view, std::string_view, std::less<>> g_map_class_prefix
{
    { "wxAnimationCtrl", "wx.adv."},
    { "wxAuiNotebook", "wx.aui."},
    { "wxAuiToolBar", "wx.aui."},
    { "wxBannerWindow", "wx.adv."},
    { "wxCalendarCtrl", "wx.adv."},
    { "wxCommandLinkButton", "wx.adv."},
    { "wxDatePickerCtrl", "wx.adv."},
    { "wxEditableListBox", "wx.adv."},
    { "wxHtmlWindow", "wx.html."},
    { "wxSimpleHtmlListBox", "wx.html."},
    { "wxHyperlinkCtrl", "wx.adv."},
    { "wxRichTextCtrl", "wx.richtext."},
    { "wxStyledTextCtrl", "wx.stc."},
    { "wxTimePickerCtrl", "wx.adv."},
    { "wxStyledTextCtrl", "wx.stc."},
    { "wxWebView", "wx.html2."},
    { "wxWizard", "wx.adv."},
    { "wxWizardPageSimple", "wx.adv."},
    { "wxRibbonBar", "wx.ribbon."},
    { "wxRibbonButtonBar", "wx.ribbon."},
    { "wxRibbonPage", "wx.ribbon."},
    { "wxRibbonPanel", "wx.ribbon."},
    { "wxRibbonToolBar", "wx.ribbon."},
    { "wxRibbonGallery", "wx.ribbon."},
    { "wxBitmapComboBox", "wx.adv."},
    { "wxDataViewCtrl", "wx.dataview."},
    { "wxDataViewListCtrl", "wx.dataview."},
    { "wxDataViewTreeCtrl", "wx.dataview."},
    { "wxGrid", "wx.grid."},
    { "wxPropertyGridManager", "wx.propgrid."},
    { "wxPropertyGrid", "wx.propgrid."},

};
// clang-format on

Code::Code(Node* node, int language)
{
    Init(node, language);
}

void Code::Init(Node* node, int language)
{
    m_node = node;
    m_language = language;
    if (language == GEN_LANG_CPLUSPLUS)
    {
        m_break_length = Project.as_size_t(prop_cpp_line_length);
        // Always assume C++ code has one tab at the beginning of the line
        m_break_length -= m_indent_size;
    }
    else if (language == GEN_LANG_PYTHON)
    {
        m_break_length = Project.as_size_t(prop_python_line_length);
        // Always assume Python code has two tabs at the beginning of the line
        m_break_length -= (m_indent_size * 2);
    }
    m_break_at = m_break_length;

    // Reserve large enough for multiple lines -- goal is to avoid multiple reallocations
    reserve(256);
}

Code& Code::CheckLineLength(size_t next_str_size)
{
    if (m_indent)
        next_str_size += (m_indent * m_indent_size);

    if (m_auto_break && size() > m_minium_length && size() + next_str_size > m_break_at)
    {
        if (back() == ' ')
            pop_back();
        Eol().Tab();
    }
    return *this;
}

Code& Code::CheckLineLength(GenEnum::PropName next_prop_name)
{
    return CheckLineLength(m_node->as_string(next_prop_name).size());
}

Code& Code::Eol(int flag)
{
    if (flag == eol_if_empty)
    {
        if (size())
        {
            *this += '\n';
        }
    }
    else if (flag == eol_if_needed)
    {
        if (size() && back() != '\n')
        {
            // If we're in a brace section, the last line will end with \n\t
            if (size() < 3 || back() != '\t' || at(size() - 2) != '\n')
            {
                *this += '\n';
            }
        }
    }
    else
    {
        if (size() && back() == ' ')
            pop_back();
        *this += '\n';
    }

    if (m_within_braces && is_cpp() && size() && back() != '\t')
    {
        *this += '\t';
    }
    else if (m_indent > 0)
    {
        Tab(m_indent);
    }

    if (m_auto_break)
    {
        m_break_at = size() + m_break_length;
        m_minium_length = size() + 10;
    }
    return *this;
}

Code& Code::OpenBrace()
{
    if (is_cpp())
    {
        m_within_braces = true;
        if (size() && back() != '\n')
        {
            *this += '\n';
        }
        *this += '{';
        Eol();
    }
    return *this;
}

Code& Code::CloseBrace()
{
    if (is_cpp())
    {
        m_within_braces = false;
        while (tt::is_whitespace(back()))
            pop_back();
        Eol().Str("}").Eol();
    }
    return *this;
}

Code& Code::AddAuto()
{
    if (is_cpp() && is_local_var())
    {
        *this += "auto* ";
    }
    return *this;
}

void Code::InsertLineBreak(size_t cur_pos)
{
    ASSERT(cur_pos > 1 && cur_pos <= size());
    if (at(cur_pos - 1) == ' ')
    {
        at(cur_pos - 1) = '\n';
        insert(cur_pos, "\t");
    }
    else
    {
        insert(cur_pos, "\n");
    }
    m_break_at = cur_pos + m_break_length;
    m_minium_length = cur_pos + 10;
}

Code& Code::Tab(int tabs)
{
    while (tabs)
    {
        *this += '\t';
        --tabs;
    }
    return *this;
}

Code& Code::Add(tt_string_view text)
{
    CheckLineLength(text.size());

    if (is_cpp() || text.size() < 3)
    {
        *this += text;
    }
    else
    {
        auto lambda = [&](tt_string_view candidate)
        {
            // Note that you can *NOT* allocate anything on the stack and return a
            // std::string_view to it!
            std::string_view wx_prefix = "wx.";
            for (auto& iter_prefix: s_short_map)
            {
                if (candidate.starts_with(iter_prefix.first))
                {
                    wx_prefix = iter_prefix.second;
                    return wx_prefix;
                }
            }
            if (auto wx_iter = s_map_wx_prefix.find(candidate); wx_iter != s_map_wx_prefix.end())
            {
                wx_prefix = wx_iter->second;
                return wx_prefix;
            }
            return wx_prefix;
        };

        if (text.find('|') != tt::npos)
        {
            bool style_set = false;
            tt_view_vector multistr(text, "|", tt::TRIM::both);
            for (auto& iter: multistr)
            {
                if (iter.empty())
                    continue;
                if (style_set)
                    *this += '|';
                if (iter.is_sameprefix("wx"))
                {
                    auto wx_prefix = lambda(iter);
                    *this << wx_prefix << iter.substr(2);
                }
                else
                    *this += iter;
                style_set = true;
            }
        }
        // text.size() has already been checked to be certain it is at least 3 characters
        else if (text.is_sameprefix("wx") && text[2] != '.')
        {
            auto wx_prefix = lambda(text);
            *this << wx_prefix << text.substr(2);
        }
        else
        {
            *this += text;
        }
    }
    return *this;
}

Code& Code::TrueFalseIf(GenEnum::PropName prop_name)
{
    if (m_node->as_bool(prop_name))
        return AddTrue();
    else
        return AddFalse();
}

Code& Code::AddConstant(GenEnum::PropName prop_name, tt_string_view short_name)
{
    return Add(m_node->as_constant(prop_name, short_name));
}

Code& Code::Function(tt_string_view text)
{
    if (is_cpp())
    {
        *this << "->" << text;
    }
    else
    {
        *this << '.';
        if (text.is_sameprefix("wx"))
        {
            *this << "wx." << text.substr(2);
        }
        else
        {
            *this += text;
        }
    }
    return *this;
}

Code& Code::ClassMethod(tt_string_view function_name)
{
    if (is_cpp())
    {
        *this += "::";
    }
    else
    {
        *this += '.';
    }

    *this += function_name;
    return *this;
}

Code& Code::FormFunction(tt_string_view text)
{
    if (is_python())
    {
        *this += "self.";
    }
    *this += text;
    return *this;
}

Code& Code::Class(tt_string_view text)
{
    if (is_cpp())
    {
        *this += text;
    }
    else
    {
        if (text.is_sameprefix("wx"))
        {
            *this << "wx." << text.substr(2);
        }
        else
        {
            *this += text;
        }
    }
    return *this;
}

Code& Code::CreateClass(bool use_generic, tt_string_view override_name)
{
    *this += " = ";
    if (is_cpp())
    {
        *this += "new ";
        if (m_node->HasValue(prop_derived_class))
        {
            *this += m_node->prop_as_string(prop_derived_class);
            *this += '(';
            if (m_node->HasValue(prop_derived_params))
            {
                *this += m_node->value(prop_derived_params);
                if (back() != ',')
                    *this += ", ";
                if (back() != ' ')
                    *this += ' ';
            }
            return *this;
        }
    }

    tt_string class_name;
    if (override_name.empty())
        class_name = m_node->DeclName();
    else
        class_name = override_name;
    if (use_generic)
    {
        class_name.Replace("wx", "wxGeneric");
    }
    else if (m_node->isGen(gen_BookPage))
    {
        class_name = "wxPanel";
    }

    if (m_language == GEN_LANG_CPLUSPLUS)
        *this += class_name;
    else
    {
        std::string_view prefix = "wx.";
        if (auto wx_iter = g_map_class_prefix.find(class_name); wx_iter != g_map_class_prefix.end())
        {
            prefix = wx_iter->second;
        }
        *this << prefix << class_name.substr(2);
    }
    *this += '(';
    return *this;
}

Code& Code::Assign(tt_string_view class_name)
{
    *this += " = ";
    if (is_cpp())
    {
        *this << "new " << class_name << ';';
    }
    else
    {
        *this << "wx." << class_name.substr(2);
    }
    return *this;
}

Code& Code::EndFunction()
{
    *this += ')';
    if (is_cpp())
    {
        *this += ';';
    }
    return *this;
}

Code& Code::as_string(PropName prop_name)
{
    auto& str = m_node->as_string(prop_name);
    if (is_cpp())
    {
        CheckLineLength(str.size());
        *this += str;
        return *this;
    }
    std::string_view wx_prefix = "wx.";
    auto lambda = [&](tt_string_view candidate)
    {
        for (auto& iter_prefix: s_short_map)
        {
            if (candidate.starts_with(iter_prefix.first))
            {
                wx_prefix = iter_prefix.second;
                return;
            }
        }
        if (auto wx_iter = s_map_wx_prefix.find(candidate); wx_iter != s_map_wx_prefix.end())
        {
            wx_prefix = wx_iter->second;
            return;
        }
    };

    if (!tt::is_found(str.find('|')))
    {
        if (str == "wxEmptyString")
        {
            *this += "\"\"";
        }
        else
        {
            CheckLineLength(str.size());
            if (str.is_sameprefix("wx"))
            {
                lambda(str);
                *this << wx_prefix << str.substr(2);
            }
            else
                *this += str;
            return *this;
        }
    }

    auto cur_pos = size();

    tt_view_vector multistr(str, "|", tt::TRIM::both);
    bool first = true;
    for (auto& iter: multistr)
    {
        if (iter.empty())
            continue;
        if (!first)
            *this += '|';
        else
            first = false;

        if (iter == "wxEmptyString")
            *this += "\"\"";
        else if (iter.is_sameprefix("wx"))
        {
            lambda(iter);
            CheckLineLength(iter.size());
            *this << wx_prefix << iter.substr(2);
        }
        else
            *this += iter;
    }

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::NodeName(Node* node)
{
    if (!node)
        node = m_node;
    if (is_python() && !node->IsLocal() && !node->IsForm())
        *this += "self.";
    *this += node->get_node_name();
    return *this;
}

Code& Code::ParentName()
{
    if (is_python() && !m_node->GetParent()->IsLocal() && !m_node->GetParent()->IsForm())
        *this += "self.";
    *this << m_node->GetParent()->get_node_name();
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

int Code::IntValue(GenEnum::PropName prop_name) const
{
    return m_node->as_int(prop_name);
}

bool Code::PropContains(GenEnum::PropName prop_name, tt_string_view text) const
{
    return m_node->as_string(prop_name).contains(text);
}

size_t Code::PropSize(GenEnum::PropName prop_name) const
{
    return m_node->as_string(prop_name).size();
}

// clang-format off

// List of valid component parent types
static constexpr GenType s_GenParentTypes[] = {

    type_aui_toolbar,
    type_auinotebook,
    type_bookpage,
    type_choicebook,
    type_container,
    type_listbook,
    type_notebook,
    type_propgridpage,
    type_ribbonpanel,
    type_simplebook,
    type_splitter,
    type_toolbar,
    type_wizardpagesimple,

};

// clang-format on

Code& Code::ValidParentName()
{
    auto parent = m_node->GetParent();
    while (parent)
    {
        if (parent->IsSizer())
        {
            if (parent->IsStaticBoxSizer())
            {
                if (is_python() && !parent->IsLocal() && !parent->IsForm())
                    *this += "self.";
                *this += parent->get_node_name();
                Function("GetStaticBox()");
                return *this;
            }
        }
        else if (parent->IsForm())
        {
            *this += (is_cpp()) ? "this" : "self";
            return *this;
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->isType(iter))
            {
                if (is_python() && !parent->IsLocal() && !parent->IsForm())
                    *this += "self.";
                *this += parent->get_node_name();
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    Function("GetPane()");
                }
                return *this;
            }
        }
        parent = parent->GetParent();
    }

    ASSERT_MSG(parent, tt_string() << m_node->get_node_name() << " has no parent!");
    return *this;
}

Code& Code::QuotedString(GenEnum::PropName prop_name)
{
    if (!m_node->HasValue(prop_name))
    {
        if (is_cpp())
        {
            CheckLineLength(sizeof("wxEmptyString"));
            *this += "wxEmptyString";
        }
        else
        {
            *this += "\"\"";
        }
        return *this;
    }
    else
    {
        return QuotedString(m_node->as_string(prop_name));
    }
}

Code& Code::QuotedString(tt_string_view text)
{
    auto cur_pos = size();

    if (Project.as_bool(prop_internationalize))
    {
        *this += is_cpp() ? "_(" : "wx.GetTranslation(";
    }

    // This is only used by C++, but we need to know if it was set in order to generate closing parenthesis.
    bool has_utf_char = false;
    if (is_cpp())
    {
        for (auto iter: text)
        {
            if (iter < 0)
            {
                has_utf_char = true;
                break;
            }
        }

        if (has_utf_char)
        {
            *this += "wxString::FromUTF8(";
        }
    }

    *this += '"';
    for (auto c: text)
    {
        switch (c)
        {
            case '"':
                *this += "\\\"";
                break;

            // This generally isn't needed for C++, but is needed for Python
            case '\'':
                *this += "\\'";
                break;

            case '\\':
                *this += "\\\\";
                break;

            case '\t':
                *this += "\\t";
                break;

            case '\n':
                *this += "\\n";
                break;

            case '\r':
                *this += "\\r";
                break;

            default:
                *this += c;
                break;
        }
    }
    *this += '"';

    if (has_utf_char)
    {
        *this += ')';
    }
    if (Project.as_bool(prop_internationalize))
    {
        *this += ')';
    }

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::WxSize(GenEnum::PropName prop_name, bool enable_dlg_units)
{
    if (m_node->prop_as_wxSize(prop_name) == wxDefaultSize)
    {
        CheckLineLength(sizeof("wxDefaultSize"));
        *this += is_cpp() ? "wxDefaultSize" : "wx.DefaultSize";
        return *this;
    }

    auto cur_pos = size();

    bool dialog_units = m_node->value(prop_name).contains("d", tt::CASE::either);
    if (dialog_units && enable_dlg_units)
    {
        CheckLineLength(sizeof("self.ConvertDialogToPixels(wxSize(999, 999))"));
        FormFunction("ConvertDialogToPixels(");
    }

    auto size = m_node->prop_as_wxSize(prop_name);
    Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';

    if (dialog_units && enable_dlg_units)
        *this += ')';

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::Pos(GenEnum::PropName prop_name, bool enable_dlg_units)
{
    if (m_node->prop_as_wxPoint(prop_name) == wxDefaultPosition)
    {
        CheckLineLength(sizeof("wxDefaultPosition"));
        *this += is_cpp() ? "wxDefaultPosition" : "wx.DefaultPosition";
        return *this;
    }

    auto cur_pos = size();

    bool dialog_units = m_node->value(prop_name).contains("d", tt::CASE::either);
    if (dialog_units && enable_dlg_units)
    {
        CheckLineLength(sizeof("self.ConvertDialogToPixels(wxPoint(999, 999))"));
        FormFunction("ConvertDialogToPixels(");
    }

    auto size = m_node->prop_as_wxSize(prop_name);
    Class("wxPoint(").itoa(size.x).Comma().itoa(size.y) << ')';

    if (dialog_units && enable_dlg_units)
        *this += ')';

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::Style(const char* prefix, tt_string_view force_style)
{
    bool style_set = false;
    if (force_style.size())
    {
        Add(force_style);
        style_set = true;
    }

    if (m_node->HasValue(prop_tab_position) && !m_node->prop_as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        as_string(prop_tab_position);
    }
    if (m_node->HasValue(prop_orientation) && !m_node->prop_as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        as_string(prop_orientation);
    }

    // Note that as_string() may break the line, so recalculate any time as_string() is called
    auto cur_pos = size();

    if (m_node->isGen(gen_wxRichTextCtrl))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        Add("wxRE_MULTILINE");
    }

    if (m_node->HasValue(prop_style))
    {
        if (style_set)
            *this += '|';
        if (prefix)
        {
            if (is_cpp())
            {
                *this += m_node->prop_as_constant(prop_style, prefix);
            }
            else
            {
                tt_view_vector multistr(m_node->prop_as_constant(prop_style, prefix), "|", tt::TRIM::both);
                std::string_view wx_prefix = "wx.";
                auto lambda = [&](tt_string_view candidate)
                {
                    for (auto& iter_prefix: s_short_map)
                    {
                        if (candidate.starts_with(iter_prefix.first))
                        {
                            wx_prefix = iter_prefix.second;
                            return;
                        }
                    }
                    if (auto wx_iter = s_map_wx_prefix.find(candidate); wx_iter != s_map_wx_prefix.end())
                    {
                        wx_prefix = wx_iter->second;
                        return;
                    }
                };

                for (auto& iter: multistr)
                {
                    if (iter.empty())
                        continue;
                    if (style_set)
                        *this += '|';
                    if (iter.is_sameprefix("wx"))
                    {
                        lambda(iter);
                        *this << wx_prefix << iter.substr(2);
                    }
                    else
                        *this += iter;
                    style_set = true;
                }
            }
        }
        else
        {
            as_string(prop_style);
            cur_pos = size();
        }
        style_set = true;
    }

    if (m_node->HasValue(prop_window_style))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        as_string(prop_window_style);
        cur_pos = size();
    }

    if (m_node->isGen(gen_wxListView))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        as_string(prop_mode);
        cur_pos = size();
    }

    if (!style_set)
    {
        *this += "0";
    }

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::PosSizeFlags(bool uses_def_validator, tt_string_view def_style)
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

    // This could be done as a single if statement, but it is easier to read this way.
    bool style_needed = false;
    if ((m_node->HasValue(prop_style) && m_node->as_string(prop_style) != def_style))
        style_needed = true;
    else if (m_node->HasValue(prop_window_style))
        style_needed = true;
    else if (m_node->HasValue(prop_orientation) && !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
             !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
        style_needed = true;
    else if (m_node->HasValue(prop_tab_position) && !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
        style_needed = true;
    else if (m_node->isGen(gen_wxRichTextCtrl) || m_node->isGen(gen_wxListView))
        style_needed = true;

    // Do we need a style and/or a default validator?
    if (style_needed)
    {
        Comma();
        Pos().Comma().WxSize().Comma().Style();
        if (def_style.size() && ends_with(def_style))
        {
            erase(size() - def_style.size());
            if (back() == '|')
                pop_back();
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

int Code::WhatParamsNeeded(tt_string_view default_style) const
{
    if (m_node->HasValue(prop_window_name))
    {
        return (pos_needed | size_needed | style_needed | window_name_needed);
    }

    // This could be done as a single if statement, but it is easier to read this way.
    if ((m_node->HasValue(prop_style) && m_node->as_string(prop_style) != default_style))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->HasValue(prop_window_style))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->HasValue(prop_orientation) && !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
             !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->HasValue(prop_tab_position) && !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->isGen(gen_wxRichTextCtrl) || m_node->isGen(gen_wxListView))
        return (pos_needed | size_needed | style_needed);

    if (m_node->prop_as_wxSize(prop_size) != wxDefaultSize)
        return (pos_needed | size_needed);
    else if (m_node->prop_as_wxPoint(prop_pos) != wxDefaultPosition)
        return pos_needed;

    return nothing_needed;
}

Code& Code::PosSizeForceStyle(tt_string_view force_style, bool uses_def_validator)
{
    if (m_node->HasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        Comma();
        Pos().Comma().WxSize().Comma();
        Style(nullptr, force_style);
        if (uses_def_validator)
            Comma().Add("wxDefaultValidator");
        Comma();
        QuotedString(prop_window_name).EndFunction();
        return *this;
    }
    else
    {
        Comma();
        Pos().Comma().WxSize().Comma().Style(nullptr, force_style);
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
    auto cur_pos = size();

    Add("wxSizerFlags");

    if (auto& prop = m_node->prop_as_string(prop_proportion); prop != "0")
    {
        *this << '(' << prop << ')';
    }
    else
    {
        *this << "()";
    }

    if (auto& prop = m_node->prop_as_string(prop_alignment); prop.size())
    {
        if (prop.contains("wxALIGN_CENTER"))
        {
            // Note that CenterHorizontal() and CenterVertical() require wxWidgets 3.1 or higher. Their advantage is
            // generating an assert if you try to use one that is invalid if the sizer parent's orientation doesn't
            // support it. Center() just works without the assertion check.
            *this << ".Center()";
        }

        if (prop.contains("wxALIGN_LEFT"))
        {
            *this += ".Left()";
        }
        else if (prop.contains("wxALIGN_RIGHT"))
        {
            *this += ".Right()";
        }

        if (prop.contains("wxALIGN_TOP"))
        {
            *this += ".Top()";
        }
        else if (prop.contains("wxALIGN_BOTTOM"))
        {
            *this += ".Bottom()";
        }
    }

    if (auto& prop = m_node->prop_as_string(prop_flags); prop.size())
    {
        if (prop.contains("wxEXPAND"))
        {
            *this += ".Expand()";
        }
        if (prop.contains("wxSHAPED"))
        {
            *this += ".Shaped()";
        }
        if (prop.contains("wxFIXED_MINSIZE"))
        {
            *this += ".FixedMinSize()";
        }
        if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
        {
            *this += ".ReserveSpaceEvenIfHidden()";
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
                Add(".Border(").Add("wxALL, ") << border_size << ')';
            }
        }
        else
        {
            *this << ".Border(";
            tt_string border_flags;

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

            *this << border_flags << ", ";
            if (border_size == "5")
            {
                *this += is_cpp() ? "wxSizerFlags::GetDefaultBorder())" : "wx.SizerFlags.GetDefaultBorder())";
            }
            else
            {
                *this << border_size << ')';
            }
        }
    }

    m_auto_break = save_auto_break;

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

void Code::GenWindowSettings()
{
    if (HasValue(prop_window_extra_style))
    {
        if (m_node->IsForm())
        {
            FormFunction("SetExtraStyle(");
        }
        else
        {
            NodeName().Function("SetExtraStyle(");
        }

        Add(prop_window_extra_style).EndFunction();
    }

    if (IsTrue(prop_disabled))
    {
        Eol(eol_if_empty);
        if (!m_node->IsForm())
        {
            NodeName().Function("Enable(").AddFalse().EndFunction();
        }
        else
        {
            FormFunction("Enable(").AddFalse().EndFunction();
        }
    }

    if (IsTrue(prop_hidden))
    {
        Eol(eol_if_empty);
        if (!m_node->IsForm())
        {
            NodeName().Function("Hide(").EndFunction();
        }
        else
        {
            FormFunction("Hide(").EndFunction();
        }
    }

    bool allow_minmax { true };
    if (m_node->IsForm() && !m_node->isGen(gen_PanelForm) && !m_node->isGen(gen_wxToolBar))
        allow_minmax = false;

    if (allow_minmax && m_node->as_wxSize(prop_minimum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        if (!m_node->IsForm())
        {
            NodeName().Function("SetMinSize(");
        }
        else
        {
            FormFunction("SetMinSize(");
        }
        WxSize(prop_minimum_size).EndFunction();
    }

    if (allow_minmax && m_node->as_wxSize(prop_maximum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        if (!m_node->IsForm())
        {
            NodeName().Function("SetMaxSize(");
        }
        else
        {
            FormFunction("SetMaxSize(");
        }
        WxSize(prop_maximum_size).EndFunction();
    }

    if (!m_node->IsForm() && !m_node->isPropValue(prop_variant, "normal"))
    {
        Eol(eol_if_empty).NodeName().Function("SetWindowVariant(");
        if (m_node->isPropValue(prop_variant, "small"))
            Add("wxWINDOW_VARIANT_SMALL");
        else if (m_node->isPropValue(prop_variant, "mini"))
            Add("wxWINDOW_VARIANT_MINI");
        else
            Add("wxWINDOW_VARIANT_LARGE");

        EndFunction();
    }

    if (HasValue(prop_tooltip))
    {
        Eol(eol_if_empty);
        if (!m_node->IsForm())
            NodeName().Function("SetToolTip(");
        else
            FormFunction("SetToolTip(");
        QuotedString(prop_tooltip).EndFunction();
    }

    if (HasValue(prop_context_help))
    {
        Eol(eol_if_empty);
        if (!m_node->IsForm())
            NodeName().Function("SetHelpText(");
        else
            FormFunction("SetHelpText(");
        QuotedString(prop_context_help).EndFunction();
    }

    GenFontColourSettings();
}

void Code::GenFontColourSettings()
{
    auto* node = m_node;
    if (HasValue(prop_font))
    {
        FontProperty fontprop(node->get_prop_ptr(prop_font));
        if (fontprop.isDefGuiFont())
        {
            OpenBrace();
            Add("wxFont font(").Add("wxSystemSettings").ClassMethod("GetFont(").Add("wxSYS_DEFAULT_GUI_FONT").Str(")");
            EndFunction();

            if (fontprop.GetSymbolSize() != wxFONTSIZE_MEDIUM)
                Eol().Str("font.SetSymbolicSize(").Add(font_symbol_pairs.GetValue(fontprop.GetSymbolSize())).EndFunction();
            if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
                Eol().Str("font.SetStyle(").Add(font_style_pairs.GetValue(fontprop.GetStyle())).EndFunction();
            if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
                Eol().Str("font.SetWeight(").Str(font_weight_pairs.GetValue(fontprop.GetWeight())).EndFunction();
            if (fontprop.IsUnderlined())
                Eol().Str("font.SetUnderlined(").AddTrue().EndFunction();
            if (fontprop.IsStrikethrough())
                Eol().Str("font.SetStrikethrough(").AddTrue().EndFunction();
            Eol();

            if (node->IsForm())
            {
                FormFunction("SetFont(font").EndFunction();
                CloseBrace();
            }
            else if (node->isGen(gen_wxStyledTextCtrl))
            {
                NodeName().Function("StyleSetFont(").Add("wxSTC_STYLE_DEFAULT");
                Comma().Str("font").EndFunction();
                CloseBrace();
            }
            else
            {
                NodeName().Function("SetFont(font").EndFunction();
                CloseBrace();
            }
        }
        else  // not isDefGuiFont()
        {
            const auto point_size = fontprop.GetFractionalPointSize();
            OpenBrace();
            Add("wxFontInfo font_info(");
            if (point_size != static_cast<int>(point_size))  // is there a fractional value?
            {
                if (is_cpp() && Project.value(prop_wxWidgets_version) == "3.1")
                {
                    Eol().Str("#if !wxCHECK_VERSION(3, 1, 2)").Eol().Tab();

                    if (point_size <= 0)
                    {
                        Add("wxSystemSettings").ClassMethod("GetFont()").Function("GetPointSize()").EndFunction();
                    }
                    else
                    {
                        // GetPointSize() will round the result rather than truncating the decimal
                        itoa(fontprop.GetPointSize()).EndFunction();
                    }
                }
                else
                {
                    if (is_cpp() && Project.value(prop_wxWidgets_version) == "3.1")
                    {
                        Eol().Str("#else // fractional point sizes are new to wxWidgets 3.1.2").Eol().Tab();
                    }

                    std::array<char, 10> float_str;
                    if (auto [ptr, ec] = std::to_chars(float_str.data(), float_str.data() + float_str.size(), point_size);
                        ec == std::errc())
                    {
                        Str(std::string_view(float_str.data(), ptr - float_str.data())).EndFunction();
                    }

                    // REVIEW: [Randalphwa - 12-30-2022] We don't output anything if std::to_chars() results in an error

                    if (is_cpp() && Project.value(prop_wxWidgets_version) == "3.1")
                    {
                        Eol().Str("#endif");
                    }
                }
            }
            else
            {
                if (point_size <= 0)
                {
                    Add("wxSystemSettings").ClassMethod("GetFont()").Function("GetPointSize()").EndFunction();
                }
                else
                {
                    // GetPointSize() will round the result rather than truncating the decimal
                    itoa(fontprop.GetPointSize()).EndFunction();
                }
            }

            Eol(eol_if_needed).Str("font_info.");
            if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
                Str("FaceName(").QuotedString(tt_string() << fontprop.GetFaceName().wx_str()) += ").";
            if (fontprop.GetFamily() != wxFONTFAMILY_DEFAULT)
                Str("Family(").Str(font_family_pairs.GetValue(fontprop.GetFamily())) += ").";
            if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
                Str("Style(").Str(font_style_pairs.GetValue(fontprop.GetStyle())) += ").";
            if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
                Str("Weight(").Str(font_weight_pairs.GetValue(fontprop.GetWeight())) += ").";
            if (fontprop.IsUnderlined())
                Str("Underlined().");
            if (fontprop.IsStrikethrough())
                Str("Strikethrough()");

            if (back() == '.')
            {
                pop_back();
            }
            if (is_cpp())
                *this += ';';
            Eol();

            if (node->IsForm())
            {
                FormFunction("SetFont(").Add("wxFont(font_info)").EndFunction();
            }
            else
            {
                NodeName().Function("SetFont(wxFont(font_info)").EndFunction();
            }
            CloseBrace();
        }
    }  // End of font handling code

    if (auto& fg_clr = node->prop_as_string(prop_foreground_colour); fg_clr.size())
    {
        Eol(eol_if_needed);
        if (node->IsForm())
        {
            FormFunction("SetForegroundColour(");
        }
        else
        {
            NodeName().Function("SetForegroundColour(");
        }
        if (fg_clr.contains("wx"))
        {
            Add("wxSystemSettings").ClassMethod("GetColour(").Add(fg_clr) += ")";
        }
        else
        {
            const auto colour = m_node->as_wxColour(prop_foreground_colour);
            Add(tt_string().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue()));
        }
        EndFunction();
    }

    if (auto& bg_clr = node->prop_as_string(prop_background_colour); bg_clr.size())
    {
        Eol(eol_if_needed);
        if (node->IsForm())
        {
            FormFunction("SetBackgroundColour(");
        }
        else
        {
            NodeName().Function("SetBackgroundColour(");
        }
        if (bg_clr.contains("wx"))
        {
            Add("wxSystemSettings").ClassMethod("GetColour(").Add(bg_clr) += ")";
        }
        else
        {
            const auto colour = m_node->as_wxColour(prop_background_colour);
            Add(tt_string().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue()));
        }
        EndFunction();
    }
}

Code& Code::AddComment(tt_string_view text)
{
    if (empty() || !tt::is_whitespace(back()))
    {
        *this << ' ';
    }

    if (is_cpp())
    {
        *this << "// " << text;
    }
    else
    {
        *this << "# " << text;
    }
    return *this;
}

Code& Code::ColourCode(GenEnum::PropName prop_name)
{
    if (!HasValue(prop_name))
    {
        Add("wxNullColour");
    }
    else
    {
        if (PropContains(prop_name, "wx"))
        {
            Add("wxSystemSettings").ClassMethod("GetColour(").Add(prop_name).Str(")");
        }
        else
        {
            auto colour = m_node->as_wxColour(prop_name);
            Add(tt_string().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue()));
        }
    }

    return *this;
}
