/////////////////////////////////////////////////////////////////////////////
// Purpose:   Shared generated comment blocks
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   none -- see ..\..\LICENSE.html
/////////////////////////////////////////////////////////////////////////////

#pragma once

// Comment block for Python with lint directives
extern const char* const txt_PythonCmtBlock;

// Comment block for Perl with lint directives
extern const char* const txt_PerlCmtBlock;

// Comment block for Ruby with rubocop directives
extern const char* const txt_RubyCmtBlock;

// Comment block for languages using // for comments (C++)
extern const char* const txt_SlashCmtBlock;

// End of generated code comment block for C++
extern const char* const end_cpp_block;

// End of generated code comment block for Python
extern const char* const end_python_block;

// End of generated code comment block for Perl
extern const char* const end_perl_block;

// End of generated code comment block for Ruby
extern const char* const end_ruby_block;

// Get the number of lines in the C++ end block
[[nodiscard]] auto GetCppEndBlockLength() -> size_t;

// Get the number of lines in the Python end block
[[nodiscard]] auto GetPythonEndBlockLength() -> size_t;

// Get the number of lines in the Perl end block
[[nodiscard]] auto GetPerlEndBlockLength() -> size_t;

// Get the number of lines in the Ruby end block
[[nodiscard]] auto GetRubyEndBlockLength() -> size_t;

// Get the "End of generated code" line from the C++ end block
[[nodiscard]] auto GetCppEndCommentLine() -> std::string_view;

// Get the "End of generated code" line from the Python end block
[[nodiscard]] auto GetPythonEndCommentLine() -> std::string_view;

// Get the "End of generated code" line from the Perl end block
[[nodiscard]] auto GetPerlEndCommentLine() -> std::string_view;

// Get the "End of generated code" line from the Ruby end block
[[nodiscard]] auto GetRubyEndCommentLine() -> std::string_view;
