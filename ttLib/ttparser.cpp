/////////////////////////////////////////////////////////////////////////////
// Name:      ttparser.cpp
// Purpose:   Command line parser
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <cstdarg>
#include <iostream>

#include "ttparser.h"

using namespace ttlib;

// If the only type of argument we accepted was "char** argv" then we could just use that. However, then Windows GUI
// apps couldn't use this, nor could wide-char (UNICODE) Windows console or wxWidgets apps use it. It's inefficient to
// convert "char** argv" to a vector, but doing so means we can support additional types of command lines.

cmd::cmd(int argc, char** argv)
{
    for (auto argpos = 1; argpos < argc; ++argpos)
        m_originalArgs += argv[argpos];
    m_hasCommandArgs = true;
}

cmd::cmd(int argc, wchar_t** argv)
{
    for (auto argpos = 1; argpos < argc; ++argpos)
    {
        auto& arg = m_originalArgs.emplace_back();
        arg.from_utf16(argv[argpos]);
    }
    m_hasCommandArgs = true;
}

void cmd::addOption(std::string_view name, std::string_view description)
{
    assert(!name.empty());

    auto popt = std::make_unique<cmd::Option>();
    popt->m_description = description;
    popt->m_flags = 0;

    m_options.emplace(shortlong(name), std::move(popt));
}

void cmd::addOption(std::string_view name, std::string_view description, size_t flags)
{
    assert(!name.empty());

    auto popt = std::make_unique<cmd::Option>();
    popt->m_description = description;
    popt->m_flags = flags;

    m_options.emplace(shortlong(name), std::move(popt));
}

void cmd::addOption(std::string_view name, std::string_view description, size_t flags, size_t setvalue)
{
    assert(!name.empty());

    auto popt = std::make_unique<cmd::Option>();
    popt->m_description = description;
    popt->m_flags = flags;

    if (flags & cmd::shared_val)
        popt->m_setvalue = setvalue;

    m_options.emplace(shortlong(name), std::move(popt));
}

void cmd::addHiddenOption(std::string_view name, size_t flags, size_t setvalue)
{
    assert(!name.empty());

    auto popt = std::make_unique<cmd::Option>();
    popt->m_flags = flags | cmd::hidden;

    if (flags & cmd::shared_val)
        popt->m_setvalue = setvalue;

    m_options.emplace(shortlong(name), std::move(popt));
}

// If the name contains a '|' character, then break it into a short name and a long name. The
// two names are then added to the m_shortlong map so that any option name request that has a
// short name can be remapped to it's long name.
cstr cmd::shortlong(std::string_view name)
{
    ttlib::cstr result;
    if (auto pos = name.find('|'); !ttlib::is_error(pos))
    {
        std::string shortname;
        shortname.assign(name.substr(0, pos));
        result.assign(name.substr(pos + 1));
        m_shortlong.emplace(shortname, result);
    }
    else
    {
        result.assign(name);
        // If only a short name is specified, map it to a long name using the same name. It makes future lookup less
        // complicated if it's always possible to map a short name to some kind of long name, even if it's identical to
        // the short name.
        if (name.size() < 2)
        {
            std::string shortname;
            shortname.assign(name);
            m_shortlong.emplace(shortname, result);
        }
    }
    return result;
}

// Option can be a short name, a long name, or two combined names separated with a '|'.
cmd::Option* cmd::findOption(std::string_view option) const
{
    assert(!option.empty());

    ttlib::cstr longname;
    if (auto pos = option.find('|'); !ttlib::is_error(pos))
    {
        longname.assign(option.substr(pos + 1));
    }
    else
    {
        longname.assign(option);
        if (option.size() < 2)
        {
            auto entry = m_shortlong.find(longname);
            if (entry != m_shortlong.end())
                longname = entry->second;
        }
    }

    if (auto entry = m_options.find(longname); entry != m_options.end())
    {
        return entry->second.get();
    }
    return nullptr;
}

bool cmd::isOption(std::string_view name) const
{
    assert(!name.empty());

    if (auto option = findOption(name); option)
    {
        if ((option->m_flags & needsarg))
        {
            return (option->m_result.size());
        }
        return (!option->m_result.empty() && option->m_result.is_sameas("true"));
    }
    return false;
}

std::optional<ttlib::cstr> cmd::getOption(std::string_view name)
{
    assert(!name.empty());

    if (auto option = findOption(name); option)
    {
        if ((option->m_flags & needsarg) && option->m_result.empty())
        {
            return {};
        }
        return { option->m_result };
    }
    return {};
}

