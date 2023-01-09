import sys

sys.path.append('python')

import wx
import mainframe

class MyApp(wx.App):
    def OnInit(self):
        frame = mainframe.MainFrame(None)
        self.SetTopWindow(frame)

        frame.Show(True)
        return True

app = MyApp()
app.MainLoop()
