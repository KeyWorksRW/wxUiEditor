/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse doxygen comment blocks into DocComment
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "doc_types.h"

#include <string_view>

namespace docparser
{

    // Parse a doxygen comment block into a structured DocComment.
    // Accepts the raw text between /** and */ (or joined /// lines).
    // Strips leading '*' and whitespace from Javadoc-style blocks.
    [[nodiscard]] DocComment ParseCommentBlock(std::string_view text);

}  // namespace docparser
