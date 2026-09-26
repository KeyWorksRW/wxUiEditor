/////////////////////////////////////////////////////////////////////////////
// Purpose:   Unit tests for the text-diff engine (src/tools/compare/diff.cpp)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include "tools/compare/diff.h"

// Both sides of a DiffResult carry the same DiffType sequence, so counting one side is enough.
static auto CountLines(const DiffResult& result, DiffType type) -> size_t
{
    size_t count = 0;
    for (const DiffLine& line: result.left_lines)
    {
        if (line.type == type)
        {
            ++count;
        }
    }
    return count;
}

TEST_CASE("Diff::Compare reports no differences for identical text")
{
    const DiffResult result = Diff::Compare("alpha\nbravo\ncharlie", "alpha\nbravo\ncharlie");

    CHECK_FALSE(result.has_differences);
    CHECK(result.left_lines.size() == 3U);
    CHECK(result.right_lines.size() == 3U);
    CHECK(CountLines(result, DiffType::added) == 0U);
    CHECK(CountLines(result, DiffType::deleted) == 0U);
}

TEST_CASE("Diff::Compare treats a trailing newline as no change")
{
    const DiffResult result = Diff::Compare("alpha\nbravo\n", "alpha\nbravo");

    CHECK_FALSE(result.has_differences);
    CHECK(result.left_lines.size() == 2U);
}

TEST_CASE("Diff::Compare marks an inserted line as added")
{
    const DiffResult result = Diff::Compare("alpha\nbravo", "alpha\ninserted\nbravo");

    CHECK(result.has_differences);
    CHECK(CountLines(result, DiffType::added) == 1U);
    CHECK(CountLines(result, DiffType::deleted) == 0U);
    CHECK(result.left_lines.size() == result.right_lines.size());
}

TEST_CASE("Diff::Compare marks a removed line as deleted")
{
    const DiffResult result = Diff::Compare("alpha\nremoved\nbravo", "alpha\nbravo");

    CHECK(result.has_differences);
    CHECK(CountLines(result, DiffType::deleted) == 1U);
    CHECK(CountLines(result, DiffType::added) == 0U);
    CHECK(result.left_lines.size() == result.right_lines.size());
}

TEST_CASE("Diff::Compare reports nothing for two empty strings")
{
    const DiffResult result = Diff::Compare("", "");

    CHECK_FALSE(result.has_differences);
    CHECK(result.left_lines.empty());
    CHECK(result.right_lines.empty());
}

TEST_CASE("Diff::Compare keeps only the requested context around a change")
{
    const std::string_view original = "one\ntwo\nthree\nfour\nfive\nsix\nseven";
    const std::string_view modified = "one\ntwo\nthree\nCHANGED\nfive\nsix\nseven";

    const DiffResult result = Diff::Compare(original, modified, 1);

    CHECK(result.has_differences);
    // A changed line is reported as one deletion plus one insertion.
    CHECK(CountLines(result, DiffType::deleted) == 1U);
    CHECK(CountLines(result, DiffType::added) == 1U);
    // With one line of context the result is "three", the deleted line, the added line and "five".
    CHECK(result.left_lines.size() == 4U);
    CHECK(result.left_lines.front().text == "three");
    CHECK(result.left_lines.back().text == "five");
}
