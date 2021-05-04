/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert image to Header (.h) or XPM (.xpm) file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// There are several controls in this dialog that are hidden or displayed based on user interaction. Because of that, there
// are times when just calling Layout() is insufficient -- so to play it safe, whenever the dialog's size may need to be
// changed, both Fit() and Layout() are called. That's still not 100% accurate, but it's close enough that the user isn't
// likely to notice the extra spacing that sometimes occurs.

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
    m_fileOutput->SetInitialDirectory(dir);

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

    Fit();
    Layout();
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
        }
        else
        {
            m_comboXpmMask->SetStringSelection("none");
            m_staticXpmRGB->SetLabelText("0 0 0");
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
        }
        else
        {
            m_comboHdrMask->SetStringSelection("none");
            m_staticHdrRGB->SetLabelText("0 0 0");
        }

        m_bmpOriginal->SetBitmap(IsHeaderPage() ? m_hdrImage : m_xpmImage);
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

        if (IsHeaderPage())
        {
            m_fileOutput->SetPath(outFilename);
            AdjustOutputFilename();
        }
        else
        {
            outFilename.replace_extension(".xpm");
            m_fileOutput->SetPath(outFilename);
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
        m_fileOutput->SetPath(wxEmptyString);
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
        if (m_fileOutput->GetPath().size() && m_fileOutput->GetPath() != m_lastOutputFile)
            m_btnConvert->Enable();
    }

    // Various static text controls and the static bitmap for the current image may be shown or hidden based on whether
    // the image got loaded or not, so we simply resize the entire dialog.

    Fit();
    Layout();
}

void EmbedImage::OnComboXpmMask(wxCommandEvent& WXUNUSED(event))
{
    if (!m_ForceXpmMask->GetValue())
    {
        m_staticXpmRGB->SetLabelText("0 0 0");
    }
    else
    {
        auto rgb = GetXpmTransparencyColor();  // this will set the mask in m_xpmImage
        m_staticXpmRGB->SetLabelText(wxString().Format("%3d %3d %3d", (int) rgb.Red(), (int) rgb.Green(), (int) rgb.Blue()));
    }

    m_bmpOriginal->SetBitmap(m_xpmImage);
    EnableConvertButton();

    Fit();
    Layout();
}

void EmbedImage::OnComboHdrMask(wxCommandEvent& WXUNUSED(event))
{
    if (!m_ForceHdrMask->GetValue())
    {
        m_staticHdrRGB->SetLabelText("0 0 0");
    }
    else
    {
        auto rgb = GetHdrTransparencyColor();  // this will set the mask in m_hdrImage
        m_staticHdrRGB->SetLabelText(wxString().Format("%3d %3d %3d", (int) rgb.Red(), (int) rgb.Green(), (int) rgb.Blue()));
    }

    m_bmpOriginal->SetBitmap(m_hdrImage);
    EnableConvertButton();

    Fit();
    Layout();
}

void EmbedImage::OnConvert(wxCommandEvent& WXUNUSED(event))
{
    if (IsHeaderPage())
        ImgageInHeaderOut();
    else
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

    wxBusyCursor wait;
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

    ttString out_name = m_fileOutput->GetPath();
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

    ttString out_name = m_fileOutput->GetPath();
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
    ttString transparency = m_comboHdrMask->GetStringSelection();
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

void EmbedImage::OnPageChanged(wxBookCtrlEvent& WXUNUSED(event))
{
    if (!m_orgImage.IsOk())
        return;

    if (IsHeaderPage())
    {
        AdjustOutputFilename();
        m_bmpOriginal->SetBitmap(m_hdrImage);
    }
    else
    {
        ttString filename = m_fileOutput->GetPath();
        if (filename.size())
        {
            filename.Replace("_png", "");
            filename.Replace("_xpm", "");
            filename.replace_extension_wx(wxT("xpm"));
            m_fileOutput->SetPath(filename);
        }
        m_bmpOriginal->SetBitmap(m_xpmImage);
    }

    EnableConvertButton();
    SetSizeLabel();
    SetOutputBitmap();
}

// This is only used for XPM output
void EmbedImage::OnConvertAlpha(wxCommandEvent& event)
{
    if (m_fileOutput->GetPath().size() && m_fileOriginal->GetPath().size())
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

                m_bmpOriginal->SetBitmap(IsHeaderPage() ? m_hdrImage : m_xpmImage);
                Fit();
                Layout();
                return;
            }
        }
        OnComboXpmMask(event);
    }
    else
    {
        m_xpmImage = m_orgImage.Copy();

        SetSizeLabel();
        m_staticDimensions->Show();

        OnComboXpmMask(event);
    }

    if (IsXpmPage())
    {
        m_bmpOriginal->SetBitmap(m_xpmImage);
        EnableConvertButton();
    }
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

        OnComboXpmMask(event);
    }
    else
    {
        m_xpmImage = m_orgImage.Copy();
        OnComboXpmMask(event);
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
        OnComboHdrMask(event);
    }
    else
    {
        m_hdrImage = m_orgImage.Copy();
        OnComboHdrMask(event);
    }
}

