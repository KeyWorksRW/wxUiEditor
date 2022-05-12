/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains various images handling functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include <wx/bitmap.h>  // wxBitmap class interface
#include <wx/icon.h>    // wxIcon base header

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl
#include <wx/mstream.h>  // Memory stream classes

#include "wxui/ribbonpanel_base.h"  // Most of the embedded generator images will be declared here
#include "wxui/ui_images.h"         // This is generated from the Images form

#include "bitmaps.h"

using namespace wxue_img;

struct ImageMap
{
    const char* name;
    const unsigned char* data;
    size_t size_data;
};

// Images that may be accessed more than once (typically for popup menus) should be at the front for fastest access. The rest
// are typically only loaded during application startup, and never accessed again.

static const ImageMap png_headers[] = {

    { "unknown", unknown_png, sizeof(unknown_png) },
    { "default", default_png, sizeof(default_png) },

    { "flex_grid_sizer", flex_grid_sizer_png, sizeof(flex_grid_sizer_png) },
    { "grid_bag_sizer", grid_bag_sizer_png, sizeof(grid_bag_sizer_png) },
    { "grid_sizer", grid_sizer_png, sizeof(grid_sizer_png) },
    { "sizer", sizer_png, sizeof(sizer_png) },
    { "sizer_horizontal", sizer_horizontal_png, sizeof(sizer_horizontal_png) },
    { "slider", slider_png, sizeof(slider_png) },
    { "spacer", spacer_png, sizeof(spacer_png) },
    { "stddialogbuttonsizer", stddialogbuttonsizer_png, sizeof(stddialogbuttonsizer_png) },
    { "text_sizer", text_sizer_png, sizeof(text_sizer_png) },
    { "wrap_sizer", wrap_sizer_png, sizeof(wrap_sizer_png) },
    { "wxStaticBoxSizer", wxStaticBoxSizer_png, sizeof(wxStaticBoxSizer_png) },
    { "wxStaticCheckBoxSizer", wxStaticCheckBoxSizer_png, sizeof(wxStaticCheckBoxSizer_png) },
    { "wxStaticRadioBtnSizer", wxStaticRadioBtnSizer_png, sizeof(wxStaticRadioBtnSizer_png) },

    { "auinotebook", auinotebook_png, sizeof(auinotebook_png) },
    { "auitoolbar", auitoolbar_png, sizeof(auitoolbar_png) },
    { "bmp_combo_box", bmp_combo_box_png, sizeof(bmp_combo_box_png) },
    { "book_page", book_page_png, sizeof(book_page_png) },
    { "calendar", calendar_png, sizeof(calendar_png) },
    { "check3state", check3state_png, sizeof(check3state_png) },
    { "colourPickerIcon", colourPickerIcon_png, sizeof(colourPickerIcon_png) },
    { "CustomControl", customControl_png, sizeof(customControl_png) },
    { "dataview_ctrl", dataview_ctrl_png, sizeof(dataview_ctrl_png) },
    { "dataviewlist_column", dataviewlist_column_png, sizeof(dataviewlist_column_png) },
    { "dataviewlist_ctrl", dataviewlist_ctrl_png, sizeof(dataviewlist_ctrl_png) },
    { "dataviewtree_ctrl", dataviewtree_ctrl_png, sizeof(dataviewtree_ctrl_png) },
    { "datepicker", datepicker_png, sizeof(datepicker_png) },
    { "default", default_png, sizeof(default_png) },
    { "dirPicker", dirPicker_png, sizeof(dirPicker_png) },
    { "doc_mdi_parent_frame", doc_mdi_parent_frame_png, sizeof(doc_mdi_parent_frame_png) },
    { "filePicker", filePicker_png, sizeof(filePicker_png) },
    { "fontPicker", fontPicker_png, sizeof(fontPicker_png) },
    { "gauge", gauge_png, sizeof(gauge_png) },
    { "genericdir_ctrl", genericdir_ctrl_png, sizeof(genericdir_ctrl_png) },
    { "grid", grid_png, sizeof(grid_png) },
    { "htmlwin", htmlwin_png, sizeof(htmlwin_png) },
    { "hyperlink_ctrl", hyperlink_ctrl_png, sizeof(hyperlink_ctrl_png) },
    { "infobar", infobar_png, sizeof(infobar_png) },
    { "menu", menu_png, sizeof(menu_png) },
    { "menuitem", menuitem_png, sizeof(menuitem_png) },
    { "pagectrl", pagectrl_png, sizeof(pagectrl_png) },
    { "project", project_png, sizeof(project_png) },
    { "propgriditem", propgriditem_png, sizeof(propgriditem_png) },
    { "propgridpage", propgridpage_png, sizeof(propgridpage_png) },
    { "radio_box", radio_box_png, sizeof(radio_box_png) },
    { "ribbon_bar", ribbon_bar_png, sizeof(ribbon_bar_png) },
    { "ribbon_button", ribbon_button_png, sizeof(ribbon_button_png) },
    { "ribbon_buttonbar", ribbon_buttonbar_png, sizeof(ribbon_buttonbar_png) },
    { "ribbon_gallery", ribbon_gallery_png, sizeof(ribbon_gallery_png) },
    { "ribbon_gallery_item", ribbon_gallery_item_png, sizeof(ribbon_gallery_item_png) },
    { "ribbon_page", ribbon_page_png, sizeof(ribbon_page_png) },
    { "ribbon_panel", ribbon_panel_png, sizeof(ribbon_panel_png) },
    { "richtextctrl", richtextctrl_png, sizeof(richtextctrl_png) },
    { "scintilla", scintilla_png, sizeof(scintilla_png) },
    { "scrollbar", scrollbar_png, sizeof(scrollbar_png) },
    { "search", search_png, sizeof(search_png) },
    { "separator", separator_png, sizeof(separator_png) },
    { "spin_ctrl", spin_ctrl_png, sizeof(spin_ctrl_png) },
    { "spin_ctrl_double", spin_ctrl_double_png, sizeof(spin_ctrl_double_png) },
    { "spinbtn", spinbtn_png, sizeof(spinbtn_png) },
    { "static_bitmap", static_bitmap_png, sizeof(static_bitmap_png) },
    { "static_line", static_line_png, sizeof(static_line_png) },
    { "statusbar", statusbar_png, sizeof(statusbar_png) },
    { "submenu", submenu_png, sizeof(submenu_png) },
    { "timepicker", timepicker_png, sizeof(timepicker_png) },
    { "toggle_button", toggle_button_png, sizeof(toggle_button_png) },
    { "tool", tool_png, sizeof(tool_png) },
    { "toolseparator", toolseparator_png, sizeof(toolseparator_png) },
    { "tree_ctrl", tree_ctrl_png, sizeof(tree_ctrl_png) },
    { "treelistctrl", treelistctrl_png, sizeof(treelistctrl_png) },
    { "treelistctrlcolumn", treelistctrlcolumn_png, sizeof(treelistctrlcolumn_png) },
    { "wxActivityIndicator", wxactivityIndicator_png, sizeof(wxactivityIndicator_png) },
    { "wxAnimation", wxanimation_png, sizeof(wxanimation_png) },
    { "wxBannerWindow", wxbannerWindow_png, sizeof(wxbannerWindow_png) },
    { "wxButton", wxButton_png, sizeof(wxButton_png) },
    { "wxCheckBox", wxcheckBox_png, sizeof(wxcheckBox_png) },
    { "wxCheckListBox", wxCheckListBox_png, sizeof(wxCheckListBox_png) },
    { "wxChoice", wxChoice_png, sizeof(wxChoice_png) },
    { "wxChoicebook", wxChoicebook_png, sizeof(wxChoicebook_png) },
    { "wxCollapsiblePane", wxCollapsiblePane_png, sizeof(wxCollapsiblePane_png) },
    { "wxComboBox", wxcomboBox_png, sizeof(wxcomboBox_png) },
    { "wxCommandLinkButton", wxCommandLinkButton_png, sizeof(wxCommandLinkButton_png) },
    { "wxDialog", wxDialog_png, sizeof(wxDialog_png) },
    { "wxFileCtrl", wxfileCtrl_png, sizeof(wxfileCtrl_png) },
    { "wxFrame", wxFrame_png, sizeof(wxFrame_png) },
    { "wxEditListBox", wxEditListBox_png, sizeof(wxEditListBox_png) },
    { "wxListBox", wxListBox_png, sizeof(wxListBox_png) },
    { "wxListView", wxListView_png, sizeof(wxListView_png) },
    { "wxListbook", wxListbook_png, sizeof(wxListbook_png) },
    { "wxMenuBar", wxMenuBar_png, sizeof(wxMenuBar_png) },
    { "wxNotebook", wxnotebook_png, sizeof(wxnotebook_png) },
    { "wxPanel", wxPanel_png, sizeof(wxPanel_png) },
    { "wxPopupTransientWindow", WXPOPupTransientWindow_png, sizeof(WXPOPupTransientWindow_png) },
    { "wxPropertyGrid", wxPropertyGrid_png, sizeof(wxPropertyGrid_png) },
    { "wxPropertyGridManager", wxPropertyGridManager_png, sizeof(wxPropertyGridManager_png) },
    { "wxRadioButton", wxradioButton_png, sizeof(wxradioButton_png) },
    { "wxRearrangeCtrl", wxRearrangeCtrl_png, sizeof(wxRearrangeCtrl_png) },
    { "wxScrolledWindow", wxscrolledWindow_png, sizeof(wxscrolledWindow_png) },
    { "wxSimpleHtmlListBox", wxSimpleHtmlListBox_png, sizeof(wxSimpleHtmlListBox_png) },
    { "wxSimplebook", wxsimplebook_png, sizeof(wxsimplebook_png) },
    { "wxSplitterWindow", wxsplitterWindow_png, sizeof(wxsplitterWindow_png) },
    { "wxStaticText", wxStaticText_png, sizeof(wxStaticText_png) },
    { "wxTextCtrl", wxtextCtrl_png, sizeof(wxtextCtrl_png) },
    { "wxToolBar", wxToolBar_png, sizeof(wxToolBar_png) },
    { "wxToolbook", wxToolbook_png, sizeof(wxToolbook_png) },
    { "wxTreebook", wxTreebook_png, sizeof(wxTreebook_png) },
    { "wxWizard", wxWizard_png, sizeof(wxWizard_png) },
    { "wxWizardPageSimple", wxWizardPageSimple_png, sizeof(wxWizardPageSimple_png) },
    { "webview", webview_png, sizeof(webview_png) },

// A different Icon is used for debug builds so that it is easier to identify that a debug build is being run.

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    { "logo16", debug_16_png, sizeof(debug_16_png) },
    { "logo32", debug_32_png, sizeof(debug_32_png) }
#else
    { "logo16", logo16_png, sizeof(logo16_png) },
    { "logo32", logo32_png, sizeof(logo32_png) }
#endif

};

