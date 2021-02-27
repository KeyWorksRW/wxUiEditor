/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/infobar.h>  // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/timer.h>    // wxTimer, wxStopWatch and global time-related functions

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class StaticLineGenerator : public BaseGenerator
{
public:
    wxObject* Create(Node* node, wxObject* parent) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class StatusBarGenerator : public BaseGenerator
{
public:
    wxObject* Create(Node* node, wxObject* parent) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class StaticBitmapGenerator : public BaseGenerator
{
public:
    wxObject* Create(Node* node, wxObject* parent) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class GaugeGenerator : public BaseGenerator
{
public:
    wxObject* Create(Node* node, wxObject* parent) override;
    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class SliderGenerator : public BaseGenerator
{
public:
    wxObject* Create(Node* node, wxObject* parent) override;
    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class HyperlinkGenerator : public BaseGenerator
{
public:
    wxObject* Create(Node* node, wxObject* parent) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class InfoBarGenerator : public BaseGenerator
{
public:
    wxObject* Create(Node* node, wxObject* parent) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

protected:
    void OnButton(wxCommandEvent& event);
    void OnTimer(wxTimerEvent&);

private:
    wxTimer m_timer;
    wxInfoBar* m_infobar;
};
