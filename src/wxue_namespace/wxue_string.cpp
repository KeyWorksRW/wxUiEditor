/////////////////////////////////////////////////////////////////////////////
// Purpose:   std::string with additional methods
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dir.h>  // wxDir is a portable equivalent of Unix open strstrdir/strreaddir/strclose
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/string.h>    // wxString class

#include <cstring>

#include "wxue_string.h"

namespace wxue
{

    auto string::trim(wxue::TRIM where) -> string&
    {
        if (empty())
        {
            return *this;
        }
        if (where == wxue::TRIM::right || where == wxue::TRIM::both)
        {
            auto len = length();
            for (--len; len != std::string::npos; --len)
            {
                // char ch = at(len);
                char ch = c_str()[len];
                if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' && ch != '\f')
                {
                    break;
                }
            }

            if (len + 1 < length())
            {
                erase(len + 1, length() - len);
            }
        }

        // If trim(right) was called above, the string may now be empty -- front() fails on an empty
        // string
        if (!empty() && (where == wxue::TRIM::left || where == wxue::TRIM::both))
        {
            // Assume that most strings won't start with whitespace, so return as quickly as
            // possible if that is the case.
            if (!wxue::is_whitespace(front()))
            {
                return *this;
            }

            size_t pos;
            for (pos = 1; pos < length(); ++pos)
            {
                if (!wxue::is_whitespace(c_str()[pos]))
                {
                    break;
                }
            }
            replace(0, length(), substr(pos, length() - pos));
        }

        return *this;
    }

    /**
     * @param src -- string to parse
     * @param chBegin -- character that prefixes the string
     * @param chEnd -- character that terminates the string.
     */
    auto string::AssignSubString(std::string_view src, char chBegin, char chEnd) -> size_t
    {
        if (src.empty())
        {
            assign(wxue::emptystring);
            return npos;
        }

        size_t pos = 0;
        // step over any leading whitespace unless chBegin is a whitespace character
        if (!wxue::is_whitespace(chBegin))
        {
            while (wxue::is_whitespace(src[pos]))
            {
                ++pos;
            }
        }

        if (src[pos] == chBegin)
        {
            ++pos;
            auto start = pos;
            while (pos < src.length() && src[pos] != chEnd)
            {
                // REVIEW: [KeyWorks - 01-26-2020] '\"' is also valid for the C compiler, though the
                // slash is unnecessary. Should we support it?

                // only check quotes and backslashes -- a slash is ignored before other character
                // pairs.
                if (src[pos] == '\\' && pos + 1 < src.length())
                {
                    if (src[pos + 1] == '\\' ||
                        ((chBegin == '"' || chBegin == '\'') && src[pos + 1] == chEnd))
                    {
                        // step over an escaped quote if the string to fetch is within a quote
                        pos += 2;
                        continue;
                    }
                }
                ++pos;
            }
            insert(0, src.substr(start, pos - start));
        }
        else
        {
            // if the string didn't start with chBegin, just copy the string
            *this = src;
            return src.length();
        }
        return pos;
    }

    /**
     * @param str -- the string to extract from
     * @param start -- the offset into the string to begin extraction from -- this should either be
     * to leading whitespace, or to the character that marks the beginning of the sub string.
     * @return The offset to the character that terminated extraction, or **npos** if a terminating
     *         character could not be found.
     */
    auto string::ExtractSubString(std::string_view src, size_t start) -> size_t
    {
        if (src.empty())
        {
            assign(wxue::emptystring);
            return npos;
        }

        // start by finding the first non-whitespace character
        size_t pos = start;
        while (pos < src.length() && wxue::is_whitespace(src[pos]))
        {
            ++pos;
        }

        if (pos >= src.length())
        {
            return npos;
        }

        // based on the opening character, determine what the matching end character should be
        char chEnd;
        char chBegin = src[pos];
        if (chBegin == '`' || chBegin == '\'')
        {
            chEnd = '\'';
        }
        else if (chBegin == '"')
        {
            chEnd = '"';
        }
        else if (chBegin == '<')
        {
            chEnd = '>';
        }
        else if (chBegin == '[')
        {
            chEnd = ']';
        }
        else if (chBegin == '(')
        {
            chEnd = ')';
        }
        else
        {
            return npos;
        }

        // now find the substring between chBegin and chEnd
        auto startPos = pos + 1;
        for (++pos; pos < src.length(); ++pos)
        {
            // only check quotes -- a slash is valid before other character pairs.
            if (src[pos] == '\\' && (chBegin == '"' || chBegin == '\'') && pos + 1 < src.length() &&
                (src[pos + 1] == chEnd))
            {
                // step over an escaped quote if the string to fetch is within a quote
                ++pos;
                continue;
            }
            if (src[pos] == chEnd)
            {
                break;
            }
        }

        if (pos >= src.length())
        {
            return npos;
        }

        assign(src.substr(startPos, pos - startPos));
        return pos;
    }

