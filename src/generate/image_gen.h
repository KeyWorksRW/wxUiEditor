/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for generating embedded images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <vector>

class Code;
struct EmbeddedImage;

std::vector<std::string> base64_encode(unsigned char const* data, size_t data_size);
void AddPythonImageName(Code& code, const EmbeddedImage* embed);
