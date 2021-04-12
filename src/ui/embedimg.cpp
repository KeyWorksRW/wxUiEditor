/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert image to Header (.h) or XPM (.xpm) file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <filesystem>
#include <fstream>

#include <wx/artprov.h>
#include <wx/mstream.h>

#include <tttextfile.h>  // textfile -- Classes for reading and writing line-oriented files

#include "embedimg.h"  // auto-generated: embedimg_base.h and embedimg_base.cpp

#include "bitmaps.h"    // Map of bitmaps accessed by name
#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class
#include "uifuncs.h"    // Miscellaneous functions for displaying UI
#include "utils.h"      // Utility functions that work with properties

#include "../xpm/empty.xpm"

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

    m_comboMask->Append("none");
    m_comboMask->Append("custom");

    m_comboMask->Append("Black");
    m_comboMask->Append("Dark Green");
    m_comboMask->Append("Dark Grey");
    m_comboMask->Append("Dark Slate Grey");
    m_comboMask->Append("Dim Grey");
    m_comboMask->Append("Grey");
    m_comboMask->Append("Light Grey");
    m_comboMask->Append("Grey94");
    m_comboMask->Append("Magenta");
    m_comboMask->Append("White");

    m_comboMask->Append("upper left");
    m_comboMask->Append("lower left");
    m_comboMask->Append("upper right");
    m_comboMask->Append("lower right");

    m_comboMask->SetStringSelection("none");

    m_staticSave->SetLabelText(wxEmptyString);
    m_staticSize->SetLabelText(wxEmptyString);
    m_staticDimensions->SetLabelText(wxEmptyString);

    m_btnConvert->Disable();
    m_ConvertAlphaChannel->Disable();

    Fit();
}

