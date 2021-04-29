/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions accessing translatable strings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>

#include <ttcstr.h>   // cstr -- Classes for handling zero-terminated char strings.
#include <ttcview.h>  // cview -- string_view functionality on a zero-terminated char string.

/// This needs to be declared and initialized in whatever source file you use to declare all
/// of your strings
extern const std::map<int, const char*>* _tt_CurLanguage;

inline void _ttSetCurLanguage(const std::map<int, const char*>* lang)
{
    _tt_CurLanguage = lang;
}

/// Looks up the translated string based on the current _tt_CurLanguage map.
const char* _tt(int id);

/// Looks up the translated string based on the current _tt_CurLanguage map.
///
/// Use this if you need a zero-terminated string_view
ttlib::cview _ttv(int id);

/// Looks up the translated string based on the current _tt_CurLanguage map.
///
/// Use this if you need to use << or + operators to add to the string.
ttlib::cstr _ttc(int id);

#if defined(_WIN32)

/// Looks up the translated string based on the current _tt_CurLanguage map.
///
/// Use this to pass the string to the wxString class in wxWidgets. On Windows,
/// it returns a UTF16 converted string. On non-Windows, it simply returns a const char*.
std::wstring _ttwx(int id);

#else

/// Looks up the translated string based on the current _tt_CurLanguage map.
///
/// Use this to pass the string to the wxString class in wxWidgets. On Windows,
/// it returns a UTF16 converted string. On non-Windows, it simply returns a const char*.
inline const char* _ttwx(int id)
{
    return _tt(id);
}

#endif  // _WIN32

/////////////////////////////////////////////////////////////////////
//
// The following can be used if you prefer to perform the lookup using an english string rather than an id. You must
// have declared and initialized a _tt_english pointer that points to your std:map pair of ids and english strings. You
// can then call _ttSetCurLanguage to point to a different language and all of the calls below will return the matching
// translation.
//
// Note that lookup is considerable slower than using an ID, and is more error prone -- if the string parameter you pass
// does not match exactly the string in _tt_english, then you will not get a translated string.
//
/////////////////////////////////////////////////////////////////////

/// If you want to use _tt(const char*) then you must declare and initialize this variable in
/// whatever source file you use to declare all of your strings
extern const std::map<int, const char*>* _tt_english;

/// This will lookup the string in _tt_english and use the matching id to lookup the string
/// in whatever _tt_CurLanguage is pointing to.
const char* _tt(const char* str);

/// This will lookup the string in _tt_english and use the matching id to lookup the string
/// in whatever _tt_CurLanguage is pointing to.
///
/// Use this if you need a zero-terminated string_view
ttlib::cview _ttv(const char* str);

/// This will lookup the string in _tt_english and use the matching id to lookup the string
/// in whatever _tt_CurLanguage is pointing to.
///
/// Use this if you need to use << or + operators to add to the string.
ttlib::cstr _ttc(const char* str);

#if defined(_WIN32)

/// Looks up the translated string based on the current _tt_CurLanguage map.
///
/// Use this to pass the string to the wxString class in wxWidgets. On Windows,
/// it returns a UTF16 converted string. On non-Windows, it simply returns a const char*.
std::wstring _ttwx(const char* str);

#else

/// Looks up the translated string based on the current _tt_CurLanguage map.
///
/// Use this to pass the string to the wxString class in wxWidgets. On Windows,
/// it returns a UTF16 converted string. On non-Windows, it simply returns a const char*.
inline const char* _ttwx(const char* str)
{
    return _tt(str);
}

#endif  // _WIN32
