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

locale_codes = [b"EUR_EN", b"EUR_FR", b"EUR_GE", b"EUR_IT", b"EUR_SP", b"EUR_DU", b"EUR_PO", b"EUR_RU", b"JPN_JP", b"USA_EN", b"USA_FR", b"USA_SP", b"USA_PO"]
locale_offsets = [0x14BC, 0x14CB]
verbose = True

class Game(object):
    def __init__(self, cia: str):
        self.cia_path = os.path.abspath(cia)
        self.name = os.path.splitext(cia)[0]
        self.banner_ext = "bin"
        self.v = "v"
        if not verbose:
            self.v = ""


    def extract_cia(self):
        os.mkdir(f"./{self.name}")
        if verbose:
            subprocess.run(["ctrtool", f"--contents={self.name}/contents", f"{self.name}.cia"], stdout=subprocess.DEVNULL)
        else:
            subprocess.run(["ctrtool", f"--contents={self.name}/contents", f"{self.name}.cia"])
        
        subprocess.run([
            "3dstool", f"-x{self.v}", "-t", "cxi", "-f", f"{self.name}/contents.0000.00000000",
            "--header", f"{self.name}/ncch.header",
            "--exh", f"{self.name}/exheader.bin",
            "--exefs", f"{self.name}/exefs.bin",
            "--romfs", f"{self.name}/romfs.bin"])

        subprocess.run(["3dstool", f"-x{self.v}", "-t", "exefs", "-f", f"{self.name}/exefs.bin", "--header", f"{self.name}/exefs.header", "--exefs-dir", f"{self.name}/exefs"])
        
        os.mkdir(f"./{self.name}/banner")
        if os.path.exists(f"./{self.name}/exefs/banner.bnr"):
            self.banner_ext = "bnr"
        subprocess.run(["3dstool", f"-x{self.v}", "-t", "banner", "-f", f"{self.name}/exefs/banner.{self.banner_ext}", "--banner-dir", f"{self.name}/banner"])

    def edit_bcmdl(self):
        for i in range(1, 14):
            with open(f"./{self.name}/banner/banner{i}.bcmdl", "r+b") as f:
                f.seek(locale_offsets[0], 0)
                f.write(locale_codes[i-1])
                f.seek(locale_offsets[1], 0)
                f.write(locale_codes[i-1])

    def repack_cia(self):
        os.remove(f"./{self.name}/exefs/banner.{self.banner_ext}")
        subprocess.run([
            "3dstool", f"-c{self.v}", "-t", "banner", 
            "-f", f"{self.name}/exefs/banner.{self.banner_ext}", 
            "--banner-dir", f"{self.name}/banner"])

        subprocess.run([
            "3dstool", f"-c{self.v}", "-t", "exefs",
            "-f", f"{self.name}/exefs.bin",
            "--header", f"{self.name}/exefs.header",
            "--exefs-dir", f"{self.name}/exefs"])
        
        subprocess.run([
            "3dstool", f"-c{self.v}", "-t", "cxi", 
            "-f", f"{self.name}/{self.name}.cxi", 
            "--header", f"{self.name}/ncch.header", 
            "--exh", f"{self.name}/exheader.bin",
            "--exefs", f"{self.name}/exefs.bin",
            "--romfs", f"{self.name}/romfs.bin"])
        
        if not os.path.exists("./out"):
            os.mkdir("./out")
        subprocess.run(["makerom", "-f", "cia", "-o", f"out/{self.name}.cia", "-content", f"{self.name}/{self.name}.cxi:0:0x00"])

def check_requirements():
    if not os.path.exists("./3dstool.exe"):
        print("ERROR: 3dstool.exe is missing")
    if not os.path.exists("./ctrtool.exe"):
        print("ERROR: ctrtool.exe is missing!")
    if not os.path.exists("./makerom.exe"):
        print("ERROR: makerom.exe is missing!")

if __name__ == "__main__":
    check_requirements()
    try:
        shutil.rmtree("./Kurukuru Kururin")
    except FileNotFoundError:
        print("dir is already gone")
    game = Game("Kurukuru Kururin.cia")
    game.extract_cia()
    game.edit_bcmdl()
    game.repack_cia()