@echo off
pushd .\build
set CommonCompilerFlags=/I"../src" /I"../../../sdl/include" -Zi -nologo -Oi -Od -fp:fast -MD -DSUMO_DEBUG
set CommonLinkerFlags=-subsystem:console -incremental:no -debug SDL2.lib SDL2main.lib opengl32.lib user32.lib /LIBPATH:"../../../sdl/lib/x86"

cl %CommonCompilerFlags% ../src/so_meta.cpp /link %CommonLinkerFlags% -out:so_meta.exe
so_meta %1 > generated_meta_code.h

cl %CommonCompilerFlags% %1 /link %CommonLinkerFlags% -out:%~n1.exe
popd
.\build\%~n1.exe
