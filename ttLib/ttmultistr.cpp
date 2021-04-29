/////////////////////////////////////////////////////////////////////////////
// Name:      ttmultistr.cpp
// Purpose:   Breaks a single string into multiple strings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2018-2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ttmultistr.h"

using namespace ttlib;

void multistr::SetString(std::string_view str, char separator)
{
    clear();
    size_t start = 0;
    size_t end = str.find_first_of(separator);
    while (end != std::string_view::npos)
    {
        emplace_back();
        back().assign(str.substr(start, end - start));

        start = end + sizeof(char);
        if (start >= str.length())
            return;
        end = str.find_first_of(separator, start);
    }
    emplace_back();
    back().assign(str.substr(start));
}

void multistr::SetString(std::string_view str, std::string_view separator)
{
    clear();
    size_t start = 0;
    size_t end = str.find_first_of(separator);
    while (end != std::string_view::npos)
    {
        emplace_back();
        back().assign(str.substr(start, end - start));

        start = end + separator.size();
        if (start >= str.length())
            return;
        end = str.find_first_of(separator, start);
    }
    emplace_back();
    back().assign(str.substr(start));
}

/////////////////////////////////////// multiview ///////////////////////////////////////

void multiview::SetString(std::string_view str, char separator)
{
    clear();
    size_t start = 0;
    size_t end = str.find_first_of(separator);
    while (end != std::string_view::npos)
    {
        push_back(str.substr(start, end - start));

        start = end + sizeof(char);
        if (start >= str.length())
            return;
        end = str.find_first_of(separator, start);
    }
    push_back(str.substr(start));
}

void multiview::SetString(std::string_view str, std::string_view separator)
{
    clear();
    size_t start = 0;
    size_t end = str.find_first_of(separator);
    while (end != std::string_view::npos)
    {
        push_back(str.substr(start, end - start));

        start = end + separator.size();
        if (start >= str.length())
            return;
        end = str.find_first_of(separator, start);
    }
    push_back(str.substr(start));
}
