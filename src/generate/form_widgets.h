/////////////////////////////////////////////////////////////////////////////
// Purpose:   Form component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

// Note that the Mockup code does not call Create() for forms.

class FrameFormGenerator : public BaseGenerator
{
public:
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class PopupWinGenerator : public BaseGenerator
{
public:
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class PanelFormGenerator : public BaseGenerator
{
public:
    // Return true if all construction and settings code was written to src_code
    bool GenConstruction(Node*, BaseCodeGenerator* code_gen) override;

    std::optional<ttlib::cstr> GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class DialogFormGenerator : public BaseGenerator
{
public:
    // Return true if all construction and settings code was written to src_code
    bool GenConstruction(Node*, BaseCodeGenerator* code_gen) override;

    std::optional<ttlib::cstr> GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};
