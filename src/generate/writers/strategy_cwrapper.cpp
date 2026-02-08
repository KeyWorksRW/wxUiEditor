/////////////////////////////////////////////////////////////////////////////
// Purpose:   C-wrapper and wxBinding strategy base classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_cwrapper.h"

#include "code.h"        // Code -- Helper class for generating code
#include "node.h"        // Node class
#include "write_code.h"  // WriteCode class

// ---- CWrapperStrategy (abstract base for all non-C++ languages) ----

CWrapperStrategy::CWrapperStrategy(const LanguageTraits& traits) : LanguageStrategy(traits) {}

auto CWrapperStrategy::IsFeatureSupported(Node* /* node */, GenEnum::PropName /* prop */) -> bool
{
    // Default: all features supported. Python and Ruby strategies override this
    // for specific feature gaps in their bindings.
    return true;
}

void CWrapperStrategy::EmitPlatformEnd(WriteCode* writer)
{
    // Default for scripting languages: just unindent (Python, Ruby base behavior).
    // Ruby overrides to also emit "end".
    writer->Unindent();
}

void CWrapperStrategy::EmitConstruction(Code& /* code */, Node* /* node */)
{
    // Phase 4 stub — construction pattern migration will happen incrementally.
}

void CWrapperStrategy::EmitEventBinding(Code& /* code */, Node* /* node */, NodeEvent* /* event */)
{
    // Phase 4 stub — event binding migration is complex and will happen incrementally.
}

void CWrapperStrategy::EmitImageData(Code& /* code */, const EmbeddedImage* /* embed */,
                                     WriteCode* /* writer */)
{
    // Phase 4 stub — image data emission will be migrated from WriteImageConstruction.
}

// ---- WxBindingStrategy (abstract base for Python and Ruby) ----

WxBindingStrategy::WxBindingStrategy(const LanguageTraits& traits) : CWrapperStrategy(traits) {}

void WxBindingStrategy::EmitImport(Code& code, std::string_view module)
{
    // Both Python and Ruby use a simple import/require statement.
    // Python: "import wx" or "from wx import adv"
    // Ruby: "require 'wx'" or "require 'wx/core'"
    // The exact keyword differs but the pattern is the same.
    code.Str(module);
}

void WxBindingStrategy::EmitVarDecl(Code& code, std::string_view /* type */, std::string_view name)
{
    // Dynamically typed languages don't need type declarations.
    // Just emit the variable name — the caller handles prefix (self./@).
    code.Str(name);
}
