/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert image to Header (.h) or XPM (.xpm) file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <filesystem>
#include <fstream>

#include <wx/artprov.h>  // wxArtProvider class
#include <wx/mstream.h>  // Memory stream classes
#include <wx/utils.h>
#include <wx/wfstream.h>  // File stream classes

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "embedimg.h"  // auto-generated: embedimg_base.h and embedimg_base.cpp

#include "auto_freeze.h"  // AutoFreeze -- Automatically Freeze/Thaw a window
#include "bitmaps.h"      // Map of bitmaps accessed by name
#include "mainframe.h"    // MainFrame -- Main window frame
#include "node.h"         // Node class
#include "uifuncs.h"      // Miscellaneous functions for displaying UI
#include "utils.h"        // Utility functions that work with properties

#include "../xpm/empty.xpm"

// Any mime type in the following list with NOT be converted to PNG even if m_check_make_png is set to true

// clang-format off
static constexpr const char* lst_no_png_conversion[] = {

    "image/x-ani",
    "image/x-cur",
    "image/gif",
    "image/x-ico",
    "image/jpeg",

};
// clang-format on

bool isConvertibleMime(const ttString& suffix)
{
    for (auto& iter: lst_no_png_conversion)
    {
        if (suffix.is_sameas(iter))
            return false;
    }
    return true;
}

void MainFrame::OnEmbedImageConverter(wxCommandEvent& WXUNUSED(event))
{
    EmbedImage dlg(this);
    dlg.ShowModal();
}

EmbedImage::EmbedImage(wxWindow* parent) : EmbedImageBase(parent)
{
    m_cwd.assignCwd();

    ttString dir;
    auto dir_property = wxGetApp().GetProject()->prop_as_string(prop_original_art);
    if (dir_property.size())
        dir = dir_property;
    else
        dir = "./";
    dir.make_absolute();
    m_fileOriginal->SetInitialDirectory(dir);

    dir_property = wxGetApp().GetProject()->prop_as_string(prop_converted_art);
    if (dir_property.size())
        dir = dir_property;
    else
        dir = "./";
    dir.make_absolute();
    m_fileHeader->SetInitialDirectory(dir);

    m_btnClose->SetLabel(_tt("Close"));

    m_comboXpmMask->Append("none");
    m_comboXpmMask->Append("custom");

    m_comboXpmMask->Append("Black");
    m_comboXpmMask->Append("Dark Green");
    m_comboXpmMask->Append("Dark Grey");
    m_comboXpmMask->Append("Dark Slate Grey");
    m_comboXpmMask->Append("Dim Grey");
    m_comboXpmMask->Append("Grey");
    m_comboXpmMask->Append("Light Grey");
    m_comboXpmMask->Append("Grey94");
    m_comboXpmMask->Append("Magenta");
    m_comboXpmMask->Append("White");

    m_comboXpmMask->Append("upper left");
    m_comboXpmMask->Append("lower left");
    m_comboXpmMask->Append("upper right");
    m_comboXpmMask->Append("lower right");

    m_comboXpmMask->SetStringSelection("none");

    for (unsigned int index = 0; index < m_comboXpmMask->GetCount(); ++index)
    {
        m_comboHdrMask->Append(m_comboXpmMask->GetString(index));
    }
    m_comboHdrMask->SetStringSelection("none");

    m_staticSave->SetLabelText(wxEmptyString);
    m_staticSize->SetLabelText(wxEmptyString);
    m_staticDimensions->SetLabelText(wxEmptyString);

    // Disable conversion button until both input and out filenames have been set
    m_btnConvert->Disable();

    // Disable controls that require a valid input image
    m_ConvertAlphaChannel->Disable();
    m_comboHdrMask->Disable();
    m_ForceHdrMask->Disable();
    m_comboXpmMask->Disable();
    m_ForceXpmMask->Disable();

    // m_fileOriginal->GetTextCtrl()->SetEditable(false);

    Fit();
}

