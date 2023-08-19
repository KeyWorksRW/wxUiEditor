###############################################################################
# Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
#
# Do not edit any code above the "End of generated code" comment block.
# Any changes before that block will be lost if it is re-generated!
###############################################################################

"""Wizard class generated by wxUiEditor."""

import wx
import wx.adv

import images

class Wizard(wx.adv.Wizard):

    def __init__(self, parent, id=wx.ID_ANY, title="Wizard Tests", pos=
                wx.DefaultPosition, style=wx.DEFAULT_DIALOG_STYLE):
        wx.adv.Wizard.__init__(self)

        self.SetExtraStyle(self.GetExtraStyle() | wx.adv.WIZARD_EX_HELPBUTTON)
        self.SetBorder(15)
        if not self.Create(parent, id, title, wx.BitmapBundle.FromBitmap(
            images.wiztest_png.Bitmap), pos, style):
            return

        wizPage = wx.adv.WizardPageSimple(self)

        box_sizer = wx.BoxSizer(wx.VERTICAL)

        self.staticText = wx.StaticText(wizPage, wx.ID_ANY,
            "This is the first Wizard page")
        box_sizer.Add(self.staticText, wx.SizerFlags().Border(wx.ALL))

        self.calendar = wx.adv.CalendarCtrl(wizPage, wx.ID_ANY, wx.DefaultDateTime,
            wx.DefaultPosition, wx.DefaultSize, wx.adv.CAL_SHOW_HOLIDAYS)
        box_sizer.Add(self.calendar, wx.SizerFlags().Border(wx.ALL))
        wizPage.SetSizerAndFit(box_sizer)

        m_wizPage2 = wx.adv.WizardPageSimple(self)

        box_sizer2 = wx.BoxSizer(wx.VERTICAL)

        self.staticText2 = wx.StaticText(wizPage2, wx.ID_ANY,
            "This is the second Wizard page which is wider.")
        box_sizer2.Add(self.staticText2, wx.SizerFlags().Border(wx.ALL))

        parent_sizer3 = wx.BoxSizer(wx.VERTICAL)

        box_sizer_2 = wx.BoxSizer(wx.HORIZONTAL)

        self.staticText = wx.StaticText(wizPage2, wx.ID_ANY, "Scrollbar:")
        box_sizer_2.Add(self.staticText, wx.SizerFlags().Border(wx.ALL))

        self.scrollBar = wx.ScrollBar(wizPage2, wx.ID_ANY, wx.DefaultPosition,
            wx.DefaultSize, wx.SB_HORIZONTAL)
        self.scrollBar.SetScrollbar(0, 1, 100, 1)
        box_sizer_2.Add(self.scrollBar, wx.SizerFlags(1).Expand().Border(wx.ALL))

        parent_sizer3.Add(box_sizer_2, wx.SizerFlags().Border(wx.ALL))

        box_sizer_3 = wx.BoxSizer(wx.HORIZONTAL)

        self.staticText_2 = wx.StaticText(wizPage2, wx.ID_ANY, "Normal SpinCtrl")
        box_sizer_3.Add(self.staticText_2, wx.SizerFlags().Border(wx.ALL))

        self.spinCtrl = wx.SpinCtrl(wizPage2, wx.ID_ANY, wx.EmptyString,
            wx.DefaultPosition, wx.DefaultSize, wx.SP_ARROW_KEYS, 0, 100, 4)
        box_sizer_3.Add(self.spinCtrl, wx.SizerFlags().Border(wx.ALL))

        parent_sizer3.Add(box_sizer_3, wx.SizerFlags().Border(wx.ALL))

        box_sizer_4 = wx.BoxSizer(wx.HORIZONTAL)

        self.staticText_3 = wx.StaticText(wizPage2, wx.ID_ANY, "Double SpinCtrl")
        box_sizer_4.Add(self.staticText_3, wx.SizerFlags().Border(wx.ALL))

        self.spinCtrlDouble = wx.SpinCtrlDouble(wizPage2)
        box_sizer_4.Add(self.spinCtrlDouble, wx.SizerFlags().Border(wx.ALL))

        parent_sizer3.Add(box_sizer_4, wx.SizerFlags().Border(wx.ALL))

        box_sizer_5 = wx.BoxSizer(wx.HORIZONTAL)

        self.staticText_4 = wx.StaticText(wizPage2, wx.ID_ANY, "Spin Button")
        box_sizer_5.Add(self.staticText_4, wx.SizerFlags().Border(wx.ALL))

        self.spinBtn = wx.SpinButton(wizPage2, wx.ID_ANY)
        self.spinBtn.SetRange(0, 10)
        box_sizer_5.Add(self.spinBtn, wx.SizerFlags().Border(wx.ALL))

        parent_sizer3.Add(box_sizer_5, wx.SizerFlags().Border(wx.ALL))

        box_sizer2.Add(parent_sizer3, wx.SizerFlags().Border(wx.ALL))
        wizPage2.SetSizerAndFit(box_sizer2)

        m_wizPage3 = wx.adv.WizardPageSimple(self, None, None, wx.BitmapBundle.FromBitmap(
            images.wiztest2_png.Bitmap))

        box_sizer3 = wx.BoxSizer(wx.VERTICAL)

        self.staticText3 = wx.StaticText(wizPage3, wx.ID_ANY,
            "This is the final Wizard page")
        box_sizer3.Add(self.staticText3, wx.SizerFlags().Border(wx.ALL))

        box_sizer_6 = wx.BoxSizer(wx.HORIZONTAL)

        self.hyperlink = wx.adv.HyperlinkCtrl(wizPage3, wx.ID_ANY, "Blank Page",
            "https://blank.page/")
        box_sizer_6.Add(self.hyperlink, wx.SizerFlags().Center().Border(wx.ALL))

        self.searchCtrl = wx.SearchCtrl(wizPage3, wx.ID_ANY, "")
        self.searchCtrl.SetHint("Search for something...")
        self.searchCtrl.ShowSearchButton(True)
        self.searchCtrl.ShowCancelButton(True)
        box_sizer_6.Add(self.searchCtrl, wx.SizerFlags(1).Border(wx.ALL))

        box_sizer3.Add(box_sizer_6, wx.SizerFlags().Expand().Border(wx.ALL))
        wizPage3.SetSizerAndFit(box_sizer3)

        wizPage.Chain(m_wizPage2).Chain(m_wizPage3)
        self.GetPageAreaSizer().Add(wizPage)
        self.Center(wx.BOTH)

        # Bind Event handlers
        self.Bind(wx.EVT_INIT_DIALOG, self.OnInit)
        self.Bind(wx.adv.EVT_WIZARD_BEFORE_PAGE_CHANGED, self.OnBeforeChange)

    # Unimplemented Event handler functions
    # Copy any listed and paste them below the comment block, or to your inherited class.
    """
    """


    # Add the following below the comment block to add a simple
    # Run() function to launch the wizard
    """
    def Run(self):
        return self.RunWizard(self.GetPageAreaSizer().GetItem(0).GetWindow())
    """

# ************* End of generated code ***********
# DO NOT EDIT THIS COMMENT BLOCK!
#
# Code below this comment block will be preserved
# if the code for this class is re-generated.
# ***********************************************

    def OnInit(self, event):
        event.Skip()

    def OnBeforeChange(self, event):
        event.Skip()

    def Run(self):
        return self.RunWizard(self.GetPageAreaSizer().GetItem(0).GetWindow())
