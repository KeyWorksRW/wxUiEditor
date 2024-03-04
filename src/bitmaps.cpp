/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains various images handling functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include <frozen/map.h>

#include <wx/bitmap.h>  // wxBitmap class interface
#include <wx/icon.h>    // wxIcon base header

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl
#include <wx/mstream.h>  // Memory stream classes

#include "wxui/ribbonpanel_base.h"  // Most of the embedded generator images will be declared here
#include "wxui/ui_images.h"         // This is generated from the Images List

#include "bitmaps.h"

using namespace wxue_img;

struct ImageMap
{
    const char* name;
    const unsigned char* data;
    size_t size_data;
};

const std::map<std::string_view, std::function<wxBitmapBundle(int width, int height)>> map_svg_functions = {
    { "auitoolbar", bundle_auitoolbar_svg },
    { "bitmap", bundle_bitmap_svg },
    { "bitmap_toggle_button", bundle_bitmap_toggle_button_svg },
    { "bmp_combo_box", bundle_bmp_combo_box_svg },
    { "book_page", bundle_book_page_svg },
    { "calendar", bundle_calendar_svg },
    { "check3state", bundle_check3state_svg },
    { "close_btn", bundle_close_btn_svg },
    { "colourPickerIcon", bundle_colourPickerIcon_svg },
    { "ctx_help_btn", bundle_ctx_help_btn_svg },
    { "data_list", bundle_data_list_svg },
    { "datepicker", bundle_datepicker_svg },
    { "dirPicker", bundle_dirPicker_svg },
    { "filePicker", bundle_filePicker_svg },
    { "flex_grid_sizer", bundle_flex_grid_sizer_svg },
    { "folder", bundle_folder_svg },
    { "fontPicker", bundle_fontPicker_svg },
    { "gauge", bundle_gauge_svg },
    { "grid", bundle_grid_svg },
    { "grid_bag_sizer", bundle_grid_bag_sizer_svg },
    { "grid_sizer", bundle_grid_sizer_svg },
    { "images", bundle_images_svg },
    { "notebook", bundle_notebook_svg },
    { "notebook_aui", bundle_notebook_aui_svg },
    { "notebook_choice", bundle_notebook_choice_svg },
    { "notebook_list", bundle_notebook_list_svg },
    { "pagectrl", bundle_pagectrl_svg },
    { "popup_window", bundle_popup_window_svg },
    { "project", bundle_project_svg },
    { "prop_sheet_dlg", bundle_prop_sheet_dlg_svg },
    { "radio_box", bundle_radio_box_svg },
    { "ribbon_bar", bundle_ribbon_bar_svg },
    { "richtextctrl", bundle_richtextctrl_svg },
    { "scintilla", bundle_scintilla_svg },
    { "search", bundle_search_svg },
    { "sizer", bundle_sizer_svg },
    { "sizer_horizontal", bundle_sizer_horizontal_svg },
    { "slider", bundle_slider_svg },
    { "spacer", bundle_spacer_svg },
    { "spin_ctrl", bundle_spin_ctrl_svg },
    { "spin_ctrl_double", bundle_spin_ctrl_double_svg },
    { "spinbtn", bundle_spinbtn_svg },
    { "static_line", bundle_static_line_svg },
    { "stddialogbuttonsizer", bundle_stddialogbuttonsizer_svg },
    { "svg", bundle_svg_svg },
    { "text_file", bundle_text_file_svg },
    { "text_sizer", bundle_text_sizer_svg },
    { "timepicker", bundle_timepicker_svg },
    { "toggle_button", bundle_toggle_button_svg },
    { "wrap_sizer", bundle_wrap_sizer_svg },
    { "wxButton", bundle_wxButton_svg },
    { "wxCheckBox", bundle_wxCheckBox_svg },
    { "wxChoice", bundle_wxChoice_svg },
    { "wxCollapsiblePane", bundle_wxCollapsiblePane_svg },
    { "wxComboBox", bundle_wxComboBox_svg },
    { "wxCommandLinkButton", bundle_wxCommandLinkButton_svg },
    { "wxDialog", bundle_wxDialog_svg },
    { "wxFrame", bundle_wxFrame_svg },
    { "wxPanel", bundle_wxPanel_svg },
    { "wxRadioButton", bundle_wxRadioButton_svg },
    { "wxScrolledWindow", bundle_wxScrolledWindow_svg },
    { "wxSimplebook", bundle_wxSimplebook_svg },
    { "wxSplitterWindow", bundle_wxSplitterWindow_svg },
    { "wxStaticBoxSizer", bundle_wxStaticBoxSizer_svg },
    { "wxStaticCheckBoxSizer", bundle_wxStaticCheckBoxSizer_svg },
    { "wxStaticRadioBtnSizer", bundle_wxStaticRadioBtnSizer_svg },
    { "wxStaticText", bundle_wxStaticText_svg },
    { "wxTextCtrl", bundle_wxTextCtrl_svg },
    { "wxTimer", bundle_wxTimer_svg },
    { "wxToolBar", bundle_wxToolBar_svg },
    { "wxToolbook", bundle_wxToolbook_svg },
    { "wxTreebook", bundle_wxTreebook_svg },
    { "wxWizard", bundle_wxWizard_svg },
    { "wxWizardPageSimple", bundle_wxWizardPageSimple_svg },
    { "propgrid_cat", bundle_propgrid_cat_svg },

    { "propgriditem", bundle_propgriditem_svg },
    { "propgridpage", bundle_propgridpage_svg },
    { "wxPropertyGrid", bundle_wxPropertyGrid_svg },
    { "wxPropertyGridManager", bundle_wxPropertyGrid_svg },
    { "xml_file", bundle_xml_file_svg },

    { "unknown", bundle_unknown_svg },
};