void EmbedImage::OnInputChange(wxFileDirPickerEvent& WXUNUSED(event))
{
    ttString file = m_fileOriginal->GetTextCtrlValue();

    if (!file.file_exists())
        return;

    file.make_relative_wx(m_cwd);
    file.backslashestoforward();
    m_fileOriginal->SetPath(file);

    m_staticSave->SetLabelText(wxEmptyString);
    m_staticSize->SetLabelText(wxEmptyString);
    m_staticSave->Hide();
    m_staticSize->Hide();
    m_staticDimensions->Hide();
    m_bmpOriginal->Hide();
    m_staticOriginal->Hide();

    m_orginal_size = 0;

    bool isImageLoaded { false };
    if (file.has_extension(".h") || file.has_extension(".hpp") || file.has_extension(".hh") || file.has_extension(".hxx"))
    {
        {
            wxBusyCursor wait;
            m_orgImage = GetHeaderImage(file.sub_cstr(), &m_orginal_size, &m_mime_type);
        }

        if (m_orgImage.IsOk())
        {
            isImageLoaded = true;

            // Note that we allow header to header conversion. That makes converting wxFormBuilder headers, and the options
            // png conversion and c++17.
        }
        else
        {
            appMsgBox(ttlib::cstr() << "Unrecognized file format in " << m_fileOriginal->GetTextCtrlValue().wx_str(),
                      "Header Image");
            m_fileOriginal->SetPath(wxEmptyString);
        }
    }

    else
    {
        m_radio_header->Enable();
        m_mime_type.clear();

        // We need to know what the original file type is because if we convert it to a header, then some file formats can be
        // converted to PNG before saving.

        wxFFileInputStream stream(m_fileOriginal->GetTextCtrlValue());
        if (stream.IsOk())
        {
            wxImageHandler* handler;
            auto& list = wxImage::GetHandlers();
            for (auto node = list.GetFirst(); node; node = node->GetNext())
            {
                handler = (wxImageHandler*) node->GetData();
                if (handler->CanRead(stream))
                {
                    m_mime_type = handler->GetMimeType();

                    if (handler->LoadFile(&m_orgImage, stream))
                    {
                        m_orginal_size = stream.GetSize();
                        isImageLoaded = true;
                        m_check_make_png->Show(isConvertibleMime(m_mime_type));
                        break;
                    }
                    else
                    {
                        appMsgBox(ttlib::cstr() << "Unable to read " << m_fileOriginal->GetTextCtrlValue().wx_str(),
                                  "Input Image");
                        break;
                    }
                }
            }
        }

        if (!isImageLoaded)
        {
            appMsgBox(ttlib::cstr() << "The file format in " << m_fileOriginal->GetTextCtrlValue().wx_str()
                                    << " is unsupported",
                      "Input Image");
            m_fileOriginal->SetPath(wxEmptyString);
        }
    }

    if (isImageLoaded)
    {
        m_hdrImage = m_orgImage.Copy();
        m_xpmImage = m_orgImage.Copy();

        m_comboHdrMask->Enable();
        m_ForceHdrMask->Enable();
        m_comboXpmMask->Enable();
        m_ForceXpmMask->Enable();

        m_ConvertAlphaChannel->Enable(m_xpmImage.HasAlpha());
        bool hasAlphaChannel = m_xpmImage.HasAlpha();
        if (hasAlphaChannel && m_ConvertAlphaChannel->GetValue())
        {
            m_xpmImage.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD);
        }

        if (m_ForceXpmMask->GetValue())
        {
            auto transparency = m_comboXpmMask->GetStringSelection();
            if (transparency != "none" && transparency != "custom")
            {
                wxColor rgb(transparency);
                m_xpmImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
            }
        }

        if (m_ForceHdrMask->GetValue())
        {
            auto transparency = m_comboHdrMask->GetStringSelection();
            if (transparency != "none" && transparency != "custom")
            {
                wxColor rgb(transparency);
                m_hdrImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
            }
        }

        if (m_xpmImage.HasMask())
        {
            wxColor clr = { m_xpmImage.GetMaskRed(), m_xpmImage.GetMaskGreen(), m_xpmImage.GetMaskBlue() };
            auto name = wxTheColourDatabase->FindName(clr);
            if (name.empty())
            {
                m_comboXpmMask->SetStringSelection("custom");
            }
            else
            {
                if (m_comboXpmMask->FindString(name) >= 0)
                {
                    m_comboXpmMask->SetStringSelection(name);
                }
                else
                {
                    m_comboXpmMask->SetStringSelection("custom");
                }
            }

            m_staticXpmRGB->SetLabelText(
                wxString().Format("%3d %3d %3d", (int) clr.Red(), (int) clr.Green(), (int) clr.Blue()));
            m_staticXpmRGB->Show();
        }
        else
        {
            m_comboXpmMask->SetStringSelection("none");
            m_staticXpmRGB->Hide();
        }

        if (m_hdrImage.HasMask())
        {
            wxColor clr = { m_hdrImage.GetMaskRed(), m_hdrImage.GetMaskGreen(), m_hdrImage.GetMaskBlue() };
            auto name = wxTheColourDatabase->FindName(clr);
            if (name.empty())
            {
                m_comboHdrMask->SetStringSelection("custom");
            }
            else
            {
                if (m_comboHdrMask->FindString(name) >= 0)
                {
                    m_comboHdrMask->SetStringSelection(name);
                }
                else
                {
                    m_comboHdrMask->SetStringSelection("custom");
                }
            }

            m_staticHdrRGB->SetLabelText(
                wxString().Format("%3d %3d %3d", (int) clr.Red(), (int) clr.Green(), (int) clr.Blue()));
            m_staticHdrRGB->Show();
        }
        else
        {
            m_comboHdrMask->SetStringSelection("none");
            m_staticHdrRGB->Hide();
        }

        m_bmpOriginal->SetBitmap(m_radio_header->GetValue() ? m_hdrImage : m_xpmImage);
        m_bmpOriginal->Show();
        m_staticOriginal->Show();
        SetSizeLabel();
        m_staticDimensions->Show();

        // Now that we have a loaded image, set the output file.
        ttString outFilename;
        auto dir_property = wxGetApp().GetProject()->prop_as_string(prop_converted_art);
        if (dir_property.size())
        {
            outFilename = dir_property;
            outFilename.append_filename_wx(file.filename());
        }
        else
        {
            outFilename = file.filename();
        }

        if (m_radio_header->GetValue())
        {
            m_fileHeader->SetPath(outFilename);
            AdjustOutputFilename();
        }
        else
        {
            outFilename.replace_extension(".xpm");
            m_fileHeader->SetPath(outFilename);
        }
        m_btnConvert->Enable();
        SetOutputBitmap();
    }
    else
    {
        m_hdrImage.Destroy();
        m_xpmImage.Destroy();
        m_orgImage.Destroy();
        m_bmpOriginal->SetBitmap(wxImage(empty_xpm));

        // If the input file is invalid, don't allow an output value
        m_fileHeader->SetPath(wxEmptyString);
        m_btnConvert->Disable();

        m_ConvertAlphaChannel->Disable();
        m_comboHdrMask->Disable();
        m_ForceHdrMask->Disable();
        m_comboXpmMask->Disable();
        m_ForceXpmMask->Disable();
    }

    if (m_fileOriginal->GetPath() != m_lastInputFile)
    {
        m_lastInputFile.clear();
        if (m_fileHeader->GetPath().size() && m_fileHeader->GetPath() != m_lastOutputFile)
            m_btnConvert->Enable();
    }

    // Various static text controls and the static bitmap for the current image may be shown or hidden based on whether
    // the image got loaded or not, so we simply resize the entire dialog.

    Fit();
}

