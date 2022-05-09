/////////////////////////////////////////////////////////////////////////////
// Purpose:   Smart Numeric Casts
// Author:    John Morrison Leon
// Article:   https://www.codeproject.com/Articles/5329780/Smart-Numeric-Casts-to-End-the-Agony-of-int-or-sta
/////////////////////////////////////////////////////////////////////////////

// These classes are based on the work by John Leon in the article cited above.

#pragma once

// equivalent to static_cast<const int>
class to_int
{
    const unsigned long long v;
    inline to_int(to_int const& _v) = delete;  // prevent copying

public:
    explicit inline to_int(unsigned char _v) : v(_v) {}
    explicit inline to_int(unsigned short _v) : v(_v) {}
    explicit inline to_int(unsigned int _v) : v(_v) {}
    explicit inline to_int(unsigned long _v) : v(_v) {}
    explicit inline to_int(unsigned long long _v) : v(_v) {}

    inline operator int() const noexcept
    {
        ASSERT_MSG(v <= INT_MAX, "value is too large to convert to int")
        return static_cast<const int>(v);
    }
};

// equivalent to static_cast<const size_t>
class to_size_t
{
    const long long v;
    inline to_size_t(to_size_t const& _v) = delete;  // prevent copying

public:
    explicit inline to_size_t(char _v) : v(_v) {}
    explicit inline to_size_t(short _v) : v(_v) {}
    explicit inline to_size_t(int _v) : v(_v) {}
    explicit inline to_size_t(long _v) : v(_v) {}
    explicit inline to_size_t(long long _v) : v(_v) {}

    inline operator size_t() const noexcept { return static_cast<const size_t>(v); }
};

class to_char
{
    const long long v;
    inline to_char(to_char const& _v) = delete;  // prevent copying

public:
    explicit inline to_char(short _v) : v(_v) {}
    explicit inline to_char(unsigned short _v) : v(_v) {}
    explicit inline to_char(int _v) : v(_v) {}
    explicit inline to_char(unsigned int _v) : v(_v) {}
    explicit inline to_char(long _v) : v(_v) {}
    explicit inline to_char(unsigned long _v) : v(_v) {}
    explicit inline to_char(long long _v) : v(_v) {}
    explicit inline to_char(unsigned long long _v) : v(_v) {}

    inline operator char() const noexcept
    {
        ASSERT_MSG(v <= UCHAR_MAX, "value is too large to convert to char")
        return static_cast<const char>(v);
    }
};

class to_uchar
{
    const unsigned long long v;
    inline to_uchar(to_uchar const& _v) = delete;  // prevent copying

public:
    explicit inline to_uchar(char _v) : v(_v) {}
    explicit inline to_uchar(short _v) : v(_v) {}
    explicit inline to_uchar(unsigned short _v) : v(_v) {}
    explicit inline to_uchar(int _v) : v(_v) {}
    explicit inline to_uchar(unsigned int _v) : v(_v) {}
    explicit inline to_uchar(long _v) : v(_v) {}
    explicit inline to_uchar(unsigned long _v) : v(_v) {}
    explicit inline to_uchar(long long _v) : v(_v) {}
    explicit inline to_uchar(unsigned long long _v) : v(_v) {}

    inline operator unsigned char() const noexcept
    {
        ASSERT_MSG(v <= UCHAR_MAX, "value is too large to convert to char")
        return static_cast<const unsigned char>(v);
    }
};