// Images that may be accessed more than once (typically for popup menus) should be at the front for fastest access. The rest
// are typically only loaded during application startup, and never accessed again.

static const ImageMap png_headers[] = {

    { .name = "unknown", .data = unknown_png, .size_data = sizeof(unknown_png) },
    { "default", default_png, sizeof(default_png) },

    { "CustomControl", customControl_png, sizeof(customControl_png) },
    { "dataview_ctrl", dataview_ctrl_png, sizeof(dataview_ctrl_png) },
    { "dataviewlist_column", dataviewlist_column_png, sizeof(dataviewlist_column_png) },
    { "dataviewlist_ctrl", dataviewlist_ctrl_png, sizeof(dataviewlist_ctrl_png) },
    { "dataviewtree_ctrl", dataviewtree_ctrl_png, sizeof(dataviewtree_ctrl_png) },
    { "default", default_png, sizeof(default_png) },
    { "doc_mdi_parent_frame", doc_mdi_parent_frame_png, sizeof(doc_mdi_parent_frame_png) },
    { "genericdir_ctrl", genericdir_ctrl_png, sizeof(genericdir_ctrl_png) },
    { "htmlwin", htmlwin_png, sizeof(htmlwin_png) },
    { "hyperlink_ctrl", hyperlink_ctrl_png, sizeof(hyperlink_ctrl_png) },
    { "infobar", infobar_png, sizeof(infobar_png) },
    { "menu", menu_png, sizeof(menu_png) },
    { "menuitem", menuitem_png, sizeof(menuitem_png) },
    { "ribbon_button", ribbon_button_png, sizeof(ribbon_button_png) },
    { "ribbon_buttonbar", ribbon_buttonbar_png, sizeof(ribbon_buttonbar_png) },
    { "ribbon_gallery", ribbon_gallery_png, sizeof(ribbon_gallery_png) },
    { "ribbon_gallery_item", ribbon_gallery_item_png, sizeof(ribbon_gallery_item_png) },
    { "ribbon_page", ribbon_page_png, sizeof(ribbon_page_png) },
    { "ribbon_panel", ribbon_panel_png, sizeof(ribbon_panel_png) },
    { "scrollbar", scrollbar_png, sizeof(scrollbar_png) },
    { "separator", separator_png, sizeof(separator_png) },
    { "statusbar", statusbar_png, sizeof(statusbar_png) },
    { "submenu", submenu_png, sizeof(submenu_png) },
    { "tool", tool_png, sizeof(tool_png) },
    { "tool_dropdown", tool_dropdown_png, sizeof(tool_dropdown_png) },
    { "toolseparator", toolseparator_png, sizeof(toolseparator_png) },
    { "toolspacer", toolspacer_png, sizeof(toolspacer_png) },
    { "toolStretchable", toolStretchable_png, sizeof(toolStretchable_png) },
    { "tree_ctrl", tree_ctrl_png, sizeof(tree_ctrl_png) },
    { "treelistctrl", treelistctrl_png, sizeof(treelistctrl_png) },
    { "treelistctrlcolumn", treelistctrlcolumn_png, sizeof(treelistctrlcolumn_png) },
    { "wxActivityIndicator", wxactivityIndicator_png, sizeof(wxactivityIndicator_png) },
    { "wxAnimation", wxanimation_png, sizeof(wxanimation_png) },
    { "wxBannerWindow", wxbannerWindow_png, sizeof(wxbannerWindow_png) },
    { "wxCheckListBox", wxCheckListBox_png, sizeof(wxCheckListBox_png) },
    { "wxFileCtrl", wxfileCtrl_png, sizeof(wxfileCtrl_png) },
    { "wxEditListBox", wxEditListBox_png, sizeof(wxEditListBox_png) },
    { "wxListBox", wxListBox_png, sizeof(wxListBox_png) },
    { "wxListView", wxListView_png, sizeof(wxListView_png) },
    { "wxMenuBar", wxMenuBar_png, sizeof(wxMenuBar_png) },
    { "wxRearrangeCtrl", wxRearrangeCtrl_png, sizeof(wxRearrangeCtrl_png) },
    { "wxSimpleHtmlListBox", wxSimpleHtmlListBox_png, sizeof(wxSimpleHtmlListBox_png) },
    { "webview", webview_png, sizeof(webview_png) },

};

