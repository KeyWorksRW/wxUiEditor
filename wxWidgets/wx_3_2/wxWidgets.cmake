set (cmn_src_dir ${CMAKE_CURRENT_LIST_DIR}/)

set (common_sources
    ../src/aui/auibar.cpp
    ../src/aui/auibook.cpp
    ../src/aui/dockart.cpp
    ../src/aui/floatpane.cpp
    ../src/aui/framemanager.cpp
    ../src/aui/tabart.cpp
    ../src/aui/tabmdi.cpp
    ../src/common/accelcmn.cpp
    ../src/common/accesscmn.cpp
    ../src/common/addremovectrl.cpp
    ../src/common/affinematrix2d.cpp
    ../src/common/anidecod.cpp
    ../src/common/animatecmn.cpp
    ../src/common/any.cpp
    ../src/common/appbase.cpp
    ../src/common/appcmn.cpp
    ../src/common/arcall.cpp
    ../src/common/arcfind.cpp
    ../src/common/archive.cpp
    ../src/common/arrstr.cpp
    ../src/common/artprov.cpp
    ../src/common/artstd.cpp
    ../src/common/arttango.cpp
    ../src/common/base64.cpp
    ../src/common/bmpbase.cpp
    ../src/common/bmpbndl.cpp
    ../src/common/bmpbtncmn.cpp
    ../src/common/bmpcboxcmn.cpp
    ../src/common/bookctrl.cpp
    ../src/common/btncmn.cpp
    ../src/common/cairo.cpp
    ../src/common/calctrlcmn.cpp
    ../src/common/checkboxcmn.cpp
    ../src/common/checklstcmn.cpp
    ../src/common/choiccmn.cpp
    ../src/common/clipcmn.cpp
    ../src/common/clntdata.cpp
    ../src/common/clrpickercmn.cpp
    ../src/common/cmdline.cpp
    ../src/common/cmdproc.cpp
    ../src/common/cmndata.cpp
    ../src/common/colourcmn.cpp
    ../src/common/colourdata.cpp
    ../src/common/combocmn.cpp
    ../src/common/config.cpp
    ../src/common/containr.cpp
    ../src/common/convauto.cpp
    ../src/common/cshelp.cpp
    ../src/common/ctrlcmn.cpp
    ../src/common/ctrlsub.cpp
    ../src/common/datavcmn.cpp
    ../src/common/datetime.cpp
    ../src/common/datetimefmt.cpp
    ../src/common/datstrm.cpp
    ../src/common/dcbase.cpp
    ../src/common/dcbufcmn.cpp
    ../src/common/dcgraph.cpp
    ../src/common/dcsvg.cpp
    ../src/common/debugrpt.cpp
    ../src/common/dircmn.cpp
    ../src/common/dirctrlcmn.cpp
    ../src/common/dlgcmn.cpp
    ../src/common/dndcmn.cpp
    ../src/common/dobjcmn.cpp
    ../src/common/docmdi.cpp
    ../src/common/docview.cpp
    ../src/common/dpycmn.cpp
    ../src/common/dseldlg.cpp
    ../src/common/dynlib.cpp
    ../src/common/dynload.cpp
    ../src/common/effects.cpp
    ../src/common/encconv.cpp
    ../src/common/event.cpp
    ../src/common/evtloopcmn.cpp
    ../src/common/fddlgcmn.cpp
    ../src/common/ffile.cpp
    ../src/common/file.cpp
    ../src/common/fileback.cpp
    ../src/common/fileconf.cpp
    ../src/common/filectrlcmn.cpp
    ../src/common/filefn.cpp
    ../src/common/filehistorycmn.cpp
    ../src/common/filename.cpp
    ../src/common/filepickercmn.cpp
    ../src/common/filesys.cpp
    ../src/common/filtall.cpp
    ../src/common/filtfind.cpp
    ../src/common/fldlgcmn.cpp
    ../src/common/fmapbase.cpp
    ../src/common/fontcmn.cpp
    ../src/common/fontdata.cpp
    ../src/common/fontenumcmn.cpp
    ../src/common/fontmap.cpp
    ../src/common/fontpickercmn.cpp
    ../src/common/fontutilcmn.cpp
    ../src/common/framecmn.cpp
    ../src/common/fs_arc.cpp
    ../src/common/fs_filter.cpp
    ../src/common/fs_inet.cpp
    ../src/common/fs_mem.cpp
    ../src/common/fswatchercmn.cpp
    ../src/common/ftp.cpp
    ../src/common/gaugecmn.cpp
    ../src/common/gbsizer.cpp
    ../src/common/gdicmn.cpp
    ../src/common/geometry.cpp
    ../src/common/gifdecod.cpp
    ../src/common/glcmn.cpp
    ../src/common/graphcmn.cpp
    ../src/common/gridcmn.cpp
    ../src/common/hash.cpp
    ../src/common/hashmap.cpp
    ../src/common/headercolcmn.cpp
    ../src/common/headerctrlcmn.cpp
    ../src/common/helpbase.cpp
    ../src/common/http.cpp
    ../src/common/hyperlnkcmn.cpp
    ../src/common/iconbndl.cpp
    ../src/common/imagall.cpp
    ../src/common/imagbmp.cpp
    ../src/common/image.cpp
    ../src/common/imagfill.cpp
    ../src/common/imaggif.cpp
    ../src/common/imagiff.cpp
    ../src/common/imagjpeg.cpp
    ../src/common/imagpcx.cpp
    ../src/common/imagpng.cpp
    ../src/common/imagpnm.cpp
    ../src/common/imagtga.cpp
    ../src/common/imagtiff.cpp
    ../src/common/imagxpm.cpp
    ../src/common/init.cpp
    ../src/common/intl.cpp
    ../src/common/ipcbase.cpp
    ../src/common/languageinfo.cpp
    ../src/common/layout.cpp
    ../src/common/lboxcmn.cpp
    ../src/common/list.cpp
    ../src/common/listctrlcmn.cpp
    ../src/common/log.cpp
    ../src/common/longlong.cpp
    ../src/common/lzmastream.cpp
    ../src/common/markupparser.cpp
    ../src/common/matrix.cpp
    ../src/common/mediactrlcmn.cpp
    ../src/common/memory.cpp
    ../src/common/menucmn.cpp
    ../src/common/mimecmn.cpp
    ../src/common/modalhook.cpp
    ../src/common/module.cpp
    ../src/common/mousemanager.cpp
    ../src/common/msgout.cpp
    ../src/common/mstream.cpp
    ../src/common/nbkbase.cpp
    ../src/common/notifmsgcmn.cpp
    ../src/common/numformatter.cpp
    ../src/common/object.cpp
    ../src/common/odcombocmn.cpp
    ../src/common/overlaycmn.cpp
    ../src/common/ownerdrwcmn.cpp
    ../src/common/panelcmn.cpp
    ../src/common/paper.cpp
    ../src/common/persist.cpp
    ../src/common/pickerbase.cpp
    ../src/common/platinfo.cpp
    ../src/common/popupcmn.cpp
    ../src/common/powercmn.cpp
    ../src/common/preferencescmn.cpp
    ../src/common/prntbase.cpp
    ../src/common/process.cpp
    ../src/common/protocol.cpp
    ../src/common/quantize.cpp
    ../src/common/radiobtncmn.cpp
    ../src/common/radiocmn.cpp
    ../src/common/rearrangectrl.cpp
    ../src/common/regex.cpp
    ../src/common/rendcmn.cpp
    ../src/common/rgncmn.cpp
    ../src/common/richtooltipcmn.cpp
    ../src/common/sckaddr.cpp
    ../src/common/sckfile.cpp
    ../src/common/sckipc.cpp
    ../src/common/sckstrm.cpp
    ../src/common/scrolbarcmn.cpp
    ../src/common/secretstore.cpp
    ../src/common/settcmn.cpp
    ../src/common/sizer.cpp
    ../src/common/slidercmn.cpp
    ../src/common/socket.cpp
    ../src/common/spinbtncmn.cpp
    ../src/common/spinctrlcmn.cpp
    ../src/common/srchcmn.cpp
    ../src/common/sstream.cpp
    ../src/common/statbar.cpp
    ../src/common/statbmpcmn.cpp
    ../src/common/statboxcmn.cpp
    ../src/common/statlinecmn.cpp
    ../src/common/stattextcmn.cpp
    ../src/common/stdpbase.cpp
    ../src/common/stdstream.cpp
    ../src/common/stockitem.cpp
    ../src/common/stopwatch.cpp
    ../src/common/strconv.cpp
    ../src/common/stream.cpp
    ../src/common/string.cpp
    ../src/common/stringimpl.cpp
    ../src/common/stringops.cpp
    ../src/common/strvararg.cpp
    ../src/common/sysopt.cpp
    ../src/common/tarstrm.cpp
    ../src/common/tbarbase.cpp
    ../src/common/textbuf.cpp
    ../src/common/textcmn.cpp
    ../src/common/textentrycmn.cpp
    ../src/common/textfile.cpp
    ../src/common/textmeasurecmn.cpp
    ../src/common/threadinfo.cpp
    ../src/common/time.cpp
    ../src/common/timercmn.cpp
    ../src/common/timerimpl.cpp
    ../src/common/tokenzr.cpp
    ../src/common/toplvcmn.cpp
    ../src/common/translation.cpp
    ../src/common/treebase.cpp
    ../src/common/txtstrm.cpp
    ../src/common/uiactioncmn.cpp
    ../src/common/uilocale.cpp
    ../src/common/unichar.cpp
    ../src/common/uri.cpp
    ../src/common/url.cpp
    ../src/common/ustring.cpp
    ../src/common/utilscmn.cpp
    ../src/common/valgen.cpp
    ../src/common/validate.cpp
    ../src/common/valnum.cpp
    ../src/common/valtext.cpp
    ../src/common/variant.cpp
    ../src/common/webrequest.cpp
    ../src/common/webrequest_curl.cpp
    ../src/common/webview.cpp
    ../src/common/webviewarchivehandler.cpp
    ../src/common/webviewfshandler.cpp
    ../src/common/wfstream.cpp
    ../src/common/wincmn.cpp
    ../src/common/windowid.cpp
    ../src/common/wrapsizer.cpp
    ../src/common/wxcrt.cpp
    ../src/common/wxprintf.cpp
    ../src/common/xlocale.cpp
    ../src/common/xpmdecod.cpp
    ../src/common/xti.cpp
    ../src/common/xtistrm.cpp
    ../src/common/xtixml.cpp
    ../src/common/zipstrm.cpp
    ../src/common/zstream.cpp
    ../src/generic/aboutdlgg.cpp
    ../src/generic/animateg.cpp
    ../src/generic/bannerwindow.cpp
    ../src/generic/bmpcboxg.cpp
    ../src/generic/bmpsvg.cpp
    ../src/generic/busyinfo.cpp
    ../src/generic/buttonbar.cpp
    ../src/generic/calctrlg.cpp
    ../src/generic/choicbkg.cpp
    ../src/generic/choicdgg.cpp
    ../src/generic/collheaderctrlg.cpp
    ../src/generic/combog.cpp
    ../src/generic/commandlinkbuttong.cpp
    ../src/generic/creddlgg.cpp
    ../src/generic/datavgen.cpp
    ../src/generic/datectlg.cpp
    ../src/generic/dbgrptg.cpp
    ../src/generic/dcpsg.cpp
    ../src/generic/dirctrlg.cpp
    ../src/generic/dragimgg.cpp
    ../src/generic/editlbox.cpp
    ../src/generic/filectrlg.cpp
    ../src/generic/fswatcherg.cpp
    ../src/generic/graphicc.cpp
    ../src/generic/grid.cpp
    ../src/generic/gridctrl.cpp
    ../src/generic/grideditors.cpp
    ../src/generic/gridsel.cpp
    ../src/generic/headerctrlg.cpp
    ../src/generic/helpext.cpp
    ../src/generic/htmllbox.cpp
    ../src/generic/hyperlinkg.cpp
    ../src/generic/infobar.cpp
    ../src/generic/laywin.cpp
    ../src/generic/listbkg.cpp
    ../src/generic/logg.cpp
    ../src/generic/markuptext.cpp
    ../src/generic/msgdlgg.cpp
    ../src/generic/notifmsgg.cpp
    ../src/generic/numdlgg.cpp
    ../src/generic/odcombo.cpp
    ../src/generic/preferencesg.cpp
    ../src/generic/printps.cpp
    ../src/generic/progdlgg.cpp
    ../src/generic/propdlg.cpp
    ../src/generic/renderg.cpp
    ../src/generic/richmsgdlgg.cpp
    ../src/generic/richtooltipg.cpp
    ../src/generic/rowheightcache.cpp
    ../src/generic/sashwin.cpp
    ../src/generic/scrlwing.cpp
    ../src/generic/selstore.cpp
    ../src/generic/spinctlg.cpp
    ../src/generic/splash.cpp
    ../src/generic/splitter.cpp
    ../src/generic/srchctlg.cpp
    ../src/generic/statbmpg.cpp
    ../src/generic/stattextg.cpp
    ../src/generic/textdlgg.cpp
    ../src/generic/timectrlg.cpp
    ../src/generic/tipdlg.cpp
    ../src/generic/tipwin.cpp
    ../src/generic/toolbkg.cpp
    ../src/generic/treebkg.cpp
    ../src/generic/treectlg.cpp
    ../src/generic/treelist.cpp
    ../src/generic/vlbox.cpp
    ../src/generic/vscroll.cpp
    ../src/generic/wizard.cpp
    ../src/html/chm.cpp
    ../src/html/helpctrl.cpp
    ../src/html/helpdata.cpp
    ../src/html/helpdlg.cpp
    ../src/html/helpfrm.cpp
    ../src/html/helpwnd.cpp
    ../src/html/htmlcell.cpp
    ../src/html/htmlfilt.cpp
    ../src/html/htmlpars.cpp
    ../src/html/htmltag.cpp
    ../src/html/htmlwin.cpp
    ../src/html/htmprint.cpp
    ../src/html/m_dflist.cpp
    ../src/html/m_fonts.cpp
    ../src/html/m_hline.cpp
    ../src/html/m_image.cpp
    ../src/html/m_layout.cpp
    ../src/html/m_links.cpp
    ../src/html/m_list.cpp
    ../src/html/m_pre.cpp
    ../src/html/m_span.cpp
    ../src/html/m_style.cpp
    ../src/html/m_tables.cpp
    ../src/html/styleparams.cpp
    ../src/html/winpars.cpp
    ../src/propgrid/advprops.cpp
    ../src/propgrid/editors.cpp
    ../src/propgrid/manager.cpp
    ../src/propgrid/property.cpp
    ../src/propgrid/propgrid.cpp
    ../src/propgrid/propgridiface.cpp
    ../src/propgrid/propgridpagestate.cpp
    ../src/propgrid/props.cpp
    ../src/ribbon/art_aui.cpp
    ../src/ribbon/art_internal.cpp
    ../src/ribbon/bar.cpp
    ../src/ribbon/buttonbar.cpp
    ../src/ribbon/control.cpp
    ../src/ribbon/gallery.cpp
    ../src/ribbon/page.cpp
    ../src/ribbon/panel.cpp
    ../src/ribbon/toolbar.cpp
    ../src/richtext/richtextbuffer.cpp
    ../src/richtext/richtextctrl.cpp
    ../src/richtext/richtextformatdlg.cpp
    ../src/richtext/richtexthtml.cpp
    ../src/richtext/richtextimagedlg.cpp
    ../src/richtext/richtextprint.cpp
    ../src/richtext/richtextstyledlg.cpp
    ../src/richtext/richtextstyles.cpp
    ../src/richtext/richtextsymboldlg.cpp
    ../src/richtext/richtextxml.cpp
    ../src/stc/PlatWX.cpp
    ../src/stc/ScintillaWX.cpp
    ../src/stc/scintilla/lexers/LexA68k.cxx
    ../src/stc/scintilla/lexers/LexAPDL.cxx
    ../src/stc/scintilla/lexers/LexASY.cxx
    ../src/stc/scintilla/lexers/LexAU3.cxx
    ../src/stc/scintilla/lexers/LexAVE.cxx
    ../src/stc/scintilla/lexers/LexAVS.cxx
    ../src/stc/scintilla/lexers/LexAbaqus.cxx
    ../src/stc/scintilla/lexers/LexAda.cxx
    ../src/stc/scintilla/lexers/LexAsm.cxx
    ../src/stc/scintilla/lexers/LexAsn1.cxx
    ../src/stc/scintilla/lexers/LexBaan.cxx
    ../src/stc/scintilla/lexers/LexBash.cxx
    ../src/stc/scintilla/lexers/LexBasic.cxx
    ../src/stc/scintilla/lexers/LexBatch.cxx
    ../src/stc/scintilla/lexers/LexBibTeX.cxx
    ../src/stc/scintilla/lexers/LexBullant.cxx
    ../src/stc/scintilla/lexers/LexCLW.cxx
    ../src/stc/scintilla/lexers/LexCOBOL.cxx
    ../src/stc/scintilla/lexers/LexCPP.cxx
    ../src/stc/scintilla/lexers/LexCSS.cxx
    ../src/stc/scintilla/lexers/LexCaml.cxx
    ../src/stc/scintilla/lexers/LexCmake.cxx
    ../src/stc/scintilla/lexers/LexCoffeeScript.cxx
    ../src/stc/scintilla/lexers/LexConf.cxx
    ../src/stc/scintilla/lexers/LexCrontab.cxx
    ../src/stc/scintilla/lexers/LexCsound.cxx
    ../src/stc/scintilla/lexers/LexD.cxx
    ../src/stc/scintilla/lexers/LexDMAP.cxx
    ../src/stc/scintilla/lexers/LexDMIS.cxx
    ../src/stc/scintilla/lexers/LexDiff.cxx
    ../src/stc/scintilla/lexers/LexECL.cxx
    ../src/stc/scintilla/lexers/LexEDIFACT.cxx
    ../src/stc/scintilla/lexers/LexEScript.cxx
    ../src/stc/scintilla/lexers/LexEiffel.cxx
    ../src/stc/scintilla/lexers/LexErlang.cxx
    ../src/stc/scintilla/lexers/LexErrorList.cxx
    ../src/stc/scintilla/lexers/LexFlagship.cxx
    ../src/stc/scintilla/lexers/LexForth.cxx
    ../src/stc/scintilla/lexers/LexFortran.cxx
    ../src/stc/scintilla/lexers/LexGAP.cxx
    ../src/stc/scintilla/lexers/LexGui4Cli.cxx
    ../src/stc/scintilla/lexers/LexHTML.cxx
    ../src/stc/scintilla/lexers/LexHaskell.cxx
    ../src/stc/scintilla/lexers/LexHex.cxx
    ../src/stc/scintilla/lexers/LexInno.cxx
    ../src/stc/scintilla/lexers/LexJSON.cxx
    ../src/stc/scintilla/lexers/LexKVIrc.cxx
    ../src/stc/scintilla/lexers/LexKix.cxx
    ../src/stc/scintilla/lexers/LexLaTeX.cxx
    ../src/stc/scintilla/lexers/LexLisp.cxx
    ../src/stc/scintilla/lexers/LexLout.cxx
    ../src/stc/scintilla/lexers/LexLua.cxx
    ../src/stc/scintilla/lexers/LexMMIXAL.cxx
    ../src/stc/scintilla/lexers/LexMPT.cxx
    ../src/stc/scintilla/lexers/LexMSSQL.cxx
    ../src/stc/scintilla/lexers/LexMagik.cxx
    ../src/stc/scintilla/lexers/LexMake.cxx
    ../src/stc/scintilla/lexers/LexMarkdown.cxx
    ../src/stc/scintilla/lexers/LexMatlab.cxx
    ../src/stc/scintilla/lexers/LexMetapost.cxx
    ../src/stc/scintilla/lexers/LexModula.cxx
    ../src/stc/scintilla/lexers/LexMySQL.cxx
    ../src/stc/scintilla/lexers/LexNimrod.cxx
    ../src/stc/scintilla/lexers/LexNsis.cxx
    ../src/stc/scintilla/lexers/LexNull.cxx
    ../src/stc/scintilla/lexers/LexOScript.cxx
    ../src/stc/scintilla/lexers/LexOpal.cxx
    ../src/stc/scintilla/lexers/LexPB.cxx
    ../src/stc/scintilla/lexers/LexPLM.cxx
    ../src/stc/scintilla/lexers/LexPO.cxx
    ../src/stc/scintilla/lexers/LexPOV.cxx
    ../src/stc/scintilla/lexers/LexPS.cxx
    ../src/stc/scintilla/lexers/LexPascal.cxx
    ../src/stc/scintilla/lexers/LexPerl.cxx
    ../src/stc/scintilla/lexers/LexPowerPro.cxx
    ../src/stc/scintilla/lexers/LexPowerShell.cxx
    ../src/stc/scintilla/lexers/LexProgress.cxx
    ../src/stc/scintilla/lexers/LexProps.cxx
    ../src/stc/scintilla/lexers/LexPython.cxx
    ../src/stc/scintilla/lexers/LexR.cxx
    ../src/stc/scintilla/lexers/LexRebol.cxx
    ../src/stc/scintilla/lexers/LexRegistry.cxx
    ../src/stc/scintilla/lexers/LexRuby.cxx
    ../src/stc/scintilla/lexers/LexRust.cxx
    ../src/stc/scintilla/lexers/LexSML.cxx
    ../src/stc/scintilla/lexers/LexSQL.cxx
    ../src/stc/scintilla/lexers/LexSTTXT.cxx
    ../src/stc/scintilla/lexers/LexScriptol.cxx
    ../src/stc/scintilla/lexers/LexSmalltalk.cxx
    ../src/stc/scintilla/lexers/LexSorcus.cxx
    ../src/stc/scintilla/lexers/LexSpecman.cxx
    ../src/stc/scintilla/lexers/LexSpice.cxx
    ../src/stc/scintilla/lexers/LexTACL.cxx
    ../src/stc/scintilla/lexers/LexTADS3.cxx
    ../src/stc/scintilla/lexers/LexTAL.cxx
    ../src/stc/scintilla/lexers/LexTCL.cxx
    ../src/stc/scintilla/lexers/LexTCMD.cxx
    ../src/stc/scintilla/lexers/LexTeX.cxx
    ../src/stc/scintilla/lexers/LexTxt2tags.cxx
    ../src/stc/scintilla/lexers/LexVB.cxx
    ../src/stc/scintilla/lexers/LexVHDL.cxx
    ../src/stc/scintilla/lexers/LexVerilog.cxx
    ../src/stc/scintilla/lexers/LexVisualProlog.cxx
    ../src/stc/scintilla/lexers/LexYAML.cxx
    ../src/stc/scintilla/lexlib/Accessor.cxx
    ../src/stc/scintilla/lexlib/CharacterCategory.cxx
    ../src/stc/scintilla/lexlib/CharacterSet.cxx
    ../src/stc/scintilla/lexlib/LexerBase.cxx
    ../src/stc/scintilla/lexlib/LexerModule.cxx
    ../src/stc/scintilla/lexlib/LexerNoExceptions.cxx
    ../src/stc/scintilla/lexlib/LexerSimple.cxx
    ../src/stc/scintilla/lexlib/PropSetSimple.cxx
    ../src/stc/scintilla/lexlib/StyleContext.cxx
    ../src/stc/scintilla/lexlib/WordList.cxx
    ../src/stc/scintilla/src/AutoComplete.cxx
    ../src/stc/scintilla/src/CallTip.cxx
    ../src/stc/scintilla/src/CaseConvert.cxx
    ../src/stc/scintilla/src/CaseFolder.cxx
    ../src/stc/scintilla/src/Catalogue.cxx
    ../src/stc/scintilla/src/CellBuffer.cxx
    ../src/stc/scintilla/src/CharClassify.cxx
    ../src/stc/scintilla/src/ContractionState.cxx
    ../src/stc/scintilla/src/Decoration.cxx
    ../src/stc/scintilla/src/Document.cxx
    ../src/stc/scintilla/src/EditModel.cxx
    ../src/stc/scintilla/src/EditView.cxx
    ../src/stc/scintilla/src/Editor.cxx
    ../src/stc/scintilla/src/ExternalLexer.cxx
    ../src/stc/scintilla/src/Indicator.cxx
    ../src/stc/scintilla/src/KeyMap.cxx
    ../src/stc/scintilla/src/LineMarker.cxx
    ../src/stc/scintilla/src/MarginView.cxx
    ../src/stc/scintilla/src/PerLine.cxx
    ../src/stc/scintilla/src/PositionCache.cxx
    ../src/stc/scintilla/src/RESearch.cxx
    ../src/stc/scintilla/src/RunStyles.cxx
    ../src/stc/scintilla/src/ScintillaBase.cxx
    ../src/stc/scintilla/src/Selection.cxx
    ../src/stc/scintilla/src/Style.cxx
    ../src/stc/scintilla/src/UniConversion.cxx
    ../src/stc/scintilla/src/ViewStyle.cxx
    ../src/stc/scintilla/src/XPM.cxx
    ../src/stc/stc.cpp
    ../src/xml/xml.cpp
    ../src/xrc/xh_activityindicator.cpp
    ../src/xrc/xh_animatctrl.cpp
    ../src/xrc/xh_aui.cpp
    ../src/xrc/xh_auitoolb.cpp
    ../src/xrc/xh_bannerwindow.cpp
    ../src/xrc/xh_bmp.cpp
    ../src/xrc/xh_bmpbt.cpp
    ../src/xrc/xh_bmpcbox.cpp
    ../src/xrc/xh_bookctrlbase.cpp
    ../src/xrc/xh_bttn.cpp
    ../src/xrc/xh_cald.cpp
    ../src/xrc/xh_chckb.cpp
    ../src/xrc/xh_chckl.cpp
    ../src/xrc/xh_choic.cpp
    ../src/xrc/xh_choicbk.cpp
    ../src/xrc/xh_clrpicker.cpp
    ../src/xrc/xh_cmdlinkbn.cpp
    ../src/xrc/xh_collpane.cpp
    ../src/xrc/xh_combo.cpp
    ../src/xrc/xh_comboctrl.cpp
    ../src/xrc/xh_dataview.cpp
    ../src/xrc/xh_datectrl.cpp
    ../src/xrc/xh_dirpicker.cpp
    ../src/xrc/xh_dlg.cpp
    ../src/xrc/xh_editlbox.cpp
    ../src/xrc/xh_filectrl.cpp
    ../src/xrc/xh_filepicker.cpp
    ../src/xrc/xh_fontpicker.cpp
    ../src/xrc/xh_frame.cpp
    ../src/xrc/xh_gauge.cpp
    ../src/xrc/xh_gdctl.cpp
    ../src/xrc/xh_grid.cpp
    ../src/xrc/xh_html.cpp
    ../src/xrc/xh_htmllbox.cpp
    ../src/xrc/xh_hyperlink.cpp
    ../src/xrc/xh_infobar.cpp
    ../src/xrc/xh_listb.cpp
    ../src/xrc/xh_listbk.cpp
    ../src/xrc/xh_listc.cpp
    ../src/xrc/xh_mdi.cpp
    ../src/xrc/xh_menu.cpp
    ../src/xrc/xh_notbk.cpp
    ../src/xrc/xh_odcombo.cpp
    ../src/xrc/xh_panel.cpp
    ../src/xrc/xh_propdlg.cpp
    ../src/xrc/xh_radbt.cpp
    ../src/xrc/xh_radbx.cpp
    ../src/xrc/xh_ribbon.cpp
    ../src/xrc/xh_richtext.cpp
    ../src/xrc/xh_scrol.cpp
    ../src/xrc/xh_scwin.cpp
    ../src/xrc/xh_simplebook.cpp
    ../src/xrc/xh_sizer.cpp
    ../src/xrc/xh_slidr.cpp
    ../src/xrc/xh_spin.cpp
    ../src/xrc/xh_split.cpp
    ../src/xrc/xh_srchctrl.cpp
    ../src/xrc/xh_statbar.cpp
    ../src/xrc/xh_stbmp.cpp
    ../src/xrc/xh_stbox.cpp
    ../src/xrc/xh_stlin.cpp
    ../src/xrc/xh_sttxt.cpp
    ../src/xrc/xh_styledtextctrl.cpp
    ../src/xrc/xh_text.cpp
    ../src/xrc/xh_tglbtn.cpp
    ../src/xrc/xh_timectrl.cpp
    ../src/xrc/xh_toolb.cpp
    ../src/xrc/xh_toolbk.cpp
    ../src/xrc/xh_tree.cpp
    ../src/xrc/xh_treebk.cpp
    ../src/xrc/xh_unkwn.cpp
    ../src/xrc/xh_wizrd.cpp
    ../src/xrc/xmlres.cpp
    ../src/xrc/xmlreshandler.cpp
    ../src/xrc/xmlrsall.cpp

    ../src/common/extended.cpp  # C++ copy of extended.c
)

