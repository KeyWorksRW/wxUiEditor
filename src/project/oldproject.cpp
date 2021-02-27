/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load older version of wxUiEditor project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "oldproject.h"

#include "mainapp.h"  // App -- Main application class

#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "pjtsettings.h"   // ProjectSettings -- Hold data for currently loaded project
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

// Called to replace properties in older project files
static bool CheckProperty(pugi::xml_node& xml_prop, Node* node);

static bool ProcessCheckBox(pugi::xml_node& xml_prop, Node* node);
static bool HandleDownLevelProperty(pugi::xml_node& xml_prop, ttlib::cview prop_name, ttlib::cview class_name, Node* node);

// Short term, this can be used to convert down-level .wxui project files. Long term, it can be used to convert wxFormBuilder
// projects. Note that there ARE some duplicated wxEVT_ values in the few cases where we no longer support variations of the
// same event.

// const std::map<const char*, const char*> g_mapNewEventNames = {
const std::map<std::string, const char*> g_mapNewEventNames = {

    { "OnActivate", "wxEVT_ACTIVATE" },
    { "OnActivateApp", "wxEVT_ACTIVATE_APP" },
    { "OnAuiPaneActivated", "wxEVT_AUI_PANE_ACTIVATED" },
    { "OnAuiPaneButton", "wxEVT_AUI_PANE_BUTTON" },
    { "OnAuiPaneClose", "wxEVT_AUI_PANE_CLOSE" },
    { "OnAuiPaneMaximize", "wxEVT_AUI_PANE_MAXIMIZE" },
    { "OnAuiPaneRestore", "wxEVT_AUI_PANE_RESTORE" },
    { "OnAuiRender", "wxEVT_AUI_RENDER" },
    { "OnAuiToolBarBeginDrag", "wxEVT_AUITOOLBAR_BEGIN_DRAG" },
    { "OnAuiToolBarMiddleClick", "wxEVT_AUITOOLBAR_MIDDLE_CLICK" },
    { "OnAuiToolBarOverflowClick", "wxEVT_AUITOOLBAR_OVERFLOW_CLICK" },
    { "OnAuiToolBarRightClick", "wxEVT_AUITOOLBAR_RIGHT_CLICK" },
    { "OnAuiToolBarToolDropDown", "wxEVT_AUITOOLBAR_TOOL_DROPDOWN" },
    { "OnAux1DClick", "wxEVT_AUX1_DCLICK" },
    { "OnAux1Down", "wxEVT_AUX1_DOWN" },
    { "OnAux1Up", "wxEVT_AUX1_UP" },
    { "OnAux2DClick", "wxEVT_AUX2_DCLICK" },
    { "OnAux2Down", "wxEVT_AUX2_DOWN" },
    { "OnAux2Up", "wxEVT_AUX2_UP" },
    { "OnButtonClick", "wxEVT_BUTTON" },
    { "OnCalendar", "wxEVT_CALENDAR_DOUBLECLICKED" },
    { "OnCalendarDay", "wxEVT_CALENDAR_DAY_CHANGED" },
    { "OnCalendarMonth", "wxEVT_CALENDAR_MONTH_CHANGED" },
    { "OnCalendarPageChanged", "wxEVT_CALENDAR_PAGE_CHANGED" },
    { "OnCalendarSelChanged", "wxEVT_CALENDAR_SEL_CHANGED" },
    { "OnCalendarWeekClicked", "wxEVT_CALENDAR_WEEK_CLICKED" },
    { "OnCalendarWeekDayClicked", "wxEVT_CALENDAR_WEEKDAY_CLICKED" },
    { "OnCalendarYear", "wxEVT_CALENDAR_YEAR_CHANGED" },
    { "OnCancelButton", "wxEVT_SEARCHCTRL_CANCEL_BTN" },
    { "OnChar", "wxEVT_CHAR" },
    { "OnCharHook", "wxEVT_CHAR_HOOK" },
    { "OnCheckBox", "wxEVT_CHECKBOX" },
    { "OnCheckListBox", "wxEVT_LISTBOX" },
    { "OnCheckListBoxDClick", "wxEVT_LISTBOX_DCLICK" },
    { "OnCheckListBoxToggled", "wxEVT_CHECKLISTBOX" },
    { "OnChoice", "wxEVT_CHOICE" },
    { "OnClose", "wxEVT_CLOSE_WINDOW" },
    { "OnColourChanged", "wxEVT_COLOURPICKER_CHANGED" },
    { "OnCombobox", "wxEVT_COMBOBOX" },
    { "OnComboboxCloseup", "wxEVT_COMBOBOX_CLOSEUP" },
    { "OnComboboxDropdown", "wxEVT_COMBOBOX_DROPDOWN" },
    { "OnCommandScroll", "AllScrollEvents" },
    { "OnCommandScrollBottom", "wxEVT_SCROLL_BOTTOM" },
    { "OnCommandScrollChanged", "wxEVT_SCROLL_CHANGED" },
    { "OnCommandScrollLineDown", "wxEVT_SCROLL_LINEDOWN" },
    { "OnCommandScrollLineUp", "wxEVT_SCROLL_LINEUP" },
    { "OnCommandScrollPageDown", "wxEVT_SCROLL_PAGEDOWN" },
    { "OnCommandScrollPageUp", "wxEVT_SCROLL_PAGEUP" },
    { "OnCommandScrollThumbRelease", "wxEVT_SCROLL_THUMBRELEASE" },
    { "OnCommandScrollThumbTrack", "wxEVT_SCROLL_THUMBTRACK" },
    { "OnCommandScrollTop", "wxEVT_SCROLL_TOP" },
    { "OnDataViewCtrlColumnHeaderClick", "wxEVT_DATAVIEW_COLUMN_HEADER_CLICK" },
    { "OnDataViewCtrlColumnHeaderRightClick", "wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK" },
    { "OnDataViewCtrlColumnReordered", "wxEVT_DATAVIEW_COLUMN_REORDERED" },
    { "OnDataViewCtrlColumnSorted", "wxEVT_DATAVIEW_COLUMN_SORTED" },
    { "OnDataViewCtrlItemActivated", "wxEVT_DATAVIEW_ITEM_ACTIVATED" },
    { "OnDataViewCtrlItemBeginDrag", "wxEVT_DATAVIEW_ITEM_BEGIN_DRAG" },
    { "OnDataViewCtrlItemCollapsed", "wxEVT_DATAVIEW_ITEM_COLLAPSED" },
    { "OnDataViewCtrlItemCollapsing", "wxEVT_DATAVIEW_ITEM_COLLAPSING" },
    { "OnDataViewCtrlItemContextMenu", "wxEVT_DATAVIEW_ITEM_CONTEXT_MENU" },
    { "OnDataViewCtrlItemDrop", "wxEVT_DATAVIEW_ITEM_DROP" },
    { "OnDataViewCtrlItemDropPossible", "wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE" },
    { "OnDataViewCtrlItemEditingDone", "wxEVT_DATAVIEW_ITEM_EDITING_DONE" },
    { "OnDataViewCtrlItemEditingStarted", "wxEVT_DATAVIEW_ITEM_EDITING_STARTED" },
    { "OnDataViewCtrlItemExpanded", "wxEVT_DATAVIEW_ITEM_EXPANDED" },
    { "OnDataViewCtrlItemExpanding", "wxEVT_DATAVIEW_ITEM_EXPANDING" },
    { "OnDataViewCtrlItemStartEditing", "wxEVT_DATAVIEW_ITEM_START_EDITING" },
    { "OnDataViewCtrlItemValueChanged", "wxEVT_DATAVIEW_ITEM_VALUE_CHANGED" },
    { "OnDataViewCtrlSelectionChanged", "wxEVT_DATAVIEW_SELECTION_CHANGED" },
    { "OnDataViewListCtrlColumnHeaderClick", "wxEVT_DATAVIEW_COLUMN_HEADER_CLICK" },
    { "OnDataViewListCtrlColumnHeaderRightClick", "wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK" },
    { "OnDataViewListCtrlColumnReordered", "wxEVT_DATAVIEW_COLUMN_REORDERED" },
    { "OnDataViewListCtrlColumnSorted", "wxEVT_DATAVIEW_COLUMN_SORTED" },
    { "OnDataViewListCtrlItemActivated", "wxEVT_DATAVIEW_ITEM_ACTIVATED" },
    { "OnDataViewListCtrlItemBeginDrag", "wxEVT_DATAVIEW_ITEM_BEGIN_DRAG" },
    { "OnDataViewListCtrlItemCollapsed", "wxEVT_DATAVIEW_ITEM_COLLAPSED" },
    { "OnDataViewListCtrlItemCollapsing", "wxEVT_DATAVIEW_ITEM_COLLAPSING" },
    { "OnDataViewListCtrlItemContextMenu", "wxEVT_DATAVIEW_ITEM_CONTEXT_MENU" },
    { "OnDataViewListCtrlItemDrop", "wxEVT_DATAVIEW_ITEM_DROP" },
    { "OnDataViewListCtrlItemDropPossible", "wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE" },
    { "OnDataViewListCtrlItemEditingDone", "wxEVT_DATAVIEW_ITEM_EDITING_DONE" },
    { "OnDataViewListCtrlItemEditingStarted", "wxEVT_DATAVIEW_ITEM_EDITING_STARTED" },
    { "OnDataViewListCtrlItemExpanded", "wxEVT_DATAVIEW_ITEM_EXPANDED" },
    { "OnDataViewListCtrlItemExpanding", "wxEVT_DATAVIEW_ITEM_EXPANDING" },
    { "OnDataViewListCtrlItemStartEditing", "wxEVT_DATAVIEW_ITEM_START_EDITING" },
    { "OnDataViewListCtrlItemValueChanged", "wxEVT_DATAVIEW_ITEM_VALUE_CHANGED" },
    { "OnDataViewListCtrlSelectionChanged", "wxEVT_DATAVIEW_SELECTION_CHANGED" },
    { "OnDataViewTreeCtrlColumnHeaderClick", "wxEVT_DATAVIEW_COLUMN_HEADER_CLICK" },
    { "OnDataViewTreeCtrlColumnHeaderRightClick", "wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK" },
    { "OnDataViewTreeCtrlColumnReordered", "wxEVT_DATAVIEW_COLUMN_REORDERED" },
    { "OnDataViewTreeCtrlColumnSorted", "wxEVT_DATAVIEW_COLUMN_SORTED" },
    { "OnDataViewTreeCtrlItemActivated", "wxEVT_DATAVIEW_ITEM_ACTIVATED" },
    { "OnDataViewTreeCtrlItemBeginDrag", "wxEVT_DATAVIEW_ITEM_BEGIN_DRAG" },
    { "OnDataViewTreeCtrlItemCollapsed", "wxEVT_DATAVIEW_ITEM_COLLAPSED" },
    { "OnDataViewTreeCtrlItemCollapsing", "wxEVT_DATAVIEW_ITEM_COLLAPSING" },
    { "OnDataViewTreeCtrlItemContextMenu", "wxEVT_DATAVIEW_ITEM_CONTEXT_MENU" },
    { "OnDataViewTreeCtrlItemDrop", "wxEVT_DATAVIEW_ITEM_DROP" },
    { "OnDataViewTreeCtrlItemDropPossible", "wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE" },
    { "OnDataViewTreeCtrlItemEditingDone", "wxEVT_DATAVIEW_ITEM_EDITING_DONE" },
    { "OnDataViewTreeCtrlItemEditingStarted", "wxEVT_DATAVIEW_ITEM_EDITING_STARTED" },
    { "OnDataViewTreeCtrlItemExpanded", "wxEVT_DATAVIEW_ITEM_EXPANDED" },
    { "OnDataViewTreeCtrlItemExpanding", "wxEVT_DATAVIEW_ITEM_EXPANDING" },
    { "OnDataViewTreeCtrlItemStartEditing", "wxEVT_DATAVIEW_ITEM_START_EDITING" },
    { "OnDataViewTreeCtrlItemValueChanged", "wxEVT_DATAVIEW_ITEM_VALUE_CHANGED" },
    { "OnDataViewTreeCtrlSelectionChanged", "wxEVT_DATAVIEW_SELECTION_CHANGED" },
    { "OnDateChanged", "wxEVT_DATE_CHANGED" },
    { "OnDirChanged", "wxEVT_DIRPICKER_CHANGED" },
    { "OnDirctrlFileActivated", "wxEVT_DIRCTRL_FILEACTIVATED" },
    { "OnDirctrlSelectionChanged", "wxEVT_DIRCTRL_SELECTIONCHANGED" },
    { "OnEnterWindow", "wxEVT_ENTER_WINDOW" },
    { "OnEraseBackground", "wxEVT_ERASE_BACKGROUND" },
    { "OnFileChanged", "wxEVT_FILEPICKER_CHANGED" },
    { "OnFontChanged", "wxEVT_FONTPICKER_CHANGED" },
    { "OnGridCellChange", "wxEVT_GRID_CELL_CHANGED" },
    { "OnGridCellLeftClick", "wxEVT_GRID_CELL_LEFT_CLICK" },
    { "OnGridCellLeftDClick", "wxEVT_GRID_CELL_LEFT_DCLICK" },
    { "OnGridCellRightClick", "wxEVT_GRID_CELL_RIGHT_CLICK" },
    { "OnGridCellRightDClick", "wxEVT_GRID_CELL_RIGHT_DCLICK" },
    { "OnGridCmdCellChange", "wxEVT_GRID_CELL_CHANGED" },
    { "OnGridCmdCellLeftClick", "wxEVT_GRID_CELL_LEFT_CLICK" },
    { "OnGridCmdCellLeftDClick", "wxEVT_GRID_CELL_LEFT_DCLICK" },
    { "OnGridCmdCellRightClick", "wxEVT_GRID_CELL_RIGHT_CLICK" },
    { "OnGridCmdCellRightDClick", "wxEVT_GRID_CELL_RIGHT_DCLICK" },
    { "OnGridCmdColSize", "wxEVT_GRID_COL_SIZE" },
    { "OnGridCmdEditorCreated", "wxEVT_GRID_EDITOR_CREATED" },
    { "OnGridCmdEditorHidden", "wxEVT_GRID_EDITOR_HIDDEN" },
    { "OnGridCmdEditorShown", "wxEVT_GRID_EDITOR_SHOWN" },
    { "OnGridCmdLabelLeftClick", "wxEVT_GRID_LABEL_LEFT_CLICK" },
    { "OnGridCmdLabelLeftDClick", "wxEVT_GRID_LABEL_LEFT_DCLICK" },
    { "OnGridCmdLabelRightClick", "wxEVT_GRID_LABEL_RIGHT_CLICK" },
    { "OnGridCmdLabelRightDClick", "wxEVT_GRID_LABEL_RIGHT_DCLICK" },
    { "OnGridCmdRangeSelect", "wxEVT_GRID_RANGE_SELECT" },
    { "OnGridCmdRowSize", "wxEVT_GRID_ROW_SIZE" },
    { "OnGridCmdSelectCell", "wxEVT_GRID_SELECT_CELL" },
    { "OnGridColSize", "wxEVT_GRID_COL_SIZE" },
    { "OnGridEditorCreated", "wxEVT_GRID_EDITOR_CREATED" },
    { "OnGridEditorHidden", "wxEVT_GRID_EDITOR_HIDDEN" },
    { "OnGridEditorShown", "wxEVT_GRID_EDITOR_SHOWN" },
    { "OnGridLabelLeftClick", "wxEVT_GRID_LABEL_LEFT_CLICK" },
    { "OnGridLabelLeftDClick", "wxEVT_GRID_LABEL_LEFT_DCLICK" },
    { "OnGridLabelRightClick", "wxEVT_GRID_LABEL_RIGHT_CLICK" },
    { "OnGridLabelRightDClick", "wxEVT_GRID_LABEL_RIGHT_DCLICK" },
    { "OnGridRangeSelect", "wxEVT_GRID_RANGE_SELECT" },
    { "OnGridRowSize", "wxEVT_GRID_ROW_SIZE" },
    { "OnGridSelectCell", "wxEVT_GRID_SELECT_CELL" },
    { "OnHtmlCellClicked", "wxEVT_HTML_CELL_CLICKED" },
    { "OnHtmlCellHover", "wxEVT_HTML_CELL_HOVER" },
    { "OnHtmlLinkClicked", "wxEVT_HTML_LINK_CLICKED" },
    { "OnHyperlink", "wxEVT_HYPERLINK" },
    { "OnIconize", "wxEVT_ICONIZE" },
    { "OnIdle", "wxEVT_IDLE" },
    { "OnInitDialog", "wxEVT_INIT_DIALOG" },
    { "OnKeyDown", "wxEVT_KEY_DOWN" },
    { "OnKeyUp", "wxEVT_KEY_UP" },
    { "OnKillFocus", "wxEVT_KILL_FOCUS" },
    { "OnLeaveWindow", "wxEVT_LEAVE_WINDOW" },
    { "OnLeftDClick", "wxEVT_LEFT_DCLICK" },
    { "OnLeftDown", "wxEVT_LEFT_DOWN" },
    { "OnLeftUp", "wxEVT_LEFT_UP" },
    { "OnListBeginDrag", "wxEVT_LIST_BEGIN_DRAG" },
    { "OnListBeginLabelEdit", "wxEVT_LIST_BEGIN_LABEL_EDIT" },
    { "OnListBeginRDrag", "wxEVT_LIST_BEGIN_RDRAG" },
    { "OnListBox", "wxEVT_LISTBOX" },
    { "OnListBoxDClick", "wxEVT_LISTBOX_DCLICK" },
    { "OnListCacheHint", "wxEVT_LIST_CACHE_HINT" },
    { "OnListColBeginDrag", "wxEVT_LIST_COL_BEGIN_DRAG" },
    { "OnListColClick", "wxEVT_LIST_COL_CLICK" },
    { "OnListColDragging", "wxEVT_LIST_COL_DRAGGING" },
    { "OnListColEndDrag", "wxEVT_LIST_COL_END_DRAG" },
    { "OnListColRightClick", "wxEVT_LIST_COL_RIGHT_CLICK" },
    { "OnListDeleteAllItems", "wxEVT_LIST_DELETE_ALL_ITEMS" },
    { "OnListDeleteItem", "wxEVT_LIST_DELETE_ITEM" },
    { "OnListEndLabelEdit", "wxEVT_LIST_END_LABEL_EDIT" },
    { "OnListInsertItem", "wxEVT_LIST_INSERT_ITEM" },
    { "OnListItemActivated", "wxEVT_LIST_ITEM_ACTIVATED" },
    { "OnListItemDeselected", "wxEVT_LIST_ITEM_DESELECTED" },
    { "OnListItemFocused", "wxEVT_LIST_ITEM_FOCUSED" },
    { "OnListItemMiddleClick", "wxEVT_LIST_ITEM_MIDDLE_CLICK" },
    { "OnListItemRightClick", "wxEVT_LIST_ITEM_RIGHT_CLICK" },
    { "OnListItemSelected", "wxEVT_LIST_ITEM_SELECTED" },
    { "OnListKeyDown", "wxEVT_LIST_KEY_DOWN" },
    { "OnMaximize", "wxEVT_MAXIMIZE" },
    { "OnMenuSelection", "wxEVT_MENU" },
    { "OnMiddleDClick", "wxEVT_MIDDLE_DCLICK" },
    { "OnMiddleDown", "wxEVT_MIDDLE_DOWN" },
    { "OnMiddleUp", "wxEVT_MIDDLE_UP" },
    { "OnMotion", "wxEVT_MOTION" },
    { "OnMouseWheel", "wxEVT_MOUSEWHEEL" },
    { "OnMove", "wxEVT_MOVE" },
    { "OnMove", "wxEVT_MOVE" },
    { "OnMoveEnd", "wxEVT_MOVE_END" },
    { "OnMoveStart", "wxEVT_MOVE_START" },
    { "OnMoving", "wxEVT_MOVING" },
    { "OnPaint", "wxEVT_PAINT" },
    { "OnPropertyGridChanged", "wxEVT_PG_CHANGED" },
    { "OnPropertyGridChanging", "wxEVT_PG_CHANGING" },
    { "OnRadioBox", "wxEVT_RADIOBOX" },
    { "OnRadioButton", "wxEVT_RADIOBUTTON" },
    { "OnRibbonBarHelpClick", "wxEVT_RIBBONBAR_HELP_CLICKED" },
    { "OnRibbonBarPageChanged", "wxEVT_RIBBONBAR_PAGE_CHANGED" },
    { "OnRibbonBarPageChanging", "wxEVT_RIBBONBAR_PAGE_CHANGING" },
    { "OnRibbonBarTabLeftDClick", "wxEVT_RIBBONBAR_TAB_LEFT_DCLICK" },
    { "OnRibbonBarTabMiddleDown", "wxEVT_RIBBONBAR_TAB_MIDDLE_DOWN" },
    { "OnRibbonBarTabMiddleUp", "wxEVT_RIBBONBAR_TAB_MIDDLE_UP" },
    { "OnRibbonBarTabRightDown", "wxEVT_RIBBONBAR_TAB_RIGHT_DOWN" },
    { "OnRibbonBarTabRightUp", "wxEVT_RIBBONBAR_TAB_RIGHT_UP" },
    { "OnRibbonBarToggled", "wxEVT_RIBBONBAR_TOGGLED" },
    { "OnRibbonButtonClicked", "wxEVT_RIBBONBUTTON_CLICKED" },
    { "OnRibbonButtonClicked", "wxEVT_RIBBONBUTTON_CLICKED" },
    { "OnRibbonButtonDropdownClicked", "wxEVT_RIBBONBUTTON_DROPDOWN_CLICKED" },
    { "OnRibbonGalleryClicked", "wxEVT_RIBBONGALLERY_CLICKED" },
    { "OnRibbonGalleryExtensionButtonPressed", "wxEVT_BUTTON" },
    { "OnRibbonGalleryHoverChanged", "wxEVT_RIBBONGALLERY_HOVER_CHANGED" },
    { "OnRibbonGallerySelected", "wxEVT_RIBBONGALLERY_SELECTED" },
    { "OnRibbonPanelExtbuttonActivated", "wxEVT_RIBBONPANEL_EXTBUTTON_ACTIVATED" },
    { "OnRibbonToolClicked", "wxEVT_RIBBONTOOL_DROPDOWN_CLICKED" },
    { "OnRibbonToolDropdownClicked", "wxEVT_RIBBONTOOL_DROPDOWN_CLICKED" },
    { "OnRichTextCharacter", "wxEVT_RICHTEXT_CHARACTER" },
    { "OnRichTextContentDeleted", "wxEVT_RICHTEXT_CONTENT_DELETED" },
    { "OnRichTextContentInserted", "wxEVT_RICHTEXT_CONTENT_INSERTED" },
    { "OnRichTextDelete", "wxEVT_RICHTEXT_DELETE" },
    { "OnRichTextReturn", "wxEVT_RICHTEXT_RETURN" },
    { "OnRichTextStyleChanged", "wxEVT_RICHTEXT_STYLE_CHANGED" },
    { "OnRichTextStyleSheetChanged", "wxEVT_RICHTEXT_STYLESHEET_CHANGED" },
    { "OnRichTextStyleSheetReplaced", "wxEVT_RICHTEXT_STYLESHEET_REPLACED" },
    { "OnRichTextStyleSheetReplacing", "wxEVT_RICHTEXT_STYLESHEET_REPLACING" },
    { "OnRightDClick", "wxEVT_RIGHT_DCLICK" },
    { "OnRightDown", "wxEVT_RIGHT_DOWN" },
    { "OnRightUp", "wxEVT_RIGHT_UP" },
    { "OnScroll", "AllScrollEvents" },
    { "OnScrollBottom", "wxEVT_SCROLL_BOTTOM" },
    { "OnScrollChanged", "wxEVT_SCROLL_CHANGED" },
    { "OnScrollLineDown", "wxEVT_SCROLL_LINEDOWN" },
    { "OnScrollLineUp", "wxEVT_SCROLL_LINEUP" },
    { "OnScrollPageDown", "wxEVT_SCROLL_PAGEDOWN" },
    { "OnScrollPageUp", "wxEVT_SCROLL_PAGEUP" },
    { "OnScrollThumbRelease", "wxEVT_SCROLL_THUMBRELEASE" },
    { "OnScrollThumbTrack", "wxEVT_SCROLL_THUMBTRACK" },
    { "OnScrollTop", "wxEVT_SCROLL_TOP" },
    { "OnSearchButton", "wxEVT_SEARCHCTRL_SEARCH_BTN" },
    { "OnSetFocus", "wxEVT_SET_FOCUS" },
    { "OnShow", "wxEVT_SHOW" },
    { "OnSize", "wxEVT_SIZE" },
    { "OnSlider", "wxEVT_SLIDER" },
    { "OnSpin", "wxEVT_SPIN" },
    { "OnSpinCtrl", "wxEVT_SPINCTRL" },
    { "OnSpinCtrlDouble", "wxEVT_SPINCTRLDOUBLE" },
    { "OnSpinCtrlText", "wxEVT_TEXT" },
    { "OnSpinDown", "wxEVT_SPIN_DOWN" },
    { "OnSpinUp", "wxEVT_SPIN_UP" },
    { "OnText", "wxEVT_TEXT" },
    { "OnTextEnter", "wxEVT_TEXT_ENTER" },
    { "OnTextMaxLen", "wxEVT_TEXT_MAXLEN" },
    { "OnTextURL", "wxEVT_TEXT_URL" },
    { "OnTimeChanged", "wxEVT_TIME_CHANGED" },
    { "OnTimer", "wxEVT_TIMER" },
    { "OnToggleButton", "wxEVT_TOGGLEBUTTON" },
    { "OnToolClicked", "wxEVT_TOOL" },
    { "OnToolDropdown", "wxEVT_TOOL_DROPDOWN" },
    { "OnToolEnter", "wxEVT_TOOL_ENTER" },
    { "OnToolRClicked", "wxEVT_TOOL_RCLICKED" },
    { "OnTreeBeginDrag", "wxEVT_TREE_BEGIN_DRAG" },
    { "OnTreeBeginLabelEdit", "wxEVT_TREE_BEGIN_LABEL_EDIT" },
    { "OnTreeBeginRDrag", "wxEVT_TREE_BEGIN_RDRAG" },
    { "OnTreeDeleteItem", "wxEVT_TREE_DELETE_ITEM" },
    { "OnTreeEndDrag", "wxEVT_TREE_END_DRAG" },
    { "OnTreeEndLabelEdit", "wxEVT_TREE_END_LABEL_EDIT" },
    { "OnTreeGetInfo", "wxEVT_TREE_GET_INFO" },
    { "OnTreeItemActivated", "wxEVT_TREE_ITEM_ACTIVATED" },
    { "OnTreeItemCollapsed", "wxEVT_TREE_ITEM_COLLAPSED" },
    { "OnTreeItemCollapsing", "wxEVT_TREE_ITEM_COLLAPSING" },
    { "OnTreeItemExpanded", "wxEVT_TREE_ITEM_EXPANDED" },
    { "OnTreeItemExpanding", "wxEVT_TREE_ITEM_EXPANDING" },
    { "OnTreeItemGetTooltip", "wxEVT_TREE_ITEM_GETTOOLTIP" },
    { "OnTreeItemMenu", "wxEVT_TREE_ITEM_MENU" },
    { "OnTreeItemMiddleClick", "wxEVT_TREE_ITEM_MIDDLE_CLICK" },
    { "OnTreeItemRightClick", "wxEVT_TREE_ITEM_RIGHT_CLICK" },
    { "OnTreeKeyDown", "wxEVT_TREE_KEY_DOWN" },
    { "OnTreeSelChanged", "wxEVT_TREE_SEL_CHANGED" },
    { "OnTreeSelChanging", "wxEVT_TREE_SEL_CHANGING" },
    { "OnTreeSetInfo", "wxEVT_TREE_SET_INFO" },
    { "OnTreeStateImageClick", "wxEVT_TREE_STATE_IMAGE_CLICK" },
    { "OnTreelistColumnSorted", "wxEVT_TREELIST_COLUMN_SORTED" },
    { "OnTreelistItemActivated", "wxEVT_TREELIST_ITEM_ACTIVATED" },
    { "OnTreelistItemChecked", "wxEVT_TREELIST_ITEM_CHECKED" },
    { "OnTreelistItemContextMenu", "wxEVT_TREELIST_ITEM_CONTEXT_MENU" },
    { "OnTreelistItemExpanded", "wxEVT_TREELIST_ITEM_EXPANDED" },
    { "OnTreelistItemExpanding", "wxEVT_TREELIST_ITEM_EXPANDING" },
    { "OnTreelistSelectionChanged", "wxEVT_TREELIST_SELECTION_CHANGED" },
    { "OnUpdateUI", "wxEVT_UPDATE_UI" },
    { "OnUpdateUI", "wxEVT_UPDATE_UI" },
    { "OnWizardBeforePageChanged", "wxEVT_WIZARD_BEFORE_PAGE_CHANGED" },
    { "OnWizardCancel", "wxEVT_WIZARD_CANCEL" },
    { "OnWizardFinished", "wxEVT_WIZARD_FINISHED" },
    { "OnWizardHelp", "wxEVT_WIZARD_HELP" },
    { "OnWizardPageChanged", "wxEVT_WIZARD_PAGE_CHANGED" },
    { "OnWizardPageChanging", "wxEVT_WIZARD_PAGE_CHANGING" },
    { "OnWizardPageShown", "wxEVT_WIZARD_PAGE_SHOWN" },

    { "OnApplyButtonClick", "ApplyButtonClicked" },
    { "OnCancelButtonClick", "CancelButtonClicked" },
    { "OnContextHelpButtonClick", "ContextHelpButtonClicked" },
    { "OnHelpButtonClick", "HelpButtonClicked" },
    { "OnNoButtonClick", "NoButtonClicked" },
    { "OnOKButtonClick", "OKButtonClicked" },
    { "OnSaveButtonClick", "SaveButtonClicked" },
    { "OnYesButtonClick", "YesButtonClicked" },

};

