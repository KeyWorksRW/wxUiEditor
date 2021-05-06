/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

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

#include "bitmaps.h"     // Contains various images handling functions
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties
#include "write_code.h"  // WriteCode -- Write code to Scintilla or file

#include "misc_widgets.h"

//////////////////////////////////////////  ActivityIndicatorGenerator  //////////////////////////////////////////

wxObject* ActivityIndicatorGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxActivityIndicator(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                          node->prop_as_wxSize(prop_size), node->prop_as_int(prop_window_style));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);
    widget->Start();

    return widget;
}

std::optional<ttlib::cstr> ActivityIndicatorGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxActivityIndicator(";
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

wxObject* AnimationGenerator::Create(Node* node, wxObject* parent)
{
    wxAnimation animation;
    if (node->HasValue(prop_animation))
    {
        ttlib::multistr file_names(node->prop_as_string(prop_animation), ';');
        GetAnimationImage(animation, file_names[0]);
    }
    auto widget = new wxAnimationCtrl(wxStaticCast(parent, wxWindow), wxID_ANY,
                                      animation.IsOk() ? animation : wxNullAnimation, node->prop_as_wxPoint(prop_pos),
                                      node->prop_as_wxSize(prop_size), node->prop_as_int(prop_style));

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
    code << node->get_node_name() << " = new wxAnimationCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxNullAnimation";

    GeneratePosSizeFlags(node, code);

    return code;
}

bool AnimationGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/animate.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  BannerWindowGenerator  //////////////////////////////////////////

wxObject* BannerWindowGenerator::Create(Node* node, wxObject* parent)
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
    code << node->get_node_name() << " = new wxBannerWindow(";
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

        code << ",\n    ";

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
        code << node->get_node_name() << "->SetText(" << GenerateQuotedString(node->prop_as_string(prop_title)) << ",\n    ";
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

wxObject* StaticLineGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxStaticLine(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                   node->prop_as_wxSize(prop_size),
                                   node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

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

wxObject* StatusBarGenerator::Create(Node* node, wxObject* parent)
{
    auto org_style = node->prop_as_int(prop_style);
    // Don't display the gripper as it can resize our main window rather than just the mockup window
    auto widget = new wxStatusBar(wxStaticCast(parent, wxWindow), wxID_ANY,
                                  (org_style &= ~wxSTB_SIZEGRIP) | node->prop_as_int(prop_window_style));
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

wxObject* StaticBitmapGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxBitmap(prop_bitmap),
                                            node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size),
                                            node->prop_as_int(prop_window_style));
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

wxObject* GaugeGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxGauge(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_int(prop_range),
                              node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size),
                              node->prop_as_int(prop_orientation) | node->prop_as_int(prop_style) |
                                  node->prop_as_int(prop_window_style));
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
    code << node->get_node_name() << " = new wxGauge(";
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

    GeneratePosSizeFlags(node, code, true, "orientation", "wxGA_HORIZONTAL");

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

wxObject* SliderGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxSlider(
        wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_int(prop_value), node->prop_as_int(prop_minValue),
        node->prop_as_int(prop_maxValue), node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size),
        node->prop_as_int(prop_orientation) | node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));
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
    code << node->get_node_name() << " = new wxSlider(";
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

    GeneratePosSizeFlags(node, code, true, "orientation", "wxSL_HORIZONTAL");

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

wxObject* HyperlinkGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxHyperlinkCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                      node->prop_as_wxString(prop_url), node->prop_as_wxPoint(prop_pos),
                                      node->prop_as_wxSize(prop_size),
                                      node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    if (node->HasValue(prop_hover_color))
    {
        widget->SetHoverColour(node->prop_as_wxColour(prop_hover_color));
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
    code << node->get_node_name() << " = new wxHyperlinkCtrl(";

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
    if (node->HasValue(prop_hover_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(";
        auto& clr = node->prop_as_string(prop_hover_color);
        if (clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << clr << ");";
        else
        {
            wxColour colour = ConvertToColour(clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
    }

    if (node->HasValue(prop_normal_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(";
        auto& clr = node->prop_as_string(prop_normal_color);
        if (clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << clr << ");";
        else
        {
            wxColour colour = ConvertToColour(clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
    }

    if (node->HasValue(prop_visited_color))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(";
        auto& clr = node->prop_as_string(prop_visited_color);
        if (clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << clr << ");";
        else
        {
            wxColour colour = ConvertToColour(clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
    }
    return code;
}

std::optional<ttlib::cstr> HyperlinkGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool HyperlinkGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/hyperlink.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  InfoBarGenerator  //////////////////////////////////////////

wxObject* InfoBarGenerator::Create(Node* node, wxObject* parent)
{
    m_infobar = new wxInfoBar(wxStaticCast(parent, wxWindow));

    // Show the message before effects are added in case the show_effect has a delay (which would delay the display of
    // the dialog)

    m_infobar->ShowMessage("Message ...", wxICON_INFORMATION);

    m_infobar->SetShowHideEffects((wxShowEffect) node->prop_as_int(prop_show_effect),
                                  (wxShowEffect) node->prop_as_int(prop_hide_effect));
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
    code << node->get_node_name() << " = new wxInfoBar(this);";

    return code;
}

std::optional<ttlib::cstr> InfoBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    code << '\t' << node->get_node_name() << "->SetShowHideEffects(" << node->prop_as_string(prop_show_effect) << ", "
         << node->prop_as_string(prop_hide_effect) << ");";

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