set (msw_sources
    ../src/aui/barartmsw.cpp
    ../src/aui/tabartmsw.cpp
    ../src/common/taskbarcmn.cpp
    ../src/generic/activityindicator.cpp
    ../src/generic/clrpickerg.cpp
    ../src/generic/collpaneg.cpp
    ../src/generic/filepickerg.cpp
    ../src/generic/fontpickerg.cpp
    ../src/generic/prntdlgg.cpp
    ../src/generic/statusbr.cpp
    ../src/msw/aboutdlg.cpp
    ../src/msw/accel.cpp
    ../src/msw/anybutton.cpp
    ../src/msw/app.cpp
    ../src/msw/appprogress.cpp
    ../src/msw/artmsw.cpp
    ../src/msw/basemsw.cpp
    ../src/msw/bitmap.cpp
    ../src/msw/bmpbndl.cpp
    ../src/msw/bmpbuttn.cpp
    ../src/msw/bmpcbox.cpp
    ../src/msw/brush.cpp
    ../src/msw/button.cpp
    ../src/msw/calctrl.cpp
    ../src/msw/caret.cpp
    ../src/msw/checkbox.cpp
    ../src/msw/checklst.cpp
    ../src/msw/choice.cpp
    ../src/msw/clipbrd.cpp
    ../src/msw/colordlg.cpp
    ../src/msw/colour.cpp
    ../src/msw/combo.cpp
    ../src/msw/combobox.cpp
    ../src/msw/commandlinkbutton.cpp
    ../src/msw/control.cpp
    ../src/msw/crashrpt.cpp
    ../src/msw/cursor.cpp
    ../src/msw/customdraw.cpp
    ../src/msw/data.cpp
    ../src/msw/datecontrols.cpp
    ../src/msw/datectrl.cpp
    ../src/msw/datetimectrl.cpp
    ../src/msw/dc.cpp
    ../src/msw/dcclient.cpp
    ../src/msw/dcmemory.cpp
    ../src/msw/dcprint.cpp
    ../src/msw/dcscreen.cpp
    ../src/msw/dde.cpp
    ../src/msw/debughlp.cpp
    ../src/msw/dialog.cpp
    ../src/msw/dialup.cpp
    ../src/msw/dib.cpp
    ../src/msw/dir.cpp
    ../src/msw/dirdlg.cpp
    ../src/msw/display.cpp
    ../src/msw/dlmsw.cpp
    ../src/msw/dragimag.cpp
    ../src/msw/enhmeta.cpp
    ../src/msw/evtloop.cpp
    ../src/msw/evtloopconsole.cpp
    ../src/msw/fdrepdlg.cpp
    ../src/msw/filedlg.cpp
    ../src/msw/font.cpp
    ../src/msw/fontdlg.cpp
    ../src/msw/fontenum.cpp
    ../src/msw/fontutil.cpp
    ../src/msw/frame.cpp
    ../src/msw/fswatcher.cpp
    ../src/msw/gauge.cpp
    ../src/msw/gdiimage.cpp
    ../src/msw/gdiobj.cpp
    ../src/msw/gdiplus.cpp
    ../src/msw/glcanvas.cpp
    ../src/msw/graphics.cpp
    ../src/msw/graphicsd2d.cpp
    ../src/msw/headerctrl.cpp
    ../src/msw/helpbest.cpp
    ../src/msw/helpchm.cpp
    ../src/msw/helpwin.cpp
    ../src/msw/hyperlink.cpp
    ../src/msw/icon.cpp
    ../src/msw/imaglist.cpp
    ../src/msw/iniconf.cpp
    ../src/msw/joystick.cpp
    ../src/msw/listbox.cpp
    ../src/msw/listctrl.cpp
    ../src/msw/main.cpp
    ../src/msw/mdi.cpp
    ../src/msw/mediactrl_am.cpp
    ../src/msw/mediactrl_qt.cpp
    ../src/msw/mediactrl_wmp10.cpp
    ../src/msw/menu.cpp
    ../src/msw/menuitem.cpp
    ../src/msw/metafile.cpp
    ../src/msw/mimetype.cpp
    ../src/msw/minifram.cpp
    ../src/msw/msgdlg.cpp
    ../src/msw/nativdlg.cpp
    ../src/msw/nativewin.cpp
    ../src/msw/nonownedwnd.cpp
    ../src/msw/notebook.cpp
    ../src/msw/notifmsg.cpp
    ../src/msw/ole/access.cpp
    ../src/msw/ole/activex.cpp
    ../src/msw/ole/automtn.cpp
    ../src/msw/ole/comimpl.cpp
    ../src/msw/ole/dataobj.cpp
    ../src/msw/ole/dropsrc.cpp
    ../src/msw/ole/droptgt.cpp
    ../src/msw/ole/oleutils.cpp
    ../src/msw/ole/safearray.cpp
    ../src/msw/ole/uuid.cpp
    ../src/msw/ownerdrw.cpp
    ../src/msw/palette.cpp
    ../src/msw/pen.cpp
    ../src/msw/popupwin.cpp
    ../src/msw/power.cpp
    ../src/msw/printdlg.cpp
    ../src/msw/printwin.cpp
    ../src/msw/progdlg.cpp
    ../src/msw/radiobox.cpp
    ../src/msw/radiobut.cpp
    ../src/msw/regconf.cpp
    ../src/msw/region.cpp
    ../src/msw/registry.cpp
    ../src/msw/renderer.cpp
    ../src/msw/richmsgdlg.cpp
    ../src/msw/richtooltip.cpp
    ../src/msw/rt/notifmsgrt.cpp
    ../src/msw/rt/utilsrt.cpp
    ../src/msw/scrolbar.cpp
    ../src/msw/secretstore.cpp
    ../src/msw/settings.cpp
    ../src/msw/slider.cpp
    ../src/msw/snglinst.cpp
    ../src/msw/sockmsw.cpp
    ../src/msw/sound.cpp
    ../src/msw/spinbutt.cpp
    ../src/msw/spinctrl.cpp
    ../src/msw/stackwalk.cpp
    ../src/msw/statbmp.cpp
    ../src/msw/statbox.cpp
    ../src/msw/statline.cpp
    ../src/msw/stattext.cpp
    ../src/msw/statusbar.cpp
    ../src/msw/stdpaths.cpp
    ../src/msw/systhemectrl.cpp
    ../src/msw/taskbar.cpp
    ../src/msw/taskbarbutton.cpp
    ../src/msw/textctrl.cpp
    ../src/msw/textentry.cpp
    ../src/msw/textmeasure.cpp
    ../src/msw/tglbtn.cpp
    ../src/msw/thread.cpp
    ../src/msw/timectrl.cpp
    ../src/msw/timer.cpp
    ../src/msw/toolbar.cpp
    ../src/msw/tooltip.cpp
    ../src/msw/toplevel.cpp
    ../src/msw/treectrl.cpp
    ../src/msw/uiaction.cpp
    ../src/msw/uilocale.cpp
    ../src/msw/urlmsw.cpp
    ../src/msw/utils.cpp
    ../src/msw/utilsexc.cpp
    ../src/msw/utilsgui.cpp
    ../src/msw/utilswin.cpp
    ../src/msw/uxtheme.cpp
    ../src/msw/volume.cpp
    ../src/msw/webrequest_winhttp.cpp
    ../src/msw/webview_edge.cpp
    ../src/msw/webview_ie.cpp
    ../src/msw/window.cpp
    ../src/ribbon/art_msw.cpp
)

