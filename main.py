#extract .cia
#ctrtool --contents=contents <nameOfCia>.cia
#ctrtool --romfsdir=romfs --exefsdir=exefs --exheader=exheader.bin contents.0000.00000000
#extract banner



#create .cxi from .bin files
#3dstool -cvtf cxi CIA.cxi --header ncch.header --exh exheader.bin --exefs exefs.bin --romfs romfs.bin --logo logo.bcma.lz
#create .cia from .cxi
#makerom -f cia -o whatever.cia -content CIA.cxi:0:0x00

import os
import subprocess

class Game(object):
    def __init__(self, cia: str):
        self.cia_path = os.path.abspath(cia)
        self.name = os.path.basename(cia)
        self.work_dir = f"/{self.name}"

    def extract_cia(self):
        subprocess.run(["ctrtool", "--contents=contents", f"{self.name}.cia"])
        os.rename("contents.0000.00000000", f"{self.name}/contents.0000.00000000")



def check_requirements():
    return

if __name__ == "__main__":
    game = Game("OG.cia")
    game.extract_cia()