wxImage GetInternalImage(ttlib::cview name)
{
    for (auto& iter: png_headers)
    {
        if (name.is_sameas(iter.name))
        {
            return LoadHeaderImage(iter.data, iter.size_data);
        }
    }

    // If all else fails, return a default 16x16 image
    return LoadHeaderImage(default_png, sizeof(default_png));
}

wxIcon GetIconImage(ttlib::cview name)
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

wxImage GetHeaderImage(ttlib::cview filename, size_t* p_original_size, ttString* p_mime_type)
{
    wxImage image;

    if (!ttlib::file_exists(filename))
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetHeaderImage doesn't exist");
        return image;
    }

    std::ifstream fileOriginal(filename, std::ios::binary | std::ios::in);
    if (!fileOriginal.is_open())
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetHeaderImage could not be read");
        return image;
    }
    std::string in_buf(std::istreambuf_iterator<char>(fileOriginal), {});
    if (in_buf.size() < 1)
    {
        MSG_ERROR(ttlib::cstr() << filename << " is empty!");
        return image;
    }

    // Turn this into a null-terminated string. We don't care what's at the end, because it won't be a number.
    in_buf[in_buf.size() - 1] = 0;

    size_t image_buffer_size = 0;
    size_t actual_size = 0;

    auto buf_ptr = strchr(in_buf.c_str(), '[');
    if (buf_ptr)
    {
        image_buffer_size = ttlib::atoi(++buf_ptr);
    }

    buf_ptr = strchr(buf_ptr, '{');
    if (!buf_ptr)
    {
        MSG_ERROR(ttlib::cstr() << filename << " doesn't contain an opening brace");
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
                } while (ttlib::is_digit(*buf_ptr));

                if (!*buf_ptr)
                {
                    FAIL_MSG(ttlib::cstr() << filename << " doesn't contain a closing brace");
                    wxMessageBox((ttlib::cstr() << filename << " doesn't contain a closing brace").wx_str());
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
        MSG_ERROR(ttlib::cstr() << filename << " is not a valid graphics header file");
        return image;
    }

    auto image_buffer = std::make_unique<unsigned char[]>(image_buffer_size);
    unsigned char* ptr_out_buf = image_buffer.get();

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
                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
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

                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
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
                    *p_mime_type = handler->GetMimeType();

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