set (unix_sources
    ../src/aui/tabartgtk.cpp
    ../src/common/fdiodispatcher.cpp
    ../src/common/selectdispatcher.cpp
    ../src/common/socketiohandler.cpp
    ../src/common/taskbarcmn.cpp
    ../src/generic/accel.cpp
    ../src/generic/activityindicator.cpp
    ../src/generic/caret.cpp
    ../src/generic/clrpickerg.cpp
    ../src/generic/collpaneg.cpp
    ../src/generic/colrdlgg.cpp
    ../src/generic/dirdlgg.cpp
    ../src/generic/fdrepdlg.cpp
    ../src/generic/filedlgg.cpp
    ../src/generic/filepickerg.cpp
    ../src/generic/fontdlgg.cpp
    ../src/generic/fontpickerg.cpp
    ../src/generic/imaglist.cpp
    ../src/generic/listctrl.cpp
    ../src/generic/prntdlgg.cpp
    ../src/generic/statusbr.cpp
    ../src/gtk/aboutdlg.cpp
    ../src/gtk/activityindicator.cpp
    ../src/gtk/animate.cpp
    ../src/gtk/anybutton.cpp
    ../src/gtk/artgtk.cpp
    ../src/gtk/bmpbuttn.cpp
    ../src/gtk/bmpcbox.cpp
    ../src/gtk/button.cpp
    ../src/gtk/calctrl.cpp
    ../src/gtk/checkbox.cpp
    ../src/gtk/checklst.cpp
    ../src/gtk/choice.cpp
    ../src/gtk/clrpicker.cpp
    ../src/gtk/collpane.cpp
    ../src/gtk/colordlg.cpp
    ../src/gtk/combobox.cpp
    ../src/gtk/control.cpp
    ../src/gtk/dataview.cpp
    ../src/gtk/dialog.cpp
    ../src/gtk/dirdlg.cpp
    ../src/gtk/filedlg.cpp
    ../src/gtk/filepicker.cpp
    ../src/gtk/fontdlg.cpp
    ../src/gtk/fontpicker.cpp
    ../src/gtk/frame.cpp
    ../src/gtk/gauge.cpp
    ../src/gtk/glcanvas.cpp
    ../src/gtk/gnome/gvfs.cpp
    ../src/gtk/hyperlink.cpp
    ../src/gtk/infobar.cpp
    ../src/gtk/listbox.cpp
    ../src/gtk/mdi.cpp
    ../src/gtk/menu.cpp
    ../src/gtk/mnemonics.cpp
    ../src/gtk/msgdlg.cpp
    ../src/gtk/nativewin.cpp
    ../src/gtk/notebook.cpp
    ../src/gtk/notifmsg.cpp
    ../src/gtk/print.cpp
    ../src/gtk/radiobox.cpp
    ../src/gtk/radiobut.cpp
    ../src/gtk/scrolbar.cpp
    ../src/gtk/scrolwin.cpp
    ../src/gtk/slider.cpp
    ../src/gtk/spinbutt.cpp
    ../src/gtk/spinctrl.cpp
    ../src/gtk/srchctrl.cpp
    ../src/gtk/statbmp.cpp
    ../src/gtk/statbox.cpp
    ../src/gtk/statline.cpp
    ../src/gtk/stattext.cpp
    ../src/gtk/taskbar.cpp
    ../src/gtk/textctrl.cpp
    ../src/gtk/textentry.cpp
    ../src/gtk/tglbtn.cpp
    ../src/gtk/toolbar.cpp
    ../src/gtk/webview_webkit.cpp
    ../src/gtk/webview_webkit2.cpp
    ../src/gtk/webview_webkit2_extension.cpp
    ../src/gtk1/bmpbuttn.cpp
    ../src/gtk1/button.cpp
    ../src/gtk1/checkbox.cpp
    ../src/gtk1/checklst.cpp
    ../src/gtk1/choice.cpp
    ../src/gtk1/combobox.cpp
    ../src/gtk1/control.cpp
    ../src/gtk1/dialog.cpp
    ../src/gtk1/filedlg.cpp
    ../src/gtk1/fontdlg.cpp
    ../src/gtk1/frame.cpp
    ../src/gtk1/gauge.cpp
    ../src/gtk1/listbox.cpp
    ../src/gtk1/mdi.cpp
    ../src/gtk1/menu.cpp
    ../src/gtk1/mnemonics.cpp
    ../src/gtk1/notebook.cpp
    ../src/gtk1/radiobox.cpp
    ../src/gtk1/radiobut.cpp
    ../src/gtk1/scrolbar.cpp
    ../src/gtk1/scrolwin.cpp
    ../src/gtk1/slider.cpp
    ../src/gtk1/spinbutt.cpp
    ../src/gtk1/spinctrl.cpp
    ../src/gtk1/statbmp.cpp
    ../src/gtk1/statbox.cpp
    ../src/gtk1/statline.cpp
    ../src/gtk1/stattext.cpp
    ../src/gtk1/taskbar.cpp
    ../src/gtk1/textctrl.cpp
    ../src/gtk1/tglbtn.cpp
    ../src/gtk1/toolbar.cpp
    ../src/unix/apptraits.cpp
    ../src/unix/appunix.cpp
    ../src/unix/dir.cpp
    ../src/unix/dlunix.cpp
    ../src/unix/epolldispatcher.cpp
    ../src/unix/evtloopunix.cpp
    ../src/unix/fdiounix.cpp
    ../src/unix/fontenum.cpp
    ../src/unix/fontutil.cpp
    ../src/unix/fswatcher_kqueue.cpp
    ../src/unix/glegl.cpp
    ../src/unix/glx11.cpp
    ../src/unix/mediactrl.cpp
    ../src/unix/mediactrl_gstplayer.cpp
    ../src/unix/snglinst.cpp
    ../src/unix/sockunix.cpp
    ../src/unix/sound_sdl.cpp
    ../src/unix/stackwalk.cpp
    ../src/unix/threadpsx.cpp
    ../src/unix/timerunx.cpp
    ../src/unix/utilsunx.cpp
    ../src/unix/wakeuppipe.cpp
)

