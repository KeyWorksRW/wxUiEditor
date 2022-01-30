/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/activityindicator.h>  // wxActivityIndicator declaration.
#include <wx/animate.h>            // wxAnimation and wxAnimationCtrl
#include <wx/bannerwindow.h>       // wxBannerWindow class declaration
#include <wx/event.h>              // Event classes
#include <wx/gauge.h>              // wxGauge interface
#include <wx/generic/statbmpg.h>   // wxGenericStaticBitmap header
#include <wx/hyperlink.h>          // Hyperlink control
#include <wx/infobar.h>            // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/slider.h>             // wxSlider interface
#include <wx/statbmp.h>            // wxStaticBitmap class interface
#include <wx/statline.h>           // wxStaticLine class interface
#include <wx/statusbr.h>           // wxStatusBar class interface

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "bitmaps.h"      // Contains various images handling functions
#include "gen_common.h"   // GeneratorLibrary -- Generator classes
#include "mainapp.h"      // App -- Main application class
#include "node.h"         // Node class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties
#include "write_code.h"   // WriteCode -- Write code to Scintilla or file

#include "misc_widgets.h"

//////////////////////////////////////////  ActivityIndicatorGenerator  //////////////////////////////////////////

wxObject* ActivityIndicatorGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxActivityIndicator(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                          DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Start();

    return widget;
}

std::optional<ttlib::cstr> ActivityIndicatorGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

bool ActivityIndicatorGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/activityindicator.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  AnimationGenerator  //////////////////////////////////////////

wxObject* AnimationGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto animation = node->prop_as_wxAnimation(prop_animation);
    auto widget = new wxAnimationCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, animation, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    if (animation.IsOk())
        widget->Play();

    return widget;
}

std::optional<ttlib::cstr> AnimationGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->HasValue(prop_animation))
    {
        ttlib::multiview parts(node->prop_as_string(prop_animation), ';');
        ttlib::cstr name(parts[IndexImage].filename());
        name.remove_extension();
        name.LeftTrim();
        if (parts[IndexType].is_sameprefix("Embed"))
        {
            auto embed = wxGetApp().GetProjectSettings()->GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                name = "wxue_img::" + embed->array_name;
            }
        }

        code << "GetAnimFromHdr(" << name << ", sizeof(" << name << "))";
    }
    else
    {
        code << "wxNullAnimation";
    }

    GeneratePosSizeFlags(node, code, false, "wxAC_DEFAULT_STYLE");
    if (node->HasValue(prop_inactive_bitmap))
    {
        code << "\n\t" << node->get_node_name() << "->SetInactiveBitmap(";
        code << GenerateBitmapCode(node->prop_as_string(prop_inactive_bitmap)) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> AnimationGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    if (node->prop_as_bool(prop_play))
    {
        ttlib::cstr code;
        code << node->get_node_name() << "->Play();";
        return code;
    }
    else
    {
        return {};
    }
}

bool AnimationGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/animate.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  BannerWindowGenerator  //////////////////////////////////////////

wxObject* BannerWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxBannerWindow(wxStaticCast(parent, wxWindow),
                                     (wxDirection) g_NodeCreator.GetConstantAsInt(node->prop_as_string(prop_direction)));

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->prop_as_wxBitmap(prop_bitmap));
    }

    else if (node->HasValue(prop_start_colour) && node->HasValue(prop_end_colour))
    {
        widget->SetGradient(node->prop_as_wxColour(prop_start_colour), node->prop_as_wxColour(prop_end_colour));
    }

    if (node->HasValue(prop_title) || node->HasValue(prop_message))
    {
        widget->SetText(node->prop_as_wxString(prop_title), node->prop_as_wxString(prop_message));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> BannerWindowGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_direction) << ");";

    return code;
}

std::optional<ttlib::cstr> BannerWindowGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;
    if (node->HasValue(prop_bitmap))
    {
        code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";
    }
    else if (node->HasValue(prop_start_colour) && node->HasValue(prop_end_colour))
    {
        auto& start_colour = node->prop_as_string(prop_start_colour);
        code << node->get_node_name() << "->SetGradient(";
        if (start_colour.contains("wx"))
            code << "wxSystemSettings::GetColour(" << start_colour << ")";
        else
        {
            wxColour colour = ConvertToColour(start_colour);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue());
        }

        code << ",\n\t";

        auto& end_colour = node->prop_as_string(prop_end_colour);
        if (end_colour.contains("wx"))
            code << "wxSystemSettings::GetColour(" << end_colour << "));";
        else
        {
            wxColour colour = ConvertToColour(end_colour);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i));", colour.Red(), colour.Green(), colour.Blue());
        }
        auto_indent = indent::auto_keep_whitespace;
    }

    if (node->HasValue(prop_title) || node->HasValue(prop_message))
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetText(" << GenerateQuotedString(node->prop_as_string(prop_title)) << ",\n\t";
        code << GenerateQuotedString(node->prop_as_string(prop_message)) << ");";
        auto_indent = indent::auto_keep_whitespace;
    }

    return code;
}

