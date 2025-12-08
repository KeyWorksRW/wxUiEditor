/////////////////////////////////////////////////////////////////////////////
// Purpose:   Utility functions that work with properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdio>  // For snprintf
#include <format>
#include <set>

#include <wx/filedlg.h>  // wxFileDialog base header
#include <wx/gdicmn.h>   // Common GDI classes, types and declarations
#include <wx/mstream.h>  // Memory stream classes

#include "mainframe.h"       // MainFrame -- Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "tt_view_vector.h"  // tt_view_vector -- read/write line-oriented strings/files
#include "ttwx.h"            // ttwx namespace helpers for wxString-aware utilities
#include "utils.h"           // Utility functions that work with properties

// Look for search string in line, and if found, replace with replace_with string. If all
// is true, replace all instances, otherwise only the first instance is replaced.
auto utils::replace_in_line(std::string& line, std::string_view search,
                            std::string_view replace_with, bool all) -> void
{
    for (auto pos = line.find(search, 0); pos != std::string::npos;
         pos = line.find(search, pos + replace_with.length()))
    {
        line.replace(pos, search.length(), replace_with);
        if (!all)
        {
            break;
        }
    }
}

tt_string DoubleToStr(double val)
{
    tt_string result;

    std::array<char, 20> str {};
    if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), val); ec == std::errc())
    {
        result.assign(str.data(), ptr);
    }
    return result;
}

tt_string ClearPropFlag(tt_string_view flag, tt_string_view currentValue)
{
    tt_string result;
    if (flag.empty() || currentValue.empty())
    {
        result = currentValue;
        return result;
    }

    tt_view_vector mstr(currentValue, '|');
    for (auto& iter: mstr)
    {
        if (iter != flag)
        {
            if (result.size())
            {
                result << '|';
            }
            result << iter;
        }
    }
    return result;
}

tt_string ClearMultiplePropFlags(tt_string_view flags, tt_string_view currentValue)
{
    tt_string result;
    if (flags.empty() || currentValue.empty())
    {
        result = currentValue;
        return result;
    }

    tt_string_vector mflags(flags, '|');

    tt_string_vector mstr(currentValue, '|');
    for (auto& iter: mstr)
    {
        bool isFlagged = false;
        for (auto& itFlags: mflags)
        {
            if (iter == itFlags)
            {
                isFlagged = true;
                break;
            }
        }

        if (!isFlagged)
        {
            if (result.size())
            {
                result << '|';
            }
            result << iter;
        }
    }
    return result;
}

tt_string SetPropFlag(tt_string_view flag, tt_string_view currentValue)
{
    tt_string result(currentValue);
    if (flag.empty())
    {
        return result;
    }

    tt_view_vector mstr(currentValue, '|');
    if (std::ranges::any_of(mstr,
                            [&](const auto& iter)
                            {
                                return iter.is_sameas(flag);
                            }))
    {
        return result;  // flag has already been added
    }
    if (result.size())
    {
        result << '|';
    }
    result << flag;
    return result;
}

bool isPropFlagSet(tt_string_view flag, tt_string_view currentValue)
{
    if (flag.empty() || currentValue.empty())
    {
        return false;
    }

    tt_view_vector mstr(currentValue, '|');
    return std::ranges::any_of(mstr,
                               [&](const auto& iter)
                               {
                                   return iter.is_sameas(flag);
                               });
}