NodeSharedPtr LoadOldProject(pugi::xml_document& doc)
{
    NodeSharedPtr project;
    try
    {
        auto root = doc.first_child();
        if (!root)
        {
            FAIL_MSG("Project does not have a root child.");
            throw std::runtime_error("Invalid project file");
        }

        auto object = root.child("object");
        if (!object)
        {
            FAIL_MSG("Project does not have a \"object\" node.");
            throw std::runtime_error("Invalid project file");
        }
        OldProject old_project;
        project = old_project.CreateOldProjectNode(object);
    }

    catch (const std::exception& DBG_PARAM(e))
    {
        MSG_ERROR(e.what());
        appMsgBox(_tt("This wxUiEditor project file is invalid and cannot be loaded."), _tt(strIdTitleLoadProject));
    }

    return project;
}

NodeSharedPtr OldProject::CreateOldProjectNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem)
{
    auto class_name = xml_obj.attribute("class").as_cview();
    if (class_name.empty())
        return NodeSharedPtr();

    // This section of code is used to replace class names of down-level projects

    if (class_name.is_sameas("grid_bag_cell"))
    {
        class_name = "gbsizeritem";
    }
    else if (class_name.is_sameas("wxPanel") && parent->GetClassName().contains("book"))
    {
        class_name = "BookPage";
    }
    else if (class_name.contains("bookpage"))
    {
        class_name = "oldbookpage";
    }
    else if (class_name.is_sameas("wxListCtrl"))
        class_name = "wxListView";
    else if (class_name.is_sameas("Dialog"))
        class_name = "wxDialog";
    else if (class_name.is_sameas("Frame"))
        class_name = "wxFrame";
    else if (class_name.is_sameas("Wizard"))
        class_name = "wxWizard";
    else if (class_name.is_sameas("WizardPageSimple"))
        class_name = "wxWizardPageSimple";
    else if (class_name.is_sameas("Panel"))
        class_name = "PanelForm";

    auto newobject = g_NodeCreator.CreateNode(class_name, parent);
    if (!newobject)
    {
        FAIL_MSG(ttlib::cstr() << "Invalid project file: could not create " << class_name);
        throw std::runtime_error("Invalid project file");
    }

    auto xml_prop = xml_obj.child("property");
    while (xml_prop)
    {
        if (auto prop_name = xml_prop.attribute("name").as_cview(); prop_name.size())
        {
            // CheckProperty() will convert properties that have changed from older versions
            if (CheckProperty(xml_prop, newobject.get()))
            {
                xml_prop = xml_prop.next_sibling("property");
                continue;
            }
            else if (auto prop = newobject->get_prop_ptr(prop_name); prop)
            {
                if (prop->GetType() == Type::Bool)
                    prop->set_value(xml_prop.text().as_bool());
                else
                    prop->set_value(xml_prop.text().as_cview());
            }
            else
            {
                if (HandleDownLevelProperty(xml_prop, prop_name, class_name, newobject.get()))
                {
                    xml_prop = xml_prop.next_sibling("property");
                    continue;
                }

                if (auto value = xml_prop.text().as_string(); *value)
                {
                    if (prop_name.is_sameas("name"))
                    {
                        // This gets special-cased because "name" is still used in some places, just not as the class name
                        if (auto prop_varname = newobject.get()->get_prop_ptr(txtVarName); prop_varname)
                        {
                            prop_varname->set_value(value);
                            xml_prop = xml_prop.next_sibling("property");
                            continue;
                        }
                    }

                    // We get here if a property is specified that we don't recognize. While we can continue to load
                    // just fine, if the user attempts to save the project than the property will be lost.

                    // TODO: [KeyWorks - 06-03-2020] We need to store a list of unrecognized properties and display them to
                    // the user all at once after the project is completely loaded. We also need to flag the project file as
                    // unsaveable (only SaveAs can be used. See issue #69).

                    MSG_WARNING(ttlib::cstr("Unrecognized property: ") << prop_name << " in class: " << class_name);

                    if (!m_not_supported_warning)
                        appMsgBox(ttlib::cstr().Format(
                            "The property named \"%s\" of class \"%s\" is not supported by this version of wxUiEditor.\n"
                            "If your project file was just converted from an older version, then the conversion was not "
                            "complete.\n"
                            "Otherwise, this project is from a newer version of wxUiEditor.\n\n"
                            "The property's value is: %s\n"
                            "If you save this project, YOU WILL LOSE DATA",
                            prop_name.c_str(), class_name.c_str(), value));
                    m_not_supported_warning = true;
                }
            }
        }
        xml_prop = xml_prop.next_sibling("property");
    }

    if (class_name.is_sameas("VerticalBoxSizer"))
    {
        auto prop = newobject->get_prop_ptr(txtOrient);
        prop->set_value("wxVERTICAL");
    }

    auto xml_event = xml_obj.child("event");
    while (xml_event)
    {
        if (auto event_name = xml_event.attribute("name").as_cview(); event_name.size())
        {
            if (auto result = g_mapNewEventNames.find(event_name.c_str()); result != g_mapNewEventNames.end())
            {
                event_name = result->second;
            }
            if (auto event = newobject->GetEvent(event_name); event)
            {
                event->set_value(xml_event.text().as_cview());
            }
        }

        xml_event = xml_event.next_sibling("event");
    }

    auto child = xml_obj.child("object");
    if (g_NodeCreator.IsOldHostType(newobject->GetClassName()))
    {
        newobject = CreateOldProjectNode(child, parent, newobject.get());
        child = child.next_sibling("object");
    }
    else if (sizeritem)
    {
        for (auto& iter: sizeritem->get_props_vector())
        {
            if (iter.as_string() != iter.GetPropertyInfo()->GetDefaultValue())
            {
                auto prop_value = newobject->get_value_ptr(iter.GetPropertyInfo()->GetName());
                if (prop_value)
                    *prop_value = iter.as_string();
            }
        }
        parent->AddChild(newobject);
        newobject->SetParent(parent->GetSharedPtr());
    }
    else if (parent)
    {
        parent->AddChild(newobject);
        newobject->SetParent(parent->GetSharedPtr());
    }

    while (child)
    {
        CreateOldProjectNode(child, newobject.get());
        child = child.next_sibling("object");
    }

    return newobject;
}

