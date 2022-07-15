/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert image to Header (.h) or XPM (.xpm) file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "internal/convert_img_base.h"  // ConvertImageBase

class ConvertImageDlg : public ConvertImageBase
{
public:
    ConvertImageDlg(wxWindow* parent = nullptr);

protected:
    bool IsHeaderPage() { return (m_choicebook->GetChoiceCtrl()->GetSelection() != 1); };
    bool IsXpmPage() { return (m_choicebook->GetChoiceCtrl()->GetSelection() == 1); };

    // Call this to re-enable the convert button
    void EnableConvertButton();
    void SetSizeLabel();
    void AdjustOutputFilename();

    // Handlers for EmbedImageBase events.

    void OnCheckPngConversion(wxCommandEvent& event) override;
    void OnConvert(wxCommandEvent& event) override;
    void OnConvertAlpha(wxCommandEvent& event) override;
    void OnForceHdrMask(wxCommandEvent& event) override;
    void OnForceXpmMask(wxCommandEvent& event) override;
    void OnInputChange(wxFileDirPickerEvent& event) override;
    void OnOutputChange(wxFileDirPickerEvent& event) override;
    void OnPageChanged(wxBookCtrlEvent& event) override;

    // if force is set, this will update src bitmap display and re-enable Convert btn
    void OnComboHdrMask(wxCommandEvent& event) override;

    // if force is set, this will update src bitmap display and re-enable Convert btn
    void OnComboXpmMask(wxCommandEvent& event) override;

    void ImgageInHeaderOut();
    void ImageInXpmOut();

    void SetOutputBitmap();

    // If current transparency is anything other than "none" or "custom" then this will set the
    // mask color in the image to the specified color.
    wxColor GetHdrTransparencyColor();

    // If current transparency is anything other than "none" or "custom" then this will set the
    // mask color in the image to the specified color.
    wxColor GetXpmTransparencyColor();

private:
    ttString m_cwd;
    ttString m_lastInputFile;
    ttString m_lastOutputFile;
    wxImage m_xpmImage;
    wxImage m_hdrImage;
    wxImage m_orgImage;

    size_t m_orginal_size;

    ttString m_mime_type;  // mime string specifying the original image type
};
