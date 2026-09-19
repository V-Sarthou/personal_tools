@echo off
setlocal enabledelayedexpansion

:: Check if the input is set
if "%~1"=="" (
    echo error: file or folder not provided
    goto :end
)

:: Check if the input exists
if not exist "%~1" (
    echo error: file or folder not found: "%~1"
    goto :end
)

:: Check if the input is a directory
if exist "%~1\" (
    for %%F in ("%~1\*.HIF") do (
        call :ConvertFile "%%~fF" "%%~dpnF_2K.jpg"
    )
) else (
    call :ConvertFile "%~1" "%~dpn1_2K.jpg"
)

:end
echo Finished
pause
goto :eof


:: ========================================================
:: SINGLE CONVERSION SUBROUTINE
:: ========================================================
:ConvertFile
echo Converting %~nx1 to %~nx2
magick "%~1" -resize 1920x1080 -quality 98 "%~2"
goto :eof
