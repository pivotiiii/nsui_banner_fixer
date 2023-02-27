# NSUI Banner Fixer

This fixes GBA VC banners created with [New Super Ultimate Injector v28](https://gbatemp.net/threads/discussion-new-super-ultimate-injector-nsui.500376/post-9174080) not showing the VC text box for non-US consoles.

WARNING: Overwriting an injected GBA game will overwrite its save file.
Consider backing up your saves before, e.g. with [GBAVCSM](https://github.com/TurdPooCharger/GBAVCSM).

# Usage

Command line:

For a single .cia file: `nsui_banner_fixer.exe example.cia`

For all .cia files in the same directory as the .exe: `nsui_banner_fixer.exe`

Double clicking the executable also fixes all .cia files in the directory.

By default your fixed .cia files will be created in the `out` directory.

Use the `-r` argument to instead edit the .cia files directly without making a copy.

Use the `-v` argument to see more info during the process.

#  Requirements

Visual C++ Redistributable for Visual Studio 2017, 2019 or 2022: You most  likely already have this installed, but in case you get any errors about missing .dll files, you can download the installer for [64bit](https://aka.ms/vs/16/release/vc_redist.x64.exe) or [32bit](https://aka.ms/vs/16/release/vc_redist.x86.exe) directly from Microsoft.
