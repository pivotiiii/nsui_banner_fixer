# NSUI Banner Fixer

This fixes GBA VC banners created with [New Super Ultimate Injector v28](https://gbatemp.net/threads/discussion-new-super-ultimate-injector-nsui.500376/post-9174080) not showing the VC text box for non-US consoles.

![HNI_0032](https://user-images.githubusercontent.com/17112987/231853029-00142486-cb25-452a-9734-05e5c277f149.jpg) ![HNI_0033](https://user-images.githubusercontent.com/17112987/231853046-1bb2a3f2-cd1b-4a2d-a6b6-1639a607d560.jpg)

> [!Warning]
> Overwriting an injected GBA game will overwrite its save file.
> Consider backing up your saves before, e.g. with [GBAVCSM](https://github.com/TurdPooCharger/GBAVCSM).

# Download

**The download can be found at the bottom of the releases section [HERE](https://github.com/pivotiiii/nsui_banner_fixer/releases/latest).**

There are two versions available:

Any version from 2.0 onwards is written in C++ and should work out of the box. This is the recommended version.

Any version before 2.0 is written in Python and tends to be falsely identified as a threat by antivirus software. If you used these versions before and encounter any errors with the new C++ version, the download for the last Python version is still available [here](https://github.com/pivotiiii/nsui_banner_fixer/releases/tag/v1.4.1) both as a standalone executable or as a plain Python 3.6+ script with dependencies.

# Usage

### Double clicking the .exe or .py file:

If you put all .cia files you want to fix into the same directory as `nsui_banner_fixer.exe/.py` you can simply double click and all fixed .cia files will be saved in the newly created `out` directory.

### Command line:

For a single .cia file: `nsui_banner_fixer.exe example.cia`

For all .cia files in your current working directory: `nsui_banner_fixer.exe`

By default your fixed .cia files will be created in the `out` directory of your current working directory.

Use the `-r` argument to instead edit the .cia files directly without making a copy.

| Argument      | Explanation                                                           |
|---------------|-----------------------------------------------------------------------|
| `-r`, `--replace` | Fix the .cia file(s) directly instead of saving a fixed copy in `/out`. |
| `-v`, `--verbose` | Display more output as the program is working.                        |
|  `-q`, `--quiet`  | Silence any non error output.                                         |
|   `-h`, `--help`  | Display a help message with usage instructions.                       |
|   `--version`   | Display the program version.                                          |
|   `--licenses`  | Display license information.                                          |
