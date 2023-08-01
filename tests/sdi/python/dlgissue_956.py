###############################################################################
# Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
#
# Do not edit any code above the "End of generated code" comment block.
# Any changes before that block will be lost if it is re-generated!
###############################################################################

import wx

class DlgIssue_956(wx.Dialog):
    def __init__(self, parent, id=wx.ID_ANY, title="colspan codegen issue",
                pos=wx.DefaultPosition, size=wx.DefaultSize,
                style=wx.DEFAULT_DIALOG_STYLE, name=wx.DialogNameStr):
        wx.Dialog.__init__(self)

        if not self.Create(parent, id, title, pos, size, style, name):
            return

        dlg_sizer = wx.BoxSizer(wx.VERTICAL)

        panel_2 = wx.Panel(self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize,
            wx.BORDER_THEME|wx.TAB_TRAVERSAL)
        dlg_sizer.Add(panel_2, wx.SizerFlags().Expand().Border(wx.ALL))

        grid_bag_sizer = wx.GridBagSizer()

        self.staticText_4 = wx.StaticText(panel_2, wx.ID_ANY, "list test")
        grid_bag_sizer.Add(self.staticText_4, wx.GBPosition(0, 0), wx.GBSpan(1, 1), wx.ALL,
            5)

        self.choice = wx.Choice(panel_2, wx.ID_ANY)
        grid_bag_sizer.Add(self.choice, wx.GBPosition(0, 1), wx.GBSpan(1, 1),
            wx.ALL|wx.EXPAND, 5)

        self.btn = wx.Button(panel_2, wx.ID_ANY, wx.EmptyString)
        self.btn.SetLabelMarkup("&Refresh")
        grid_bag_sizer.Add(self.btn, wx.GBPosition(0, 2), wx.GBSpan(1, 1), wx.ALL, 5)

        self.btn_2 = wx.Button(panel_2, wx.ID_ANY, wx.EmptyString)
        self.btn_2.SetLabelMarkup("&New")
        grid_bag_sizer.Add(self.btn_2, wx.GBPosition(0, 3), wx.GBSpan(1, 1), wx.ALL, 5)

        self.btn_3 = wx.Button(panel_2, wx.ID_ANY, wx.EmptyString)
        self.btn_3.SetLabelMarkup("&Edit")
        grid_bag_sizer.Add(self.btn_3, wx.GBPosition(0, 4), wx.GBSpan(1, 1), wx.ALL, 5)

        self.btn_4 = wx.Button(panel_2, wx.ID_ANY, wx.EmptyString)
        self.btn_4.SetLabelMarkup("&Delete")
        grid_bag_sizer.Add(self.btn_4, wx.GBPosition(0, 5), wx.GBSpan(1, 1), wx.ALL, 5)

        self.staticText_5 = wx.StaticText(panel_2, wx.ID_ANY, "test type")
        grid_bag_sizer.Add(self.staticText_5, wx.GBPosition(1, 0), wx.GBSpan(1, 1), wx.ALL,
            5)

        self.choice_2 = wx.Choice(panel_2, wx.ID_ANY)
        grid_bag_sizer.Add(self.choice_2, wx.GBPosition(1, 1), wx.GBSpan(1, 5),
            wx.ALL|wx.EXPAND, 5)

        grid_bag_sizer.AddGrowableCol(1)
        grid_bag_sizer.AddGrowableRow(1)
        panel_2.SetSizerAndFit(grid_bag_sizer)

        if "wxMac" not in wx.PlatformInfo:
            stdBtn_line = wx.StaticLine(self, wx.ID_ANY, wx.DefaultPosition, wx.Size(20, -1))
            dlg_sizer.Add(stdBtn_line, wx.SizerFlags().Expand().Border(wx.ALL))

        stdBtn = wx.StdDialogButtonSizer()
        stdBtn_OK = wx.Button(self, wx.ID_OK)
        stdBtn.SetAffirmativeButton(stdBtn_OK)
        stdBtn_Cancel = wx.Button(self, wx.ID_CANCEL)
        stdBtn.SetCancelButton(stdBtn_Cancel)
        stdBtn_OK.SetDefault()
        stdBtn.Realize()
        dlg_sizer.Add(stdBtn, wx.SizerFlags().Expand().Border(wx.ALL))

        self.SetSizerAndFit(dlg_sizer)
        self.Centre(wx.BOTH)
# ************* End of generated code ***********
# DO NOT EDIT THIS COMMENT BLOCK!
#
# Code below this comment block will be preserved
# if the code for this class is re-generated.
# ***********************************************
