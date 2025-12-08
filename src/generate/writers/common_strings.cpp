/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common strings used in code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "common_strings.h"

// This *must* be written on a line by itself with *no* indentation.
const std::string_view ruby_begin_cmt_block = "=begin";

// This *must* be written on a line by itself with *no* indentation.
const std::string_view ruby_end_cmt_block = "=end";

// This *MUST* be written without any indendation
const std::string_view perl_begin_cmt_block = "=pod";

// This *MUST* be written without any indendation
const std::string_view perl_end_cmt_block = "=cut";

const std::string_view python_triple_quote = R"(""")";