set (osx_sources
    ../src/generic/caret.cpp
    ../src/generic/clrpickerg.cpp
    ../src/generic/collpaneg.cpp
    ../src/generic/colrdlgg.cpp
    ../src/generic/dirdlgg.cpp
    ../src/generic/fdrepdlg.cpp
    ../src/generic/filedlgg.cpp
    ../src/generic/filepickerg.cpp
    ../src/generic/fontdlgg.cpp
    ../src/generic/fontpickerg.cpp
    ../src/generic/icon.cpp
    ../src/generic/imaglist.cpp
    ../src/generic/listctrl.cpp
    ../src/generic/prntdlgg.cpp
    ../src/generic/statusbr.cpp
    ../src/generic/textmeasure.cpp
    ../src/osx/accel.cpp
    ../src/osx/anybutton_osx.cpp
    ../src/osx/artmac.cpp
    ../src/osx/bmpbuttn_osx.cpp
    ../src/osx/brush.cpp
    ../src/osx/button_osx.cpp
    ../src/osx/carbon/app.cpp
    ../src/osx/carbon/clipbrd.cpp
    ../src/osx/carbon/control.cpp
    ../src/osx/carbon/cursor.cpp
    ../src/osx/carbon/dataobj.cpp
    ../src/osx/carbon/dcclient.cpp
    ../src/osx/carbon/dcprint.cpp
    ../src/osx/carbon/dcscreen.cpp
    ../src/osx/carbon/font.cpp
    ../src/osx/carbon/fontdlg.cpp
    ../src/osx/carbon/frame.cpp
    ../src/osx/carbon/gdiobj.cpp
    ../src/osx/carbon/graphics.cpp
    ../src/osx/carbon/mdi.cpp
    ../src/osx/carbon/metafile.cpp
    ../src/osx/carbon/popupwin.cpp
    ../src/osx/carbon/region.cpp
    ../src/osx/carbon/renderer.cpp
    ../src/osx/carbon/statbrma.cpp
    ../src/osx/checkbox_osx.cpp
    ../src/osx/checklst_osx.cpp
    ../src/osx/choice_osx.cpp
    ../src/osx/combobox_osx.cpp
    ../src/osx/core/bitmap.cpp
    ../src/osx/core/cfstring.cpp
    ../src/osx/core/colour.cpp
    ../src/osx/core/dcmemory.cpp
    ../src/osx/core/evtloop_cf.cpp
    ../src/osx/core/fontenum.cpp
    ../src/osx/core/hid.cpp
    ../src/osx/core/mimetype.cpp
    ../src/osx/core/printmac.cpp
    ../src/osx/core/secretstore.cpp
    ../src/osx/core/sockosx.cpp
    ../src/osx/core/strconv_cf.cpp
    ../src/osx/core/timer.cpp
    ../src/osx/core/utilsexc_cf.cpp
    ../src/osx/dialog_osx.cpp
    ../src/osx/dnd_osx.cpp
    ../src/osx/fontutil.cpp
    ../src/osx/fswatcher_fsevents.cpp
    ../src/osx/gauge_osx.cpp
    ../src/osx/glcanvas_osx.cpp
    ../src/osx/listbox_osx.cpp
    ../src/osx/menu_osx.cpp
    ../src/osx/menuitem_osx.cpp
    ../src/osx/minifram.cpp
    ../src/osx/nonownedwnd_osx.cpp
    ../src/osx/notebook_osx.cpp
    ../src/osx/palette.cpp
    ../src/osx/pen.cpp
    ../src/osx/printdlg_osx.cpp
    ../src/osx/radiobox_osx.cpp
    ../src/osx/radiobut_osx.cpp
    ../src/osx/scrolbar_osx.cpp
    ../src/osx/slider_osx.cpp
    ../src/osx/spinbutt_osx.cpp
    ../src/osx/srchctrl_osx.cpp
    ../src/osx/statbmp_osx.cpp
    ../src/osx/statbox_osx.cpp
    ../src/osx/statline_osx.cpp
    ../src/osx/stattext_osx.cpp
    ../src/osx/textctrl_osx.cpp
    ../src/osx/textentry_osx.cpp
    ../src/osx/tglbtn_osx.cpp
    ../src/osx/toolbar_osx.cpp
    ../src/osx/toplevel_osx.cpp
    ../src/osx/uiaction_osx.cpp
    ../src/osx/utils_osx.cpp
    ../src/osx/window_osx.cpp
)

