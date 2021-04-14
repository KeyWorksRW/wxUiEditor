/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource form  (usually a dialog)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "winres_form.h"

#include "uifuncs.h"  // Miscellaneous functions for displaying UI

rcForm::rcForm() {}

void rcForm::ParseDialog(ttlib::textfile& txtfile, size_t& curTxtLine)
{
    auto line = txtfile[curTxtLine].subview();
    auto end = line.find_space();
    if (end == tt::npos)
        throw std::invalid_argument(_tt("Expected an ID then a DIALOG or DIALOGEX."));

    // TODO: [KeyWorks - 10-18-2020] While unusual, I think it's possible the dialog is a quoted string instead of an
    // ID.
    m_ID = line.substr(0, end);

    // Note that we can't change the name here or we won't match with the list of names saved from the dialog that got
    // the resource file.
    m_Name = line.substr(0, end);

    line.remove_prefix(end);
    line.moveto_digit();
    GetDimensions(line);

    for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.is_sameprefix("STYLE"))
        {
            AddStyle(txtfile, curTxtLine);
        }
        else if (line.is_sameprefix("CAPTION"))
        {
            line.moveto_nextword();
            m_Title.ExtractSubString(line);
        }
        else if (line.is_sameprefix("FONT"))
        {
            line.moveto_nextword();
            // TODO: [KeyWorks - 10-18-2020] This needs to be ignored for all "standard" fonts, but might be critical
            // for fonts used for non-English dialogs.
        }
        else if (line.is_sameprefix("BEGIN"))
        {
            ++curTxtLine;
            ParseControls(txtfile, curTxtLine);
            break;
        }
    }

    // TODO: [KeyWorks - 10-18-2020] The last step will be to figure what to use for the base and derived filenames.
}

void rcForm::AddStyle(ttlib::textfile& txtfile, size_t& curTxtLine)
{
    ttlib::cstr style(txtfile[curTxtLine]);

    // A line ending with a , or | character means it is continued onto the next line.

    while (style.back() == ',' || style.back() == '|')
    {
        std::string_view tmp("");
        for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
        {
            tmp = ttlib::find_nonspace(txtfile[curTxtLine]);
            if (!tmp.empty() && tmp[0] != '/')  // ignore blank lines and comments
                break;
        }
        style += tmp;
    }

    if (style.contains("DS_CENTER"))
        m_Center = "wxBOTH";
    if (style.contains("WS_EX_CONTEXTHELP"))
        m_ExStyles += "wxDIALOG_EX_CONTEXTHELP";

    ttlib::cstr original_styles(ttlib::stepover(style));

    if (original_styles.contains("DS_MODALFRAME"))
    {
        m_Styles = "wxDEFAULT_DIALOG_STYLE";
        // It's common for dialogs to duplicate the sytles that DS_MODALFRAM add, so we remove them here to
        // avoid adding them.
        original_styles.Replace("WS_CAPTION", "");
        original_styles.Replace("WS_SYSMENU", "");
        original_styles.Replace("WS_POPUP", "");
    }

    if (original_styles.contains("WS_CAPTION"))
    {
        if (!m_Styles.empty())
            m_Styles += "|";
        m_Styles += "wxCAPTION";
    }

    if (original_styles.find("WS_SYSMENU") != tt::npos)
    {
        if (!m_Styles.empty())
            m_Styles += "|";
        m_Styles += "wxSYSTEM_MENU";
    }

    if (original_styles.find("WS_MAXIMIZEBOX") != tt::npos)
    {
        if (!m_Styles.empty())
            m_Styles += "|";
        m_Styles += "wxMAXIMIZE_BOX";
    }

    if (original_styles.find("WS_MINIMIZEBOX") != tt::npos)
    {
        if (!m_Styles.empty())
            m_Styles += "|";
        m_Styles += "wxMINIMIZE_BOX";
    }

    if (original_styles.find("WS_THICKFRAME") != tt::npos || original_styles.find("WS_SIZEBOX") != tt::npos)
    {
        // There is no thick frame in wxWidgets -- wxBORDER_THEME is at least a double frame
        if (m_WinStyles.size())
            m_WinStyles += "|";
        m_WinStyles += "wxBORDER_THEME";
    }

    if (original_styles.find("WS_CLIPCHILDREN") != tt::npos)
    {
        if (!m_WinStyles.empty())
            m_WinStyles += "|";
        m_WinStyles += "wxCLIP_CHILDREN";
    }

    if (original_styles.find("WS_CLIPSIBLINGS") != tt::npos)
    {
        // Not supported
    }

    if (original_styles.find("WS_POPUP") != tt::npos)
    {
        if (!m_WinStyles.empty())
            m_WinStyles += "|";
        m_WinStyles += "wxPOPUP_WINDOW|wxBORDER_DEFAULT";
    }

    // REVIEW: [KeyWorks - 08-24-2019] Note that we do not convert WS_HSCROLL or WS_VSCROLL.
    // The assumption is that this would be better handled by a sizer parent.

    if (original_styles.find("WS_EX_TOPMOST") != tt::npos)
    {
        if (!m_Styles.empty())
            m_Styles += "|";
        m_Styles += "wxSTAY_ON_TOP";
    }
}

