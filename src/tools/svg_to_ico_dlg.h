#pragma once  // NOLINT(#pragma once in main file)

#include <wx/string.h>

#include "svg_to_ico_dlg_base.h"

class wxCommandEvent;
class wxFileDirPickerEvent;
class wxInitDialogEvent;

// Writes a Windows .ico file containing 16x16, 32x32, 48x48 and 256x256 PNG-compressed
// images rendered from svg_path. Returns false and fills error_msg on failure.
[[nodiscard]] bool ConvertSvgToIco(const wxString& svg_path, const wxString& ico_path,
                                   wxString* error_msg = nullptr);

// Dialog for choosing the source SVG and the destination ICO. The controls live in the
// wxUiEditor-generated SvgToIcoDlgBase class (src/wxui/wxUiEditor.wxui).
class SvgToIcoDlg : public SvgToIcoDlgBase
{
public:
    explicit SvgToIcoDlg(wxWindow* parent);

protected:
    void OnClose(wxCommandEvent& event) override;
    void OnConvert(wxCommandEvent& event) override;
    void OnFileChanged(wxFileDirPickerEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;

private:
    void SetStatus(const wxString& message);
    void UpdateConvertState();
};