    /**
     * @param oldtext -- the text to search for
     * @param newtext -- the text to replace it with
     * @param replace_all -- replace all occurrences or just the first one
     * @param CaseSensitive -- indicates whether or not to use a case-insensitive search
     * @return Number of replacements made
     */
    auto string::Replace(std::string_view oldtext, std::string_view newtext, bool replace_all,
                         wxue::CASE checkcase) -> size_t
    {
        if (oldtext.empty())
        {
            return false;
        }

        size_t replacements = 0;
        if (auto pos = locate(oldtext, 0, checkcase); wxue::is_found(pos))
        {
            do
            {
                if (oldtext.size() == newtext.size())
                {
                    for (size_t idx = 0; idx < newtext.size(); ++idx)
                    {
                        data()[pos + idx] = newtext[idx];
                    }
                }
                else
                {
                    erase(pos, oldtext.length());
                    insert(pos, newtext);
                }
                ++replacements;
                pos += newtext.length();
                if (pos >= size() || !replace_all)
                {
                    break;
                }
                pos = locate(oldtext, pos, checkcase);
            } while (wxue::is_found(pos));
        }

        return replacements;
    }

    auto string::MakeLower() -> string&
    {
        wxString str = wxString::FromUTF8(data());
        str.MakeLower();
        *this = str.utf8_string();
        return *this;
    }

    string& string::MakeUpper()
    {
        wxString str = wxString::FromUTF8(data());
        str.MakeUpper();
        *this = str.utf8_string();
        return *this;
    }

    auto string::backslashestoforward() -> string&
    {
        for (auto pos = find('\\'); pos != std::string::npos; pos = find('\\'))
        {
            replace(pos, 1, "/");
        }
        return *this;
    }

    auto string::forwardslashestoback() -> string&
    {
        for (auto pos = find('/'); pos != std::string::npos; pos = find('/'))
        {
            replace(pos, 1, "\\");
        }
        return *this;
    }

    auto string::replace_extension(std::string_view newExtension) -> string&
    {
        if (empty())
        {
            if (newExtension.empty())
            {
                return *this;
            }
            if (newExtension.at(0) != '.')
            {
                push_back('.');
            }
            append(newExtension);
            return *this;
        }

        auto pos_file = find_filename();
        if (!wxue::is_found(pos_file))
        {
            pos_file = 0;
        }

        if (auto pos = find_last_of('.'); wxue::is_found(pos) && pos > pos_file)
        {
            // If the string only contains . or .. then it is a folder
            if (pos == 0 || (pos == 1 && at(0) != '.'))
            {
                return *this;  // can't add an extension if it isn't a valid filename
            }

            if (newExtension.empty())
            {
                // If the new extension is empty, then just erase the old extension.
                erase(pos);
            }
            else
            {
                // If the new extension doesn't start with '.', then keep our own '.' prefix.
                if (newExtension.at(0) != '.')
                {
                    ++pos;
                }
                replace(pos, length() - pos, newExtension);
            }
        }
        else if (newExtension.size())
        {
            // Current filename doesn't have an extension, so append the new one
            if (newExtension.at(0) != '.')
            {
                push_back('.');
            }
            append(newExtension);
        }

        return *this;
    }

