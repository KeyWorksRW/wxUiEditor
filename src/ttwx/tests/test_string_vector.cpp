/////////////////////////////////////////////////////////////////////////////
// Purpose:   Catch2 tests for ttwx::StringVector class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <catch2/catch_test_macros.hpp>
#include <wx/string.h>

#include "ttwx_string_vector.h"

TEST_CASE("StringVector::SetString with char separator", "[StringVector]")
{
    SECTION("Basic semicolon separation")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one;two;three"), ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Comma separation")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("apple,banana,cherry"), ',');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "apple");
        REQUIRE(vec[1] == "banana");
        REQUIRE(vec[2] == "cherry");
    }

    SECTION("Single element")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("single"), ';');
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == "single");
    }

    SECTION("Empty string")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view(""), ';');
        REQUIRE(vec.empty());
    }

    SECTION("Empty elements")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one;;three"), ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Trailing separator")
    {
        ttwx::StringVector vec;
        // StringVector does *not* consider a trailing separator to indicate an empty element
        vec.SetString(std::string_view("one;two;"), ';');
        REQUIRE(vec.size() == 2);
    }
}

TEST_CASE("StringVector::SetString with string_view separator", "[StringVector]")
{
    SECTION("Double character separator")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one||two||three"), std::string_view("||"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Multi-character separator")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one<->two<->three"), std::string_view("<->"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Separator not found")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("no separator here"), std::string_view("||"));
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == "no separator here");
    }
}

TEST_CASE("StringVector::SetString with multiple separators", "[StringVector]")
{
    SECTION("Multiple line endings")
    {
        std::vector<std::string_view> separators = { "\r\n", "\r", "\n" };
        ttwx::StringVector vec;
        vec.SetString(std::string_view("line1\r\nline2\nline3\rline4"), separators);
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
        REQUIRE(vec[3] == "line4");
    }

    SECTION("Multiple punctuation separators")
    {
        std::vector<std::string_view> separators = { ";", "," };
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one;two,three;four"), separators);
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
        REQUIRE(vec[3] == "four");
    }
}

TEST_CASE("StringVector::SetString with TRIM options", "[StringVector]")
{
    SECTION("TRIM::right")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one  ;  two  ;  three  "), ';', ttwx::TRIM::right);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "  two");
        REQUIRE(vec[2] == "  three");
    }

    SECTION("TRIM::left")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("  one;  two;  three"), ';', ttwx::TRIM::left);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("TRIM::both")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("  one  ;  two  ;  three  "), ';', ttwx::TRIM::both);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("TRIM::none")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("  one  ;  two  "), ';', ttwx::TRIM::none);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "  one  ");
        REQUIRE(vec[1] == "  two  ");
    }
}

TEST_CASE("StringVector with wxString", "[StringVector]")
{
    SECTION("wxString with char separator")
    {
        ttwx::StringVector vec;
        wxString str = wxT("one;two;three");
        vec.SetString(str, ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("wxString with string_view separator")
    {
        ttwx::StringVector vec;
        wxString str = wxT("one||two||three");
        vec.SetString(str, std::string_view("||"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }
}

TEST_CASE("StringVector::ReadString", "[StringVector]")
{
    SECTION("Read multi-line string")
    {
        ttwx::StringVector vec;
        vec.ReadString(std::string_view("line1\nline2\nline3"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
    }

    SECTION("Read with Windows line endings")
    {
        ttwx::StringVector vec;
        vec.ReadString(std::string_view("line1\r\nline2\r\nline3"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
    }

    SECTION("Read with mixed line endings")
    {
        ttwx::StringVector vec;
        vec.ReadString(std::string_view("line1\r\nline2\nline3\rline4"));
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
        REQUIRE(vec[3] == "line4");
    }

    SECTION("Read empty string")
    {
        ttwx::StringVector vec;
        vec.ReadString(std::string_view(""));
        REQUIRE(vec.empty());
    }

    SECTION("Read single line without newline")
    {
        ttwx::StringVector vec;
        vec.ReadString(std::string_view("single line"));
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == "single line");
    }

    SECTION("Read with wxString")
    {
        ttwx::StringVector vec;
        wxString str = wxT("line1\nline2");
        vec.ReadString(str);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
    }
}

TEST_CASE("StringVector::is_sameas", "[StringVector]")
{
    SECTION("Identical vectors")
    {
        ttwx::StringVector vec1;
        vec1.SetString(std::string_view("one;two;three"), ';');
        ttwx::StringVector vec2;
        vec2.SetString(std::string_view("one;two;three"), ';');
        REQUIRE(vec1.is_sameas(vec2));
    }

    SECTION("Different sizes")
    {
        ttwx::StringVector vec1;
        vec1.SetString(std::string_view("one;two"), ';');
        ttwx::StringVector vec2;
        vec2.SetString(std::string_view("one;two;three"), ';');
        REQUIRE_FALSE(vec1.is_sameas(vec2));
    }

    SECTION("Different content")
    {
        ttwx::StringVector vec1;
        vec1.SetString(std::string_view("one;two;three"), ';');
        ttwx::StringVector vec2;
        vec2.SetString(std::string_view("one;two;four"), ';');
        REQUIRE_FALSE(vec1.is_sameas(vec2));
    }

    SECTION("Empty vectors")
    {
        ttwx::StringVector vec1;
        ttwx::StringVector vec2;
        REQUIRE(vec1.is_sameas(vec2));
    }
}

TEST_CASE("StringVector constructor with parameters", "[StringVector]")
{
    SECTION("Constructor with char separator")
    {
        ttwx::StringVector vec(std::string_view("one;two;three"), ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Constructor with string_view separator")
    {
        ttwx::StringVector vec(std::string_view("one||two||three"), std::string_view("||"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Constructor with trim option")
    {
        ttwx::StringVector vec(std::string_view("  one  ;  two  "), ';', ttwx::TRIM::both);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
    }
}

TEST_CASE("StringVector vector operations", "[StringVector]")
{
    SECTION("Clear and reuse")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one;two;three"), ';');
        REQUIRE(vec.size() == 3);

        vec.SetString(std::string_view("a,b"), ',');
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "a");
        REQUIRE(vec[1] == "b");
    }

    SECTION("Iterate through vector")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one;two;three"), ';');

        size_t count = 0;
        for (const auto& str : vec)
        {
            REQUIRE_FALSE(str.empty());
            count++;
        }
        REQUIRE(count == 3);
    }

    SECTION("Modify vector elements")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one;two;three"), ';');
        vec[1] = "modified";
        REQUIRE(vec[1] == "modified");
    }

    SECTION("Add elements")
    {
        ttwx::StringVector vec;
        vec.SetString(std::string_view("one;two"), ';');
        vec.push_back("three");
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[2] == "three");
    }
}
