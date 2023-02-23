#!/usr/bin/env python3

app_name = "nsui_banner_fixer"
description = "YANBF Generator"
version = "1.2"
imports = ["os", "subprocess", "argparse", "shutil"]
bdir = "../dist/nsui_banner_fixer"

############################################################################################

from cx_Freeze import setup, Executable
import pkgutil
import os
import shutil

#from https://stackoverflow.com/a/70638967
BasicPackages = ["collections","encodings","importlib"] + imports
def AllPackage(): 
    return [i.name for i in list(pkgutil.iter_modules()) if i.ispkg]; # Return name of all package

def notFound(A,v): # Check if v outside A
    try: 
        A.index(v)
        return False
    except:
        return True

# Dependencies are automatically detected, but it might need fine tuning.
# "packages": ["os"] is used as example only
man_excludes = []
excludes = [i for i in AllPackage() if notFound(BasicPackages,i)] + man_excludes
build_exe_options = {
    "includes": BasicPackages,
    "excludes": excludes,
    "build_exe": bdir}


# Dependencies are automatically detected, but it might need fine tuning.
# "packages": ["os"] is used as example only
#build_exe_options = {
#    #"packages": ["PIL", "requests"], 
#    "excludes": ["tkinter, PyQt5"], 
#    "build_exe": "dist"}

# base="Win32GUI" should be used only for Windows GUI app
base = None

setup(
    name = app_name,
    version = version,
    description="NSUI Banner Fixer",
    options={"build_exe": build_exe_options},
    executables=[Executable(script = "main.py", target_name = "nsui_banner_fixer.exe", base = base)],
)

os.mkdir(f"{bdir}/tools")
shutil.copy2("./tools/ctrtool.exe", f"{bdir}/tools")
shutil.copy2("./tools/3dstool.exe", f"{bdir}/tools")
shutil.copy2("./tools/makerom.exe", f"{bdir}/tools")

shutil.copy2("./tools/3dstool_license.txt", f"{bdir}/tools")
os.rename(f"{bdir}/frozen_application_license.txt", f"{bdir}/tools/frozen_application_license.txt")

if os.path.exists(f"{bdir}.{version}.zip"):
    inp = input("please increase version or type y to overwrite: ")
    if inp != "y":
        print("did not create a zip file")
        exit()
shutil.make_archive(f"{bdir}.{version}", 'zip', f"{bdir}")
