# Guidance for AI Coding in wxUiEditor

This repository contains C++, Perl, Python, and Ruby code using wxWidgets for GUI functionality. Follow these standards and priorities for all code contributions.

## Project Overview

- Multi-language project: C++, Perl, Python, Ruby
- GUI: wxWidgets (all languages)

## Key Files and Directories
- `src/ttwx/ttwx.h`: Project-specific C++ utilities
- `./frozen/include/frozen`: Immutable container library

# C++ Coding Standards

## Naming Conventions
- Variables: snake_case
- Classes/Methods/Functions: PascalCase
- Constants: UPPER_SNAKE_CASE
- Enum types: PascalCase (singular, e.g., Color)
- Enum values: snake_case

## Code Style
- Indentation: 4 spaces
- Line length: 100 characters maximum
- Use blank lines to separate logical code sections

## Function Declarations
- Always use trailing return type syntax
- Functions returning bool must have [[nodiscard]] attribute

## Enum Guidelines
- Use enum class for type safety (never traditional enums)
- Specify underlying type using std:: types (e.g., std::uint8_t)

## Conditionals
- Always use braces for conditional statements, even single-line statements
- Do not use else after a conditional that exits (return, throw, break)

## Modern C++ Features
- Prefer auto for variable declarations when type is obvious
- Use constexpr for compile-time constants and functions
- Use range-based for loops over traditional loops when iterating containers
- Use smart pointers (std::unique_ptr, std::shared_ptr) instead of raw pointers
- Use structured bindings and concepts for clarity and safety
- Prefer C++20 ranges library algorithms over manual loops when applicable

## Library Priority (in order)
1. C++ Standard Library (std::) – Always check here first
2. wxWidgets library – Use when standard library doesn't provide functionality
3. ttwx:: namespace (from src/ttwx/ttwx.h) – Project-specific utilities
4. Frozen containers (from ./frozen/include/frozen) – For immutable collections

## Immutable Containers
- Use frozen library types for immutable versions of: std::set, std::unordered_set, std::map, std::unordered_map
- Corresponding frozen types: frozen::set, frozen::unordered_set, frozen::map, frozen::unordered_map
- Always include the appropriate frozen header file
- Prefer constexpr or constinit for frozen containers

## String Searching
- Consider Boyer-Moore or Knuth-Morris-Pratt algorithms from frozen library if faster than std::search

## String Conversions
- **Always use `wxString::ToStdString()`** for converting `wxString` to `std::string` or `std::string_view`.
- Prefer wxString::ToStdString() wxString::utf8_string() (copy)

## Debug Checks
- Use ASSERT, ASSERT_MSG, and FAIL_MSG macros from assertion_dlg.h for debug checks

## File and Code Restrictions
- Do not edit any code between the comment block "// Do not edit any code above the "End of generated code" comment block.

# Perl Coding Standards

## Naming Conventions
- Variables: snake_case
- Packages: PascalCase
- Wx:: methods: PascalCase
- Constants: UPPER_SNAKE_CASE
- Begin wxWidgets functions with Wx:

## Code Style
- Indentation: 4 spaces
- Line length: 100 characters maximum
- Use blank lines to separate logical code sections

# Python Coding Standards

## Naming Conventions
- Variables/Functions: snake_case
- Packages: PascalCase
- wx. methods: PascalCase
- Constants: UPPER_SNAKE_CASE
- Begin wxWidgets functions with wx.

## Code Style
- Indentation: 4 spaces
- Line length: 90 characters maximum
- Use blank lines to separate logical code sections

# Ruby Coding Standards

## Naming Conventions
- Variables/Functions: snake_case
- Classes: PascalCase
- Wx:: methods: PascalCase
- Constants: UPPER_SNAKE_CASE
- Begin wxWidgets functions with Wx:
- Use a leading _ for an unused parameter, e.g. _event

## Code Style
- Indentation: 2 spaces
- Line length: 80 characters maximum
- Use blank lines to separate logical code sections

# Important Reminders

- Always check if Standard Library provides needed functionality before using external libraries
- Include appropriate frozen headers when using immutable containers
- Validate that enum classes use explicit underlying types
- Ensure all bool-returning functions have [[nodiscard]] attribute
- Verify conditionals use braces even for single statements
- Do not edit any code between the comment block "// Do not edit any code above the 'End of generated code' comment block." and "// DO NOT EDIT THIS COMMENT BLOCK!"
- For Node and NodeProperty classes, prefer as_view() methods
- All generator classes derive from generate/base_generator.h
