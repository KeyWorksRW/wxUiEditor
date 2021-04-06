/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains translatable strings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <map>

#include "strings.h"

const std::map<int, const char*> englishStrings = {

    { strIdCantReadFile, "Unable to read the file " },
    { strIdTitleCodeGeneration, "Code Generation" },
    { strIdNoBmpURL, "You cannot use a URL for a bitmap property: " },
    { strIdInternalError, "An internal error has occurred: " },
    { strIdClassesGenerated, "Class(es) generated to " },
    { strIdTitleGenerateClasses, "Generate Classes" },
    { strIdCannotLoadProjectFile, "Cannot load project file: " },
    { strIdGenerateMissingFile, "-generate must be followed by a project filename" },
    { strIdTitleLoadProject, "Load Project" },
    { strIdCantOpen, "Cannot open " },
    { strIdTitleAbout, "About wxUiEditor" },
    { strIdOK, "OK" },
    { strIdCmdDisplayHelp, "display this help message" },
    { strIdStatusProjectLoaded, "Project loaded" },
    { strIdCantWrite, "Cannot create or write to the file " },
    { strIdCancel, "Cancel" },
    { strIdNoRecentFiles, "There are no recently opened projects." },
    { strIdProjectMRUQuery,
      "The project file '%s' doesn't exist.\n\nWould you like to remove it from the recent files list?" },
    { strIdTitleOpenMRU, "Open recent project" },
    { strIdTitleSaveAs, "Save Project As" },
    { strIdTitleOpenOrImport, "Open or Import Project" },
    { strIdTitleGenerators, "Generators" },
    { strIdTitlePropEventPanel, "Node Configuration" },
    { strIdTitleEditor, "Editor" },
    { strIdSaved, " saved" },
    { strIdProjectName, "Project: " },
    { strIdMockupTabTitle, "Mock Up" },
    { strIdCantMoveUp, "This component cannot be moved up any further." },
    { strIdCantMoveLeft, "This component cannot be moved left any further." },
    { strIdMoveTitle, "Move item" },
    { strIdTitlePreferences, "Preferences" },
    { strIdClose, "Close" },
    { strIdSize, "Size: " },
    { stdImportFile, "Import file" },
    { strIdImportFormBuilder, "Import wxFormBuilder project" },
    { strIdUndo, "Undo" },
    { strIdRedo, "Redo" },
    { strIdPaste, "Paste" },
    { strId_tb_in_sizer, "For the toolbar to be owned by the frame window, it should be placed directly under the frame, "
                         "not under a sizer. Use Cut and Paste to move it." },

};

const std::map<int, const char*>* _tt_english = &englishStrings;
const std::map<int, const char*>* _tt_CurLanguage = &englishStrings;
