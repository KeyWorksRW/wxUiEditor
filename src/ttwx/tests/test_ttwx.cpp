/////////////////////////////////////////////////////////////////////////////
// Purpose:   Catch2 tests for ttwx namespace functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <catch2/catch_test_macros.hpp>
#include <wx/string.h>

#include "ttwx.h"  // NOLINT (header file not used) // cppcheck-suppress notusedInclude

TEST_CASE("ttwx::find_oneof", "[ttwx]")
{
    SECTION("Find characters in string")
    {
        wxString test_str = "Hello, World!";

        // Find comma or exclamation
        auto pos = ttwx::find_oneof(test_str, ",!", 0);
        REQUIRE(pos == 5);

        // Find from offset
        pos = ttwx::find_oneof(test_str, ",!", 6);
        REQUIRE(pos == 12);

        // Not found
        pos = ttwx::find_oneof(test_str, "xyz", 0);
        REQUIRE(pos == std::string::npos);
    }

    SECTION("Empty inputs")
    {
        wxString test_str = "Hello";
        auto pos = ttwx::find_oneof(test_str, "", 0);
        REQUIRE(pos == std::string::npos);

        wxString empty_str;
        pos = ttwx::find_oneof(empty_str, "abc", 0);
        REQUIRE(pos == std::string::npos);
    }

    SECTION("Start position beyond string")
    {
        wxString test_str = "Hello";
        auto pos = ttwx::find_oneof(test_str, "H", 10);
        REQUIRE(pos == std::string::npos);
    }
}

TEST_CASE("ttwx::find_nonspace", "[ttwx]")
{
    SECTION("Find non-whitespace in string")
    {
        std::string_view test_str = "   Hello World";
        auto result = ttwx::find_nonspace(test_str);
        REQUIRE(result == "Hello World");
    }

    SECTION("String with tabs and spaces")
    {
        std::string_view test_str = "\t\t  Text";
        auto result = ttwx::find_nonspace(test_str);
        REQUIRE(result == "Text");
    }

    SECTION("No leading whitespace")
    {
        std::string_view test_str = "NoSpace";
        auto result = ttwx::find_nonspace(test_str);
        REQUIRE(result == "NoSpace");
    }

    SECTION("All whitespace")
    {
        std::string_view test_str = "   \t\t  ";
        auto result = ttwx::find_nonspace(test_str);
        REQUIRE(result.empty());
    }

    SECTION("Empty string")
    {
        std::string_view test_str;
        auto result = ttwx::find_nonspace(test_str);
        REQUIRE(result.empty());
    }
}

TEST_CASE("ttwx::find_extension", "[ttwx]")
{
    SECTION("Normal file extensions")
    {
        auto result = ttwx::find_extension("file.txt");
        REQUIRE(result == ".txt");

        result = ttwx::find_extension("document.cpp");
        REQUIRE(result == ".cpp");

        result = ttwx::find_extension("archive.tar.gz");
        REQUIRE(result == ".gz");
    }

    SECTION("No extension")
    {
        auto result = ttwx::find_extension("filename");
        REQUIRE(result.empty());

        result = ttwx::find_extension("folder/");
        REQUIRE(result.empty());
    }

    SECTION("Hidden files (starting with dot)")
    {
        auto result = ttwx::find_extension(".gitignore");
        REQUIRE(result.empty());

        result = ttwx::find_extension(".hidden.txt");
        REQUIRE(result == ".txt");
    }

    SECTION("Directory navigation")
    {
        auto result = ttwx::find_extension(".");
        REQUIRE(result.empty());

        result = ttwx::find_extension("..");
        REQUIRE(result.empty());
    }

    SECTION("Extension with path")
    {
        auto result = ttwx::find_extension("/path/to/file.cpp");
        REQUIRE(result == ".cpp");
    }
}

