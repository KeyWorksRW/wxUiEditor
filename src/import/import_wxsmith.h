/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node_classes.h"

#include "import_interface.h"  // ImportInterface -- Import interface

class WxSmith : public ImportInterface
{
public:
    WxSmith();

    bool Import(const ttString& filename) override;
    pugi::xml_document& GetDocument() override { return m_docOut; }

protected:
    void ProcessContent(const pugi::xml_node& xml_obj, Node* node);
    void ProcessAttributes(const pugi::xml_node& xml_obj, Node* node);
    void ProcessBitmap(const pugi::xml_node& xml_obj, Node* node);
    void ProcessHandler(const pugi::xml_node& xml_obj, Node* node);
    void ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent = nullptr);

    NodeSharedPtr CreateXrcNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem = nullptr);

private:
    pugi::xml_document m_docOut;
};