wxImage GetInternalImage(tt_string_view name)
{
    for (auto& iter: png_headers)
    {
        if (name == iter.name)
        {
            return LoadHeaderImage(iter.data, iter.size_data);
        }
    }

    // If all else fails, return a default 16x16 image
    return LoadHeaderImage(default_png, sizeof(default_png));
}

wxBitmapBundle GetSvgImage(tt_string_view name, int width, int height)
{
    if (auto bndl_function = GetSvgFunction(name); bndl_function)
    {
        return bndl_function(width, height);
    }

    return bundle_unknown_svg(width, height);
}

wxBitmapBundle GetSvgImage(tt_string_view name, const wxSize& size)
{
    return GetSvgImage(name, size.GetWidth(), size.GetHeight());
}

wxIcon GetIconImage(tt_string_view name)
{
    for (auto& iter: png_headers)
    {
        if (name.is_sameas(iter.name))
        {
            auto image = LoadHeaderImage(iter.data, iter.size_data);
            image.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD);
            wxIcon icon;
            icon.CopyFromBitmap(image);
            return icon;
        }
    }

    auto image = GetInternalImage(name);
    wxIcon icon;
    icon.CopyFromBitmap(image);
    return icon;
}

// [KeyWorks - 05-04-2021] Note that we don't display warnings or errors to the user since this will be called during project
// loading, and there could be dozens of calls to the same problem file(s).