wxSystemColour ConvertToSystemColour(tt_string_view value)
{
    // clang-format off

    #define IS_SYSCOLOUR(name) if (value == #name) return name;

    IS_SYSCOLOUR(wxSYS_COLOUR_SCROLLBAR)

    IS_SYSCOLOUR(wxSYS_COLOUR_BACKGROUND)
    IS_SYSCOLOUR(wxSYS_COLOUR_ACTIVECAPTION)
    IS_SYSCOLOUR(wxSYS_COLOUR_INACTIVECAPTION)
    IS_SYSCOLOUR(wxSYS_COLOUR_MENU)
    IS_SYSCOLOUR(wxSYS_COLOUR_WINDOW)
    IS_SYSCOLOUR(wxSYS_COLOUR_WINDOWFRAME)
    IS_SYSCOLOUR(wxSYS_COLOUR_MENUTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_CAPTIONTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_ACTIVEBORDER)
    IS_SYSCOLOUR(wxSYS_COLOUR_INACTIVEBORDER)
    IS_SYSCOLOUR(wxSYS_COLOUR_APPWORKSPACE)
    IS_SYSCOLOUR(wxSYS_COLOUR_HIGHLIGHT)
    IS_SYSCOLOUR(wxSYS_COLOUR_HIGHLIGHTTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_BTNFACE)
    IS_SYSCOLOUR(wxSYS_COLOUR_BTNSHADOW)
    IS_SYSCOLOUR(wxSYS_COLOUR_GRAYTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_BTNTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_INACTIVECAPTIONTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_BTNHIGHLIGHT)
    IS_SYSCOLOUR(wxSYS_COLOUR_3DDKSHADOW)
    IS_SYSCOLOUR(wxSYS_COLOUR_3DLIGHT)
    IS_SYSCOLOUR(wxSYS_COLOUR_INFOTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_INFOBK)

    IS_SYSCOLOUR(wxSYS_COLOUR_LISTBOX)
    IS_SYSCOLOUR(wxSYS_COLOUR_HOTLIGHT)
    IS_SYSCOLOUR(wxSYS_COLOUR_GRADIENTACTIVECAPTION)
    IS_SYSCOLOUR(wxSYS_COLOUR_GRADIENTINACTIVECAPTION)
    IS_SYSCOLOUR(wxSYS_COLOUR_MENUHILIGHT)
    IS_SYSCOLOUR(wxSYS_COLOUR_MENUBAR)
    IS_SYSCOLOUR(wxSYS_COLOUR_LISTBOXTEXT)
    IS_SYSCOLOUR(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT)

    return wxSYS_COLOUR_BTNFACE;
    // clang-format on
}

auto ConvertFontFamilyToString(wxFontFamily family) -> const char*
{
    const char* result;

    switch (family)
    {
        case wxFONTFAMILY_DECORATIVE:
            result = "wxFONTFAMILY_DECORATIVE";
            break;
        case wxFONTFAMILY_ROMAN:
            result = "wxFONTFAMILY_ROMAN";
            break;
        case wxFONTFAMILY_SCRIPT:
            result = "wxFONTFAMILY_SCRIPT";
            break;
        case wxFONTFAMILY_SWISS:
            result = "wxFONTFAMILY_SWISS";
            break;
        case wxFONTFAMILY_MODERN:
            result = "wxFONTFAMILY_MODERN";
            break;
        case wxFONTFAMILY_TELETYPE:
            result = "wxFONTFAMILY_TELETYPE";
            break;
        default:
            result = "wxFONTFAMILY_DEFAULT";
            break;
    }

    return result;
}

auto ConvertEscapeSlashes(tt_string_view str) -> tt_string
{
    tt_string result;

    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        auto current_char = str[pos];
        if (current_char == '\\')
        {
            // REVIEW: [KeyWorks - 06-07-2020] Like the original wxString version, this will not
            // save a backslash if it is at the end of a line. Is that intentional or just a bug?
            if (pos < str.length() - 1)
            {
                auto next_char = str[pos + 1];

                switch (next_char)
                {
                    case 'n':
                        result += '\n';
                        ++pos;
                        break;

                    case 't':
                        result += '\t';
                        ++pos;
                        break;

                    case 'r':
                        result += '\r';
                        ++pos;
                        break;

                    case '\\':
                        result += '\\';
                        ++pos;
                        break;

                    default:
                        // REVIEW: [Randalphwa - 10-30-2025] This was just added, but not tested...

                        // If not a recognized escape, just add the backslash and the next character
                        result += '\\';
                        // Do not increment pos here, so the next character will be processed
                        // normally
                        break;
                }
            }
        }
        else
        {
            result += current_char;
        }
    }

    return result;
}

auto DlgPoint(Node* node, GenEnum::PropName prop) -> wxPoint
{
    if (!isScalingEnabled(node, prop))
    {
        return node->as_wxPoint(prop);
    }
    return wxGetMainFrame()->getWindow()->FromDIP(node->as_wxPoint(prop));
}