    auto string::find_filename() const noexcept -> size_t
    {
        if (empty())
        {
            return npos;
        }

        auto pos = find_last_of('/');

#ifdef _WIN32
        // Windows filenames can contain both forward and back slashes, so check for a backslash as
        // well.
        auto back = find_last_of('\\');
        if (back != npos)
        {
            // If there is no forward slash, or the backslash appears after the forward slash, then
            // use it's position.
            if (pos == npos || back > pos)
            {
                pos = back;
            }
        }
#endif
        if (pos == npos)
        {
            pos = find_last_of(':');
            if (pos == npos)
            {
                return npos;
            }
        }

        return pos + 1;
    }

    auto string::replace_filename(std::string_view newFilename) -> string&
    {
        if (empty())
        {
            assign(newFilename);
            return *this;
        }

        auto pos = find_last_of('/');

#ifdef _WIN32
        // Windows filenames can contain both forward and back slashes, so check for a backslash as
        // well.
        auto back = find_last_of('\\');
        if (back != npos)
        {
            // If there is no forward slash, or the backslash appears after the forward slash, then
            // use it's position.
            if (pos == npos || back > pos)
            {
                pos = back;
            }
        }
#endif
        if (pos == npos)
        {
            pos = find_last_of(':');
            if (pos == npos)
            {
                // If we get here, we think the entire current string is a filename.
                assign(newFilename);
                return *this;
            }
        }

        replace(pos + 1, length() - (pos + 1), newFilename);
        return *this;
    }

    string& string::append_filename(std::string_view filename)
    {
        if (filename.empty())
        {
            return *this;
        }
        if (empty())
        {
            assign(filename);
            return *this;
        }

        auto last = back();
        if (last != '/' && last != '\\')
        {
            push_back('/');
        }
        append(filename);
        return *this;
    }

    string& string::assignCwd()
    {
        *this = wxGetCwd().ToStdString();
        return *this;
    }

    string& string::make_relative(string_view relative_to)
    {
        if (empty())
        {
            return *this;
        }

        wxFileName filename(wx());
        filename.MakeAbsolute();

        wxString base_dir;
        if (relative_to.empty())
        {
            base_dir = wxGetCwd();
        }
        else
        {
            base_dir = relative_to.wx();
        }

        filename.MakeRelativeTo(base_dir);
        *this = filename.GetFullPath().ToStdString();
        return *this;
    }

    string& string::make_absolute()
    {
        if (!empty())
        {
            wxFileName filename(wx());
            filename.MakeAbsolute();
            *this = filename.GetFullPath().ToStdString();
        }
        return *this;
    }

    wxDateTime string::last_write_time() const
    {
        wxFileName filename(wx());
        return filename.GetModificationTime();
    }

    wxULongLong string::file_size() const
    {
        return wxFileName::GetSize(wx());
    }

    bool string::ChangeDir(bool is_dir) const
    {
        if (empty())
        {
            return false;
        }

        wxString path_str = wx();
        if (!is_dir)
        {
            wxFileName filename(path_str);
            path_str = filename.GetPath();
        }

        if (!wxFileName::DirExists(path_str))
        {
            return false;
        }

        return wxSetWorkingDirectory(path_str);
    }

    bool string::assignEnvVar(const char* env_var)
    {
        clear();
        if (!env_var || !*env_var)
        {
            return false;
        }
        auto* pEnv = std::getenv(env_var);
        if (!pEnv)
        {
            return false;
        }
        assign(pEnv);
        return true;
    }

    void string::erase_from(char chr)
    {
        auto pos = find(chr);
        if (pos != npos)
        {
            erase(pos);
            trim();
        }
    }

    void string::erase_from(std::string_view sub)
    {
        auto pos = find(sub);
        if (pos != npos)
        {
            erase(pos);
            trim();
        }
    }

    // This is a static function, so no access to the class buffer.
    bool string::MkDir(const string& path, bool recursive)
    {
        if (path.empty())
        {
            return false;
        }

        wxFileName dir(path.wx());
        const int flags = recursive ? wxPATH_MKDIR_FULL : 0;
        return dir.Mkdir(wxS_DIR_DEFAULT, flags);
    }

}  // namespace wxue
