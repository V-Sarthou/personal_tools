#!/usr/bin/env python

import urllib.request
import shutil
import os
import time
import sys
import subprocess
import ssl

url_pattern = "https://vs34.userscontent.net/dash/609473/fragment-{i}-f2-v1-x3.m4s"
range_vals = [1, 271]


def init_dir(dir):
  try:
    os.stat(dir)
  except:
    os.makedirs(dir)

cwd = os.getcwd()
dir = "segments"
file_pattern = "seg{i}.m4s"
file_list = []
index_range = range(range_vals[0], range_vals[1] + 1)
for i in index_range:
  file_list.append("{dir}/{file}".format(dir=dir, file=file_pattern.format(i=i)))

context = ssl._create_unverified_context()
do_download = True
if do_download:
  init_dir(os.path.join(cwd, dir))
  download_max_tries = 10
  for local_file, i in zip(file_list, index_range):
    url = str(url_pattern).format(i=i)
    print(url)
    print(local_file)
    sys.stdout.flush()
    download_ok = False
    download_num_tries = 0
    while not download_ok:
      try:
        with urllib.request.urlopen(url, context=context) as response, open(local_file, 'wb') as out_file:
          shutil.copyfileobj(response, out_file)
      except TimeoutError:
        print("Could not download file. Retrying...")
        time.sleep(1)
        download_num_tries += 1
        if download_num_tries > download_max_tries:
          raise
      else:
        download_ok = True

do_combine = True
if do_combine:
  combine_cmd = ["ffmpeg", "-i", "concat:{file_list}".format(file_list='|'.join(file_list)), "-c", "copy", "output.mp4"]
  print(' '.join(combine_cmd))
  sys.stdout.flush()
  subprocess.call(combine_cmd)
