/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <ttmultistr_wx.h>  // ttMultiString -- Class for handling multiple strings

#include "code.h"

#include "node.h"  // Node class

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

Code& Code::Assign(ttlib::sview class_name)
{
    m_code << " = ";
    if (m_language == GEN_LANG_CPLUSPLUS)
    {
        m_code << "new " << class_name;
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
    if (m_language == GEN_LANG_CPLUSPLUS)
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
            else
                m_code << "wx." << iter.substr(2);
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
