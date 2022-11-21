/////////////////////////////////////////////////////////////////////////////
// Purpose:   Project generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class ProjectGenerator : public BaseGenerator
{
public:
    bool PopupMenuAddCommands(NavPopupMenu*, Node*) override;

    // There are so many additions to the 1.1 version of wxUiEditor that it's easier
    // just to increase the Project's version number.
    int GetRequiredVersion(Node* /*node*/) override { return minRequiredVer + 2; }
};

class FolderGenerator : public BaseGenerator
{
public:
    bool PopupMenuAddCommands(NavPopupMenu*, Node*) override;
};

class SubFolderGenerator : public BaseGenerator
{
public:
    bool PopupMenuAddCommands(NavPopupMenu*, Node*) override;
};
