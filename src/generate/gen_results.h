/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code generation file writing functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace pugi
{
    class xml_node;
}

#include <chrono>

class GenResults
{
public:
    void StartClock();
    void EndClock();
    void Clear();

    [[nodiscard]] auto GetFileCount() const { return m_file_count; }
    void SetFileCount(size_t count) { m_file_count = count; }
    void IncrementFileCount() { ++m_file_count; }

    [[nodiscard]] auto GetElapsed() const { return m_elapsed; }

    [[nodiscard]] auto GetMsgs() -> auto& { return m_msgs; }
    [[nodiscard]] auto GetMsgs() const -> const auto& { return m_msgs; }

    [[nodiscard]] auto GetUpdatedFiles() -> auto& { return m_updated_files; }
    [[nodiscard]] auto GetUpdatedFiles() const -> const auto& { return m_updated_files; }

private:
    size_t m_file_count { 0 };
    size_t m_elapsed { 0 };
    std::vector<std::string> m_msgs;
    std::vector<std::string> m_updated_files;
    std::chrono::steady_clock::time_point m_start_time;
};

// If pClassList is non-null, it must contain the base class name of every form that needs
// updating.
//
// ../generate/gen_codefiles.cpp
bool GenerateCppFiles(GenResults& results, std::vector<std::string>* pClassList = nullptr);

// ../generate/gen_codefiles.cpp
void GenInhertedClass(GenResults& results);

// If out_file contains a file, it will override project xrc_file and combine_xrc settings.
//
// If NeedsGenerateCheck is true, this will not write any files, but will return true if at
// least one file needs to be generated.
//
// If pClassList is non-null, it will contain the base class name of every form that needs
// updating.
//
// ../generate/gen_xrc.cpp
bool GenerateXrcFiles(GenResults& results, std::vector<std::string>* pClassList = nullptr);

void GenerateTmpFiles(const std::vector<std::string>& ClassList, pugi::xml_node root,
                      GenLang language = GEN_LANG_CPLUSPLUS);