void EmbedImage::OnXpmMask(wxCommandEvent& WXUNUSED(event))
{
    if (!m_ForceXpmMask->GetValue())
    {
        m_staticXpmRGB->Hide();
        return;
    }

    auto rgb = GetXpmTransparencyColor();
    m_staticXpmRGB->SetLabelText(wxString().Format("%3d %3d %3d", (int) rgb.Red(), (int) rgb.Green(), (int) rgb.Blue()));
    m_staticXpmRGB->Show();

    m_bmpOriginal->SetBitmap(m_radio_header->GetValue() ? m_hdrImage : m_xpmImage);
    Layout();
}

void EmbedImage::OnHdrMask(wxCommandEvent& WXUNUSED(event))
{
    if (!m_ForceHdrMask->GetValue())
    {
        m_staticHdrRGB->Hide();
        return;
    }

    auto rgb = GetHdrTransparencyColor();
    m_staticHdrRGB->SetLabelText(wxString().Format("%3d %3d %3d", (int) rgb.Red(), (int) rgb.Green(), (int) rgb.Blue()));
    m_staticHdrRGB->Show();

    m_bmpOriginal->SetBitmap(m_radio_header->GetValue() ? m_hdrImage : m_xpmImage);
    Layout();
}

void EmbedImage::OnConvert(wxCommandEvent& WXUNUSED(event))
{
    if (m_radio_header->GetValue())
        ImgageInHeaderOut();
    else if (m_radio_XPM->GetValue())
        ImageInXpmOut();

    SetOutputBitmap();
}