bool BannerWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/bannerwindow.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  StaticLineGenerator  //////////////////////////////////////////

wxObject* StaticLineGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxStaticLine(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> StaticLineGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxStaticLine(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    if (node->prop_as_string(prop_style) != "wxLI_HORIZONTAL")
    {
        GeneratePosSizeFlags(node, code);
    }
    else
    {
        auto pos = node->prop_as_wxPoint(prop_pos);
        auto size = node->prop_as_wxPoint(prop_size);
        auto& win_name = node->prop_as_string(prop_window_name);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (win_name.empty() && win_style.empty() && size.x == -1 && size.y == -1 && pos.x == -1 && pos.y == -1)
        {
            code << ");";
            // A lot easier to remove the id once we get here then to add the logic above to not add it
            code.Replace(", wxID_ANY", "");
            return code;
        }

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
    }

    return code;
}

bool StaticLineGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/statline.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  StatusBarGenerator  //////////////////////////////////////////

wxObject* StatusBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto org_style = GetStyleInt(node);
    // Don't display the gripper as it can resize our main window rather than just the mockup window
    auto widget = new wxStatusBar(wxStaticCast(parent, wxWindow), wxID_ANY, (org_style &= ~wxSTB_SIZEGRIP));
    widget->SetFieldsCount(node->prop_as_int(prop_fields));

    if (org_style & wxSTB_SIZEGRIP)
        widget->SetStatusText("gripper not displayed in Mock Up");

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> StatusBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = CreateStatusBar(";

    if (node->prop_as_string(prop_window_name).size())
    {
        code << node->prop_as_int(prop_fields) << ", " << node->prop_as_string(prop_id);
        GenStyle(node, code);
        code << ", " << node->prop_as_string(prop_window_name);
    }
    else if (node->prop_as_int(prop_style) != wxSTB_DEFAULT_STYLE || node->prop_as_int(prop_window_style) > 0)
    {
        code << node->prop_as_int(prop_fields) << ", " << node->prop_as_string(prop_id);
        GenStyle(node, code);
    }
    else if (node->prop_as_string(prop_id) != "wxID_ANY")
    {
        code << node->prop_as_int(prop_fields) << ", " << node->prop_as_string(prop_id);
    }
    else if (node->prop_as_int(prop_fields) > 1)
    {
        code << node->prop_as_int(prop_fields);
    }

    code << ");";

    return code;
}

std::optional<ttlib::cstr> StatusBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StatusBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/statusbr.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  StaticBitmapGenerator  //////////////////////////////////////////

wxObject* StaticBitmapGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxBitmap(prop_bitmap),
                                  DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
    if (auto value = node->prop_as_string(prop_scale_mode); value != "None")
    {
        if (value == "Fill")
            widget->SetScaleMode(wxStaticBitmap::Scale_Fill);
        else if (value == "AspectFit")
            widget->SetScaleMode(wxStaticBitmap::Scale_AspectFit);
        else if (value == "AspectFill")
            widget->SetScaleMode(wxStaticBitmap::Scale_AspectFill);
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> StaticBitmapGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    bool use_generic_version = (node->prop_as_string(prop_scale_mode) != "None");
    if (use_generic_version)
        code << node->get_node_name() << " = new wxGenericStaticBitmap(";
    else
        code << node->get_node_name() << " = new wxStaticBitmap(";

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->HasValue(prop_bitmap))
    {
        if (use_generic_version)
        {
            // wxGenericStaticBitmap expects a wxBitmap, so it's fine to pass it a wxImage
            code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
        }
        else
        {
            // wxStaticBitmap requires a wxGDIImage for the bitmap, and that won't accept a wxImage.
            code << "wxBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ")";
        }
    }
    else
    {
        code << "wxNullBitmap";
    }

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> StaticBitmapGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    if (node->prop_as_string(prop_scale_mode) == "None")
        return {};

    ttlib::cstr code;

    code << node->get_node_name() << "->SetScaleMode(wxStaticBitmap::Scale_" << node->prop_as_string(prop_scale_mode)
         << ");";

    return code;
}

