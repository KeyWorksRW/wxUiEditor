/////////////////////////////////////////////////////////////////////////////
// Purpose:   Sizer component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxGBSizerItem;
class wxRadioButton;
class wxCheckBox;
class wxGBPosition;
class wxGBSpan;

// This class is needed to get it to display on the Sizers toolbar, but it's not an actual component
class SpacerGenerator : public BaseGenerator
{
public:
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
};

class BoxSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* /*parent*/) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, bool /* add_comments */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};

class GridSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* /*parent*/) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class WrapSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* /*parent*/) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class StaticBoxSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class StaticCheckboxBoxSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

private:
    wxCheckBox* m_checkbox;
};

class StaticRadioBtnBoxSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

private:
    wxRadioButton* m_radiobtn;
};

class FlexGridSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* /*parent*/) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class GridBagSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* /*parent*/) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

protected:
    wxGBSizerItem* GetGBSizerItem(Node* sizeritem, const wxGBPosition& position, const wxGBSpan& span, wxObject* child);
};

class StdDialogButtonSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, bool /* add_comments */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};

class TextSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    // std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};
