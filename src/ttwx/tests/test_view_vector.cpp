/////////////////////////////////////////////////////////////////////////////
// Purpose:   Catch2 tests for ttwx::ViewVector class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <catch2/catch_test_macros.hpp>
#include <wx/string.h>

#include "ttwx_view_vector.h"

TEST_CASE("ViewVector::SetString with char separator", "[ViewVector]")
{
    SECTION("Basic semicolon separation")
    {
        std::string source = "one;two;three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Comma separation")
    {
        std::string source = "apple,banana,cherry";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ',');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "apple");
        REQUIRE(vec[1] == "banana");
        REQUIRE(vec[2] == "cherry");
    }

    SECTION("Single element")
    {
        std::string source = "single";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == "single");
    }

    SECTION("Empty string")
    {
        std::string source = "";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');
        REQUIRE(vec.empty());
    }

    SECTION("Empty elements")
    {
        std::string source = "one;;three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Trailing separator")
    {
        std::string source = "one;two;";
        ttwx::ViewVector vec;
        // ViewVector does *not* consider a trailing separator to indicate an empty element
        vec.SetString(std::string_view(source), ';');
        REQUIRE(vec.size() == 2);
    }
}

TEST_CASE("ViewVector::SetString with string_view separator", "[ViewVector]")
{
    SECTION("Double character separator")
    {
        std::string source = "one||two||three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), std::string_view("||"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Multi-character separator")
    {
        std::string source = "one<->two<->three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), std::string_view("<->"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Separator not found")
    {
        std::string source = "no separator here";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), std::string_view("||"));
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == "no separator here");
    }
}

TEST_CASE("ViewVector::SetString with multiple separators", "[ViewVector]")
{
    SECTION("Multiple line endings")
    {
        std::string source = "line1\r\nline2\nline3\rline4";
        std::vector<std::string_view> separators = { "\r\n", "\r", "\n" };
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), separators);
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
        REQUIRE(vec[3] == "line4");
    }

    SECTION("Multiple punctuation separators")
    {
        std::string source = "one;two,three;four";
        std::vector<std::string_view> separators = { ";", "," };
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), separators);
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
        REQUIRE(vec[3] == "four");
    }
}

TEST_CASE("ViewVector::SetString with TRIM options", "[ViewVector]")
{
    SECTION("TRIM::right")
    {
        std::string source = "one  ;  two  ;  three  ";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';', ttwx::TRIM::right);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "  two");
        REQUIRE(vec[2] == "  three");
    }

    SECTION("TRIM::left")
    {
        std::string source = "  one;  two;  three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';', ttwx::TRIM::left);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("TRIM::both")
    {
        std::string source = "  one  ;  two  ;  three  ";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';', ttwx::TRIM::both);
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("TRIM::none")
    {
        std::string source = "  one  ;  two  ";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';', ttwx::TRIM::none);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "  one  ");
        REQUIRE(vec[1] == "  two  ");
    }
}

