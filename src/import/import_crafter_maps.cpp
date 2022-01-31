/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCrafter mappings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_enums.h"

using namespace GenEnum;

std::map<int, GenEnum::GenName> g_map_id_generator = {

    { 4400, gen_wxButton },         // verified
    { 4401, gen_wxBoxSizer },       // verified
    { 4402, gen_wxFrame },          // verified
    { 4403, gen_wxFlexGridSizer },  // verified
    { 4404, gen_wxButton },         // wxBitmapButton, converted to wxButton
    { 4405, gen_wxStaticText },     // verified
    { 4406, gen_wxTextCtrl },       // verified
    { 4407, gen_PanelForm },        // verified, but we don't support font, foreground colour, or tooltip
    { 4408, gen_wxPanel },          // verified, but we don't support font, foreground colour, or tooltip
    { 4409, gen_wxStaticBitmap },   // verified
    { 4410, gen_wxComboBox },       // verified
    { 4411, gen_wxChoice },         // verified
    { 4412, gen_wxListBox },        // verified
    { 4413, gen_wxListView },       // verified
    { 4415, gen_wxCheckBox },       // verified
    { 4416, gen_wxRadioBox },       // verified
    { 4417, gen_wxRadioButton },    // verified
    { 4418, gen_wxStaticLine },     // verified
    { 4419, gen_wxSlider },         // verified
    { 4420, gen_wxGauge },          // verified
    { 4421, gen_wxDialog },         // verified
    { 4422, gen_wxTreeCtrl },       // verified (wxCrafter doesn't support any styles)
    { 4423, gen_wxHtmlWindow },     // verified (wxUiEditor doesn't support URL)
    { 4424, gen_wxRichTextCtrl },   // verified
    { 4425, gen_wxCheckListBox },   // verified
    { 4426, gen_wxGrid },
    { 4427, gen_wxToggleButton },      // verified
    { 4428, gen_wxSearchCtrl },        // verified
    { 4429, gen_wxColourPickerCtrl },  // verified
    { 4430, gen_wxFontPickerCtrl },    // verified
    { 4431, gen_wxFilePickerCtrl },    // verified
    { 4432, gen_wxDirPickerCtrl },     // verified
    { 4433, gen_wxDatePickerCtrl },    // verified
    { 4434, gen_wxCalendarCtrl },      // verified
    { 4435, gen_wxScrollBar },         // verified
    { 4436, gen_wxSpinCtrl },          // verified
    { 4437, gen_wxSpinButton },        // verified
    { 4438, gen_wxHyperlinkCtrl },     // verified
    { 4439, gen_wxGenericDirCtrl },    // verified
    { 4440, gen_wxScrolledWindow },    // verified
    { 4441, gen_BookPage },
    { 4442, gen_wxNotebook },          // verified
    { 4443, gen_wxToolbook },          // verified
    { 4444, gen_wxListbook },          // verified
    { 4445, gen_wxChoicebook },        // verified
    { 4446, gen_wxTreebook },          // verified
    { 4447, gen_wxSplitterWindow },    // verified
    { 4448, gen_wxPanel },             // verified -- used as a child of wxSplitterWindow
    { 4449, gen_wxStaticBoxSizer },    // verified
    { 4450, gen_wxWizard },            // verified
    { 4451, gen_wxWizardPageSimple },  // verified
    { 4452, gen_wxGridSizer },         // verified
    { 4453, gen_wxGridBagSizer },      // verified
    { 4454, gen_spacer },              // verified
    { 4456, gen_wxAuiNotebook },       // verified
    { 4457, gen_wxMenuBar },           // verified
    { 4458, gen_wxMenu },              // verified
    { 4459, gen_wxMenuItem },          // verified
    { 4460, gen_submenu },             // verified
    { 4461, gen_wxToolBar },           // verified
    { 4462, gen_tool },                // verified
    { 4463, gen_wxAuiToolBar },
    { 4464, gen_wxStatusBar },             // verified
    { 4465, gen_CustomControl },           // verified
    { 4466, gen_wxStyledTextCtrl },        // verified
    { 4467, gen_wxStdDialogButtonSizer },  // verified
    { 4469, gen_wxDataViewListCtrl },
    { 4470, gen_wxDataViewTreeCtrl },
    { 4471, gen_wxBannerWindow },          // verified
    { 4474, gen_wxCommandLinkButton },     // verified
    { 4475, gen_wxCollapsiblePane },       // verified
    { 4477, gen_wxInfoBar },               // verified
    { 4479, gen_wxWebView },               // verified
    { 4484, gen_wxPopupTransientWindow },  // no way for wxCrafter to create this
    { 4485, gen_wxPropertyGridManager },   // verified
    { 4486, gen_propGridItem },            // verified
    { 4488, gen_wxRibbonBar },             // verified
    { 4489, gen_wxRibbonPage },            // verified
    { 4490, gen_wxRibbonPanel },           // verified
    { 4491, gen_wxRibbonButtonBar },       // verified
    { 4492, gen_ribbonButton },            // normal button
    { 4493, gen_ribbonButton },            // hybrid button
    { 4494, gen_ribbonButton },            // dropdown button
    { 4495, gen_ribbonButton },            // toggle button
    { 4496, gen_wxRibbonGallery },         // verified
    { 4497, gen_ribbonGalleryItem },       // verified
    { 4498, gen_wxRibbonToolBar },         // verified
    { 4499, gen_ribbonTool },              // normal tool
    { 4500, gen_ribbonTool },              // hybrid tool
    { 4501, gen_ribbonTool },              // dropdown tool
    { 4502, gen_ribbonTool },              // toggle tool
    { 4503, gen_ribbonSeparator },         // verified
    { 4504, gen_toolSeparator },
    { 4509, gen_wxTreeListCtrl },      // verified
    { 4510, gen_TreeListCtrlColumn },  // verified
    { 4511, gen_wxSimplebook },        // verified
    { 4513, gen_wxToggleButton },      // verified
    { 4519, gen_wxAnimationCtrl },     // needs support for load and play
    { 4520, gen_wxBitmapComboBox },
    { 4521, gen_wxRearrangeCtrl },      // verified
    { 4522, gen_wxSimpleHtmlListBox },  // verified
    { 4523, gen_wxActivityIndicator },  // verified
    { 4524, gen_wxTimePickerCtrl },     // verified

    { 4414, gen_unknown },  // column header for wxListView, supported directly as a wxListView property
    { 4468, gen_unknown },  // Theses are the buttons to enable in gen_wxStdDialogButtonSizer
    { 4476, gen_unknown },  // "pane" for gen_wxCollapsiblePane -- supported directly

    { 4455, gen_unknown /* WXTREEBOOK_SUB_PAGE */ },
    { 4472, gen_unknown /* WXDATAVIEWCOL */ },
    { 4473, gen_unknown /* WXDATAVIEWTREELISTCTRL */ },
    { 4478, gen_unknown /* WXINFOBARBUTTON */ },
    { 4480, gen_unknown /* WXAUIMANAGER */ },
    { 4481, gen_unknown /* WXIMAGELIST */ },
    { 4482, gen_unknown /* WXBITMAP */ },
    { 4483, gen_unknown /* WXTIMER */ },  // doesn't seem to be used, even when imported from wxFB
    { 4487, gen_unknown /* WXPGPROPERTY_SUB */ },
    { 4505, gen_unknown /* WXGLCANVAS */ },
    { 4506, gen_unknown /* WXGRIDCOL */ },
    { 4507, gen_unknown /* WXGRIDROW */ },
    { 4508, gen_unknown /* WXMEDIACTRL */ },
    { 4512, gen_unknown /* WXTASKBARICON */ },
    { 4514, gen_unknown /* WXAUITOOLBARLABEL */ },
    { 4515, gen_unknown /* WXAUITOOLBARITEM_SPACE */ },
    { 4516, gen_unknown /* WXAUITOOLBARITEM_STRETCHSPACE */ },
    { 4517, gen_unknown /* WXTOOLBARITEM_STRETCHSPACE */ },
    { 4518, gen_unknown /* WXAUITOOLBARTOPLEVEL */ },

};

