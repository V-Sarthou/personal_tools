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


def askyesno(question, title="New Git version"):
        return messagebox.askyesno(title, question)


def greater_than(a1, a2):
    for (e1, e2) in zip(a1, a2):
        if e1 != e2:
            return (int(e1) > int(e2))
    return False


if __name__ == '__main__':
    cur_ver = subprocess.check_output(['git', '--version']).decode('utf-8').splitlines()[0].split()[2].split('.')[:3]
    print("Current Git version: {ver}".format(ver='.'.join(cur_ver)))

    with urlopen("https://api.github.com/repos/git-for-windows/git/releases/latest") as f:
        files_json = json.loads(f.read())
        last_ver = files_json["tag_name"].split('.')
        last_ver[0] = last_ver[0].replace("v", "")
        print("Available Git version: {ver}".format(ver='.'.join(last_ver)))
        if greater_than(last_ver, cur_ver):
            for file_info in files_json["assets"]:
                if file_info["name"][-10:] == "64-bit.exe":
                    if askyesno("A new version of Git is available: {ver}.\nWould you like to download it?".format(ver='.'.join(last_ver))):
                        webbrowser.open(file_info["browser_download_url"])
