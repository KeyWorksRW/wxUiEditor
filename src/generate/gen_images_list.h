/////////////////////////////////////////////////////////////////////////////
// Purpose:   Images List Embedded images generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
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

    bool modifyProperty(NodeProperty* prop, tt_string_view value) override;

private:
    wxStaticBitmap* m_bitmap;
    wxStaticText* m_image_name;
    wxStaticText* m_text_info;
};

class EmbeddedImageGenerator : public BaseGenerator
{
public:
    bool AllowPropertyChange(wxPropertyGridEvent*, NodeProperty*, Node*) override;
};

namespace img_list
{
    void GatherImages(Node* parent, std::set<std::string>& images, std::vector<std::string>& new_images);

    // Ensure any Images List is in the first position and that all it's children are sorted.
    void UpdateImagesList(int ProjectVersion);

    // Used to compare prop_bitmap descriptions -- this can be used in a sort() function
    bool CompareImageNames(NodeSharedPtr a, NodeSharedPtr b);

    Node* FindImageList();

    void FixPropBitmap(Node* parent);

};  // namespace img_list
