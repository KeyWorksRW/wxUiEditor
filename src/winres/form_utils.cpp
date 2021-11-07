/////////////////////////////////////////////////////////////////////////////
// Purpose:   resForm utility functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "winres_form.h"

#include "node_creator.h"  // NodeCreator -- Class used to create nodes

bool resForm::is_same_top(const resCtrl* left, const resCtrl* right)
{
    if (left->du_top() == right->du_top())
        return true;
    if (left->GetNode()->isGen(gen_wxStaticText))
    {
        if (left->du_top() - 1 == right->du_top() || left->du_top() - 2 == right->du_top())
            return true;
    }
    return false;
}

bool resForm::is_within_vertical(const resCtrl* left, const resCtrl* right) const
{
    if (left->du_top() >= right->du_top() && left->du_bottom() <= right->du_bottom())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void resForm::SortCtrls()
{
    // Sort controls vertically. For each group of controls that have the same vertical position, sort them horizontally.

    std::sort(m_ctrls.begin(), m_ctrls.end(),
              [](resCtrl a, resCtrl b)
              {
                  if (a.du_top() == b.du_top())
                      return (a.du_left() < b.du_left());
                  else
                      return (a.du_top() < b.du_top());
              });

    // Sometimes a static text control will be placed to the left of another control such as an edit control, and moved down
    // a little bit so that it aligns with the control it precedes. When we sorted controls vertically, the static text
    // control will appear below the control it is supposed to precede.

    for (size_t idx = 1; idx < m_ctrls.size(); ++idx)
    {
        if (m_ctrls[idx].isGen(gen_wxStaticText) && is_same_top(&m_ctrls[idx], &m_ctrls[idx - 1]))
        {
            if (m_ctrls[idx - 1].du_left() > m_ctrls[idx].du_left() + m_ctrls[idx].du_width())
            {
                std::swap(m_ctrls[idx - 1], m_ctrls[idx]);
                m_ctrls[idx - 1].GetDialogRect().SetTop(m_ctrls[idx].du_top());
            }
        }
    }

    // After the initial sorting, some tweaks may have been made to change positions -- such as when a text control is
    // preceded by a static text control that is centered vertically. We may add other changes as well, such as moving
    // OK/Cancel buttons that were set on the right side. Changing the top position of a control will typically mean that it
    // needs to be resorted horizonally, so we fix that here.

    for (size_t begin = 0; begin < m_ctrls.size() - 1; ++begin)
    {
        auto end = begin + 1;
        while (is_same_top(&m_ctrls[begin], &m_ctrls[end]))
        {
            ++end;
            if (end >= m_ctrls.size())
                break;
        }

        if (end > begin + 1)
        {
            std::sort(m_ctrls.begin() + begin, m_ctrls.begin() + end,
                      [](resCtrl a, resCtrl b) { return a.du_left() < b.du_left(); });
        }
    }
}
