/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for generating embedded images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <vector>

class Code;
class EmbeddedImage;

std::vector<std::string> base64_encode(unsigned char const* data, size_t data_size,
                                       GenLang language = GEN_LANG_PYTHON);

// Note: GenerateBundleParameter is now a Code class method.
// See code.h for the declaration.
