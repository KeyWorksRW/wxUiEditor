/////////////////////////////////////////////////////////////////////////////
// Purpose:   Smart Numeric Casts
// Author:    John Morrison Leon
// Article:
// https://www.codeproject.com/Articles/5329780/Smart-Numeric-Casts-to-End-the-Agony-of-int-or-sta
/////////////////////////////////////////////////////////////////////////////

// These classes are based on the work by John Leon in the article cited above.

#pragma once

// equivalent to static_cast<const int>
class to_int
{
    const unsigned long long v;
    to_int(to_int const& _value) = delete;  // prevent copying

public:
    explicit to_int(unsigned char _value) : v(_value) {}
    explicit to_int(unsigned short _value) : v(_value) {}
    explicit to_int(unsigned int _value) : v(_value) {}
    explicit to_int(unsigned long _value) : v(_value) {}
    explicit to_int(unsigned long long _value) : v(_value) {}

    operator int() const noexcept
    {
        ASSERT_MSG(static_cast<const int>(v) == -1 || v <= INT_MAX,
                   "value is too large to convert to int")
        return static_cast<const int>(v);
    }
};

// equivalent to static_cast<const size_t>
class to_size_t
{
    const long long v;
    to_size_t(to_size_t const& _value) = delete;  // prevent copying

public:
    explicit to_size_t(char _value) : v(_value) {}
    explicit to_size_t(short _value) : v(_value) {}
    explicit to_size_t(int _value) : v(_value) {}
    explicit to_size_t(long _value) : v(_value) {}
    explicit to_size_t(long long _value) : v(_value) {}
    explicit to_size_t(uint64_t _value) : v(static_cast<long long>(_value)) {}

    operator size_t() const noexcept { return static_cast<const size_t>(v); }
};

class to_char
{
    const long long v;
    to_char(to_char const& _value) = delete;  // prevent copying

public:
    explicit to_char(short _value) : v(_value) {}
    explicit to_char(unsigned short _value) : v(_value) {}
    explicit to_char(int _value) : v(_value) {}
    explicit to_char(unsigned int _value) : v(_value) {}
    explicit to_char(long _value) : v(_value) {}
    explicit to_char(unsigned long _value) : v(_value) {}
    explicit to_char(long long _value) : v(_value) {}
    explicit to_char(unsigned long long _value) : v(static_cast<long long>(_value)) {}

    operator char() const noexcept
    {
        ASSERT_MSG(v <= UCHAR_MAX, "value is too large to convert to char")
        return static_cast<const char>(v);
    }
};

class to_uchar
{
    const unsigned long long v;
    to_uchar(to_uchar const& _value) = delete;  // prevent copying

public:
    explicit to_uchar(char _value) : v(_value) {}
    explicit to_uchar(short _value) : v(_value) {}
    explicit to_uchar(unsigned short _value) : v(_value) {}
    explicit to_uchar(int _value) : v(_value) {}
    explicit to_uchar(unsigned int _value) : v(_value) {}
    explicit to_uchar(long _value) : v(_value) {}
    explicit to_uchar(unsigned long _value) : v(_value) {}
    explicit to_uchar(long long _value) : v(_value) {}
    explicit to_uchar(unsigned long long _value) : v(_value) {}

    operator unsigned char() const noexcept
    {
        ASSERT_MSG(v <= UCHAR_MAX, "value is too large to convert to char")
        return static_cast<const unsigned char>(v);
    }
};
