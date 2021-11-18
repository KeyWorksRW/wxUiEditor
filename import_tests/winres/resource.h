//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by main_test.rc
//
#define IDM_HELPKEYS                    0x001E

/* Menu Command Defines */
#define IDM_FILE            0
#define IDM_OPEN            101
#define IDM_PRINT           102
#define IDM_ASSOCIATE       103
#define IDM_SEARCH          104
#define IDM_RUN             105
#define IDM_MOVE            106
#define IDM_COPY            107
#define IDM_DELETE          108
#define IDM_RENAME          109
#define IDM_ATTRIBS         110
#define IDM_MAKEDIR         111
#define IDM_SELALL          112
#define IDM_DESELALL        113
#define IDM_UNDO            114
#define IDM_EXIT            115
#define IDM_SELECT          116
#define IDM_UNDELETE        117
#define IDM_COPYTOCLIPBOARD 118
#define IDM_COMPRESS        119
#define IDM_UNCOMPRESS      120
#define IDM_PASTE           121
#define IDM_EDIT            122
#define IDM_CUTTOCLIPBOARD  123
#define IDM_STARTCMDSHELL   124
#define IDM_GOTODIR         125
#define IDM_HISTORYBACK     126
#define IDM_HISTORYFWD      127
#define IDM_STARTPOWERSHELL 128
#define IDM_STARTBASHSHELL  129
#define IDM_CLOSEWINDOW     130

// This IDM_ is reserved for IDH_GROUP_ATTRIBS
#define IDM_GROUP_ATTRIBS   199

#define IDM_DISK            1
#define IDM_DISKCOPY        201
#define IDM_LABEL           202
#define IDM_FORMAT          203
//#define IDM_SYSDISK         204
#define IDM_CONNECT         205
#define IDM_DISCONNECT      206
#define IDM_DRIVESMORE      251
#define IDM_CONNECTIONS     252
#define IDM_SHAREDDIR       253

// AS added to end
#define IDM_SHAREAS         254
#define IDM_STOPSHARE       255

#define IDM_TREE            2
#define IDM_EXPONE          301
#define IDM_EXPSUB          302
#define IDM_EXPALL          303
#define IDM_COLLAPSE        304
#define IDM_NEWTREE         305

#define IDM_VIEW            3
#define IDM_VNAME           401
#define IDM_VDETAILS        402
#define IDM_VOTHER          403

#define IDM_BYNAME          404
#define IDM_BYTYPE          405
#define IDM_BYSIZE          406
#define IDM_BYDATE          407    // reverse date sort
#define IDM_BYFDATE         408    // forward date sort

#define IDM_VINCLUDE        409
#define IDM_REPLACE         410

#define IDM_TREEONLY        411
#define IDM_DIRONLY         412
#define IDM_BOTH            413
#define IDM_SPLIT           414

#define IDM_ESCAPE          420

#define IDM_OPTIONS         4
#define IDM_CONFIRM         501
#define IDM_LOWERCASE       502
#define IDM_STATUSBAR       503
#define IDM_MINONRUN        504
#define IDM_ADDPLUSES       505
#define IDM_EXPANDTREE      506

#define IDM_DRIVEBAR      507   /* Options->Drivebar */
#define IDM_TOOLBAR     508   /* Options->Toolbar */
#define IDM_NEWWINONCONNECT 509  /* Options->New Window On Connect */

#define IDM_FONT            510
#define IDM_SAVESETTINGS    511

#define IDM_TOOLBARCUST     512

#ifdef PROGMAN
#define IDM_SAVENOW         513
#endif

#define IDM_INDEXONLAUNCH   514

#define IDM_PREF          515

#define IDM_SECURITY        5
#define IDM_PERMISSIONS     605      // !! WARNING HARD CODED !!
#define IDM_AUDITING        606
#define IDM_OWNER           607

#define IDM_EXTENSIONS      6
// iExt 0, command ids 700-799
// iExt 1, command ids 800-899
// etc.
// iExt 9, command ids 1600-1699

// NOTE: IDM_WINDOW is IDM_EXTENSIONS (6) + MAX_EXTENSIONS (10)
#define IDM_WINDOW           16
#define IDM_CASCADE          1701
#define IDM_TILE             1702

#define IDM_TILEHORIZONTALLY 1703
#define IDM_REFRESH          1704
#define IDM_ARRANGE          1705
#define IDM_NEWWINDOW        1706
#define IDM_CHILDSTART       1707

#define IDM_HELP            17
#define IDM_HELPINDEX       1801
#define IDM_HELPKEYS        0x001E
#define IDM_HELPCOMMANDS    0x0020
#define IDM_HELPPROCS       0x0021
#define IDM_HELPHELP        1802
#define IDM_ABOUT           1803

#define IDM_DRIVELISTJUMP 2000  /* for defining an accelerator */

