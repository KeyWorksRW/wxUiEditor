/////////////////////////////////////////////////////////////////////////////
// Name:      ttcstr.cpp
// Purpose:   Class for handling zero-terminated char strings.
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ttstrings.h"

const char* _tt(int id)
{
    assert(_tt_CurLanguage);
    if (auto result = _tt_CurLanguage->find(id); result != _tt_CurLanguage->end())
    {
        return result->second;
    }
    return "";
}

ttlib::cview _ttv(int id)
{
    assert(_tt_CurLanguage);
    if (auto result = _tt_CurLanguage->find(id); result != _tt_CurLanguage->end())
    {
        return result->second;
    }
    return "";
}

ttlib::cstr _ttc(int id)
{
    assert(_tt_CurLanguage);
    if (auto result = _tt_CurLanguage->find(id); result != _tt_CurLanguage->end())
    {
        return ttlib::cstr(result->second);
    }
    return ttlib::cstr();
}

#if defined(_WIN32)

std::wstring _ttwx(int id)
{
    assert(_tt_CurLanguage);
    if (auto result = _tt_CurLanguage->find(id); result != _tt_CurLanguage->end())
    {
        return ttlib::utf8to16(result->second);
    }
    return std::wstring();
}

#endif

const char* _tt(const char* str)
{
    if (!_tt_english || !str)
        return str;

    for (auto iter = _tt_english->begin(); iter != _tt_english->end(); ++iter)
    {
        if (ttlib::is_sameas(iter->second, str))
        {
            return _tt(iter->first);
        }
    }

    return str;
}

ttlib::cview _ttv(const char* str)
{
    if (!_tt_english || !str)
        return str;

    for (auto iter = _tt_english->begin(); iter != _tt_english->end(); ++iter)
    {
        if (ttlib::is_sameas(iter->second, str))
        {
            return _ttv(iter->first);
        }
    }

    return str;
}

ttlib::cstr _ttc(const char* str)
{
    if (!str)
        return ttlib::cstr();
    if (!_tt_english)
        return ttlib::cstr(str);

    for (auto iter = _tt_english->begin(); iter != _tt_english->end(); ++iter)
    {
        if (ttlib::is_sameas(iter->second, str))
        {
            return _ttc(iter->first);
        }
    }

    return ttlib::cstr(str);
}

#if defined(_WIN32)

std::wstring _ttwx(const char* str)
{
    if (!str)
        return std::wstring();
    if (!_tt_english)
        return ttlib::utf8to16(str);

    for (auto iter = _tt_english->begin(); iter != _tt_english->end(); ++iter)
    {
        if (ttlib::is_sameas(iter->second, str))
        {
            return _ttwx(iter->first);
        }
    }

    return ttlib::utf8to16(str);
}

#endif
