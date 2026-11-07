/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to convert an SVG file into a multi-resolution Windows icon
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "mainframe.h"  // MainFrame -- Main application window

#include <wx/bitmap.h>
#include <wx/bmpbndl.h>
#include <wx/button.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wfstream.h>

#include "svg_to_ico_dlg.h"  // ConvertSvgToIco, SvgToIcoDlg

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

static bool ReportError(wxString* error_msg, const wxString& message)
{
    if (error_msg != nullptr)
    {
        *error_msg = message;
    }
    return false;
}

static void WriteBytes(wxOutputStream& output_stream, std::span<const std::uint8_t> bytes)
{
    std::ignore = output_stream.Write(bytes.data(), bytes.size());
}

static void WriteByte(wxOutputStream& output_stream, std::uint8_t value)
{
    std::ignore = output_stream.Write(&value, 1);
}

static void WriteUint16Le(wxOutputStream& output_stream, std::uint16_t value)
{
    const std::array<std::uint8_t, 2> bytes = { static_cast<std::uint8_t>(value & 0xFFU),
                                                static_cast<std::uint8_t>((value >> 8) & 0xFFU) };
    WriteBytes(output_stream, bytes);
}

static void WriteUint32Le(wxOutputStream& output_stream, std::uint32_t value)
{
    const std::array<std::uint8_t, 4> bytes = { static_cast<std::uint8_t>(value & 0xFFU),
                                                static_cast<std::uint8_t>((value >> 8) & 0xFFU),
                                                static_cast<std::uint8_t>((value >> 16) & 0xFFU),
                                                static_cast<std::uint8_t>((value >> 24) & 0xFFU) };
    WriteBytes(output_stream, bytes);
}

bool ConvertSvgToIco(const wxString& svg_path, const wxString& ico_path, wxString* error_msg)
{
    constexpr auto icon_sizes = std::to_array<int>({ 16, 32, 48, 256 });
    constexpr std::array<std::uint8_t, 16> EMPTY_DIRECTORY_ENTRY = {};

    const wxBitmapBundle bundle = wxBitmapBundle::FromSVGFile(svg_path, wxSize(256, 256));
    if (!bundle.IsOk())
    {
        return ReportError(error_msg, wxString() << "Unable to load the SVG file: " << svg_path);
    }

    wxFileOutputStream output_stream(ico_path);
    if (!output_stream.IsOk())
    {
        return ReportError(error_msg, wxString() << "Unable to create the icon file: " << ico_path);
    }

    // ICONDIR: reserved, type (1 == icon), and the number of images in the file.
    WriteUint16Le(output_stream, 0);
    WriteUint16Le(output_stream, 1);
    WriteUint16Le(output_stream, static_cast<std::uint16_t>(icon_sizes.size()));

    // Four ICONDIRENTRY blocks filled with zeros -- they are rewritten after the PNG data
    // has been written so that the offsets and sizes are known.
    for (std::size_t entry_index = 0; entry_index < icon_sizes.size(); ++entry_index)
    {
        WriteBytes(output_stream, EMPTY_DIRECTORY_ENTRY);
    }

    std::array<wxFileOffset, icon_sizes.size()> image_offsets = {};
    std::array<std::uint32_t, icon_sizes.size()> image_sizes = {};

    std::size_t image_index = 0;
    for (const int icon_size: icon_sizes)
    {
        const wxBitmap icon_bitmap = bundle.GetBitmap(wxSize(icon_size, icon_size));
        if (!icon_bitmap.IsOk())
        {
            return ReportError(error_msg, wxString() << "Unable to render a " << icon_size << "x"
                                                     << icon_size
                                                     << " image from the SVG file: " << svg_path);
        }

        const wxImage icon_image = icon_bitmap.ConvertToImage();
        if (!icon_image.IsOk())
        {
            return ReportError(error_msg, wxString() << "Unable to create a " << icon_size << "x"
                                                     << icon_size
                                                     << " image from the SVG file: " << svg_path);
        }

        const wxFileOffset image_offset = output_stream.TellO();
        if (!icon_image.SaveFile(output_stream, wxBITMAP_TYPE_PNG))
        {
            return ReportError(error_msg, wxString() << "Unable to write the " << icon_size << "x"
                                                     << icon_size
                                                     << " PNG data to the icon file: " << ico_path);
        }

        const wxFileOffset image_size = output_stream.TellO() - image_offset;
        image_offsets[image_index] = image_offset;
        image_sizes[image_index] = static_cast<std::uint32_t>(image_size);
        ++image_index;
    }

    if (output_stream.SeekO(6) == wxInvalidOffset)
    {
        return ReportError(error_msg,
                           wxString() << "Unable to update the icon directory in: " << ico_path);
    }

    for (std::size_t entry_index = 0; entry_index < icon_sizes.size(); ++entry_index)
    {
        const int icon_size = icon_sizes[entry_index];
        // A width or height of zero means 256 pixels in the ICO format.
        const std::uint8_t dimension { static_cast<std::uint8_t>(icon_size == 256 ? 0 :
                                                                                    icon_size) };
        WriteByte(output_stream, dimension);  // width
        WriteByte(output_stream, dimension);  // height
        WriteByte(output_stream, 0);          // color count (0 for 32-bit images)
        WriteByte(output_stream, 0);          // reserved, must be 0
        WriteUint16Le(output_stream, 1);      // wPlanes must be 1 per the ICO format
        WriteUint16Le(output_stream, 32);     // wBitCount
        WriteUint32Le(output_stream, image_sizes[entry_index]);  // dwBytesInRes
        WriteUint32Le(output_stream, static_cast<std::uint32_t>(image_offsets[entry_index]));
    }

    if (!output_stream.IsOk())
    {
        return ReportError(error_msg, wxString() << "Unable to write the icon file: " << ico_path);
    }

    // wxFileOutputStream::Close() returns false if the file could not be closed. Note that
    // IsOk() must not be called after Close() -- wxFileOutputStream::IsOk() also tests
    // wxFile::IsOpened(), which is false once the file has been closed.
    if (!output_stream.Close())
    {
        return ReportError(error_msg, wxString() << "Unable to close the icon file: " << ico_path);
    }

    return true;
}

