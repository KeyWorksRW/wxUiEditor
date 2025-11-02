/////////////////////////////////////////////////////////////////////////////
// Purpose:   Verify ttwx::StringVector class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ttwx_string_vector.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

#include <wx/filename.h>

#include "verify.h"

#include "assertion_dlg.h"  // Assertion Dialog

// Function to verify all capabilities of ttwx::StringVector

// NOLINTNEXTLINE (clang-analyzer-core.cognitive-complexity) // cppcheck-suppress
// highCognitiveComplexity
auto VerifyStringVector() -> bool  // NOLINT(clang-analyzer-core.cognitive-complexity)
{
    // Test 1: String parsing with character separator
    {
        ttwx::StringVector string_vector;
        string_vector.SetString(std::string_view("apple;banana;cherry"), ";");

        VERIFY_EQUAL(string_vector.size(), 3U, "Basic semicolon parsing should create 3 elements");
        ASSERT_MSG(string_vector[0] == "apple", "First element should be 'apple'");
        ASSERT_MSG(string_vector[1] == "banana", "Second element should be 'banana'");
        ASSERT_MSG(string_vector[2] == "cherry", "Third element should be 'cherry'");

        // Test with different separator
        string_vector.SetString(std::string_view("one,two,three"), ',');
        VERIFY_EQUAL(string_vector.size(), 3U, "Comma parsing should create 3 elements");
        ASSERT_MSG(string_vector[0] == "one", "First element should be 'one'");
        ASSERT_MSG(string_vector[1] == "two", "Second element should be 'two'");
        ASSERT_MSG(string_vector[2] == "three", "Third element should be 'three'");
    }

    // Test 2: String parsing with string separator
    {
        ttwx::StringVector string_vector;
        string_vector.SetString(std::string_view("first||second||third"), std::string_view("||"));

        VERIFY_EQUAL(string_vector.size(), 3U, "Double pipe parsing should create 3 elements");
        ASSERT_MSG(string_vector[0] == "first", "First element should be 'first'");
        ASSERT_MSG(string_vector[1] == "second", "Second element should be 'second'");
        ASSERT_MSG(string_vector[2] == "third", "Third element should be 'third'");

        // Test with newline separator
        string_vector.SetString(std::string_view("line1\r\nline2\r\nline3"),
                                std::string_view("\r\n"));
        VERIFY_EQUAL(string_vector.size(), 3U, "CRLF parsing should create 3 elements");
        ASSERT_MSG(string_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(string_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(string_vector[2] == "line3", "Third line should be 'line3'");
    }

    // Test 3: TRIM functionality
    {
        ttwx::StringVector string_vector;

        // Test TRIM::none
        string_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                                ttwx::TRIM::none);
        VERIFY_EQUAL(string_vector.size(), 3U, "No trim should create 3 elements");
        ASSERT_MSG(string_vector[0] == "  apple  ", "First element should preserve spaces");
        ASSERT_MSG(string_vector[1] == " banana ", "Second element should preserve spaces");
        ASSERT_MSG(string_vector[2] == "  cherry  ", "Third element should preserve spaces");

        // Test TRIM::both
        string_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                                ttwx::TRIM::both);
        VERIFY_EQUAL(string_vector.size(), 3U, "Trim both should create 3 elements");
        ASSERT_MSG(string_vector[0] == "apple", "First element should be trimmed");
        ASSERT_MSG(string_vector[1] == "banana", "Second element should be trimmed");
        ASSERT_MSG(string_vector[2] == "cherry", "Third element should be trimmed");

        // Test TRIM::left
        string_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                                ttwx::TRIM::left);
        VERIFY_EQUAL(string_vector.size(), 3U, "Trim left should create 3 elements");
        ASSERT_MSG(string_vector[0] == "apple  ", "First element should be left-trimmed");
        ASSERT_MSG(string_vector[1] == "banana ", "Second element should be left-trimmed");
        ASSERT_MSG(string_vector[2] == "cherry  ", "Third element should be left-trimmed");

        // Test TRIM::right
        string_vector.SetString(std::string_view("  apple  ; banana ;  cherry  "), ';',
                                ttwx::TRIM::right);
        VERIFY_EQUAL(string_vector.size(), 3U, "Trim right should create 3 elements");
        ASSERT_MSG(string_vector[0] == "  apple", "First element should be right-trimmed");
        ASSERT_MSG(string_vector[1] == " banana", "Second element should be right-trimmed");
        ASSERT_MSG(string_vector[2] == "  cherry", "Third element should be right-trimmed");
    }

    // Test 4: Edge cases for string parsing
    {
        ttwx::StringVector string_vector;

        // Empty string
        string_vector.SetString(std::string_view(""), ";");
        VERIFY_EQUAL(string_vector.size(), 0U, "Empty string should create no elements");

        // String with only separators
        string_vector.SetString(std::string_view(";;;"), ";");
        VERIFY_EQUAL(string_vector.size(), 3U,
                     "String with only separators should create empty elements");
        for (auto& idx: string_vector)
        {
            ASSERT_MSG(idx.empty(), "All elements should be empty strings");
        }

        // Single element (no separator)
        string_vector.SetString(std::string_view("single"), ";");
        VERIFY_EQUAL(string_vector.size(), 1U, "Single element should create 1 element");
        ASSERT_MSG(string_vector[0] == "single", "Single element should be 'single'");

        // Leading and trailing separators
        string_vector.SetString(std::string_view(";first;last;"), ";");
        VERIFY_EQUAL(string_vector.size(), 3U,
                     "Leading/trailing separators should create 4 elements");
        ASSERT_MSG(string_vector[0].empty(), "First element should be empty");
        ASSERT_MSG(string_vector[1] == "first", "Second element should be 'first'");
        ASSERT_MSG(string_vector[2] == "last", "Third element should be 'last'");
    }

    // Test 5: Constructor variants
    {
        // Default constructor
        ttwx::StringVector sv1;
        VERIFY_EQUAL(sv1.size(), 0U, "Default constructor should create empty vector");

        // Parameterized constructor with character separator
        ttwx::StringVector sv2(std::string_view("a;b;c"), ";");
        VERIFY_EQUAL(sv2.size(), 3U, "Constructor with char separator should create 3 elements");
        ASSERT_MSG(sv2[0] == "a", "First element should be 'a'");
        ASSERT_MSG(sv2[1] == "b", "Second element should be 'b'");
        ASSERT_MSG(sv2[2] == "c", "Third element should be 'c'");

        // Parameterized constructor with TRIM
        ttwx::StringVector sv3(std::string_view(" x ; y ; z "), ';', ttwx::TRIM::both);
        VERIFY_EQUAL(sv3.size(), 3U, "Constructor with trim should create 3 elements");
        ASSERT_MSG(sv3[0] == "x", "First element should be trimmed to 'x'");
        ASSERT_MSG(sv3[1] == "y", "Second element should be trimmed to 'y'");
        ASSERT_MSG(sv3[2] == "z", "Third element should be trimmed to 'z'");
    }

    // Test 6: ReadString capabilities
    {
        ttwx::StringVector string_vector;

        // Test with different line endings
        string_vector.ReadString(std::string_view("line1\nline2\nline3"));
        VERIFY_EQUAL(string_vector.size(), 3U, "ReadString with \n should create 3 lines");
        ASSERT_MSG(string_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(string_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(string_vector[2] == "line3", "Third line should be 'line3'");

        // Test with CRLF
        string_vector.ReadString(std::string_view("first\r\nsecond\r\nthird"));
        VERIFY_EQUAL(string_vector.size(), 3U, "ReadString with \r\n should create 3 lines");
        ASSERT_MSG(string_vector[0] == "first", "First line should be 'first'");
        ASSERT_MSG(string_vector[1] == "second", "Second line should be 'second'");
        ASSERT_MSG(string_vector[2] == "third", "Third line should be 'third'");

        // Test with CR only
        string_vector.ReadString(std::string_view("one\rtwo\rthree"));
        VERIFY_EQUAL(string_vector.size(), 3U, "ReadString with \r should create 3 lines");
        ASSERT_MSG(string_vector[0] == "one", "First line should be 'one'");
        ASSERT_MSG(string_vector[1] == "two", "Second line should be 'two'");
        ASSERT_MSG(string_vector[2] == "three", "Third line should be 'three'");

        // Test with mixed line endings
        string_vector.ReadString(std::string_view("mixed\nline\r\nendings\r"));
        VERIFY_EQUAL(string_vector.size(), 3U,
                     "ReadString with mixed endings should create 4 lines");
        ASSERT_MSG(string_vector[0] == "mixed", "First line should be 'mixed'");
        ASSERT_MSG(string_vector[1] == "line", "Second line should be 'line'");
        ASSERT_MSG(string_vector[2] == "endings", "Third line should be 'endings'");

        // Test wxString overload
        wxString wxStr = wxT("wx1\nwx2\nwx3");
        string_vector.ReadString(wxStr);
        VERIFY_EQUAL(string_vector.size(), 3U, "ReadString with wxString should create 3 lines");
        ASSERT_MSG(string_vector[0] == "wx1", "First line should be 'wx1'");
        ASSERT_MSG(string_vector[1] == "wx2", "Second line should be 'wx2'");
        ASSERT_MSG(string_vector[2] == "wx3", "Third line should be 'wx3'");
    }

    // Test 7: Comparison and utility methods
    {
        ttwx::StringVector string_vector1;
        ttwx::StringVector string_vector2;

        // Test is_sameas() with identical vectors
        string_vector1.ReadString(std::string_view("same\nlines\nhere"));
        string_vector2.ReadString(std::string_view("same\nlines\nhere"));
        ASSERT_MSG(string_vector1.is_sameas(string_vector2), "Identical vectors should be same");

        // Test is_sameas() with different content
        string_vector2.ReadString(std::string_view("different\nlines\nhere"));
        ASSERT_MSG(!string_vector1.is_sameas(string_vector2),
                   "Different content should not be same");

        // Test is_sameas() with different sizes
        string_vector2.ReadString(std::string_view("same\nlines"));
        ASSERT_MSG(!string_vector1.is_sameas(string_vector2), "Different sizes should not be same");
    }

    // Test 8: wxString integration
    {
        ttwx::StringVector string_vector;

        // Test SetString with wxString and character separator
        wxString wxStr1 = wxT("wx;string;test");
        string_vector.SetString(wxStr1, ";");
        VERIFY_EQUAL(string_vector.size(), 3U, "SetString with wxString should create 3 elements");
        ASSERT_MSG(string_vector[0] == "wx", "First element should be 'wx'");
        ASSERT_MSG(string_vector[1] == "string", "Second element should be 'string'");
        ASSERT_MSG(string_vector[2] == "test", "Third element should be 'test'");

        // Test SetString with wxString and string separator
        wxString wxStr2 = wxT("first||second||third");
        string_vector.SetString(wxStr2, std::string_view("||"));
        VERIFY_EQUAL(string_vector.size(), 3U,
                     "SetString with wxString and string separator should create 3 elements");
        ASSERT_MSG(string_vector[0] == "first", "First element should be 'first'");
        ASSERT_MSG(string_vector[1] == "second", "Second element should be 'second'");
        ASSERT_MSG(string_vector[2] == "third", "Third element should be 'third'");

        // Test SetString with wxString and TRIM
        wxString wxStr3 = wxT("  trim  ;  test  ");
        string_vector.SetString(wxStr3, ';', ttwx::TRIM::both);
        VERIFY_EQUAL(string_vector.size(), 2U,
                     "SetString with wxString and trim should create 2 elements");
        ASSERT_MSG(string_vector[0] == "trim", "First element should be trimmed to 'trim'");
        ASSERT_MSG(string_vector[1] == "test", "Second element should be trimmed to 'test'");
    }

    // Test 9: Edge cases and error handling
    {
        ttwx::StringVector string_vector;

        // Empty ReadString
        string_vector.ReadString(std::string_view(""));
        VERIFY_EQUAL(string_vector.size(), 0U, "Empty ReadString should create no elements");

        // Single line without line ending
        string_vector.ReadString(std::string_view("single"));
        VERIFY_EQUAL(string_vector.size(), 1U, "Single line should create 1 element");
        ASSERT_MSG(string_vector[0] == "single", "Single line should be 'single'");

        // Only line endings
        string_vector.ReadString(std::string_view("\n\n\n"));
        VERIFY_EQUAL(string_vector.size(), 3U, "Only line endings should create empty lines");
        for (size_t i = 0; i < string_vector.size(); ++i)
        {
            ASSERT_MSG(string_vector[i].empty(), "All lines should be empty");
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
        string_vector.SetString(std::string_view(largeString), ";");
        VERIFY_EQUAL(string_vector.size(), numElements,
                     "Large string should create correct number of elements");
        ASSERT_MSG(string_vector[0] == "item0", "First element should be 'item0'");
        ASSERT_MSG(string_vector[numElements - 1] == "item" + std::to_string(numElements - 1),
                   "Last element should be correct");
    }

    // Test 10: std::vector separators functionality
    {
        ttwx::StringVector string_vector;

        // Test with multiple single-character separators
        std::vector<std::string_view> separators = { ";", "," };
        string_vector.SetString(std::string_view("apple;banana,cherry;orange,grape"), separators);
        VERIFY_EQUAL(string_vector.size(), 5U,
                     "Multiple single-char separators should create 5 elements");
        ASSERT_MSG(string_vector[0] == "apple", "First element should be 'apple'");
        ASSERT_MSG(string_vector[1] == "banana", "Second element should be 'banana'");
        ASSERT_MSG(string_vector[2] == "cherry", "Third element should be 'cherry'");
        ASSERT_MSG(string_vector[3] == "orange", "Fourth element should be 'orange'");
        ASSERT_MSG(string_vector[4] == "grape", "Fifth element should be 'grape'");

        // Test with multiple multi-character separators
        std::vector<std::string_view> multiSeparators = { "||", "::" };
        string_vector.SetString(std::string_view("first||second::third||fourth"), multiSeparators);
        VERIFY_EQUAL(string_vector.size(), 4U,
                     "Multiple multi-char separators should create 4 elements");
        ASSERT_MSG(string_vector[0] == "first", "First element should be 'first'");
        ASSERT_MSG(string_vector[1] == "second", "Second element should be 'second'");
        ASSERT_MSG(string_vector[2] == "third", "Third element should be 'third'");
        ASSERT_MSG(string_vector[3] == "fourth", "Fourth element should be 'fourth'");

        // Test with line ending separators (common use case)
        std::vector<std::string_view> lineSeparators = { "\r\n", "\r", "\n" };
        string_vector.SetString(std::string_view("line1\r\nline2\nline3\rline4"), lineSeparators);
        VERIFY_EQUAL(string_vector.size(), 4U, "Line ending separators should create 4 lines");
        ASSERT_MSG(string_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(string_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(string_vector[2] == "line3", "Third line should be 'line3'");
        ASSERT_MSG(string_vector[3] == "line4", "Fourth line should be 'line4'");

        // Test with TRIM functionality and multiple separators
        std::vector<std::string_view> trimSeparators = { ";", "," };
        string_vector.SetString(std::string_view("  first  ;  second  ,  third  "), trimSeparators,
                                ttwx::TRIM::both);
        VERIFY_EQUAL(string_vector.size(), 3U,
                     "Vector separators with trim should create 3 elements");
        ASSERT_MSG(string_vector[0] == "first", "First element should be trimmed");
        ASSERT_MSG(string_vector[1] == "second", "Second element should be trimmed");
        ASSERT_MSG(string_vector[2] == "third", "Third element should be trimmed");
    }

    // Test 11: File reading capabilities
    {
        // Create a temporary test file
        const std::string tempFilename = "test_stringvector_temp.txt";
        const std::string testContent = "line1\nline2\r\nline3\r\nend";

        // Write test file
        {
            std::ofstream file(tempFilename, std::ios::binary);
            file.write(testContent.c_str(), static_cast<std::streamsize>(testContent.size()));
        }

        ttwx::StringVector string_vector;

        // Test ReadFile with std::string_view
        bool readResult = string_vector.ReadFile(std::string_view(tempFilename));
        ASSERT_MSG(readResult, "ReadFile should succeed");
        VERIFY_EQUAL(string_vector.size(), 4U, "File should create 4 lines");
        ASSERT_MSG(string_vector[0] == "line1", "First line should be 'line1'");
        ASSERT_MSG(string_vector[1] == "line2", "Second line should be 'line2'");
        ASSERT_MSG(string_vector[2] == "line3", "Third line should be 'line3'");
        ASSERT_MSG(string_vector[3] == "end", "Fourth line should be 'end'");

        // Test get_ReadFilename()
        const wxString& filename = string_vector.get_ReadFilename();
        ASSERT_MSG(!filename.IsEmpty(), "Filename should be stored");

        // Test ReadFile with wxString
        wxString wxFilename = wxString::FromUTF8(tempFilename);
        bool readResult2 = string_vector.ReadFile(wxFilename);
        ASSERT_MSG(readResult2, "ReadFile with wxString should succeed");
        VERIFY_EQUAL(string_vector.size(), 4U, "File should still create 4 lines");

        // Test ReadFile with wxFileName
        wxFileName wxFn(wxFilename);
        bool readResult3 = string_vector.ReadFile(wxFn);
        ASSERT_MSG(readResult3, "ReadFile with wxFileName should succeed");
        VERIFY_EQUAL(string_vector.size(), 4U, "File should still create 4 lines");

        // Test reading non-existent file
        bool readResult4 = string_vector.ReadFile(std::string_view("non_existent_file.txt"));
        ASSERT_MSG(!readResult4, "ReadFile should fail for non-existent file");

        // Clean up
        std::filesystem::remove(tempFilename);
    }

    // Test 12: Vector modification operations (unique to StringVector)
    {
        ttwx::StringVector string_vector;

        // Setup initial vector
        string_vector.SetString(std::string_view("one;two;three"), ";");
        VERIFY_EQUAL(string_vector.size(), 3U, "Initial vector should have 3 elements");

        // Test push_back
        string_vector.push_back("four");
        VERIFY_EQUAL(string_vector.size(), 4U, "After push_back should have 4 elements");
        ASSERT_MSG(string_vector[3] == "four", "Fourth element should be 'four'");

        // Test emplace_back
        string_vector.emplace_back("five");
        VERIFY_EQUAL(string_vector.size(), 5U, "After emplace_back should have 5 elements");
        ASSERT_MSG(string_vector[4] == "five", "Fifth element should be 'five'");

        // Test insert
        auto iter = string_vector.begin() + 2;
        string_vector.insert(iter, "inserted");
        VERIFY_EQUAL(string_vector.size(), 6U, "After insert should have 6 elements");
        ASSERT_MSG(string_vector[2] == "inserted", "Inserted element should be at position 2");
        ASSERT_MSG(string_vector[3] == "three", "Previous element should shift to position 3");

        // Test erase single element
        iter = string_vector.begin() + 2;
        string_vector.erase(iter);
        VERIFY_EQUAL(string_vector.size(), 5U, "After erase should have 5 elements");
        ASSERT_MSG(string_vector[2] == "three", "Element at position 2 should be 'three' again");

        // Test erase range
        auto iter_start = string_vector.begin() + 1;
        auto iter_end = string_vector.begin() + 3;
        string_vector.erase(iter_start, iter_end);
        VERIFY_EQUAL(string_vector.size(), 3U, "After range erase should have 3 elements");
        ASSERT_MSG(string_vector[0] == "one", "First element should still be 'one'");
        ASSERT_MSG(string_vector[1] == "four", "Second element should now be 'four'");
        ASSERT_MSG(string_vector[2] == "five", "Third element should now be 'five'");

        // Test pop_back
        string_vector.pop_back();
        VERIFY_EQUAL(string_vector.size(), 2U, "After pop_back should have 2 elements");

        // Test clear
        string_vector.clear();
        VERIFY_EQUAL(string_vector.size(), 0U, "After clear should have 0 elements");
        ASSERT_MSG(string_vector.empty(), "Vector should be empty after clear");

        // Test reserve and capacity
        string_vector.reserve(100);
        ASSERT_MSG(string_vector.capacity() >= 100,
                   "Capacity should be at least 100 after reserve");
        VERIFY_EQUAL(string_vector.size(), 0U, "Size should still be 0 after reserve");
    }

    // Test 13: Advanced vector modification and manipulation
    {
        ttwx::StringVector string_vector;
        string_vector.SetString(std::string_view("apple;banana;cherry;date"), ";");

        // Test element modification through direct access
        string_vector[0] = "apricot";
        ASSERT_MSG(string_vector[0] == "apricot", "First element should be modified to 'apricot'");

        // Test element modification through at()
        string_vector.at(1) = "blueberry";
        ASSERT_MSG(string_vector[1] == "blueberry",
                   "Second element should be modified to 'blueberry'");

        // Test element modification through iterator
        auto iter = string_vector.begin() + 2;
        *iter = "cranberry";
        ASSERT_MSG(string_vector[2] == "cranberry",
                   "Third element should be modified to 'cranberry'");

        // Test front() and back()
        ASSERT_MSG(string_vector.front() == "apricot", "front() should return first element");
        ASSERT_MSG(string_vector.back() == "date", "back() should return last element");

        // Modify via front() and back()
        string_vector.front() = "avocado";
        string_vector.back() = "durian";
        ASSERT_MSG(string_vector[0] == "avocado", "First element should be modified via front()");
        ASSERT_MSG(string_vector[3] == "durian", "Last element should be modified via back()");

        // Test resize
        string_vector.resize(6, "filler");
        VERIFY_EQUAL(string_vector.size(), 6U, "After resize should have 6 elements");
        ASSERT_MSG(string_vector[4] == "filler", "Fifth element should be 'filler'");
        ASSERT_MSG(string_vector[5] == "filler", "Sixth element should be 'filler'");

        // Test resize smaller
        string_vector.resize(3);
        VERIFY_EQUAL(string_vector.size(), 3U, "After resize down should have 3 elements");
    }

    // Test 14: Iterators and range-based operations
    {
        ttwx::StringVector string_vector;
        string_vector.SetString(std::string_view("one;two;three;four;five"), ";");

        // Test range-based for loop
        size_t count = 0;
        for (const auto& str: string_vector)
        {
            ASSERT_MSG(!str.empty(), "Each element should be non-empty");
            ++count;
        }
        VERIFY_EQUAL(count, 5U, "Range-based loop should iterate over 5 elements");

        // Test iterator arithmetic
        auto iter_begin = string_vector.begin();
        auto iter_end = string_vector.end();
        VERIFY_EQUAL(static_cast<size_t>(iter_end - iter_begin), 5U,
                     "Iterator distance should be 5");

        // Test reverse iterators
        auto riter = string_vector.rbegin();
        ASSERT_MSG(*riter == "five", "Reverse iterator should start at last element");
        ++riter;
        ASSERT_MSG(*riter == "four", "After increment should point to 'four'");

        // Test const iterators
        const ttwx::StringVector& const_ref = string_vector;
        auto citer = const_ref.cbegin();
        ASSERT_MSG(*citer == "one", "Const iterator should point to first element");
    }

    // Test 15: Copy and assignment operations
    {
        ttwx::StringVector sv1;
        sv1.SetString(std::string_view("original;data"), ";");

        // Test copy constructor (using std::vector's copy constructor)
        ttwx::StringVector sv2 = sv1;
        VERIFY_EQUAL(sv2.size(), 2U, "Copied vector should have 2 elements");
        ASSERT_MSG(sv2[0] == "original", "Copied vector should have same content");
        ASSERT_MSG(sv2[1] == "data", "Copied vector should have same content");

        // Modify original and verify copy is independent
        sv1[0] = "modified";
        ASSERT_MSG(sv2[0] == "original", "Copied vector should be independent");

        // Test assignment operator
        ttwx::StringVector sv3;
        sv3 = sv1;
        VERIFY_EQUAL(sv3.size(), 2U, "Assigned vector should have 2 elements");
        ASSERT_MSG(sv3[0] == "modified", "Assigned vector should have updated content");

        // Test self-assignment safety
        sv3 = sv3;
        VERIFY_EQUAL(sv3.size(), 2U, "Self-assignment should not break vector");
    }

    // Test 16: std::vector algorithm compatibility
    {
        ttwx::StringVector string_vector;
        string_vector.SetString(std::string_view("zebra;apple;mango;banana"), ";");

        // Test std::sort
        std::sort(string_vector.begin(), string_vector.end());
        ASSERT_MSG(string_vector[0] == "apple", "After sort, first should be 'apple'");
        ASSERT_MSG(string_vector[1] == "banana", "After sort, second should be 'banana'");
        ASSERT_MSG(string_vector[2] == "mango", "After sort, third should be 'mango'");
        ASSERT_MSG(string_vector[3] == "zebra", "After sort, fourth should be 'zebra'");

        // Test std::find
        auto iter = std::find(string_vector.begin(), string_vector.end(), "mango");
        ASSERT_MSG(iter != string_vector.end(), "std::find should locate 'mango'");
        ASSERT_MSG(*iter == "mango", "Found iterator should point to 'mango'");

        // Test std::find_if
        auto iter2 = std::find_if(string_vector.begin(), string_vector.end(),
                                  [](const std::string& s)
                                  {
                                      return s.length() > 5;
                                  });
        ASSERT_MSG(iter2 != string_vector.end(),
                   "std::find_if should find string longer than 5 chars");
        ASSERT_MSG(*iter2 == "banana", "Found string should be 'banana'");

        // Test std::count
        string_vector.push_back("apple");
        auto apple_count = std::count(string_vector.begin(), string_vector.end(), "apple");
        VERIFY_EQUAL(static_cast<size_t>(apple_count), 2U, "Should count 2 occurrences of 'apple'");

        // Test std::reverse
        std::ranges::reverse(string_vector);
        ASSERT_MSG(string_vector[0] == "apple", "After reverse, first should be 'apple'");
        ASSERT_MSG(string_vector.back() == "apple", "After reverse, last should be 'apple'");
    }

    // If we reach here, all tests passed

    return true;
}
