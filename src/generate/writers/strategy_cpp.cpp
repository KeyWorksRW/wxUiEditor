/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++ language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include "strategy_cpp.h"

#include "code.h"        // Code -- Helper class for generating code
#include "node.h"        // Node class
#include "write_code.h"  // WriteCode class

CppStrategy::CppStrategy(const LanguageTraits& traits) : LanguageStrategy(traits) {}

void CppStrategy::EmitConstruction(Code& /* code */, Node* /* node */)
{
    // Phase 4 stub — full implementation in later phases when construction patterns
    // are migrated from individual generators to strategy objects.
}

void CppStrategy::EmitEventBinding(Code& /* code */, Node* /* node */, NodeEvent* /* event */)
{
    // Phase 4 stub — event binding migration is complex and will happen incrementally.
}

void CppStrategy::EmitImport(Code& code, std::string_view module)
{
    // C++ uses #include for imports
    // module must include delimiters: "<wx/wx.h>" or "\"myfile.h\""
    code.Str("#include ").Str(module);
}

void CppStrategy::EmitVarDecl(Code& code, std::string_view type, std::string_view name)
{
    // C++ uses explicit type declarations: auto* name or Type* name
    code.Str(type).Str(" ").Str(name);
}

bool CppStrategy::IsFeatureSupported(Node* /* node */, GenEnum::PropName /* prop */)
{
    // C++ has full feature parity — all features are supported
    return true;
}

void CppStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    bool has_prior = false;

    if (platforms.find("Windows") != std::string_view::npos)
    {
        code.Eol() << "#if defined(__WINDOWS__)";
        has_prior = true;
    }
    if (platforms.find("Unix") != std::string_view::npos)
    {
        if (has_prior)
        {
            code << " || ";
        }
        else
        {
            code.Eol() << "#if ";
            has_prior = true;
        }
        code << "defined(__UNIX__)";
    }
    if (platforms.find("Mac") != std::string_view::npos)
    {
        if (has_prior)
        {
            code << " || ";
        }
        else
        {
            code.Eol() << "#if ";
            has_prior = true;
        }
        code << "defined(__WXOSX__)";
    }
}

void CppStrategy::EmitPlatformEnd(WriteCode* writer)
{
    // Caller guarantees EmitPlatformBegin was invoked first with valid platforms.
    writer->writeLine("#endif  // limited to specific platforms");
}

std::string CppStrategy::MapClassName(std::string_view wx_class_name)
{
    // C++ uses wxWidgets class names unmodified
    return std::string(wx_class_name);
}

void CppStrategy::EmitImageData(Code& /* code */, const EmbeddedImage* /* embed */,
                                WriteCode* /* writer */)
{
    // Phase 4 stub — full image data emission will be migrated from
    // WriteImageConstruction in a later phase.
}