bool GetAnimationImage(wxAnimation& animation, ttlib::cview filename)
{
    if (!ttlib::file_exists(filename))
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetAnimationanimation doesn't exist");
        return animation.IsOk();
    }

    std::ifstream fileOriginal(filename, std::ios::binary | std::ios::in);
    if (!fileOriginal.is_open())
    {
        MSG_ERROR(ttlib::cstr() << filename << " passed to GetAnimationImage could not be read");
        return animation.IsOk();
    }
    std::string in_buf(std::istreambuf_iterator<char>(fileOriginal), {});
    if (in_buf.size() < 1)
    {
        MSG_ERROR(ttlib::cstr() << filename << " is empty!");
        return animation.IsOk();
    }

    // Turn this into a null-terminated string. We don't care what's at the end, because it won't be a number.
    in_buf[in_buf.size() - 1] = 0;

    size_t image_buffer_size = 0;
    size_t actual_size = 0;

    auto buf_ptr = strchr(in_buf.c_str(), '[');
    if (buf_ptr)
    {
        image_buffer_size = ttlib::atoi(++buf_ptr);
    }

    buf_ptr = strchr(buf_ptr, '{');
    if (!buf_ptr)
    {
        MSG_ERROR(ttlib::cstr() << filename << " doesn't contain an opening brace");
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
                } while (ttlib::is_digit(*buf_ptr));

                if (!*buf_ptr)
                {
                    FAIL_MSG(ttlib::cstr() << filename << " doesn't contain a closing brace");
                    wxMessageBox((ttlib::cstr() << filename << " doesn't contain a closing brace").wx_str());
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
        MSG_ERROR(ttlib::cstr() << filename << " is not a valid graphics header file");
        return animation.IsOk();
    }

    auto image_buffer = std::make_unique<unsigned char[]>(image_buffer_size);
    unsigned char* ptr_out_buf = image_buffer.get();

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
                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
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

                ptr_out_buf[actual_size] = value;

                if (++actual_size > image_buffer_size)
                {
                    MSG_ERROR(ttlib::cstr()
                              << filename << " actual image size is larger that the size specified in brackets");
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
