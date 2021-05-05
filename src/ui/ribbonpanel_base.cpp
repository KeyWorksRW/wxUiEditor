////////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/ribbon/page.h>
#include <wx/ribbon/panel.h>

#include "ribbon_ids.h"
#include "gen_enums.h"     // Enumerations for generators

using namespace GenEnum;

#include "ribbonpanel_base.h"

#include "../art_headers/book_page_png.hxx"
#include "../art_headers/calendar_png.hxx"
#include "../art_headers/colourPickerIcon_png.hxx"
#include "../art_headers/dataview_ctrl_png.hxx"
#include "../art_headers/dataviewlist_column_png.hxx"
#include "../art_headers/datepicker_png.hxx"
#include "../art_headers/dirPicker_png.hxx"
#include "../art_headers/doc_mdi_parent_frame_png.hxx"
#include "../art_headers/filePicker_png.hxx"
#include "../art_headers/flex_grid_sizer_png.hxx"
#include "../art_headers/fontPicker_png.hxx"
#include "../art_headers/gauge_png.hxx"
#include "../art_headers/genericdir_ctrl_png.hxx"
#include "../art_headers/grid_bag_sizer_png.hxx"
#include "../art_headers/grid_png.hxx"
#include "../art_headers/grid_sizer_png.hxx"
#include "../art_headers/htmlwin_png.hxx"
#include "../art_headers/hyperlink_ctrl_png.hxx"
#include "../art_headers/infobar_png.hxx"
#include "../art_headers/menu_png.hxx"
#include "../art_headers/menuitem_png.hxx"
#include "../art_headers/propgriditem_png.hxx"
#include "../art_headers/propgridpage_png.hxx"
#include "../art_headers/radio_box_png.hxx"
#include "../art_headers/ribbon_bar_png.hxx"
#include "../art_headers/ribbon_button_png.hxx"
#include "../art_headers/ribbon_buttonbar_png.hxx"
#include "../art_headers/ribbon_gallery_item_png.hxx"
#include "../art_headers/ribbon_page_png.hxx"
#include "../art_headers/ribbon_panel_png.hxx"
#include "../art_headers/richtextctrl_png.hxx"
#include "../art_headers/scintilla_png.hxx"
#include "../art_headers/search_png.hxx"
#include "../art_headers/separator_png.hxx"
#include "../art_headers/sizer_horizontal_png.hxx"
#include "../art_headers/sizer_png.hxx"
#include "../art_headers/slider_png.hxx"
#include "../art_headers/spacer_png.hxx"
#include "../art_headers/spin_ctrl_png.hxx"
#include "../art_headers/static_bitmap_png.hxx"
#include "../art_headers/static_line_png.hxx"
#include "../art_headers/statusbar_png.hxx"
#include "../art_headers/stddialogbuttonsizer_png.hxx"
#include "../art_headers/submenu_png.hxx"
#include "../art_headers/text_sizer_png.hxx"
#include "../art_headers/timepicker_png.hxx"
#include "../art_headers/tool_png.hxx"
#include "../art_headers/toolseparator_png.hxx"
#include "../art_headers/tree_ctrl_png.hxx"
#include "../art_headers/treelistctrl_png.hxx"
#include "../art_headers/treelistctrlcolumn_png.hxx"
#include "../art_headers/wrap_sizer_png.hxx"
#include "../art_headers/wxActivityIndicator_png.hxx"
#include "../art_headers/wxBannerWindow_png.hxx"
#include "../art_headers/wxButton_png.hxx"
#include "../art_headers/wxCheckBox_png.hxx"
#include "../art_headers/wxChoicebook_png.hxx"
#include "../art_headers/wxCollapsiblePane_png.hxx"
#include "../art_headers/wxComboBox_png.hxx"
#include "../art_headers/wxDialog_png.hxx"
#include "../art_headers/wxFileCtrl_png.hxx"
#include "../art_headers/wxFrame_png.hxx"
#include "../art_headers/wxListBox_png.hxx"
#include "../art_headers/wxListbook_png.hxx"
#include "../art_headers/wxMenuBar_png.hxx"
#include "../art_headers/wxNotebook_png.hxx"
#include "../art_headers/wxPanel_png.hxx"
#include "../art_headers/wxPopupTransientWindow_png.hxx"
#include "../art_headers/wxPropertyGridManager_png.hxx"
#include "../art_headers/wxPropertyGrid_png.hxx"
#include "../art_headers/wxRadioButton_png.hxx"
#include "../art_headers/wxScrolledWindow_png.hxx"
#include "../art_headers/wxSimplebook_png.hxx"
#include "../art_headers/wxSplitterWindow_png.hxx"
#include "../art_headers/wxStaticBoxSizer_png.hxx"
#include "../art_headers/wxStaticText_png.hxx"
#include "../art_headers/wxTextCtrl_png.hxx"
#include "../art_headers/wxToolBar_png.hxx"
#include "../art_headers/wxToolbook_png.hxx"
#include "../art_headers/wxTreebook_png.hxx"
#include "../art_headers/wxWizardPageSimple_png.hxx"
#include "../art_headers/wxWizard_png.hxx"

