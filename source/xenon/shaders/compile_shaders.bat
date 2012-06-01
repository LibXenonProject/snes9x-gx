@echo off

set PATH=%XEDK%\bin\win32;%PATH%;
set INCLUDE=%XEDK%\include\win32;%XEDK%\include\xbox;%XEDK%\include\xbox\sys;%INCLUDE%
set LIB=%XEDK%\lib\win32;%XEDK%\lib\xbox;%LIB%
set _NT_SYMBOL_PATH=SRV*%XEDK%\bin\xbox\symsrv;%_NT_SYMBOL_PATH%

echo.
echo Setting environment for using Microsoft Xbox 360 SDK tools.
echo.

echo Compile 2xBR-v3.5a shader
fxc /Fh 2xBR-v3.5a.vs.h /Tvs_3_0 filters\2xBR-v3.5a.cg /Emain_vertex  /VnVS2xBRa
fxc /Fh 2xBR-v3.5a.ps.h /Tps_3_0 filters\2xBR-v3.5a.cg /Emain_fragment /VnPS2xBRa

echo Compile 5xBR-v3.7a shader
fxc /Fh 5xBR-v3.7a.vs.h /Tvs_3_0 filters\5xBR-v3.7a.cg /Emain_vertex /VnVS5xBRa
fxc /Fh 5xBR-v3.7a.ps.h /Tps_3_0 filters\5xBR-v3.7a.cg /Emain_fragment /VnPS5xBRa

rem echo Compile 5xBR-v3.7b shader
rem fxc /Fh 5xBR-v3.7b.vs.h /Tvs_3_0 filters\5xBR-v3.7b.cg /Emain_vertex /VnVS5xBRb
rem fxc /Fh 5xBR-v3.7b.ps.h /Tps_3_0 filters\5xBR-v3.7b.cg /Emain_fragment /VnPS5xBRb

rem echo Compile 5xBR-v3.7c shader
rem fxc /Fh 5xBR-v3.7c.vs.h /Tvs_3_0 filters\5xBR-v3.7c.cg /Emain_vertex /VnVS5xBRc
rem fxc /Fh 5xBR-v3.7c.ps.h /Tps_3_0 filters\5xBR-v3.7c.cg /Emain_fragment /VnPS5xBRc

echo Compile Scanline
fxc /Fh scanline.vs.h /Tvs_3_0 filters\scanline.cg /Emain_vertex  /VnVSScanline
fxc /Fh scanline.ps.h /Tps_3_0 filters\scanline.cg /Emain_fragment /VnPSScanline

echo Compile Normal
fxc /Fh simple.vs.h /Tvs_3_0 filters\normal.cg /Emain_vertex  /VnVSSimple
fxc /Fh simple.ps.h /Tps_3_0 filters\normal.cg /Emain_fragment /VnPSSimple

cmd
