/////////////////////////////////////////////////////////////////////////////
// Purpose:   Forward defintions of Node classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This header file can be included in header files that either don't need immediate access to the various Node classes,
// or for the Node class header files to avoid recursive #includes to get the required class names.

#pragma once

class Node;
class NodeEvent;
class NodeEventInfo;
class NodeDeclaration;
class NodeProperty;

class BaseGenerator;
class NodeCreator;
class NodeCategory;
class PropertyInfo;

using NodeSharedPtr = std::shared_ptr<Node>;