std::map<std::string, GenEnum::PropName> g_map_crafter_props = {

    // strings must be lower case even though they appear mixed case in the .wxcp file -- they are converted to lower-case
    // before pattern matching.

    { "# columns", prop_cols },
    { "# rows", prop_rows },
    { "allow cell editing", prop_editing },
    { "bg colour", prop_background_colour },
    { "bitmap size", prop_bitmapsize },
    { "choices", prop_contents },
    { "class decorator", prop_class_decoration },
    { "class name", prop_derived_class },
    { "col label horizontal align", prop_col_label_horiz_alignment },
    { "col label vertical align", prop_col_label_vert_alignment },
    { "column flags", prop_flags },
    { "default button", prop_default },
    { "default path", prop_defaultfolder },
    { "enable window persistency", prop_persist },
    { "fg colour", prop_foreground_colour },
    { "field count", prop_fields },
    { "file", prop_derived_file },
    { "filter index", prop_defaultfilter },
    { "fit content to columns", prop_autosize_cols },
    { "growable columns", prop_growablecols },
    { "growable rows", prop_growablerows },
    { "help string", prop_statusbar },
    { "horizontal gap", prop_hgap },
    { "hover colour", prop_hover_color },
    { "html code", prop_html_content },
    { "include file", prop_derived_header },
    { "inherited class", prop_class_name },
    { "load and play", prop_play },
    { "major dimension", prop_majorDimension },
    { "max length", prop_maxlength },
    { "max value", prop_max },
    { "maximum #rows", prop_max_rows },
    { "min value", prop_min },
    { "minimum #rows", prop_min_rows },
    { "minimum pane size", prop_min_pane_size },
    { "minimum size", prop_minimum_size },
    { "normal colour", prop_normal_color },
    { "padding", prop_packing },
    { "page size", prop_pagesize },
    { "ribbon theme", prop_theme },
    { "row label horizontal align", prop_row_label_horiz_alignment },
    { "row label vertical align", prop_row_label_vert_alignment },
    { "sash gravity", prop_sashgravity },
    { "sash position", prop_sashpos },
    { "scroll rate x", prop_scroll_rate_x },
    { "scroll rate y", prop_scroll_rate_y },
    { "selected", prop_select },
    { "separator size", prop_separation },
    { "show cancel button", prop_cancel_button },
    { "show hidden files", prop_show_hidden },
    { "show search button", prop_search_button },
    { "split mode", prop_splitmode },
    { "text hint", prop_hint },
    { "use native header", prop_native_col_header },
    { "vertical gap", prop_vgap },
    { "visited colour", prop_visited_color },

};