wxImage GetHeaderImage(tt_string_view filename, size_t* p_original_size, tt_string* p_mime_type)
{
    wxImage image;

    if (!filename.file_exists())
    {
        MSG_ERROR(tt_string() << filename << " passed to GetHeaderImage doesn't exist");
        return image;
    }

    std::ifstream fileOriginal(filename.as_str(), std::ios::binary | std::ios::in);
    if (!fileOriginal.is_open())
    {
        MSG_ERROR(tt_string() << filename << " passed to GetHeaderImage could not be read");
        return image;
    }
    std::string in_buf(std::istreambuf_iterator<char>(fileOriginal), {});
    if (in_buf.size() < 1)
    {
        MSG_ERROR(tt_string() << filename << " is empty!");
        return image;
    }

    // Turn this into a null-terminated string. We don't care what's at the end, because it won't be a number.
    in_buf[in_buf.size() - 1] = 0;

    size_t image_buffer_size = 0;
    size_t actual_size = 0;

    auto buf_ptr = strchr(in_buf.c_str(), '[');
    if (buf_ptr)
    {
        image_buffer_size = tt::atoi(++buf_ptr);
    }

    buf_ptr = strchr(buf_ptr, '{');
    if (!buf_ptr)
    {
        MSG_ERROR(tt_string() << filename << " doesn't contain an opening brace");
        return image;
    }

    bool isUiditorFile { true };

    if (!image_buffer_size)
    {
        // wxFormBuilder doesn't add the size of the image in brackets so we have to parse it without storing the
        // results in order to calculate the size, then parse it again to actually store the data in an image buffer.
        isUiditorFile = false;

        auto save_ptr = buf_ptr;

        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                ++image_buffer_size;

                // Step over any hex digit
                if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                    buf_ptr += 2;

                // Now step over the digit, then fall through and step over the first character following the digit
                do
                {
                    ++buf_ptr;
                } while (tt::is_digit(*buf_ptr));

                if (!*buf_ptr)
                {
                    FAIL_MSG(tt_string() << filename << " doesn't contain a closing brace");
                    wxMessageBox((tt_string() << filename << " doesn't contain a closing brace").make_wxString());
                    return image;
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);

        buf_ptr = save_ptr;
    }

    // We don't have a reliable way of identifying either wxFormBuilder or wxUiEditor generated graphics header files, so we
    // start by making certain the image buffer is at least a possible size. It might still be wrong, but we'll fail to find
    // a handler to load it, so that should be a sufficient way of trying to load some random  header file.

    if (image_buffer_size < 4 || image_buffer_size > in_buf.size() / 2)
    {
        MSG_ERROR(tt_string() << filename << " is not a valid graphics header file");
        return image;
    }

    auto image_buffer = std::make_unique<unsigned char[]>(image_buffer_size);
    auto out_buffer = image_buffer.get();

    if (isUiditorFile)
    {
        // This section is for wxUiEditor which uses decimal and no spaces
        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                unsigned char value = (to_uchar) (*buf_ptr - '0');
                for (++buf_ptr; *buf_ptr >= '0' && *buf_ptr <= '9'; ++buf_ptr)
                {
                    value = (value * 10) + (to_uchar) (*buf_ptr - '0');
                }
                out_buffer[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(tt_string() << filename << " actual image size is larger that the size specified in brackets");
                    return image;
                }
            }
            else
            {
                ++buf_ptr;
            }
        } while (*buf_ptr != '}' && *buf_ptr);
    }

    else
    {
        // This section is for wxFormBuilder which uses hexadecimal and a lot of whitespace
        do
        {
            // The format is assumed to be "0x" followed by two hexadcimal digits

            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                    buf_ptr += 2;

                unsigned char value = 0;

                // Get the high value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value = (to_uchar) (*buf_ptr - '0') * 16;
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value = (to_uchar) ((*buf_ptr - 'A') + 10) * 16;
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value = (to_uchar) ((*buf_ptr - 'a') + 10) * 16;

                ++buf_ptr;

                // Get the low value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value += (to_uchar) (*buf_ptr - '0');
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value += (to_uchar) ((*buf_ptr - 'A') + 10);
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value += (to_uchar) ((*buf_ptr - 'a') + 10);

                out_buffer[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(tt_string() << filename << " actual image size is larger that the size specified in brackets");
                    return image;
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);
    }
    wxMemoryInputStream stream(image_buffer.get(), actual_size);

    wxImageHandler* handler;
    auto& list = wxImage::GetHandlers();
    for (auto node = list.GetFirst(); node; node = node->GetNext())
    {
        handler = (wxImageHandler*) node->GetData();
        if (handler->CanRead(stream))
        {
            if (handler->LoadFile(&image, stream))
            {
                if (p_original_size)
                    *p_original_size = actual_size;
                if (p_mime_type)
                    *p_mime_type = handler->GetMimeType().utf8_string();

                return image;
            }
        }
    }

    return image;
}

// This is almost identical to GetImageFromArray() -- the only difference is that this one
// first tries to load the image via the PNG handler.
wxImage LoadHeaderImage(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream stream(data, size_data);
    wxImage image;

    // Images are almost always in PNG format, so check that first. If it fails, then let wxWidgets figure out the format.

    auto handler = wxImage::FindHandler(wxBITMAP_TYPE_PNG);
    if (handler && handler->CanRead(stream) && handler->LoadFile(&image, stream))
    {
        return image;
    }

    image.LoadFile(stream);
    return image;
};

