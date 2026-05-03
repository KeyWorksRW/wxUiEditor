#include <algorithm>
#include <cstring>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <print>

#include "buffer.hxx"
#include "cmark_ctype.hxx"

void CMarkStringBuffer::Init(size_t initial_size)
{
    m_buffer.clear();
    if (initial_size > 0)
    {
        m_buffer.reserve(initial_size);
    }
}

void CMarkStringBuffer::Grow(size_t target_size)
{
    assert(target_size > 0);

    const size_t current_asize = m_buffer.capacity();
    if (target_size <= current_asize)
    {
        return;
    }

    if (target_size > static_cast<size_t>(INT32_MAX / 2))
    {
        std::print(stderr,
                   "[cmark] CMarkStringBuffer::Grow requests buffer with size > {:L}, aborting\n",
                   (INT32_MAX / 2));
        abort();
    }

    // Oversize the buffer by 50% to guarantee amortized linear time complexity on
    // append operations.
    size_t new_size = target_size + (target_size / 2);
    ++new_size;
    new_size = (new_size + 7) & ~7;

    m_buffer.reserve(new_size);
}

void CMarkStringBuffer::Free()
{
    m_buffer.clear();
    m_buffer.shrink_to_fit();
}

void CMarkStringBuffer::Clear()
{
    m_buffer.clear();
}

void CMarkStringBuffer::Set(const char* data, size_t length)
{
    if (length == 0 || data == nullptr)
    {
        Clear();
        return;
    }

    m_buffer.assign(data, length);
}

void CMarkStringBuffer::Sets(const char* string)
{
    if (string != nullptr)
    {
        m_buffer.assign(string);
    }
    else
    {
        m_buffer.clear();
    }
}

void CMarkStringBuffer::Putc(int character)
{
    m_buffer.push_back(static_cast<char>(character & 0xFF));
}

void CMarkStringBuffer::Put(const unsigned char* data, size_t length)
{
    if (length == 0 || data == nullptr)
    {
        return;
    }

    m_buffer.append(reinterpret_cast<const char*>(data), length);
}

void CMarkStringBuffer::Put(std::string_view string)
{
    if (string.empty())
    {
        return;
    }

    m_buffer.append(string);
}

void CMarkStringBuffer::Puts(const char* string)
{
    if (string != nullptr)
    {
        m_buffer.append(string);
    }
}

void CMarkStringBuffer::CopyCStr(char* data, size_t data_size) const
{
    if (data == nullptr || data_size == 0)
    {
        return;
    }

    data[0] = '\0';

    if (m_buffer.empty())
    {
        return;
    }

    const size_t copy_length = std::min(m_buffer.size(), data_size - 1);
    std::memmove(data, m_buffer.data(), copy_length);
    data[copy_length] = '\0';
}

void CMarkStringBuffer::Swap(CMarkStringBuffer& other)
{
    m_buffer.swap(other.m_buffer);
}

std::string CMarkStringBuffer::Detach()
{
    std::string result;
    m_buffer.swap(result);
    return result;
}

int CMarkStringBuffer::Cmp(const CMarkStringBuffer& other) const
{
    const int result = std::memcmp(m_buffer.data(), other.m_buffer.data(),
                                   std::min(m_buffer.size(), other.m_buffer.size()));
    if (result != 0)
    {
        return result;
    }
    if (m_buffer.size() < other.m_buffer.size())
    {
        return -1;
    }
    if (m_buffer.size() > other.m_buffer.size())
    {
        return 1;
    }
    return 0;
}

size_t CMarkStringBuffer::Strchr(int character, size_t position) const
{
    if (position >= m_buffer.size())
    {
        return static_cast<size_t>(-1);
    }

    const char* found_ptr = static_cast<const char*>(
        std::memchr(m_buffer.data() + position, character, m_buffer.size() - position));
    if (found_ptr == nullptr)
    {
        return static_cast<size_t>(-1);
    }

    return static_cast<size_t>(found_ptr - m_buffer.data());
}

size_t CMarkStringBuffer::Strrchr(int character, size_t position) const
{
    if (m_buffer.empty())
    {
        return static_cast<size_t>(-1);
    }

    if (position >= m_buffer.size())
    {
        position = m_buffer.size() - 1;
    }

    for (size_t index = position + 1; index > 0; --index)
    {
        const size_t real_index = index - 1;
        if (static_cast<unsigned char>(m_buffer[real_index]) ==
            static_cast<unsigned char>(character))
        {
            return real_index;
        }
    }

    return static_cast<size_t>(-1);
}

void CMarkStringBuffer::Truncate(size_t length)
{
    if (length < m_buffer.size())
    {
        m_buffer.resize(length);
    }
}

void CMarkStringBuffer::Drop(size_t count)
{
    if (count == 0)
    {
        return;
    }

    if (count > m_buffer.size())
    {
        count = m_buffer.size();
    }

    m_buffer.erase(0, count);
}

void CMarkStringBuffer::Rtrim()
{
    while (!m_buffer.empty())
    {
        if (!cmark_isspace(static_cast<unsigned char>(m_buffer.back())))
        {
            break;
        }

        m_buffer.pop_back();
    }
}

void CMarkStringBuffer::Trim()
{
    if (m_buffer.empty())
    {
        return;
    }

    size_t index = 0;
    while (index < m_buffer.size() && cmark_isspace(static_cast<unsigned char>(m_buffer[index])))
    {
        ++index;
    }

    if (index > 0)
    {
        m_buffer.erase(0, index);
    }

    Rtrim();
}

// Destructively modify string, collapsing consecutive
// space and newline characters into a single space.
void CMarkStringBuffer::NormalizeWhitespace()
{
    bool last_char_was_space = false;
    size_t write_index = 0;

    for (char ch: m_buffer)
    {
        if (cmark_isspace(static_cast<unsigned char>(ch)))
        {
            if (!last_char_was_space)
            {
                m_buffer[write_index++] = ' ';
                last_char_was_space = true;
            }
        }
        else
        {
            m_buffer[write_index++] = ch;
            last_char_was_space = false;
        }
    }

    Truncate(write_index);
}

// Destructively unescape a string: remove backslashes before punctuation chars.
void CMarkStringBuffer::Unescape()
{
    size_t write_index = 0;

    for (size_t read_index = 0; read_index < m_buffer.size(); ++read_index)
    {
        if (m_buffer[read_index] == '\\' &&
            cmark_ispunct(static_cast<unsigned char>(m_buffer[read_index + 1])))
        {
            ++read_index;
        }

        m_buffer[write_index++] = m_buffer[read_index];
    }

    Truncate(write_index);
}
