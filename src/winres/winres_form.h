/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource form  (usually a dialog)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

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

    void ParseDialog(WinResource* pWinResource, ttlib::textfile& txtfile, size_t& curTxtLine);

    // Remove outer quotes, prefix a digit with id_ -- this is how the id gets stored in the
    // dialog.
    ttlib::cstr ConvertDialogId(ttlib::cview id);

    // Call this after
    void AddSizersAndChildren();
    size_t GetFormType() const { return m_form_type; }
    Node* GetFormNode() const { return m_node.get(); }
    auto GetFormName() const { return m_node->prop_as_string(prop_class_name); }
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
    // Returns true if button was processed, otherwise treat it like a normal button.
    bool ProcessStdButton(Node* parent_sizer, size_t idx_child);
    // Adopts child node and sets child flag to indicate it has been added
    void Adopt(const NodeSharedPtr& node, resCtrl& child);

    // Fills in m_group_ctrls with every control within the boundaries of the group box
    void CollectGroupControls(size_t idx_parent);

    // -1 if no horizontal alignment needed
    // 0 if box sizer needed
    // > 0 number of columns required for wxFlexGridSizer
    int GridSizerNeeded(size_t idx_start, size_t idx_end, const resCtrl* p_static_box);

    // Similar to GridSizerNeeded(), but only processes m_group_ctrls
    int GroupGridSizerNeeded(size_t idx_start) const;

    void AddStaticBoxChildren(const resCtrl& box, size_t idx_group_box);
    void AddStyle(ttlib::textfile& txtfile, size_t& curTxtLine);
    void AppendStyle(GenEnum::PropName prop_name, ttlib::cview style);
    void ParseControls(ttlib::textfile& txtfile, size_t& curTxtLine);

    // Returns true if val1 is within range of val2 using a fudge value below and above val2.
    bool isInRange(int32_t val1, int32_t val2) { return (val1 >= (val2 - FudgeAmount) && val1 <= (val2 + FudgeAmount)); }

private:
    // These are in dialog coordinates
    wxRect m_du_rect { 0, 0, 0, 0 };

    // These are in pixels
    wxRect m_pixel_rect { 0, 0, 0, 0 };

    NodeSharedPtr m_node;

    size_t m_form_type;

    std::vector<resCtrl> m_ctrls;

    // This will contain pointers to every control within a wxStaticBoxSizer. It gets reset
    // every time a new wxStaticBoxSizer needs to be processed (see CollectGroupControls()).
    std::vector<resCtrl*> m_group_ctrls;

    WinResource* m_pWinResource;

#if defined(_DEBUG)
    // Makes it easier to know exactly which form we're looking at in the debugger
    ttlib::cstr m_form_id;
#endif  // _DEBUG
};
