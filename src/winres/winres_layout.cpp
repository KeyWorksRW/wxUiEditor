/////////////////////////////////////////////////////////////////////////////
// Purpose:   rcForm layout code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "winres_form.h"

#include "node_creator.h"  // NodeCreator -- Class used to create nodes

void rcForm::AddSizersAndChildren()
{
    // First sort all children horizontally
    // std::sort(std::begin(m_ctrls), std::end(m_ctrls), [](rcCtrl a, rcCtrl b) { return a.GetLeft() < b.GetLeft(); });

    // Now sort vertically
    // std::sort(std::begin(m_ctrls), std::end(m_ctrls), [](rcCtrl a, rcCtrl b) { return a.GetTop() < b.GetTop(); });
    std::sort(m_ctrls.begin(), m_ctrls.end(), [](rcCtrl a, rcCtrl b) { return a.GetTop() < b.GetTop(); });

    for (size_t begin = 0; begin < m_ctrls.size() - 1; ++begin)
    {
        auto end = begin + 1;
        while (m_ctrls[end].GetTop() == m_ctrls[begin].GetTop())
        {
            ++end;
            if (end >= m_ctrls.size())
                break;
        }

        if (end > begin + 1)
        {
            std::sort(m_ctrls.begin() + begin, m_ctrls.begin() + end,
                      [](rcCtrl a, rcCtrl b) { return a.GetLeft() < b.GetLeft(); });
        }
    }

    auto parent = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, m_node.get());
    parent->prop_set_value(prop_var_name, "dlg_sizer");
    m_node->Adopt(parent);

    for (size_t idx_child = 0; idx_child < m_ctrls.size(); ++idx_child)
    {
        auto& child = m_ctrls[idx_child];
        // A group box will have added a bunch of children, so if the current child has been added already then ignore it.
        // Note that children can appear to the left or right of a group box, which is why we can't just step over a group of
        // children when a group box is encountered.
        if (child.isAdded())
            continue;

        if (child.GetNode()->isGen(gen_wxStaticBoxSizer))
        {
            if (child.GetWidth() > GetWidth() - 30)
                child.GetNode()->prop_set_value(prop_flags, "wxEXPAND");

            AddStaticBoxChildren(child, idx_child);
            parent->Adopt(child.GetNodePtr());
            child.setAdded();
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
            auto sizer = g_NodeCreator.CreateNode(gen_wxBoxSizer, parent.get());
            parent->Adopt(sizer);
            sizer->prop_set_value(prop_orientation, "wxHORIZONTAL");

            while (idx_child < m_ctrls.size() && m_ctrls[idx_child].GetTop() == child.GetTop())
            {
                if (m_ctrls[idx_child].isAdded())
                    break;  // means there was a static box to the right

                // Note that we add the child we are comparing to first.
                sizer->Adopt(m_ctrls[idx_child].GetNodePtr());
                ++idx_child;
            }
            if (idx_child < m_ctrls.size() && m_ctrls[idx_child].isAdded())
                continue;

            // In order to properly step through the loop
            --idx_child;

            if (m_ctrls[idx_child].GetLeft() + m_ctrls[idx_child].GetWidth() > m_width - 10)
            {
                sizer->prop_set_value(prop_alignment, "wxALIGN_RIGHT");
            }
        }
        else
        {
            auto sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, parent.get());
            parent->Adopt(sizer);
            sizer->Adopt(child.GetNodePtr());
            if (idx_child == 0)
                continue;

            while (idx_child < m_ctrls.size() && m_ctrls[idx_child].GetTop() != m_ctrls[idx_child - 1].GetTop())
            {
                if (m_ctrls[idx_child].isAdded())
                    break;  // means there was a static box to the right

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

void rcForm::AddStaticBoxChildren(const rcCtrl& box, size_t idx_group_box)
{
    CollectGroupControls(idx_group_box);

    const auto& static_box = m_ctrls[idx_group_box];
    for (size_t idx_child = 0; idx_child < m_group_ctrls.size(); ++idx_child)
    {
        auto result = GroupGridSizerNeeded(idx_child);
        if (result < 0)
        {
            // No vertical alignment with the next control, so just add it normally
            auto& child = reinterpret_cast<rcCtrl&>(*m_group_ctrls[idx_child]);
            Adopt(static_box.GetNodePtr(), child);

            // REVIEW: [KeyWorks - 06-16-2021] Does this actually happen in a group box?
            int dlg_margin = (box.GetWidth() / 2) - child.GetWidth();
            if (child.GetRight() < (box.GetWidth() - dlg_margin))
                child.GetNode()->prop_set_value(prop_alignment, "wxALIGN_CENTER_HORIZONTAL");
            continue;
        }
        else if (result == 0)
        {
            // Single row vertical alignment with now horizontal alignment in the next row, so use a horizontal box sizer
            auto sizer = g_NodeCreator.CreateNode(gen_wxBoxSizer, box.GetNode());
            sizer->prop_set_value(prop_orientation, "wxHORIZONTAL");
            static_box.GetNodePtr()->Adopt(sizer);

            auto& child = reinterpret_cast<rcCtrl&>(*m_group_ctrls[idx_child]);

            while (idx_child < m_group_ctrls.size() && m_group_ctrls[idx_child]->GetTop() == child.GetTop())
            {
                // Note that we add the child we are comparing to first.
                Adopt(sizer, *m_group_ctrls[idx_child]);
                ++idx_child;
            }
            // In order to properly step through the loop
            --idx_child;
        }
        else
        {
            // There's more than one row with vertical alignment, and at least one of those rows has a column with horizontal
            // alignment, so use a wxFlexGridSizer.

            auto total_columns = result;  // This is just for readability
            auto sizer = g_NodeCreator.CreateNode(gen_wxFlexGridSizer, box.GetNode());
            sizer->prop_set_value(prop_cols, ttlib::itoa(total_columns));
            static_box.GetNodePtr()->Adopt(sizer);

            auto& child = reinterpret_cast<rcCtrl&>(*m_group_ctrls[idx_child]);
            std::vector<int> positions;
            positions.reserve(total_columns);
            for (size_t idx = 0; idx < total_columns; ++idx)
            {
                if (m_group_ctrls[idx_child + idx]->GetTop() == child.GetTop())
                    positions.emplace_back(m_group_ctrls[idx_child + idx]->GetLeft());
                else
                    positions.emplace_back(-1);
            }

            // It's possible that there's an initial group of aligned controls followed by some randomly positioned control.
            // However, that should be a rare case, so we assume that if we're using a grid, then all the rest of the
            // controls in the group box should be placed into that grid.

            // There may be gaps in the columns, so cur_column is used to track which column is being added.
            int cur_column = 0;
            for (; idx_child < m_group_ctrls.size(); ++idx_child)
            {
                // This covers the case where the previous rows had gaps in the columns
                if (positions[cur_column] == -1)
                    positions[cur_column] = m_group_ctrls[idx_child]->GetLeft();

                while (m_group_ctrls[idx_child]->GetLeft() > positions[cur_column])
                {
                    auto spacer = g_NodeCreator.CreateNode(gen_spacer, sizer.get());
                    sizer->Adopt(spacer);
                    ++cur_column;
                    if (cur_column >= total_columns)
                    {
                        break;
                    }
                }
                Adopt(sizer, *m_group_ctrls[idx_child]);
                ++cur_column;
                if (cur_column >= total_columns)
                    cur_column = 0;
            }
            return;
        }
    }

    // TODO: [KeyWorks - 05-31-2021] Depending on the number and position of the children, we may need to change the
    // orientation as well as spanning more than one column or row.
}

int rcForm::GridSizerNeeded(size_t idx_start, size_t idx_end, const rcCtrl* /* p_static_box */)
{
    ASSERT(idx_end < m_ctrls.size());

    if (idx_start + 1 > idx_end || m_ctrls[idx_start + 1].GetTop() != m_ctrls[idx_start].GetTop())
        return -1;

    size_t row_children = 2;
    while (idx_start + row_children < idx_end && m_ctrls[idx_start + row_children].GetTop() != m_ctrls[idx_start].GetTop())
        ++row_children;

    size_t idx_next_row = idx_start + row_children;
    if (idx_next_row >= idx_end)
        return 0;  // only one aligned row, so a box sizer is needed

    size_t max_columns = row_children;

    while (idx_next_row < idx_end && m_ctrls[idx_next_row + 1].GetTop() != m_ctrls[idx_next_row].GetTop())
    {
        row_children = 2;
        while (idx_next_row + row_children < idx_end &&
               m_ctrls[idx_next_row + row_children].GetTop() != m_ctrls[idx_next_row].GetTop())
            ++row_children;
        if (row_children > max_columns)
            max_columns = row_children;

        idx_next_row = idx_next_row + row_children;
        if (idx_next_row >= idx_end)
            break;
    }

    return static_cast<int>(max_columns);
}

int rcForm::GroupGridSizerNeeded(size_t idx_start) const
{
    if (idx_start + 1 >= m_group_ctrls.size() || m_group_ctrls[idx_start + 1]->GetTop() != m_group_ctrls[idx_start]->GetTop())
        return -1;

    size_t row_children = 2;
    while (idx_start + row_children < m_group_ctrls.size() &&
           m_group_ctrls[idx_start + row_children]->GetTop() == m_group_ctrls[idx_start]->GetTop())
        ++row_children;

    size_t idx_next_row = idx_start + row_children;
    if (idx_next_row + 1 >= m_group_ctrls.size() ||
        m_group_ctrls[idx_next_row + +1]->GetTop() != m_group_ctrls[idx_next_row]->GetTop())
        return 0;  // only one aligned row, so a box sizer is needed

    size_t max_columns = row_children;

    while (idx_next_row + 1 < m_group_ctrls.size() &&
           m_group_ctrls[idx_next_row + 1]->GetTop() == m_group_ctrls[idx_next_row]->GetTop())
    {
        row_children = 2;
        while (idx_next_row + row_children < m_group_ctrls.size() &&
               m_group_ctrls[idx_next_row + row_children]->GetTop() == m_group_ctrls[idx_next_row]->GetTop())
            ++row_children;
        if (row_children > max_columns)
            max_columns = row_children;

        idx_next_row = idx_next_row + row_children;
    }

    return static_cast<int>(max_columns);
}

void rcForm::CollectGroupControls(size_t idx_parent)
{
    m_group_ctrls.clear();
    auto& rc_parent = m_ctrls[idx_parent].GetDialogRect();

    for (size_t idx = idx_parent + 1; idx < m_ctrls.size(); ++idx)
    {
        auto& child_rc = m_ctrls[idx].GetDialogRect();
        if (rc_parent.Contains(m_ctrls[idx].GetDialogRect()))
        {
            m_group_ctrls.emplace_back(&m_ctrls[idx]);
            continue;
        }
        else
        {
            // It's possible the control is to the left or right of the group box in which case we keep looking. However, if
            // it's below it, then we're done.
            if (child_rc.GetTop() >= (rc_parent.GetTop() + rc_parent.GetHeight()))
                return;
        }
    }
}

void rcForm::Adopt(const NodeSharedPtr& node, rcCtrl& child)
{
    ASSERT_MSG(!child.isAdded(), "Logic problem, child has already been added.");

    node->Adopt(child.GetNodePtr());
    child.setAdded();
}
