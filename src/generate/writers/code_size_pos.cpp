/////////////////////////////////////////////////////////////////////////////
// Purpose:   Size and position code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "code.h"

#include "node.h"  // Node class

using namespace code;

auto Code::WxSize(GenEnum::PropName prop_name, int enable_dpi_scaling) -> Code&
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

auto Code::WxSize(wxSize size, int enable_dpi_scaling) -> Code&
{
    auto cur_pos = this->size();
    auto size_scaling = is_ScalingEnabled(prop_size, enable_dpi_scaling);

    if (is_ruby())
    {
        return WxSize_Ruby(size, cur_pos, size_scaling);
    }
    if (is_perl())
    {
        return WxSize_Perl(size, cur_pos, size_scaling);
    }
    return WxSize_Other(size, cur_pos, size_scaling);
}

auto Code::WxSize_Ruby(wxSize size, size_t cur_pos, bool size_scaling) -> Code&
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

auto Code::WxSize_Perl(wxSize size, size_t cur_pos, bool size_scaling) -> Code&
{
    if (size == wxDefaultSize)
    {
        CheckLineLength((sizeof("wxDefaultSize") - 1));
        *this += "wxDefaultSize";
        return *this;
    }

    if (size_scaling)
    {
        CheckLineLength(sizeof(", $self->FromDIP->new(Wx::Size->new(999, 999))"));
        AddScaledSizePerl(size);
    }
    else
    {
        CheckLineLength(sizeof("Wx::Size->new(999, 999)"));
        AddUnscaledSizePerl(size);
    }

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

auto Code::WxSize_Other(wxSize size, size_t cur_pos, bool size_scaling) -> Code&
{
    if (size == wxDefaultSize)
    {
        CheckLineLength((sizeof("DefaultSize") - 1) + m_language_wxPrefix.size());
        if (is_perl())
        {
            *this << "wxDefaultSize";
        }
        else
        {
            *this << m_language_wxPrefix << "DefaultSize";
        }
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

auto Code::WxPoint(wxPoint position, int enable_dpi_scaling) -> Code&
{
    auto cur_pos = this->size();
    auto pos_scaling = is_ScalingEnabled(prop_pos, enable_dpi_scaling);

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
        if (is_perl())
        {
            // Perl uses wxDefaultPosition
            *this << "wxDefaultPosition";
        }
        else
        {
            *this << m_language_wxPrefix << "DefaultPosition";
        }
        return *this;
    }

    if (pos_scaling)
    {
        if (is_cpp())
        {
            CheckLineLength(sizeof("FromDIP(wxPoint(999, 999))"));
            FormFunction("FromDIP(");
            Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
            *this += ')';
        }
        else if (is_python())
        {
            CheckLineLength(sizeof("self.FromDIP(wxPoint(999, 999))"));
            FormFunction("FromDIP(");
            Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
            *this += ')';
        }
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
