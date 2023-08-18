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
void GenerateSingleBitmapCode(Code& code, const tt_string& description);
