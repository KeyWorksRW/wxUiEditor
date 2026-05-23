/////////////////////////////////////////////////////////////////////////////
// Purpose:   C-wrapper and wxBinding strategy base classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "language_traits.h"

// Abstract base for all non-C++ language strategies.
// Captures shared patterns for languages that access wxWidgets through C wrappers.
class CWrapperStrategy : public LanguageStrategy
{
public:
    explicit CWrapperStrategy(const LanguageTraits& traits);

    // Default implementation: Non-C++ languages never need feature-gap workarounds
    // for features that their binding supports. Override per-language for specific gaps.
    [[nodiscard]] auto IsFeatureSupported(Node* node, GenEnum::PropName prop) -> bool override;

    // Default: Non-C++ platform end just unindents. Ruby overrides to also add "end".
    void EmitPlatformEnd(WriteCode* writer) override;

    // Stubs for complex behaviors — to be migrated incrementally
    void EmitConstruction(Code& code, Node* node) override;
    void EmitEventBinding(Code& code, Node* node, NodeEvent* event) override;
    void EmitImageData(Code& code, const EmbeddedImage* embed, WriteCode* writer) override;
};

// Abstract base for wxPython and wxRuby3 strategies.
// Shared behavior: wx-prefix mapping for class names and constants,
// near-full feature parity with wrapper-gap handling.
class WxBindingStrategy : public CWrapperStrategy
{
public:
    explicit WxBindingStrategy(const LanguageTraits& traits);

    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