#include <wx/mstream.h>  // Memory stream classes

// Convert a data header file into a wxImage
static wxImage GetImgFromHdr(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream strm(data, size_data);
    wxImage image;
    image.LoadFile(strm);
    return image;
};

RibbonPanelBase::RibbonPanelBase(wxWindow* parent, wxWindowID id,
		const wxPoint& pos, const wxSize& size, long style) :
	wxPanel(parent, id, pos, size, style)
{
    parent_sizer = new wxBoxSizer(wxVERTICAL);

    m_rbnBar = new wxRibbonBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRIBBON_BAR_SHOW_PAGE_LABELS);
    m_rbnBar->SetArtProvider(new wxRibbonMSWArtProvider);
    parent_sizer->Add(m_rbnBar, wxSizerFlags().Expand().Border(wxALL));

    auto page_forms = new wxRibbonPage(m_rbnBar, wxID_ANY, wxString::FromUTF8("Forms"));

    auto panel_form_windows = new wxRibbonPanel(page_forms, wxID_ANY, wxString::FromUTF8("Windows"));

    auto forms_bar_windows = new wxRibbonToolBar(panel_form_windows, wxID_ANY);
    {
        forms_bar_windows->AddTool(gen_wxDialog, GetImgFromHdr(wxDialog_png, sizeof(wxDialog_png)), wxString::FromUTF8("wxDialog"), wxRIBBON_BUTTON_NORMAL);
        forms_bar_windows->AddTool(gen_PanelForm, GetImgFromHdr(wxPanel_png, sizeof(wxPanel_png)), wxString::FromUTF8("wxPanel"), wxRIBBON_BUTTON_NORMAL);
        forms_bar_windows->AddTool(gen_wxFrame, GetImgFromHdr(wxFrame_png, sizeof(wxFrame_png)), wxString::FromUTF8("wxFrame"), wxRIBBON_BUTTON_NORMAL);
        forms_bar_windows->AddTool(NewPopupWin, GetImgFromHdr(wxPopupTransientWindow_png, sizeof(wxPopupTransientWindow_png)), wxString::FromUTF8("wxPopupTransientWindow"), wxRIBBON_BUTTON_NORMAL);
    }
    forms_bar_windows->Realize();

    auto panel_wizard = new wxRibbonPanel(page_forms, wxID_ANY, wxString::FromUTF8("Wizard"),

    GetImgFromHdr(wxWizard_png, sizeof(wxWizard_png)));

    auto forms_bar_wizard = new wxRibbonToolBar(panel_wizard, wxID_ANY);
    {
        forms_bar_wizard->AddTool(gen_wxWizard, GetImgFromHdr(wxWizard_png, sizeof(wxWizard_png)), wxString::FromUTF8("wxWizard"), wxRIBBON_BUTTON_NORMAL);
        forms_bar_wizard->AddTool(gen_wxWizardPageSimple, GetImgFromHdr(wxWizardPageSimple_png, sizeof(wxWizardPageSimple_png)), wxString::FromUTF8("wxWizardPageSimple"), wxRIBBON_BUTTON_NORMAL);
    }
    forms_bar_wizard->Realize();

    auto panel_bars = new wxRibbonPanel(page_forms, wxID_ANY, wxString::FromUTF8("Bars"));

    auto forms_bar_bars = new wxRibbonToolBar(panel_bars, wxID_ANY);
    {
        forms_bar_bars->AddTool(gen_MenuBar, GetImgFromHdr(wxMenuBar_png, sizeof(wxMenuBar_png)), wxString::FromUTF8("wxMenuBar"), wxRIBBON_BUTTON_NORMAL);
        forms_bar_bars->AddTool(gen_ToolBar, GetImgFromHdr(wxToolBar_png, sizeof(wxToolBar_png)), wxString::FromUTF8("wxToolBar"), wxRIBBON_BUTTON_NORMAL);
    }
    forms_bar_bars->Realize();

    auto pg_sizer = new wxRibbonPage(m_rbnBar, wxID_ANY, wxString::FromUTF8("Sizers"));

    auto panel_basic = new wxRibbonPanel(pg_sizer, wxID_ANY, wxString::FromUTF8("Basic"));

    auto sizer_bar_basic = new wxRibbonToolBar(panel_basic, wxID_ANY);
    {
        sizer_bar_basic->AddTool(gen_wxBoxSizer, GetImgFromHdr(sizer_horizontal_png, sizeof(sizer_horizontal_png)), wxString::FromUTF8("Horizontal wxBoxSizer"), wxRIBBON_BUTTON_NORMAL);
        sizer_bar_basic->AddTool(gen_VerticalBoxSizer, GetImgFromHdr(sizer_png, sizeof(sizer_png)), wxString::FromUTF8("Vertical wxBoxSizer"), wxRIBBON_BUTTON_NORMAL);
        sizer_bar_basic->AddTool(NewStaticSizer, GetImgFromHdr(wxStaticBoxSizer_png, sizeof(wxStaticBoxSizer_png)), wxString::FromUTF8("wxStaticBoxSizer"), wxRIBBON_BUTTON_DROPDOWN);
        sizer_bar_basic->AddTool(gen_wxWrapSizer, GetImgFromHdr(wrap_sizer_png, sizeof(wrap_sizer_png)), wxString::FromUTF8("wxWrapSizer"), wxRIBBON_BUTTON_NORMAL);
    }
    sizer_bar_basic->Realize();

    auto sizer_panel_grids = new wxRibbonPanel(pg_sizer, wxID_ANY, wxString::FromUTF8("Grids"));

    auto sizer_bar_grids = new wxRibbonToolBar(sizer_panel_grids, wxID_ANY);
    {
        sizer_bar_grids->AddTool(gen_wxGridSizer, GetImgFromHdr(grid_sizer_png, sizeof(grid_sizer_png)), wxString::FromUTF8("wxGridSizer"), wxRIBBON_BUTTON_NORMAL);
        sizer_bar_grids->AddTool(gen_wxFlexGridSizer, GetImgFromHdr(flex_grid_sizer_png, sizeof(flex_grid_sizer_png)), wxString::FromUTF8("wxFlexGridSizer"), wxRIBBON_BUTTON_NORMAL);
        sizer_bar_grids->AddTool(gen_wxGridBagSizer, GetImgFromHdr(grid_bag_sizer_png, sizeof(grid_bag_sizer_png)), wxString::FromUTF8("wxGridBagSizer"), wxRIBBON_BUTTON_NORMAL);
    }
    sizer_bar_grids->Realize();

    auto sizer_panel_other = new wxRibbonPanel(pg_sizer, wxID_ANY, wxString::FromUTF8("Other"));

    auto sizer_bar_other = new wxRibbonToolBar(sizer_panel_other, wxID_ANY);
    {
        sizer_bar_other->AddTool(gen_wxStdDialogButtonSizer, GetImgFromHdr(stddialogbuttonsizer_png, sizeof(stddialogbuttonsizer_png)), wxString::FromUTF8("wxStdDialogButtonSizer"), wxRIBBON_BUTTON_NORMAL);
        sizer_bar_other->AddTool(gen_TextSizer, GetImgFromHdr(text_sizer_png, sizeof(text_sizer_png)), wxString::FromUTF8("wxTextSizerWrapper"), wxRIBBON_BUTTON_NORMAL);
        sizer_bar_other->AddTool(gen_spacer, GetImgFromHdr(spacer_png, sizeof(spacer_png)), wxString::FromUTF8("spacer"), wxRIBBON_BUTTON_NORMAL);
    }
    sizer_bar_other->Realize();

    auto pg_common = new wxRibbonPage(m_rbnBar, wxID_ANY, wxString::FromUTF8("Common"));

    auto panel_common_controls = new wxRibbonPanel(pg_common, wxID_ANY, wxString::FromUTF8("Controls"),
    wxNullBitmap, wxDefaultPosition, wxDefaultSize,
    wxRIBBON_PANEL_STRETCH|wxRIBBON_PANEL_FLEXIBLE);

    auto common_bar_controls = new wxRibbonToolBar(panel_common_controls, wxID_ANY);
    {
        common_bar_controls->AddTool(gen_wxStaticText, GetImgFromHdr(wxStaticText_png, sizeof(wxStaticText_png)), wxString::FromUTF8("wxStaticText"), wxRIBBON_BUTTON_NORMAL);
        common_bar_controls->AddTool(gen_wxTextCtrl, GetImgFromHdr(wxTextCtrl_png, sizeof(wxTextCtrl_png)), wxString::FromUTF8("wxTextCtrl"), wxRIBBON_BUTTON_NORMAL);
        common_bar_controls->AddTool(NewCheckbox, GetImgFromHdr(wxCheckBox_png, sizeof(wxCheckBox_png)), wxString::FromUTF8("Check Boxes"), wxRIBBON_BUTTON_DROPDOWN);
        common_bar_controls->AddTool(gen_wxRadioButton, GetImgFromHdr(wxRadioButton_png, sizeof(wxRadioButton_png)), wxString::FromUTF8("wxRadioButton"), wxRIBBON_BUTTON_NORMAL);
        common_bar_controls->AddTool(NewButton, GetImgFromHdr(wxButton_png, sizeof(wxButton_png)), wxString::FromUTF8("Buttons"), wxRIBBON_BUTTON_DROPDOWN);
        common_bar_controls->AddTool(NewSpin, GetImgFromHdr(spin_ctrl_png, sizeof(spin_ctrl_png)), wxString::FromUTF8("Spin controls"), wxRIBBON_BUTTON_DROPDOWN);
    }
    common_bar_controls->Realize();

    auto panel_choices = new wxRibbonPanel(pg_common, wxID_ANY, wxString::FromUTF8("Choices"),
    wxNullBitmap, wxDefaultPosition, wxDefaultSize,
    wxRIBBON_PANEL_STRETCH|wxRIBBON_PANEL_FLEXIBLE);

    auto common_bar_choices = new wxRibbonToolBar(panel_choices, wxID_ANY);
    {
        common_bar_choices->AddTool(NewCombobox, GetImgFromHdr(wxComboBox_png, sizeof(wxComboBox_png)), wxString::FromUTF8("Combos & Choice"), wxRIBBON_BUTTON_DROPDOWN);
        common_bar_choices->AddTool(NewListbox, GetImgFromHdr(wxListBox_png, sizeof(wxListBox_png)), wxString::FromUTF8("Lists"), wxRIBBON_BUTTON_DROPDOWN);
        common_bar_choices->AddTool(gen_wxRadioBox, GetImgFromHdr(radio_box_png, sizeof(radio_box_png)), wxString::FromUTF8("wxRadioBox"), wxRIBBON_BUTTON_NORMAL);
    }
    common_bar_choices->Realize();

    auto panel_pickkers = new wxRibbonPanel(pg_common, wxID_ANY, wxString::FromUTF8("Pickers"),
    wxNullBitmap, wxDefaultPosition, wxDefaultSize,
    wxRIBBON_PANEL_STRETCH|wxRIBBON_PANEL_FLEXIBLE);

    auto common_bar_pickers = new wxRibbonToolBar(panel_pickkers, wxID_ANY);
    {
        common_bar_pickers->AddTool(gen_wxFilePickerCtrl, GetImgFromHdr(filePicker_png, sizeof(filePicker_png)), wxString::FromUTF8("wxFilePickerCtrl"), wxRIBBON_BUTTON_NORMAL);
        common_bar_pickers->AddTool(gen_wxDirPickerCtrl, GetImgFromHdr(dirPicker_png, sizeof(dirPicker_png)), wxString::FromUTF8("wxDirPickerCtrl"), wxRIBBON_BUTTON_NORMAL);
        common_bar_pickers->AddTool(gen_wxFontPickerCtrl, GetImgFromHdr(fontPicker_png, sizeof(fontPicker_png)), wxString::FromUTF8("wxFontPickerCtrl"), wxRIBBON_BUTTON_NORMAL);
        common_bar_pickers->AddTool(gen_wxColourPickerCtrl, GetImgFromHdr(colourPickerIcon_png, sizeof(colourPickerIcon_png)), wxString::FromUTF8("wxColourPickerCtrl"), wxRIBBON_BUTTON_NORMAL);
        common_bar_pickers->AddTool(gen_wxDatePickerCtrl, GetImgFromHdr(datepicker_png, sizeof(datepicker_png)), wxString::FromUTF8("wxDatePickerCtrl"), wxRIBBON_BUTTON_NORMAL);
        common_bar_pickers->AddTool(gen_wxTimePickerCtrl, GetImgFromHdr(timepicker_png, sizeof(timepicker_png)), wxString::FromUTF8("wxTimePickerCtrl"), wxRIBBON_BUTTON_NORMAL);
    }
    common_bar_pickers->Realize();

    auto panel_other = new wxRibbonPanel(pg_common, wxID_ANY, wxString::FromUTF8("Other"),
    wxNullBitmap, wxDefaultPosition, wxDefaultSize,
    wxRIBBON_PANEL_STRETCH|wxRIBBON_PANEL_FLEXIBLE);

    auto common_bar_other = new wxRibbonToolBar(panel_other, wxID_ANY);
    {
        common_bar_other->AddTool(gen_wxStaticBitmap, GetImgFromHdr(static_bitmap_png, sizeof(static_bitmap_png)), wxString::FromUTF8("wxStaticBitmap"), wxRIBBON_BUTTON_NORMAL);
        common_bar_other->AddTool(gen_wxStaticLine, GetImgFromHdr(static_line_png, sizeof(static_line_png)), wxString::FromUTF8("wxStaticLine"), wxRIBBON_BUTTON_NORMAL);
        common_bar_other->AddTool(gen_wxSlider, GetImgFromHdr(slider_png, sizeof(slider_png)), wxString::FromUTF8("wxSlider"), wxRIBBON_BUTTON_NORMAL);
        common_bar_other->AddTool(gen_wxGauge, GetImgFromHdr(gauge_png, sizeof(gauge_png)), wxString::FromUTF8("wxGauge"), wxRIBBON_BUTTON_NORMAL);
    }
    common_bar_other->Realize();

    auto page_containers = new wxRibbonPage(m_rbnBar, wxID_ANY, wxString::FromUTF8("Containers"));

    auto panel_windows = new wxRibbonPanel(page_containers, wxID_ANY, wxString::FromUTF8("Windows"));

    auto container_bar_windows = new wxRibbonToolBar(panel_windows, wxID_ANY);
    {
        container_bar_windows->AddTool(gen_wxPanel, GetImgFromHdr(wxPanel_png, sizeof(wxPanel_png)), wxString::FromUTF8("wxPanel"), wxRIBBON_BUTTON_NORMAL);
        container_bar_windows->AddTool(gen_wxSplitterWindow, GetImgFromHdr(wxSplitterWindow_png, sizeof(wxSplitterWindow_png)), wxString::FromUTF8("wxSplitterWindow"), wxRIBBON_BUTTON_NORMAL);
        container_bar_windows->AddTool(gen_wxScrolledWindow, GetImgFromHdr(wxScrolledWindow_png, sizeof(wxScrolledWindow_png)), wxString::FromUTF8("wxScrolledWindow"), wxRIBBON_BUTTON_NORMAL);
        container_bar_windows->AddTool(gen_wxCollapsiblePane, GetImgFromHdr(wxCollapsiblePane_png, sizeof(wxCollapsiblePane_png)), wxString::FromUTF8("wxCollapsiblePane"), wxRIBBON_BUTTON_NORMAL);
    }
    container_bar_windows->Realize();

    auto panel_books = new wxRibbonPanel(page_containers, wxID_ANY, wxString::FromUTF8("Books"));

    auto container_bar_books = new wxRibbonToolBar(panel_books, wxID_ANY);
    {
        container_bar_books->AddTool(gen_wxChoicebook, GetImgFromHdr(wxChoicebook_png, sizeof(wxChoicebook_png)), wxString::FromUTF8("wxChoicebook"), wxRIBBON_BUTTON_NORMAL);
        container_bar_books->AddTool(gen_wxListbook, GetImgFromHdr(wxListbook_png, sizeof(wxListbook_png)), wxString::FromUTF8("wxListbook"), wxRIBBON_BUTTON_NORMAL);
        container_bar_books->AddTool(gen_wxNotebook, GetImgFromHdr(wxNotebook_png, sizeof(wxNotebook_png)), wxString::FromUTF8("wxNotebook"), wxRIBBON_BUTTON_NORMAL);
        container_bar_books->AddTool(gen_wxSimplebook, GetImgFromHdr(wxSimplebook_png, sizeof(wxSimplebook_png)), wxString::FromUTF8("wxSimplebook"), wxRIBBON_BUTTON_NORMAL);
        container_bar_books->AddTool(gen_wxToolbook, GetImgFromHdr(wxToolbook_png, sizeof(wxToolbook_png)), wxString::FromUTF8("wxToolbook"), wxRIBBON_BUTTON_NORMAL);
        container_bar_books->AddTool(gen_wxTreebook, GetImgFromHdr(wxTreebook_png, sizeof(wxTreebook_png)), wxString::FromUTF8("wxTreebook"), wxRIBBON_BUTTON_NORMAL);
    }
    container_bar_books->Realize();

    auto panel_page = new wxRibbonPanel(page_containers, wxID_ANY, wxString::FromUTF8("Page"));

    auto container_bar_page = new wxRibbonToolBar(panel_page, wxID_ANY);
    {
        container_bar_page->AddTool(gen_BookPage, GetImgFromHdr(book_page_png, sizeof(book_page_png)), wxString::FromUTF8("Book Page"), wxRIBBON_BUTTON_NORMAL);
    }
    container_bar_page->Realize();

    auto page_data = new wxRibbonPage(m_rbnBar, wxID_ANY, wxString::FromUTF8("Data"));

    auto panel_properties = new wxRibbonPanel(page_data, wxID_ANY, wxString::FromUTF8("Properties"));

    auto data_bar_properties = new wxRibbonToolBar(panel_properties, wxID_ANY);
    {
        data_bar_properties->AddTool(gen_wxPropertyGrid, GetImgFromHdr(wxPropertyGrid_png, sizeof(wxPropertyGrid_png)), wxString::FromUTF8("wxPropertyGrid"), wxRIBBON_BUTTON_NORMAL);
        data_bar_properties->AddTool(gen_wxPropertyGridManager, GetImgFromHdr(wxPropertyGridManager_png, sizeof(wxPropertyGridManager_png)), wxString::FromUTF8("wxPropertyGridManager"), wxRIBBON_BUTTON_NORMAL);
        data_bar_properties->AddTool(gen_propGridPage, GetImgFromHdr(propgridpage_png, sizeof(propgridpage_png)), wxString::FromUTF8("wxPropertyGrid Page"), wxRIBBON_BUTTON_NORMAL);
        data_bar_properties->AddTool(gen_propGridItem, GetImgFromHdr(propgriditem_png, sizeof(propgriditem_png)), wxString::FromUTF8("wxPropertyGrid Item"), wxRIBBON_BUTTON_NORMAL);
    }
    data_bar_properties->Realize();

    auto panel_data_trees = new wxRibbonPanel(page_data, wxID_ANY, wxString::FromUTF8("Trees"));

    auto data_bar_trees = new wxRibbonToolBar(panel_data_trees, wxID_ANY);
    {
        data_bar_trees->AddTool(gen_wxTreeCtrl, GetImgFromHdr(tree_ctrl_png, sizeof(tree_ctrl_png)), wxString::FromUTF8("wxTreeCtrl"), wxRIBBON_BUTTON_NORMAL);
        data_bar_trees->AddTool(gen_wxTreeListCtrl, GetImgFromHdr(treelistctrl_png, sizeof(treelistctrl_png)), wxString::FromUTF8("wxTreeListCtrl"), wxRIBBON_BUTTON_NORMAL);
        data_bar_trees->AddTool(gen_TreeListCtrlColumn, GetImgFromHdr(treelistctrlcolumn_png, sizeof(treelistctrlcolumn_png)), wxString::FromUTF8("wxTreeListCtrl Column"), wxRIBBON_BUTTON_NORMAL);
    }
    data_bar_trees->Realize();

    auto panel_trees = new wxRibbonPanel(page_data, wxID_ANY, wxString::FromUTF8("DataView"));

    auto data_bar_dataview = new wxRibbonToolBar(panel_trees, wxID_ANY);
    {
        data_bar_dataview->AddTool(NewDataCtrl, GetImgFromHdr(dataview_ctrl_png, sizeof(dataview_ctrl_png)), wxString::FromUTF8("Data Control"), wxRIBBON_BUTTON_DROPDOWN);
        data_bar_dataview->AddTool(gen_dataViewColumn, GetImgFromHdr(dataviewlist_column_png, sizeof(dataviewlist_column_png)), wxString::FromUTF8("DataView Column"), wxRIBBON_BUTTON_NORMAL);
        data_bar_dataview->AddTool(gen_dataViewListColumn, GetImgFromHdr(doc_mdi_parent_frame_png, sizeof(doc_mdi_parent_frame_png)), wxString::FromUTF8("DataViewList Column"), wxRIBBON_BUTTON_NORMAL);
    }
    data_bar_dataview->Realize();

    auto page_bars = new wxRibbonPage(m_rbnBar, wxID_ANY, wxString::FromUTF8("Bars"));

    auto panel_bars_menu = new wxRibbonPanel(page_bars, wxID_ANY, wxString::FromUTF8("Menu"));

    auto bars_bar_menu = new wxRibbonToolBar(panel_bars_menu, wxID_ANY);
    {
        bars_bar_menu->AddTool(gen_wxMenuBar, GetImgFromHdr(wxMenuBar_png, sizeof(wxMenuBar_png)), wxString::FromUTF8("wxMenuBar"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_menu->AddTool(gen_wxMenu, GetImgFromHdr(menu_png, sizeof(menu_png)), wxString::FromUTF8("wxMenu"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_menu->AddTool(gen_submenu, GetImgFromHdr(submenu_png, sizeof(submenu_png)), wxString::FromUTF8("submenu"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_menu->AddTool(gen_wxMenuItem, GetImgFromHdr(menuitem_png, sizeof(menuitem_png)), wxString::FromUTF8("wxMenuItem"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_menu->AddTool(gen_separator, GetImgFromHdr(separator_png, sizeof(separator_png)), wxString::FromUTF8("separator"), wxRIBBON_BUTTON_NORMAL);
    }
    bars_bar_menu->Realize();

    auto panel_bars_tool = new wxRibbonPanel(page_bars, wxID_ANY, wxString::FromUTF8("Tool"));

    auto bars_bar_tool = new wxRibbonToolBar(panel_bars_tool, wxID_ANY);
    {
        bars_bar_tool->AddTool(gen_wxToolBar, GetImgFromHdr(wxToolBar_png, sizeof(wxToolBar_png)), wxString::FromUTF8("wxToolBar"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_tool->AddTool(gen_tool, GetImgFromHdr(tool_png, sizeof(tool_png)), wxString::FromUTF8("Tool"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_tool->AddTool(gen_toolSeparator, GetImgFromHdr(toolseparator_png, sizeof(toolseparator_png)), wxString::FromUTF8("Tool Separator"), wxRIBBON_BUTTON_NORMAL);
    }
    bars_bar_tool->Realize();

    auto panel_bars_ribbon = new wxRibbonPanel(page_bars, wxID_ANY, wxString::FromUTF8("Ribbon"));

    auto bars_bar_ribbon = new wxRibbonToolBar(panel_bars_ribbon, wxID_ANY);
    {
        bars_bar_ribbon->AddTool(gen_wxRibbonBar, GetImgFromHdr(ribbon_bar_png, sizeof(ribbon_bar_png)), wxString::FromUTF8("wxRibbonBar"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_ribbon->AddTool(gen_wxRibbonPage, GetImgFromHdr(ribbon_page_png, sizeof(ribbon_page_png)), wxString::FromUTF8("wxRibbonPage"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_ribbon->AddTool(gen_wxRibbonPanel, GetImgFromHdr(ribbon_panel_png, sizeof(ribbon_panel_png)), wxString::FromUTF8("wxRibbonPanel"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_ribbon->AddTool(NewRibbonType, GetImgFromHdr(ribbon_buttonbar_png, sizeof(ribbon_buttonbar_png)), wxString::FromUTF8("Ribbon Bar Type"), wxRIBBON_BUTTON_DROPDOWN);
        bars_bar_ribbon->AddTool(gen_ribbonButton, GetImgFromHdr(ribbon_button_png, sizeof(ribbon_button_png)), wxString::FromUTF8("Ribbon Button"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_ribbon->AddTool(gen_ribbonGalleryItem, GetImgFromHdr(ribbon_gallery_item_png, sizeof(ribbon_gallery_item_png)), wxString::FromUTF8("Ribbon Gallery Item"), wxRIBBON_BUTTON_NORMAL);
    }
    bars_bar_ribbon->Realize();

    auto panel_bars_rother = new wxRibbonPanel(page_bars, wxID_ANY, wxString::FromUTF8("Other"));

    auto bars_bar_other = new wxRibbonToolBar(panel_bars_rother, wxID_ANY);
    {
        bars_bar_other->AddTool(gen_wxStatusBar, GetImgFromHdr(statusbar_png, sizeof(statusbar_png)), wxString::FromUTF8("wxStatusBar"), wxRIBBON_BUTTON_NORMAL);
        bars_bar_other->AddTool(gen_wxInfoBar, GetImgFromHdr(infobar_png, sizeof(infobar_png)), wxString::FromUTF8("wxInfoBar"), wxRIBBON_BUTTON_NORMAL);
    }
    bars_bar_other->Realize();

    auto pg_other = new wxRibbonPage(m_rbnBar, wxID_ANY, wxString::FromUTF8("Other"));

    auto panel_editors = new wxRibbonPanel(pg_other, wxID_ANY, wxString::FromUTF8("Editors"));

    auto other_bar_editors = new wxRibbonToolBar(panel_editors, wxID_ANY);
    {
        other_bar_editors->AddTool(gen_wxRichTextCtrl, GetImgFromHdr(richtextctrl_png, sizeof(richtextctrl_png)), wxString::FromUTF8("wxRichTextCtrl"), wxRIBBON_BUTTON_NORMAL);
        other_bar_editors->AddTool(gen_wxStyledTextCtrl, GetImgFromHdr(scintilla_png, sizeof(scintilla_png)), wxString::FromUTF8("wxStyledTextCtrl"), wxRIBBON_BUTTON_NORMAL);
        other_bar_editors->AddTool(gen_wxGrid, GetImgFromHdr(grid_png, sizeof(grid_png)), wxString::FromUTF8("wxGrid"), wxRIBBON_BUTTON_NORMAL);
    }
    other_bar_editors->Realize();

    auto panel_controls = new wxRibbonPanel(pg_other, wxID_ANY, wxString::FromUTF8("Controls"));

    auto other_bar_ctrls = new wxRibbonToolBar(panel_controls, wxID_ANY);
    {
        other_bar_ctrls->AddTool(gen_wxHyperlinkCtrl, GetImgFromHdr(hyperlink_ctrl_png, sizeof(hyperlink_ctrl_png)), wxString::FromUTF8("wxHyperlinkCtrl"), wxRIBBON_BUTTON_NORMAL);
        other_bar_ctrls->AddTool(gen_wxSearchCtrl, GetImgFromHdr(search_png, sizeof(search_png)), wxString::FromUTF8("wxSearchCtrl"), wxRIBBON_BUTTON_NORMAL);
        other_bar_ctrls->AddTool(gen_wxCalendarCtrl, GetImgFromHdr(calendar_png, sizeof(calendar_png)), wxString::FromUTF8("wxCalendarCtrl"), wxRIBBON_BUTTON_NORMAL);
        other_bar_ctrls->AddTool(gen_wxFileCtrl, GetImgFromHdr(wxFileCtrl_png, sizeof(wxFileCtrl_png)), wxString::FromUTF8("wxFileCtrl"), wxRIBBON_BUTTON_NORMAL);
        other_bar_ctrls->AddTool(gen_wxGenericDirCtrl, GetImgFromHdr(genericdir_ctrl_png, sizeof(genericdir_ctrl_png)), wxString::FromUTF8("wxGenericDirCtrl"), wxRIBBON_BUTTON_NORMAL);
        other_bar_ctrls->AddTool(gen_wxActivityIndicator, GetImgFromHdr(wxActivityIndicator_png, sizeof(wxActivityIndicator_png)), wxString::FromUTF8("wxActivityIndicator"), wxRIBBON_BUTTON_NORMAL);
        other_bar_ctrls->AddTool(gen_wxBannerWindow, GetImgFromHdr(wxBannerWindow_png, sizeof(wxBannerWindow_png)), wxString::FromUTF8("wxBannerWindow"), wxRIBBON_BUTTON_NORMAL);
    }
    other_bar_ctrls->Realize();

    auto panel_html = new wxRibbonPanel(pg_other, wxID_ANY, wxString::FromUTF8("HTML"));

    auto other_bar_html = new wxRibbonToolBar(panel_html, wxID_ANY);
    {
        other_bar_html->AddTool(gen_wxHtmlWindow, GetImgFromHdr(htmlwin_png, sizeof(htmlwin_png)), wxString::FromUTF8("wxHtmlWindow"), wxRIBBON_BUTTON_NORMAL);
    }
    other_bar_html->Realize();
    m_rbnBar->Realize();

    SetSizerAndFit(parent_sizer);

    // Event handlers
    forms_bar_windows->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    forms_bar_wizard->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    forms_bar_bars->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    sizer_bar_basic->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    sizer_bar_basic->Bind(wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, &RibbonPanelBase::OnDropDown, this);
    sizer_bar_grids->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    sizer_bar_other->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    common_bar_controls->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    common_bar_controls->Bind(wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, &RibbonPanelBase::OnDropDown, this);
    common_bar_choices->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    common_bar_choices->Bind(wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, &RibbonPanelBase::OnDropDown, this);
    common_bar_pickers->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    common_bar_other->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    container_bar_windows->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    container_bar_books->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    container_bar_page->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    data_bar_properties->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    data_bar_trees->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    data_bar_dataview->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    data_bar_dataview->Bind(wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, &RibbonPanelBase::OnDropDown, this);
    bars_bar_menu->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    bars_bar_tool->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    bars_bar_ribbon->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    bars_bar_ribbon->Bind(wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, &RibbonPanelBase::OnDropDown, this);
    bars_bar_other->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    other_bar_editors->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    other_bar_ctrls->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
    other_bar_html->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &RibbonPanelBase::OnToolClick, this);
}
