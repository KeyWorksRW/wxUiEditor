///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include "../wxui/ui_images.h"

#include "preferences_dlg.h"

bool PreferencesDlg::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);
    dlg_sizer->SetMinSize(750, -1);

    auto* notebook = new wxNotebook(this, wxID_ANY);
    {
        wxWithImages::Images bundle_list;
        bundle_list.push_back(wxue_img::bundle_cpp_logo_svg(24, 24));
        bundle_list.push_back(wxue_img::bundle_python_logo_only_svg(24, 24));
        bundle_list.push_back(wxue_img::bundle_ruby_logo_svg(24, 24));
        bundle_list.push_back(wxue_img::bundle_perl_logo_svg(24, 24));
        bundle_list.push_back(wxue_img::bundle_wxlogo_svg(24, 24));
        notebook->SetImages(bundle_list);
    }
    dlg_sizer->Add(notebook, wxSizerFlags(1).Expand().Border(wxALL));

    auto* page_general = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    notebook->AddPage(page_general, "General", true);

    m_general_page_sizer = new wxBoxSizer(wxVERTICAL);

    m_box_dark_settings = new wxBoxSizer(wxHORIZONTAL);

    m_check_dark_mode = new wxCheckBox(page_general, wxID_ANY, "Dark Mode");
    m_check_dark_mode->SetToolTip("Requires closing and restarting wxUiEditor");
    m_box_dark_settings->Add(m_check_dark_mode, wxSizerFlags().Border(wxALL));

    m_check_high_contrast = new wxCheckBox(page_general, wxID_ANY, "High Contrast");
    m_check_high_contrast->SetToolTip("Only used if Dark Mode is selected");
    m_box_dark_settings->Add(m_check_high_contrast, wxSizerFlags().Border(wxALL));
    m_box_dark_settings->ShowItems(false);
    m_general_page_sizer->Add(m_box_dark_settings,
    wxSizerFlags().Expand().Border(wxRIGHT|wxTOP|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

    auto* box_sizer2 = new wxBoxSizer(wxHORIZONTAL);

    auto* box_sizer3 = new wxBoxSizer(wxVERTICAL);

    m_check_right_propgrid = new wxCheckBox(page_general, wxID_ANY, "Property Panel on Right");
    m_check_right_propgrid->SetToolTip("If checked, the Property panel will be moved to the right side");
    box_sizer3->Add(m_check_right_propgrid, wxSizerFlags(1).Border(wxALL));

    m_check_load_last = new wxCheckBox(page_general, wxID_ANY, "Always load last project");
    box_sizer3->Add(m_check_load_last, wxSizerFlags(1).Border(wxALL));

    m_check_fullpath = new wxCheckBox(page_general, wxID_ANY, "Full project path in title bar");
    m_check_fullpath->SetValue(true);
    box_sizer3->Add(m_check_fullpath, wxSizerFlags(1).Border(wxALL));

    auto* checkBox_wakatime = new wxCheckBox(page_general, wxID_ANY, "Enable WakaTime");
    checkBox_wakatime->SetValue(true);
    checkBox_wakatime->SetValidator(wxGenericValidator(&m_isWakaTimeEnabled));
    checkBox_wakatime->SetToolTip(
    "If you have WakaTime installed, checking this will record time spent in the editor as \"designing\". (See https://wakatime.com/about)");
    box_sizer3->Add(checkBox_wakatime, wxSizerFlags().Border(wxALL));

    box_sizer2->Add(box_sizer3, wxSizerFlags().Border(wxALL));

    auto* box_sizer4 = new wxBoxSizer(wxVERTICAL);

    m_check_svg_bitmaps = new wxCheckBox(page_general, wxID_ANY, "Default SVG bitmaps");
    m_check_svg_bitmaps->SetToolTip("If checked, new bitmaps will default to SVG files");
    box_sizer4->Add(m_check_svg_bitmaps, wxSizerFlags().Border(wxALL));

    m_check_prefer_comments = new wxCheckBox(page_general, wxID_ANY, "Generate explanatory comments");
    m_check_prefer_comments->SetValue(true);
    m_check_prefer_comments->SetToolTip("When checked, explanatory comments will sometimes be added to the generated code.");
    box_sizer4->Add(m_check_prefer_comments, wxSizerFlags().Border(wxALL));

    box_sizer2->Add(box_sizer4, wxSizerFlags().Border(wxALL));

    m_general_page_sizer->Add(box_sizer2, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer8 = new wxBoxSizer(wxHORIZONTAL);

    auto* static_text4 = new wxStaticText(page_general, wxID_ANY, "Tree &Icon Size:");
    static_text4->SetToolTip("The size of the icons used in toolbars and tree controls");
    box_sizer8->Add(static_text4, wxSizerFlags().Center().Border(wxALL));

    m_choice_icon_size = new wxChoice(page_general, wxID_ANY);
    m_choice_icon_size->Append("16");
    m_choice_icon_size->Append("18");
    m_choice_icon_size->Append("20");
    m_choice_icon_size->Append("22");
    m_choice_icon_size->Append("24");
    m_choice_icon_size->Append("26");
    m_choice_icon_size->Append("28");
    m_choice_icon_size->Append("30");
    m_choice_icon_size->Append("32");
    m_choice_icon_size->SetStringSelection("18");
    box_sizer8->Add(m_choice_icon_size, wxSizerFlags().Border(wxALL));

    m_general_page_sizer->Add(box_sizer8, wxSizerFlags().Border(wxALL));

    m_box_code_font = new wxBoxSizer(wxHORIZONTAL);

    m_btn_font = new wxCommandLinkButton(page_general, wxID_ANY, "Font", "Font for code panels");
    m_box_code_font->Add(m_btn_font, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

    m_general_page_sizer->Add(m_box_code_font,
    wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));
    page_general->SetSizerAndFit(m_general_page_sizer);

    auto* page_cpp = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    notebook->AddPage(page_cpp, "C++", false, 0);

    auto* page_sizer_2 = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText_3 = new wxStaticText(page_cpp, wxID_ANY, "&Line Length");
    staticText_3->Wrap(200);
    box_sizer->Add(staticText_3, wxSizerFlags().Center().Border(wxALL));

    auto* text_cpp_line_length = new wxTextCtrl(page_cpp, wxID_ANY, "110");
    text_cpp_line_length->SetValidator(wxTextValidator(wxFILTER_DIGITS, &m_cpp_line_length));
    text_cpp_line_length->SetToolTip(
    "Most generated code will not exceed this length. This will be the initial value when a new project is created.");
    box_sizer->Add(text_cpp_line_length, wxSizerFlags().Border(wxALL));

    page_sizer_2->Add(box_sizer, wxSizerFlags().Border(wxALL));

    m_check_cpp_snake_case = new wxCheckBox(page_cpp, wxID_ANY, "Default variables should use snake_case");
    m_check_cpp_snake_case->SetToolTip(
    "By default, default variables use camelCase. Check this to use snake_case instead (e.g., foo_bar instead of fooBar)");
    page_sizer_2->Add(m_check_cpp_snake_case, wxSizerFlags().Border(wxALL));

    auto* box_sizer_5 = new wxBoxSizer(wxHORIZONTAL);

    auto* static_text = new wxStaticText(page_cpp, wxID_ANY, "wxWidgets version");
    box_sizer_5->Add(static_text, wxSizerFlags().Center().Border(wxALL));

    m_choice_cpp_version = new wxChoice(page_cpp, wxID_ANY);
    m_choice_cpp_version->Append("3.1.0");
    m_choice_cpp_version->Append("3.2.0");
    m_choice_cpp_version->Append("3.3.0");
    m_choice_cpp_version->SetStringSelection("3.2");
    m_choice_cpp_version->SetToolTip("Code requiring a newer version then this will be placed in a conditional block.");
    box_sizer_5->Add(m_choice_cpp_version, wxSizerFlags().Border(wxALL));

    page_sizer_2->Add(box_sizer_5, wxSizerFlags().Border(wxALL));

    auto* grid_sizer3 = new wxGridSizer(2, 0, 0);

    auto* staticText_7 = new wxStaticText(page_cpp, wxID_ANY, "wxWidgets &keyword color:");
    grid_sizer3->Add(staticText_7, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_cpp = new wxColourPickerCtrl(page_cpp, wxID_ANY, wxColour("#C76605"));
    grid_sizer3->Add(m_colour_cpp, wxSizerFlags().Border(wxALL));

    auto* staticText9 = new wxStaticText(page_cpp, wxID_ANY, "&C++ keyword color:");
    grid_sizer3->Add(staticText9, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_cpp_keyword = new wxColourPickerCtrl(page_cpp, wxID_ANY, wxColour("#0000FF"));
    grid_sizer3->Add(m_colour_cpp_keyword, wxSizerFlags().Border(wxALL));

    auto* staticText10 = new wxStaticText(page_cpp, wxID_ANY, "&Comment color:");
    grid_sizer3->Add(staticText10, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_cpp_comment = new wxColourPickerCtrl(page_cpp, wxID_ANY, wxColour("#008000"));
    grid_sizer3->Add(m_colour_cpp_comment, wxSizerFlags().Border(wxALL));

    auto* staticText11 = new wxStaticText(page_cpp, wxID_ANY, "&Number color:");
    grid_sizer3->Add(staticText11, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_cpp_number = new wxColourPickerCtrl(page_cpp, wxID_ANY, wxColour("#FF0000"));
    grid_sizer3->Add(m_colour_cpp_number, wxSizerFlags().Border(wxALL));

    auto* staticText12 = new wxStaticText(page_cpp, wxID_ANY, "&String color:");
    grid_sizer3->Add(staticText12, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_cpp_string = new wxColourPickerCtrl(page_cpp, wxID_ANY, wxColour("#008000"));
    grid_sizer3->Add(m_colour_cpp_string, wxSizerFlags().Border(wxALL));

    page_sizer_2->Add(grid_sizer3, wxSizerFlags().Border(wxALL));
    page_cpp->SetSizerAndFit(page_sizer_2);

    auto* page_python = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    notebook->AddPage(page_python, "Python", false, 1);

    auto* page_sizer_3 = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_3 = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText_4 = new wxStaticText(page_python, wxID_ANY, "&Line Length");
    staticText_4->Wrap(200);
    box_sizer_3->Add(staticText_4, wxSizerFlags().Center().Border(wxALL));

    auto* text_python_line_length = new wxTextCtrl(page_python, wxID_ANY, "90");
    text_python_line_length->SetValidator(wxTextValidator(wxFILTER_DIGITS, &m_python_line_length));
    text_python_line_length->SetToolTip(
    "Most generated code will not exceed this length. This will be the initial value when a new project is created.");
    box_sizer_3->Add(text_python_line_length, wxSizerFlags().Border(wxALL));

    page_sizer_3->Add(box_sizer_3, wxSizerFlags().Border(wxALL));

    auto* box_sizer7 = new wxBoxSizer(wxHORIZONTAL);

    auto* static_text3 = new wxStaticText(page_python, wxID_ANY, "wxPython version");
    box_sizer7->Add(static_text3, wxSizerFlags().Center().Border(wxALL));

    m_choice_python_version = new wxChoice(page_python, wxID_ANY);
    m_choice_python_version->Append("4.2.0");
    m_choice_python_version->SetStringSelection("4.2.0");
    m_choice_python_version->SetToolTip("Code requiring a newer version then this will be placed in a conditional block.");
    box_sizer7->Add(m_choice_python_version, wxSizerFlags().Border(wxALL));

    page_sizer_3->Add(box_sizer7, wxSizerFlags().Border(wxALL));

    auto* grid_sizer2 = new wxGridSizer(2, 0, 0);

    auto* staticText = new wxStaticText(page_python, wxID_ANY, "&wxWidgets keyword color:");
    grid_sizer2->Add(staticText, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_python = new wxColourPickerCtrl(page_python, wxID_ANY, wxColour("#FF00FF"));
    grid_sizer2->Add(m_colour_python, wxSizerFlags().Border(wxALL));

    auto* staticText5 = new wxStaticText(page_python, wxID_ANY, "&wxWidgets keyword color:");
    grid_sizer2->Add(staticText5, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_python_keyword = new wxColourPickerCtrl(page_python, wxID_ANY, wxColour("#0000FF"));
    grid_sizer2->Add(m_colour_python_keyword, wxSizerFlags().Border(wxALL));

    auto* staticText6 = new wxStaticText(page_python, wxID_ANY, "&Comment color:");
    grid_sizer2->Add(staticText6, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_python_comment = new wxColourPickerCtrl(page_python, wxID_ANY, wxColour("#008000"));
    grid_sizer2->Add(m_colour_python_comment, wxSizerFlags().Border(wxALL));

    auto* staticText7 = new wxStaticText(page_python, wxID_ANY, "&Number color:");
    grid_sizer2->Add(staticText7, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_python_number = new wxColourPickerCtrl(page_python, wxID_ANY, wxColour("#FF0000"));
    grid_sizer2->Add(m_colour_python_number, wxSizerFlags().Border(wxALL));

    auto* staticText8 = new wxStaticText(page_python, wxID_ANY, "&String color:");
    grid_sizer2->Add(staticText8, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_python_string = new wxColourPickerCtrl(page_python, wxID_ANY, wxColour("#008000"));
    grid_sizer2->Add(m_colour_python_string, wxSizerFlags().Border(wxALL));

    page_sizer_3->Add(grid_sizer2, wxSizerFlags().Border(wxALL));
    page_python->SetSizerAndFit(page_sizer_3);

    auto* page_ruby = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    notebook->AddPage(page_ruby, "Ruby", false, 2);

    auto* page_sizer_4 = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_4 = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText_5 = new wxStaticText(page_ruby, wxID_ANY, "&Line Length");
    staticText_5->Wrap(200);
    box_sizer_4->Add(staticText_5, wxSizerFlags().Center().Border(wxALL));

    auto* text_ruby_line_length = new wxTextCtrl(page_ruby, wxID_ANY, "80");
    text_ruby_line_length->SetValidator(wxTextValidator(wxFILTER_DIGITS, &m_ruby_line_length));
    text_ruby_line_length->SetToolTip(
    "Most generated code will not exceed this length. This will be the initial value when a new project is created.");
    box_sizer_4->Add(text_ruby_line_length, wxSizerFlags().Border(wxALL));

    page_sizer_4->Add(box_sizer_4, wxSizerFlags().Border(wxALL));

    auto* box_sizer6 = new wxBoxSizer(wxHORIZONTAL);

    auto* static_text2 = new wxStaticText(page_ruby, wxID_ANY, "wxRuby version");
    box_sizer6->Add(static_text2, wxSizerFlags().Center().Border(wxALL));

    m_choice_ruby_version = new wxChoice(page_ruby, wxID_ANY);
    m_choice_ruby_version->Append("1.0.0");
    m_choice_ruby_version->Append("1.2.0");
    m_choice_ruby_version->Append("1.2.1");
    m_choice_ruby_version->SetStringSelection("1.2.1");
    m_choice_ruby_version->SetToolTip("Code requiring a newer version then this will be placed in a conditional block.");
    box_sizer6->Add(m_choice_ruby_version, wxSizerFlags().Border(wxALL));

    page_sizer_4->Add(box_sizer6, wxSizerFlags().Border(wxALL));

    auto* grid_sizer4 = new wxGridSizer(2, 0, 0);

    auto* staticText13 = new wxStaticText(page_ruby, wxID_ANY, "wxWidgets &keyword color:");
    grid_sizer4->Add(staticText13, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_ruby = new wxColourPickerCtrl(page_ruby, wxID_ANY, wxColour("#FF00FF"));
    grid_sizer4->Add(m_colour_ruby, wxSizerFlags().Border(wxALL));

    auto* staticText15 = new wxStaticText(page_ruby, wxID_ANY, "&Comment color:");
    grid_sizer4->Add(staticText15, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_ruby_comment = new wxColourPickerCtrl(page_ruby, wxID_ANY, wxColour("#008000"));
    grid_sizer4->Add(m_colour_ruby_comment, wxSizerFlags().Border(wxALL));

    auto* staticText16 = new wxStaticText(page_ruby, wxID_ANY, "&Number color:");
    grid_sizer4->Add(staticText16, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_ruby_number = new wxColourPickerCtrl(page_ruby, wxID_ANY, wxColour("#FF0000"));
    grid_sizer4->Add(m_colour_ruby_number, wxSizerFlags().Border(wxALL));

    auto* staticText17 = new wxStaticText(page_ruby, wxID_ANY, "&String color:");
    grid_sizer4->Add(staticText17, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_ruby_string = new wxColourPickerCtrl(page_ruby, wxID_ANY, wxColour("#008000"));
    grid_sizer4->Add(m_colour_ruby_string, wxSizerFlags().Border(wxALL));

    page_sizer_4->Add(grid_sizer4, wxSizerFlags().Border(wxALL));
    page_ruby->SetSizerAndFit(page_sizer_4);

    auto* page_perl = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    notebook->AddPage(page_perl, "Perl", false, 3);

    auto* page_sizer5 = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer13 = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText27 = new wxStaticText(page_perl, wxID_ANY, "&Line Length");
    staticText27->Wrap(200);
    box_sizer13->Add(staticText27, wxSizerFlags().Center().Border(wxALL));

    auto* text_perl_line_length = new wxTextCtrl(page_perl, wxID_ANY, "80");
    text_perl_line_length->SetValidator(wxTextValidator(wxFILTER_DIGITS, &m_perl_line_length));
    text_perl_line_length->SetToolTip(
    "Most generated code will not exceed this length. This will be the initial value when a new project is created.");
    box_sizer13->Add(text_perl_line_length, wxSizerFlags().Border(wxALL));

    page_sizer5->Add(box_sizer13, wxSizerFlags().Border(wxALL));

    auto* box_sizer14 = new wxBoxSizer(wxHORIZONTAL);

    auto* static_text7 = new wxStaticText(page_perl, wxID_ANY, "wxPerl version");
    box_sizer14->Add(static_text7, wxSizerFlags().Center().Border(wxALL));

    m_choice_perl_version = new wxChoice(page_perl, wxID_ANY);
    m_choice_perl_version->Append("0.91");
    m_choice_perl_version->Append("3.2.0");
    m_choice_perl_version->SetStringSelection("3.2.0");
    m_choice_perl_version->SetToolTip("Code requiring a newer version then this will be placed in a conditional block.");
    box_sizer14->Add(m_choice_perl_version, wxSizerFlags().Border(wxALL));

    page_sizer5->Add(box_sizer14, wxSizerFlags().Border(wxALL));

    auto* grid_sizer7 = new wxGridSizer(2, 0, 0);

    auto* staticText28 = new wxStaticText(page_perl, wxID_ANY, "wxWidgets &keyword color:");
    grid_sizer7->Add(staticText28, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_perl = new wxColourPickerCtrl(page_perl, wxID_ANY, wxColour("#FF00FF"));
    grid_sizer7->Add(m_colour_perl, wxSizerFlags().Border(wxALL));

    auto* staticText29 = new wxStaticText(page_perl, wxID_ANY, "&Comment color:");
    grid_sizer7->Add(staticText29, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_perl_comment = new wxColourPickerCtrl(page_perl, wxID_ANY, wxColour("#008000"));
    grid_sizer7->Add(m_colour_perl_comment, wxSizerFlags().Border(wxALL));

    auto* staticText30 = new wxStaticText(page_perl, wxID_ANY, "&Number color:");
    grid_sizer7->Add(staticText30, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_perl_number = new wxColourPickerCtrl(page_perl, wxID_ANY, wxColour("#FF0000"));
    grid_sizer7->Add(m_colour_perl_number, wxSizerFlags().Border(wxALL));

    auto* staticText31 = new wxStaticText(page_perl, wxID_ANY, "&String color:");
    grid_sizer7->Add(staticText31, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_perl_string = new wxColourPickerCtrl(page_perl, wxID_ANY, wxColour("#008000"));
    grid_sizer7->Add(m_colour_perl_string, wxSizerFlags().Border(wxALL));

    page_sizer5->Add(grid_sizer7, wxSizerFlags().Border(wxALL));
    page_perl->SetSizerAndFit(page_sizer5);

    auto* page_xrc = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    notebook->AddPage(page_xrc, "XRC", false, 4);

    auto* page_sizer2 = new wxBoxSizer(wxVERTICAL);

    auto* grid_sizer = new wxGridSizer(2, 0, 0);

    auto* staticText3 = new wxStaticText(page_xrc, wxID_ANY, "&Attribute color:");
    grid_sizer->Add(staticText3, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_xrc_attribute = new wxColourPickerCtrl(page_xrc, wxID_ANY, wxColour("#FF00FF"));
    grid_sizer->Add(m_colour_xrc_attribute, wxSizerFlags().Border(wxALL));

    auto* staticText4 = new wxStaticText(page_xrc, wxID_ANY, "&String color:");
    grid_sizer->Add(staticText4, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_xrc_string = new wxColourPickerCtrl(page_xrc, wxID_ANY, wxColour("#008000"));
    grid_sizer->Add(m_colour_xrc_string, wxSizerFlags().Border(wxALL));

    auto* staticText2 = new wxStaticText(page_xrc, wxID_ANY, "&Tag color:");
    grid_sizer->Add(staticText2, wxSizerFlags().CenterVertical().Border(wxALL));

    m_colour_xrc_tag = new wxColourPickerCtrl(page_xrc, wxID_ANY, wxColour("#0000FF"));
    grid_sizer->Add(m_colour_xrc_tag, wxSizerFlags().Border(wxALL));

    page_sizer2->Add(grid_sizer, wxSizerFlags().Border(wxALL));

    auto* box_sizer5 = new wxBoxSizer(wxHORIZONTAL);

    page_sizer2->Add(box_sizer5, wxSizerFlags().Border(wxALL));
    page_xrc->SetSizerAndFit(page_sizer2);

    auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    dlg_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    if (pos != wxDefaultPosition)
    {
        SetPosition(FromDIP(pos));
    }
    if (size == wxDefaultSize)
    {
        SetSizerAndFit(dlg_sizer);
    }
    else
    {
        SetSizer(dlg_sizer);
        if (size.x == wxDefaultCoord || size.y == wxDefaultCoord)
        {
            Fit();
        }
        SetSize(FromDIP(size));
        Layout();
    }
    Centre(wxBOTH);

    // Event handlers
    Bind(wxEVT_BUTTON, &PreferencesDlg::OnOK, this, wxID_OK);
    m_btn_font->Bind(wxEVT_BUTTON, &PreferencesDlg::OnFontButton, this);
    Bind(wxEVT_INIT_DIALOG, &PreferencesDlg::OnInit, this);

    return true;
}

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************

/////////////////// Non-generated Copyright/License Info ////////////////////
// Purpose:
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>  // A wxWidgets implementation of Scintilla.  This class is the

#include "../customprops/font_prop_dlg.h"  // FontStringProperty class
#include "../panels/base_panel.h"          // BasePanel class
#include "font_prop.h"                     // FontProperty class
#include "mainframe.h"                     // CMainFrame -- Main window frame
#include "preferences.h"                   // Set/Get wxUiEditor preferences
#include "project_handler.h"               // ProjectHandler class

void PreferencesDlg::OnInit(wxInitDialogEvent& event)
{
    m_check_dark_mode->SetValue(UserPrefs.is_DarkMode());
    m_check_high_contrast->SetValue(UserPrefs.is_HighContrast());
    m_check_fullpath->SetValue(UserPrefs.is_FullPathTitle());
    m_check_prefer_comments->SetValue(UserPrefs.is_AddComments());
    m_check_svg_bitmaps->SetValue(UserPrefs.is_SvgImages());

    m_check_cpp_snake_case->SetValue(UserPrefs.is_CppSnakeCase());

    m_check_load_last->SetValue(UserPrefs.is_LoadLastProject());
    m_check_right_propgrid->SetValue(UserPrefs.is_RightPropGrid());
    m_isWakaTimeEnabled = UserPrefs.is_WakaTimeEnabled();

    m_choice_cpp_version->SetStringSelection(UserPrefs.get_CppWidgetsVersion().make_wxString());
    m_choice_python_version->SetStringSelection(UserPrefs.get_PythonVersion().make_wxString());
    m_choice_ruby_version->SetStringSelection(UserPrefs.get_RubyVersion().make_wxString());

    m_colour_cpp->SetColour(UserPrefs.get_CppColour());
    m_colour_cpp_comment->SetColour(UserPrefs.get_CppCommentColour());
    m_colour_cpp_keyword->SetColour(UserPrefs.get_CppKeywordColour());
    m_colour_cpp_number->SetColour(UserPrefs.get_CppNumberColour());
    m_colour_cpp_string->SetColour(UserPrefs.get_CppStringColour());

    m_colour_python->SetColour(UserPrefs.get_PythonColour());
    m_colour_python_comment->SetColour(UserPrefs.get_PythonCommentColour());
    m_colour_python_keyword->SetColour(UserPrefs.get_PythonKeywordColour());
    m_colour_python_number->SetColour(UserPrefs.get_PythonNumberColour());
    m_colour_python_string->SetColour(UserPrefs.get_PythonStringColour());

    m_colour_ruby->SetColour(UserPrefs.get_RubyColour());
    m_colour_ruby_comment->SetColour(UserPrefs.get_RubyCommentColour());
    m_colour_ruby_number->SetColour(UserPrefs.get_RubyNumberColour());
    m_colour_ruby_string->SetColour(UserPrefs.get_RubyStringColour());

    m_colour_xrc_attribute->SetColour(UserPrefs.get_XrcAttributeColour());
    m_colour_xrc_string->SetColour(UserPrefs.get_XrcDblStringColour());
    m_colour_xrc_tag->SetColour(UserPrefs.get_XrcTagColour());

    m_cpp_line_length = std::to_string(UserPrefs.get_CppLineLength());
    m_python_line_length = std::to_string(UserPrefs.get_PythonLineLength());
    m_ruby_line_length = std::to_string(UserPrefs.get_RubyLineLength());

    m_choice_icon_size->SetStringSelection(std::to_string(UserPrefs.get_IconSize()));

    FontProperty font_prop(UserPrefs.get_CodeDisplayFont().ToStdView());
    m_btn_font->SetMainLabel(font_prop.as_wxString());

#if defined(__WXMSW__)
    m_box_dark_settings->ShowItems(true);
#endif

    m_general_page_sizer->Layout();
    Fit();

    // This will transfer data from the validator variables to the controls
    event.Skip();
}

void PreferencesDlg::OnFontButton(wxCommandEvent& WXUNUSED(event))
{
    FontPropDlg dlg(this, m_btn_font->GetMainLabel());
    if (dlg.ShowModal() == wxID_OK)
    {
        FontProperty font_prop(dlg.GetFontDescription());
        m_btn_font->SetMainLabel(dlg.GetResults());
        m_general_page_sizer->Layout();
        Fit();
    }
}

void PreferencesDlg::OnOK(wxCommandEvent& WXUNUSED(event))
{
    if (!Validate() || !TransferDataFromWindow())
    {
        // TODO: [Randalphwa - 08-01-2023]
        // If either of these fail, there's no warning to the user
        return;
    }

    bool is_prop_grid_changed = false;
    bool is_dark_changed = false;
    bool is_fullpath_changed = false;

    if (m_check_dark_mode->GetValue() != UserPrefs.is_DarkMode())
        is_dark_changed = true;
    if (m_check_high_contrast->GetValue() != UserPrefs.is_HighContrast())
        is_dark_changed = true;
    if (m_check_fullpath->GetValue() != UserPrefs.is_FullPathTitle())
        is_fullpath_changed = true;

    if (m_choice_cpp_version->GetStringSelection().ToStdString() != UserPrefs.get_CppWidgetsVersion())
    {
        UserPrefs.set_CppWidgetsVersion(m_choice_cpp_version->GetStringSelection().ToStdString());
        Project.getProjectNode()->modifyProperty(prop_wxWidgets_version, UserPrefs.get_CppWidgetsVersion());
    }

    if (m_choice_python_version->GetStringSelection().ToStdString() != UserPrefs.get_PythonVersion())
    {
        UserPrefs.set_PythonVersion(m_choice_python_version->GetStringSelection().ToStdString());
        Project.getProjectNode()->modifyProperty(prop_wxPython_version, UserPrefs.get_PythonVersion());
    }

    if (m_choice_ruby_version->GetStringSelection().ToStdString() != UserPrefs.get_RubyVersion())
    {
        UserPrefs.set_RubyVersion(m_choice_ruby_version->GetStringSelection().ToStdString());
        Project.getProjectNode()->modifyProperty(prop_wxRuby_version, UserPrefs.get_RubyVersion());
    }

    UserPrefs.set_DarkModePending(
        Prefs::PENDING_DARK_MODE_ENABLE |
        (m_check_dark_mode->GetValue() ? Prefs::PENDING_DARK_MODE_ON : Prefs::PENDING_DARK_MODE_OFF));
    UserPrefs.set_HighContrast(m_check_high_contrast->GetValue());
    UserPrefs.set_FullPathTitle(m_check_fullpath->GetValue());
    UserPrefs.set_AddComments(m_check_prefer_comments->GetValue());
    UserPrefs.set_SvgImages(m_check_svg_bitmaps->GetValue());

    UserPrefs.set_CppSnakeCase(m_check_cpp_snake_case->GetValue());
    UserPrefs.set_LoadLastProject(m_check_load_last->GetValue());
    UserPrefs.set_RightPropGrid(m_check_right_propgrid->GetValue());
    UserPrefs.set_WakaTimeEnabled(m_isWakaTimeEnabled);

    if (auto panel = wxGetFrame().GetCppPanel(); panel)
    {
        if (UserPrefs.get_CppColour() != m_colour_cpp->GetColour())
        {
            UserPrefs.set_CppColour(m_colour_cpp->GetColour());
            panel->SetColor(wxSTC_C_WORD, m_colour_cpp->GetColour());
        }

        if (UserPrefs.get_CppCommentColour() != m_colour_cpp_comment->GetColour())
        {
            UserPrefs.set_CppCommentColour(m_colour_cpp_comment->GetColour());
            panel->SetColor(wxSTC_C_COMMENTLINE, m_colour_cpp_comment->GetColour());
        }

        if (UserPrefs.get_CppKeywordColour() != m_colour_cpp_keyword->GetColour())
        {
            UserPrefs.set_CppKeywordColour(m_colour_cpp_keyword->GetColour());
            panel->SetColor(wxSTC_C_WORD2, m_colour_cpp_keyword->GetColour());
        }

        if (UserPrefs.get_CppNumberColour() != m_colour_cpp_number->GetColour())
        {
            UserPrefs.set_CppNumberColour(m_colour_cpp_number->GetColour());
            panel->SetColor(wxSTC_C_NUMBER, m_colour_cpp_number->GetColour());
        }

        if (UserPrefs.get_CppStringColour() != m_colour_cpp_string->GetColour())
        {
            UserPrefs.set_CppStringColour(m_colour_cpp_string->GetColour());
            panel->SetColor(wxSTC_C_STRING, m_colour_cpp_string->GetColour());
        }
    }

    if (auto panel = wxGetFrame().GetPythonPanel(); panel)
    {
        if (UserPrefs.get_PythonColour() != m_colour_python->GetColour())
        {
            UserPrefs.set_PythonColour(m_colour_python->GetColour());
            panel->SetColor(wxSTC_P_WORD, m_colour_python->GetColour());
        }

        if (UserPrefs.get_PythonKeywordColour() != m_colour_python_keyword->GetColour())
        {
            UserPrefs.set_PythonKeywordColour(m_colour_python_keyword->GetColour());
            panel->SetColor(wxSTC_P_WORD2, m_colour_python_keyword->GetColour());
        }

        if (UserPrefs.get_PythonCommentColour() != m_colour_python_comment->GetColour())
        {
            UserPrefs.set_PythonCommentColour(m_colour_python_comment->GetColour());
            panel->SetColor(wxSTC_P_COMMENTLINE, m_colour_python_comment->GetColour());
        }

        if (UserPrefs.get_PythonNumberColour() != m_colour_python_number->GetColour())
        {
            UserPrefs.set_PythonNumberColour(m_colour_python_number->GetColour());
            panel->SetColor(wxSTC_P_NUMBER, m_colour_python_number->GetColour());
        }

        if (UserPrefs.get_PythonStringColour() != m_colour_python_string->GetColour())
        {
            UserPrefs.set_PythonStringColour(m_colour_python_string->GetColour());
            panel->SetColor(wxSTC_P_STRING, m_colour_python_string->GetColour());
        }
    }

    if (auto panel = wxGetFrame().GetRubyPanel(); panel)
    {
        if (UserPrefs.get_RubyColour() != m_colour_ruby->GetColour())
        {
            UserPrefs.set_RubyColour(m_colour_ruby->GetColour());
            panel->SetColor(wxSTC_RB_WORD, m_colour_ruby->GetColour());
        }

        if (UserPrefs.get_RubyCommentColour() != m_colour_ruby_comment->GetColour())
        {
            UserPrefs.set_RubyCommentColour(m_colour_ruby_comment->GetColour());
            panel->SetColor(wxSTC_RB_COMMENTLINE, m_colour_ruby_comment->GetColour());
        }

        if (UserPrefs.get_RubyNumberColour() != m_colour_ruby_number->GetColour())
        {
            UserPrefs.set_RubyNumberColour(m_colour_ruby_number->GetColour());
            panel->SetColor(wxSTC_RB_NUMBER, m_colour_ruby_number->GetColour());
        }

        if (UserPrefs.get_RubyStringColour() != m_colour_ruby_string->GetColour())
        {
            UserPrefs.set_RubyStringColour(m_colour_ruby_string->GetColour());
            panel->SetColor(wxSTC_RB_STRING, m_colour_ruby_string->GetColour());
            panel->SetColor(wxSTC_RB_STRING_Q, m_colour_ruby_string->GetColour());
            panel->SetColor(wxSTC_RB_STRING_QQ, m_colour_ruby_string->GetColour());
            panel->SetColor(wxSTC_RB_STRING_QX, m_colour_ruby_string->GetColour());
            panel->SetColor(wxSTC_RB_STRING_QR, m_colour_ruby_string->GetColour());
            panel->SetColor(wxSTC_RB_STRING_QW, m_colour_ruby_string->GetColour());
        }
    }

    if (auto panel = wxGetFrame().GetXrcPanel(); panel)
    {
        if (UserPrefs.get_XrcAttributeColour() != m_colour_xrc_attribute->GetColour())
        {
            UserPrefs.set_XrcAttributeColour(m_colour_xrc_attribute->GetColour());
            panel->SetColor(wxSTC_H_ATTRIBUTE, m_colour_xrc_attribute->GetColour());
        }

        if (UserPrefs.get_XrcDblStringColour() != m_colour_xrc_string->GetColour())
        {
            UserPrefs.set_XrcDblStringColour(m_colour_xrc_string->GetColour());
            panel->SetColor(wxSTC_H_DOUBLESTRING, m_colour_xrc_string->GetColour());
        }

        if (UserPrefs.get_XrcTagColour() != m_colour_xrc_tag->GetColour())
        {
            UserPrefs.set_XrcTagColour(m_colour_xrc_tag->GetColour());
            panel->SetColor(wxSTC_H_TAG, m_colour_xrc_tag->GetColour());
        }
    }

    if (UserPrefs.get_CodeDisplayFont() != m_btn_font->GetMainLabel().utf8_string())
    {
        FontProperty font_prop(tt_string_view(m_btn_font->GetMainLabel().utf8_string()));
        auto font = font_prop.GetFont();
        UserPrefs.set_CodeDisplayFont(font_prop.as_string());
        wxGetFrame().GetCppPanel()->SetCodeFont(font);
        wxGetFrame().GetPythonPanel()->SetCodeFont(font);
        wxGetFrame().GetRubyPanel()->SetCodeFont(font);
        wxGetFrame().GetXrcPanel()->SetCodeFont(font);
    }

    auto line_length = tt::atoi(m_cpp_line_length.ToStdString());

    auto fix_line_length = [&]()
    {
        if (line_length < 40)
            line_length = 40;
        else if (line_length > 300)
            line_length = 300;
    };

    fix_line_length();
    UserPrefs.set_CppLineLength(line_length);

    line_length = tt::atoi(m_python_line_length.ToStdString());
    fix_line_length();
    UserPrefs.set_PythonLineLength(line_length);

    line_length = tt::atoi(m_ruby_line_length.ToStdString());
    fix_line_length();
    UserPrefs.set_RubyLineLength(line_length);

    auto old_size = UserPrefs.get_IconSize();
    UserPrefs.set_IconSize(tt::atoi(m_choice_icon_size->GetStringSelection().ToStdString()));
    bool is_icon_size_changed = old_size != UserPrefs.get_IconSize();

    // UserPrefs.set_CodeDisplayFont(m_code_font_picker->GetSelectedFontInfo());

    UserPrefs.WriteConfig();

    if (is_prop_grid_changed || is_dark_changed || is_icon_size_changed)
    {
        tt_string msg("You must close and reopen wxUiEditor for");
        if (is_prop_grid_changed)
        {
            msg += " the Property Panel";
            if (is_dark_changed)
                msg += " and Dark Mode";
        }
        if (is_dark_changed)
            msg += " the Dark Mode";
        if (is_icon_size_changed)
        {
            if (!msg.ends_with("for"))
                msg += " and";
            msg += " Icon Size";
        }

        msg += " setting(s) to take effect.";

        wxMessageBox(msg);
    }

    if (is_fullpath_changed)
    {
        wxGetFrame().UpdateFrame();
    }

    EndModal(wxID_OK);
}
