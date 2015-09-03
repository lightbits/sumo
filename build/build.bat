@echo off
pushd .\build
set INCLUDE_PATHS=/I"../src" /I"../../../sdl/include"
set LIB_PATHS=/LIBPATH:"../../../sdl/lib/x86"
set DEFINES=/DSUMO_DEBUG
set CLOPT=-Zi -nologo -Oi -Od -fp:fast /MD %DEFINES%
set LINKOPT=/SUBSYSTEM:CONSOLE /INCREMENTAL:NO /DEBUG
set DEPENDENCIES="SDL2.lib" "SDL2main.lib" "opengl32.lib" "user32.lib"
cl %INCLUDE_PATHS% %CLOPT% %1 /link %LINKOPT% %LIB_PATHS% %DEPENDENCIES% /OUT:%~n1.exe
popd
.\build\%~n1.exe
