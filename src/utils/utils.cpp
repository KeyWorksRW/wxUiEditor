/////////////////////////////////////////////////////////////////////////////
// Purpose:   Utility functions that work with properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-16-2026]

#include <array>
#include <cstddef>
#include <cstdio>  // For snprintf
#include <format>
#include <set>

#include <wx/filedlg.h>   // wxFileDialog base header
#include <wx/filename.h>  // wxFileName class
#include <wx/gdicmn.h>    // Common GDI classes, types and declarations
#include <wx/mstream.h>   // Memory stream classes

#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

#include "wxue_namespace/wxue_string.h"         // wxue::string -- std::string with utility methods
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector
#include "wxue_namespace/wxue_view_vector.h"    // wxue::ViewVector

#include "keywords.h"

// Look for search string in line, and if found, replace with replace_with string. If all
// is true, replace all instances, otherwise only the first instance is replaced.
void utils::replace_in_line(std::string& line, std::string_view search,
                            std::string_view replace_with, bool replace_all)
{
    for (size_t pos = line.find(search, 0); pos != std::string::npos;
         pos = line.find(search, pos + replace_with.length()))
    {
        line.replace(pos, search.length(), replace_with);
        if (!replace_all)
        {
            break;
        }
    }
}

wxue::string DoubleToStr(double val)
{
    return wxue::string(std::format("{}", val));
}

wxue::string ClearPropFlag(wxue::string_view flag, wxue::string_view currentValue)
{
    if (flag.empty() || currentValue.empty())
    {
        return wxue::string(currentValue);
    }

    wxue::string result;
    const wxue::ViewVector mstr(currentValue, '|');
    for (const auto& iter: mstr)
    {
        if (iter != flag)
        {
            if (!result.empty())
            {
                result << '|';
            }
            result << iter;
        }
    }
    return result;
}

wxue::string ClearMultiplePropFlags(wxue::string_view flags, wxue::string_view currentValue)
{
    if (flags.empty() || currentValue.empty())
    {
        return wxue::string(currentValue);
    }

    wxue::string result;

    const wxue::StringVector mflags(flags, '|');

    const wxue::StringVector mstr(currentValue, '|');
    for (const auto& iter: mstr)
    {
        bool isFlagged = false;
        for (const auto& itFlags: mflags)
        {
            if (iter == itFlags)
            {
                isFlagged = true;
                break;
            }
        }

        if (!isFlagged)
        {
            if (!result.empty())
            {
                result << '|';
            }
            result << iter;
        }
    }
    return result;
}

wxue::string SetPropFlag(wxue::string_view flag, wxue::string_view currentValue)
{
    wxue::string result(currentValue);
    if (flag.empty())
    {
        return result;
    }

    wxue::ViewVector mstr(currentValue, '|');
    if (std::ranges::any_of(mstr,
                            [&](const auto& iter)
                            {
                                return iter.is_sameas(flag);
                            }))
    {
        return result;  // flag has already been added
    }
    if (!result.empty())
    {
        result << '|';
    }
    result << flag;
    return result;
}

bool isPropFlagSet(wxue::string_view flag, wxue::string_view currentValue)
{
    if (flag.empty() || currentValue.empty())
    {
        return false;
    }

    wxue::ViewVector mstr(currentValue, '|');
    return std::ranges::any_of(mstr,
                               [&](const auto& iter)
                               {
                                   return iter.is_sameas(flag);
                               });
}

wxSystemColour ConvertToSystemColour(wxue::string_view value)
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

const char* ConvertFontFamilyToString(wxFontFamily family)
{
    switch (family)
    {
        case wxFONTFAMILY_DECORATIVE:
            return "wxFONTFAMILY_DECORATIVE";
        case wxFONTFAMILY_ROMAN:
            return "wxFONTFAMILY_ROMAN";
        case wxFONTFAMILY_SCRIPT:
            return "wxFONTFAMILY_SCRIPT";
        case wxFONTFAMILY_SWISS:
            return "wxFONTFAMILY_SWISS";
        case wxFONTFAMILY_MODERN:
            return "wxFONTFAMILY_MODERN";
        case wxFONTFAMILY_TELETYPE:
            return "wxFONTFAMILY_TELETYPE";
        default:
            return "wxFONTFAMILY_DEFAULT";
    }
}

