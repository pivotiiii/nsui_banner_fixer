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
import psutil # type: ignore
import re
import sys

locale_codes = [b"EUR_EN", b"EUR_FR", b"EUR_GE", b"EUR_IT", b"EUR_SP", b"EUR_DU", b"EUR_PO", b"EUR_RU", b"JPN_JP", b"USA_EN", b"USA_FR", b"USA_SP", b"USA_PO"]
locale_offsets = [0x14BC, 0x14CB]
verbose = False
replace = False
script_dir = os.path.dirname(os.path.abspath(__file__))
temp_dir = os.path.join(script_dir, "temp")
dstool = os.path.join(script_dir, "tools", "3dstool.exe")
ctrtool = os.path.join(script_dir, "tools", "ctrtool.exe")
makerom = os.path.join(script_dir, "tools", "makerom.exe")

class Game(object):
    def __init__(self, cia: str):
        self.cia_path = os.path.abspath(cia)
        print(self.cia_path)
        self.name = os.path.splitext(os.path.basename(cia))[0]
        self.banner_ext = "bin"
        self.v = "v"
        if not verbose:
            self.v = ""
        try:
            os.mkdir(temp_dir)
        except FileExistsError:
            clean_dirs()
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
        try:
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
        except FileNotFoundError:
            finish(f"ERROR: Could not find ./{self.cwd}/banner/banner{i}.bcmdl after extracting the banner.\nWas {self.name}.cia created with NSUI v28 using the 3D GBA banner?")

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
        
        if not os.path.exists(os.path.join(script_dir, "out")):
            os.mkdir(os.path.join(script_dir, "out"))
        if replace:
            out_cia = self.cia_path
        else:
            out_cia = os.path.join(script_dir, "out", f"{self.name}.cia").replace("\\\\", "/")
        subprocess.run([makerom, "-f", "cia", 
                        "-o", out_cia, 
                        "-content", f"{self.cwd}/{self.name}.cxi:0:0x00", 
                        "-major", str(self.version[0]),
                        "-minor", str(self.version[1]), 
                        "-micro", str(self.version[2])])
        shutil.rmtree(temp_dir)

def check_requirements():
    if not os.path.exists(dstool):
        finish("ERROR: 3dstool.exe is missing")
    if not os.path.exists(ctrtool):
        finish("ERROR: ctrtool.exe is missing!")
    if not os.path.exists(makerom):
        finish("ERROR: makerom.exe is missing!")

def get_cias() -> list:
    return [os.path.abspath(f) for f in os.listdir(script_dir) if f.endswith(".cia")]

def clean_dirs():
    if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)

def finish(err_msg = None):
    if err_msg:
        print("")
        print(err_msg)
    if psutil.Process(os.getpid()).parent().parent().name() == "explorer.exe":
        os.system('pause')        
    raise SystemExit


if __name__ == "__main__":
    check_requirements()
    
    parser = argparse.ArgumentParser(
        prog = "nsui_banner_fixer.exe", 
        description = "NSUI Banner Fixer",
        epilog = "Either supply a .cia file as argument or place all the files you want to convert next to nsui_banner_fixer.exe.")
    parser.add_argument("input", metavar = "input.cia", type = str, nargs = "*", help = "path to a .cia file")
    parser.add_argument("-v", "--verbose", action = "store_true", help = "show more information while fixing cia")
    parser.add_argument("-r", "--replace", action = "store_true", help = "replace .cia files instead of saving to /out")

    args = parser.parse_args()
    verbose = args.verbose
    replace = args.replace
    if args.input:
        cia = args.input[0]
        if not cia.endswith(".cia"):
            finish("ERROR: did you pass a .cia file?")
        if not os.path.exists(cia):
            finish("ERROR: could not find .cia file")
        files = [cia]
    else:
        files = get_cias()

    if len(files) == 0:
        parser.print_help()
        finish()
    clean_dirs()
    for cia in files:
        name = os.path.splitext(os.path.basename(cia))[0]
        print(f"\n{name}.cia")
        game = Game(cia)
        print("--- extracting cia")
        game.extract_cia()
        print("--- editing banner")
        game.edit_bcmdl()
        print("--- repacking cia")
        game.repack_cia()
        if replace:
            print(f"--- done --> replaced {name}.cia")
        else:
            print(f"--- done --> saved at out/{name}.cia")
    print("\nWARNING! Overwriting an injected GBA game will overwrite its save file.")
    print("Consider backing up your saves before, e.g. with GBAVCSM (https://github.com/TurdPooCharger/GBAVCSM)")
    finish()