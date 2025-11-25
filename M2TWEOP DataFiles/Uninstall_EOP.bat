@echo off
echo WARNING!!! THIS FILE WILL DELETE ALL EOP FILES. MAKE BACKUPS OF ANY SCRIPTS YOU WISH TO PRESERVE FIRST BEFORE CONTUINING!

set /p userResponse=Do you really want to uninstall all EOP files? (Y/N): 

if /i "%userResponse%"=="Y" (
    echo You chose to continue.
    rmdir /s /q  "eopData"
    rmdir /s /q  "youneuoyData"
    DEL /F /Q "M2TWEOP_GUI.exe"
    DEL /F /Q "M2TWEOP GUI.exe"
    DEL /F /Q "M2TWEOPLibrary.dll"
    DEL /F /Q "M2TWEOPLibrary.pdb"
    DEL /F /Q "imgui.ini"
) else (
    echo You chose to cancel.
)

pause