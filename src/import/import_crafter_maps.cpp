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
    { 4457, gen_wxMenuBar },
    { 4458, gen_wxMenu },
    { 4459, gen_wxMenuItem },
    { 4460, gen_submenu },
    { 4461, gen_wxToolBar },
    { 4462, gen_tool },
    { 4463, gen_wxAuiToolBar },
    { 4464, gen_wxStatusBar },
    { 4465, gen_CustomControl },
    { 4466, gen_wxStyledTextCtrl },        // verified
    { 4467, gen_wxStdDialogButtonSizer },  // verified
    { 4469, gen_wxDataViewListCtrl },
    { 4470, gen_wxDataViewTreeCtrl },
    { 4471, gen_wxBannerWindow },
    { 4474, gen_wxCommandLinkButton },
    { 4475, gen_wxCollapsiblePane },
    { 4477, gen_wxInfoBar },
    { 4479, gen_wxWebView },
    { 4484, gen_wxPopupTransientWindow },
    { 4485, gen_wxPropertyGridManager },
    { 4486, gen_propGridItem },
    { 4488, gen_wxRibbonBar },
    { 4489, gen_wxRibbonPage },
    { 4490, gen_wxRibbonPanel },
    { 4504, gen_toolSeparator },
    { 4509, gen_wxTreeListCtrl },
    { 4511, gen_wxSimplebook },    // verified
    { 4513, gen_wxToggleButton },  // verified
    { 4519, gen_wxAnimationCtrl },
    { 4520, gen_wxBitmapComboBox },
    { 4521, gen_wxRearrangeCtrl },
    { 4522, gen_wxSimpleHtmlListBox },
    { 4523, gen_wxActivityIndicator },
    { 4524, gen_wxTimePickerCtrl },

    { 4414, gen_unknown },  // column header for wxListView, supported directly as a wxListView property
    { 4468, gen_unknown },  // Theses are the buttons to enable in gen_wxStdDialogButtonSizer

    { 4455, gen_unknown /* WXTREEBOOK_SUB_PAGE */ },
    { 4472, gen_unknown /* WXDATAVIEWCOL */ },
    { 4473, gen_unknown /* WXDATAVIEWTREELISTCTRL */ },
    { 4476, gen_unknown /* WXCOLLAPSIBLEPANE_PANE */ },
    { 4478, gen_unknown /* WXINFOBARBUTTON */ },
    { 4480, gen_unknown /* WXAUIMANAGER */ },
    { 4481, gen_unknown /* WXIMAGELIST */ },
    { 4482, gen_unknown /* WXBITMAP */ },
    { 4483, gen_unknown /* WXTIMER */ },
    { 4487, gen_unknown /* WXPGPROPERTY_SUB */ },
    { 4491, gen_unknown /* WXRIBBONBUTTONBAR */ },
    { 4492, gen_unknown /* WXRIBBONBUTTON */ },
    { 4493, gen_unknown /* WXRIBBONHYBRIDBUTTON */ },
    { 4494, gen_unknown /* WXRIBBONDROPDOWNBUTTON */ },
    { 4495, gen_unknown /* WXRIBBONTOGGLEBUTTON */ },
    { 4496, gen_unknown /* WXRIBBONGALLERY */ },
    { 4497, gen_unknown /* WXRIBBONGALLERYITME */ },
    { 4498, gen_unknown /* WXRIBBONTOOLBAR */ },
    { 4499, gen_unknown /* WXRIBBONTOOL */ },
    { 4500, gen_unknown /* WXRIBBONHYBRIDTOOL */ },
    { 4501, gen_unknown /* WXRIBBONDROPDOWNTOOL */ },
    { 4502, gen_unknown /* WXRIBBONTOGGLETOOL */ },
    { 4503, gen_unknown /* WXRIBBONTOOLSEPARATOR */ },
    { 4505, gen_unknown /* WXGLCANVAS */ },
    { 4506, gen_unknown /* WXGRIDCOL */ },
    { 4507, gen_unknown /* WXGRIDROW */ },
    { 4508, gen_unknown /* WXMEDIACTRL */ },
    { 4510, gen_unknown /* WXTREELISTCTRLCOL */ },
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
    { "bg colour", prop_background_colour },
    { "bitmap size", prop_bitmapsize },
    { "choices", prop_contents },
    { "class decorator", prop_class_decoration },
    { "class name", prop_derived_class },
    { "default button", prop_default },
    { "default path", prop_defaultfolder },
    { "enable window persistency", prop_persist },
    { "fg colour", prop_foreground_colour },
    { "file", prop_derived_file },
    { "filter index", prop_defaultfilter },
    { "growable columns", prop_growablecols },
    { "growable rows", prop_growablerows },
    { "help string", prop_statusbar },
    { "horizontal gap", prop_hgap },
    { "hover colour", prop_hover_color },
    { "html code", prop_html_content },
    { "include file", prop_derived_header },
    { "inherited class", prop_class_name },
    { "major dimension", prop_majorDimension },
    { "max length", prop_maxlength },
    { "max value", prop_max },
    { "min value", prop_min },
    { "minimum pane size", prop_min_pane_size },
    { "minimum size", prop_minimum_size },
    { "normal colour", prop_normal_color },
    { "page size", prop_pagesize },
    { "sash gravity", prop_sashgravity },
    { "sash position", prop_sashpos },
    { "scroll rate x", prop_scroll_rate_x },
    { "scroll rate y", prop_scroll_rate_y },
    { "show cancel button", prop_cancel_button },
    { "show hidden files", prop_show_hidden },
    { "show search button", prop_search_button },
    { "split mode", prop_splitmode },
    { "text hint", prop_hint },
    { "vertical gap", prop_vgap },
    { "visited colour", prop_visited_color },

    // { "minimum", prop_min_size},

};