void rcForm::ParseControls(ttlib::textfile& txtfile, size_t& curTxtLine)
{
    for (; curTxtLine < txtfile.size(); ++curTxtLine)
    {
        auto line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        if (line.empty() || line[0] == '/')  // ignore blank lines and comments
            continue;

        // A line ending with a , or | character means it is continued onto the next line.
        while (line.back() == ',' || line.back() == '|')
        {
            std::string_view nextline = {};
            for (++curTxtLine; curTxtLine < txtfile.size(); ++curTxtLine)
            {
                nextline = ttlib::find_nonspace(txtfile[curTxtLine]);
                if (!nextline.empty() && nextline[0] != '/')  // ignore blank lines and comments
                    break;
            }
            if (nextline.empty())
            {
                throw std::invalid_argument("Control line ends with ',' or '|' but no further lines available.");
            }

            // At this point, curTxtLine is pointing to the continuation of the control. Change the line so
            // that it now contains the original line plus the additional line. Continue looping in case
            // the information continues over several lines.
            txtfile[curTxtLine].assign(line);
            txtfile[curTxtLine].append(nextline);
            line = txtfile[curTxtLine].subview(txtfile[curTxtLine].find_nonspace());
        }

        if (line.is_sameprefix("END"))
            break;
        if (line.is_sameprefix("LTEXT") || line.is_sameprefix("CTEXT") || line.is_sameprefix("RTEXT"))
        {
            auto& control = m_ctrls.emplace_back();
            control.ParseStaticCtrl(line);
        }
        else if (line.is_sameprefix("EDITTEXT"))
        {
            auto& control = m_ctrls.emplace_back();
            control.ParseEditCtrl(line);
        }
        else if (line.is_sameprefix("DEFPUSHBUTTON") || line.is_sameprefix("PUSHBUTTON"))
        {
            auto& control = m_ctrls.emplace_back();
            control.ParsePushButton(line);
        }
    }
}

void rcForm::GetDimensions(ttlib::cview line)
{
    if (line[0] == ',')
        line.moveto_digit();

    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.left = ttlib::atoi(line);

    auto pos = line.find_first_of(',');
    if (pos == std::string::npos)
        throw std::invalid_argument("Expected comma-separated dimensions");

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.top = ttlib::atoi(line);

    pos = line.find_first_of(',');
    if (pos == std::string::npos)
        throw std::invalid_argument("Expected comma-separated dimensions");

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.right = ttlib::atoi(line);

    pos = line.find_first_of(',');
    if (pos == std::string::npos)
        throw std::invalid_argument("Expected comma-separated dimensions");

    line.remove_prefix(pos);
    line.moveto_digit();
    if (line.empty() || !ttlib::is_digit(line[0]))
        throw std::invalid_argument("Expected a numeric dimension value");
    m_rc.bottom = ttlib::atoi(line);

    // The resource file uses dialog coordinates which we need to convert into pixel dimensions. Unfortunately,
    // the only way to get an exact algorithm is to being running on Windows which has the exact same font
    // available as is specified for the dialog. The following algorithm assumes 8pt MS Shell Dlg running 1252
    // codepage.

    m_rc.left = (m_rc.left * 6) / 4;
    m_rc.right = (m_rc.right * 6) / 4;
    m_rc.top = (m_rc.top * 13) / 8;
    m_rc.bottom = (m_rc.bottom * 13) / 8;
}
