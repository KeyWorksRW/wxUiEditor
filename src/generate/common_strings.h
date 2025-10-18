/**
 * @file common_strings.h
 * @brief Common strings used in code generation.
 *
 * @author Ralph Walden
 * @copyright Copyright (c) 2025 KeyWorks Software (Ralph Walden)
 * @license Apache License -- see ../../LICENSE
 */

/**
 * @var ruby_begin_cmt_block
 * @brief Ruby begin comment block marker ("=begin").
 *
 * This string must be written on a line by itself with no indentation.
 */
extern const std::string_view ruby_begin_cmt_block;

/**
 * @var ruby_end_cmt_block
 * @brief Ruby end comment block marker ("=end").
 *
 * This string must be written on a line by itself with no indentation.
 */
extern const std::string_view ruby_end_cmt_block;

/**
 * @brief String view containing the end-of-generated-code comment for Python, Perl, and Ruby.
 *
 * This string is typically used to mark the end of generated code in source files
 * for Python, Perl, and Ruby, using the appropriate comment syntax.
 *
 * Example value: "# ************* End of generated code"
 */
extern const std::string_view python_perl_ruby_end_cmt_line;  // "# ************* End of generated code"
/**
 * @var perl_begin_cmt_block
 * @brief Perl begin comment block marker ("=pod").
 *
 * This string must be written on a line by itself with no indentation.
 */
extern const std::string_view perl_begin_cmt_block;

/**
 * @var perl_end_cmt_block
 * @brief Perl end comment block marker ("=cut").
 *
 * This string must be written on a line by itself with no indentation.
 */
extern const std::string_view perl_end_cmt_block;

// "\"\"\"";
/**
 * @brief A string view representing the Python triple quote sequence.
 *
 * This constant can be used when generating or parsing Python code that requires
 * triple quotes (""" ... """) for multi-line strings or docstrings.
 */
extern const std::string_view python_triple_quote;  // "\"\"\"";
