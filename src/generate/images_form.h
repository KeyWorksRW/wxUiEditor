/////////////////////////////////////////////////////////////////////////////
// Purpose:   Embedded images generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxStaticBitmap;
class wxStaticText;

class ImagesGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    void OnImageSelected();

private:
    wxStaticBitmap* m_bitmap;
    wxStaticText* m_text_info;
};

class EmbeddedImageGenerator : public BaseGenerator
{
public:
};