wxSize DlgSize(Node* node, GenEnum::PropName prop)
{
    if (!isScalingEnabled(node, prop))
    {
        return node->as_wxSize(prop);
    }
    return wxGetMainFrame()->getWindow()->FromDIP(node->as_wxSize(prop));
}

auto DlgPoint(int width) -> int
{
    wxPoint dlg_point = { width, -1 };
    wxGetMainFrame()->getWindow()->FromDIP(dlg_point);
    return dlg_point.x;
}

auto GetSizeInfo(tt_string_view description) -> wxSize
{
    wxSize size;

    tt_view_vector size_description;
    if (description.contains(";"))
    {
        size_description.SetString(description, ';', tt::TRIM::left);
    }
    else
    {
        size_description.SetString(description, ',');
    }

    ASSERT(size_description.size())
    ASSERT(size_description[0].size())

    if (size_description.size())
    {
        size_t start = size_description[0].front() == '[' ? 1 : 0;
        size.x = size_description[0].atoi(start);
        if (size_description.size() > 1)
        {
            size.y = size_description[1].atoi();
        }
        else
        {
            size.y = 0;
        }
    }
    else
    {
        size.x = 16;
        size.y = 16;
    }

    return size;
}

// Any mime type in the following list will NOT be converted to PNG.

#include <array>

// clang-format off
inline constexpr auto lst_no_png_conversion = std::to_array<const char*>({
    "image/x-ani",
    "image/x-cur",
    "image/gif",
    "image/x-ico",
    "image/jpeg"
});
// clang-format on

bool isConvertibleMime(const tt_string& suffix)
{
    return std::ranges::all_of(lst_no_png_conversion,
                               [&](const char* iter)
                               {
                                   return !suffix.is_sameas(iter);
                               });
}

extern const char* g_u8_cpp_keywords;  // defined in ../panels/base_panel.cpp
extern const char* g_python_keywords;
extern const char* g_ruby_keywords;
extern const char* g_perl_keywords;

std::set<std::string> g_set_cpp_keywords;
std::set<std::string> g_set_python_keywords;
std::set<std::string> g_set_ruby_keywords;
std::set<std::string> g_set_perl_keywords;

auto isValidVarName(const std::string& str, GenLang language) -> bool
{
    // variable names must start with an alphabetic character or underscore character
    if ((str[0] < 'a' || str[0] > 'z') && (str[0] < 'A' || str[0] > 'Z') && str[0] != '_')
    {
        return false;
    }

    for (auto iter: str)
    {
        if ((iter < 'a' || iter > 'z') && (iter < 'A' || iter > 'Z') &&
            (iter < '0' || iter > '9') && iter != '_')
        {
            return false;
        }
    }

    // Ensure that the variable name is not a keyword in the specified language

    auto lambda = [&](std::set<std::string>& set_keywords, const char* language_keywords) -> bool
    {
        if (set_keywords.empty())
        {
            tt_string_vector keywords(language_keywords, ' ');
            for (auto& iter: keywords)
            {
                set_keywords.emplace(iter);
            }
        }

        if (set_keywords.contains(str))
        {
            return false;
        }

        return true;
    };

    // The set is only initialized the first time this function is called.
    if (language == GEN_LANG_CPLUSPLUS)
    {
        return lambda(g_set_cpp_keywords, g_u8_cpp_keywords);
    }
    if (language == GEN_LANG_PYTHON)
    {
        return lambda(g_set_python_keywords, g_python_keywords);
    }
    if (language == GEN_LANG_RUBY)
    {
        return lambda(g_set_ruby_keywords, g_ruby_keywords);
    }
    if (language == GEN_LANG_PERL)
    {
        return lambda(g_set_perl_keywords, g_perl_keywords);
    }

    return true;
}

auto CreateBaseFilename(Node* form_node, const tt_string& class_name) -> tt_string
{
    tt_string filename;
    if (class_name.size())
    {
        filename = class_name;
    }
    else
    {
        filename = form_node->as_string(prop_class_name);
    }

    if (filename.ends_with("Base"))
    {
        filename.erase(filename.size() - (sizeof("Base") - 1));
        filename += "_base";
    }

    filename.MakeLower();

    return filename;
}

