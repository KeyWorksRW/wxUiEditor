/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../../pugixml/pugixml.hpp"

class Node;

class ImportXML
{
public:
    pugi::xml_document& GetDocument() { return m_docOut; }
    virtual bool Import(const ttString& filename) = 0;

protected:
    void HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent = nullptr);

    pugi::xml_document m_docOut;
};
