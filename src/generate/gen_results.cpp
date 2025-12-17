/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code generation file writing functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <format>

#include "gen_results.h"

void GenResults::StartClock()
{
    m_start_time = std::chrono::steady_clock::now();
}

void GenResults::EndClock()
{
    auto end_time = std::chrono::steady_clock::now();
    m_elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_start_time).count();
    m_msgs.emplace_back(std::format("Elapsed time: {} milliseconds", m_elapsed));
}

void GenResults::Clear()
{
    m_elapsed = 0;
    m_file_count = 0;
    m_msgs.clear();
    m_updated_files.clear();
}
