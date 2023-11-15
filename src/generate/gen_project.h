/////////////////////////////////////////////////////////////////////////////
// Purpose:   Project generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class ProjectGenerator : public BaseGenerator
{
public:
    bool PopupMenuAddCommands(NavPopupMenu*, Node*) override;

    // Don't change versions in a debug build -- it makes it easier to test the version
    // checking code.

#if !defined(_DEBUG)
    // There are so many additions to the 1.1 version of wxUiEditor that it's easier
    // just to increase the Project's version number.
    int GetRequiredVersion(Node* /*node*/) override { return minRequiredVer + 1; }
#endif  // _DEBUG

    bool AllowPropertyChange(wxPropertyGridEvent*, NodeProperty*, Node*) override;
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