std::optional<ttlib::cstr> StaticBitmapGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StaticBitmapGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    if (node->prop_as_string(prop_scale_mode) != "None")
        InsertGeneratorInclude(node, "#include <wx/generic/statbmpg.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/statbmp.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  GaugeGenerator  //////////////////////////////////////////

wxObject* GaugeGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxGauge(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_int(prop_range),
                              DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
    widget->SetValue(node->prop_as_int(prop_position));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool GaugeGenerator::OnPropertyChange(wxObject* widget, Node* /* node */, NodeProperty* prop)
{
    if (prop->isProp(prop_position))
    {
        wxStaticCast(widget, wxGauge)->SetValue(prop->as_int());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> GaugeGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", " << node->prop_as_string(prop_range);

    auto& win_name = node->prop_as_string(prop_window_name);
    if (win_name.size())
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
    }

    GeneratePosSizeFlags(node, code, true, "wxGA_HORIZONTAL");

    return code;
}

std::optional<ttlib::cstr> GaugeGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // If a validator has been specified, then the variable will be initialized in the header file.
    if (node->prop_as_string(prop_validator_variable).empty())
    {
        code << node->get_node_name() << "->SetValue(" << node->prop_as_string(prop_position) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> GaugeGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool GaugeGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/gauge.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  SliderGenerator  //////////////////////////////////////////

wxObject* SliderGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSlider(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_int(prop_value),
                               node->prop_as_int(prop_minValue), node->prop_as_int(prop_maxValue),
                               DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetValue(node->prop_as_int(prop_position));
    if (node->prop_as_int(prop_line_size) > 0)
        widget->SetLineSize(node->prop_as_int(prop_line_size));
    if (node->prop_as_int(prop_page_size) > 0)
        widget->SetPageSize(node->prop_as_int(prop_page_size));
#if defined(_WIN32)
    if (node->prop_as_int(prop_tick_frequency) > 0)
        widget->SetTickFreq(node->prop_as_int(prop_tick_frequency));
    if (node->prop_as_int(prop_thumb_length) > 0)
        widget->SetThumbLength(node->prop_as_int(prop_thumb_length));
#endif  // _WIN32

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SliderGenerator::OnPropertyChange(wxObject* widget, Node* /* node */, NodeProperty* prop)
{
    if (prop->isProp(prop_position))
    {
        wxStaticCast(widget, wxSlider)->SetValue(prop->as_int());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> SliderGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", " << node->prop_as_string(prop_position);
    code << ", " << node->prop_as_string(prop_minValue) << ", " << node->prop_as_string(prop_maxValue);

    auto& win_name = node->prop_as_string(prop_window_name);
    if (win_name.size())
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
    }

    GeneratePosSizeFlags(node, code, true, "wxSL_HORIZONTAL");

    return code;
}

std::optional<ttlib::cstr> SliderGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> SliderGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // If a validator has been specified, then the variable will be initialized in the header file.
    if (node->prop_as_string(prop_validator_variable).empty())
    {
        code << node->get_node_name() << "->SetValue(" << node->prop_as_string(prop_position) << ");";
    }

    if (node->prop_as_int(prop_line_size) > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetLineSize(" << node->prop_as_string(prop_line_size) << ");";
    }

    if (node->prop_as_int(prop_page_size) > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetPageSize(" << node->prop_as_string(prop_page_size) << ");";
    }

    if (node->prop_as_int(prop_tick_frequency) > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetTickFreq(" << node->prop_as_string(prop_tick_frequency) << ");";
    }

    if (node->prop_as_int(prop_thumb_length) > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetThumbLength(" << node->prop_as_string(prop_thumb_length) << ");";
    }
    return code;
}

bool SliderGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/slider.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  HyperlinkGenerator  //////////////////////////////////////////

wxObject* HyperlinkGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxHyperlinkCtrlBase* widget;
    if (node->prop_as_bool(prop_underlined))
    {
        widget = new wxHyperlinkCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                     node->prop_as_wxString(prop_url), DlgPoint(parent, node, prop_pos),
                                     DlgSize(parent, node, prop_size), GetStyleInt(node));
    }
    else
    {
        widget = new wxGenericHyperlinkCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                            node->prop_as_wxString(prop_url), DlgPoint(parent, node, prop_pos),
                                            DlgSize(parent, node, prop_size), GetStyleInt(node));

        if (!node->HasValue(prop_font))
        {
            widget->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        }
    }

    if (node->HasValue(prop_hover_color))
    {
        widget->SetHoverColour(node->prop_as_wxColour(prop_hover_color));
    }
    else if (node->prop_as_bool(prop_sync_hover_colour))
    {
        widget->SetHoverColour(widget->GetNormalColour());
    }

    if (node->HasValue(prop_normal_color))
    {
        widget->SetNormalColour(node->prop_as_wxColour(prop_normal_color));
    }
    if (node->HasValue(prop_visited_color))
    {
        widget->SetVisitedColour(node->prop_as_wxColour(prop_visited_color));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> HyperlinkGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node, !node->prop_as_bool(prop_underlined));

    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }
    code << ", ";
    auto& url = node->prop_as_string(prop_url);
    if (url.size())
    {
        code << GenerateQuotedString(url);
    }
    else
    {
        code << "wxEmptyString";
    }

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> HyperlinkGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;
    if (!node->prop_as_bool(prop_underlined) && !node->HasValue(prop_font))
    {
        code << node->get_node_name() << "->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));";
    }

    if (node->HasValue(prop_hover_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(" << GenerateColourCode(node, prop_hover_color) << ");";
    }
    else if (node->prop_as_bool(prop_sync_hover_colour))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(" << node->get_node_name() << "->GetNormalColour());";
    }

    if (node->HasValue(prop_normal_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetNormalColour(" << GenerateColourCode(node, prop_normal_color) << ");";
    }

    if (node->HasValue(prop_visited_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetVisitedColour(" << GenerateColourCode(node, prop_visited_color) << ");";
    }
    return code;
}

bool HyperlinkGenerator::IsGeneric(Node* node)
{
    return (!node->prop_as_bool(prop_underlined));
}

std::optional<ttlib::cstr> HyperlinkGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool HyperlinkGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */, std::set<std::string>& set_hdr)
{
    // If there's an event, then this has to be in the header file.
    set_hdr.insert("#include <wx/hyperlink.h>");
    return true;
}

