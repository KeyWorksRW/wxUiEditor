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
    int GetRequiredVersion(Node* /*node*/) override;

    bool ModifyProperty(NodeProperty* prop, tt_string_view value) override;

private:
    wxStaticBitmap* m_bitmap;
    wxStaticText* m_image_name;
    wxStaticText* m_text_info;
};

class EmbeddedImageGenerator : public BaseGenerator
{
public:
};

void GatherImages(Node* parent, std::set<std::string>& images, std::vector<std::string>& new_images);
