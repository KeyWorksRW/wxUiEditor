/////////////////////////////////////////////////////////////////////////////
// Purpose:   Unit tests for the wxue:: string helpers (src/wxue_namespace/)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include "wxue.h"
#include "wxue_string.h"

TEST_CASE("wxue::string::MakeLower and MakeUpper convert in place")
{
    wxue::string text("MiXeD");

    CHECK(text.MakeLower() == "mixed");
    CHECK(text.MakeUpper() == "MIXED");
}

TEST_CASE("wxue::string::trim removes whitespace from the requested side")
{
    wxue::string right_only("  alpha  ");
    CHECK(right_only.trim() == "  alpha");

    wxue::string both_sides("  alpha  ");
    CHECK(both_sides.trim(wxue::TRIM::both) == "alpha");
}

TEST_CASE("wxue::string::BothTrim removes whitespace from both sides")
{
    wxue::string text("  alpha  ");

    text.BothTrim();
    CHECK(text == "alpha");
}

TEST_CASE("wxue::string::Replace reports how many occurrences it replaced")
{
    wxue::string text("alpha-beta-alpha");

    CHECK(text.Replace("alpha", "gamma", wxue::REPLACE::all) == 2U);
    CHECK(text == "gamma-beta-gamma");
}

TEST_CASE("wxue::string::subview returns a view into the string")
{
    const wxue::string path("alpha/bravo");

    CHECK(path.subview(6) == "bravo");
    CHECK(path.subview(0, 5) == "alpha");
}

TEST_CASE("wxue::atoi handles sign, whitespace, hex and garbage")
{
    CHECK(wxue::atoi("42") == 42);
    CHECK(wxue::atoi("  12") == 12);
    CHECK(wxue::atoi("-7") == -7);
    CHECK(wxue::atoi("+7") == 7);
    CHECK(wxue::atoi("0x1A") == 26);
    CHECK(wxue::atoi("nonsense") == 0);
    CHECK(wxue::atoi("") == 0);
}

TEST_CASE("wxue::has_alpha detects alphabetic content")
{
    CHECK(wxue::has_alpha("abc123"));
    CHECK_FALSE(wxue::has_alpha("12345"));
    CHECK_FALSE(wxue::has_alpha(""));
}

TEST_CASE("wxue::string_view compares case-insensitively on request")
{
    const wxue::string_view view("Hello");

    CHECK(view.comparei("hello") == 0);
    CHECK(view.comparei("help") < 0);
    CHECK(view.is_sameas("hello", wxue::CASE::either));
    CHECK_FALSE(view.is_sameas("hello"));
    CHECK(view.is_sameprefix("Hel"));  // spellchecker:disable-line
    CHECK_FALSE(view.is_sameprefix("Hello there"));
}

TEST_CASE("wxue::string_view::locate honors the requested case handling")
{
    const wxue::string_view haystack("alpha bravo charlie");

    CHECK(haystack.locate("bravo") == 6U);
    CHECK(haystack.locate("BRAVO", 0, wxue::CASE::either) == 6U);
    CHECK(haystack.locate("delta") == wxue::npos);
}

TEST_CASE("wxue::string_view extension and filename split a path")
{
    const wxue::string_view path("path/to/file.txt");

    CHECK(path.extension() == ".txt");
    CHECK(path.filename() == "file.txt");
    CHECK(wxue::string_view("no_extension").extension().empty());
}

TEST_CASE("whitespace helpers locate the next word")
{
    const wxue::string_view view("alpha bravo");

    CHECK(view.find_space(0) == 5U);
    CHECK(view.find_nonspace(5) == 6U);
    CHECK(view.stepover(0) == 6U);

    // The free functions overload on both std::string_view and wxString, so a string literal
    // would be ambiguous -- pass an explicit std::string_view.
    // Note: the free wxue::find_space(std::string_view) is only declared, never defined
    // (src/wxue_namespace/wxue.h:103), so it cannot be called or tested here.
    const std::string_view text("alpha bravo");
    CHECK(wxue::find_nonspace(std::string_view("   alpha")) == "alpha");
    CHECK(wxue::stepover(text) == "bravo");
}

TEST_CASE("wxue::string_view::moveto_nonspace advances past leading whitespace")
{
    wxue::string_view view("   alpha bravo");

    CHECK(view.moveto_nonspace());
    CHECK(view == "alpha bravo");
}