wxue::string ConvertEscapeSlashes(wxue::string_view str)
{
    wxue::string result;

    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        const char current_char = str[pos];
        if (current_char == '\\')
        {
            // REVIEW: [KeyWorks - 06-07-2020] Like the original wxString version, this will not
            // save a backslash if it is at the end of a line. Is that intentional or just a bug?
            if (pos < str.length() - 1)
            {
                const char next_char = str[pos + 1];

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
        result += current_char;
    }

    return result;
}

wxPoint DlgPoint(Node* node, GenEnum::PropName prop)
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

int DlgPoint(int width)
{
    return wxGetMainFrame()->getWindow()->FromDIP(wxPoint(width, -1)).x;
}

wxSize GetSizeInfo(wxue::string_view description)
{
    wxSize size;

    wxue::ViewVector size_description;
    if (description.contains(";"))
    {
        size_description.SetString(description, ';', wxue::TRIM::left);
    }
    else
    {
        size_description.SetString(description, ',');
    }

    ASSERT(!size_description.empty())
    ASSERT(!size_description[0].empty())

    if (!size_description.empty())
    {
        const size_t start = size_description[0].front() == '[' ? 1 : 0;
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

bool isConvertibleMime(const wxue::string& suffix)
{
    return std::ranges::all_of(lst_no_png_conversion,
                               [&](const char* iter)
                               {
                                   return !suffix.is_sameas(iter);
                               });
}

std::set<std::string> g_set_cpp_keywords;
std::set<std::string> g_set_python_keywords;
std::set<std::string> g_set_ruby_keywords;
std::set<std::string> g_set_fortran_keywords;
std::set<std::string> g_set_go_keywords;
std::set<std::string> g_set_julia_keywords;
std::set<std::string> g_set_luajit_keywords;
std::set<std::string> g_set_typescript_keywords;

bool isValidVarName(const std::string& str, GenLang language)
{
    if (str.empty())
        return false;

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
            wxue::StringVector keywords(language_keywords, ' ');
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
    if (language == GenLang::cplusplus)
    {
        return lambda(g_set_cpp_keywords, g_u8_cpp_keywords);
    }
    if (language == GenLang::python)
    {
        return lambda(g_set_python_keywords, g_python_keywords);
    }
    if (language == GenLang::ruby)
    {
        return lambda(g_set_ruby_keywords, g_ruby_keywords);
    }
    if (language == GenLang::typescript)
    {
        return lambda(g_set_typescript_keywords, g_typescript_keywords);
    }

    return true;
}

wxue::string CreateBaseFilename(Node* form_node, const wxue::string& class_name)
{
    wxue::string filename;
    if (!class_name.empty())
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

wxue::string CreateDerivedFilename(Node* form_node, const wxue::string& class_name)
{
    wxue::string filename;
    if (!class_name.empty())
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

std::string ConvertToSnakeCase(std::string_view str)
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

wxue::string ConvertToUpperSnakeCase(wxue::string_view str)
{
    wxue::string result(str);
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

std::optional<wxue::string> FileNameToVarName(wxue::string_view filename, size_t max_length)
{
    ASSERT(max_length > sizeof("_name_truncated"))

    if (filename.empty())
    {
        // caller's description does not include a filename
        return {};
    }

    wxue::string var_name;

    if (wxue::is_digit(filename[0]))
    {
        var_name += "img_";
    }

    for (size_t pos = 0; pos < filename.size(); ++pos)
    {
        auto current_ch = static_cast<char>(filename[pos]);
        if (wxue::is_alnum(current_ch) || current_ch == '_')
        {
            var_name += current_ch;
        }
        else
        {
            if (current_ch == '.')
            {
                // Always convert a period to an underscore in case it is preceding the extension
                var_name += '_';
            }
            else if (!var_name.empty() && var_name.back() != '_' && pos > 0 &&
                     static_cast<unsigned char>(filename[pos - 1]) < 128)
            {
                var_name += '_';
            }
            // Ignore the first byte of a UTF-8 character sequence
            else if (static_cast<unsigned char>(current_ch) != 0xFF)
            {
                var_name += std::format("{:02x}", static_cast<unsigned char>(current_ch));
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

bool isScalingEnabled(Node* node, GenEnum::PropName prop_name, [[maybe_unused]] GenLang m_language)
{
    return !wxue::contains(node->as_string(prop_name), 'n', wxue::CASE::either);
}

std::string_view GenLangToString(GenLang language)
{
    switch (language)
    {
        case GenLang::cplusplus:
            return "C++";

        case GenLang::python:
            return "Python";
        case GenLang::ruby:
            return "Ruby";
        case GenLang::fortran:
            return "Fortran";
        case GenLang::go:
            return "Go";
        case GenLang::julia:
            return "Julia";
        case GenLang::luajit:
            return "LuaJIT";
        case GenLang::typescript:
            return "TypeScript";
        case GenLang::xrc:
            return "XRC";
        default:
            return "an unknown language";
    }
}

GenLang ConvertToGenLang(wxue::string_view language)
{
    if (language.starts_with("C++") || language.starts_with("Folder C++"))
    {
        return GenLang::cplusplus;
    }

    if (language == "Python" || language.starts_with("wxPython") ||
        language.starts_with("Folder wxPython"))
    {
        return GenLang::python;
    }
    if (language == "Ruby" || language.starts_with("wxRuby") ||
        language.starts_with("Folder wxRuby"))
    {
        return GenLang::ruby;
    }
    if (language == "Fortran" || language.starts_with("kwxFortran") ||
        language.starts_with("Folder kwxFortran"))
    {
        return GenLang::fortran;
    }
    if (language == "GO" || language.starts_with("kwxGO") || language.starts_with("Folder kwxGO"))
    {
        return GenLang::go;
    }
    if (language == "Julia" || language.starts_with("kwxJulia") ||
        language.starts_with("Folder kwxJulia"))
    {
        return GenLang::julia;
    }
    if (language == "LuaJIT" || language.starts_with("kwxLuaJIT") ||
        language.starts_with("Folder kwxLuaJIT"))
    {
        return GenLang::luajit;
    }
    if (language == "TypeScript" || language.starts_with("kwxTypeScript") ||
        language.starts_with("Folder kwxTypeScript"))
    {
        return GenLang::typescript;
    }
    if (language.starts_with("XRC") || language.starts_with("Folder XRC"))
    {
        return GenLang::xrc;
    }
    // If this wasn't an actual language setting, then return all languages
    return static_cast<GenLang>(GenLang::cplusplus | GenLang::python | GenLang::ruby |
                                GenLang::xrc);
}

std::string GetLanguageExtension(GenLang language)
{
    switch (language)
    {
        case GenLang::cplusplus:
            return ".cpp";

        case GenLang::python:
            return ".py";
        case GenLang::ruby:
            return ".rb";
        case GenLang::fortran:
            return ".f90";
        case GenLang::go:
            return ".go";
        case GenLang::julia:
            return ".jl";
        case GenLang::luajit:
            return ".lua";
        case GenLang::typescript:
            return ".ts";
        case GenLang::xrc:
            return ".xrc";

        default:
            return ".cpp";
    }
}

ClassOverrideType GetClassOverrideType(Node* node)
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

bool CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream, size_t size)
{
    constexpr size_t BUF_KB_SIZE = 1024;
    constexpr size_t BUF_SIZE = 64 * BUF_KB_SIZE;

    const size_t buf_size = (size > BUF_SIZE) ? BUF_SIZE : size;

    auto read_buf = std::make_unique<unsigned char[]>(BUF_SIZE);
    size_t read_size = buf_size;

    size_t copied_data = 0;
    for (;;)
    {
        if (size != wxue::npos && copied_data + read_size > size)
        {
            read_size = size - copied_data;
        }
        inputStream->Read(read_buf.get(), read_size);

        const size_t actually_read = inputStream->LastRead();
        outputStream->Write(read_buf.get(), actually_read);
        if (outputStream->LastWrite() != actually_read)
        {
            return false;
        }

        if (size == wxue::npos)
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

void AddHeaderExtension(wxue::string& filename, bool force)
{
    if (Project.as_string(prop_header_ext).empty())
    {
        return;
    }

    if (filename.extension().empty() || force)
    {
        filename.replace_extension(Project.as_string(prop_header_ext));
    }
}

void AddHeaderExtension(wxString& filename, bool force)
{
    wxFileName file_name(filename);
    if (file_name.HasExt() && !force)
    {
        return;
    }

    const wxue::string& header_ext = Project.as_string(prop_header_ext);
    if (header_ext.empty())
    {
        return;
    }

    // wxFileName::SetExt() expects the extension without a leading dot
    const wxString new_ext =
        header_ext.starts_with('.') ? header_ext.subview(1).wx() : header_ext.wx();
    file_name.SetExt(new_ext);

    // Replace only the filename+extension portion of the original string, preserving
    // any path prefix exactly as it was.
    const size_t sep_pos = filename.find_last_of("\\/");
    const size_t name_start = (sep_pos == wxString::npos) ? 0 : sep_pos + 1;
    filename.replace(name_start, filename.length() - name_start, file_name.GetFullName());
}

wxString ShowOpenProjectDialog(wxWindow* parent)
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
