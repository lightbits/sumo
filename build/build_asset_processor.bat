@echo off
pushd .\build
cl -nologo ../src/assets/asset_processor.cpp /EHsc /link -out:asset_processor.exe
popd
.\build\asset_processor.exe
