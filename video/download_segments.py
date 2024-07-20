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

do_download = True
do_combine = True

def init_dir(dir):
  try:
    os.stat(dir)
  except:
    os.makedirs(dir)

cwd = os.getcwd()
dir = "segments"
file_pattern = "seg{i}.ts"
file_list = []
#file_list.append("segments/init-a1-x3.mp4")
index_range = range(range_vals[0], range_vals[1] + 1)
for i in index_range:
  file_list.append("{dir}/{file}".format(dir=dir, file=file_pattern.format(i=i)))

final_file_list = []

if do_download:
  context = ssl._create_unverified_context()
  init_dir(os.path.join(cwd, dir))
  download_max_tries = 10
  for local_file, i in zip(file_list, index_range):
    url = str(url_pattern).format(i=i)
    #url = str(url_pattern).format(i=i, j=(1 + i%4))
    print(url)
    print(local_file)
    sys.stdout.flush()
    download_ok = False
    download_num_tries = 0
    while not download_ok:
      try:
        h = {
          "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:123.0) Gecko/20100101 Firefox/123.0",
        }
        r = urllib.request.Request(url, headers=h)
        with urllib.request.urlopen(r, context=context) as response, open(local_file, 'wb') as out_file:
          shutil.copyfileobj(response, out_file)
      except TimeoutError:
        print("Could not download file. Retrying...")
        sys.stdout.flush()
        time.sleep(1)
        download_num_tries += 1
        if download_num_tries > download_max_tries:
          raise
      except Exception as e:
        print(str(e))
        sys.stdout.flush()
        download_ok = True
      else:
        final_file_list.append(local_file)
        download_ok = True
else:
  final_file_list = file_list

if do_combine:
  combine_cmd = ["ffmpeg", "-i", "concat:{file_list}".format(file_list='|'.join(final_file_list)), "-c", "copy", "output.mp4"]
  print(' '.join(combine_cmd))
  sys.stdout.flush()
  subprocess.call(combine_cmd)
