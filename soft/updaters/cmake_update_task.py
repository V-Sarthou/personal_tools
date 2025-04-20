#!python -u

import subprocess
import json
import webbrowser
import sys

try:
    from urllib2 import urlopen
except ImportError: # python 3
    from urllib.request import urlopen

try:
    from tkinter import messagebox
except ImportError:
    sys.stderr.write("Could not import tkinter.messagebox.\nInstall it with:\npython -m pip install tkinter\n")
    sys.exit(1)


def askyesno(question, title="New CMake version"):
        return messagebox.askyesno(title, question)


def greater_than(a1, a2):
    for (e1, e2) in zip(a1, a2):
        if e1 != e2:
            return (int(e1) > int(e2))
    return False


if __name__ == '__main__':
    cur_cmake_ver = subprocess.check_output(['cmake', '--version']).decode('utf-8').splitlines()[0].split()[2].split('.')
    print("Current CMake version: {ver}".format(ver='.'.join(cur_cmake_ver)))

    base_url = "https://cmake.org/files/LatestRelease/"
    info_file = "cmake-latest-files-v1.json"
    with urlopen("{base_url}{info_file}".format(base_url=base_url, info_file=info_file)) as f:
        cmake_files_json = json.loads(f.read())
        last_cmake_ver_j = cmake_files_json["version"]
        last_cmake_ver = [str(last_cmake_ver_j["major"]), str(last_cmake_ver_j["minor"]), str(last_cmake_ver_j["patch"])]
        print("Available CMake version: {ver}".format(ver='.'.join(last_cmake_ver)))
        if greater_than(last_cmake_ver, cur_cmake_ver):
            for file_info in cmake_files_json["files"]:
                if file_info["os"][0] == "windows" and file_info["architecture"][0] == "x86_64" and file_info["class"] == "installer":
                    if askyesno("A new version of CMake is available: {ver}.\nWould you like to download it?".format(ver='.'.join(last_cmake_ver))):
                        webbrowser.open("{base_url}{file_name}".format(base_url=base_url, file_name=file_info["name"]))