bool GetAnimationImage(wxAnimation& animation, tt_string_view filename)
{
    if (!filename.file_exists())
    {
        MSG_ERROR(tt_string() << filename << " passed to GetAnimationanimation doesn't exist");
        return animation.IsOk();
    }

    std::ifstream fileOriginal(filename.as_str(), std::ios::binary | std::ios::in);
    if (!fileOriginal.is_open())
    {
        MSG_ERROR(tt_string() << filename << " passed to GetAnimationImage could not be read");
        return animation.IsOk();
    }
    std::string in_buf(std::istreambuf_iterator<char>(fileOriginal), {});
    if (in_buf.size() < 1)
    {
        MSG_ERROR(tt_string() << filename << " is empty!");
        return animation.IsOk();
    }

    // Turn this into a null-terminated string. We don't care what's at the end, because it won't be a number.
    in_buf[in_buf.size() - 1] = 0;

    size_t image_buffer_size = 0;
    size_t actual_size = 0;

    auto buf_ptr = strchr(in_buf.c_str(), '[');
    if (buf_ptr)
    {
        image_buffer_size = tt::atoi(++buf_ptr);
    }

    buf_ptr = strchr(buf_ptr, '{');
    if (!buf_ptr)
    {
        MSG_ERROR(tt_string() << filename << " doesn't contain an opening brace");
        return animation.IsOk();
    }

    bool isUiditorFile { true };

    if (!image_buffer_size)
    {
        // wxFormBuilder doesn't add the size of the animation in brackets so we have to parse it without storing the
        // results in order to calculate the size, then parse it again to actually store the data in an animation buffer.
        isUiditorFile = false;

        auto save_ptr = buf_ptr;

        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                ++image_buffer_size;

                // Step over any hex digit
                if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                    buf_ptr += 2;

                // Now step over the digit, then fall through and step over the first character following the digit
                do
                {
                    ++buf_ptr;
                } while (tt::is_digit(*buf_ptr));

                if (!*buf_ptr)
                {
                    FAIL_MSG(tt_string() << filename << " doesn't contain a closing brace");
                    wxMessageBox((tt_string() << filename << " doesn't contain a closing brace").make_wxString());
                    return animation.IsOk();
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);

        buf_ptr = save_ptr;
    }

    // We don't have a reliable way of identifying either wxFormBuilder or wxUiEditor generated graphics header files, so we
    // start by making certain the image buffer is at least a possible size. It might still be wrong, but we'll fail to find
    // a handler to load it, so that should be a sufficient way of trying to load some random  header file.

    if (image_buffer_size < 4 || image_buffer_size > in_buf.size() / 2)
    {
        MSG_ERROR(tt_string() << filename << " is not a valid graphics header file");
        return animation.IsOk();
    }

    auto image_buffer = std::make_unique<unsigned char[]>(image_buffer_size);
    auto out_buffer = image_buffer.get();

    if (isUiditorFile)
    {
        // This section is for wxUiEditor which uses decimal and no spaces
        do
        {
            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                unsigned char value = (to_uchar) (*buf_ptr - '0');
                for (++buf_ptr; *buf_ptr >= '0' && *buf_ptr <= '9'; ++buf_ptr)
                {
                    value = (value * 10) + (to_uchar) (*buf_ptr - '0');
                }
                out_buffer[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(tt_string() << filename << " actual image size is larger that the size specified in brackets");
                    return animation.IsOk();
                }
            }
            else
            {
                ++buf_ptr;
            }
        } while (*buf_ptr != '}' && *buf_ptr);
    }

    else
    {
        // This section is for wxFormBuilder which uses hexadecimal and a lot of whitespace
        do
        {
            // The format is assumed to be "0x" followed by two hexadcimal digits

            if (*buf_ptr >= '0' && *buf_ptr <= '9')
            {
                if (buf_ptr[0] == '0' && (buf_ptr[1] == 'x' || buf_ptr[1] == 'X'))
                    buf_ptr += 2;

                unsigned char value = 0;

                // Get the high value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value = (to_uchar) (*buf_ptr - '0') * 16;
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value = (to_uchar) ((*buf_ptr - 'A') + 10) * 16;
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value = (to_uchar) ((*buf_ptr - 'a') + 10) * 16;

                ++buf_ptr;

                // Get the low value
                if (*buf_ptr >= '0' && *buf_ptr <= '9')
                    value += (to_uchar) (*buf_ptr - '0');
                else if (*buf_ptr >= 'A' && *buf_ptr <= 'F')
                    value += (to_uchar) ((*buf_ptr - 'A') + 10);
                else if (*buf_ptr >= 'a' && *buf_ptr <= 'f')
                    value += (to_uchar) ((*buf_ptr - 'a') + 10);

                out_buffer[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(tt_string() << filename << " actual image size is larger that the size specified in brackets");
                    return animation.IsOk();
                }
            }
            ++buf_ptr;
        } while (*buf_ptr != '}' && *buf_ptr);
    }
    wxMemoryInputStream stream(image_buffer.get(), actual_size);
    animation.Load(stream);
    return animation.IsOk();
}

wxAnimation LoadAnimationImage(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream stream(data, size_data);
    wxAnimation animation;
    animation.Load(stream);
    return animation;
};
