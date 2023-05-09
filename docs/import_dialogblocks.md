# Importing DialogBlocks projects

wxUiEditor can only import a DialogBlocks project if it was saved as an XML file rather than in a binary format. You can change how the project file is saved using the following steps:

1) Open the project in DialogBlocks and select the project
2) Either click the Settings toolbar button or choose the Settings command from the View menu
3) In the Settings dialog, select Project and check "Save as XML"
4) Click OK and then click the Save toolbar button or the Save Project command from the File menu

As an alternative, you can also use DialogBlocks to export your project as an XRC file. However, the XRC will usually contain less information and the wxUiEditor-generated code may not work the same as the DialogBlocks-generated code.
