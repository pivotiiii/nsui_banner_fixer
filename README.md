# NSUI Banner Fixer

This fixes GBA VC banners created with [New Super Ultimate Injector v28](https://gbatemp.net/threads/discussion-new-super-ultimate-injector-nsui.500376/post-9174080) not showing the VC text box for non-US consoles.

![HNI_0032](https://user-images.githubusercontent.com/17112987/231853029-00142486-cb25-452a-9734-05e5c277f149.jpg) ![HNI_0033](https://user-images.githubusercontent.com/17112987/231853046-1bb2a3f2-cd1b-4a2d-a6b6-1639a607d560.jpg)

WARNING: Overwriting an injected GBA game will overwrite its save file.
Consider backing up your saves before, e.g. with [GBAVCSM](https://github.com/TurdPooCharger/GBAVCSM).

# Download

The download can be found at the bottom of the releases section [HERE](https://github.com/pivotiiii/nsui_banner_fixer/releases/latest). 

There are two versions available:

The `.exe` version is a standalone executable that comes prepackaged with all necessary Python files and dependencies. It tends to be falsely identified as a threat by antivirus software because of that. You may need to create an exception for `nsui_banner_fixer.exe` in your antivirus software.

The `.py` version is just the plain Python script +  dependencies. As long as you have Python 3 (I am using 3.9, lower versions may work) installed on your system this works the same as the .exe but will not be identified as a virus. It also saves a few MB of disk space :)

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


