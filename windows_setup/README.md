# Software

- [7-Zip](https://www.7-zip.org/download.html)
- [Adobe Reader](https://get.adobe.com/reader/)
- [Audacity](https://www.audacityteam.org/download/windows/)
- [CDRTFE](https://cdrtfe.sourceforge.io/cdrtfe/download_en.html)
- [ffmpeg](https://github.com/BtbN/FFmpeg-Builds/releases/latest)
- [Filezilla](https://filezilla-project.org/download.php?show_all=1)
- [Firefox](https://download.mozilla.org/?product=firefox-stub&os=win)
- [Foobar 2000](https://www.foobar2000.org/download)
- [GIMP](https://www.gimp.org/downloads/)
- [Git](https://git-scm.com/download/win)
- [Inkscape](https://inkscape.org/release/)
- [IrfanView](https://www.fosshub.com/IrfanView.html)
- [JDK](https://adoptium.net/)
- [LibreOffice](https://libreoffice.org/download)
- [Link Shell Extension](https://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html#download)
- [LockHunter](https://lockhunter.com/download.htm)
- [MPC-HC & codecs](https://codecguide.com/download_kl.htm)
- [Notepad++](https://notepad-plus-plus.org/downloads/)
- [Nvidia driver](https://www.nvidia.com/Download/index.aspx)
- [OBS](https://github.com/obsproject/obs-studio/releases/latest)
- [Open Shell](https://github.com/Open-Shell/Open-Shell-Menu/releases/latest)
- [PDFSam](https://pdfsam.org/download-pdfsam-basic/)
- [Process Explorer](https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer)
- [Python](https://www.python.org/downloads/)
- [Rapid Environment Editor](https://www.rapidee.com/en/download)
- [Transmission](https://transmissionbt.com/download)
- [WinCDEmu](https://wincdemu.sysprogs.org/download/)

# System setup

- Disable LUA (see .reg file)
- Disable LockScreen (see .reg file)
- File Explorer Options
- Disable UAC
- Disable OneDrive
- TaskBar
  - disable combine
  - display all tray icons
  - Hide search
  - Remove news & interests
- Default Apps
- Disable telemetry:
  - taskschd.msc
  - Disable Library/Microsoft/Windows/Application Experience/Microsoft Compatibility Appraiser
  - Launch
    ```
    sc delete DiagTrack
    sc delete dmwappushservice
    echo “” >
    C:\ProgramData\Microsoft\Diagnosis\ETLLogs\AutoLogger\AutoLogger-Diagtrack-Listener.etl
    reg add HKLM\SOFTWARE\Policies\Microsoft\Windows\DataCollection /v AllowTelemetry /t REG_DWORD /d 0 /f
    ```