static bool CheckProperty(pugi::xml_node& xml_prop, Node* node)
{
    if (node->GetClassName() == "wxCheckBox")
    {
        return ProcessCheckBox(xml_prop, node);
    }

    return false;
}

static bool ProcessCheckBox(pugi::xml_node& xml_prop, Node* node)
{
    auto prop_name = xml_prop.attribute("name").as_cview();

    // wxCHK_2STATE and wxCHK_3STATE are now part of the type property instead of style
    if (prop_name.is_sameas(txtStyle))
    {
        ttlib::multistr styles(xml_prop.text().as_string(), '|');
        ttlib::cstr new_style;
        for (auto& iter: styles)
        {
            if (iter.is_sameas("wxCHK_2STATE"))
                continue;  // this is the default, so ignore it
            else if (iter.is_sameas("wxCHK_3STATE"))
            {
                auto prop = node->get_prop_ptr("type");
                prop->set_value("wxCHK_3STATE");
            }
            else
            {
                if (new_style.size())
                    new_style += "|";
                new_style += iter;
            }
        }

        if (new_style.size())
        {
            auto prop = node->get_prop_ptr(txtStyle);
            prop->set_value(new_style);
        }
        return true;
    }

    // If we get here, than process normally
    return false;
}

static bool HandleDownLevelProperty(pugi::xml_node& xml_prop, ttlib::cview prop_name, ttlib::cview class_name, Node* node)
{
    if (prop_name.is_sameas("event_handler"))
    {
        // This property is now ignored
        return true;
    }

    if (prop_name.is_sameas("derived_name"))
    {
        auto prop = node->get_prop_ptr(txtDerivedClassName);
        prop->set_value(xml_prop.text().as_cview());
        return true;
    }

    if (prop_name.is_sameas("orient"))
    {
        auto prop = node->get_prop_ptr(txtOrient);
        prop->set_value(xml_prop.text().as_cview());
        return true;
    }

    if (prop_name.is_sameas("access"))
    {
        auto prop = node->get_prop_ptr(txtAccess);
        prop->set_value(xml_prop.text().as_cview());
        return true;
    }

    if (prop_name.is_sameas("name"))
    {
        if (node->IsForm())
        {
            auto prop = node->get_prop_ptr(txtClassName);
            prop->set_value(xml_prop.text().as_cview());
            return true;
        }
    }

    if (prop_name.is_sameas("flag") && (class_name.is_sameas("sizeritem") || class_name.is_sameas("gbsizeritem")))
    {
        auto flag_value = xml_prop.text().as_cview();

        ttlib::cstr border_value;
        if (flag_value.contains("wxALL"))
            border_value = "wxALL";
        else
        {
            if (flag_value.contains("wxLEFT"))
            {
                if (border_value.size())
                    border_value << '|';
                border_value << "wxLEFT";
            }
            if (flag_value.contains("wxRIGHT"))
            {
                if (border_value.size())
                    border_value << '|';
                border_value << "wxRIGHT";
            }
            if (flag_value.contains("wxTOP"))
            {
                if (border_value.size())
                    border_value << '|';
                border_value << "wxTOP";
            }
            if (flag_value.contains("wxBOTTOM"))
            {
                if (border_value.size())
                    border_value << '|';
                border_value << "wxBOTTOM";
            }
        }

        if (border_value.size())
        {
            auto prop = node->get_prop_ptr(txtBorders);
            prop->set_value(border_value);
        }

        ttlib::cstr align_value;
        if (flag_value.contains("wxALIGN_LEFT"))
        {
            align_value << "wxALIGN_LEFT";
        }
        if (flag_value.contains("wxALIGN_TOP"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_TOP";
        }
        if (flag_value.contains("wxALIGN_RIGHT"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_RIGHT";
        }
        if (flag_value.contains("wxALIGN_BOTTOM"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_BOTTOM";
        }

        if (flag_value.contains("wxALIGN_CENTER") || flag_value.contains("wxALIGN_CENTRE"))
        {
            if (flag_value.contains("wxALIGN_CENTER_VERTICAL") && flag_value.contains("wxALIGN_CENTRE_VERTICAL"))
            {
                if (align_value.size())
                    align_value << '|';
                align_value << "wxALIGN_CENTER_VERTICAL";
            }
            else if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") || flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
            {
                if (align_value.size())
                    align_value << '|';
                align_value << "wxALIGN_CENTER_HORIZONTAL";
            }
            if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") || flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
            {
                if (align_value.size())
                    align_value << '|';
                align_value << "wxALIGN_CENTER_HORIZONTAL";
            }

            // Because we use contains(), all we know is that a CENTER flag was used, but not which one.
            // If we get here and no CENTER flag has been added, then assume that "wxALIGN_CENTER" or
            // "wxALIGN_CENTRE" was specified.

            if (!align_value.contains("wxALIGN_CENTER"))
            {
                if (align_value.size())
                    align_value << '|';
                align_value << "wxALIGN_CENTER";
            }
        }
        if (align_value.size())
        {
            auto prop = node->get_prop_ptr(txtAlignment);
            prop->set_value(align_value);
        }

        ttlib::cstr flags_value;
        if (flag_value.contains("wxEXPAND"))
        {
            flags_value << "wxEXPAND";
        }
        if (flag_value.contains("wxSHAPED"))
        {
            if (flags_value.size())
                flags_value << '|';
            flags_value << "wxSHAPED";
        }
        if (flag_value.contains("wxFIXED_MINSIZE"))
        {
            if (flags_value.size())
                flags_value << '|';
            flags_value << "wxFIXED_MINSIZE";
        }
        if (flag_value.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
        {
            if (flags_value.size())
                flags_value << '|';
            flags_value << "wxRESERVE_SPACE_EVEN_IF_HIDDEN";
        }
        if (flags_value.size())
        {
            auto prop = node->get_prop_ptr(txtFlags);
            prop->set_value(flags_value);
        }

        return true;
    }

    if (prop_name.is_sameas("border"))
    {
        auto prop = node->get_prop_ptr(txtBorderSize);
        prop->set_value(xml_prop.text().as_cview());

        return true;
    }

    if (prop_name.is_sameas(txtValue) && class_name.is_sameas("wxComboBox"))
    {
        auto prop = node->get_prop_ptr("selection_string");
        prop->set_value(xml_prop.text().as_cview());

        return true;
    }
    else if (prop_name.is_sameas("selection") && (class_name.is_sameas("wxComboBox") || class_name.is_sameas("wxChoice")))
    {
        auto prop = node->get_prop_ptr("selection_int");
        prop->set_value(xml_prop.text().as_cview());

        return true;
    }
    return false;
}
