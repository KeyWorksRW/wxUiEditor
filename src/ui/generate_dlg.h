/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for choosing and generating specific language file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../wxui/generatedlg_base.h"  // auto-generated: ../wxui/generate_dlg_base.h and ../wxui/generate_dlg_base.cpp

struct GenResults
{
    size_t file_count { 0 };
    std::vector<ttlib::cstr> msgs;
    std::vector<ttlib::cstr> updated_files;
};

// If pClassList is non-null, it must contain the base class name of every form that needs
// updating.
//
// ../generate/gen_codefiles.cpp
bool GenerateCodeFiles(GenResults& results, std::vector<ttlib::cstr>* pClassList = nullptr);

// ../generate/gen_codefiles.cpp
void GenInhertedClass(GenResults& results);

// ../generate/gen_python.cpp
bool GeneratePythonFiles(GenResults& results, std::vector<ttlib::cstr>* pClassList = nullptr);

// ../generate/gen_lua.cpp
bool GenerateLuaFiles(GenResults& results, std::vector<ttlib::cstr>* pClassList = nullptr);

// ../generate/gen_php.cpp
bool GeneratePhpFiles(GenResults& results, std::vector<ttlib::cstr>* pClassList = nullptr);

// If out_file contains a file, it will override project xrc_file and combine_xrc settings.
//
// If NeedsGenerateCheck is true, this will not write any files, but will return true if at
// least one file needs to be generated.
//
// If pClassList is non-null, it will contain the base class name of every form that needs
// updating.
//
// ../generate/gen_xrc.cpp
bool GenerateXrcFiles(GenResults& results, ttlib::cstr out_file = {}, std::vector<ttlib::cstr>* pClassList = nullptr);

#if defined(INTERNAL_TESTING)
void GenerateTmpFiles(const std::vector<ttlib::cstr>& ClassList, pugi::xml_node root);
#endif

class GenerateDlg : public GenerateDlgBase
{
public:
    GenerateDlg();  // If you use this constructor, you must call Create(parent)
    GenerateDlg(wxWindow* parent);

protected:
    void OnInit(wxInitDialogEvent& event) override;
};