TEST_CASE("ttwx::replace_extension", "[ttwx]")
{
    SECTION("Replace existing extension")
    {
        wxString filename = "document.txt";
        ttwx::replace_extension(filename, ".cpp");
        REQUIRE(filename == "document.cpp");

        filename = "file.old.ext";
        ttwx::replace_extension(filename, ".new");
        REQUIRE(filename == "file.old.new");
    }

    SECTION("Add extension to file without one")
    {
        wxString filename = "document";
        ttwx::replace_extension(filename, ".txt");
        REQUIRE(filename == "document.txt");
    }

    SECTION("Extension without leading dot")
    {
        wxString filename = "file.txt";
        ttwx::replace_extension(filename, "cpp");
        REQUIRE(filename == "file.cpp");
    }

    SECTION("Remove extension")
    {
        wxString filename = "document.txt";
        ttwx::replace_extension(filename, "");
        REQUIRE(filename == "document");
    }

    SECTION("Replace extension with path")
    {
        wxString filename = "C:\\path\\to\\file.txt";
        ttwx::replace_extension(filename, ".cpp");
        REQUIRE(filename.Contains(".cpp"));
        REQUIRE_FALSE(filename.Contains(".txt"));
    }
}

TEST_CASE("ttwx::find_filename", "[ttwx]")
{
    SECTION("Unix-style paths")
    {
        auto result = ttwx::find_filename("/path/to/file.txt");
        REQUIRE(result == "file.txt");

        result = ttwx::find_filename("/usr/local/bin/program");
        REQUIRE(result == "program");
    }

#ifdef _WIN32
    SECTION("Windows-style paths")
    {
        auto result = ttwx::find_filename("C:\\Users\\Name\\file.txt");
        REQUIRE(result == "file.txt");

        result = ttwx::find_filename("C:\\folder\\subfolder\\document.doc");
        REQUIRE(result == "document.doc");
    }

    SECTION("Mixed slashes (Windows)")
    {
        auto result = ttwx::find_filename("C:/Users/Name\\file.txt");
        REQUIRE(result == "file.txt");
    }

    SECTION("Drive letter only")
    {
        auto result = ttwx::find_filename("C:file.txt");
        REQUIRE(result == "file.txt");
    }
#endif

    SECTION("No path separator")
    {
        auto result = ttwx::find_filename("filename.txt");
        REQUIRE(result == "filename.txt");
    }

    SECTION("Empty string")
    {
        auto result = ttwx::find_filename("");
        REQUIRE(result.empty());
    }

    SECTION("Trailing slash")
    {
        auto result = ttwx::find_filename("/path/to/folder/");
        REQUIRE(result.empty());
    }
}

TEST_CASE("ttwx::append_filename", "[ttwx]")
{
    SECTION("Append to path with trailing slash")
    {
        wxString path = "/path/to/folder/";
        ttwx::append_filename(path, "file.txt");
        REQUIRE(path == "/path/to/folder/file.txt");
    }

    SECTION("Append to path without trailing slash")
    {
        wxString path = "/path/to/folder";
        ttwx::append_filename(path, "file.txt");
        REQUIRE(path == "/path/to/folder/file.txt");
    }

    SECTION("Append to empty path")
    {
        wxString path;
        ttwx::append_filename(path, "file.txt");
        REQUIRE(path == "file.txt");
    }

    SECTION("Append empty filename")
    {
        wxString path = "/path/to/folder";
        ttwx::append_filename(path, "");
        REQUIRE(path == "/path/to/folder");
    }

    SECTION("Chaining calls")
    {
        wxString path = "/base";
        ttwx::append_filename(path, "file1.txt");
        // Note: This will convert file1.txt to a folder and append file2.txt
        auto& result = ttwx::append_filename(path, "file2.txt");
        REQUIRE(&result == &path);  // Check it returns reference
    }
}

