/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>             // Event classes
#include <wx/gauge.h>             // wxGauge interface
#include <wx/generic/statbmpg.h>  // wxGenericStaticBitmap header
#include <wx/hyperlink.h>         // Hyperlink control
#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/slider.h>            // wxSlider interface
#include <wx/statbmp.h>           // wxStaticBitmap class interface
#include <wx/statline.h>          // wxStaticLine class interface
#include <wx/statusbr.h>          // wxStatusBar class interface

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "misc_widgets.h"

//////////////////////////////////////////  StaticLineGenerator  //////////////////////////////////////////

wxObject* StaticLineGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxStaticLine(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                         node->prop_as_wxSize("size"), node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> StaticLineGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxStaticLine(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");

    if (node->prop_as_string(txt_style) != "wxLI_HORIZONTAL")
    {
        GeneratePosSizeFlags(node, code);
    }
    else
    {
        auto pos = node->prop_as_wxPoint("pos");
        auto size = node->prop_as_wxPoint("size");
        auto& win_name = node->prop_as_string("window_name");
        auto& win_style = node->prop_as_string("window_style");

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
    auto org_style = node->prop_as_int(txt_style);
    // Don't display the gripper as it can resize our main window rather than just the mockup window
    auto widget = new wxStatusBar(wxStaticCast(parent, wxWindow), wxID_ANY,
                                  (org_style &= ~wxSTB_SIZEGRIP) | node->prop_as_int("window_style"));
    widget->SetFieldsCount(node->prop_as_int("fields"));

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

    if (node->prop_as_string("window_name").size())
    {
        code << node->prop_as_int("fields") << ", " << node->prop_as_string("id");
        GenStyle(node, code);
        code << ", " << node->prop_as_string("window_name");
    }
    else if (node->prop_as_int(txt_style) != wxSTB_DEFAULT_STYLE || node->prop_as_int("window_style") > 0)
    {
        code << node->prop_as_int("fields") << ", " << node->prop_as_string("id");
        GenStyle(node, code);
    }
    else if (node->prop_as_string("id") != "wxID_ANY")
    {
        code << node->prop_as_int("fields") << ", " << node->prop_as_string("id");
    }
    else if (node->prop_as_int("fields") > 1)
    {
        code << node->prop_as_int("fields");
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
    auto widget = new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxBitmap("bitmap"),
                                            node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                                            node->prop_as_int("window_style"));
    if (auto value = node->prop_as_string("scale_mode"); value != "None")
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

    bool use_generic_version = (node->prop_as_string("scale_mode") != "None");
    if (use_generic_version)
        code << node->get_node_name() << " = new wxGenericStaticBitmap(";
    else
        code << node->get_node_name() << " = new wxStaticBitmap(";

    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";
    if (node->HasValue("bitmap"))
    {
        if (use_generic_version)
        {
            // wxGenericStaticBitmap expects a wxBitmap, so it's fine to pass it a wxImage
            code << GenerateBitmapCode(node->prop_as_string("bitmap"));
        }
        else
        {
            // wxStaticBitmap requires a wxGDIImage for the bitmap, and that won't accept a wxImage.
            code << "wxBitmap(" << GenerateBitmapCode(node->prop_as_string("bitmap")) << ")";
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
    if (node->prop_as_string("scale_mode") == "None")
        return {};

    ttlib::cstr code;

    code << node->get_node_name() << "->SetScaleMode(wxStaticBitmap::Scale_" << node->prop_as_string("scale_mode") << ");";

    return code;
}

std::optional<ttlib::cstr> StaticBitmapGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StaticBitmapGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    if (node->prop_as_string("scale_mode") != "None")
        InsertGeneratorInclude(node, "#include <wx/generic/statbmpg.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/statbmp.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  GaugeGenerator  //////////////////////////////////////////

wxObject* GaugeGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxGauge(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_int("range"), node->prop_as_wxPoint("pos"),
                    node->prop_as_wxSize("size"),
                    node->prop_as_int("orientation") | node->prop_as_int(txt_style) | node->prop_as_int("window_style"));
    widget->SetValue(node->prop_as_int("position"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool GaugeGenerator::OnPropertyChange(wxObject* widget, Node* /* node */, NodeProperty* prop)
{
    if (prop->GetPropName() == "position")
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
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", " << node->prop_as_string("range");

    auto& win_name = node->prop_as_string("window_name");
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
    if (node->prop_as_string("validator_variable").empty())
    {
        code << node->get_node_name() << "->SetValue(" << node->prop_as_string("position") << ");";
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
    if (node->prop_as_string("validator_variable").size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  SliderGenerator  //////////////////////////////////////////

wxObject* SliderGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxSlider(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_int(txt_value), node->prop_as_int("minValue"),
                     node->prop_as_int("maxValue"), node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                     node->prop_as_int("orientation") | node->prop_as_int(txt_style) | node->prop_as_int("window_style"));
    widget->SetValue(node->prop_as_int("position"));
    if (node->prop_as_int("line_size") > 0)
        widget->SetLineSize(node->prop_as_int("line_size"));
    if (node->prop_as_int("page_size") > 0)
        widget->SetPageSize(node->prop_as_int("page_size"));
#if defined(_WIN32)
    if (node->prop_as_int("tick_frequency") > 0)
        widget->SetTickFreq(node->prop_as_int("tick_frequency"));
    if (node->prop_as_int("thumb_length") > 0)
        widget->SetThumbLength(node->prop_as_int("thumb_length"));
#endif  // _WIN32

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SliderGenerator::OnPropertyChange(wxObject* widget, Node* /* node */, NodeProperty* prop)
{
    if (prop->GetPropName() == "position")
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
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", " << node->prop_as_string("position");
    code << ", " << node->prop_as_string("minValue") << ", " << node->prop_as_string("maxValue");

    auto& win_name = node->prop_as_string("window_name");
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

std::optional<ttlib::cstr> SliderGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> SliderGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // If a validator has been specified, then the variable will be initialized in the header file.
    if (node->prop_as_string("validator_variable").empty())
    {
        code << node->get_node_name() << "->SetValue(" << node->prop_as_string("position") << ");";
    }

    if (node->prop_as_int("line_size") > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetLineSize(" << node->prop_as_string("line_size") << ");";
    }

    if (node->prop_as_int("page_size") > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetPageSize(" << node->prop_as_string("page_size") << ");";
    }

    if (node->prop_as_int("tick_frequency") > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetTickFreq(" << node->prop_as_string("tick_frequency") << ");";
    }

    if (node->prop_as_int("thumb_length") > 0)
    {
        if (code.size())
            code << "\n";
        code << node->get_node_name() << "->SetThumbLength(" << node->prop_as_string("thumb_length") << ");";
    }
    return code;
}

bool SliderGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/slider.h>", set_src, set_hdr);
    if (node->prop_as_string("validator_variable").size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  HyperlinkGenerator  //////////////////////////////////////////

wxObject* HyperlinkGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxHyperlinkCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->GetPropertyAsString(txt_label),
                            node->GetPropertyAsString("url"), node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                            node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    if (node->HasValue("hover_color"))
    {
        widget->SetHoverColour(node->prop_as_wxColour("hover_color"));
    }
    if (node->HasValue("normal_color"))
    {
        widget->SetNormalColour(node->prop_as_wxColour("normal_color"));
    }
    if (node->HasValue("visited_color"))
    {
        widget->SetVisitedColour(node->prop_as_wxColour("visited_color"));
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

    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";

    auto& label = node->prop_as_string(txt_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }
    code << ", ";
    auto& url = node->prop_as_string("url");
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
    if (node->HasValue("hover_color"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(";
        auto& clr = node->prop_as_string("hover_color");
        if (clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << clr << ");";
        else
        {
            wxColour colour = ConvertToColour(clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
    }

    if (node->HasValue("normal_color"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(";
        auto& clr = node->prop_as_string("normal_color");
        if (clr.contains("wx"))
            code << "wxSystemSettings::GetColour(" << clr << ");";
        else
        {
            wxColour colour = ConvertToColour(clr);
            code << ttlib::cstr().Format("wxColour(%i, %i, %i);", colour.Red(), colour.Green(), colour.Blue());
        }
    }

    if (node->HasValue("visited_color"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHoverColour(";
        auto& clr = node->prop_as_string("visited_color");
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

    m_infobar->SetShowHideEffects((wxShowEffect) node->prop_as_int("show_effect"),
                                  (wxShowEffect) node->prop_as_int("hide_effect"));
    m_infobar->SetEffectDuration(node->prop_as_int("duration"));

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

    code << '\t' << node->get_node_name() << "->SetShowHideEffects(" << node->prop_as_string("show_effect") << ", "
         << node->prop_as_string("hide_effect") << ");";

    if (node->prop_as_int("duration") != 500)
    {
        code << "\n\t" << node->get_node_name() << "->SetEffectDuration(" << node->prop_as_string("duration") << ");";
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
