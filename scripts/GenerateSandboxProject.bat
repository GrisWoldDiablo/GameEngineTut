@echo off
echo --- Generating Sandbox Solution ---
pushd %~dp0\..\Hazelnut\SandboxProject\Assets\Scripts\
call ..\..\..\..\vendor\premake\bin\premake5.exe vs2022
popd
PAUSE