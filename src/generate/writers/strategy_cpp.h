/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++ language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: CppStrategy implements the LanguageStrategy interface for native C++ code
// generation. C++ is the only language that directly calls wxWidgets C++ APIs without going
// through a C wrapper. This means: direct `new` construction, `->` member access, `#ifdef`
// platform conditionals, raw byte arrays for images, and full feature parity with wxWidgets.
// No prefix mapping is needed—wx class names pass through unmodified.

#pragma once

#include "language_traits.h"

class CppStrategy : public LanguageStrategy
{
public:
    explicit CppStrategy(const LanguageTraits& traits);

    void EmitConstruction(Code& code, Node* node) override;
    void EmitEventBinding(Code& code, Node* node, NodeEvent* event) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
    [[nodiscard]] auto IsFeatureSupported(Node* node, GenEnum::PropName prop) -> bool override;

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitPlatformEnd(WriteCode* writer) override;
    [[nodiscard]] auto MapClassName(std::string_view wx_class_name) -> std::string override;
    void EmitImageData(Code& code, const EmbeddedImage* embed, WriteCode* writer) override;
};
