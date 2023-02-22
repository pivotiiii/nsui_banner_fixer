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

locale_codes = [b"EUR_EN", b"EUR_FR", b"EUR_GE", b"EUR_IT", b"EUR_SP", b"EUR_DU", b"EUR_PO", b"EUR_RU", b"JPN_JP", b"USA_EN", b"USA_FR", b"USA_SP", b"USA_PO"]
locale_offsets = [0x14BC, 0x14CB]
verbose = False

class Game(object):
    def __init__(self, cia: str):
        self.cia_path = os.path.abspath(cia)
        self.name = os.path.splitext(os.path.basename(cia))[0]
        self.banner_ext = "bin"
        self.v = "v"
        if not verbose:
            self.v = ""
        os.mkdir("./temp")
        self.cwd = f"temp/{self.name}"
        


    def extract_cia(self):
        os.mkdir(f"./{self.cwd}")
        if verbose:
            subprocess.run(["ctrtool", f"--contents={self.cwd}/contents", f"{self.name}.cia"])
        else:
            subprocess.run(["ctrtool", f"--contents={self.cwd}/contents", f"{self.name}.cia"], stdout=subprocess.DEVNULL)
        
        subprocess.run([
            "3dstool", f"-x{self.v}", "-t", "cxi", "-f", f"{self.cwd}/contents.0000.00000000",
            "--header", f"{self.cwd}/ncch.header",
            "--exh", f"{self.cwd}/exheader.bin",
            "--exefs", f"{self.cwd}/exefs.bin",
            "--romfs", f"{self.cwd}/romfs.bin"])

        subprocess.run(["3dstool", f"-x{self.v}", "-t", "exefs", "-f", f"{self.cwd}/exefs.bin", "--header", f"{self.cwd}/exefs.header", "--exefs-dir", f"{self.cwd}/exefs"])
        
        os.mkdir(f"./{self.cwd}/banner")
        if os.path.exists(f"./{self.cwd}/exefs/banner.bnr"):
            self.banner_ext = "bnr"
        subprocess.run(["3dstool", f"-x{self.v}", "-t", "banner", "-f", f"{self.cwd}/exefs/banner.{self.banner_ext}", "--banner-dir", f"{self.cwd}/banner"])

    def edit_bcmdl(self):
        for i in range(1, 14):
            with open(f"./{self.cwd}/banner/banner{i}.bcmdl", "r+b") as f:
                f.seek(locale_offsets[0], 0)
                data = f.read(6)
                if verbose: print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[0])} was {data}")
                if data != b"USA_EN" and data != locale_codes[i-1]:
                    print("ERROR: wrong offset for locale string 1")
                    exit()
                f.seek(locale_offsets[0], 0)
                f.write(locale_codes[i-1])
                f.seek(locale_offsets[1], 0)
                data = f.read(6)
                if verbose: print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[1])} was {data}")
                if data != b"USA_EN" and data != locale_codes[i-1]:
                    print("ERROR: wrong offset for locale string 2")
                    exit()
                f.seek(locale_offsets[1], 0)
                f.write(locale_codes[i-1])
                if verbose:
                    f.seek(locale_offsets[0], 0)
                    print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[0])} is now {f.read(6)}")
                    f.seek(locale_offsets[1], 0)
                    print(f"banner{i}.bcmdl of {self.name}.cia at {hex(locale_offsets[1])} is now {f.read(6)}")
            

    def repack_cia(self):
        os.remove(f"./{self.cwd}/exefs/banner.{self.banner_ext}")
        subprocess.run([
            "3dstool", f"-c{self.v}", "-t", "banner", 
            "-f", f"{self.cwd}/exefs/banner.{self.banner_ext}", 
            "--banner-dir", f"{self.cwd}/banner"])

        subprocess.run([
            "3dstool", f"-c{self.v}", "-t", "exefs",
            "-f", f"{self.cwd}/exefs.bin",
            "--header", f"{self.cwd}/exefs.header",
            "--exefs-dir", f"{self.cwd}/exefs"])
        
        subprocess.run([
            "3dstool", f"-c{self.v}", "-t", "cxi", 
            "-f", f"{self.cwd}/{self.name}.cxi", 
            "--header", f"{self.cwd}/ncch.header", 
            "--exh", f"{self.cwd}/exheader.bin",
            "--exefs", f"{self.cwd}/exefs.bin",
            "--romfs", f"{self.cwd}/romfs.bin"])
        
        if not os.path.exists("./out"):
            os.mkdir("./out")
        subprocess.run(["makerom", "-f", "cia", "-o", f"out/{self.name}.cia", "-content", f"{self.cwd}/{self.name}.cxi:0:0x00"])
        shutil.rmtree(f"./temp")

def check_requirements():
    if not os.path.exists("./3dstool.exe"):
        print("ERROR: 3dstool.exe is missing")
    if not os.path.exists("./ctrtool.exe"):
        print("ERROR: ctrtool.exe is missing!")
    if not os.path.exists("./makerom.exe"):
        print("ERROR: makerom.exe is missing!")

def get_cias() -> list:
    return [os.path.abspath(f) for f in os.listdir(".") if f.endswith(".cia")]

def clean_dirs():
    if os.path.exists(f"./temp"):
            shutil.rmtree(f"./temp")


if __name__ == "__main__":
    check_requirements()
    
    parser = argparse.ArgumentParser(
        prog = "nsui_banner_fixer.exe", 
        description = "NSUI Banner Fixer",
        epilog = "Either supply a .cia file as argument or place all the files you want to convert next to nsui_banner_fixer.exe.")
    parser.add_argument("input", metavar = "input.cia", type = str, nargs = "*", help = "path to a .cia file")
    parser.add_argument("-v", "--verbose", action = "store_true", help = "show more information while fixing cia")

    args = parser.parse_args()
    verbose = args.verbose
    if args.input:
        cia = args.input[0]
        if not cia.endswith(".cia"):
            print("ERROR: did you pass a .cia file?")
            exit()
        if not os.path.exists(cia):
            print("ERROR: could not find .cia file")
            exit()
        files = [cia]
    else:
        files = get_cias()

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
        print(f"--- done --> saved at out/{name}.cia")
    print("\nWARNING! Overwriting an injected GBA game will overwrite its save file.")
    print("Consider backing up your saves before, e.g. with GBAVCSM (https://github.com/TurdPooCharger/GBAVCSM)")