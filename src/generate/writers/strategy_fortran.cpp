/////////////////////////////////////////////////////////////////////////////
// Purpose:   Fortran language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_fortran.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

FortranStrategy::FortranStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

void FortranStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // Fortran uses preprocessor directives for platform conditionals
    bool has_prior = false;

    if (platforms.find("Windows") != std::string_view::npos)
    {
        code.Eol() << "#if defined(_WIN32)";
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
        code << "defined(__unix__)";
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
        }
        code << "defined(__APPLE__)";
    }
}

void FortranStrategy::EmitPlatformEnd(WriteCode* writer)
{
    writer->writeLine("#endif");
}

void FortranStrategy::EmitImport(Code& code, std::string_view module)
{
    // Fortran: use <module>
    code.Str("use ").Str(module);
}

void FortranStrategy::EmitVarDecl(Code& code, std::string_view type, std::string_view name)
{
    // Fortran: type(c_ptr) :: name  (for opaque handles)
    if (type.empty())
    {
        code.Str("type(c_ptr) :: ").Str(name);
    }
    else
    {
        code.Str(type).Str(" :: ").Str(name);
    }
}
