#!/usr/bin/env python

import sys
import os
import glob
import subprocess

def file_exists(file):
  try:
    os.stat(file)
  except:
    return False
  return True

def convert_to_flac(file, dest_file):
  if file_exists(dest_file):
    print("%s already exists" % dest_file)
  else:
    print("Writing {f}".format(f=dest_file).encode("utf8"))
    sys.stdout.flush()
    subprocess.call(["ffmpeg", "-i", file, "-compression_level", "12", dest_file])

def get_dest_song_file(file):
  return "%s.flac" % os.path.splitext(file)[0]

def all_songs_in(path):
  for root, dirs, files in os.walk(path):
    for filename in files:
      if filename.endswith(".m4a"):
        yield os.path.join(root, filename)


search_dir = os.path.abspath(sys.argv[1])

for song in all_songs_in(search_dir):
  convert_to_flac(song, get_dest_song_file(song))