auto CreateDerivedFilename(Node* form_node, const tt_string& class_name) -> tt_string
{
    tt_string filename;
    if (class_name.size())
    {
        filename = class_name;
    }
    else
    {
        filename = form_node->as_string(prop_derived_class_name);
    }

    if (filename.ends_with("Derived"))
    {
        filename.erase(filename.size() - (sizeof("Derived") - 1));
        filename += "_derived";
    }
    else if (!form_node->as_string(prop_base_file).ends_with("_base"))
    {
        filename += "_derived";
    }
    filename.MakeLower();

    return filename;
}

auto ConvertToSnakeCase(std::string_view str) -> std::string
{
    std::string result(str);
    for (size_t pos = 0, original_pos = 0; pos < result.size(); ++pos, ++original_pos)
    {
        if (str[original_pos] == '(')
        {
            // Assume that '(' means a function name is being passed, so stop when we reach the
            // first '('.
            break;
        }

        if (result[pos] >= 'A' && result[pos] <= 'Z')
        {
            result[pos] = static_cast<char>(result[pos] - 'A' + 'a');
            if (pos > 0)
            {
                // Do not add an underscore if the previous letter is uppercase
                if (str[original_pos - 1] >= 'A' && str[original_pos - 1] <= 'Z')
                {
                    continue;
                }
                result.insert(pos, "_");
                ++pos;
            }
        }
    }
    return result;
}

auto ConvertToUpperSnakeCase(tt_string_view str) -> tt_string
{
    tt_string result(str);
    for (size_t pos = 0, original_pos = 0; pos < result.size(); ++pos, ++original_pos)
    {
        if (result[pos] >= 'A' && result[pos] <= 'Z')
        {
            if (pos > 0)
            {
                // Do not add an underscore if the previous letter is uppercase
                if (str[original_pos - 1] >= 'A' && str[original_pos - 1] <= 'Z')
                {
                    continue;
                }
                result.insert(pos, "_");
                ++pos;
            }
        }
        else if (result[pos] >= 'a' && result[pos] <= 'z')
        {
            // convert to uppercase
            result[pos] = static_cast<char>(result[pos] - 'a' + 'A');
        }
    }
    return result;
}

auto FileNameToVarName(tt_string_view filename, size_t max_length) -> std::optional<tt_string>
{
    ASSERT(max_length > sizeof("_name_truncated"))

    if (filename.empty())
    {
        // caller's description does not include a filename
        return {};
    }

    tt_string var_name;

    if (ttwx::is_digit(filename[0]))
    {
        var_name += "img_";
    }

    for (size_t pos = 0; pos < filename.size(); pos++)
    {
        auto iter = static_cast<char>(filename[pos]);
        if (ttwx::is_alnum(iter) || iter == '_')
        {
            var_name += iter;
        }
        else
        {
            if (iter == '.')
            {
                // Always convert a period to an underscore in case it is preceeding the extension
                var_name += '_';
            }
            else if (var_name.size() && var_name.back() != '_' && pos > 0 &&
                     static_cast<unsigned char>(filename[pos - 1]) < 128)
            {
                var_name += '_';
            }
            // Ignore the first byte of a UTF-8 character sequence
            else if (static_cast<unsigned char>(iter) != 0xFF)
            {
                var_name += std::format("{:02x}", static_cast<unsigned char>(iter));
            }
        }

        if (var_name.size() > (max_length - sizeof("_name_truncated")))
        {
            // We don't want to create a variable name that is too long
            var_name += "_name_truncated";
            break;
        }
    }

    return var_name;
}

bool isScalingEnabled(Node* node, GenEnum::PropName prop_name, GenLang m_language)
{
    if (tt::contains(node->as_string(prop_name), 'n', tt::CASE::either))
    {
        return false;
    }

#if !PERL_FROM_DIP
    // REVIEW: [Randalphwa - 03-02-2025] As far as I have been able to determine, wxPerl does
    // not have a FromDIP function. So we need to disable DPI scaling for Perl.

    if (m_language == GEN_LANG_PERL)
    {
        return false;
    }
#endif
    return true;
}

auto GenLangToString(GenLang language) -> std::string_view
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            return "C++";
            break;
        case GEN_LANG_PERL:
            return "Perl";
            break;
        case GEN_LANG_PYTHON:
            return "Python";
            break;
        case GEN_LANG_RUBY:
            return "Ruby";
            break;
        case GEN_LANG_XRC:
            return "XRC";
            break;

        default:
            return "an unknown language";
            break;
    }
}

