#!/usr/bin/env python3

app_name = "nsui_banner_fixer"
description = "YANBF Generator"
version = "1.0.0"
imports = ["os", "subprocess", "argparse", "shutil"]

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
    "build_exe": "../dist"}


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

os.mkdir("../dist/tools")
shutil.copy2("./tools/ctrtool.exe", "../dist/tools")
shutil.copy2("./tools/3dstool.exe", "../dist/tools")
shutil.copy2("./tools/makerom.exe", "../dist/tools")

shutil.copy2("./tools/3dstool_license.txt", "../dist/tools")
shutil.copy2("../dist/frozen_application_license.txt", "../dist/tools")