bool cmd::parse(int argc, char** argv)
{
    for (auto argpos = 1; argpos < argc; ++argpos)
        m_originalArgs += argv[argpos];
    m_hasCommandArgs = true;
    return parse();
}

bool cmd::parse()
{
    if (!m_hasCommandArgs)
    {
#if defined(_WIN32)
        WinInit();
#else
        m_results.emplace_back(Result::no_argc);
        return false;
#endif  // _WIN32
    }

    bool result = true;

    // We need to look at the next argument with a check for going beyond the end, so we use argpos instead of a normal
    // iteration
    for (size_t argpos = 0; argpos < m_originalArgs.size(); ++argpos)
    {
        auto arg = m_originalArgs[argpos].subview();
        if (arg.empty())
            continue;

#if defined(_WIN32)
        if (arg.at(0) == '-' || arg.at(0) == '/')
#else
        if (arg[0] == '-')
#endif  // _WIN32
        {
            arg.remove_prefix(1);

            // long names are sometimes specified with -- so remove the second hyphen if that's the case
            if (arg.at(0) == '-')
                arg.remove_prefix(1);

            // If the argument is followed by a quote, then add it whether this is an argument type or not
            if (auto pos = arg.find('"'); pos != tt::npos)
            {
                auto name = shortlong(arg.substr(0, pos));
                if (auto entry = m_options.find(name); entry != m_options.end())
                {
                    entry->second->m_result.ExtractSubString(arg, pos);
                }
                else
                {
                    m_results.emplace_back(Result::unknown_opt);
                }
                continue;
            }

            if (arg.is_sameprefix("?"))
            {
                m_HelpRequested = true;
                continue;
            }

            // REVIEW: [KeyWorks - 04-28-2020] If we are going to allow joined short names, then we have to handle that here

            if (auto option = findOption(arg); option)
            {
                if (option->m_flags & cmd::help)
                {
                    m_HelpRequested = true;
                    continue;
                }

                if (!(option->m_flags & cmd::needsarg))
                {
                    option->m_result = "true";
                    if (option->m_flags & cmd::shared_val)
                    {
                        if (m_sharedvalue == tt::npos)
                            m_sharedvalue = option->m_setvalue;
                        else
                            m_sharedvalue |= option->m_setvalue;
                    }
                    continue;
                }

                // If we get here, it means the option wants to store the next argument

                ++argpos;
                if (argpos >= m_originalArgs.size())
                {
                    m_results.emplace_back(Result::noarg);
                    result = false;
                    break;
                }

                arg = m_originalArgs[argpos].subview();

#if defined(_WIN32)
                if (arg.empty() || arg.at(0) == '-' || arg.at(0) == '/')
#else
                if (arg.empty() || arg[0] == '-')
#endif  // _WIN32
                {
                    m_results.emplace_back(Result::invalid_arg);
                    result = false;
                    --argpos;
                    continue;
                }

                option->m_result.assign(arg);
            }
            else
            {
                // indicate the argument doesn't match any option supplied to us
                m_results.emplace_back(Result::unknown_opt);
                result = false;
            }
        }
        else
        {
            if (arg.at(0) == '"')
            {
                auto entry = m_extras.emplace_back();
                entry.ExtractSubString(arg);
            }
            else
            {
                m_extras.emplace_back(arg);
            }
        }
    }

    // Verify that all required options have been seen. Note that boolean options will have set m_result to "true" which
    // is why we can check for empty()

    for (auto entry = m_options.begin(); entry != m_options.end(); ++entry)
    {
        if (entry->second->m_flags & cmd::required && entry->second->m_result.empty())
        {
            m_results.emplace_back(Result::missing);
            return false;
        }
    }

    return result;
}

std::vector<ttlib::cstr> cmd::getUsage()
{
    std::vector<ttlib::cstr> usage;

    size_t maxSize = 0;
    for (auto option = m_options.begin(); option != m_options.end(); ++option)
    {
        if (option->first.size() > maxSize)
            maxSize = option->first.size();
    }

    ++maxSize;

    ttlib::cstr format;
    format.Format("    -%%-%ds  %%s", maxSize);

    for (auto option = m_options.begin(); option != m_options.end(); ++option)
    {
        if (option->second->m_flags & cmd::hidden)
            continue;
        auto& entry = usage.emplace_back();
        entry.Format(format, option->first.c_str(), option->second->m_description.c_str());
    }

    return usage;
}

#if defined(_WIN32)

void cmd::WinInit()
{
    int argc;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    for (auto argpos = 1; argpos < argc; ++argpos)
    {
        auto& arg = m_originalArgs.emplace_back();
        arg.from_utf16(argv[argpos]);
    }
    LocalFree(argv);
}

#endif  // _WIN32
