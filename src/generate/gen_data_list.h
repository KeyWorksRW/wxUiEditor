/////////////////////////////////////////////////////////////////////////////
// Purpose:   Data List generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxStaticBitmap;
class wxStaticText;

class DataGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    int GetRequiredVersion(Node* /*node*/) override;

private:
    wxStaticText* m_item_name;
    wxStaticText* m_text_info;
};

class DataStringGenerator : public BaseGenerator
{
public:
};

class DataXmlGenerator : public BaseGenerator
{
public:
};

namespace data_list
{
    Node* FindDataList();

};  // namespace data_list