//////////////////////////////////////////  InfoBarGenerator  //////////////////////////////////////////

wxObject* InfoBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    m_infobar = new wxInfoBar(wxStaticCast(parent, wxWindow));

    // Show the message before effects are added in case the show_effect has a delay (which would delay the display of
    // the dialog)

    m_infobar->ShowMessage("Message ...", wxICON_INFORMATION);

    m_infobar->SetShowHideEffects((wxShowEffect) node->prop_as_mockup(prop_show_effect, "info_"),
                                  (wxShowEffect) node->prop_as_mockup(prop_hide_effect, "info_"));
    m_infobar->SetEffectDuration(node->prop_as_int(prop_duration));

    m_infobar->Bind(wxEVT_BUTTON, &InfoBarGenerator::OnButton, this);
    m_infobar->Bind(wxEVT_TIMER, &InfoBarGenerator::OnTimer, this);
    m_infobar->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return m_infobar;
}

std::optional<ttlib::cstr> InfoBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node) << GetParentName(node) << ");";

    return code;
}

std::optional<ttlib::cstr> InfoBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    code << '\t' << node->get_node_name() << "->SetShowHideEffects(" << node->prop_as_constant(prop_show_effect, "info_")
         << ", " << node->prop_as_constant(prop_hide_effect, "info_") << ");";

    if (node->prop_as_int(prop_duration) != 500)
    {
        code << "\n\t" << node->get_node_name() << "->SetEffectDuration(" << node->prop_as_string(prop_duration) << ");";
    }

    return code;
}

void InfoBarGenerator::OnButton(wxCommandEvent& event)
{
    m_timer.SetOwner(m_infobar);
    m_timer.StartOnce(m_infobar->GetEffectDuration() + 1000);

    event.Skip();
}

void InfoBarGenerator::OnTimer(wxTimerEvent& /* event */)
{
    m_infobar->ShowMessage("Message ...");
}

bool InfoBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/infobar.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  CustomControl  //////////////////////////////////////////

wxObject* CustomControl::CreateMockup(Node* /* node */, wxObject* parent)
{
    auto widget = new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, GetInternalImage("CustomControl"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CustomControl::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new ";
    if (node->HasValue(prop_namespace))
        code << node->prop_as_string(prop_namespace) << "::";
    code << node->prop_as_string(prop_class_name) << node->prop_as_string(prop_parameters) << ';';

    return code;
}

std::optional<ttlib::cstr> CustomControl::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;
    if (node->HasValue(prop_settings_code))
    {
        auto_indent = indent::auto_keep_whitespace;
        code << node->prop_as_string(prop_settings_code);
        code.Replace("@@", "\n", tt::REPLACE::all);
        return code;
    }
    else
    {
        return {};
    }
}

bool CustomControl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    if (node->HasValue(prop_header))
    {
        set_src.insert(ttlib::cstr() << "#include \"" << node->prop_as_string(prop_header) << '"');
    }

    if (node->prop_as_string(prop_class_access) != "none" && node->HasValue(prop_class_name))
    {
        if (node->HasValue(prop_namespace))
        {
            set_hdr.insert(ttlib::cstr("namespace ") << node->prop_as_string(prop_namespace) << "\n{\n\t"
                                                     << "class " << node->prop_as_string(prop_class_name) << ";\n}");
        }
        else
            set_hdr.insert(ttlib::cstr() << "class " << node->prop_as_string(prop_class_name) << ';');
    }
    return true;
}

std::optional<ttlib::cstr> CustomControl::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}