TEST_CASE("ttwx::append_folder_name", "[ttwx]")
{
    SECTION("Append folder to path")
    {
        wxString path = "/path/to";
        ttwx::append_folder_name(path, "folder");
        REQUIRE(path == "/path/to/folder/");
    }

    SECTION("Append to path without trailing slash")
    {
        wxString path = "/path";
        ttwx::append_folder_name(path, "subfolder");
        REQUIRE(path == "/path/subfolder/");
    }

    SECTION("Append to empty path")
    {
        wxString path;
        ttwx::append_folder_name(path, "folder");
        REQUIRE(path == "folder/");
    }

    SECTION("Append empty folder name")
    {
        wxString path = "/path";
        ttwx::append_folder_name(path, "");
        REQUIRE(path == "/path");
    }

    SECTION("Multiple appends")
    {
        wxString path = "/base";
        ttwx::append_folder_name(path, "level1");
        ttwx::append_folder_name(path, "level2");
        REQUIRE(path == "/base/level1/level2/");
    }
}

TEST_CASE("ttwx::extract_substring", "[ttwx]")
{
    SECTION("Double quotes")
    {
        wxString dest;
        std::string_view src = R"(  "Hello World"  )";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == "Hello World");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("Single quotes")
    {
        wxString dest;
        std::string_view src = "  'Test String'  ";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == "Test String");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("Angle brackets")
    {
        wxString dest;
        std::string_view src = "  <header.h>  ";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == "header.h");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("Square brackets")
    {
        wxString dest;
        std::string_view src = "  [array element]  ";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == "array element");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("Parentheses")
    {
        wxString dest;
        std::string_view src = "  (function args)  ";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == "function args");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("Backtick quotes")
    {
        wxString dest;
        std::string_view src = "  `code block`  ";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == "code block");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("Escaped quotes")
    {
        wxString dest;
        std::string_view src = R"(  "He said \"Hello\""  )";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == R"(He said \"Hello\")");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("No closing delimiter")
    {
        wxString dest;
        std::string_view src = R"(  "Unclosed string )";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(pos == std::string::npos);
    }

    SECTION("Empty string")
    {
        wxString dest;
        std::string_view src = "";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(dest == "");
        REQUIRE(pos == std::string::npos);
    }

    SECTION("Start offset beyond whitespace")
    {
        wxString dest;
        std::string_view src = R"(before "extracted" after)";
        auto pos = ttwx::extract_substring(src, dest, 7);
        REQUIRE(dest == "extracted");
        REQUIRE(ttwx::is_found(pos));
    }

    SECTION("Invalid starting character")
    {
        wxString dest;
        std::string_view src = "  NoDelimiter  ";
        auto pos = ttwx::extract_substring(src, dest, 0);
        REQUIRE(pos == std::string::npos);
    }
}

TEST_CASE("ttwx::create_substring", "[ttwx]")
{
    SECTION("Helper function returns substring")
    {
        std::string_view src = R"(  "Test"  )";
        auto result = ttwx::create_substring(src, 0);
        REQUIRE(result == "Test");
    }

    SECTION("With offset")
    {
        std::string_view src = R"(before "Test" after)";
        auto result = ttwx::create_substring(src, 7);
        REQUIRE(result == "Test");
    }
}

TEST_CASE("ttwx::MakeLower", "[ttwx]")
{
    SECTION("Convert uppercase to lowercase")
    {
        std::string str = "HELLO WORLD";
        ttwx::MakeLower(str);
        REQUIRE(str == "hello world");
    }

    SECTION("Mixed case")
    {
        std::string str = "HeLLo WoRLd";
        ttwx::MakeLower(str);
        REQUIRE(str == "hello world");
    }

    SECTION("Already lowercase")
    {
        std::string str = "already lowercase";
        ttwx::MakeLower(str);
        REQUIRE(str == "already lowercase");
    }

    SECTION("With numbers and symbols")
    {
        std::string str = "Test123!@#ABC";
        ttwx::MakeLower(str);
        REQUIRE(str == "test123!@#abc");
    }

    SECTION("Empty string")
    {
        std::string str;
        ttwx::MakeLower(str);
        REQUIRE(str.empty());
    }
}