SvgToIcoDlg::SvgToIcoDlg(wxWindow* parent)
{
    std::ignore = Create(parent);
}

void SvgToIcoDlg::OnInit(wxInitDialogEvent& event)
{
    // The affirmative button is created by the wxStdDialogButtonSizer in the base class, so it
    // can only be relabelled from here.
    if (wxWindow* ok_button = FindWindow(wxID_OK); ok_button != nullptr)
    {
        ok_button->SetLabel("Convert");
    }

    UpdateConvertState();

    event.Skip();
}

void SvgToIcoDlg::OnClose([[maybe_unused]] wxCommandEvent& event)
{
    // The dialog is only ever shown with ShowModal(), so EndModal is used rather than Close().
    EndModal(wxID_CANCEL);
}

void SvgToIcoDlg::OnConvert([[maybe_unused]] wxCommandEvent& event)
{
    const wxString svg_path = m_fileSvg->GetPath();

    wxFileName ico_name(m_fileIco->GetPath());
    if (!ico_name.HasExt())
    {
        ico_name.SetExt("ico");
    }
    const wxString ico_path = ico_name.GetFullPath();

    if (wxFileName::FileExists(ico_path))
    {
        const wxString prompt =
            wxString() << ico_path << "\n\nThis file already exists. Do you want to replace it?";
        if (wxMessageBox(prompt, "Convert SVG to Windows Icon",
                         wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) != wxYES)
        {
            return;
        }
    }

    // Prevent a second conversion from starting while this one is running.
    if (wxWindow* ok_button = FindWindow(wxID_OK); ok_button != nullptr)
    {
        ok_button->Disable();
    }

    wxString error;
    if (ConvertSvgToIco(svg_path, ico_path, &error))
    {
        SetStatus(wxString() << "Created " << ico_path << " with 16, 32, 48 and 256 pixel images.");
    }
    else
    {
        SetStatus(error);
        wxMessageBox(error, "SVG to Icon", wxOK | wxICON_ERROR, this);
    }

    UpdateConvertState();
}

void SvgToIcoDlg::OnFileChanged(wxFileDirPickerEvent& event)
{
    // Default the destination to the source file's path with an .ico extension so the user does
    // not have to type it. Never overwrite a destination the user has already picked.
    if (event.GetEventObject() == m_fileSvg && m_fileIco->GetPath().empty())
    {
        const wxFileName svg_name(m_fileSvg->GetPath());
        if (svg_name.IsOk())
        {
            wxFileName ico_name = svg_name;
            ico_name.SetExt("ico");
            m_fileIco->SetPath(ico_name.GetFullPath());
        }
    }

    UpdateConvertState();
}

void SvgToIcoDlg::SetStatus(const wxString& message)
{
    m_statusText->SetLabelText(message);
    m_statusText->Wrap(FromDIP(420));
    Fit();
}

void SvgToIcoDlg::UpdateConvertState()
{
    const wxString svg_path = m_fileSvg->GetPath();
    const wxString ico_path = m_fileIco->GetPath();

    bool can_convert = false;
    if (!svg_path.empty() && wxFileName::FileExists(svg_path) && !ico_path.empty())
    {
        // An empty folder means the current directory, which always exists.
        const wxString ico_folder = wxFileName(ico_path).GetPath();
        can_convert = ico_folder.empty() || wxFileName::DirExists(ico_folder);
    }

    if (wxWindow* ok_button = FindWindow(wxID_OK); ok_button != nullptr)
    {
        ok_button->Enable(can_convert);
    }
}

void MainFrame::OnConvertSvgToIco([[maybe_unused]] wxCommandEvent& event)
{
    SvgToIcoDlg svg_to_ico_dlg(this);
    svg_to_ico_dlg.ShowModal();
}