set (wxCLib_sources
    ../src/jpeg/jaricom.c
    ../src/jpeg/jcapimin.c
    ../src/jpeg/jcapistd.c
    ../src/jpeg/jcarith.c
    ../src/jpeg/jccoefct.c
    ../src/jpeg/jccolor.c
    ../src/jpeg/jcdctmgr.c
    ../src/jpeg/jchuff.c
    ../src/jpeg/jcinit.c
    ../src/jpeg/jcmainct.c
    ../src/jpeg/jcmarker.c
    ../src/jpeg/jcmaster.c
    ../src/jpeg/jcomapi.c
    ../src/jpeg/jcparam.c
    ../src/jpeg/jcprepct.c
    ../src/jpeg/jcsample.c
    ../src/jpeg/jctrans.c
    ../src/jpeg/jdapimin.c
    ../src/jpeg/jdapistd.c
    ../src/jpeg/jdarith.c
    ../src/jpeg/jdatadst.c
    ../src/jpeg/jdatasrc.c
    ../src/jpeg/jdcoefct.c
    ../src/jpeg/jdcolor.c
    ../src/jpeg/jddctmgr.c
    ../src/jpeg/jdhuff.c
    ../src/jpeg/jdinput.c
    ../src/jpeg/jdmainct.c
    ../src/jpeg/jdmarker.c
    ../src/jpeg/jdmaster.c
    ../src/jpeg/jdmerge.c
    ../src/jpeg/jdpostct.c
    ../src/jpeg/jdsample.c
    ../src/jpeg/jdtrans.c
    ../src/jpeg/jerror.c
    ../src/jpeg/jfdctflt.c
    ../src/jpeg/jfdctfst.c
    ../src/jpeg/jfdctint.c
    ../src/jpeg/jidctflt.c
    ../src/jpeg/jidctfst.c
    ../src/jpeg/jidctint.c
    ../src/jpeg/jmemmgr.c
    ../src/jpeg/jmemnobs.c
    ../src/jpeg/jquant1.c
    ../src/jpeg/jquant2.c
    ../src/jpeg/jutils.c

    ../src/tiff/libtiff/tif_aux.c
    ../src/tiff/libtiff/tif_close.c
    ../src/tiff/libtiff/tif_codec.c
    ../src/tiff/libtiff/tif_color.c
    ../src/tiff/libtiff/tif_compress.c
    ../src/tiff/libtiff/tif_dir.c
    ../src/tiff/libtiff/tif_dirinfo.c
    ../src/tiff/libtiff/tif_dirread.c
    ../src/tiff/libtiff/tif_dirwrite.c
    ../src/tiff/libtiff/tif_dumpmode.c
    ../src/tiff/libtiff/tif_error.c
    ../src/tiff/libtiff/tif_extension.c
    ../src/tiff/libtiff/tif_fax3.c
    ../src/tiff/libtiff/tif_fax3sm.c
    ../src/tiff/libtiff/tif_flush.c
    ../src/tiff/libtiff/tif_getimage.c
    ../src/tiff/libtiff/tif_jbig.c
    ../src/tiff/libtiff/tif_jpeg.c
    ../src/tiff/libtiff/tif_jpeg_12.c
    ../src/tiff/libtiff/tif_luv.c
    ../src/tiff/libtiff/tif_lzma.c
    ../src/tiff/libtiff/tif_lzw.c
    ../src/tiff/libtiff/tif_next.c
    ../src/tiff/libtiff/tif_ojpeg.c
    ../src/tiff/libtiff/tif_open.c
    ../src/tiff/libtiff/tif_packbits.c
    ../src/tiff/libtiff/tif_pixarlog.c
    ../src/tiff/libtiff/tif_predict.c
    ../src/tiff/libtiff/tif_print.c
    ../src/tiff/libtiff/tif_read.c
    ../src/tiff/libtiff/tif_strip.c
    ../src/tiff/libtiff/tif_swab.c
    ../src/tiff/libtiff/tif_thunder.c
    ../src/tiff/libtiff/tif_tile.c
    ../src/tiff/libtiff/tif_version.c
    ../src/tiff/libtiff/tif_warning.c
    ../src/tiff/libtiff/tif_webp.c
    ../src/tiff/libtiff/tif_win32.c
    ../src/tiff/libtiff/tif_write.c
    ../src/tiff/libtiff/tif_zip.c
    ../src/tiff/libtiff/tif_zstd.c

    ../src/png/png.c
    ../src/png/pngerror.c
    ../src/png/pngget.c
    ../src/png/pngmem.c
    ../src/png/pngpread.c
    ../src/png/pngread.c
    ../src/png/pngrio.c
    ../src/png/pngrtran.c
    ../src/png/pngrutil.c
    ../src/png/pngset.c
    ../src/png/pngtrans.c
    ../src/png/pngwio.c
    ../src/png/pngwrite.c
    ../src/png/pngwtran.c
    ../src/png/pngwutil.c
    ../src/png/arm/arm_init.c
    ../src/png/arm/filter_neon_intrinsics.c
    ../src/png/arm/palette_neon_intrinsics.c
    ../src/png/intel/intel_init.c
    ../src/png/intel/filter_sse2_intrinsics.c

    ../src/zlib/adler32.c
    ../src/zlib/compress.c
    ../src/zlib/crc32.c
    ../src/zlib/deflate.c
    ../src/zlib/gzclose.c
    ../src/zlib/gzlib.c
    ../src/zlib/gzread.c
    ../src/zlib/gzwrite.c
    ../src/zlib/infback.c
    ../src/zlib/inffast.c
    ../src/zlib/inflate.c
    ../src/zlib/inftrees.c
    ../src/zlib/trees.c
    ../src/zlib/uncompr.c
    ../src/zlib/zutil.c

    # wx_wxregex

    ../src/pcre/pcre2_auto_possess.c
    ../src/pcre/pcre2_chartables.c
    ../src/pcre/pcre2_compile.c
    ../src/pcre/pcre2_config.c
    ../src/pcre/pcre2_context.c
    ../src/pcre/pcre2_convert.c
    ../src/pcre/pcre2_dfa_match.c
    ../src/pcre/pcre2_error.c
    ../src/pcre/pcre2_extuni.c
    ../src/pcre/pcre2_find_bracket.c
    ../src/pcre/pcre2_jit_compile.c
    ../src/pcre/pcre2_maketables.c
    ../src/pcre/pcre2_match.c
    ../src/pcre/pcre2_match_data.c
    ../src/pcre/pcre2_newline.c
    ../src/pcre/pcre2_ord2utf.c
    ../src/pcre/pcre2_pattern_info.c
    ../src/pcre/pcre2_script_run.c
    ../src/pcre/pcre2_serialize.c
    ../src/pcre/pcre2_string_utils.c
    ../src/pcre/pcre2_study.c
    ../src/pcre/pcre2_substitute.c
    ../src/pcre/pcre2_substring.c
    ../src/pcre/pcre2_tables.c
    ../src/pcre/pcre2_ucd.c
    ../src/pcre/pcre2_valid_utf.c
    ../src/pcre/pcre2_xclass.c

    # wx_wxexpat
    ../src/expat/expat/lib/xmlparse.c
    ../src/expat/expat/lib/xmlrole.c
    ../src/expat/expat/lib/xmltok.c

    # wxBase
    # ../src/common/extended.c
)
