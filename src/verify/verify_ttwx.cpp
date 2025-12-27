/////////////////////////////////////////////////////////////////////////////
// Purpose:   Verify ttwx namespace functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "mainframe.h"  // CMainFrame

#include "ttwx.h"  // ttwx namespace functions

auto VerifyTTwx() -> bool  // NOLINT (cppcheck-suppress)
{
    // Test character classification functions
    {
        // Test is_alnum
        ASSERT(wxue::is_alnum('a') == true);
        ASSERT(wxue::is_alnum('Z') == true);
        ASSERT(wxue::is_alnum('5') == true);
        ASSERT(wxue::is_alnum('@') == false);
        ASSERT(wxue::is_alnum(' ') == false);

        // Test is_alpha
        ASSERT(wxue::is_alpha('a') == true);
        ASSERT(wxue::is_alpha('Z') == true);
        ASSERT(wxue::is_alpha('5') == false);
        ASSERT(wxue::is_alpha('@') == false);

        // Test is_digit
        ASSERT(wxue::is_digit('0') == true);
        ASSERT(wxue::is_digit('9') == true);
        ASSERT(wxue::is_digit('a') == false);
        ASSERT(wxue::is_digit(' ') == false);

        // Test is_lower
        ASSERT(wxue::is_lower('a') == true);
        ASSERT(wxue::is_lower('z') == true);
        ASSERT(wxue::is_lower('A') == false);
        ASSERT(wxue::is_lower('5') == false);

        // Test is_upper
        ASSERT(wxue::is_upper('A') == true);
        ASSERT(wxue::is_upper('Z') == true);
        ASSERT(wxue::is_upper('a') == false);
        ASSERT(wxue::is_upper('5') == false);

        // Test is_whitespace
        ASSERT(wxue::is_whitespace(' ') == true);
        ASSERT(wxue::is_whitespace('\t') == true);
        ASSERT(wxue::is_whitespace('\n') == true);
        ASSERT(wxue::is_whitespace('a') == false);

        // Test is_punctuation
        ASSERT(wxue::is_punctuation('.') == true);
        ASSERT(wxue::is_punctuation(',') == true);
        ASSERT(wxue::is_punctuation('!') == true);
        ASSERT(wxue::is_punctuation('a') == false);
        ASSERT(wxue::is_punctuation('5') == false);

        // Test is_blank
        ASSERT(wxue::is_blank(' ') == true);
        ASSERT(wxue::is_blank('\t') == true);
        ASSERT(wxue::is_blank('\n') == false);  // newline is not blank

        // Test is_graph
        ASSERT(wxue::is_graph('a') == true);
        ASSERT(wxue::is_graph('!') == true);
        ASSERT(wxue::is_graph(' ') == false);
        ASSERT(wxue::is_graph('\t') == false);

        // Test is_print
        ASSERT(wxue::is_print('a') == true);
        ASSERT(wxue::is_print(' ') == true);
        ASSERT(wxue::is_print('\t') == false);
        ASSERT(wxue::is_print('\n') == false);

        // Test is_cntrl
        ASSERT(wxue::is_cntrl('\t') == true);
        ASSERT(wxue::is_cntrl('\n') == true);
        ASSERT(wxue::is_cntrl('a') == false);
        ASSERT(wxue::is_cntrl(' ') == false);
    }

    // Test is_found template function
    {
        ASSERT(wxue::is_found(0) == true);
        ASSERT(wxue::is_found(5) == true);
        ASSERT(wxue::is_found(-1) == false);
        ASSERT(wxue::is_found(std::string::npos) == false);
        ASSERT(wxue::is_found(size_t(-1)) == false);
    }

    // Test find_nonspace function
    {
        std::string_view test1 = "   hello";
        auto result1 = wxue::find_nonspace(test1);
        ASSERT(result1 == "hello");

        std::string_view test2 = "hello";
        auto result2 = wxue::find_nonspace(test2);
        ASSERT(result2 == "hello");

        std::string_view test3 = "   ";
        auto result3 = wxue::find_nonspace(test3);
        ASSERT(result3.empty());

        std::string_view test4 = "";
        auto result4 = wxue::find_nonspace(test4);
        ASSERT(result4.empty());

        // Test wxString version
        wxString wxtest = "  \t world";
        auto wxresult = wxue::find_nonspace(wxtest);
        ASSERT(wxresult == "world");
    }

    // Test string utility functions
    {
        // Test back_slashesto_forward
        wxString test1 = "C:\\path\\to\\file.txt";
        wxue::back_slashesto_forward(test1);
        ASSERT(test1 == "C:/path/to/file.txt");

        // Test forward_slashesto_back
        wxString test2 = "C:/path/to/file.txt";
        wxue::forward_slashesto_back(test2);
        ASSERT(test2 == "C:\\path\\to\\file.txt");

        // Test add_trailing_slash
        wxString test3 = "C:/folder";
        wxue::add_trailing_slash(test3);
        ASSERT(test3 == "C:/folder/");

        wxString test4 = "C:/folder/";
        wxue::add_trailing_slash(test4);
        ASSERT(test4 == "C:/folder/");  // Should not add another slash

        wxString test5 = "C:/folder";
        wxue::add_trailing_slash(test5, true);  // always = true
        ASSERT(test5 == "C:/folder/");

        wxString test6 = "C:/folder/";
        wxue::add_trailing_slash(test6, true);  // always = true
        ASSERT(test6 == "C:/folder//");         // Should add another slash
    }

    // Test extract_substring and create_substring functions
    {
        // Test with double quotes
        std::string_view src1 = "  \"hello world\" ";
        wxString dest1;
        auto pos1 = wxue::extract_substring(src1, dest1, 0);
        ASSERT(dest1 == "hello world");
        ASSERT(pos1 == 14);  // Position of closing quote

        // Test with single quotes
        std::string_view src2 = "'test string'";
        wxString dest2;
        (void) wxue::extract_substring(src2, dest2, 0);  // Suppress unused variable warning
        ASSERT(dest2 == "test string");

        // Test with angle brackets
        std::string_view src3 = "<filename.h>";
        wxString dest3;
        (void) wxue::extract_substring(src3, dest3, 0);
        ASSERT(dest3 == "filename.h");

        // Test with square brackets
        std::string_view src4 = "[array_index]";
        wxString dest4;
        (void) wxue::extract_substring(src4, dest4, 0);
        ASSERT(dest4 == "array_index");

        // Test with parentheses
        std::string_view src5 = "(function_call)";
        wxString dest5;
        (void) wxue::extract_substring(src5, dest5, 0);
        ASSERT(dest5 == "function_call");

        // Test escaped quotes
        std::string_view src6 = "\"escaped \\\"quote\\\" here\"";
        wxString dest6;
        (void) wxue::extract_substring(src6, dest6, 0);
        ASSERT(dest6 == "escaped \\\"quote\\\" here");

        // Test create_substring convenience function
        auto result7 = wxue::create_substring("  'convenience test' ", 0);
        ASSERT(result7 == "convenience test");

        // Test empty string
        std::string_view src8 = "";
        wxString dest8;
        auto pos8 = wxue::extract_substring(src8, dest8, 0);
        ASSERT(dest8.IsEmpty());
        ASSERT(pos8 == std::string::npos);

        // Test no valid delimiters
        std::string_view src9 = "no delimiters here";
        wxString dest9;
        auto pos9 = wxue::extract_substring(src9, dest9, 0);
        ASSERT(pos9 == std::string::npos);
    }

    // Test file/path functions
    {
        // Test find_extension
        auto ext1 = wxue::find_extension(std::string_view("file.txt"));
        ASSERT(ext1 == ".txt");

        auto ext2 = wxue::find_extension(std::string_view("file.name.cpp"));
        ASSERT(ext2 == ".cpp");

        auto ext3 = wxue::find_extension(std::string_view("no_extension"));
        ASSERT(ext3.empty());

        auto ext4 = wxue::find_extension(std::string_view("folder."));
        ASSERT(ext4.empty());  // Just a dot is treated as folder

        auto ext5 = wxue::find_extension(std::string_view(".hidden"));
        ASSERT(ext5.empty());  // Hidden file starting with dot

        // Test wxString version
        wxString wxfile = "test.h";
        auto wxext = wxue::find_extension(wxfile);
        ASSERT(wxext == ".h");

        // Test find_filename
        auto fname1 = wxue::find_filename(std::string_view("/path/to/file.txt"));
        ASSERT(fname1 == "file.txt");

        auto fname2 = wxue::find_filename(std::string_view("C:\\Windows\\file.exe"));
        ASSERT(fname2 == "file.exe");

        auto fname3 = wxue::find_filename(std::string_view("file.txt"));
        ASSERT(fname3 == "file.txt");  // No path

        auto fname4 = wxue::find_filename(std::string_view("C:/folder/"));
        ASSERT(fname4.empty());  // Ends with slash

        auto fname5 = wxue::find_filename(std::string_view(""));
        ASSERT(fname5.empty());

        auto fname6 = wxue::find_filename(std::string_view("C:file.txt"));
        ASSERT(fname6 == "file.txt");  // Drive letter

        // Test wxString version
        wxString wxpath = "/usr/bin/test";
        auto wxfname = wxue::find_filename(wxpath);
        ASSERT(wxfname == "test");

        // Test replace_extension
        wxString rext1 = "file.txt";
        wxue::replace_extension(rext1, ".cpp");
        ASSERT(rext1 == "file.cpp");

        wxue::replace_extension(rext1, "");  // Remove extension
        ASSERT(rext1 == "file");

        wxString rext2 = "file.old.txt";
        wxue::replace_extension(rext2, "h");  // Without dot
        ASSERT(rext2 == "file.old.h");

        wxString rext3 = "no_extension";
        wxue::replace_extension(rext3, ".txt");
        ASSERT(rext3 == "no_extension.txt");

        wxue::replace_extension(rext3, "");  // Remove extension
        ASSERT(rext3 == "no_extension");     // Should remain unchanged
        wxue::replace_extension(rext3, "");  // Remove non-existent extension
        ASSERT(rext3 == "no_extension");     // Should remain unchanged

        // Test append_filename
        wxString path1 = "/usr/bin";
        auto& result1 = wxue::append_filename(path1, "test");
        ASSERT(result1 == "/usr/bin/test");
        ASSERT(&result1 == &path1);  // Should return reference to same object

        wxString path2 = "/usr/bin/";
        wxue::append_filename(path2, "test.exe");
        ASSERT(path2 == "/usr/bin/test.exe");

        wxString path3 = "";
        wxue::append_filename(path3, "file.txt");
        ASSERT(path3 == "file.txt");

        wxString path4 = "C:\\Windows";
        wxue::append_filename(path4, "system32");
        ASSERT(path4 == "C:\\Windows/system32");  // Uses forward slash

        // Test empty filename
        wxString path5 = "/path";
        wxue::append_filename(path5, "");
        ASSERT(path5 == "/path");  // Should not change

        // Test append_folder_name
        wxString folder1 = "/usr";
        auto& result2 = wxue::append_folder_name(folder1, "bin");
        ASSERT(result2 == "/usr/bin/");
        ASSERT(&result2 == &folder1);  // Should return reference to same object

        wxString folder2 = "";
        wxue::append_folder_name(folder2, "root");
        ASSERT(folder2 == "root/");

        wxString folder3 = "C:\\";
        wxue::append_folder_name(folder3, "Windows");
        ASSERT(folder3 == "C:\\Windows/");

        wxString folder4 = "file::///C:/path";
        wxue::append_folder_name(folder4, "Windows");
        ASSERT(folder4 == "file::///C:/path/Windows/");
    }

    // Test itoa template function
    {
        ASSERT(wxue::itoa(123) == "123");
        ASSERT(wxue::itoa(-456) == "-456");
        ASSERT(wxue::itoa(0) == "0");
        ASSERT(wxue::itoa(3.14f) == "3.140000");    // float
        ASSERT(wxue::itoa(2.71828) == "2.718280");  // double

        // Test with different integer types
        ASSERT(wxue::itoa(static_cast<short>(42)) == "42");
        ASSERT(wxue::itoa(static_cast<long>(1234567)) == "1234567");
        ASSERT(wxue::itoa(static_cast<unsigned int>(999)) == "999");
    }

    // Test atoi function
    {
        // Decimal numbers
        ASSERT(wxue::atoi("123") == 123);
        ASSERT(wxue::atoi("0") == 0);
        ASSERT(wxue::atoi("-456") == -456);
        ASSERT(wxue::atoi("+789") == 789);

        // Hexadecimal numbers
        ASSERT(wxue::atoi("0x1A") == 26);
        ASSERT(wxue::atoi("0x1a") == 26);
        ASSERT(wxue::atoi("0XFF") == 255);
        ASSERT(wxue::atoi("-0x10") == -16);
        ASSERT(wxue::atoi("+0X20") == 32);

        // Leading/trailing whitespace
        ASSERT(wxue::atoi("   42") == 42);
        ASSERT(wxue::atoi("\t-99 ") == -99);

        // Invalid input
        ASSERT(wxue::atoi("") == 0);
        ASSERT(wxue::atoi("abc") == 0);
        ASSERT(wxue::atoi("0x") == 0);
    }

    return true;
}
