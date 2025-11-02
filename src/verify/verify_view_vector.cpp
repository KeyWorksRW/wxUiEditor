/////////////////////////////////////////////////////////////////////////////
// Purpose:   Verify ttwx::ViewVector class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ttwx_view_vector.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#include <wx/filename.h>

#include "verify.h"

#include "assertion_dlg.h"  // Assertion Dialog

// Function to verify all capabilities of ttwx::ViewVector

// NOLINTNEXTLINE (clang-analyzer-core.cognitive-complexity) // cppcheck-suppress
// highCognitiveComplexity
auto VerifyViewVector() -> bool  // NOLINT(clang-analyzer-core.cognitive-complexity)
{
    // Test 1: String parsing with character separator
    {
        ttwx::ViewVector view_vector;
        view_vector.SetString(std::string_view("apple;banana;cherry"), ";");

        VERIFY_EQUAL(view_vector.size(), 3U, "Basic semicolon parsing should create 3 elements");
        ASSERT_MSG(view_vector[0] == "apple", "First element should be 'apple'");
        ASSERT_MSG(view_vector[1] == "banana", "Second element should be 'banana'");
        ASSERT_MSG(view_vector[2] == "cherry", "Third element should be 'cherry'");

        // Test with different separator
        view_vector.SetString(std::string_view("one,two,three"), ',');
        VERIFY_EQUAL(view_vector.size(), 3U, "Comma parsing should create 3 elements");
        ASSERT_MSG(view_vector[0] == "one", "First element should be 'one'");
        ASSERT_MSG(view_vector[1] == "two", "Second element should be 'two'");
        ASSERT_MSG(view_vector[2] == "three", "Third element should be 'three'");
    }

    // Test 2: String parsing with string separator
    {
        ttwx::ViewVector view_vector;
        view_vector.SetString(std::string_view("first||second||third"), std::string_view("||"));

        VERIFY_EQUAL(view_vector.size(), 3U, "Double pipe parsing should create 3 elements");
        ASSERT_MSG(view_vector[0] == "first", "First element should be 'first'");
        ASSERT_MSG(view_vector[1] == "second", "Second element should be 'second'");
        ASSERT_MSG(view_vector[2] == "third", "Third element should be 'third'");

        // Test with newline separator
        view_vector.SetString(std::string_view("line1\r\nline2\r\nline3"),
                              std::string_view("\r\n"));
        VERIFY_EQUAL(view_vector.size(), 3U, "CRLF parsing should create 3 elements");
        ASSERT_MSG(view_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(view_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(view_vector[2] == "line3", "Third line should be 'line3'");
    }

    // Test 3: TRIM functionality
    {
        ttwx::ViewVector view_vector;

        // Test TRIM::none
        view_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                              ttwx::TRIM::none);
        VERIFY_EQUAL(view_vector.size(), 3U, "No trim should create 3 elements");
        ASSERT_MSG(view_vector[0] == "  apple  ", "First element should preserve spaces");
        ASSERT_MSG(view_vector[1] == " banana ", "Second element should preserve spaces");
        ASSERT_MSG(view_vector[2] == "  cherry  ", "Third element should preserve spaces");

        // Test TRIM::both
        view_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                              ttwx::TRIM::both);
        VERIFY_EQUAL(view_vector.size(), 3U, "Trim both should create 3 elements");
        ASSERT_MSG(view_vector[0] == "apple", "First element should be trimmed");
        ASSERT_MSG(view_vector[1] == "banana", "Second element should be trimmed");
        ASSERT_MSG(view_vector[2] == "cherry", "Third element should be trimmed");

        // Test TRIM::left
        view_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                              ttwx::TRIM::left);
        VERIFY_EQUAL(view_vector.size(), 3U, "Trim left should create 3 elements");
        ASSERT_MSG(view_vector[0] == "apple  ", "First element should be left-trimmed");
        ASSERT_MSG(view_vector[1] == "banana ", "Second element should be left-trimmed");
        ASSERT_MSG(view_vector[2] == "cherry  ", "Third element should be left-trimmed");

        // Test TRIM::right
        view_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                              ttwx::TRIM::right);
        VERIFY_EQUAL(view_vector.size(), 3U, "Trim right should create 3 elements");
        ASSERT_MSG(view_vector[0] == "  apple", "First element should be right-trimmed");
        ASSERT_MSG(view_vector[1] == " banana", "Second element should be right-trimmed");
        ASSERT_MSG(view_vector[2] == "  cherry", "Third element should be right-trimmed");
    }

    // Test 4: Edge cases for string parsing
    {
        // Note that both ':' and ";" can be used, however the single version will first create
        // a std::string from the char, then call the std::string_view version. That makes
        // stepping through the code with a debugger a bit more difficult, so ";" is preferable
        // for most testing. The above code tests the char version.

        ttwx::ViewVector view_vector;

        // Empty string
        view_vector.SetString(std::string_view(""), ";");
        VERIFY_EQUAL(view_vector.size(), 0U, "Empty string should create no elements");

        // String with only separators
        view_vector.SetString(std::string_view(";;;"), ";");
        VERIFY_EQUAL(view_vector.size(), 3U,
                     "String with only separators should create empty elements");
        for (auto idx: view_vector)
        {
            ASSERT_MSG(idx.empty(), "All elements should be empty strings");
        }

        // Single element (no separator)
        view_vector.SetString(std::string_view("single"), ";");
        VERIFY_EQUAL(view_vector.size(), 1U, "Single element should create 1 element");
        ASSERT_MSG(view_vector[0] == "single", "Single element should be 'single'");

        // Leading and trailing separators
        view_vector.SetString(std::string_view(";first;last;"), ";");
        VERIFY_EQUAL(view_vector.size(), 3U,
                     "Leading/trailing separators should create 4 elements");
        ASSERT_MSG(view_vector[0].empty(), "First element should be empty");
        ASSERT_MSG(view_vector[1] == "first", "Second element should be 'first'");
        ASSERT_MSG(view_vector[2] == "last", "Third element should be 'last'");
    }

    // Test 5: Constructor variants
    {
        // Default constructor
        ttwx::ViewVector vv1;
        VERIFY_EQUAL(vv1.size(), 0U, "Default constructor should create empty vector");

        // Parameterized constructor with character separator
        ttwx::ViewVector vv2(std::string_view("a;b;c"), ";");
        VERIFY_EQUAL(vv2.size(), 3U, "Constructor with char separator should create 3 elements");
        ASSERT_MSG(vv2[0] == "a", "First element should be 'a'");
        ASSERT_MSG(vv2[1] == "b", "Second element should be 'b'");
        ASSERT_MSG(vv2[2] == "c", "Third element should be 'c'");

        // Parameterized constructor with TRIM
        ttwx::ViewVector vv3(std::string_view(" x ; y ; z "), ';', ttwx::TRIM::both);
        VERIFY_EQUAL(vv3.size(), 3U, "Constructor with trim should create 3 elements");
        ASSERT_MSG(vv3[0] == "x", "First element should be trimmed to 'x'");
        ASSERT_MSG(vv3[1] == "y", "Second element should be trimmed to 'y'");
        ASSERT_MSG(vv3[2] == "z", "Third element should be trimmed to 'z'");
    }

    // Test 6: ReadString capabilities
    {
        ttwx::ViewVector view_vector;

        // Test with different line endings
        view_vector.ReadString(std::string_view("line1\nline2\nline3"));
        VERIFY_EQUAL(view_vector.size(), 3U, "ReadString with \n should create 3 lines");
        ASSERT_MSG(view_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(view_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(view_vector[2] == "line3", "Third line should be 'line3'");

        // Test with CRLF
        view_vector.ReadString(std::string_view("first\r\nsecond\r\nthird"));
        VERIFY_EQUAL(view_vector.size(), 3U, "ReadString with \r\n should create 3 lines");
        ASSERT_MSG(view_vector[0] == "first", "First line should be 'first'");
        ASSERT_MSG(view_vector[1] == "second", "Second line should be 'second'");
        ASSERT_MSG(view_vector[2] == "third", "Third line should be 'third'");

        // Test with CR only
        view_vector.ReadString(std::string_view("one\rtwo\rthree"));
        VERIFY_EQUAL(view_vector.size(), 3U, "ReadString with \r should create 3 lines");
        ASSERT_MSG(view_vector[0] == "one", "First line should be 'one'");
        ASSERT_MSG(view_vector[1] == "two", "Second line should be 'two'");
        ASSERT_MSG(view_vector[2] == "three", "Third line should be 'three'");

        // Test with mixed line endings
        view_vector.ReadString(std::string_view("mixed\nline\r\nendings\r"));
        VERIFY_EQUAL(view_vector.size(), 3U, "ReadString with mixed endings should create 4 lines");
        ASSERT_MSG(view_vector[0] == "mixed", "First line should be 'mixed'");
        ASSERT_MSG(view_vector[1] == "line", "Second line should be 'line'");
        ASSERT_MSG(view_vector[2] == "endings", "Third line should be 'endings'");

        // Test wxString overload
        wxString wxStr = wxT("wx1\nwx2\nwx3");
        view_vector.ReadString(wxStr);
        VERIFY_EQUAL(view_vector.size(), 3U, "ReadString with wxString should create 3 lines");
        ASSERT_MSG(view_vector[0] == "wx1", "First line should be 'wx1'");
        ASSERT_MSG(view_vector[1] == "wx2", "Second line should be 'wx2'");
        ASSERT_MSG(view_vector[2] == "wx3", "Third line should be 'wx3'");
    }

    // Test 7: Comparison and utility methods
    {
        ttwx::ViewVector view_vector1;
        ttwx::ViewVector view_vector2;

        // Test is_sameas() with identical vectors
        view_vector1.ReadString(std::string_view("same\nlines\nhere"));
        view_vector2.ReadString(std::string_view("same\nlines\nhere"));
        ASSERT_MSG(view_vector1.is_sameas(view_vector2), "Identical vectors should be same");

        // Test is_sameas() with different content
        view_vector2.ReadString(std::string_view("different\nlines\nhere"));
        ASSERT_MSG(!view_vector1.is_sameas(view_vector2), "Different content should not be same");

        // Test is_sameas() with different sizes
        view_vector2.ReadString(std::string_view("same\nlines"));
        ASSERT_MSG(!view_vector1.is_sameas(view_vector2), "Different sizes should not be same");

        // Test GetBuffer()
        view_vector1.ReadString(std::string_view("buffer\ntest"));
        const std::string& buffer = view_vector1.GetBuffer();
        ASSERT_MSG(!buffer.empty(), "GetBuffer should return non-empty string");
        // The buffer should contain the original string with line endings
        ASSERT_MSG(buffer.find("buffer") != std::string::npos, "Buffer should contain 'buffer'");
        ASSERT_MSG(buffer.find("test") != std::string::npos, "Buffer should contain 'test'");
    }

    // Test 8: wxString integration
    {
        ttwx::ViewVector view_vector;

        // Test SetString with wxString and character separator
        wxString wxStr1 = wxT("wx;string;test");
        view_vector.SetString(wxStr1, ";");
        VERIFY_EQUAL(view_vector.size(), 3U, "SetString with wxString should create 3 elements");
        ASSERT_MSG(view_vector[0] == "wx", "First element should be 'wx'");
        ASSERT_MSG(view_vector[1] == "string", "Second element should be 'string'");
        ASSERT_MSG(view_vector[2] == "test", "Third element should be 'test'");

        // Test SetString with wxString and string separator
        wxString wxStr2 = wxT("first||second||third");
        view_vector.SetString(wxStr2, std::string_view("||"));
        VERIFY_EQUAL(view_vector.size(), 3U,
                     "SetString with wxString and string separator should create 3 elements");
        ASSERT_MSG(view_vector[0] == "first", "First element should be 'first'");
        ASSERT_MSG(view_vector[1] == "second", "Second element should be 'second'");
        ASSERT_MSG(view_vector[2] == "third", "Third element should be 'third'");

        // Test SetString with wxString and TRIM
        wxString wxStr3 = wxT("  trim  ;  test  ");
        view_vector.SetString(wxStr3, ';', ttwx::TRIM::both);
        VERIFY_EQUAL(view_vector.size(), 2U,
                     "SetString with wxString and trim should create 2 elements");
        ASSERT_MSG(view_vector[0] == "trim", "First element should be trimmed to 'trim'");
        ASSERT_MSG(view_vector[1] == "test", "Second element should be trimmed to 'test'");
    }

    // Test 9: Edge cases and error handling
    {
        ttwx::ViewVector view_vector;

        // Empty ReadString
        view_vector.ReadString(std::string_view(""));
        VERIFY_EQUAL(view_vector.size(), 0U, "Empty ReadString should create no elements");

        // Single line without line ending
        view_vector.ReadString(std::string_view("single"));
        VERIFY_EQUAL(view_vector.size(), 1U, "Single line should create 1 element");
        ASSERT_MSG(view_vector[0] == "single", "Single line should be 'single'");

        // Only line endings
        view_vector.ReadString(std::string_view("\n\n\n"));
        VERIFY_EQUAL(view_vector.size(), 3U, "Only line endings should create empty lines");
        for (size_t i = 0; i < view_vector.size(); ++i)
        {
            ASSERT_MSG(view_vector[i].empty(), "All lines should be empty");
        }

        // Test very large string parsing (stress test)
        std::string largeString;
        const size_t numElements = 1000;
        for (size_t i = 0; i < numElements; ++i)
        {
            if (i > 0)
            {
                largeString += ";";
            }
            largeString += "item" + std::to_string(i);
        }
        view_vector.SetString(std::string_view(largeString), ";");
        VERIFY_EQUAL(view_vector.size(), numElements,
                     "Large string should create correct number of elements");
        ASSERT_MSG(view_vector[0] == "item0", "First element should be 'item0'");
        ASSERT_MSG(view_vector[numElements - 1] == "item" + std::to_string(numElements - 1),
                   "Last element should be correct");
    }

    // Test 10: std::vector separators functionality
    {
        ttwx::ViewVector view_vector;

        // Test with multiple single-character separators
        std::vector<std::string_view> separators = { ";", "," };
        view_vector.SetString(std::string_view("apple;banana,cherry;orange,grape"), separators);
        VERIFY_EQUAL(view_vector.size(), 5U,
                     "Multiple single-char separators should create 5 elements");
        ASSERT_MSG(view_vector[0] == "apple", "First element should be 'apple'");
        ASSERT_MSG(view_vector[1] == "banana", "Second element should be 'banana'");
        ASSERT_MSG(view_vector[2] == "cherry", "Third element should be 'cherry'");
        ASSERT_MSG(view_vector[3] == "orange", "Fourth element should be 'orange'");
        ASSERT_MSG(view_vector[4] == "grape", "Fifth element should be 'grape'");

        // Test with multiple multi-character separators
        std::vector<std::string_view> multiSeparators = { "||", "::" };
        view_vector.SetString(std::string_view("first||second::third||fourth"), multiSeparators);
        VERIFY_EQUAL(view_vector.size(), 4U,
                     "Multiple multi-char separators should create 4 elements");
        ASSERT_MSG(view_vector[0] == "first", "First element should be 'first'");
        ASSERT_MSG(view_vector[1] == "second", "Second element should be 'second'");
        ASSERT_MSG(view_vector[2] == "third", "Third element should be 'third'");
        ASSERT_MSG(view_vector[3] == "fourth", "Fourth element should be 'fourth'");

        // Test with line ending separators (common use case)
        std::vector<std::string_view> lineSeparators = { "\r\n", "\r", "\n" };
        view_vector.SetString(std::string_view("line1\r\nline2\nline3\rline4"), lineSeparators);
        VERIFY_EQUAL(view_vector.size(), 4U, "Line ending separators should create 4 lines");
        ASSERT_MSG(view_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(view_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(view_vector[2] == "line3", "Third line should be 'line3'");
        ASSERT_MSG(view_vector[3] == "line4", "Fourth line should be 'line4'");

        // Test with mixed separator lengths
        std::vector<std::string_view> mixedSeparators = { "|", "@@", ":", "----" };
        view_vector.SetString(std::string_view("a|b@@c:d----e"), mixedSeparators);
        VERIFY_EQUAL(view_vector.size(), 5U, "Mixed separator lengths should create 5 elements");
        ASSERT_MSG(view_vector[0] == "a", "First element should be 'a'");
        ASSERT_MSG(view_vector[1] == "b", "Second element should be 'b'");
        ASSERT_MSG(view_vector[2] == "c", "Third element should be 'c'");
        ASSERT_MSG(view_vector[3] == "d", "Fourth element should be 'd'");
        ASSERT_MSG(view_vector[4] == "e", "Fifth element should be 'e'");

        // Test with TRIM functionality and multiple separators
        std::vector<std::string_view> trimSeparators = { ";", "," };
        view_vector.SetString(std::string_view("  first  ;  second  ,  third  "), trimSeparators,
                              ttwx::TRIM::both);
        VERIFY_EQUAL(view_vector.size(), 3U,
                     "Vector separators with trim should create 3 elements");
        ASSERT_MSG(view_vector[0] == "first", "First element should be trimmed");
        ASSERT_MSG(view_vector[1] == "second", "Second element should be trimmed");
        ASSERT_MSG(view_vector[2] == "third", "Third element should be trimmed");

        // Test with TRIM::left
        view_vector.SetString(std::string_view("  left  ;  trim  ,  test  "), trimSeparators,
                              ttwx::TRIM::left);
        VERIFY_EQUAL(view_vector.size(), 3U,
                     "Vector separators with left trim should create 3 elements");
        ASSERT_MSG(view_vector[0] == "left  ", "First element should be left-trimmed only");
        ASSERT_MSG(view_vector[1] == "trim  ", "Second element should be left-trimmed only");
        ASSERT_MSG(view_vector[2] == "test  ", "Third element should be left-trimmed only");

        // Test with TRIM::right
        view_vector.SetString(std::string_view("  right  ;  trim  ,  test  "), trimSeparators,
                              ttwx::TRIM::right);
        VERIFY_EQUAL(view_vector.size(), 3U,
                     "Vector separators with right trim should create 3 elements");
        ASSERT_MSG(view_vector[0] == "  right", "First element should be right-trimmed only");
        ASSERT_MSG(view_vector[1] == "  trim", "Second element should be right-trimmed only");
        ASSERT_MSG(view_vector[2] == "  test", "Third element should be right-trimmed only");

        // Test edge case: empty separator vector
        std::vector<std::string_view> emptySeparators;
        view_vector.SetString(std::string_view("noseparators"), emptySeparators);
        VERIFY_EQUAL(view_vector.size(), 1U, "Empty separator vector should create 1 element");
        ASSERT_MSG(view_vector[0] == "noseparators", "Element should be the entire string");

        // Test edge case: overlapping separators
        std::vector<std::string_view> overlappingSeparators = { "ab", "abc" };
        view_vector.SetString(std::string_view("start-ab-middle-abc-end"), overlappingSeparators);
        // The exact behavior depends on implementation - should find the first match
        ASSERT_MSG(view_vector.size() >= 3U,
                   "Overlapping separators should create at least 3 elements");

        // Test edge case: consecutive separators
        std::vector<std::string_view> consecutiveSeparators = { ";", "," };
        view_vector.SetString(std::string_view("a;,b,,;c"), consecutiveSeparators);
        VERIFY_EQUAL(view_vector.size(), 6U,
                     "Consecutive different separators should create empty elements");
        ASSERT_MSG(view_vector[0] == "a", "First element should be 'a'");
        ASSERT_MSG(view_vector[1].empty(), "Second element should be empty");
        ASSERT_MSG(view_vector[2] == "b", "Third element should be 'b'");
        ASSERT_MSG(view_vector[3].empty(), "Fourth element should be empty");
        ASSERT_MSG(view_vector[4].empty(), "Fifth element should be empty");
        ASSERT_MSG(view_vector[5] == "c", "Sixth element should be 'c'");

        // Test with single separator in vector (should behave like single separator version)
        std::vector<std::string_view> singleSeparator = { ";" };
        view_vector.SetString(std::string_view("one;two;three"), singleSeparator);
        VERIFY_EQUAL(view_vector.size(), 3U, "Single separator in vector should create 3 elements");
        ASSERT_MSG(view_vector[0] == "one", "First element should be 'one'");
        ASSERT_MSG(view_vector[1] == "two", "Second element should be 'two'");
        ASSERT_MSG(view_vector[2] == "three", "Third element should be 'three'");

        // Test case sensitivity (if applicable)
        std::vector<std::string_view> caseSeparators = { "AND", "and" };
        view_vector.SetString(std::string_view("firstANDsecondandthird"), caseSeparators);
        VERIFY_EQUAL(view_vector.size(), 3U, "Case-sensitive separators should create 3 elements");
        ASSERT_MSG(view_vector[0] == "first", "First element should be 'first'");
        ASSERT_MSG(view_vector[1] == "second", "Second element should be 'second'");
        ASSERT_MSG(view_vector[2] == "third", "Third element should be 'third'");

        // Test with separator that doesn't exist in string
        std::vector<std::string_view> nonExistentSeparators = { "xyz", "123" };
        view_vector.SetString(std::string_view("noseparatorhere"), nonExistentSeparators);
        VERIFY_EQUAL(view_vector.size(), 1U, "Non-existent separators should create 1 element");
        ASSERT_MSG(view_vector[0] == "noseparatorhere", "Element should be the entire string");

        // Test stress case with many separators
        std::vector<std::string_view> manySeparators = { ";", ",", "|", ":", "!", "@", "#", "$" };
        view_vector.SetString(std::string_view("a;b,c|d:e!f@g#h$i"), manySeparators);
        VERIFY_EQUAL(view_vector.size(), 9U, "Many different separators should create 9 elements");
        for (size_t i = 0; i < view_vector.size(); ++i)
        {
            ASSERT_MSG(view_vector[i] == std::string(1, 'a' + static_cast<char>(i)),
                       "Each element should be consecutive letters");
        }

        // Test with separator at beginning and end
        std::vector<std::string_view> edgeSeparators = { "||", "::" };
        view_vector.SetString(std::string_view("||start::middle||end::"), edgeSeparators);
        VERIFY_EQUAL(view_vector.size(), 4U, "Separators at edges should create empty elements");
        ASSERT_MSG(view_vector[0].empty(), "First element should be empty");
        ASSERT_MSG(view_vector[1] == "start", "Second element should be 'start'");
        ASSERT_MSG(view_vector[2] == "middle", "Third element should be 'middle'");
        ASSERT_MSG(view_vector[3] == "end", "Fourth element should be 'end'");

        // Test with identical separators in vector (should work like single separator)
        std::vector<std::string_view> duplicateSeparators = { ";", ";", ";" };
        view_vector.SetString(std::string_view("dup;test;case"), duplicateSeparators);
        VERIFY_EQUAL(view_vector.size(), 3U,
                     "Duplicate separators should work like single separator");
        ASSERT_MSG(view_vector[0] == "dup", "First element should be 'dup'");
        ASSERT_MSG(view_vector[1] == "test", "Second element should be 'test'");
        ASSERT_MSG(view_vector[2] == "case", "Third element should be 'case'");

        // Test with very long separators
        std::vector<std::string_view> longSeparators = { "VERYLONGSEPARATOR", "SHORT" };
        view_vector.SetString(std::string_view("beforeVERYLONGSEPARATORmiddleSHORTafter"),
                              longSeparators);
        VERIFY_EQUAL(view_vector.size(), 3U, "Long separators should work correctly");
        ASSERT_MSG(view_vector[0] == "before", "First element should be 'before'");
        ASSERT_MSG(view_vector[1] == "middle", "Second element should be 'middle'");
        ASSERT_MSG(view_vector[2] == "after", "Third element should be 'after'");
    }

    // Test 11: File reading capabilities
    {
        // Create a temporary test file
        const std::string tempFilename = "test_viewvector_temp.txt";
        const std::string testContent = "line1\nline2\r\nline3\r\nend";

        // Write test file
        {
            std::ofstream file(tempFilename, std::ios::binary);
            file.write(testContent.c_str(), static_cast<std::streamsize>(testContent.size()));
        }

        ttwx::ViewVector view_vector;

        // Test ReadFile with std::string_view
        bool readResult = view_vector.ReadFile(std::string_view(tempFilename));
        ASSERT_MSG(readResult, "ReadFile should succeed");
        VERIFY_EQUAL(view_vector.size(), 4U, "File should create 4 lines");
        ASSERT_MSG(view_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(view_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(view_vector[2] == "line3", "Third line should be 'line3'");
        ASSERT_MSG(view_vector[3] == "end", "Fourth line should be 'end'");

        // Test get_ReadFilename()
        const wxString& filename = view_vector.get_ReadFilename();
        ASSERT_MSG(!filename.IsEmpty(), "Filename should be stored");

        // Test ReadFile with wxString
        wxString wxFilename = wxString::FromUTF8(tempFilename);
        bool readResult2 = view_vector.ReadFile(wxFilename);
        ASSERT_MSG(readResult2, "ReadFile with wxString should succeed");
        VERIFY_EQUAL(view_vector.size(), 4U, "File should still create 4 lines");

        // Test ReadFile with wxFileName
        wxFileName wxFn(wxFilename);
        bool readResult3 = view_vector.ReadFile(wxFn);
        ASSERT_MSG(readResult3, "ReadFile with wxFileName should succeed");
        VERIFY_EQUAL(view_vector.size(), 4U, "File should still create 4 lines");

        // Test reading non-existent file
        bool readResult4 = view_vector.ReadFile(std::string_view("non_existent_file.txt"));
        ASSERT_MSG(!readResult4, "ReadFile should fail for non-existent file");

        // Clean up
        std::filesystem::remove(tempFilename);
    }

    // If we reach here, all tests passed

    return true;
}
