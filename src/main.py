#extract .cia
#ctrtool --contents=contents <nameOfCia>.cia
#ctrtool --romfsdir=romfs --exefsdir=exefs --exheader=exheader.bin contents.0000.00000000
#extract banner
#3dstool  -xv -t banner -f "D:\Documents\VS Code\nsui_banner_fixer\src\OG\exefs\banner.bin" --banner-dir "D:\Documents\VS Code\nsui_banner_fixer\src\OG\banner"


#create .cxi from .bin files
#3dstool -cvtf cxi CIA.cxi --header ncch.header --exh exheader.bin --exefs exefs.bin --romfs romfs.bin --logo logo.bcma.lz
#create .cia from .cxi
#makerom -f cia -o whatever.cia -content CIA.cxi:0:0x00

import os
import subprocess
import shutil
import argparse
import re
import sys

locale_codes = [b"EUR_EN", b"EUR_FR", b"EUR_GE", b"EUR_IT", b"EUR_SP", b"EUR_DU", b"EUR_PO", b"EUR_RU", b"JPN_JP", b"USA_EN", b"USA_FR", b"USA_SP", b"USA_PO"]
locale_offsets = [0x14BC, 0x14CB]

verbose = False
replace = False

if "nsui_banner_fixer.exe" in sys.argv[0]:      #.exe nuitka
    script_dir = os.path.abspath(sys.argv[0])
else:                                           #.py
    script_dir = os.path.abspath(__file__)
temp_dir = os.path.join(os.getcwd(), "temp")

dstool = os.path.join(os.path.dirname(script_dir), "tools", "3dstool.exe")
ctrtool = os.path.join(os.path.dirname(script_dir), "tools", "ctrtool.exe")
makerom = os.path.join(os.path.dirname(script_dir), "tools", "makerom.exe")

class Game(object):
    def __init__(self, cia: str):
        self.cia_path = os.path.abspath(cia)
        self.name = os.path.splitext(os.path.basename(cia))[0]
        self.banner_ext = "bin"
        self.v = "v" if verbose else ""
        self.cwd = os.path.join(temp_dir, self.name)
        self.version = self.get_version()
        
    def get_version(self):
        output = subprocess.check_output([ctrtool, "-i", self.cia_path])
        for line in [bytes.decode(sys.stdout.encoding) for bytes in output.splitlines()]:
            if line.startswith("Title version:"):
                try:
                    match = re.search(r"(\d+).(\d+).(\d+)", line)
                    major = max(min(63, int(match.group(1))), 0) #values 0-63 https://github.com/3DSGuy/Project_CTR/blob/master/makerom/README.md#:~:text=%2Dmajor%20%3Cversion%3E,micro%20for%20the%20title.
                    minor = max(min(63, int(match.group(2))), 0) #values 0-63
                    micro = max(min(15, int(match.group(3))), 0) #values 0-15
                    return (major, minor, micro)
                except (AttributeError, IndexError):
                    break
        return (0, 0, 0)

    def extract_cia(self):
        os.mkdir(self.cwd)
        if verbose:
            subprocess.run([ctrtool, f"--contents={self.cwd}/contents", self.cia_path])
        else:
            subprocess.run([ctrtool, f"--contents={self.cwd}/contents", self.cia_path], stdout=subprocess.DEVNULL)
        
        subprocess.run([
            dstool, f"-x{self.v}", "-t", "cxi", "-f", f"{self.cwd}/contents.0000.00000000",
            "--header", f"{self.cwd}/ncch.header",
            "--exh", f"{self.cwd}/exheader.bin",
            "--exefs", f"{self.cwd}/exefs.bin",
            "--romfs", f"{self.cwd}/romfs.bin"])

        subprocess.run([dstool, f"-x{self.v}", "-t", "exefs", "-f", f"{self.cwd}/exefs.bin", "--header", f"{self.cwd}/exefs.header", "--exefs-dir", f"{self.cwd}/exefs"])
        
        os.mkdir(os.path.join(self.cwd, "banner"))
        if os.path.exists(os.path.join(self.cwd, "exefs", "banner.bnr")):
            self.banner_ext = "bnr"
        subprocess.run([dstool, f"-x{self.v}", "-t", "banner", "-f", f"{self.cwd}/exefs/banner.{self.banner_ext}", "--banner-dir", f"{self.cwd}/banner"])

    def edit_bcmdl(self):
        for i in range(1, 14):
            with open(os.path.join(self.cwd, "banner", f"banner{i}.bcmdl"), "r+b") as f:
                f.seek(locale_offsets[0], 0)
                data = f.read(6)
                if verbose: print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[0])} was {data}")
                if data != b"USA_EN" and data != locale_codes[i-1]:
                    finish("ERROR: wrong offset for locale string 1")
                f.seek(locale_offsets[0], 0)
                f.write(locale_codes[i-1])
                f.seek(locale_offsets[1], 0)
                data = f.read(6)
                if verbose: print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[1])} was {data}")
                if data != b"USA_EN" and data != locale_codes[i-1]:
                    finish("ERROR: wrong offset for locale string 2")
                f.seek(locale_offsets[1], 0)
                f.write(locale_codes[i-1])
                if verbose:
                    f.seek(locale_offsets[0], 0)
                    print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[0])} is now {f.read(6)}")
                    f.seek(locale_offsets[1], 0)
                    print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[1])} is now {f.read(6)}")

    def repack_cia(self):
        os.remove(os.path.join(self.cwd, "exefs", f"banner.{self.banner_ext}"))
        subprocess.run([
            dstool, f"-c{self.v}", "-t", "banner", 
            "-f", f"{self.cwd}/exefs/banner.{self.banner_ext}", 
            "--banner-dir", f"{self.cwd}/banner"])

        subprocess.run([
            dstool, f"-c{self.v}", "-t", "exefs",
            "-f", f"{self.cwd}/exefs.bin",
            "--header", f"{self.cwd}/exefs.header",
            "--exefs-dir", f"{self.cwd}/exefs"])
        
        subprocess.run([
            dstool, f"-c{self.v}", "-t", "cxi", 
            "-f", f"{self.cwd}/{self.name}.cxi", 
            "--header", f"{self.cwd}/ncch.header", 
            "--exh", f"{self.cwd}/exheader.bin",
            "--exefs", f"{self.cwd}/exefs.bin",
            "--romfs", f"{self.cwd}/romfs.bin"])
        
        
        if replace:
            out_cia = self.cia_path
        else:
            out_dir = os.path.join(os.getcwd(), "out")
            if not os.path.exists(out_dir):
                os.mkdir(os.path.join(out_dir))
            out_cia = os.path.join(out_dir, f"{self.name}.cia")
        content_path_rel = os.path.relpath(os.path.join(self.cwd, f"{self.name}.cxi")) #makerom doesn't work with absolute paths because of the colon after the drive letter :(
        subprocess.run([makerom, "-f", "cia", 
                        "-o", out_cia, 
                        "-content", f"{content_path_rel}:0:0x00", 
                        "-major", str(self.version[0]),
                        "-minor", str(self.version[1]), 
                        "-micro", str(self.version[2])])
        shutil.rmtree(temp_dir)

    def fix_banner(self):
        clean_dirs()
        os.mkdir(temp_dir)
        print(self.cia_path)
        print("--- extracting cia")
        self.extract_cia()
        print("--- editing banner")
        self.edit_bcmdl()
        print("--- repacking cia")
        self.repack_cia()
        if replace:
            print(f"--- done --> replaced {self.name}.cia")
        else:
            print(f"--- done --> saved at out/{self.name}.cia")
        clean_dirs()