void EmbedImage::OnInputChange(wxFileDirPickerEvent& WXUNUSED(event))
{
    ttString file = m_fileOriginal->GetTextCtrlValue();
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

    bool isImageLoaded { false };
    if (file.has_extension(".h") || file.has_extension(".hpp") || file.has_extension(".hh") || file.has_extension(".hxx"))
    {
        m_orgImage = GetHeaderImage(file.sub_cstr());
        if (m_orgImage.IsOk())
        {
            isImageLoaded = true;
            m_radio_header->SetValue(false);
            m_radio_header->Disable();
            m_radio_XPM->SetValue(true);
        }
    }

    if (!isImageLoaded)
    {
        m_radio_header->Enable();
    }

    if (isImageLoaded || m_orgImage.LoadFile(m_fileOriginal->GetTextCtrlValue()))
    {
        m_curImage = m_orgImage.Copy();
        m_ConvertAlphaChannel->Enable(m_curImage.HasAlpha());
        bool hasAlphaChannel = m_curImage.HasAlpha();
        if (hasAlphaChannel && m_ConvertAlphaChannel->GetValue())
        {
            m_curImage.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD);
        }

        if (m_UseMask->GetValue())
        {
            auto transparency = m_comboMask->GetStringSelection();
            if (transparency != "none" && transparency != "custom")
            {
                wxColor rgb(transparency);
                m_curImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
            }
        }

        if (m_curImage.HasMask())
        {
            wxColor clr = { m_curImage.GetMaskRed(), m_curImage.GetMaskGreen(), m_curImage.GetMaskBlue() };
            auto name = wxTheColourDatabase->FindName(clr);
            if (name.empty())
            {
                m_comboMask->SetStringSelection("custom");
            }
            else
            {
                if (m_comboMask->FindString(name) >= 0)
                    m_comboMask->SetStringSelection(name);
                else
                    m_comboMask->SetStringSelection("custom");
            }

            m_staticRGB->SetLabelText(wxString().Format("%3d %3d %3d", (int) clr.Red(), (int) clr.Green(), (int) clr.Blue()));
            m_staticRGB->Show();
        }
        else
        {
            m_comboMask->SetStringSelection("none");
            m_staticRGB->Hide();
        }

        m_bmpOriginal->SetBitmap(m_curImage);
        m_bmpOriginal->Show();
        m_staticOriginal->Show();

        wxString text;
        text << _ttwx(strIdSize) << m_curImage.GetWidth() << " x " << m_curImage.GetHeight();
        if (hasAlphaChannel)
            text << (m_ConvertAlphaChannel->GetValue() ? " (had alpha channel)" : " (has alpha channel)");

        m_staticDimensions->SetLabelText(text);
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
            auto ext_property = wxGetApp().GetProject()->prop_as_string(prop_header_ext);
            if (ext_property.empty())
                ext_property = ".h";
            if (!outFilename.contains("_png"))
            {
                outFilename.remove_extension();
                outFilename << "_png" << ext_property;
            }
            else
            {
                outFilename.replace_extension_wx(ext_property);
            }
        }
        else
        {
            outFilename.replace_extension(".xpm");
        }
        m_fileHeader->SetPath(outFilename);
        m_btnConvert->Enable();
        SetOutputBitmap();
    }
    else
    {
        m_curImage.Destroy();
        m_orgImage.Destroy();
        m_bmpOriginal->SetBitmap(wxImage(empty_xpm));

        // If the input file is invalid, don't allow an output value
        m_fileHeader->SetPath(wxEmptyString);
        m_btnConvert->Disable();
        m_ConvertAlphaChannel->Disable();
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

void EmbedImage::OnMask(wxCommandEvent& WXUNUSED(event))
{
    if (m_fileHeader->GetPath().size() && m_fileOriginal->GetPath().size())
        m_btnConvert->Enable();

    if (!m_UseMask->GetValue())
    {
        m_staticRGB->Hide();
        return;
    }

    auto rgb = GetTransparencyColor();
    m_staticRGB->SetLabelText(wxString().Format("%3d %3d %3d", (int) rgb.Red(), (int) rgb.Green(), (int) rgb.Blue()));
    m_staticRGB->Show();

    m_bmpOriginal->SetBitmap(m_curImage);  // GetTransparencyColor() may have changed m_curImage
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

inline bool is_printable(unsigned char ch)
{
    return (ch > 31 && ch < 128);
}

inline bool is_numeric(unsigned char ch)
{
    return (ch >= '0' && ch <= '9');
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

    if (!m_curImage.IsOk())
    {
        appMsgBox(ttlib::cstr() << _tt(strIdCantOpen) << in_filename.wx_str());
        return;
    }

    size_t original_size = std::filesystem::file_size(std::filesystem::path(in_filename.wx_str()));

    // Maximize compression
    m_curImage.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
    m_curImage.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);

    wxMemoryOutputStream save_stream;
    m_curImage.SaveFile(save_stream, wxBITMAP_TYPE_PNG);
    auto read_stream = save_stream.GetOutputStreamBuffer();

    ttString out_name = m_fileHeader->GetPath();
    ttlib::cstr string_name = out_name.sub_cstr();

    string_name.remove_extension();
    string_name.Replace(".", "_", true);

    ttlib::textfile file;
    if (m_check_constexpr->GetValue())
        file.addEmptyLine().Format("inline constexpr const unsigned char %s[%zu] = {", string_name.filename().c_str(),
                                   read_stream->GetBufferSize());
    else
        file.addEmptyLine().Format("static const unsigned char %s[%zu] = {", string_name.filename().c_str(), read_stream->GetBufferSize());

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
        m_staticSize->SetLabelText(ttlib::cstr().Format("Original size: %kzu -- Output size if saved: %kzu", original_size, buf_size));
        m_staticSize->Show();
    }
    else
    {
        if (file.WriteFile(out_name.utf8_str().data()))
        {
            m_staticSave->SetLabelText(wxString() << out_name << " saved.");
            m_staticSave->Show();
            m_staticSize->SetLabelText(ttlib::cstr().Format("Original size: %kzu -- Output size: %kzu", original_size, buf_size));
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

    if (!m_curImage.IsOk())
    {
        appMsgBox(ttlib::cstr() << _tt(strIdCantOpen) << in_filename.wx_str());
        return;
    }

    size_t original_size = std::filesystem::file_size(std::filesystem::path(in_filename.wx_str()));

    ttString out_name = m_fileHeader->GetPath();
    if (out_name.size())
    {
        out_name.replace_extension(".xpm");

        if (m_curImage.SaveFile(out_name, wxBITMAP_TYPE_XPM))
        {
            size_t output_size = std::filesystem::file_size(std::filesystem::path(out_name.wx_str()));
            m_staticSave->SetLabelText(wxString() << out_name << " saved.");
            m_staticSave->Show();
            m_staticSize->SetLabelText(ttlib::cstr().Format("Original size: %kzu -- XPM size: %kzu", original_size, output_size));
            m_staticSize->Show();
            m_lastOutputFile = out_name;
            m_btnConvert->Disable();
        }
        GetSizer()->Fit(this);
    }
}

wxColor EmbedImage::GetTransparencyColor()
{
    wxColor rgb { 0, 0, 0 };
    ttString transparency = m_comboMask->GetStringSelection();
    if (transparency == "none" || transparency == "custom")
    {
        rgb = { m_curImage.GetMaskRed(), m_curImage.GetMaskGreen(), m_curImage.GetMaskBlue() };
        return rgb;
    }

    if (transparency == "upper left")
    {
        auto pBits = m_curImage.GetData();
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_curImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "lower left")
    {
        auto pBits = m_curImage.GetData();
        pBits += (((m_curImage.GetHeight() - 1) * m_curImage.GetWidth()) * 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_curImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "upper right")
    {
        auto pBits = m_curImage.GetData();
        pBits += (((m_curImage.GetWidth()) * 3) - 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_curImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else if (transparency == "lower right")
    {
        auto pBits = m_curImage.GetData();
        pBits += (((m_curImage.GetHeight()) * (m_curImage.GetWidth()) * 3) - 3);
        rgb = { pBits[0], pBits[1], pBits[2] };
        m_curImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    else
    {
        rgb = transparency;
        m_curImage.SetMaskColour(rgb.Red(), rgb.Green(), rgb.Blue());
    }
    return rgb;
}

void EmbedImage::OnHeaderOutput(wxCommandEvent& WXUNUSED(event))
{
    ttString filename = m_fileHeader->GetPath();
    if (filename.size())
    {
        auto ext_property = wxGetApp().GetProject()->prop_as_string(prop_header_ext);
        if (ext_property.empty())
            ext_property = ".h";
        if (!filename.contains("_png"))
        {
            filename.remove_extension();
            filename << "_png" << ext_property;
        }
        else
        {
            filename.replace_extension_wx(ext_property);
        }
        m_fileHeader->SetPath(filename);
    }

    if (m_fileOriginal->GetPath() != m_lastInputFile)
    {
        m_lastInputFile.clear();
        if (m_fileHeader->GetPath().size() && m_fileHeader->GetPath() != m_lastOutputFile)
            m_btnConvert->Enable();
    }

    SetOutputBitmap();
}

void EmbedImage::OnXpmOutput(wxCommandEvent& WXUNUSED(event))
{
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

    SetOutputBitmap();
}

void EmbedImage::OnConvertAlpha(wxCommandEvent& event)
{
    if (m_fileHeader->GetPath().size() && m_fileOriginal->GetPath().size())
        m_btnConvert->Enable();

    if (!m_orgImage.IsOk())
    {
        m_staticDimensions->Show();
        return;  // means the user hasn't supplied an input filename yet
    }

    if (m_curImage.IsOk())
    {
        wxString text;
        text << _ttwx(strIdSize) << m_curImage.GetWidth() << " x " << m_curImage.GetHeight();
        if (m_curImage.HasAlpha())
            text << (m_ConvertAlphaChannel->GetValue() ? " (had alpha channel)" : " (has alpha channel)");

        m_staticDimensions->SetLabelText(text);
        m_staticDimensions->Show();
    }

    if (m_ConvertAlphaChannel->GetValue())
    {
        m_curImage.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD);

        if (m_curImage.HasMask())
        {
            wxColor clr = { m_curImage.GetMaskRed(), m_curImage.GetMaskGreen(), m_curImage.GetMaskBlue() };
            auto name = wxTheColourDatabase->FindName(clr);
            if (name.empty())
            {
                m_comboMask->SetStringSelection("custom");
            }
            else
            {
                if (m_comboMask->FindString(name) >= 0)
                    m_comboMask->SetStringSelection(name);
                else
                    m_comboMask->SetStringSelection("custom");
            }

            if (!m_UseMask->GetValue())
            {
                m_staticRGB->SetLabelText(wxString().Format("%3d %3d %3d", (int) clr.Red(), (int) clr.Green(), (int) clr.Blue()));
                m_staticRGB->Show();

                m_bmpOriginal->SetBitmap(m_curImage);  // GetTransparencyColor() may have changed m_curImage
                Layout();
                return;
            }
        }
        OnMask(event);
    }
    else
    {
        m_curImage = m_orgImage.Copy();

        wxString text;
        text << _ttwx(strIdSize) << m_curImage.GetWidth() << " x " << m_curImage.GetHeight();
        if (m_curImage.HasAlpha())
            text << (m_ConvertAlphaChannel->GetValue() ? " (had alpha channel)" : " (has alpha channel)");

        m_staticDimensions->SetLabelText(text);
        m_staticDimensions->Show();

        OnMask(event);
    }

    m_bmpOriginal->SetBitmap(m_curImage);
}

void EmbedImage::OnForceMask(wxCommandEvent& event)
{
    if (m_fileHeader->GetPath().size() && m_fileOriginal->GetPath().size())
        m_btnConvert->Enable();

    if (m_UseMask->GetValue())
    {
        ttString transparency = m_comboMask->GetStringSelection();
        if (transparency == "none")
        {
            // Magenta is rarely used in graphics making it ideal as a mask color. If a mask is being forced, check the
            // four corner pixels, and if any of them are Magenta then set the mask to Magenta.

            for (;;)
            {
                auto pBits = m_curImage.GetData();
                // check upper left for Magenta
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboMask->SetStringSelection("Magenta");
                    break;
                }

                // check lower left
                pBits += (((m_curImage.GetHeight() - 1) * m_curImage.GetWidth()) * 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboMask->SetStringSelection("Magenta");
                    break;
                }

                // check upper right
                pBits = m_curImage.GetData() + (((m_curImage.GetWidth()) * 3) - 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboMask->SetStringSelection("Magenta");
                    break;
                }

                // check upper right
                pBits = m_curImage.GetData() + (((m_curImage.GetHeight()) * (m_curImage.GetWidth()) * 3) - 3);
                if (pBits[0] == 255 && pBits[1] == 0 && pBits[2] == 255)
                {
                    m_comboMask->SetStringSelection("Magenta");
                    break;
                }

                break;
            }
        }

        OnMask(event);
        return;
    }
    else
    {
        m_curImage = m_orgImage.Copy();

        m_bmpOriginal->SetBitmap(m_curImage);
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

    if (out_file.has_extension(".h") || out_file.has_extension(".hpp") || out_file.has_extension(".hh") || out_file.has_extension(".hxx"))
    {
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
