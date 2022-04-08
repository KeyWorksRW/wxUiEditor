////////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////////

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/image.h>

#include "ribbon_ids.h"
#include "gen_enums.h"     // Enumerations for generators

using namespace GenEnum;

#include "menucombobox_base.h"

#include <wx/mstream.h>  // memory stream classes

// Convert a data array into a wxImage
inline wxImage wxueImage(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream strm(data, size_data);
    wxImage image;
    image.LoadFile(strm);
    return image;
};

namespace wxue_img
{
    extern const unsigned char bmp_combo_box_png[492];
    extern const unsigned char wxChoice_png[330];
    extern const unsigned char wxComboBox_png[233];
}

MenuCombobox::MenuCombobox() : wxMenu()
{
    if (!wxImage::FindHandler(wxBITMAP_TYPE_PNG))
        wxImage::AddHandler(new wxPNGHandler);

    auto menu_item = Append(gen_wxComboBox, "Insert wxComboBox");
    menu_item->SetBitmap(wxBitmapBundle::FromBitmap(wxueImage(wxue_img::wxComboBox_png, sizeof(wxue_img::wxComboBox_png))));

    auto menu_item_2 = Append(gen_wxChoice, "Insert wxChoice");
    menu_item_2->SetBitmap(wxBitmapBundle::FromBitmap(wxueImage(wxue_img::wxChoice_png, sizeof(wxue_img::wxChoice_png))));

    auto menu_item_3 = Append(gen_wxBitmapComboBox, "Insert wxBitmapComboBox");
    menu_item_3->SetBitmap(wxBitmapBundle::FromBitmap(wxueImage(wxue_img::bmp_combo_box_png, sizeof(wxue_img::bmp_combo_box_png))));
}