auto ConvertToGenLang(tt_string_view language) -> GenLang
{
    if (language.starts_with("C++") || language.starts_with("Folder C++"))
    {
        return GEN_LANG_CPLUSPLUS;
    }
    if (language == "Perl" || language.starts_with("wxPerl") ||
        language.starts_with("Folder wxPerl"))
    {
        return GEN_LANG_PERL;
    }
    if (language == "Python" || language.starts_with("wxPython") ||
        language.starts_with("Folder wxPython"))
    {
        return GEN_LANG_PYTHON;
    }
    if (language == "Ruby" || language.starts_with("wxRuby") ||
        language.starts_with("Folder wxRuby"))
    {
        return GEN_LANG_RUBY;
    }
    if (language.starts_with("XRC") || language.starts_with("Folder XRC"))
    {
        return GEN_LANG_XRC;

        // If this wasn't an actual language setting, then return all languages
    }
    return static_cast<GenLang>(GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON | GEN_LANG_RUBY |
                                GEN_LANG_PERL | GEN_LANG_XRC);
}

auto GetLanguageExtension(GenLang language) -> std::string
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            return ".cpp";
        case GEN_LANG_PERL:
            return ".pl";
        case GEN_LANG_PYTHON:
            return ".py";
        case GEN_LANG_RUBY:
            return ".rb";
        case GEN_LANG_XRC:
            return ".xrc";

        default:
            return ".cpp";
    }
}

auto GetClassOverrideType(Node* node) -> ClassOverrideType
{
    ASSERT(node != nullptr);
    if (node->HasValue(prop_subclass))
    {
        if (node->as_string(prop_subclass).starts_with("wxGeneric"))
        {
            return ClassOverrideType::Generic;  // Use the wxGeneric version of the class
        }

        return ClassOverrideType::Subclass;  // User specified a subclass
    }
    if (node->as_bool(prop_use_generic))
    {
        return ClassOverrideType::Generic;  // Use the wxGeneric version of the class
    }

    return ClassOverrideType::None;  // No override specified
}

auto CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream, size_t size) -> bool
{
    constexpr size_t BUF_KB_SIZE = 1024;
    constexpr size_t BUF_SIZE = 64 * BUF_KB_SIZE;

    size_t buf_size = (size > BUF_SIZE) ? BUF_SIZE : size;

    std::array<unsigned char, BUF_SIZE> read_buf {};
    auto read_size = buf_size;

    size_t copied_data = 0;
    for (;;)
    {
        if (size != tt::npos && copied_data + read_size > size)
        {
            read_size = size - copied_data;
        }
        inputStream->Read(read_buf.data(), read_size);

        auto actually_read = inputStream->LastRead();
        outputStream->Write(read_buf.data(), actually_read);
        if (outputStream->LastWrite() != actually_read)
        {
            return false;
        }

        if (size == tt::npos)
        {
            if (inputStream->Eof())
            {
                break;
            }
        }
        else
        {
            copied_data += actually_read;
            if (copied_data >= size)
            {
                break;
            }
        }
    }

    return true;
}

auto ShowOpenProjectDialog(wxWindow* parent) -> wxString
{
    wxFileDialog dialog(parent, "Open or Import Project", wxEmptyString, wxEmptyString,
                        wxString(std::format("wxUiEditor Project File (*{})|*{}"
                                             "|wxCrafter Project File (*.wxcp)|*.wxcp"
                                             "|DialogBlocks Project File (*.fjd)|*.fjd"
                                             "|wxFormBuilder Project File (*.fbp)|*.fbp"
                                             "|wxGlade File (*.wxg)|*.wxg"
                                             "|wxSmith File (*.wxs)|*.wxs"
                                             "|XRC File (*.xrc)|*.xrc"
                                             "|Windows Resource File (*.rc)|*.rc||",
                                             PROJECT_FILE_EXTENSION, PROJECT_FILE_EXTENSION)
                                     .c_str()),
                        wxFD_OPEN);

    if (dialog.ShowModal() == wxID_OK)
    {
        return dialog.GetPath();
    }
    return wxEmptyString;
}
