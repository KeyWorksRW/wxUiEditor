/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert image to Header (.h) or XPM (.xpm) file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
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
    // Handlers for EmbedImageBase events.

    void OnConvert(wxCommandEvent& event) override;
    void OnConvertAlpha(wxCommandEvent& event) override;
    void OnForceMask(wxCommandEvent& event) override;
    void OnInputChange(wxFileDirPickerEvent& event) override;
    void OnMask(wxCommandEvent& event) override;
    void OnOutputChange(wxFileDirPickerEvent& event) override;

    void OnHeaderOutput(wxCommandEvent& event) override;
    void OnXpmOutput(wxCommandEvent& event) override;

    void ImgageInHeaderOut();
    void ImageInXpmOut();

    void SetOutputBitmap();

    // If current transparency is anything other than "none" or "custom" then this will set the
    // mask color in the image to the specified color.
    wxColor GetTransparencyColor();

private:
    ttString m_cwd;
    ttString m_lastInputFile;
    ttString m_lastOutputFile;
    wxImage m_curImage;
    wxImage m_orgImage;
};
