/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import interface
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// All import classes should inherit from this class. Each class should parse the filename passed to the Import() method and
// convert it into wxUiEditor XML project document (stored in memory). If the function returns true, then the call will call
// GetDocument() to access the XML tree.

#pragma once

#include "../../pugixml/pugixml.hpp"

class Node;

// Functions common to different Import classes
void HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent = nullptr);

class ImportInterface
{
public:
    virtual bool Import(const ttString& filename) = 0;
    virtual pugi::xml_document& GetDocument() = 0;
};