void EmbedImage::ImgageInHeaderOut()
{
    ttString in_filename = m_fileOriginal->GetTextCtrlValue();
    if (in_filename.empty())
    {
        appMsgBox("You need to specify a file to convert.");
        return;
    }
    if (!in_filename.filename())
    {
        appMsgBox("The input file you specified doesn't exist.");
        return;
    }

    if (!m_orgImage.IsOk())
    {
        appMsgBox(ttlib::cstr() << _tt(strIdCantOpen) << in_filename.wx_str());
        return;
    }

    wxMemoryOutputStream save_stream;
    if (m_check_make_png->GetValue() && isConvertibleMime(m_mime_type))
    {
        // Maximize compression
        m_hdrImage.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
        m_hdrImage.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
        m_hdrImage.SaveFile(save_stream, wxBITMAP_TYPE_PNG);
    }
    else
    {
        m_hdrImage.SaveFile(save_stream, m_mime_type);
    }

    auto read_stream = save_stream.GetOutputStreamBuffer();

    ttString out_name = m_fileHeader->GetPath();
    ttlib::cstr string_name = out_name.sub_cstr();

    string_name.remove_extension();
    string_name.Replace(".", "_", true);

    ttlib::textfile file;
    if (m_check_c17->GetValue())
        file.addEmptyLine().Format("inline constexpr const unsigned char %s[%zu] = {", string_name.filename().c_str(),
                                   read_stream->GetBufferSize());
    else
        file.addEmptyLine().Format("static const unsigned char %s[%zu] = {", string_name.filename().c_str(),
                                   read_stream->GetBufferSize());

    read_stream->Seek(0, wxFromStart);

    auto buf = static_cast<unsigned char*>(read_stream->GetBufferStart());

    size_t pos = 0;
    auto buf_size = read_stream->GetBufferSize();

    while (pos < buf_size)
    {
        {
            auto& line = file.addEmptyLine();
            for (; pos < buf_size && line.size() < 116; ++pos)
            {
                line << static_cast<int>(buf[pos]) << ',';
            }
        }
    }

    if (file[file.size() - 1].back() == ',')
        file[file.size() - 1].pop_back();

    file.addEmptyLine() << "};";

    if (out_name.empty())
    {
        m_staticSize->SetLabelText(
            ttlib::cstr().Format("Original size: %kzu -- Output size if saved: %kzu", m_orginal_size, buf_size));
        m_staticSize->Show();
    }
    else
    {
        if (file.WriteFile(out_name.utf8_str().data()))
        {
            m_staticSave->SetLabelText(wxString() << out_name << " saved.");
            m_staticSave->Show();
            m_staticSize->SetLabelText(
                ttlib::cstr().Format("Original size: %kzu -- Output size: %kzu", m_orginal_size, buf_size));
            m_staticSize->Show();
            m_lastOutputFile = out_name;
            m_btnConvert->Disable();
        }
        else
        {
            m_staticSave->SetLabelText(wxString() << _ttwx(strIdCantOpen) << out_name);
            m_staticSave->Show();
        }
    }
    GetSizer()->Fit(this);
}

void EmbedImage::ImageInXpmOut()
{
    ttString in_filename = m_fileOriginal->GetTextCtrlValue();
    if (in_filename.empty())
    {
        appMsgBox("You need to specify a file to convert.");
        return;
    }
    if (!in_filename.filename())
    {
        appMsgBox("The input file you specified doesn't exist.");
        return;
    }

    if (!m_xpmImage.IsOk())
    {
        appMsgBox(ttlib::cstr() << _tt(strIdCantOpen) << in_filename.wx_str());
        return;
    }

    ttString out_name = m_fileHeader->GetPath();
    if (out_name.size())
    {
        out_name.replace_extension(".xpm");

        if (m_xpmImage.SaveFile(out_name, wxBITMAP_TYPE_XPM))
        {
            size_t output_size = std::filesystem::file_size(std::filesystem::path(out_name.wx_str()));
            m_staticSave->SetLabelText(wxString() << out_name << " saved.");
            m_staticSave->Show();
            m_staticSize->SetLabelText(
                ttlib::cstr().Format("Original size: %kzu -- XPM size: %kzu", m_orginal_size, output_size));
            m_staticSize->Show();
            m_lastOutputFile = out_name;
            m_btnConvert->Disable();
        }
        GetSizer()->Fit(this);
    }
}