#define IDD_DIALOG1                     101
#define IDD_BLANK                       101
#define IDM_OPEN                        101
#define IDD_MAINTEST                    102
#define IDM_PRINT                       102
#define IDM_ASSOCIATE                   103
#define IDM_SEARCH                      104
#define IDM_RUN                         105
#define IDM_MOVE                        106
#define IDM_COPY                        107
#define IDM_DELETE                      108
#define IDM_RENAME                      109
#define IDM_ATTRIBS                     110
#define IDM_MAKEDIR                     111
#define IDM_EXIT                        115
#define IDM_SELECT                      116
#define IDM_COPYTOCLIPBOARD             118
#define IDM_COMPRESS                    119
#define IDM_UNCOMPRESS                  120
#define IDM_PASTE                       121
#define IDM_EDIT                        122
#define IDM_CUTTOCLIPBOARD              123
#define IDM_STARTCMDSHELL               124
#define IDM_GOTODIR                     125
#define IDD_MAINDLG                     127
#define IDM_STARTPOWERSHELL             128
#define IDM_STARTBASHSHELL              129
#define IDM_DISKCOPY                    201
#define IDM_LABEL                       202
#define IDM_FORMAT                      203
#define IDM_DRIVESMORE                  251
#define IDM_EXPONE                      301
#define IDM_EXPSUB                      302
#define IDM_EXPALL                      303
#define IDM_COLLAPSE                    304
#define IDM_VNAME                       401
#define IDM_VDETAILS                    402
#define IDM_VOTHER                      403
#define IDM_BYNAME                      404
#define IDM_BYTYPE                      405
#define IDM_BYSIZE                      406
#define IDM_BYDATE                      407
#define IDM_BYFDATE                     408
#define IDM_VINCLUDE                    409
#define IDM_TREEONLY                    411
#define IDM_DIRONLY                     412
#define IDM_BOTH                        413
#define IDM_SPLIT                       414
#define FRAMEMENU                       500
#define IDM_CONFIRM                     501
#define IDM_STATUSBAR                   503
#define IDM_MINONRUN                    504
#define IDM_ADDPLUSES                   505
#define IDM_DRIVEBAR                    507
#define IDM_TOOLBAR                     508
#define IDM_FONT                        510
#define IDM_SAVESETTINGS                511
#define IDM_TOOLBARCUST                 512
#define IDM_INDEXONLAUNCH               514
#define IDM_PREF                        515
#define IDM_PERMISSIONS                 605
#define IDM_AUDITING                    606
#define IDM_OWNER                       607
#define IDEDIT_CLASS                    1000
#define IDC_EDIT1                       1000
#define IDEDIT_FILE                     1001
#define IDLIST_MEMBERS                  1004
#define IDBTN_ADD                       1005
#define IDBTN_REMOVE                    1006
#define IDCHECK_ICONS                   1007
#define IDCHECK_METHOD_INITIALIZE       1008
#define IDCHECK_METHOD_CLOSE            1009
#define IDRADIO_TYPE_CTTDLG             1010
#define IDRADIO_TYPE_CAXDIALOGIMPL      1011
#define IDRADIO_TYPE_CDIALOGIMPL        1012
#define IDCHECK_DISP_EVENT              1013
#define IDCHECK_CENTER_WINDOW           1014
#define IDCHECK_SHADED_BUTTONS          1015
#define IDRADIO_TYPE_PROPERTY           1016
#define IDCHECK_METHOD_CANCEL           1017
#define IDCHECK_CONSTRUCTOR             1020
#define IDCHECK_DESTRUCTOR              1021
#define IDBTN_EDIT                      1033
#define IDTXT_CUR_DIR                   1034
#define IDC_CHECK1                      1038
#define IDC_CHECK2                      1039
#define IDC_CHECK3                      1040
#define IDC_CHECK4                      1041
#define IDC_COMBO1                      1042
#define IDCOMBO_DLG_IDS                 1042
#define IDCOMBO_RC_FILES                1043
#define IDCHECK_RESIZABLE_CDIALOG       1045
#define IDCHECK_RESIZABLE_CAXDIALOG     1046
#define IDE_COLNAME                     1297
#define IDC_COLLIST                     1370
#define IDB_ADDCOL                      1372
#define IDB_DELCOL                      1373
#define IDM_CASCADE                     1701
#define IDM_TILE                        1702
#define IDM_TILEHORIZONTALLY            1703
#define IDM_REFRESH                     1704
#define IDM_ARRANGE                     1705
#define IDM_NEWWINDOW                   1706
#define IDM_HELPINDEX                   1801
#define IDM_HELPHELP                    1802
#define IDM_ABOUT                       1803
#define IDD_COLUMNEDIT                  2020
#define IDC_COL_INITNUM_EDIT            2021
#define IDC_COL_INCREASENUM_EDIT        2022
#define IDC_COL_TEXT_GRP_STATIC         2023
#define IDC_COL_DEC_RADIO               2024
#define IDC_COL_OCT_RADIO               2025
#define IDC_COL_HEX_RADIO               2026
#define IDC_COL_BIN_RADIO               2027
#define IDC_COL_TEXT_RADIO              2028
#define IDC_COL_NUM_RADIO               2029
#define IDC_COL_INITNUM_STATIC          2030
#define IDC_COL_INCRNUM_STATIC          2031
#define IDC_COL_FORMAT_GRP_STATIC       2032
#define IDC_COL_NUM_GRP_STATIC          2033
#define IDC_COL_TEXT_EDIT               2034
#define IDC_COL_LEADZERO_CHECK          2035
#define IDC_COL_REPEATNUM_STATIC        2036
#define IDC_COL_REPEATNUM_EDIT          2037
#define ID_Menu                         40001

// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        104
#define _APS_NEXT_COMMAND_VALUE         40002
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