def check_requirements():
    if not os.path.exists(dstool):
        finish("ERROR: 3dstool.exe is missing")
    if not os.path.exists(ctrtool):
        finish("ERROR: ctrtool.exe is missing!")
    if not os.path.exists(makerom):
        finish("ERROR: makerom.exe is missing!")

def get_cias(args) -> list:
    if args.input:
        cia = args.input[0]
        if not cia.endswith(".cia"):
            finish("ERROR: did you pass a .cia file?")
        if not os.path.exists(cia):
            finish("ERROR: could not find .cia file")
        return [cia]
    else:
        return [os.path.abspath(f) for f in os.listdir(os.getcwd()) if f.endswith(".cia")]

def clean_dirs():
    if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)

def finish(err_msg = None):
    if err_msg:
        print("")
        print(err_msg)
    if os.name == 'nt' and 'PROMPT' not in os.environ: #launched from explorer
        os.system('pause')  
    raise SystemExit


if __name__ == "__main__":
    check_requirements()
    
    parser = argparse.ArgumentParser(
        prog = "nsui_banner_fixer.exe", 
        description = "NSUI Banner Fixer",
        epilog = "Either supply a .cia file as an argument or place all the files you want to convert in your current working directory.")
    parser.add_argument("input", metavar = "input.cia", type = str, nargs = "*", help = "path to a .cia file")
    parser.add_argument("-v", "--verbose", action = "store_true", help = "show more information while fixing cia")
    parser.add_argument("-r", "--replace", action = "store_true", help = "replace .cia files instead of saving to /out")

    args = parser.parse_args()
    verbose = args.verbose
    replace = args.replace
    files = get_cias(args)

    if len(files) == 0:
        parser.print_help()
        finish()
    
    for cia in files:
        game = Game(cia)
        try:
            game.fix_banner()
        except FileNotFoundError:
            print(f"ERROR: Was {game.name}.cia created with NSUI v28 using the 3D GBA banner?")
    
    print("\nWARNING! Overwriting an injected GBA game will overwrite its save file.")
    print("Consider backing up your saves before, e.g. with GBAVCSM (https://github.com/TurdPooCharger/GBAVCSM)")
    finish()