@echo off

@REM This batch file assumes that you have installed both Ruby and wxRuby3, that they are in
@REM your $PATH and you are able to run wxRuby3 programs from the command line. It also
@REM assumes that you have built a debug version of wxUiEditor.exe and that it is in the
@REM build\stage\bin\debug directory.

echo Generating Ruby files...
..\..\build\stage\bin\debug\wxUiEditor.exe --gen_ruby sdi_test.wxui
type sdi_test.log
cd ruby
ruby mainframe.rb
cd ..
