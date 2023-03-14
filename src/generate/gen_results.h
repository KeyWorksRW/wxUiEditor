/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code generation file writing functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace pugi
{
    class xml_node;
}

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    #include <chrono>
#endif

struct GenResults
{
    size_t file_count { 0 };
    std::vector<tt_string> msgs;
    std::vector<tt_string> updated_files;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    std::chrono::steady_clock::time_point start_time;
    size_t elapsed;

    void StartClock() { start_time = std::chrono::steady_clock::now(); }

    void EndClock()
    {
        auto end_time = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        tt_string msg;
        msg << "Elapsed time: " << elapsed << " milliseconds";
        msgs.emplace_back(msg);
    }
#else
    void StartClock() {}
    void EndClock() {}
#endif
};

// If pClassList is non-null, it must contain the base class name of every form that needs
// updating.
//
// ../generate/gen_codefiles.cpp
bool GenerateCodeFiles(GenResults& results, std::vector<tt_string>* pClassList = nullptr);

// ../generate/gen_codefiles.cpp
void GenInhertedClass(GenResults& results);

// ../generate/gen_python.cpp
bool GeneratePythonFiles(GenResults& results, std::vector<tt_string>* pClassList = nullptr);

// If out_file contains a file, it will override project xrc_file and combine_xrc settings.
//
// If NeedsGenerateCheck is true, this will not write any files, but will return true if at
// least one file needs to be generated.
//
// If pClassList is non-null, it will contain the base class name of every form that needs
// updating.
//
// ../generate/gen_xrc.cpp
bool GenerateXrcFiles(GenResults& results, tt_string out_file = {}, std::vector<tt_string>* pClassList = nullptr);

#if defined(INTERNAL_TESTING)
void GenerateTmpFiles(const std::vector<tt_string>& ClassList, pugi::xml_node root, int language = GEN_LANG_CPLUSPLUS);
#endif
