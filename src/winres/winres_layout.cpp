/////////////////////////////////////////////////////////////////////////////
// Purpose:   rcForm layout code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "winres_form.h"

#include "node_creator.h"   // NodeCreator -- Class used to create nodes

void rcForm::AddSizersAndChildren()
{
    // First sort all children horizontally
    std::sort(std::begin(m_ctrls), std::end(m_ctrls), [](rcCtrl a, rcCtrl b) { return a.GetLeft() < b.GetLeft(); });

    // Now sort vertically
    std::sort(std::begin(m_ctrls), std::end(m_ctrls), [](rcCtrl a, rcCtrl b) { return a.GetTop() < b.GetTop(); });

    auto parent = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, m_node.get());
    m_node->Adopt(parent);

    NodeSharedPtr sizer;

    for (size_t idx_child = 0; idx_child < m_ctrls.size(); ++idx_child)
    {
        const auto& child = m_ctrls[idx_child];
        if (child.GetNode()->isGen(gen_wxStaticBoxSizer))
        {
            AddStaticBoxChildren(idx_child);
            parent->Adopt(child.GetNodePtr());
            continue;
        }

        if (idx_child + 1 >= m_ctrls.size())
        {
            // If last control is a button, we may need to center or right-align it.
            if (child.GetNode()->isGen(gen_wxButton))
            {
                int dlg_margin = (GetWidth() / 2) - child.GetWidth();
                if (child.GetLeft() > dlg_margin)
                {
                    if (child.GetRight() < (GetWidth() - dlg_margin))
                        child.GetNode()->prop_set_value(prop_alignment, "wxALIGN_CENTER_HORIZONTAL");
                    else
                        child.GetNode()->prop_set_value(prop_alignment, "wxALIGN_RIGHT");
                }
            }

            // orphaned child, add to form's top level sizer
            parent->Adopt(child.GetNodePtr());
            return;
        }

        if (m_ctrls[idx_child + 1].GetTop() == child.GetTop())
        {
            // If there is more than one child with the same top position, then create a horizontal box sizer
            // and add all children with the same top position.
            sizer = g_NodeCreator.CreateNode(gen_wxBoxSizer, parent.get());
            parent->Adopt(sizer);
            sizer->prop_set_value(prop_orientation, "wxHORIZONTAL");

            while (idx_child < m_ctrls.size() && m_ctrls[idx_child].GetTop() == child.GetTop())
            {
                // Note that we add the child we are comparing to first.
                sizer->Adopt(m_ctrls[idx_child].GetNodePtr());
                ++idx_child;
            }
            // In order to properly step through the loop
            --idx_child;

            if (m_ctrls[idx_child].GetLeft() + m_ctrls[idx_child].GetWidth() > m_width - 10)
            {
                sizer->prop_set_value(prop_alignment, "wxALIGN_RIGHT");
            }
        }
        else
        {
            sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, parent.get());
            parent->Adopt(sizer);
            sizer->Adopt(child.GetNodePtr());

            if (idx_child + 2 < m_ctrls.size())
            {
                // If the next two sizers have the same top, then they need to be placed in a horizontal sizer.
                if (m_ctrls[idx_child + 1].GetTop() == m_ctrls[idx_child + 2].GetTop())
                    continue;
            }
            ++idx_child;

            while (idx_child < m_ctrls.size() && m_ctrls[idx_child].GetTop() != m_ctrls[idx_child - 1].GetTop())
            {
                // Note that we add the child we are comparing to first.
                sizer->Adopt(m_ctrls[idx_child].GetNodePtr());
                if (idx_child + 2 < m_ctrls.size())
                {
                    // If the next two sizers have the same top, then they need to be placed in a horizontal sizer.
                    if (m_ctrls[idx_child + 1].GetTop() == m_ctrls[idx_child + 2].GetTop())
                        break;
                }
                ++idx_child;
            }
        }
    }

    parent->FixDuplicateNodeNames();
}

void rcForm::AddStaticBoxChildren(size_t& idx_child)
{
    const auto& static_box = m_ctrls[idx_child];
    for (size_t idx_group_child = idx_child + 1; idx_group_child < m_ctrls.size(); ++idx_group_child)
    {
        const auto& child_ctrl = m_ctrls[idx_group_child];
        if (child_ctrl.GetRight() > static_box.GetRight() || child_ctrl.GetTop() > static_box.GetBottom())
            break;
        static_box.GetNode()->Adopt(child_ctrl.GetNodePtr());

        // Update to that caller won't process this child.
        ++idx_child;
    }

    // TODO: [KeyWorks - 05-31-2021] Depending on the number and position of the children, we may need to change the
    // orientation as well as spanning more than one column or row.
}