void EmbedImage::OnOutputChange(wxFileDirPickerEvent& WXUNUSED(event))
{
    if (m_fileOriginal->GetPath() != m_lastInputFile)
    {
        m_lastInputFile.clear();
        if (m_fileOutput->GetPath().size() && m_fileOutput->GetPath() != m_lastOutputFile)
        {
            EnableConvertButton();
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

    ttString out_file = m_fileOutput->GetPath();
    if (out_file.empty() || !out_file.file_exists())
    {
        m_bmpOutput->Hide();
        m_staticOutput->Hide();
        return;
    }

    wxBusyCursor wait;
    wxImage image;

    if (out_file.has_extension(".h") || out_file.has_extension(".hpp") || out_file.has_extension(".hh") ||
        out_file.has_extension(".hxx"))
    {
        image = GetHeaderImage(out_file.sub_cstr());
    }
    else
    {
        image.LoadFile(out_file);

#if defined(_DEBUG)
        auto has_mask = m_xpmImage.HasMask();
        wxColor rgb;
        if (has_mask)
        {
            rgb = { m_xpmImage.GetMaskRed(), m_xpmImage.GetMaskGreen(), m_xpmImage.GetMaskBlue() };
        }
#endif  // _DEBUG
    }

    if (image.IsOk())
    {
        m_bmpOutput->SetBitmap(image);
        m_bmpOutput->Show();
        m_staticOutput->Show();
    }
    else
    {
        m_bmpOutput->Hide();
        m_staticOutput->Hide();
    }

    Fit();
    Layout();
}

void EmbedImage::OnCheckPngConversion(wxCommandEvent& WXUNUSED(event))
{
    if (IsHeaderPage())
    {
        EnableConvertButton();
        AdjustOutputFilename();
    }
}

void EmbedImage::OnC17Encoding(wxCommandEvent& WXUNUSED(event))
{
    if (IsHeaderPage())
    {
        EnableConvertButton();
    }
}

void EmbedImage::AdjustOutputFilename()
{
    ttString filename = m_fileOutput->GetPath();
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
        m_fileOutput->SetPath(filename);
    }
}

void EmbedImage::SetSizeLabel()
{
    wxString size_label;
    size_label << "Size: " << m_xpmImage.GetWidth() << " x " << m_xpmImage.GetHeight();

    // Add the mime type
    size_label << "  Type: " << m_mime_type;

    if (IsXpmPage())
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

void EmbedImage::EnableConvertButton()
{
    if (m_lastOutputFile.size())
    {
        m_lastOutputFile.clear();
        m_staticSave->SetLabelText(wxEmptyString);
        m_staticSize->SetLabelText(wxEmptyString);
        m_staticSave->Hide();
        m_staticSize->Hide();
        if (m_orgImage.IsOk() && m_fileOriginal->GetPath().size() && m_fileOutput->GetPath().size())
            m_btnConvert->Enable();
        Fit();
        Layout();
    }
}