wxColor EmbedImage::GetXpmTransparencyColor()
{
    wxColor rgb { 0, 0, 0 };
    ttString transparency = m_comboXpmMask->GetStringSelection();
    if (transparency == "none" || transparency == "custom")
    {
        rgb = { m_xpmImage.GetMaskRed(), m_xpmImage.GetMaskGreen(), m_xpmImage.GetMaskBlue() };
        return rgb;
    }

    if (transparency == "upper left")
    {
        auto pBits = m_xpmImage.GetData();
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_xpmImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "lower left")
    {
        auto pBits = m_xpmImage.GetData();
        pBits += (((m_xpmImage.GetHeight() - 1) * m_xpmImage.GetWidth()) * 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_xpmImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "upper right")
    {
        auto pBits = m_xpmImage.GetData();
        pBits += (((m_xpmImage.GetWidth()) * 3) - 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_xpmImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "lower right")
    {
        auto pBits = m_xpmImage.GetData();
        pBits += (((m_xpmImage.GetHeight()) * (m_xpmImage.GetWidth()) * 3) - 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_xpmImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else
    {
        rgb = transparency;
        m_xpmImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    return rgb;
}

wxColor EmbedImage::GetHdrTransparencyColor()
{
    wxColor rgb { 0, 0, 0 };
    ttString transparency = m_comboXpmMask->GetStringSelection();
    if (transparency == "none" || transparency == "custom")
    {
        rgb = { m_hdrImage.GetMaskRed(), m_hdrImage.GetMaskGreen(), m_hdrImage.GetMaskBlue() };
        return rgb;
    }

    if (transparency == "upper left")
    {
        auto pBits = m_hdrImage.GetData();
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_hdrImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "lower left")
    {
        auto pBits = m_hdrImage.GetData();
        pBits += (((m_hdrImage.GetHeight() - 1) * m_hdrImage.GetWidth()) * 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_hdrImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "upper right")
    {
        auto pBits = m_hdrImage.GetData();
        pBits += (((m_hdrImage.GetWidth()) * 3) - 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_hdrImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "lower right")
    {
        auto pBits = m_hdrImage.GetData();
        pBits += (((m_hdrImage.GetHeight()) * (m_hdrImage.GetWidth()) * 3) - 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_hdrImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else
    {
        rgb = transparency;
        m_hdrImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    return rgb;
}

void EmbedImage::OnHeaderOutput(wxCommandEvent& WXUNUSED(event))
{
    if (!m_orgImage.IsOk())
        return;

    m_comboXpmMask->Disable();
    m_ForceXpmMask->Disable();
    m_comboHdrMask->Enable();
    m_ForceHdrMask->Enable();

    AdjustOutputFilename();

    if (m_fileOriginal->GetPath() != m_lastInputFile)
    {
        m_lastInputFile.clear();
        if (m_fileHeader->GetPath().size() && m_fileHeader->GetPath() != m_lastOutputFile)
            m_btnConvert->Enable();
    }

    SetSizeLabel();
    m_bmpOriginal->SetBitmap(m_hdrImage);
    SetOutputBitmap();
}

void EmbedImage::OnXpmOutput(wxCommandEvent& WXUNUSED(event))
{
    if (!m_orgImage.IsOk())
        return;

    m_comboHdrMask->Disable();
    m_ForceHdrMask->Disable();
    m_comboXpmMask->Enable();
    m_ForceXpmMask->Enable();

    ttString filename = m_fileHeader->GetPath();
    if (filename.size())
    {
        filename.replace_extension_wx(wxT("xpm"));
        filename.Replace("_png.", ".");
        m_fileHeader->SetPath(filename);
    }

    if (m_fileOriginal->GetPath() != m_lastInputFile)
    {
        m_lastInputFile.clear();
        if (m_fileHeader->GetPath().size() && m_fileHeader->GetPath() != m_lastOutputFile)
            m_btnConvert->Enable();
    }

    SetSizeLabel();
    m_bmpOriginal->SetBitmap(m_xpmImage);
    SetOutputBitmap();
}

void EmbedImage::OnConvertAlpha(wxCommandEvent& event)
{
    if (m_fileHeader->GetPath().size() && m_fileOriginal->GetPath().size())
        m_btnConvert->Enable();

    if (!m_orgImage.IsOk() || !m_xpmImage.IsOk())
    {
        m_staticDimensions->Hide();
        return;  // means the user hasn't supplied an input filename yet
    }

    SetSizeLabel();
    m_staticDimensions->Show();

    if (m_ConvertAlphaChannel->GetValue())
    {
        m_xpmImage.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD);

        if (m_xpmImage.HasMask())
        {
            wxColor clr = { m_xpmImage.GetMaskRed(), m_xpmImage.GetMaskGreen(), m_xpmImage.GetMaskBlue() };
            auto name = wxTheColourDatabase->FindName(clr);
            if (name.empty())
            {
                m_comboXpmMask->SetStringSelection("custom");
            }
            else
            {
                if (m_comboXpmMask->FindString(name) >= 0)
                    m_comboXpmMask->SetStringSelection(name);
                else
                    m_comboXpmMask->SetStringSelection("custom");
            }

            if (!m_ForceXpmMask->GetValue())
            {
                m_staticXpmRGB->SetLabelText(
                    wxString().Format("%3d %3d %3d", (int) clr.Red(), (int) clr.Green(), (int) clr.Blue()));
                m_staticXpmRGB->Show();

                m_bmpOriginal->SetBitmap(m_radio_header->GetValue() ? m_hdrImage : m_xpmImage);
                Layout();
                return;
            }
        }
        OnXpmMask(event);
    }
    else
    {
        m_xpmImage = m_orgImage.Copy();

        SetSizeLabel();
        m_staticDimensions->Show();

        OnXpmMask(event);
    }

    m_bmpOriginal->SetBitmap(m_radio_header->GetValue() ? m_hdrImage : m_xpmImage);
}

void EmbedImage::OnForceXpmMask(wxCommandEvent& event)
{
    if (!m_orgImage.IsOk())
        return;

    if (m_ForceXpmMask->GetValue())
    {
        ttString transparency = m_comboXpmMask->GetStringSelection();
        if (transparency == "none")
        {
            // Magenta is rarely used in graphics making it ideal as a mask color. If a mask is being forced, check the
            // four corner pixels, and if any of them are Magenta then set the mask to Magenta.

            for (;;)
            {
                auto pBits = m_xpmImage.GetData();
                // check upper left for Magenta
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboXpmMask->SetStringSelection("Magenta");
                    break;
                }

                // check lower left
                pBits += (((m_xpmImage.GetHeight() - 1) * m_xpmImage.GetWidth()) * 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboXpmMask->SetStringSelection("Magenta");
                    break;
                }

                // check upper right
                pBits = m_xpmImage.GetData() + (((m_xpmImage.GetWidth()) * 3) - 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboXpmMask->SetStringSelection("Magenta");
                    break;
                }

                // check upper right
                pBits = m_xpmImage.GetData() + (((m_xpmImage.GetHeight()) * (m_xpmImage.GetWidth()) * 3) - 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboXpmMask->SetStringSelection("Magenta");
                    break;
                }

                break;
            }
        }

        OnXpmMask(event);
        return;
    }
    else
    {
        m_xpmImage = m_orgImage.Copy();

        m_bmpOriginal->SetBitmap(m_radio_header->GetValue() ? m_hdrImage : m_xpmImage);
    }
}

void EmbedImage::OnForceHdrMask(wxCommandEvent& event)
{
    if (!m_orgImage.IsOk())
        return;

    if (m_ForceHdrMask->GetValue())
    {
        if (m_hdrImage.HasAlpha())
        {
            m_hdrImage.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD);
        }

        ttString transparency = m_comboHdrMask->GetStringSelection();
        if (transparency == "none")
        {
            // Magenta is rarely used in graphics making it ideal as a mask color. If a mask is being forced, check the
            // four corner pixels, and if any of them are Magenta then set the mask to Magenta.

            for (;;)
            {
                auto pBits = m_hdrImage.GetData();
                // check upper left for Magenta
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboHdrMask->SetStringSelection("Magenta");
                    break;
                }

                // check lower left
                pBits += (((m_hdrImage.GetHeight() - 1) * m_hdrImage.GetWidth()) * 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboHdrMask->SetStringSelection("Magenta");
                    break;
                }

                // check upper right
                pBits = m_hdrImage.GetData() + (((m_hdrImage.GetWidth()) * 3) - 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboHdrMask->SetStringSelection("Magenta");
                    break;
                }

                // check upper right
                pBits = m_hdrImage.GetData() + (((m_hdrImage.GetHeight()) * (m_hdrImage.GetWidth()) * 3) - 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboHdrMask->SetStringSelection("Magenta");
                    break;
                }

                break;
            }
        }

        // transparency may have changed if we found magenta
        transparency = m_comboHdrMask->GetStringSelection();
        if (transparency != "none" && transparency != "custom")
        {
            wxColor rgb(transparency);
            m_hdrImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
        }

        OnHdrMask(event);
        return;
    }
    else
    {
        m_hdrImage = m_orgImage.Copy();

        m_bmpOriginal->SetBitmap(m_radio_header->GetValue() ? m_hdrImage : m_xpmImage);
    }
}

void EmbedImage::OnOutputChange(wxFileDirPickerEvent& WXUNUSED(event))
{
    if (m_fileOriginal->GetPath() != m_lastInputFile)
    {
        m_lastInputFile.clear();
        if (m_fileHeader->GetPath().size() && m_fileHeader->GetPath() != m_lastOutputFile)
        {
            m_btnConvert->Enable();
            SetOutputBitmap();
        }
    }
}

void EmbedImage::SetOutputBitmap()
{
    if (m_fileOriginal->GetPath().empty())
    {
        m_bmpOutput->Hide();
        m_staticOutput->Hide();
        return;
    }

    ttString out_file = m_fileHeader->GetPath();
    if (out_file.empty() || !out_file.file_exists())
    {
        m_bmpOutput->Hide();
        m_staticOutput->Hide();
        return;
    }

    if (out_file.has_extension(".h") || out_file.has_extension(".hpp") || out_file.has_extension(".hh") ||
        out_file.has_extension(".hxx"))
    {
        wxBusyCursor wait;
        auto image = GetHeaderImage(out_file.sub_cstr());

        if (image.IsOk())
        {
            m_bmpOutput->SetBitmap(image);
            m_bmpOutput->Show();
            m_staticOutput->Show();
            return;
        }
        else
        {
            m_bmpOutput->Hide();
            m_staticOutput->Hide();
            return;
        }
    }

    wxImage image;
    if (image.LoadFile(out_file))
    {
        m_bmpOutput->SetBitmap(image);
        m_bmpOutput->Show();
        m_staticOutput->Show();
        Layout();
    }
    else
    {
        m_bmpOutput->Hide();
        m_staticOutput->Hide();
    }
}

void EmbedImage::OnCheckPngConversion(wxCommandEvent& WXUNUSED(event))
{
    if (!m_radio_header->GetValue())
        return;

    AdjustOutputFilename();
}

void EmbedImage::AdjustOutputFilename()
{
    ttString filename = m_fileHeader->GetPath();
    if (filename.size())
    {
        auto ext_property = wxGetApp().GetProject()->prop_as_string(prop_header_ext);
        if (ext_property.empty())
            ext_property = ".h";

        ttString suffix(m_mime_type);
        suffix.Replace("image/", "_");
        suffix.Replace("x-", "");  // if something like x-bmp, just use bmp

        if (m_check_make_png->GetValue() && isConvertibleMime(m_mime_type))
        {
            filename.remove_extension();
            if (!filename.contains("_png") && !suffix.is_sameas_wx("_png"))
            {
                if (filename.contains_wx(suffix))
                    suffix = "_png";
                else
                    suffix << "_png";
            }
        }

        if (!filename.contains_wx(suffix))
        {
            filename.remove_extension();
            filename << suffix << ext_property;
        }
        else
        {
            filename.replace_extension_wx(ext_property);
        }
        m_fileHeader->SetPath(filename);
    }
}

void EmbedImage::SetSizeLabel()
{
    wxString size_label;
    size_label << "Size: " << m_xpmImage.GetWidth() << " x " << m_xpmImage.GetHeight();

    // Add the mime type
    size_label << "  Type: " << m_mime_type;

    if (m_radio_XPM->GetValue())
    {
        if (m_orgImage.HasAlpha() && m_ConvertAlphaChannel->GetValue())
            size_label << " (had alpha channel)";
    }
    else
    {
        if (m_hdrImage.HasAlpha())
            size_label << " (has alpha channel)";
    }

    m_staticDimensions->SetLabelText(size_label);
}
