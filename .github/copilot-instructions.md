# Project coding standards for C++

## Naming Conventions
- Use snake_case for variable names.
- Use PascalCase for class names, method names, and function names.
- Use UPPER_SNAKE_CASE for constants.

## Code Structure
- Use 4 spaces for indentation.
- Limit lines to 100 characters.
- Use blank lines to separate logical sections of code.

## Function Declarations
- Use a trailing return type for all function declarations.
- Functions returning a bool should always have a [[nodiscard]] prefix.

## Enum Declarations
- When creating an enum, use a `std::` type (e.g., `std::uint8_t`) to reduce size.
- Use `enum class` instead of traditional enums for type safety.
- Use singular names for enum types (e.g., `Color` instead of `Colors`).
- Use PascalCase for enum type names and snake_case for enum values.

## Conditionals
- Always use braces around any statements that are part of a conditional, even for single statements.
- Do not follow a conditional with an else statement if the last statement of the conditional would prevent the else from being executed (e.g., return, throw, break).

## Modern C++ Features
- Prefer using `auto` for variable declarations when the type is obvious or improves readability.
- Use `constexpr` for compile-time constants and functions where possible.
- Favor range-based for loops over traditional for loops when iterating containers.
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) instead of raw pointers for resource management.
- Use structured bindings and concepts where appropriate to improve code clarity and safety.

## Library Function Usage
- When implementing functionality, first prefer functions from the C++ Standard Library (`std::`).
- If no suitable standard library function exists, use functions from the wxWidgets library.
- If neither provides the needed functionality, use functions from the `ttwx::` namespace (defined in `src/ttwx/ttwx.h`).
- When creating or modifying an immutable version of std::set, std::unordered_set, std::map and std::unordered_map use one of the frozen versions from the library ./frozen/include/frozen (be sure to include the appropriate header file).
- For immutable containers, use the corresponding frozen type from `./frozen/include/frozen` (such as `frozen::set`, `frozen::unordered_set`, `frozen::map`, or `frozen::unordered_map`).
- Always include the required frozen header file when using these containers.
- Prefer `constexpr` or `constinit` for frozen containers whenever possible.
- For searching, consider the Boyer-Moore or Knuth-Morris-Pratt algorithms from the frozen library if they are faster than `std::search`.

# Project coding standards for Perl

## Naming Conventions
- Use snake_case for variable names.
- Use PascalCase for package names.
- Use PascalCase for Wx:: method names.
- Use UPPER_SNAKE_CASE for constants.
- Begin wxWidgets functions with Wx:

## Code Structure
- Use 4 spaces for indentation.
- Limit lines to 100 characters.
- Use blank lines to separate logical sections of code.

# Project coding standards for Python

## Naming Conventions
- Use snake_case for variable and function names.
- Use PascalCase for package names.
- Use PascalCase for wx. method names.
- Use UPPER_SNAKE_CASE for constants.
- Begin wxWidgets functions with wx.

## Code Structure
- Use 4 spaces for indentation.
- Limit lines to 90 characters.
- Use blank lines to separate logical sections of code.

# Project coding standards for Ruby

## Naming Conventions
- Use snake_case for variable and function names.
- Use PascalCase for class names.
- Use PascalCase for Wx:: method names.
- Use UPPER_SNAKE_CASE for constants.
- Begin wxWidgets functions with Wx:

## Code Structure
- Use 2 spaces for indentation.
- Limit lines to 80 characters.
- Use blank lines to separate logical sections of code.
