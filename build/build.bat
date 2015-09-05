@echo off
pushd .\build
set CommonCompilerFlags=/I"../src" /I"../../../sdl/include" -Zi -nologo -Oi -Od -fp:fast -MD -DSUMO_DEBUG
set CommonLinkerFlags=-subsystem:console -incremental:no -debug SDL2.lib SDL2main.lib opengl32.lib user32.lib /LIBPATH:"../../../sdl/lib/x86"
cl %CommonCompilerFlags% %1 /link %CommonLinkerFlags% -out:%~n1.exe
popd
.\build\%~n1.exe