TEST_CASE("ViewVector with wxString", "[ViewVector]")
{
    SECTION("wxString with char separator")
    {
        ttwx::ViewVector vec;
        wxString str = wxT("one;two;three");
        vec.SetString(str, ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("wxString with string_view separator")
    {
        ttwx::ViewVector vec;
        wxString str = wxT("one||two||three");
        vec.SetString(str, std::string_view("||"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }
}

TEST_CASE("ViewVector::ReadString", "[ViewVector]")
{
    SECTION("Read multi-line string")
    {
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view("line1\nline2\nline3"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
    }

    SECTION("Read with Windows line endings")
    {
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view("line1\r\nline2\r\nline3"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
    }

    SECTION("Read with mixed line endings")
    {
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view("line1\r\nline2\nline3\rline4"));
        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
        REQUIRE(vec[2] == "line3");
        REQUIRE(vec[3] == "line4");
    }

    SECTION("Read empty string")
    {
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view(""));
        REQUIRE(vec.empty());
    }

    SECTION("Read single line without newline")
    {
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view("single line"));
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == "single line");
    }

    SECTION("Read with wxString")
    {
        ttwx::ViewVector vec;
        wxString str = wxT("line1\nline2");
        vec.ReadString(str);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
    }
}

TEST_CASE("ViewVector::is_sameas", "[ViewVector]")
{
    SECTION("Identical vectors")
    {
        std::string source = "one;two;three";
        ttwx::ViewVector vec1;
        vec1.SetString(std::string_view(source), ';');
        ttwx::ViewVector vec2;
        vec2.SetString(std::string_view(source), ';');
        REQUIRE(vec1.is_sameas(vec2));
    }

    SECTION("Different sizes")
    {
        std::string source1 = "one;two";
        std::string source2 = "one;two;three";
        ttwx::ViewVector vec1;
        vec1.SetString(std::string_view(source1), ';');
        ttwx::ViewVector vec2;
        vec2.SetString(std::string_view(source2), ';');
        REQUIRE_FALSE(vec1.is_sameas(vec2));
    }

    SECTION("Different content")
    {
        std::string source1 = "one;two;three";
        std::string source2 = "one;two;four";
        ttwx::ViewVector vec1;
        vec1.SetString(std::string_view(source1), ';');
        ttwx::ViewVector vec2;
        vec2.SetString(std::string_view(source2), ';');
        REQUIRE_FALSE(vec1.is_sameas(vec2));
    }

    SECTION("Empty vectors")
    {
        ttwx::ViewVector vec1;
        ttwx::ViewVector vec2;
        REQUIRE(vec1.is_sameas(vec2));
    }
}

TEST_CASE("ViewVector constructor with parameters", "[ViewVector]")
{
    SECTION("Constructor with char separator")
    {
        std::string source = "one;two;three";
        ttwx::ViewVector vec(std::string_view(source), ';');
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Constructor with string_view separator")
    {
        std::string source = "one||two||three";
        ttwx::ViewVector vec(std::string_view(source), std::string_view("||"));
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }

    SECTION("Constructor with trim option")
    {
        std::string source = "  one  ;  two  ";
        ttwx::ViewVector vec(std::string_view(source), ';', ttwx::TRIM::both);
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
    }
}

TEST_CASE("ViewVector vector operations", "[ViewVector]")
{
    SECTION("Clear and reuse")
    {
        std::string source1 = "one;two;three";
        std::string source2 = "a,b";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source1), ';');
        REQUIRE(vec.size() == 3);

        vec.SetString(std::string_view(source2), ',');
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "a");
        REQUIRE(vec[1] == "b");
    }

    SECTION("Iterate through vector")
    {
        std::string source = "one;two;three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');

        size_t count = 0;
        for (const auto& view : vec)
        {
            REQUIRE_FALSE(view.empty());
            count++;
        }
        REQUIRE(count == 3);
    }

    SECTION("Access elements by index")
    {
        std::string source = "one;two;three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");
    }
}

TEST_CASE("ViewVector::GetBuffer", "[ViewVector]")
{
    SECTION("GetBuffer returns the internal buffer")
    {
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view("line1\nline2\nline3"));
        const auto& buffer = vec.GetBuffer();
        REQUIRE_FALSE(buffer.empty());
        REQUIRE(buffer.find("line1") != std::string::npos);
        REQUIRE(buffer.find("line2") != std::string::npos);
        REQUIRE(buffer.find("line3") != std::string::npos);
    }

    SECTION("Views reference buffer memory")
    {
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view("one\ntwo"));
        const auto& buffer = vec.GetBuffer();

        // Views should point into the buffer
        REQUIRE(vec[0].data() >= buffer.data());
        REQUIRE(vec[0].data() < buffer.data() + buffer.size());
    }
}

TEST_CASE("ViewVector lifetime management", "[ViewVector]")
{
    SECTION("Views remain valid after modifying vector size")
    {
        std::string source = "one;two;three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');

        auto first_view = vec[0];
        vec.push_back("four");  // Adding element

        // Original view should still be valid
        REQUIRE(first_view == "one");
        REQUIRE(vec[0] == "one");
    }

    SECTION("Source string must outlive views when using SetString")
    {
        std::string source = "one;two;three";
        ttwx::ViewVector vec;
        vec.SetString(std::string_view(source), ';');

        // Views are valid as long as source exists
        REQUIRE(vec[0] == "one");
        REQUIRE(vec[1] == "two");
        REQUIRE(vec[2] == "three");

        // If source is modified, views become invalid
        source = "modified";
        // Note: We don't test the views here as they would be pointing to invalid memory
    }

    SECTION("ReadString creates internal buffer")
    {
        std::string original = "line1\nline2";
        ttwx::ViewVector vec;
        vec.ReadString(std::string_view(original));

        // Modifying original shouldn't affect vec since ReadString makes a copy
        original = "modified";

        REQUIRE(vec[0] == "line1");
        REQUIRE(vec[1] == "line2");
    }
}
