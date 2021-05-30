/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource form  (usually a dialog)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"    // Enumerations for generators
#include "node.h"         // Node class
#include "winres_ctrl.h"  // rcCtrl -- Process Windows Resource control data

class ttlib::textfile;

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
    size_t GetFormType() const { return m_form_type; }
    Node* GetFormNode() { return m_node.get(); }
    auto GetFormName() { return m_node->prop_as_string(prop_class_name); }

protected:
    void AppendStyle(GenEnum::PropName prop_name, ttlib::cview style);
    void AddStyle(ttlib::textfile& txtfile, size_t& curTxtLine);
    void ParseControls(ttlib::textfile& txtfile, size_t& curTxtLine);
    void GetDimensions(ttlib::cview line);

private:
    RC_RECT m_rc { 0, 0, 0, 0 };
    NodeSharedPtr m_node;
    std::vector<rcCtrl> m_ctrls;
    size_t m_form_type;
};
