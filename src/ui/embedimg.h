/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert image to Header (.h) or XPM (.xpm) file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ttstr.h>  // ttString -- wxString with additional methods similar to ttlib::cstr

#include "embedimg_base.h"

class EmbedImage : public EmbedImageBase
{
public:
    EmbedImage(wxWindow* parent = nullptr);

protected:
    void SetSizeLabel();
    void OnForceHdrMask(wxCommandEvent& event) override;
    void OnHdrMask(wxCommandEvent& event) override;
    void AdjustOutputFilename();

    // Handlers for EmbedImageBase events.

    void OnCheckPngConversion(wxCommandEvent& event) override;
    void OnConvert(wxCommandEvent& event) override;
    void OnConvertAlpha(wxCommandEvent& event) override;
    void OnForceXpmMask(wxCommandEvent& event) override;
    void OnInputChange(wxFileDirPickerEvent& event) override;
    void OnXpmMask(wxCommandEvent& event) override;
    void OnOutputChange(wxFileDirPickerEvent& event) override;

    void OnHeaderOutput(wxCommandEvent& event) override;
    void OnXpmOutput(wxCommandEvent& event) override;

    void ImgageInHeaderOut();
    void ImageInXpmOut();

    void SetOutputBitmap();

    // If current transparency is anything other than "none" or "custom" then this will set the
    // mask color in the image to the specified color.
    wxColor GetXpmTransparencyColor();
    wxColor GetHdrTransparencyColor();

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
