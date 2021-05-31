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
#include "winres_ctrl.h"  // rcCtrl -- Process Windows Resource control data

constexpr int32_t FudgeAmount = 3;

// This will either be a wxDialog or a MenuBar
class rcForm
{
public:
    rcForm();

    enum : size_t
    {
        form_dialog,
        form_panel,
        form_menu,
    };

    void ParseDialog(ttlib::textfile& txtfile, size_t& curTxtLine);

    // Call this after
    void AddSizersAndChildren();
    size_t GetFormType() const { return m_form_type; }
    Node* GetFormNode() { return m_node.get(); }
    auto GetFormName() { return m_node->prop_as_string(prop_class_name); }

protected:
    void AddStaticBoxChildren(size_t& idx_child);
    void AddStyle(ttlib::textfile& txtfile, size_t& curTxtLine);
    void AppendStyle(GenEnum::PropName prop_name, ttlib::cview style);
    void GetDimensions(ttlib::cview line);
    void ParseControls(ttlib::textfile& txtfile, size_t& curTxtLine);

    // Returns true if val1 is within range of val2 using a fudge value below and above val2.
    bool isInRange(int32_t val1, int32_t val2) { return (val1 >= (val2 - FudgeAmount) && val1 <= (val2 + FudgeAmount)); }

private:
    RC_RECT m_rc { 0, 0, 0, 0 };
    NodeSharedPtr m_node;
    NodeSharedPtr m_gridbag;
    std::vector<rcCtrl> m_ctrls;
    size_t m_form_type;

#if defined(_DEBUG)
    // Makes it easier to know exactly which form we're looking at in the debugger
    ttlib::cstr m_form_id;
#endif // _DEBUG
};
