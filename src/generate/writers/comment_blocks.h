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

// Comment block for languages using // for comments without lint directives
extern const char* const txt_SlashPlainCmtBlock;

// Comment block for Julia
extern const char* const txt_JuliaCmtBlock;

// Comment block for LuaJIT
extern const char* const txt_LuajitCmtBlock;

// Comment block for Fortran
extern const char* const txt_FortranCmtBlock;

// End of generated code comment block for C++
extern const char* const end_cpp_block;

// End of generated code comment block for Python
extern const char* const end_python_block;

// End of generated code comment block for Ruby
extern const char* const end_ruby_block;

// End of generated code comment block for TypeScript
extern const char* const end_typescript_block;

// End of generated code comment block for Go
extern const char* const end_go_block;

// End of generated code comment block for Julia
extern const char* const end_julia_block;

// End of generated code comment block for LuaJIT
extern const char* const end_luajit_block;

// End of generated code comment block for Fortran
extern const char* const end_fortran_block;

// End of generated code comment block for Rust
extern const char* const end_rust_block;

// Get the number of lines in the C++ end block
[[nodiscard]] auto GetCppEndBlockLength() -> size_t;

// Get the number of lines in the Python end block
[[nodiscard]] auto GetPythonEndBlockLength() -> size_t;

// Get the number of lines in the Ruby end block
[[nodiscard]] auto GetRubyEndBlockLength() -> size_t;

// Get the "End of generated code" line from the C++ end block
[[nodiscard]] auto GetCppEndCommentLine() -> std::string_view;

// Get the "End of generated code" line from the Python end block
[[nodiscard]] auto GetPythonEndCommentLine() -> std::string_view;

// Get the "End of generated code" line from the Ruby end block
[[nodiscard]] auto GetRubyEndCommentLine() -> std::string_view;
