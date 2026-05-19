/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse C++ class, method, enum, and typedef decls
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "doc_types.h"

#include <string_view>

namespace docparser
{

    // Parse a class declaration line: "class Name : public Base1, public Base2"
    // Returns a ClassInfo with name and bases populated (no members).
    [[nodiscard]] ClassInfo ParseClassDecl(std::string_view text);

    // Extracts return type, name, parameters (with defaults), qualifiers.
    [[nodiscard]] MethodInfo ParseMethodDecl(std::string_view text);

    // Parse an enum declaration block (the full "enum [class] Name { ... }" text).
    // Handles per-value /// comments.
    [[nodiscard]] EnumInfo ParseEnumDecl(std::string_view text);

    // Parse a typedef: "typedef OldType NewName;" or "using NewName = OldType;"
    [[nodiscard]] TypedefInfo ParseTypedef(std::string_view text);

    // Parse a #define: "#define NAME value"
    [[nodiscard]] DefineInfo ParseDefine(std::string_view text);

}  // namespace docparser
