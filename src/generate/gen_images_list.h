/////////////////////////////////////////////////////////////////////////////
// Purpose:   Images List Embedded images generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxStaticBitmap;
class wxStaticText;

class ImagesGenerator : public BaseGenerator
{
public:
    auto CreateMockup(Node* /* node */, wxObject* wxobject) -> wxObject* override;
    auto GetRequiredVersion(Node* /*node*/) -> int override;

private:
    wxStaticBitmap* m_bitmap;
    wxStaticText* m_image_name;
    wxStaticText* m_text_info;
};

class EmbeddedImageGenerator : public BaseGenerator
{
public:
    [[nodiscard]] auto AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop,
                                           Node* node) -> bool override;
};

namespace img_list
{
    void GatherImages(Node* parent, std::set<std::string>& images,
                      std::vector<std::string>& new_images);

    // Ensure any Images List is in the first position and that all it's children are sorted.
    void UpdateImagesList(int ProjectVersion);

    // Used to compare prop_bitmap descriptions -- this can be used in a sort() function
    [[nodiscard]] auto CompareImageNames(const NodeSharedPtr& lhs, const NodeSharedPtr& rhs) -> bool;

    [[nodiscard]] auto FindImageList() -> Node*;

    void FixPropBitmap(Node* parent);

};  // namespace img_list
