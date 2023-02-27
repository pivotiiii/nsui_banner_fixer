import os
import sys
import shutil
import subprocess

cia = "Kurukuru Kururin.cia"
locale_codes = [b"EUR_EN", b"EUR_FR", b"EUR_GE", b"EUR_IT", b"EUR_SP", b"EUR_DU", b"EUR_PO", b"EUR_RU", b"JPN_JP", b"USA_EN", b"USA_FR", b"USA_SP", b"USA_PO"]
locale_offsets = [0x14BC, 0x14CB]

def check_output():
    os.chdir("./tests")
    results = []
    tests = ["nsui_banner_fixer.exe", f"nsui_banner_fixer.exe {cia}", "nsui_banner_fixer.exe -r", f"nsui_banner_fixer.exe {cia} -r"]
    results.append(check_locales(1))
    results.append(check_locales(2))
    shutil.copy2(f"./{cia}", f"./{cia}_BAK.cia")
    results.append(check_locales(3))
    os.remove(f"./{cia}")
    shutil.copy2(f"./{cia}_BAK.cia", f"./{cia}")
    results.append(check_locales(4))
    os.chdir("..")
    return results, tests

def check_locales(id: int):
    cia_dir = "out/"
    if id == 1:
        subprocess.run(["nsui_banner_fixer.exe"])
    if id == 2:
        subprocess.run(["nsui_banner_fixer.exe", cia])
    if id == 3:
        subprocess.run(["nsui_banner_fixer.exe", "-r"])
        cia_dir = ""
    if id == 4:
        subprocess.run(["nsui_banner_fixer.exe", cia, "-r"])
        cia_dir = ""
    
    print(os.getcwd())
    subprocess.run(["tools/ctrtool", f"--contents=temp2/contents", f"{cia_dir}{cia}"])
    subprocess.run(["tools/3dstool", "-xv", "-t", "cxi", "-f", "temp2/contents.0000.00000000", "--exefs", "temp2/exefs.bin"])
    subprocess.run(["tools/3dstool", f"-xv", "-t", "exefs", "-f", "temp2/exefs.bin", "--exefs-dir", "temp2/exefs"])
    if os.path.exists(f"./temp2/exefs/banner.bnr"):
        banner_ext = "bnr"
    else:
        banner_ext = "bin"
    subprocess.run(["tools/3dstool", f"-xv", "-t", "banner", "-f", f"temp2/exefs/banner.{banner_ext}", "--banner-dir", "temp2/banner"])
    return read_locales()

def read_locales():
    for i in range(1, 14):
        with open(f"./temp2/banner/banner{i}.bcmdl", "r+b") as f:
            f.seek(locale_offsets[0], 0)
            data = f.read(6)
            print(f"banner{i}.bcmdl of {cia} at {hex(locale_offsets[0])} is {data}, should be {locale_codes[i-1]}")
            if data != locale_codes[i-1]:
                print("ERROR")
                return False
            f.seek(locale_offsets[1], 0)
            data = f.read(6)
            print(f"banner{i}.bcmdl of {cia} at {hex(locale_offsets[1])} is {data}, should be {locale_codes[i-1]}")    
            if data != locale_codes[i-1]:
                print("ERROR")
                return False
    return True
            

os.chdir("D:/Documents/VS Code/nsui_banner_fixer/build")
bdir = "./tests"
if os.path.exists(f"{bdir}"):
    shutil.rmtree(f"{bdir}")
os.mkdir(bdir)
os.mkdir(f"{bdir}/tools")
os.mkdir(f"{bdir}/temp2")
shutil.copy2("./tools/ctrtool.exe", f"{bdir}/tools")
shutil.copy2("./tools/3dstool.exe", f"{bdir}/tools")
shutil.copy2("./tools/makerom.exe", f"{bdir}/tools")
shutil.copy2("./nsui_banner_fixer.exe", f"{bdir}/nsui_banner_fixer.exe")
shutil.copy2(f"D:\Documents\VS Code\\nsui_banner_fixer\src\{cia}", f"{bdir}/{cia}")
results, tests = check_output()
print("")
for i in range(0, len(results)):
    print(f"{tests[i]} :  {results[i]}")
shutil.rmtree("./tests")
if False in results:
    sys.exit(1)
sys.exit(0)