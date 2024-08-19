/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource form  (usually a dialog)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"    // Enumerations for generators
#include "node.h"         // Node class
#include "winres_ctrl.h"  // resCtrl -- Process Windows Resource control data

class WinResource;

constexpr int32_t FudgeAmount = 3;

// This will either be a wxDialog or a MenuBar
class resForm
{
public:
    resForm();

    enum : size_t
    {
        form_dialog,
        form_panel,
        form_menu,
    };

    void ParseDialog(WinResource* pWinResource, tt_string_vector& txtfile, size_t& curTxtLine);
    void ParseMenu(WinResource* pWinResource, tt_string_vector& txtfile, size_t& curTxtLine);

    // Remove outer quotes, prefix a digit with id_ -- this is how the id gets stored in the
    // dialog.
    tt_string ConvertFormID(tt_string_view id);

    // Call this after
    void CreateDialogLayout();
    size_t GetFormType() const { return m_form_type; }
    Node* GetFormNode() const { return m_form_node.get(); }
    auto GetFormName() const { return m_form_node->as_string(prop_class_name); }
    int GetWidth() const { return m_pixel_rect.GetWidth(); }

    // left position in dialog units
    auto du_left() const { return m_du_rect.GetLeft(); }
    // top position in dialog units
    auto du_top() const { return m_du_rect.GetTop(); }
    // width in dialog units
    auto du_width() const { return m_du_rect.GetWidth(); }
    // height in dialog units
    auto du_height() const { return m_du_rect.GetHeight(); }

protected:
    // Called after all controls are processed, this will check for adjoining horizontal box
    // sizers, and if the have the identical type and number of controls, it will convert
    // them into a single flex gird sizer.
    void CheckForFlexGrid(Node* parent);

    // This will check all button controls in a form_dialog, and if they match a
    // wxStdDialogButtonSizer button, then that control will be created for the dialog and
    // the matching button added.
    void CheckForStdButtons();

    void AddSiblings(Node* parent_sizer, std::vector<std::reference_wrapper<resCtrl>>& actrls, resCtrl* pSibling = nullptr);

    // Adopts child node and sets child flag to indicate it has been added.
    //
    // Under Debug builds, this will catch logic errors where a child is added more than once
    void adoptChild(Node* node, resCtrl* child);

    // Adopts child node and sets child flag to indicate it has been added.
    //
    // Under Debug builds, this will catch logic errors where a child is added more than once
    void adoptChild(const NodeSharedPtr& node, resCtrl& child);

    // Fills in m_group_ctrls with every control within the boundaries of the group box
    // void CollectGroupControls(std::vector<resCtrl>& group_ctrls, size_t idx_parent);
    void CollectGroupControls(std::vector<std::reference_wrapper<resCtrl>>& group_ctrls, size_t idx_parent);

    // -1 if no horizontal alignment needed
    // 0 if box sizer needed
    // > 0 number of columns required for wxFlexGridSizer
    int GridSizerNeeded(size_t idx_start, size_t idx_end, const resCtrl* p_static_box);

    // Similar to GridSizerNeeded(), but only processes m_group_ctrls. Returns total number
    // of columns required.
    int GroupGridSizerNeeded(std::vector<std::reference_wrapper<resCtrl>>& group_ctrls, size_t idx_start) const;

    void AddStaticBoxChildren(const resCtrl& box, size_t idx_group_box);
    void AddStyle(tt_string_vector& txtfile, size_t& curTxtLine);
    void AppendStyle(GenEnum::PropName prop_name, tt_string_view style);
    void ParseControls(tt_string_vector& txtfile, size_t& curTxtLine);
    void ParseMenus(tt_string_vector& txtfile, size_t& curTxtLine);
    void ParseMenuItem(Node* parent, tt_string_vector& txtfile, size_t& curTxtLine);

    // Handle static text that needs to be centered
    void CheckForCenteredText(Node* node_parent);

    // This will create a 2-column flex grid and add controls as long as each row position
    // and size is identical
    size_t AddTwoColumnPairs(size_t idx_start);

    // This will create a 2-column flex grid and add controls as long as each row contains
    // a static text and edit control pair
    size_t AddTwoColumnStaticText(size_t idx_start);

    // Sorts all controls both vertically and horizontally.
    void SortCtrls();

    // Returns true if val1 is within range of val2 using a fudge value below and above val2.
    bool isInRange(int32_t val1, int32_t val2) const
    {
        return (val1 >= (val2 - FudgeAmount) && val1 <= (val2 + FudgeAmount));
    }

    // This will take into account a static text control to the left which is vertically centered
    // with the control on the right.
    //
    // If loose_check == true, any control can be -2 of the top of the other control.
    bool is_same_top(const resCtrl* left, const resCtrl* right, bool loose_check = false) const;

    // Returns true if child_a top/bottom is within child_b top/bottom. Only works with
    // master vector of controls (m_ctrls)
    bool is_within_vertical(const std::vector<resCtrl>& ctrls, size_t child_a, size_t child_b) const;

    // Returns true if child_a top/bottom is within child_b top/bottom. Only works with
    // master vector of controls (m_ctrls)
    bool is_within_vertical(const std::vector<std::reference_wrapper<resCtrl>>& ctrls, size_t child_a, size_t child_b) const;

    // This will take into account a static text control to the left which is vertically centered
    // with the control on the right.
    //
    // If loose_check == true, any control can be -2 of the top of the other control.
    bool is_same_top(const std::vector<resCtrl>& ctrls, size_t child_a, size_t child_b, bool loose_check = false) const;

    // This will take into account a static text control to the left which is vertically centered
    // with the control on the right.
    //
    // If loose_check == true, any control can be -2 of the top of the other control.
    bool is_same_top(const std::vector<std::reference_wrapper<resCtrl>>& ctrls, size_t child_a, size_t child_b,
                     bool loose_check = false) const;

    bool is_same_right(const std::vector<resCtrl>& ctrls, size_t child_a, size_t child_b) const;

    bool is_same_left(const std::vector<resCtrl>& ctrls, size_t child_a, size_t child_b) const
    {
        return ctrls[child_a].du_left() == ctrls[child_b].du_left();
    }

    void CreateStdButton();

    bool ParseDimensions(tt_string_view line, wxRect& duRect, wxRect& pixelRect);

    // This will search through m_ctrls and find the index of the control matching the node
    // parameter. Returns -1 if not found.
    size_t FindChildPosition(const Node* node);

    // Returns the index of the next control that has not already been added.
    //
    // Caution! This can increment the index beyond m_ctrls.size()
    void NextChild(size_t& idx_child)
    {
        for (++idx_child; idx_child < m_ctrls.size(); ++idx_child)
        {
            if (!m_ctrls[idx_child].isAdded())
                return;
        }
    }

private:
    // These are in dialog coordinates
    wxRect m_du_rect { 0, 0, 0, 0 };

    // These are in pixels
    wxRect m_pixel_rect { 0, 0, 0, 0 };

    NodeSharedPtr m_form_node;
    NodeSharedPtr m_dlg_sizer;
    NodeSharedPtr m_stdButtonSizer;

    size_t m_form_type;

    std::vector<resCtrl> m_ctrls;

    WinResource* m_pWinResource;

    bool m_is_popup_menu { false };

    // Makes it easier to know exactly which form we're looking at in the debugger
    tt_string m_form_id;
};