TEST_CASE("ttwx::atoi", "[ttwx]")
{
    SECTION("Positive decimal numbers")
    {
        REQUIRE(ttwx::atoi("123") == 123);
        REQUIRE(ttwx::atoi("456789") == 456789);
        REQUIRE(ttwx::atoi("0") == 0);
    }

    SECTION("Negative decimal numbers")
    {
        REQUIRE(ttwx::atoi("-123") == -123);
        REQUIRE(ttwx::atoi("-456") == -456);
    }

    SECTION("Numbers with plus sign")
    {
        REQUIRE(ttwx::atoi("+123") == 123);
        REQUIRE(ttwx::atoi("+456") == 456);
    }

    SECTION("Hexadecimal numbers")
    {
        REQUIRE(ttwx::atoi("0x10") == 16);
        REQUIRE(ttwx::atoi("0xFF") == 255);
        REQUIRE(ttwx::atoi("0X1A") == 26);
        REQUIRE(ttwx::atoi("0X1a") == 26);
        REQUIRE(ttwx::atoi("0x0") == 0);
    }

    SECTION("Negative hexadecimal")
    {
        REQUIRE(ttwx::atoi("-0x10") == -16);
        REQUIRE(ttwx::atoi("-0xFF") == -255);
    }

    SECTION("With leading whitespace")
    {
        REQUIRE(ttwx::atoi("  123") == 123);
        REQUIRE(ttwx::atoi("\t456") == 456);
        REQUIRE(ttwx::atoi("  \t  789") == 789);
    }

    SECTION("Empty string")
    {
        REQUIRE(ttwx::atoi("") == 0);
    }

    SECTION("Whitespace only")
    {
        REQUIRE(ttwx::atoi("   ") == 0);
        REQUIRE(ttwx::atoi("\t\t") == 0);
    }

    SECTION("Invalid strings")
    {
        REQUIRE(ttwx::atoi("abc") == 0);
        REQUIRE(ttwx::atoi("xyz123") == 0);
        REQUIRE(ttwx::atoi("!@#") == 0);
    }

    SECTION("Partial conversion")
    {
        // std::from_chars will convert up to the first non-digit
        REQUIRE(ttwx::atoi("123abc") == 123);
        REQUIRE(ttwx::atoi("456 789") == 456);
    }
}

TEST_CASE("ttwx::itoa", "[ttwx]")
{
    SECTION("Convert integers to string")
    {
        REQUIRE(ttwx::itoa(123) == "123");
        REQUIRE(ttwx::itoa(-456) == "-456");
        REQUIRE(ttwx::itoa(0) == "0");
    }

    SECTION("Convert floating point to string")
    {
        auto result = ttwx::itoa(3.14);
        REQUIRE(result.find("3.14") != std::string::npos);
    }
}

TEST_CASE("ttwx::character classification functions", "[ttwx]")
{
    SECTION("is_alnum")
    {
        REQUIRE(ttwx::is_alnum('a'));
        REQUIRE(ttwx::is_alnum('Z'));
        REQUIRE(ttwx::is_alnum('5'));
        REQUIRE_FALSE(ttwx::is_alnum(' '));
        REQUIRE_FALSE(ttwx::is_alnum('!'));
    }

    SECTION("is_alpha")
    {
        REQUIRE(ttwx::is_alpha('a'));
        REQUIRE(ttwx::is_alpha('Z'));
        REQUIRE_FALSE(ttwx::is_alpha('5'));
        REQUIRE_FALSE(ttwx::is_alpha(' '));
    }

    SECTION("is_digit")
    {
        REQUIRE(ttwx::is_digit('0'));
        REQUIRE(ttwx::is_digit('9'));
        REQUIRE_FALSE(ttwx::is_digit('a'));
        REQUIRE_FALSE(ttwx::is_digit(' '));
    }

    SECTION("is_whitespace")
    {
        REQUIRE(ttwx::is_whitespace(' '));
        REQUIRE(ttwx::is_whitespace('\t'));
        REQUIRE(ttwx::is_whitespace('\n'));
        REQUIRE_FALSE(ttwx::is_whitespace('a'));
        REQUIRE_FALSE(ttwx::is_whitespace('0'));
    }

    SECTION("is_upper")
    {
        REQUIRE(ttwx::is_upper('A'));
        REQUIRE(ttwx::is_upper('Z'));
        REQUIRE_FALSE(ttwx::is_upper('a'));
        REQUIRE_FALSE(ttwx::is_upper('0'));
    }

    SECTION("is_lower")
    {
        REQUIRE(ttwx::is_lower('a'));
        REQUIRE(ttwx::is_lower('z'));
        REQUIRE_FALSE(ttwx::is_lower('A'));
        REQUIRE_FALSE(ttwx::is_lower('0'));
    }

    SECTION("is_punctuation")
    {
        REQUIRE(ttwx::is_punctuation('!'));
        REQUIRE(ttwx::is_punctuation('.'));
        REQUIRE(ttwx::is_punctuation(','));
        REQUIRE_FALSE(ttwx::is_punctuation('a'));
        REQUIRE_FALSE(ttwx::is_punctuation('0'));
    }
}

