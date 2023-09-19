/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for generating embedded images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <vector>

class Code;
struct EmbeddedImage;

std::vector<std::string> base64_encode(unsigned char const* data, size_t data_size, int language = GEN_LANG_PYTHON);
void AddPythonImageName(Code& code, const EmbeddedImage* embed);

// Primarily designed for wxRibbon which doesn't support wxBitmapBundle.
// This will generate the code to load a single wxBitmap.
void GenerateRibbonBitmapCode(Code& code, const tt_string& description);

// Generates the code to load a wxBitmapBundle.
//
// If get_bitmap is true, a bitmap will be returned. The bitmap will be scaled to the current
// DPI using Rescale for a single bitmap, and wxBitmapBundle::GetBitmap() otherwise.
void GenerateBundleParameter(Code& code, const tt_string& description, bool get_bitmap = false);

// Generates the code to load a wxBitmapBundle.
//
// If get_bitmap is true, a bitmap will be returned. The bitmap will be scaled to the current
// DPI using Rescale for a single bitmap, and wxBitmapBundle::GetBitmap() otherwise.
void GenerateBundleParameter(Code& code, const tt_string_vector& parts, bool get_bitmap = false);
