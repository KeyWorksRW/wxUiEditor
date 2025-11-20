/////////////////////////////////////////////////////////////////////////////
// Purpose:   CodeWriter for collecting generated code in memory
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include "../../generate/writers/write_code.h"

// Simple code writer that collects code in a string buffer
class StringCodeWriter : public WriteCode
{
public:
    StringCodeWriter() { m_buffer.reserve(static_cast<size_t>(8 * 1024)); }

    void Clear() override { m_buffer.clear(); }
    [[nodiscard]] auto GetContent() const -> const std::string& { return m_buffer; }

protected:
    void doWrite(std::string_view code) override { m_buffer += code; }

private:
    std::string m_buffer;
};
