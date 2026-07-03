/////////////////////////////////////////////////////////////////////////////
// Purpose:   Size and position code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-29-2026]

#include "code.h"

#include "node.h"  // Node class

using namespace code;

Code& Code::WxSize(GenEnum::PropName prop_name, int enable_dpi_scaling)
{
    return WxSize(m_node->as_wxSize(prop_name), enable_dpi_scaling);
}

void Code::AddScaledSizeRuby(wxSize size)
{
    FormFunction("FromDIP(");
    Class("Wx::Size.new(").itoa(size.x).Comma().itoa(size.y) << ')';
    *this += ')';
}

void Code::AddUnscaledSizeRuby(wxSize size)
{
    Class("Wx::Size.new(").itoa(size.x).Comma().itoa(size.y) << ')';
}

void Code::AddScaledSizePerl(wxSize size)
{
    FormFunction("FromDIP->new(");
    Class("Wx::Size->new(").itoa(size.x).Comma().itoa(size.y) << "))";
}

void Code::AddUnscaledSizePerl(wxSize size)
{
    Class("Wx::Size->new(").itoa(size.x).Comma().itoa(size.y) << ')';
}

Code& Code::WxSize(wxSize size, int enable_dpi_scaling)
{
    const size_t cur_pos = this->size();
    const bool size_scaling = is_ScalingEnabled(prop_size, enable_dpi_scaling);

    if (is_ruby())
    {
        return WxSize_Ruby(size, cur_pos, size_scaling);
    }

    return WxSize_Other(size, cur_pos, size_scaling);
}

Code& Code::WxSize_Ruby(wxSize size, size_t cur_pos, bool size_scaling)
{
    if (size == wxDefaultSize)
    {
        CheckLineLength((sizeof("Wx::DEFAULT_SIZE") - 1));
        *this += "Wx::DEFAULT_SIZE";
        return *this;
    }

    if (size_scaling)
    {
        CheckLineLength(sizeof(", from_DIP(Wx::Size.new(999, 999))"));
        AddScaledSizeRuby(size);
    }
    else
    {
        CheckLineLength(sizeof("Wx::Size.new(999, 999)"));
        AddUnscaledSizeRuby(size);
    }

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::WxSize_Other(wxSize size, size_t cur_pos, bool size_scaling)
{
    if (size == wxDefaultSize)
    {
        CheckLineLength((sizeof("DefaultSize") - 1) + m_language_wxPrefix.size());
        *this << m_language_wxPrefix << "DefaultSize";
        return *this;
    }

    if (size_scaling)
    {
        CheckLineLength(sizeof("FromDIP(wxSize(999, 999))"));
        FormFunction("FromDIP(");
        Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';
        *this += ')';
    }
    else
    {
        CheckLineLength(sizeof("wxSize(999, 999)"));
        Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';
    }

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::WxPoint(wxPoint position, int enable_dpi_scaling)
{
    const size_t cur_pos = this->size();
    const bool pos_scaling = is_ScalingEnabled(prop_pos, enable_dpi_scaling);

    if (is_ruby())
    {
        if (position == wxDefaultPosition)
        {
            CheckLineLength((sizeof("Wx::DEFAULT_POSITION") - 1));
            *this += "Wx::DEFAULT_POSITION";
            return *this;
        }

        if (pos_scaling)
        {
            CheckLineLength(sizeof(", from_DIP(Wx::Point.new(999, 999))"));
        }
        else
        {
            CheckLineLength(sizeof("Wx::Point.new(999, 999)"));
        }

        if (pos_scaling)
        {
            FormFunction("FromDIP(");
            Class("Wx::Point.new(").itoa(position.x).Comma().itoa(position.y) << ')';
            *this += ')';
        }
        else
        {
            Class("Wx::Point.new(").itoa(position.x).Comma().itoa(position.y) << ')';
        }

        if (m_auto_break && this->size() > m_break_at)
        {
            InsertLineBreak(cur_pos);
        }

        return *this;
    }

    // The following code is for non-Ruby languages

    if (position == wxDefaultPosition)
    {
        CheckLineLength((sizeof("DefaultPosition") - 1) + m_language_wxPrefix.size());
        *this << m_language_wxPrefix << "DefaultPosition";
        return *this;
    }

    if (pos_scaling)
    {
        if (is_cpp())
        {
            CheckLineLength(sizeof("FromDIP(wxPoint(999, 999))"));
        }
        else
        {
            // Python and FFI languages have a self-reference prefix, use conservative estimate
            CheckLineLength(sizeof("self.FromDIP(wxPoint(999, 999))"));
        }
        FormFunction("FromDIP(");
        Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
        *this += ')';
    }
    else
    {
        CheckLineLength(sizeof("wxPoint(999, 999)"));
        Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
    }

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}
