/////////////////////////////////////////////////////////////////////////////
// Purpose:   Shared generated comment blocks
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   none -- see ..\..\LICENSE.html
/////////////////////////////////////////////////////////////////////////////

#pragma once

// Comment block for languages using # for comments (Python, Perl, Ruby)
extern const char* const txt_PoundCmtBlock;

// Comment block for languages using // for comments (C++)
extern const char* const txt_SlashCmtBlock;

// End of generated code comment block for C++
extern const char* const end_cpp_block;

// End of generated code comment block for Python, Perl, and Ruby
extern const char* const end_python_perl_ruby_block;

// Get the number of lines in the C++ end block
[[nodiscard]] auto GetCppEndBlockLength() -> size_t;

// Get the number of lines in the Python/Perl/Ruby end block
[[nodiscard]] auto GetPythonPerlRubyEndBlockLength() -> size_t;