TEST_CASE("ttwx::is_found", "[ttwx]")
{
    SECTION("Check if position is found")
    {
        REQUIRE(ttwx::is_found(0));
        REQUIRE(ttwx::is_found(5));
        REQUIRE(ttwx::is_found(100));
        REQUIRE_FALSE(ttwx::is_found(-1));
        REQUIRE_FALSE(ttwx::is_found(std::string::npos));
    }
}

TEST_CASE("ttwx::back_slashesto_forward", "[ttwx]")
{
    SECTION("Convert backslashes to forward slashes")
    {
        wxString path = R"(C:\Users\Name\file.txt)";
        ttwx::back_slashesto_forward(path);
        REQUIRE(path == "C:/Users/Name/file.txt");
    }

    SECTION("Mixed slashes")
    {
        wxString path = "C:\\Users/Name\\file.txt";
        ttwx::back_slashesto_forward(path);
        REQUIRE(path == "C:/Users/Name/file.txt");
    }

    SECTION("No backslashes")
    {
        wxString path = "C:/Users/Name/file.txt";
        ttwx::back_slashesto_forward(path);
        REQUIRE(path == "C:/Users/Name/file.txt");
    }
}

TEST_CASE("ttwx::forward_slashesto_back", "[ttwx]")
{
    SECTION("Convert forward slashes to backslashes")
    {
        wxString path = "C:/Users/Name/file.txt";
        ttwx::forward_slashesto_back(path);
        REQUIRE(path == "C:\\Users\\Name\\file.txt");
    }

    SECTION("Mixed slashes")
    {
        wxString path = "C:/Users\\Name/file.txt";
        ttwx::forward_slashesto_back(path);
        REQUIRE(path == "C:\\Users\\Name\\file.txt");
    }

    SECTION("No forward slashes")
    {
        wxString path = R"(C:\Users\Name\file.txt)";
        ttwx::forward_slashesto_back(path);
        REQUIRE(path == "C:\\Users\\Name\\file.txt");
    }
}

TEST_CASE("ttwx::add_trailing_slash", "[ttwx]")
{
    SECTION("Add trailing slash if missing")
    {
        wxString path = "/path/to/folder";
        ttwx::add_trailing_slash(path, false);
        REQUIRE(path == "/path/to/folder/");
    }

    SECTION("Don't add if already present (forward)")
    {
        wxString path = "/path/to/folder/";
        ttwx::add_trailing_slash(path, false);
        REQUIRE(path == "/path/to/folder/");
    }

    SECTION("Don't add if backslash present")
    {
        wxString path = "C:\\path\\to\\folder\\";
        ttwx::add_trailing_slash(path, false);
        REQUIRE(path == "C:\\path\\to\\folder\\");
    }

    SECTION("Always add slash")
    {
        wxString path = "/path/to/folder/";
        ttwx::add_trailing_slash(path, true);
        REQUIRE(path == "/path/to/folder//");
    }
}
