///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include "ui_images.h"

#include "ribbon_ids.h"
#include "gen_enums.h"     // Enumerations for generators

using namespace GenEnum;

#include "menucheckbox.h"

MenuCheckbox::MenuCheckbox() : wxMenu()
{

    auto* menu_item = Append(gen_wxCheckBox, "Insert wxCheckBox", "Insert a normal two-state checkbox", wxITEM_NORMAL);
    menu_item->SetBitmap(wxue_img::bundle_wxCheckBox_svg(24, 24));
    auto* menu_item_2 = Append(gen_Check3State, "Insert 3-state wxCheckBox", "Insert a 3-state checkbox", wxITEM_NORMAL);
    menu_item_2->SetBitmap(wxue_img::bundle_check3state_svg(24, 24));
}

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************
