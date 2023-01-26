/////////////////////////////////////////////////////////////////////////////
// Purpose:   resForm utility functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "winres_form.h"

#include "node_creator.h"  // NodeCreator -- Class used to create nodes

bool resForm::is_same_top(const resCtrl* left, const resCtrl* right, bool loose_check) const
{
    if (left->du_top() == right->du_top())
        return true;

    if (loose_check)
    {
        if (left->du_top() - 1 == right->du_top() || left->du_top() - 2 == right->du_top())
            return true;
        else if (left->du_top() == right->du_top() - 1 || left->du_top() == right->du_top() - 2)
            return true;
        else
            return false;
    }

    if (left->GetNode()->isGen(gen_wxStaticText))
    {
        if (left->du_top() - 1 == right->du_top() || left->du_top() - 2 == right->du_top())
            return true;
    }
    return false;
}

bool resForm::is_same_right(const std::vector<resCtrl>& ctrls, size_t child_a, size_t child_b) const
{
    if (ctrls[child_a].du_left() != ctrls[child_b].du_left())
        return false;
    if (ctrls[child_a].du_left() + ctrls[child_a].du_width() != ctrls[child_b].du_left() + ctrls[child_b].du_width())
        return false;
    return true;
}

bool resForm::is_same_top(const std::vector<resCtrl>& ctrls, size_t child_a, size_t child_b, bool loose_check) const
{
    if (ctrls[child_a].du_top() == ctrls[child_b].du_top())
        return true;

    if (loose_check)
    {
        if (ctrls[child_a].du_top() - 1 == ctrls[child_b].du_top() || ctrls[child_a].du_top() - 2 == ctrls[child_b].du_top())
            return true;
        else if (ctrls[child_a].du_top() == ctrls[child_b].du_top() - 1 ||
                 ctrls[child_a].du_top() == ctrls[child_b].du_top() - 2)
            return true;
        else
            return false;
    }

    if (ctrls[child_a].GetNode()->isGen(gen_wxStaticText))
    {
        if (ctrls[child_a].du_top() - 1 == ctrls[child_b].du_top() || ctrls[child_a].du_top() - 2 == ctrls[child_b].du_top())
            return true;
    }
    return false;
}

bool resForm::is_same_top(const std::vector<std::reference_wrapper<resCtrl>>& ctrls, size_t child_a, size_t child_b,
                          bool loose_check) const
{
    if (ctrls[child_a].get().du_top() == ctrls[child_b].get().du_top())
        return true;

    if (loose_check)
    {
        if (ctrls[child_a].get().du_top() - 1 == ctrls[child_b].get().du_top() ||
            ctrls[child_a].get().du_top() - 2 == ctrls[child_b].get().du_top())
            return true;
        else if (ctrls[child_a].get().du_top() == ctrls[child_b].get().du_top() - 1 ||
                 ctrls[child_a].get().du_top() == ctrls[child_b].get().du_top() - 2)
            return true;
        else
            return false;
    }

    if (ctrls[child_a].get().GetNode()->isGen(gen_wxStaticText))
    {
        if (ctrls[child_a].get().du_top() - 1 == ctrls[child_b].get().du_top() ||
            ctrls[child_a].get().du_top() - 2 == ctrls[child_b].get().du_top())
            return true;
    }
    return false;
}

bool resForm::is_within_vertical(const std::vector<resCtrl>& ctrls, size_t child_a, size_t child_b) const
{
    if (ctrls[child_a].du_top() >= ctrls[child_b].du_top() && ctrls[child_a].du_bottom() <= ctrls[child_b].du_bottom())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool resForm::is_within_vertical(const std::vector<std::reference_wrapper<resCtrl>>& ctrls, size_t child_a,
                                 size_t child_b) const
{
    if (ctrls[child_a].get().du_top() >= ctrls[child_b].get().du_top() &&
        ctrls[child_a].get().du_bottom() <= ctrls[child_b].get().du_bottom())
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
                      [](resCtrl a, resCtrl b)
                      {
                          return a.du_left() < b.du_left();
                      });
        }
    }
}

// This is almost identical to the function of the same name in resCtrl -- however that one needs to access m_node in order
// to handle a wxComboBox which has a different height then specified in the resource file.

bool resForm::ParseDimensions(tt_string_view line, wxRect& duRect, wxRect& pixelRect)
{
    duRect = { 0, 0, 0, 0 };
    pixelRect = { 0, 0, 0, 0 };
    line.moveto_nonspace();

    if (line.empty())
        return false;

    if (line.at(0) == ',')
        line.moveto_digit();

    if (line.empty() || !tt::is_digit(line.at(0)))
        return false;
    duRect.SetLeft(tt::atoi(line));

    auto pos = line.find_first_of(',');
    if (!tt::is_found(pos))
        return false;

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !tt::is_digit(line.at(0)))
        return false;
    duRect.SetTop(tt::atoi(line));

    pos = line.find_first_of(',');
    if (!tt::is_found(pos))
        return false;

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !tt::is_digit(line.at(0)))
        return false;
    duRect.SetWidth(tt::atoi(line));

    pos = line.find_first_of(',');
    if (!tt::is_found(pos))
        return false;

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !tt::is_digit(line.at(0)))
        return false;
    duRect.SetHeight(tt::atoi(line));

    /*

        On Windows 10, dialogs are supposed to use Segoe UI, 9pt font. However, a lot of dialogs are going to be using
        "MS Shell Dlg" or "MS Shell Dlg2" using an 8pt size. Those coordinates will end up being wrong when displayed by
        wxWidgets because wxWidgets follows the Windows 10 guidelines which normally uses a 9pt font.

        The following code converts dialog coordinates into pixels assuming a 9pt font.

        For the most part, these values are simply used to determine which sizer to place the control in. However, it will
        change things like the wrapping width of a wxStaticText -- our wxWidgets version will be larger than the original if
        the dialog used an 8pt font, smaller if it used a 10pt font.

    */

    pixelRect.SetLeft(static_cast<int>((static_cast<int64_t>(duRect.GetLeft()) * 7 / 4)));
    pixelRect.SetWidth(static_cast<int>((static_cast<int64_t>(duRect.GetWidth()) * 7 / 4)));
    pixelRect.SetTop(static_cast<int>((static_cast<int64_t>(duRect.GetTop()) * 15 / 4)));
    pixelRect.SetHeight(static_cast<int>((static_cast<int64_t>(duRect.GetHeight()) * 15 / 4)));

    return true;
}

size_t resForm::FindChildPosition(const Node* node)
{
    for (size_t idx_child = 0; idx_child < m_ctrls.size(); ++idx_child)
    {
        if (m_ctrls[idx_child].GetNode() == node)
            return idx_child;
    }

    return (to_size_t) -1;
}
